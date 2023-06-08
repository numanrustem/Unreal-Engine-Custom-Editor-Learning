// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickAssetAction.h"

#include "AssetToolsModule.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"


void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if (NumOfDuplicates <= 0)
	{
		// Print(TEXT/*("Please Enter Valid Number"),FColor::Red);
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("Please Enter Valid Number"));
		return;
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();

	uint32 Counter = 0;

	for (const FAssetData& SelectedAssetData : SelectedAssetsData)
	{
		for (int32 i = 0; i < NumOfDuplicates; ++i)
		{
			const FString SourceAssetPath = SelectedAssetData.ObjectPath.ToString();
			const FString NewDuplicatedAssetName = SelectedAssetData.AssetName.ToString() + TEXT("_") + FString::FromInt(i + 1);
			const FString NewPathName = FPaths::Combine(SelectedAssetData.PackagePath.ToString(),NewDuplicatedAssetName);

			if(UEditorAssetLibrary::DuplicateAsset(SourceAssetPath,NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName,false);
				++Counter;
				
			}
		}
	}

	if (Counter>0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Succesfuly duplicated " + FString::FromInt(Counter) + " files"));
		//Print(TEXT("Succesfuly duplicated " + FString::FromInt(Counter) + " files"), FColor::Green);
	}
	
}

void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedObjects = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;

	for (UObject* SelectedObject :SelectedObjects)
	{
		if (!SelectedObject) continue;

		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());

		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			DebugHeader::Print(TEXT("Failed to find prefix for class") + SelectedObject->GetClass()->GetName(),FColor::Red);
			continue;
		}
		FString OldName = SelectedObject->GetName();
		if (OldName.StartsWith(*PrefixFound))
		{
			DebugHeader::Print(OldName + TEXT(" already has prefix added"),FColor::Red);
			continue;
		}

		// To Clear If Has, other prefix
		for (TPair<UClass*,FString> pre:PrefixMap)
		{
			if (OldName.Contains(pre.Value))
			{
				OldName.RemoveAt(0,pre.Value.Len()-1);
			}
		}

		// To Clear instance suffix
		if(OldName.Contains("_Inst"))
		{
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}
		
		const FString NewNameWithPrefix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelectedObject,NewNameWithPrefix);

		++Counter;
		
	}

	if(Counter>0)
	{
		DebugHeader::ShowNotifyInfo(TEXT("Succesfuly renamed " + FString::FromInt(Counter)));
	}
	
}

void UQuickAssetAction::RemoveUnusedAssets()
{

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();

	TArray<FAssetData> UnUsedAssetsData;

	FixUpRedirectors();
	
	for (const FAssetData& SelectedAsset:SelectedAssetsData)
	{
		TArray<FString> AssetReferencer = UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAsset.ObjectPath.ToString());

		if (AssetReferencer.Num() == 0)
		{
			UnUsedAssetsData.Add(SelectedAsset);
		}

		
	}

	if (UnUsedAssetsData.Num() == 0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("No unused asset found among selected assets"),false);
		return;
	}

	
	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnUsedAssetsData);

	if (NumOfAssetsDeleted == 0) return;

	DebugHeader::ShowNotifyInfo(TEXT("Successfully deleted " + FString::FromInt(NumOfAssetsDeleted) + TEXT(" unused assets.")));

	
}

void UQuickAssetAction::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorstoFixArray;

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	FARFilter Filter;
	Filter.bRecursivePaths = true;
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassNames.Emplace("ObjectRedirector");

	TArray<FAssetData> OutRedirectors;
	AssetRegistryModule.Get().GetAssets(Filter,OutRedirectors);

	for (const FAssetData& RedirectorData:OutRedirectors)
	{
		if(UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorstoFixArray.Add(RedirectorToFix);
		}
	}

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	AssetToolsModule.Get().FixupReferencers(RedirectorstoFixArray);
	
}