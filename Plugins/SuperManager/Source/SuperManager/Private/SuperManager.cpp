// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "CustomStyle/SuperManagerStyle.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"


void FSuperManagerModule::StartupModule()
{
	FSuperManagerStyle::InitializeIcons();
	InitCBMenuExtension();
	RegisterAdvanceDeletionTab();
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

#pragma region ContentBrowserMenuExtension

void FSuperManagerModule::InitCBMenuExtension()
{
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(
		TEXT("ContentBrowser"));

	TArray<FContentBrowserMenuExtender_SelectedPaths>& ContentBrowserModuleMenuExtenders = ContentBrowserModule.
		GetAllPathViewContextMenuExtenders();

	// FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate;
	// CustomCBMenuDelegate.BindRaw(this,&FSuperManagerModule::CustomCBMenuExtender);
	// ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);

	ContentBrowserModuleMenuExtenders.Add(
		FContentBrowserMenuExtender_SelectedPaths::CreateRaw(this, &FSuperManagerModule::CustomCBMenuExtender));
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());

	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(
			FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this, &FSuperManagerModule::AddCBMenuEntry)
		);

		FolderPathsSelected = SelectedPaths;
	}

	return MenuExtender;
}

void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Unused Assets")),
		FText::FromString(TEXT("Safely delete all unused asset under folder")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"ContentBrowser.DeleteUnusedAssets"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Delete Empty Folders")),
		FText::FromString(TEXT("Safely delete all empty folders")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"ContentBrowser.DeleteEmptyFolders"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnDeleteEmptyFoldersClicked)
	);

	MenuBuilder.AddMenuEntry
	(
		FText::FromString(TEXT("Advence Deletion")),
		FText::FromString(TEXT("List assets by specifc condition in a tab for deleting ")),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"ContentBrowser.AdvanceDeletion"),
		FExecuteAction::CreateRaw(this, &FSuperManagerModule::OnAdvanceDeletionButtonClicked)
	);
}


void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	FixupRedirectors();
	
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion"));
}

void FSuperManagerModule::OnDeleteEmptyFoldersClicked()
{
	FixupRedirectors();
	
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("You can only do this to one folder"));
		return;
	}

	TArray<FString> FolderPathArray = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0],true,true);

	uint32 Counter = 0;

	FString EmptyFolderPathsNames;
	TArray<FString> EmptyFoldersPathsArray;

	if (FolderPathArray.Num()==0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("Directory dosen't have sub directory"));
		return;
	}

	for (const FString& FolderPath:FolderPathArray)
	{
		if (FolderPath.Contains(TEXT("Developers")) || FolderPath.Contains(TEXT("Collections"))) continue;

		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));
			EmptyFoldersPathsArray.Add(FolderPath);
		}

	}

	if (EmptyFoldersPathsArray.Num() == 0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("No empty foulder found under selected folder"),false);
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMessageDialog(EAppMsgType::OkCancel,TEXT("Empty Folders Found in: \n")+ EmptyFolderPathsNames + TEXT("\nWould you like to delete all?"),false);

	if (ConfirmResult == EAppReturnType::Cancel) return;

	for (const FString& EmptyFolderPath:EmptyFoldersPathsArray)
	{
		UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath) ? ++Counter : DebugHeader::Print(TEXT("Faild to delete "+ EmptyFolderPath),FColor::Red);
	}


	if (Counter>0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Succesfuly deleted ") + FString::FromInt(Counter) + TEXT("folder"));
	}

	
}

void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	if (FolderPathsSelected.Num() > 1)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("You can only do this to one folder"));
		return;
	}

	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	if (AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("No asset found under selected folder"));
		return;
	}

	const EAppReturnType::Type ConfirmResult = DebugHeader::ShowMessageDialog(
		EAppMsgType::YesNo,TEXT("A total of ")
		+ FString::FromInt(AssetsPathNames.Num())
		+ TEXT(" found.\nWould you like to procceed?")
	);

	if (ConfirmResult == EAppReturnType::No) return;

	FixupRedirectors();

	TArray<FAssetData> UnusedAssetsDataArray;

	for (const FString& AssetPathName : AssetsPathNames)
	{
		//Don't touch root folder
		if (AssetsPathNames.Contains(TEXT("Developers")) || AssetsPathNames.Contains(TEXT("Collections"))) continue;

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) return;

		TArray<FString> AssetRefferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		if (AssetRefferencers.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);

			UnusedAssetsDataArray.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsDataArray.Num() > 0)
	{
		int32 DeletedUnusedAssetCount = ObjectTools::DeleteAssets(UnusedAssetsDataArray);
		if (DeletedUnusedAssetCount > 0)
		{
			DebugHeader::ShowNotifyInfo(
				TEXT("Successfully deleted ") + FString::FromInt(DeletedUnusedAssetCount) + TEXT(" unused asset from ")
				+ FolderPathsSelected[0]);
		}
		else
		{
			DebugHeader::ShowNotifyInfo(TEXT("Unused assets not deleted."));
		}
	}
	else
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("No unused asset found under selected folder"));
	}
}

