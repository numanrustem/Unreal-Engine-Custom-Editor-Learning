// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidgets/AdvanceDeletionWidget.h"
#include "SuperManager.h"
#include "SlateBasics.h"
#include "DebugHeader.h"

#define ListAll TEXT("List All Available Assets")
#define ListUnused TEXT("List Unused Assets")
#define ListSameName TEXT("List Assets With Same Name")

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus =true;
	
	StoredAssetsData = InArgs._AssetsDataToStore;
	DisplayedAssetsData = StoredAssetsData;

	CheckBoxesArray.Empty();
	AssetsDataToDeleteArray.Empty();
	ComboBoxSourceItems.Empty();

	ComboBoxSourceItems.Add(MakeShared<FString>(ListAll));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListUnused));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListSameName));
	
	FSlateFontInfo TitleTextFont = GetEmbossedTextFont();
	TitleTextFont.Size = 30;

	
	ChildSlot
	[
		// Main Vertical Box - Can Have Multi Slot
		SNew(SVerticalBox)
		// First Vertical slot for title text
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::FromString(TEXT("Advance Deletion")))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]
		// SecondSlot for dropdown to specify the listing condition
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			//Combo Box Slot
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				ConstructComboBox()
			]
			+SHorizontalBox::Slot()
			.FillWidth(.6f)
			[
				ConstructComboHelpTexts(TEXT("Specify the listing condition in the drop. Left mouse click to  go to asset where"),ETextJustify::Center)
			]
			+SHorizontalBox::Slot()
			.FillWidth(.1f)
			[
				ConstructComboHelpTexts(TEXT("Current Folder:\n" + InArgs._CurrentSelectedFolder),ETextJustify::Right)
			]
		]
		// Third slot for asset list
		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			[
				ConstructAssetListView()
			]
		]
		// Fourth slot for 3 buttons
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			//button 1 slot
			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
			ConstructDeleteAllButton()
			]
			//button 2 slot
			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
			ConstructSelectAllButton()
			]
			//button 3 slot
			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
			ConstructDeselectAllButton()
			]
		]
	];
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView = SNew(SListView<TSharedPtr<FAssetData>>)
	.ItemHeight(24.f)
	.ListItemsSource(&DisplayedAssetsData)
	.OnGenerateRow(this,&SAdvanceDeletionTab::OnGenerateRowForList)
	.OnMouseButtonClick(this,&SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked);

	return ConstructedAssetListView.ToSharedRef();
	
}
void SAdvanceDeletionTab::RefreshAssetListView()
{
	CheckBoxesArray.Empty();
	AssetsDataToDeleteArray.Empty();
	
	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}

FSuperManagerModule& SAdvanceDeletionTab::GetSuperManagerModule()
{
	return FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
}


#pragma region ComboBoxForListingCondition

TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvanceDeletionTab::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox=
		SNew(SComboBox<TSharedPtr<FString>>)
		.OptionsSource(&ComboBoxSourceItems)
		.OnGenerateWidget(this,&SAdvanceDeletionTab::OnGenerateComboContent)
		.OnSelectionChanged(this,&SAdvanceDeletionTab::OnComboSelectionChanged)
		[
			SAssignNew(ComboDisplayTextBlock,STextBlock)
			.Text(FText::FromString(TEXT("List Assets Option")))
		];

	return ConstructedComboBox;
	
	
}

TSharedRef<SWidget> SAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> ConstructedComboText = SNew(STextBlock).Text(FText::FromString(*SourceItem.Get()));

	return ConstructedComboText;

}

void SAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{

	FSuperManagerModule& SuperManagerModule = GetSuperManagerModule();
	DebugHeader::Print(*SelectedOption.Get(),FColor::Cyan);

	ComboDisplayTextBlock->SetText(FText::FromString(*SelectedOption.Get()));

	// Pass data for our module to filter

	if (*SelectedOption.Get() == ListAll)
	{
		DisplayedAssetsData = StoredAssetsData;
		RefreshAssetListView();
	}
	else if(*SelectedOption.Get()== ListUnused)
	{
		// list all unused assets
		SuperManagerModule.ListUnusedAssetsForAssetsList(StoredAssetsData,DisplayedAssetsData);
		RefreshAssetListView();
	}
	else if(*SelectedOption.Get()== ListSameName)
	{
		// list all assets with same name
		SuperManagerModule.ListSameNameAssetsForAssetList(StoredAssetsData,DisplayedAssetsData);
		RefreshAssetListView();
	}
	
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructComboHelpTexts(const FString& TextContent,	ETextJustify::Type TextJustify)
{
	TSharedRef<STextBlock> ConstructedHelpText = SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Justification(TextJustify)
	.AutoWrapText(true);

	return ConstructedHelpText;
}

#pragma endregion

#pragma region RowWidgetForAssetView




TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
                                                                const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow<TSharedPtr<FAssetData>>,OwnerTable);

	const FString DisplayAssetClassName = AssetDataToDisplay->AssetClass.ToString();
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmbossedTextFont();
	AssetClassNameFont.Size = 10;

	FSlateFontInfo AssetNameFont = GetEmbossedTextFont();
	AssetNameFont.Size = 15;
	
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget = SNew(STableRow<TSharedPtr<FAssetData>>,OwnerTable).Padding(FMargin(5.f))
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.FillWidth(.05f)
		[
			ConstructCheckbox(AssetDataToDisplay)
		]
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		.FillWidth(.2f)
		[
			ConstructTextForRowWidget(DisplayAssetClassName,AssetClassNameFont)
		]
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			ConstructTextForRowWidget(DisplayAssetName,AssetNameFont)
		]
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		[
			ConstructButton(AssetDataToDisplay)
		]
		
	];

	return ListViewRowWidget;
}

void SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData)
{
	FSuperManagerModule& SuperManagerModule = GetSuperManagerModule();

	SuperManagerModule.SyncCBToClickedAssetForAssetList(ClickedData->ObjectPath.ToString());
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckbox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> CreatedCheckbox = SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this,&SAdvanceDeletionTab::OnCheckboxStateChanged,AssetDataToDisplay)
	.Visibility(EVisibility::Visible);

	CheckBoxesArray.Add(CreatedCheckbox);
	return CreatedCheckbox;
}

void SAdvanceDeletionTab::OnCheckboxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		if(AssetsDataToDeleteArray.Contains(AssetData))
		{
			AssetsDataToDeleteArray.Remove(AssetData);
		}
		break;
	case ECheckBoxState::Checked:
		AssetsDataToDeleteArray.AddUnique(AssetData);
		break;
	case ECheckBoxState::Undetermined:
		break;
	}
	
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent, const FSlateFontInfo& FontToUse)
{
	TSharedRef<STextBlock> CreatedText = SNew(STextBlock)
	.Text(FText::FromString(TextContent))
	.Font(FontToUse)
	.ColorAndOpacity(FColor::White);

	return CreatedText;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButton(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> CreatedButton = SNew(SButton)
	.Text(FText::FromString(TEXT("Delete")))
	.OnClicked(this,&SAdvanceDeletionTab::OnDeleteButtonClicked,AssetDataToDisplay);

	return CreatedButton;
}

FReply SAdvanceDeletionTab::OnDeleteButtonClicked(TSharedPtr<FAssetData> ClickedAssetData)
{
	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	const bool bAssetDeleted = SuperManagerModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());

	if (bAssetDeleted)
	{
		// Update the list source item
		if(StoredAssetsData.Contains(ClickedAssetData))
		{
			StoredAssetsData.Remove(ClickedAssetData);
		}
		if (DisplayedAssetsData.Contains(ClickedAssetData))
		{
			DisplayedAssetsData.Remove(ClickedAssetData);
		}

		//Refresh the list
		
		RefreshAssetListView();
	}
	
	return FReply::Handled();
}

#pragma endregion

#pragma region TabButton
TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this,&SAdvanceDeletionTab::OnDeleteAllButtonClicked);

	DeleteAllButton->SetContent(ConsturctTextForTabButtons(TEXT("Delete All")));

	return DeleteAllButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this,&SAdvanceDeletionTab::OnSelectAllButtonClicked);

	SelectAllButton->SetContent(ConsturctTextForTabButtons(TEXT("Select All")));

	return SelectAllButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this,&SAdvanceDeletionTab::OnDeselectAlButtonClicked);

	DeselectAllButton->SetContent(ConsturctTextForTabButtons(TEXT("Deselect All")));

	return DeselectAllButton;
}

FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	if (AssetsDataToDeleteArray.Num()== 0)
	{
		DebugHeader::ShowMessageDialog(EAppMsgType::Ok,TEXT("No Asset Currently Selected"));
		return FReply::Handled();
	}

	// Pass data to out module for deletion
	TArray<FAssetData> AssetDataToDelete;
	for(const TSharedPtr<FAssetData>& Data : AssetsDataToDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}

	FSuperManagerModule& SuperManagerModule = FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	const bool bAssetsDeleted = SuperManagerModule.DeleteMultipleAssetsForAssetList(AssetDataToDelete);

	if (bAssetsDeleted)
	{
		for(const TSharedPtr<FAssetData>& DeletedData : AssetsDataToDeleteArray)
		{
			if (StoredAssetsData.Contains(DeletedData))
			{
				StoredAssetsData.Remove(DeletedData);
			}
			if (DisplayedAssetsData.Contains(DeletedData))
			{
				DisplayedAssetsData.Remove(DeletedData);
			}
		}
		
		RefreshAssetListView();
	}
	
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
{
	if (CheckBoxesArray.Num()== 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeselectAlButtonClicked()
{
	if (CheckBoxesArray.Num()== 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox>& CheckBox : CheckBoxesArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState();
		}
	}
	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConsturctTextForTabButtons(const FString& Content)
{
	FSlateFontInfo ButtonTextFont = GetEmbossedTextFont();
	ButtonTextFont.Size = 15;
	TSharedRef<STextBlock> ConsturectedTextBlock = SNew(STextBlock)
	.Text(FText::FromString(Content))
	.Font(ButtonTextFont)
	.Justification(ETextJustify::Center);

	return ConsturectedTextBlock;
}

#pragma endregion 



