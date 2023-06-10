// Fill out your copyright notice in the Description page of Project Settings.


#include "AssetActions/QuickMaterialCreationWidget.h"
#include "DebugHeader.h"
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
}

#pragma endregion