void FSuperManagerModule::FixupRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsFixArray;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(
		TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;

	AssetRegistryModule.Get().GetAssets(Filter, OutRedirectors);

	for (const FAssetData& RedirectorData : OutRedirectors)
	{
		if (UObjectRedirector* RedirectorTofix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsFixArray.Add(RedirectorTofix);
		}
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	AssetToolsModule.Get().FixupReferencers(RedirectorsFixArray);
}

#pragma endregion


#pragma region CustomEditorTab

void FSuperManagerModule::RegisterAdvanceDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		FName("AdvanceDeletion"),
		FOnSpawnTab::CreateRaw(this,&FSuperManagerModule::OnSpawnAdvanceDeletionTab)
		).SetDisplayName(FText::FromString(TEXT("Advance Deletion")))
	.SetIcon(FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"ContentBrowser.AdvanceDeletion"));
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& Args)
{
	
	return
	SNew(SDockTab).TabRole(ETabRole::NomadTab)
	[
	SNew(SAdvanceDeletionTab)
	.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
	.CurrentSelectedFolder(FolderPathsSelected[0])
	];
	
	
	
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray<TSharedPtr<FAssetData>> AvailableAssetsData;
	
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelected[0]);

	for (const FString& AssetPathName : AssetsPathNames)
	{
		//Don't touch root folder
		if (AssetsPathNames.Contains(TEXT("Developers")) || AssetsPathNames.Contains(TEXT("Collections"))) continue;

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		TArray<FString> AssetRefferencers = UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);

		AvailableAssetsData.Add(MakeShared<FAssetData>(Data));
	}

	return  AvailableAssetsData;
	
}


#pragma endregion


#pragma region  ProcessDataForAdvanceDeletionTab


bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);
	
	if(ObjectTools::DeleteAssets(AssetDataForDeletion)>0)
	{
		return true;
	}


	return false;
}

bool FSuperManagerModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsToDelete)>0)
	{
		return true;
	}

	return false;
}

void FSuperManagerModule::ListUnusedAssetsForAssetsList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
	TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();
	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		TArray<FString> AssetReffrencer = UEditorAssetLibrary::FindPackageReferencersForAsset(DataSharedPtr->ObjectPath.ToString());

		if (AssetReffrencer.Num() == 0)
		{
			OutUnusedAssetsData.Add(DataSharedPtr);
		}
	}
}

void FSuperManagerModule::ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
	TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData)
{
	OutSameNameAssetsData.Empty();
 
	// Multimap for supporting finding assets with same name

	TMultiMap<FString,TSharedPtr<FAssetData>> AssetsInfoMultiMap;
	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		AssetsInfoMultiMap.Emplace(DataSharedPtr->AssetName.ToString(),DataSharedPtr);
	}

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		TArray<TSharedPtr<FAssetData>> OutAssetsData;
		AssetsInfoMultiMap.MultiFind(DataSharedPtr->AssetName.ToString(),OutAssetsData);

		if (OutAssetsData.Num() <= 1) continue;


		for (const TSharedPtr<FAssetData>& SamaNameData : OutAssetsData)
		{
			if (SamaNameData.IsValid())
			{
				OutSameNameAssetsData.AddUnique(SamaNameData);
			}
		}
	}
}

void FSuperManagerModule::SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync)
{
	TArray<FString> AssetsPathToSync;
	AssetsPathToSync.Add(AssetPathToSync);
	UEditorAssetLibrary::SyncBrowserToObjects(AssetsPathToSync);
}
#pragma endregion


void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AdvanceDeletion"));

	FSuperManagerModule::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)
