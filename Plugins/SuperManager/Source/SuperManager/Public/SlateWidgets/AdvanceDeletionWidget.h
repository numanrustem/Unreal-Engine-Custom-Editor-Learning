// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"
#include "SuperManager.h"

class SAdvanceDeletionTab : public SCompoundWidget
{

	SLATE_BEGIN_ARGS(SAdvanceDeletionTab){}

	SLATE_ARGUMENT(TArray<TSharedPtr<FAssetData>>,AssetsDataToStore)
	SLATE_ARGUMENT(FString,CurrentSelectedFolder)
	SLATE_END_ARGS()

public:

	void Construct(const FArguments& InArgs);

private:

	TArray<TSharedPtr<FAssetData>> StoredAssetsData;
	TArray<TSharedPtr<FAssetData>> DisplayedAssetsData;
	TArray<TSharedPtr<FAssetData>> AssetsDataToDeleteArray;
	TArray<TSharedRef<SCheckBox>> CheckBoxesArray;


	TSharedRef<SListView<TSharedPtr<FAssetData>>> ConstructAssetListView();
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> ConstructedAssetListView;
	void RefreshAssetListView();
	FSuperManagerModule& GetSuperManagerModule();
	

#pragma region ComboBoxForListingCondition

	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructComboBox();
	TArray<TSharedPtr<FString>> ComboBoxSourceItems;

	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);

	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption,ESelectInfo::Type InSelectInfo);

	TSharedPtr<STextBlock> ComboDisplayTextBlock;

	TSharedRef<STextBlock> ConstructComboHelpTexts(const FString& TextContent,ETextJustify::Type TextJustify);
#pragma endregion 

	
#pragma region RowWidgetForAssetView
	
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay, const TSharedRef<STableViewBase>& OwnerTable);
	void OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData);
	TSharedRef<SCheckBox> ConstructCheckbox(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	void OnCheckboxStateChanged(ECheckBoxState NewState,TSharedPtr<FAssetData> AssetData);
	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent,const FSlateFontInfo& FontToUse);
	TSharedRef<SButton> ConstructButton(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	FReply OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData);
	
#pragma endregion

#pragma region TabButton
	TSharedRef<SButton> ConstructDeleteAllButton();
	TSharedRef<SButton> ConstructSelectAllButton();
	TSharedRef<SButton> ConstructDeselectAllButton();

	FReply OnDeleteAllButtonClicked();
	FReply OnSelectAllButtonClicked();
	FReply OnDeselectAlButtonClicked();

	TSharedRef<STextBlock> ConsturctTextForTabButtons(const FString& Content);
#pragma endregion
	
	
	FSlateFontInfo GetEmbossedTextFont() const {return FCoreStyle::Get().GetFontStyle(FName("EmbossedText"));}

	
	

};


 