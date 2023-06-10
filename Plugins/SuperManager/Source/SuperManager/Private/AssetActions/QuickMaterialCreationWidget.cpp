// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickMaterialCreationWidget.h"
#include "DebugHeader.h"
#include "EditorUtilityLibrary.h"
#include "EditorAssetLibrary.h"
#include "AssetToolsModule.h"
#include "Factories/MaterialFactoryNew.h"

#pragma region QuickMaterialCreationCore

void UQuickMaterialCreationWidget::CreateMaterialFromSelectedTextures()
{
	if (bCustomMaterialName)
	{
		if (MaterialName.IsEmpty() || MaterialName.Equals(TEXT("M_"))) 
		{
			DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("Please enter valid name"));
			return;
		}
	}

	TArray<FAssetData> SelectedAssetsData = UEditorUtilityLibrary::GetSelectedAssetData();

	TArray<UTexture2D*> SelectedTexturesArray;
	FString SelectedTextureFolderPath;

	if(!ProcessSelectedData(SelectedAssetsData,SelectedTexturesArray,SelectedTextureFolderPath)) return;

	if(CheckIsNameUsed(SelectedTextureFolderPath,MaterialName)) return;


	UMaterial* CreatedMaterial =  CreateMaterialAsset(MaterialName,SelectedTextureFolderPath);

	if (!CreatedMaterial)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("Failed to create material"));
		return;
	}
}


#pragma endregion

#pragma region QuickMaterialCreation


// Process the selected data, will filter out textures, and return false if non-texture selected  
bool UQuickMaterialCreationWidget::ProcessSelectedData(const TArray<FAssetData>& SelectedDataToProcess,
	TArray<UTexture2D*>& OutSelectedTexturesArray, FString& OutSelectedTexturePackagePath)
{
	if (SelectedDataToProcess.Num()==0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("No texture selected"));
		return false;
	}

	bool bMaterialNameSet = false;
	for (const FAssetData& SelectedData : SelectedDataToProcess)
	{
		UObject* SelectedAsset =  SelectedData.GetAsset();
		if (!SelectedAsset) continue;
		UTexture2D* SelectedTexture = Cast<UTexture2D>(SelectedAsset);

		if (!SelectedTexture)
		{
			DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("Please select only textures ") + SelectedAsset->GetName() + TEXT(" is not a texture"));
			return false;
		}

		OutSelectedTexturesArray.Add(SelectedTexture);

		if (OutSelectedTexturePackagePath.IsEmpty())
		{
			OutSelectedTexturePackagePath = SelectedData.PackagePath.ToString();
		}

		if (!bCustomMaterialName && !bMaterialNameSet)
		{
			MaterialName = SelectedAsset->GetName();
			MaterialName.RemoveFromStart(TEXT("T_"));
			MaterialName.InsertAt(0,TEXT("M_"));
			bMaterialNameSet=true;
		}
	}

	return true;
}
// Will return true if the material name is used by asset under the specified folder
bool UQuickMaterialCreationWidget::CheckIsNameUsed(const FString& FolderPathToCheck, const FString& MaterialNameToCheck)
{
	TArray<FString> ExsitingAssetsPaths =  UEditorAssetLibrary::ListAssets(FolderPathToCheck,false);

	for (const FString& ExistingAssetPath : ExsitingAssetsPaths)
	{
		const FString ExistingAssetName = FPaths::GetBaseFilename(ExistingAssetPath);

		if (ExistingAssetName.Equals(MaterialNameToCheck))
		{
			DebugHeader::ShowMessageDialog(EAppMsgType::Ok, MaterialNameToCheck +TEXT(" is already used by asset"));
			return true;
		}
	}

	return false;
}

UMaterial* UQuickMaterialCreationWidget::CreateMaterialAsset(const FString& NameOfTheMaterial,
	const FString& PathToPutMaterial)
{
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));

	UMaterialFactoryNew* MaterialFactory = NewObject<UMaterialFactoryNew>();
	
	UObject* CreatedObject = AssetToolsModule.Get().CreateAsset(NameOfTheMaterial,PathToPutMaterial,UMaterial::StaticClass(),MaterialFactory);

	return Cast<UMaterial>(CreatedObject);
}

#pragma endregion



