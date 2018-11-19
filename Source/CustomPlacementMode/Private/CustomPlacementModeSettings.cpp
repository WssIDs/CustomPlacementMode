
#include "CustomPlacementModeSettings.h"
#include "UObject/UnrealType.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "EditorStyleSet.h"
#include "Framework/Notifications/NotificationManager.h"
#include "ICustomPlacementModeModule.h"
#include "Misc/MessageDialog.h"
#include <PropertyHandle.h>
#include <DetailWidgetRow.h>
#include <STextBlock.h>
#include <DetailLayoutBuilder.h>
#include <IDocumentation.h>
#include <DetailCategoryBuilder.h>
#include <ITypedTableView.h>
#include <Private/CollisionProfileDetails.h>
#include <SButton.h>
#include <SToolTip.h>
#include <STableRow.h>
#include <SlateOptMacros.h>
#include "SCheckBox.h"
#include <SComboBox.h>
#include <SEditableTextBox.h>
#include "SComboButton.h"
#include "Kismet2/SClassPickerDialog.h"
#include <Private/SClassViewer.h>
#include <Private/Factories/SoundFactoryUtility.h>
#include <ClassViewerModule.h>
#include <SListView.h>
#include <SScrollBox.h>
#include <SUniformGridPanel.h>
#include "PropertyCustomizationHelpers.h"
#include <SWindow.h>
#include <SImage.h>
#include "CoreStyle.h"
#include "Styles/CustomCategoryStyles.h"

#include "Algo/Reverse.h"
#include <SColorBlock.h>
#include <SlateBoxBrush.h>
#include "SColorPicker.h"
#include "Editor.h"
#include <MainFrame/Public/Interfaces/IMainFrameModule.h>
#include <MultiBoxBuilder.h>
#include <GenericCommands.h>

DEFINE_LOG_CATEGORY(CustomPlacementModeSettingLog);


UCustomPlacementModeSettings::UCustomPlacementModeSettings()
{
}


void UCustomPlacementModeSettings::PreEditChange(class FEditPropertyChain & PropertyAboutToChange)
{
	TempPlaceableCategoryItems.Empty();

	for (auto& Item : PlaceableCategoryItems)
	{
		TempPlaceableCategoryItems.Add(Item);
	}
	
	Super::PreEditChange(PropertyAboutToChange);
}

void UCustomPlacementModeSettings::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
		
	if (PropertyChangedEvent.Property != nullptr)
	{
		CheckTestItems(PropertyChangedEvent.Property);
	}
}


void UCustomPlacementModeSettings::CheckTestItems(UProperty* Property)
{
	FName PropertyName = (Property != nullptr) ? Property->GetFName() : NAME_None;


	FNotificationInfo DublicateSetCategoryName(NSLOCTEXT("CustomPlacementMode", "DublicateCategory", "Cannot add new element to the set while an element with the default value exists"));
	DublicateSetCategoryName.Image = FEditorStyle::GetBrush(TEXT("NotificationList.DefaultMessage"));
	DublicateSetCategoryName.FadeInDuration = 0.1f;
	DublicateSetCategoryName.FadeOutDuration = 0.5f;
	DublicateSetCategoryName.ExpireDuration = 1.5f;
	DublicateSetCategoryName.bUseThrobber = false;
	DublicateSetCategoryName.bUseSuccessFailIcons = true;
	DublicateSetCategoryName.bUseLargeFont = true;
	DublicateSetCategoryName.bFireAndForget = true;
	DublicateSetCategoryName.bAllowThrottleWhenFrameRateIsLow = false;

	if (PropertyName == "PlaceableCategoryItems")
	{
		int32 LastItemIndex = 0;
		
		if(PlaceableCategoryItems.Num() > TempPlaceableCategoryItems.Num())
		{
			for(int i = 0; i < PlaceableCategoryItems.Num(); i++)
			{
				if(!TempPlaceableCategoryItems.Contains(PlaceableCategoryItems[i]))
				{
					LastItemIndex = i;
				}
			}

			for (int i = 0; i < TempPlaceableCategoryItems.Num(); i++)
			{
				if (TempPlaceableCategoryItems[i].CategoryName == PlaceableCategoryItems[LastItemIndex].CategoryName)
				{
					PlaceableCategoryItems.RemoveAt(LastItemIndex);
					
					if (!NotificationSetAddDublicateCategoryName)
					{
						NotificationSetAddDublicateCategoryName = FSlateNotificationManager::Get().AddNotification(DublicateSetCategoryName);
					}

					else
					{
						if (!NotificationSetAddDublicateCategoryName->IsParentValid())
						{
							NotificationSetAddDublicateCategoryName = FSlateNotificationManager::Get().AddNotification(DublicateSetCategoryName);
						}
					}

					break;
				}
			}
		}
	}

	if (PropertyName == "CategoryName")
	{
		if (PlaceableCategoryItems.Num() == TempPlaceableCategoryItems.Num())
		{
			FSettingPlaceableCategoryItem ChangedItem;
			FSettingPlaceableCategoryItem ReturnedItem;

			int index = 0;

			for (auto& Item : PlaceableCategoryItems)
			{
				if (Item.CategoryName != TempPlaceableCategoryItems[index].CategoryName)
				{
					ChangedItem = Item;
					ReturnedItem = TempPlaceableCategoryItems[index];
				}

				index++;
			}

			FText ReturnDialogMessage;
			FString ReturnMessage;
			FFormatOrderedArguments ReturnArguments;
			ReturnArguments.Add(FText::FromString(ReturnedItem.CategoryName.ToString()));

			bool bReturnItems = false;

			for (int i = 0; i < TempPlaceableCategoryItems.Num(); i++)
			{
				if (ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::RecentlyPlaced() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::Basic() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::Lights() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::Visual() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::Volumes() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::Custom() ||
					ChangedItem.CategoryName == FCustomBuiltInPlacementCategories::AllClasses()
					)
				{
					bReturnItems = true;
					ReturnDialogMessage = FText::Format(NSLOCTEXT("CustomPlacementMode","DialogValueChange","Default category names are not allowed.\nThe current category name will be returned to - {0}\n\nDefaultCategories:\n-RecentlyPlaced\n-Basic\n-Lights\n-Visual\n-Volumes\n-AllClasses\n-Custom"), ReturnArguments);
					break;
				}
				
				if (TempPlaceableCategoryItems[i].CategoryName == ChangedItem.CategoryName)
				{
					bReturnItems = true;
					ReturnDialogMessage = FText::Format(NSLOCTEXT("CustomPlacementMode","DialogValueChange","Dublicate category names are not allowed.\nThe current category name will be returned to {0}"), ReturnArguments);
					break;
				}
			}

			if (bReturnItems)
			{
				FNotificationInfo InfoDublicateCategoryName(FText::Format(NSLOCTEXT("CustomPlacementMode","ValueChange","The name of the current category has been returned to {0}"), ReturnArguments));
				InfoDublicateCategoryName.Image = FEditorStyle::GetBrush(TEXT("Cascade.AddLODBeforeCurrent"));
				InfoDublicateCategoryName.FadeInDuration = 0.1f;
				InfoDublicateCategoryName.FadeOutDuration = 0.5f;
				InfoDublicateCategoryName.ExpireDuration = 1.5f;
				InfoDublicateCategoryName.bUseThrobber = false;
				InfoDublicateCategoryName.bUseSuccessFailIcons = true;
				InfoDublicateCategoryName.bUseLargeFont = true;
				InfoDublicateCategoryName.bFireAndForget = false;
				InfoDublicateCategoryName.bAllowThrottleWhenFrameRateIsLow = false;
				TSharedPtr<SNotificationItem> NotificationDeblicateCategoryName;

				FMessageDialog* DialogMsg = new FMessageDialog();

				if (DialogMsg)
				{
					FText Title = FText::FromString(TEXT("Error"));

					DialogMsg->Open(
						EAppMsgType::Ok,
						ReturnDialogMessage,
						&Title
					);
					
					int i = 0;

					for (auto& Item : PlaceableCategoryItems)
					{
						Item.CategoryName = TempPlaceableCategoryItems[i].CategoryName;
						i++;
					}

					if (!NotificationDeblicateCategoryName)
					{
						NotificationDeblicateCategoryName = FSlateNotificationManager::Get().AddNotification(InfoDublicateCategoryName);
					}

					NotificationDeblicateCategoryName->SetCompletionState(SNotificationItem::CS_Success);
					NotificationDeblicateCategoryName->ExpireAndFadeout();
				}
			}
		}
	}	

	if (PropertyName == "Items")
	{
		FNotificationInfo Info(NSLOCTEXT("CustomPlacementMode", "ItemsChange", "All problem items has been resolved"));
		Info.Image = FEditorStyle::GetBrush(TEXT("Cascade.AddLODBeforeCurrent"));
		Info.FadeInDuration = 0.1f;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 1.5f;
		Info.bUseThrobber = false;
		Info.bUseSuccessFailIcons = true;
		Info.bUseLargeFont = true;
		Info.bFireAndForget = false;
		Info.bAllowThrottleWhenFrameRateIsLow = false;
		TSharedPtr<SNotificationItem> NotificationCategoryItem;

		int i = 0;
		
		for (auto& PlacementCategoryItem : PlaceableCategoryItems)
		{
			FSettingPlaceableCategoryItem SettingCategoryItem = PlacementCategoryItem;

			if (SettingCategoryItem.Items.Num() > 0)
			{
				FString Message;
				FMessageDialog* DialogMsg = nullptr;
				TSet<TSubclassOf<AActor>> NeedForDelete;

				for (auto& Item : SettingCategoryItem.Items)
				{
					for (auto& SortedItem : SettingCategoryItem.Items)
					{
						if (Item != SortedItem)
						{
							if (Item->IsChildOf(SortedItem))
							{
								TArray<FStringFormatArg> Arguments;
								Arguments.Add(FStringFormatArg(Item->GetName()));
								Arguments.Add(FStringFormatArg(SortedItem->GetName()));

								Message += FString::Format(TEXT("{0} is a child of {1}\n"), Arguments);
								NeedForDelete.Add(Item);
							}
						}
					}
				}

				if (!Message.IsEmpty())
				{
					Message += TEXT("All child Actors will be deleted\n");
					
					if (!DialogMsg)
					{
						DialogMsg = new FMessageDialog();
						FText Title = FText::FromString(TEXT("Error"));
						FText MessageText = FText::FromString(Message);
						if(DialogMsg->Open(EAppMsgType::OkCancel, MessageText, &Title) == EAppReturnType::Ok)
						{
							for (auto& ActorItem : NeedForDelete)
							{
								PlacementCategoryItem.Items.Remove(ActorItem);
								PlacementCategoryItem.Items.CompactStable();
								PlacementCategoryItem.Items.Shrink();

								if (!NotificationCategoryItem)
								{
									NotificationCategoryItem = FSlateNotificationManager::Get().AddNotification(Info);
								}

								NotificationCategoryItem->SetCompletionState(SNotificationItem::CS_Success);
								NotificationCategoryItem->ExpireAndFadeout();
							}
						}
						else
						{
							PlacementCategoryItem.Items = TempPlaceableCategoryItems[i].Items;
							
							if (!NotificationCategoryItem)
							{
								NotificationCategoryItem = FSlateNotificationManager::Get().AddNotification(Info);
							}

							NotificationCategoryItem->SetCompletionState(SNotificationItem::CS_Success);
							NotificationCategoryItem->ExpireAndFadeout();
						}
					}
				}
			}
			i++;
		}
	}	
	
}

UCustomPlacementModeSettings* UCustomPlacementModeSettings::Get()
{
	UCustomPlacementModeSettings* CustomPlacement = GetMutableDefault<UCustomPlacementModeSettings>();

	return CustomPlacement;
}

#define LOCTEXT_NAMESPACE "FSettingPlaceableCategoryItemCustomization"

#define RowWidth_Customization 50

#define CATEGORY_WINDOW_WIDTH		650
#define CATEGORY_WINDOW_HEIGHT		630


void SCategoryEditDialog::Construct(const FArguments& InArgs)
{
	bApplyChange = false;

	if (InArgs._CategorySetup)
	{
		CategorySetup = *InArgs._CategorySetup;
	}
	if (InArgs._CategoryIndex)
	{
		CategoryIndex = InArgs._CategoryIndex;
	}

	Settings = InArgs._Settings;

	check(CategorySetup.CategoryName != "");

	OnValidateCategory = InArgs._OnValidateCategory;
	WidgetWindow = InArgs._WidgetWindow;

	RefreshClassList();

	//SelectedClass = AActor::StaticClass();
	onSetClass = FOnSetClass::CreateRaw(this, &SCategoryEditDialog::OnSetClass);


	FSlateFontInfo EditableFont = IDetailLayoutBuilder::GetDetailFont();
	EditableFont.Size = EditableFont.Size + 5;
	
	FSlateFontInfo EditableBoldFont = IDetailLayoutBuilder::GetDetailFontBold();
	EditableBoldFont.Size = EditableBoldFont.Size + 5;
	
	BackgroundImage = new FSlateBrush();
	BackgroundImage->TintColor = FSlateColor(CategorySetup.ColorSetting.BackgroundColor);
	BackgroundImage->ImageSize = FVector2D(8.0f,32.0f);

	BackgroundPressedImage = new FSlateBrush();
	BackgroundPressedImage->TintColor = FSlateColor(CategorySetup.ColorSetting.PressedSelectionColor);
	BackgroundPressedImage->ImageSize = FVector2D(8.0f, 32.0f);

	HoveredImage = new FSlateBrush();
	HoveredImage->TintColor = FSlateColor(CategorySetup.ColorSetting.HoveredColor);
	HoveredImage->ImageSize = FVector2D(8.0f, 32.0f);


	

	this->ChildSlot
	[
		SNew(SVerticalBox)

		// Category Name EditBox
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(3.f, 5.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			//.FillWidth(1)
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SCategoryEditDialog_CategoryName", "Category Name"))
					.Font(EditableBoldFont)
				]
			]
			+ SHorizontalBox::Slot()
				//.AutoWidth()
				.HAlign(HAlign_Fill)
				//.MaxWidth(250.0f)
				[
					SAssignNew(NameBox, SEditableTextBox)
					//.MinDesiredWidth(250.f)
				.Text(this, &SCategoryEditDialog::GetName)
				.Font(EditableFont)
				.OnTextCommitted(this, &SCategoryEditDialog::NewNameEntered)
				.OnTextChanged(this, &SCategoryEditDialog::OnTextChanged)
				]
		]
		// Category ID TextBlock
		+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.Padding(3.f, 3.f)
			[
				SNew(SHorizontalBox)
				.Visibility(Settings->bShowCategoryID ? EVisibility::Visible : EVisibility::Collapsed)
				+ SHorizontalBox::Slot()
				//.FillWidth(1)
				.HAlign(HAlign_Fill)
				[
					SNew(SBox)
					[
						SNew(STextBlock)
						.Text(LOCTEXT("SCategoryEditDialog_CategoryID", "Category ID"))
				.Font(IDetailLayoutBuilder::GetDetailFontBold())
					]
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Fill)
				//.MaxWidth(250.0f)
				[
					SNew(STextBlock)
					.MinDesiredWidth(250.f)
				.Text(this, &SCategoryEditDialog::GetGuid)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				]
		]
		// CheckBox
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(3.f, 3.f)
		[
			SNew(SHorizontalBox)
			
			+ SHorizontalBox::Slot()
			.Padding(3, 3)
			.FillWidth(1)
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SCategoryEditDialog_BlueprintClasses", "Blueprint classes"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked(CategorySetup.bUseBlueprintClasses ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &SCategoryEditDialog::OnCheckStateChangedBlueprintClassed)
			]
		]
		// Category Items
		+ SVerticalBox::Slot()
		.Padding(3.0f,2.0f)
		.HAlign(HAlign_Fill)
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(3, 3)
			.FillWidth(1)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Font(EditableBoldFont)
				.Text(LOCTEXT("CategoryEditorMenu_CategoryItems", "Category Items"))
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 1)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.Text(LOCTEXT("CategoryEditorMenu_NewItem", "+"))
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
				.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
				.ContentPadding( 4.0f )
				.Content()
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush("PropertyWindow.Button_AddToArray"))
				]
				.OnClicked(this, &SCategoryEditDialog::OnNewClassItem)
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 1)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.Text(LOCTEXT("CategoryEditorMenu_Delete", "-"))
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Danger")
				.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
				.ContentPadding( 4.0f )
				.Content()
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush("PropertyWindow.Button_RemoveFromArray"))
				]
				.OnClicked(this, &SCategoryEditDialog::OnDeleteClassItem)
				.IsEnabled(this, &SCategoryEditDialog::IsAnyClassItemSelected)
			]
			+ SHorizontalBox::Slot()
			.Padding(2, 1)
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.Text(LOCTEXT("CategoryEditorMenu_Empty", "Empty"))
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Danger")
				.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
				.ContentPadding( 4.0f )
				.Content()
				[
					SNew(SImage)
					.Image(FEditorStyle::GetBrush("PropertyWindow.Button_EmptyArray"))
				]
				.OnClicked(this, &SCategoryEditDialog::OnDeleteClassItems)
				.IsEnabled(this, &SCategoryEditDialog::IsClassItemsFill)
			]
			+ SHorizontalBox::Slot()
				.Padding(2, 1)
				.AutoWidth()
				.VAlign(VAlign_Center)
				[
					SNew(SButton)
					.Visibility(Settings->bShowRefreshButtons ? EVisibility::Visible : EVisibility::Collapsed)
					.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
					.Text(LOCTEXT("CategoryEditorMenu_Refresh", "Refresh"))
					.ToolTip(
						SNew(SToolTip)
						.Text(FText::FromString("Refresh Items (Experimental)"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					)
					.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
					.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
					.OnClicked(this, &SCategoryEditDialog::OnRefreshClassItem)
					.IsEnabled(this, &SCategoryEditDialog::IsClassItemsFill)
					.ContentPadding(2.0f)
					.Content()
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("AnimEditor.RefreshButton"))
					]
				]
		]
		
		+ SVerticalBox::Slot()
		.Padding(3.f, 2.f)
		.MaxHeight(200.0f)
		.VAlign(VAlign_Top)
		[
			SNew(SScrollBox)
			.Orientation(EOrientation::Orient_Vertical)
			//.ScrollBarAlwaysVisible(true)
			+SScrollBox::Slot()
			.Padding(3, 1)
			[
				SAssignNew(ObjectClassListView, SClassListView)
				.ItemHeight(15.f)
				.ListItemsSource(&ObjectClassList)
				.OnGenerateRow(this, &SCategoryEditDialog::HandleGenerateClassWidget)
				.OnMouseButtonDoubleClick(this, &SCategoryEditDialog::OnClassListItemDoubleClicked)
				.SelectionMode(ESelectionMode::Single)
				.HeaderRow(
					SNew(SHeaderRow)
					// Name
					+ SHeaderRow::Column("ClassIndex")
					.HAlignCell(HAlign_Left)
					.FixedWidth(30)
					.HeaderContentPadding(FMargin(0, 3))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ClassListHeader_Index", "ID"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
					+ SHeaderRow::Column("ClassType")
					.HAlignCell(HAlign_Left)
					.FixedWidth(100)
					.HeaderContentPadding(FMargin(0, 3))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ClassListHeader_TypeName", "Class Type"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
					+ SHeaderRow::Column("ClassName")
					.HAlignCell(HAlign_Left)
					.FillWidth(1)
					.HeaderContentPadding(FMargin(0, 3))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ClassListHeader_Name", "Class Name"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				)
			]
		]

		// Custom Category Color
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(3.f, 10.f,3.f,5.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(3, 3)
			.FillWidth(1)
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SCategoryEditDialog_CustomColor", "Custom category color"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked(CategorySetup.bUseCustomColorCategories ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &SCategoryEditDialog::OnCheckStateChangedCustomColorCategory)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(3,3)
		[
			SAssignNew(PreviewColorSection,SVerticalBox)
			//.IsEnabled(CategorySetup.bUseCustomColorCategories)
			.Visibility(CategorySetup.bUseCustomColorCategories ? EVisibility::Visible : EVisibility::Collapsed)
			// Background
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 3)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				//.IsEnabled(CategorySetup.bUseCustomColorCategories)
				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Background Color")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]

				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color with alpha unless it is ignored
						SAssignNew(BackgroundColorBox, SColorBlock)
						.Color(this, &SCategoryEditDialog::GetBackgroundColor)
						.ShowBackgroundForAlpha(true)
						.IgnoreAlpha(false)
						.OnMouseButtonDown(this, &SCategoryEditDialog::OnMouseButtonDownBackgroundColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
						//.IsEnabled(this, &FColorStructCustomization::IsValueEnabled, StructWeakHandlePtr)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color without alpha
						SNew(SColorBlock)
						.Color(this, &SCategoryEditDialog::GetBackgroundColor)
						.ShowBackgroundForAlpha(false)
						.IgnoreAlpha(true)
						.OnMouseButtonDown(this, &SCategoryEditDialog::OnMouseButtonDownBackgroundColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
					]
				]
			]
			// Hovered
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.Padding(0, 3)
			[
				SNew(SHorizontalBox)
				//.IsEnabled(CategorySetup.bUseCustomColorCategories)
				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Hovered Color")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]

				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color with alpha unless it is ignored
						SAssignNew(HoveredColorBox ,SColorBlock)
						.Color(this, &SCategoryEditDialog::GetHoveredColor)
						.ShowBackgroundForAlpha(true)
						.IgnoreAlpha(false)
						.OnMouseButtonDown(this, &SCategoryEditDialog::OnMouseButtonDownHoveredColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
						//.IsEnabled(this, &FColorStructCustomization::IsValueEnabled, StructWeakHandlePtr)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color without alpha
						SNew(SColorBlock)
						.Color(this, &SCategoryEditDialog::GetHoveredColor)
						.ShowBackgroundForAlpha(false)
						.IgnoreAlpha(true)
						.OnMouseButtonDown(this, &SCategoryEditDialog::OnMouseButtonDownHoveredColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
					]
				]
			]
			// Pressed
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.Padding(0, 3)
			[
				SNew(SHorizontalBox)
				//.IsEnabled(CategorySetup.bUseCustomColorCategories)
				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Pressed Color")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]

				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color with alpha unless it is ignored
						SAssignNew(PressedColorBox, SColorBlock)
						.Color(this, &SCategoryEditDialog::GetPressedColor)
						.ShowBackgroundForAlpha(true)
						.IgnoreAlpha(false)
						.OnMouseButtonDown(this, &SCategoryEditDialog::OnMouseButtonDownPressedColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
						//.IsEnabled(this, &FColorStructCustomization::IsValueEnabled, StructWeakHandlePtr)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color without alpha
						SNew(SColorBlock)
						.Color(this, &SCategoryEditDialog::GetPressedColor)
						.ShowBackgroundForAlpha(false)
						.IgnoreAlpha(true)
						.OnMouseButtonDown(this, &SCategoryEditDialog::OnMouseButtonDownPressedColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
					]
				]
			]
			// Text Color
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.Padding(0, 3)
			[
				SNew(SHorizontalBox)
				//.IsEnabled(CategorySetup.bUseCustomColorCategories)
				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Text color")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]

				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color with alpha unless it is ignored
						SAssignNew(TextColorBox,SColorBlock)
						.Color(this, &SCategoryEditDialog::GetTextColor)
						.ShowBackgroundForAlpha(true)
						.IgnoreAlpha(false)
						.OnMouseButtonDown(this, &SCategoryEditDialog::OnMouseButtonDownTextColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
						//.IsEnabled(this, &FColorStructCustomization::IsValueEnabled, StructWeakHandlePtr)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color without alpha
						SNew(SColorBlock)
						.Color(this, &SCategoryEditDialog::GetTextColor)
						.ShowBackgroundForAlpha(false)
						.IgnoreAlpha(true)
						.OnMouseButtonDown(this, &SCategoryEditDialog::OnMouseButtonDownTextColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
					]
				]
			]
			// Text Shadow Color
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.Padding(0, 3)
			[
				SNew(SHorizontalBox)
				//.IsEnabled(CategorySetup.bUseCustomColorCategories)
				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Text shadow color")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]

				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color with alpha unless it is ignored
						SAssignNew(TextShadowColorBox,SColorBlock)
						.Color(this, &SCategoryEditDialog::GetTextShadowColor)
						.ShowBackgroundForAlpha(true)
						.IgnoreAlpha(false)
						.OnMouseButtonDown(this, &SCategoryEditDialog::OnMouseButtonDownTextShadowColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
						//.IsEnabled(this, &FColorStructCustomization::IsValueEnabled, StructWeakHandlePtr)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color without alpha
						SNew(SColorBlock)
						.Color(this, &SCategoryEditDialog::GetTextShadowColor)
						.ShowBackgroundForAlpha(false)
						.IgnoreAlpha(true)
						.OnMouseButtonDown(this, &SCategoryEditDialog::OnMouseButtonDownTextShadowColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
					]
				]
			]
			+ SVerticalBox::Slot()
			.Padding(3, 3)
			[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Fill)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("Checkerboard"))
					]
					+ SOverlay::Slot()
					.Padding(10,10)
					[
						SAssignNew(PreviewTab, SCheckBox)
						.Style(FEditorStyle::Get(), "PlacementBrowser.Tab")
						.UncheckedImage(BackgroundImage)
						.UncheckedHoveredImage(HoveredImage)
						.UncheckedPressedImage(BackgroundPressedImage)
						.CheckedImage(BackgroundPressedImage)
						.CheckedHoveredImage(BackgroundPressedImage)
						.CheckedPressedImage(BackgroundPressedImage)
						//.Style(TabStyle)
						//.IsEnabled(CategorySetup.bUseCustomColorCategories)
						//.OnCheckStateChanged(this, &SCustomPlacementModeTools::OnPlacementTabChanged, Info.UniqueHandle)
						//.IsChecked(this, &SCustomPlacementModeTools::GetPlacementTabCheckedState, Info.UniqueHandle)
						[
							SNew(SOverlay)

							+ SOverlay::Slot()
								.VAlign(VAlign_Center)
								[
									SNew(SSpacer)
									.Size(FVector2D(1, 40))
								]

							+ SOverlay::Slot()
								.Padding(FMargin(6, 0, 15, 0))
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.TextStyle(FEditorStyle::Get(), "PlacementBrowser.Tab.Text")
									.ColorAndOpacity(this, &SCategoryEditDialog::GetSlateTextColor)
									.ShadowColorAndOpacity(/*CategorySetup.ColorSetting.TextShadowColor*/ this, &SCategoryEditDialog::GetTextShadowColor)
									.Text(this, &SCategoryEditDialog::GetName)
								]

							+ SOverlay::Slot()
								.VAlign(VAlign_Fill)
								.HAlign(HAlign_Left)
								[
									SNew(SImage)
									.Image(FEditorStyle::GetBrush("PlacementBrowser.ActiveTabBar"))
								]
						]
					]
			]
		]

		// accept or cancel button
		+ SVerticalBox::Slot()
		//.AutoHeight()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(0, 3, 0, 3)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
			
			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.Text(LOCTEXT("SCategoryEditDialog_Accept", "Accept"))
				.OnClicked(this, &SCategoryEditDialog::OnAccept)
				.IsEnabled(this, &SCategoryEditDialog::IsAcceptAvailable)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
				.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.Text(LOCTEXT("SCategoryEditDialog_Cancel", "Cancel"))
				.OnClicked(this, &SCategoryEditDialog::OnCancel)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
				.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
			]
		]
	];
}

void SCategoryEditDialog::OnSetClass(const UClass* NewClass)
{
	SelectedClass = NewClass;
}

FReply SCategoryEditDialog::OnMouseButtonDownBackgroundColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FColorPickerArgs PickerArgs;
		{
			PickerArgs.bUseAlpha = true;
			PickerArgs.bOnlyRefreshOnMouseUp = false;
			PickerArgs.bOnlyRefreshOnOk = false;
			PickerArgs.sRGBOverride = true;
			PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
			PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &SCategoryEditDialog::OnSetBackgroundColorFromColorPicker);
			PickerArgs.OnColorPickerCancelled = FOnColorPickerCancelled::CreateSP(this, &SCategoryEditDialog::OnBackgroundColorPickerCancelled);
			PickerArgs.OnInteractivePickBegin = FSimpleDelegate::CreateSP(this, &SCategoryEditDialog::OnBackgroundColorPickerInteractiveBegin);
			PickerArgs.OnInteractivePickEnd = FSimpleDelegate::CreateSP(this, &SCategoryEditDialog::OnBackgroundColorPickerInteractiveEnd);
			PickerArgs.InitialColorOverride = CategorySetup.ColorSetting.BackgroundColor;
			PickerArgs.ParentWidget = BackgroundColorBox;
			PickerArgs.OptionalOwningDetailsView = BackgroundColorBox;
			FWidgetPath ParentWidgetPath;
			if (FSlateApplication::Get().FindPathToWidget(BackgroundColorBox.ToSharedRef(), ParentWidgetPath))
			{
				PickerArgs.bOpenAsMenu = FSlateApplication::Get().FindMenuInWidgetPath(ParentWidgetPath).IsValid();
			}
		}

		OpenColorPicker(PickerArgs);
	}
	return FReply::Handled();
}

void SCategoryEditDialog::OnSetBackgroundColorFromColorPicker(FLinearColor NewColor)
{
	CategorySetup.ColorSetting.BackgroundColor = NewColor;

	BackgroundImage->TintColor = CategorySetup.ColorSetting.BackgroundColor;

}

void SCategoryEditDialog::OnBackgroundColorPickerCancelled(FLinearColor OriginalColor)
{
	CategorySetup.ColorSetting.BackgroundColor = OriginalColor;

	BackgroundImage->TintColor = CategorySetup.ColorSetting.BackgroundColor;
}

void SCategoryEditDialog::OnBackgroundColorPickerInteractiveBegin()
{
	GEditor->BeginTransaction(LOCTEXT("SetColorProperty", "Edit"));
}

void SCategoryEditDialog::OnBackgroundColorPickerInteractiveEnd()
{
	GEditor->EndTransaction();
}

FLinearColor SCategoryEditDialog::GetBackgroundColor() const
{
	return CategorySetup.ColorSetting.BackgroundColor;
}

FReply SCategoryEditDialog::OnMouseButtonDownHoveredColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FColorPickerArgs PickerArgs;
		{
			PickerArgs.bUseAlpha = true;
			PickerArgs.bOnlyRefreshOnMouseUp = false;
			PickerArgs.bOnlyRefreshOnOk = false;
			PickerArgs.sRGBOverride = true;
			PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
			PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &SCategoryEditDialog::OnSetHoveredColorFromColorPicker);
			PickerArgs.OnColorPickerCancelled = FOnColorPickerCancelled::CreateSP(this, &SCategoryEditDialog::OnHoveredColorPickerCancelled);
			PickerArgs.OnInteractivePickBegin = FSimpleDelegate::CreateSP(this, &SCategoryEditDialog::OnBackgroundColorPickerInteractiveBegin);
			PickerArgs.OnInteractivePickEnd = FSimpleDelegate::CreateSP(this, &SCategoryEditDialog::OnBackgroundColorPickerInteractiveEnd);
			PickerArgs.InitialColorOverride = CategorySetup.ColorSetting.HoveredColor;
			PickerArgs.ParentWidget = HoveredColorBox;
			PickerArgs.OptionalOwningDetailsView = HoveredColorBox;
			FWidgetPath ParentWidgetPath;
			if (FSlateApplication::Get().FindPathToWidget(HoveredColorBox.ToSharedRef(), ParentWidgetPath))
			{
				PickerArgs.bOpenAsMenu = FSlateApplication::Get().FindMenuInWidgetPath(ParentWidgetPath).IsValid();
			}
		}

		OpenColorPicker(PickerArgs);
	}
	return FReply::Handled();
}

void SCategoryEditDialog::OnSetHoveredColorFromColorPicker(FLinearColor NewColor)
{
	CategorySetup.ColorSetting.HoveredColor = NewColor;

	HoveredImage->TintColor = CategorySetup.ColorSetting.HoveredColor;
}

void SCategoryEditDialog::OnHoveredColorPickerCancelled(FLinearColor OriginalColor)
{
	CategorySetup.ColorSetting.HoveredColor = OriginalColor;

	HoveredImage->TintColor = CategorySetup.ColorSetting.HoveredColor;
}

FLinearColor SCategoryEditDialog::GetHoveredColor() const
{
	return CategorySetup.ColorSetting.HoveredColor;
}

FReply SCategoryEditDialog::OnMouseButtonDownPressedColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FColorPickerArgs PickerArgs;
		{
			PickerArgs.bUseAlpha = true;
			PickerArgs.bOnlyRefreshOnMouseUp = false;
			PickerArgs.bOnlyRefreshOnOk = false;
			PickerArgs.sRGBOverride = true;
			PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
			PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &SCategoryEditDialog::OnSetPressedColorFromColorPicker);
			PickerArgs.OnColorPickerCancelled = FOnColorPickerCancelled::CreateSP(this, &SCategoryEditDialog::OnPressedColorPickerCancelled);
			PickerArgs.OnInteractivePickBegin = FSimpleDelegate::CreateSP(this, &SCategoryEditDialog::OnBackgroundColorPickerInteractiveBegin);
			PickerArgs.OnInteractivePickEnd = FSimpleDelegate::CreateSP(this, &SCategoryEditDialog::OnBackgroundColorPickerInteractiveEnd);
			PickerArgs.InitialColorOverride = CategorySetup.ColorSetting.PressedSelectionColor;
			PickerArgs.ParentWidget = PressedColorBox;
			PickerArgs.OptionalOwningDetailsView = PressedColorBox;
			FWidgetPath ParentWidgetPath;
			if (FSlateApplication::Get().FindPathToWidget(PressedColorBox.ToSharedRef(), ParentWidgetPath))
			{
				PickerArgs.bOpenAsMenu = FSlateApplication::Get().FindMenuInWidgetPath(ParentWidgetPath).IsValid();
			}
		}

		OpenColorPicker(PickerArgs);
	}
	return FReply::Handled();
}

void SCategoryEditDialog::OnSetPressedColorFromColorPicker(FLinearColor NewColor)
{
	CategorySetup.ColorSetting.PressedSelectionColor = NewColor;

	BackgroundPressedImage->TintColor = CategorySetup.ColorSetting.PressedSelectionColor;
}

void SCategoryEditDialog::OnPressedColorPickerCancelled(FLinearColor OriginalColor)
{
	CategorySetup.ColorSetting.PressedSelectionColor = OriginalColor;

	BackgroundPressedImage->TintColor = CategorySetup.ColorSetting.PressedSelectionColor;
}

FLinearColor SCategoryEditDialog::GetPressedColor() const
{
	return CategorySetup.ColorSetting.PressedSelectionColor;
}

FReply SCategoryEditDialog::OnMouseButtonDownTextColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FColorPickerArgs PickerArgs;
		{
			PickerArgs.bUseAlpha = true;
			PickerArgs.bOnlyRefreshOnMouseUp = false;
			PickerArgs.bOnlyRefreshOnOk = false;
			PickerArgs.sRGBOverride = true;
			PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
			PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &SCategoryEditDialog::OnSetTextColorFromColorPicker);
			PickerArgs.OnColorPickerCancelled = FOnColorPickerCancelled::CreateSP(this, &SCategoryEditDialog::OnTextColorPickerCancelled);
			PickerArgs.OnInteractivePickBegin = FSimpleDelegate::CreateSP(this, &SCategoryEditDialog::OnBackgroundColorPickerInteractiveBegin);
			PickerArgs.OnInteractivePickEnd = FSimpleDelegate::CreateSP(this, &SCategoryEditDialog::OnBackgroundColorPickerInteractiveEnd);
			PickerArgs.InitialColorOverride = CategorySetup.ColorSetting.TextColor;
			PickerArgs.ParentWidget = TextColorBox;
			PickerArgs.OptionalOwningDetailsView = TextColorBox;
			FWidgetPath ParentWidgetPath;
			if (FSlateApplication::Get().FindPathToWidget(TextColorBox.ToSharedRef(), ParentWidgetPath))
			{
				PickerArgs.bOpenAsMenu = FSlateApplication::Get().FindMenuInWidgetPath(ParentWidgetPath).IsValid();
			}
		}

		OpenColorPicker(PickerArgs);
	}
	return FReply::Handled();
}

void SCategoryEditDialog::OnSetTextColorFromColorPicker(FLinearColor NewColor)
{
	CategorySetup.ColorSetting.TextColor = NewColor;
}

void SCategoryEditDialog::OnTextColorPickerCancelled(FLinearColor OriginalColor)
{
	CategorySetup.ColorSetting.TextColor = OriginalColor;
}


FReply SCategoryEditDialog::OnMouseButtonDownTextShadowColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		FColorPickerArgs PickerArgs;
		{
			PickerArgs.bUseAlpha = true;
			PickerArgs.bOnlyRefreshOnMouseUp = false;
			PickerArgs.bOnlyRefreshOnOk = false;
			PickerArgs.sRGBOverride = true;
			PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
			PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &SCategoryEditDialog::OnSetTextShadowColorFromColorPicker);
			PickerArgs.OnColorPickerCancelled = FOnColorPickerCancelled::CreateSP(this, &SCategoryEditDialog::OnTextShadowColorPickerCancelled);
			PickerArgs.OnInteractivePickBegin = FSimpleDelegate::CreateSP(this, &SCategoryEditDialog::OnBackgroundColorPickerInteractiveBegin);
			PickerArgs.OnInteractivePickEnd = FSimpleDelegate::CreateSP(this, &SCategoryEditDialog::OnBackgroundColorPickerInteractiveEnd);
			PickerArgs.InitialColorOverride = CategorySetup.ColorSetting.TextShadowColor;
			PickerArgs.ParentWidget = TextShadowColorBox;
			PickerArgs.OptionalOwningDetailsView = TextShadowColorBox;
			FWidgetPath ParentWidgetPath;
			if (FSlateApplication::Get().FindPathToWidget(TextShadowColorBox.ToSharedRef(), ParentWidgetPath))
			{
				PickerArgs.bOpenAsMenu = FSlateApplication::Get().FindMenuInWidgetPath(ParentWidgetPath).IsValid();
			}
		}

		OpenColorPicker(PickerArgs);
	}
	return FReply::Handled();
}


void SCategoryEditDialog::OnSetTextShadowColorFromColorPicker(FLinearColor NewColor)
{
	CategorySetup.ColorSetting.TextShadowColor = NewColor;
}


void SCategoryEditDialog::OnTextShadowColorPickerCancelled(FLinearColor OriginalColor)
{
	CategorySetup.ColorSetting.TextShadowColor = OriginalColor;
}

FLinearColor SCategoryEditDialog::GetTextShadowColor() const
{
	return CategorySetup.ColorSetting.TextShadowColor;
}

FSlateColor SCategoryEditDialog::GetSlateTextColor() const
{
	return FSlateColor(CategorySetup.ColorSetting.TextColor);
}

FLinearColor SCategoryEditDialog::GetTextColor() const
{
	return CategorySetup.ColorSetting.TextColor;
}

bool SCategoryEditDialog::IsAcceptAvailable() const
{
	return (CategorySetup.CategoryName != NAME_None && CategorySetup.CategoryName.ToString().Find(TEXT(" ")) == INDEX_NONE && FString(CategorySetup.CategoryName.ToString()).Len() <= 15 && FString(CategorySetup.CategoryName.ToString()).Len() > 0 && CategorySetup.CategoryName != "" && CategorySetup.CategoryName != "RecentlyPlaced" && CategorySetup.CategoryName != "Basic");
}

FReply SCategoryEditDialog::OnAccept()
{
	if (OnValidateCategory.IsBound())
	{
		if (OnValidateCategory.Execute(&CategorySetup, CategoryIndex))
		{
			bApplyChange = true;
			CloseWindow();
		}
		else
		{
			// invalid setup
			FMessageDialog::Open(EAppMsgType::Ok, LOCTEXT("SCategoryEditDialog_InvalidAccept", "Duplicate Name found. Enter correct Category Name"));
		}
	}
	else
	{
		// no validate test, just accept
		CloseWindow();
	}

	return FReply::Handled();
}

FReply SCategoryEditDialog::OnCancel()
{
	CloseWindow();
	return FReply::Handled();
}

void SCategoryEditDialog::CloseWindow()
{
	if (WidgetWindow.IsValid())
	{
		WidgetWindow.Pin()->RequestDestroyWindow();
	}
}

//FReply SCategoryEditDialog::onPickedClass()
//{
//
//	//FClassViewerInitializationOptions InitOptions;
//	//InitOptions.Mode = EClassViewerMode::ClassBrowsing;
//	//InitOptions.DisplayMode = EClassViewerDisplayMode::TreeView;
//
//
//	// Load the Class Viewer module to display a class picker
//	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
//
//	FClassViewerInitializationOptions Options;
//	Options.Mode = EClassViewerMode::ClassPicker;
//	Options.DisplayMode = EClassViewerDisplayMode::DefaultView;
//	TSharedPtr<FAssetClassParentFilter> Filter = MakeShareable(new FAssetClassParentFilter);
//	Options.ClassFilter = Filter;
//
//	Filter->DisallowedClassFlags = CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists ^ CLASS_NotPlaceable;
//	Filter->AllowedChildrenOfClasses.Add(AActor::StaticClass());
//
//	const FText TitleText = LOCTEXT("PickActorClassOptions", "Pick Actor Class");
//	UClass* ChosenClass = nullptr;
//	const bool bPressedOk = SClassPickerDialog::PickClass(TitleText, Options, ChosenClass, AActor::StaticClass());
//
//	if (bPressedOk)
//	{
//		ClassChosenClass = ChosenClass;
//	}
//
//	return FReply::Handled();
//}

TSharedRef<ITableRow> SCategoryEditDialog::HandleGenerateClassWidget(TSharedPtr< FClassListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SClassListItem, OwnerTable)
		.ClassSetup(InItem->ClassSetup)
		.ClassIndex(InItem->ClassIndex);
}

void SCategoryEditDialog::OnClassListItemDoubleClicked(TSharedPtr< FClassListItem > SelectedItem)
{
	// Load the class viewer module to display a class picker
	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

	const FText TitleText = LOCTEXT("PickActorClassOptions", "Pick Actor Class");

	FClassViewerInitializationOptions Options;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = true; 

	TSharedPtr<FCustomPropertyEditorClassFilter> ClassFilter = MakeShareable(new FCustomPropertyEditorClassFilter);
	Options.ClassFilter = ClassFilter;
	ClassFilter->ClassPropertyMetaClass = Settings->ParentClass; //UObject::StaticClass();
	ClassFilter->InterfaceThatMustBeImplemented = nullptr;
	ClassFilter->bAllowAbstract = Settings->bAllowAbstract;
	Options.bIsBlueprintBaseOnly = false;
	Options.bIsPlaceableOnly = true;
	Options.DisplayMode = Settings->bUseDisplayTree ? EClassViewerDisplayMode::TreeView : EClassViewerDisplayMode::ListView;
	Options.bAllowViewOptions = true;

	FOnClassPicked OnPicked(FOnClassPicked::CreateRaw(this, &SCategoryEditDialog::OnClassPicked));


	//TSharedPtr<SPropertyEditorClass> PropertyEditorClass;

	// Create the window to pick the class
	SAssignNew(PickerWindow,SWindow)
		.Title(TitleText)
		.Type(EWindowType::Normal)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SizingRule(ESizingRule::Autosized)
		//.ClientSize(FVector2D(0.f, 300.f))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	TSharedRef<SWidget> ClassPickerDialog = SNew(SBox)
		.WidthOverride(280)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			.MaxHeight(500)
			[
				FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer").CreateClassViewer(Options, OnPicked)
			]
		];


	PickerWindow->SetContent(ClassPickerDialog);

	GEditor->EditorAddModalWindow(PickerWindow.ToSharedRef());
	
	//FSlateApplication::Get().AddWindow( PickerWindow.ToSharedRef(), true );
}

void SCategoryEditDialog::RefreshClassList()
{
	ObjectClassList.Empty();
	int i = 0;

	for (auto Iter = CategorySetup.Items.CreateIterator(); Iter; ++Iter)
	{
		ObjectClassList.Add(MakeShareable(new FClassListItem(MakeShareable(new TSubclassOf<AActor>(*Iter)),MakeShareable(new int32(i)))));
		i++;
	}
	Algo::Reverse(ObjectClassList);
}

FReply SCategoryEditDialog::OnNewClassItem()
{
	// Load the class viewer module to display a class picker
	FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");

	const FText TitleText = LOCTEXT("PickActorClassOptions", "Pick Actor Class");

	FClassViewerInitializationOptions Options;
	Options.bShowUnloadedBlueprints = true;
	Options.bShowNoneOption = true;

	TSharedPtr<FCustomPropertyEditorClassFilter> ClassFilter = MakeShareable(new FCustomPropertyEditorClassFilter);
	Options.ClassFilter = ClassFilter;
	ClassFilter->ClassPropertyMetaClass = Settings->ParentClass;//APawn::StaticClass();
	ClassFilter->InterfaceThatMustBeImplemented = nullptr;
	ClassFilter->bAllowAbstract = Settings->bAllowAbstract;
	Options.bIsBlueprintBaseOnly = false;
	Options.bIsPlaceableOnly = true;
	Options.DisplayMode = Settings->bUseDisplayTree ? EClassViewerDisplayMode::TreeView : EClassViewerDisplayMode::ListView;
	Options.bAllowViewOptions = true;

	FOnClassPicked OnPicked(FOnClassPicked::CreateRaw(this, &SCategoryEditDialog::OnNewClassPicked));


	//TSharedPtr<SPropertyEditorClass> PropertyEditorClass;

	// Create the window to pick the class
	SAssignNew(PickerWindow, SWindow)
		.Title(TitleText)
		.Type(EWindowType::Normal)
		.AutoCenter(EAutoCenter::PreferredWorkArea)
		.SizingRule(ESizingRule::Autosized)
		//.ClientSize(FVector2D(0.f, 300.f))
		.SupportsMaximize(false)
		.SupportsMinimize(false);

	TSharedRef<SWidget> ClassPickerDialog = SNew(SBox)
		.WidthOverride(280)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.AutoHeight()
		.MaxHeight(500)
		[
			FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer").CreateClassViewer(Options, OnPicked)
		]
		];


	PickerWindow->SetContent(ClassPickerDialog);

	GEditor->EditorAddModalWindow(PickerWindow.ToSharedRef());

	return FReply::Handled();
}

void SCategoryEditDialog::OnNewClassPicked(UClass* InClass)
{
	FNotificationInfo DublicateSetCategoryName(NSLOCTEXT("CustomPlacementMode", "DublicateCategory", "Cannot add new element to the set while an element with the default value exists"));
	DublicateSetCategoryName.Image = FEditorStyle::GetBrush(TEXT("NotificationList.DefaultMessage"));
	DublicateSetCategoryName.FadeInDuration = 0.1f;
	DublicateSetCategoryName.FadeOutDuration = 0.5f;
	DublicateSetCategoryName.ExpireDuration = 1.5f;
	DublicateSetCategoryName.bUseThrobber = false;
	DublicateSetCategoryName.bUseSuccessFailIcons = true;
	DublicateSetCategoryName.bUseLargeFont = true;
	DublicateSetCategoryName.bFireAndForget = true;
	DublicateSetCategoryName.bAllowThrottleWhenFrameRateIsLow = false;

	if (!InClass)
	{
		SendToObjects(TEXT("None"));
	}
	else
	{
		UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("Picked Class - %s"), *InClass->GetPathName());
		SendToObjects(InClass->GetPathName());
	}


	TSubclassOf<AActor> Actor = TSubclassOf<AActor>(InClass);

	TSet<TSubclassOf<AActor>> OldItems;

	for (auto Iter : CategorySetup.Items)
	{
		OldItems.Add(Iter);
	}

	// Founded New Class Item from array in current category
	TSubclassOf<AActor>* SearchItem = CategorySetup.Items.Find(Actor.Get());

	PickerWindow->RequestDestroyWindow();
	if (!SearchItem)
	{
		FNotificationInfo Info(NSLOCTEXT("CustomPlacementMode", "ItemsChange", "All problem items has been resolved"));
		Info.Image = FEditorStyle::GetBrush(TEXT("Cascade.AddLODBeforeCurrent"));
		Info.FadeInDuration = 0.1f;
		Info.FadeOutDuration = 0.5f;
		Info.ExpireDuration = 1.5f;
		Info.bUseThrobber = false;
		Info.bUseSuccessFailIcons = true;
		Info.bUseLargeFont = true;
		Info.bFireAndForget = false;
		Info.bAllowThrottleWhenFrameRateIsLow = false;
		TSharedPtr<SNotificationItem> NotificationCategoryItem;

		CategorySetup.Items.Add(Actor);

		if (CategorySetup.Items.Num() > 0)
		{
			FString Message;
			FMessageDialog* DialogMsg = nullptr;
			TSet<TSubclassOf<AActor>> NeedForDelete;

			int separateindex = 0;
			FString ParentActor;
			for (auto& Item : CategorySetup.Items)
			{
				for (auto& SortedItem : CategorySetup.Items)
				{
					if (Item != SortedItem)
					{
						if (Item->IsChildOf(SortedItem))
						{
							TArray<FStringFormatArg> Arguments;
							Arguments.Add(FStringFormatArg(GetClassDisplayName(Item)));
							Arguments.Add(FStringFormatArg(GetClassDisplayName(SortedItem)));
							ParentActor = GetClassDisplayName(SortedItem);

							if (separateindex < 5)
							{
								Message += FString::Format(TEXT("{0}, "), Arguments);
							}
							else
							{
								Message += FString::Format(TEXT("{0}"), Arguments);
								separateindex = 0;
							}

							separateindex++;
							//Message += FString::Format(TEXT("{0} is a child of {1}\n"), Arguments);
							NeedForDelete.Add(Item);
						}
					}
				}
			}

			//is a child of {1}
			if (!Message.IsEmpty())
			{
				Message.RemoveFromEnd(TEXT(", "), ESearchCase::CaseSensitive);

				TArray<FStringFormatArg> SeparateArguments;
				SeparateArguments.Add(FStringFormatArg(ParentActor));
				Message += FString::Format(TEXT(" are child classes of {0}"), SeparateArguments);
				Message += TEXT(" \nThese child classes will be deleted. Would you like to continue?");

				if (!DialogMsg)
				{
					DialogMsg = new FMessageDialog();
					FText Title = FText::FromString(TEXT("Error"));
					FText MessageText = FText::FromString(Message);
					if (DialogMsg->Open(EAppMsgType::OkCancel, MessageText, &Title) == EAppReturnType::Ok)
					{
						for (auto& ActorItem : NeedForDelete)
						{
							CategorySetup.Items.Remove(ActorItem);
							CategorySetup.Items.CompactStable();
							CategorySetup.Items.Shrink();

							if (!NotificationCategoryItem)
							{
								NotificationCategoryItem = FSlateNotificationManager::Get().AddNotification(Info);
							}

							NotificationCategoryItem->SetCompletionState(SNotificationItem::CS_Success);
							NotificationCategoryItem->ExpireAndFadeout();
						}
					}
					else
					{
						CategorySetup.Items = OldItems;

						if (!NotificationCategoryItem)
						{
							NotificationCategoryItem = FSlateNotificationManager::Get().AddNotification(Info);
						}

						NotificationCategoryItem->SetCompletionState(SNotificationItem::CS_Success);
						NotificationCategoryItem->ExpireAndFadeout();
					}
				}
			}
		}
	}
	else
	{
		if (!NotificationSetAddDublicateCategoryName)
		{
			NotificationSetAddDublicateCategoryName = FSlateNotificationManager::Get().AddNotification(DublicateSetCategoryName);
		}

		else
		{
			if (!NotificationSetAddDublicateCategoryName->IsParentValid())
			{
				NotificationSetAddDublicateCategoryName = FSlateNotificationManager::Get().AddNotification(DublicateSetCategoryName);
			}
		}
	}

	RefreshClassList();
	ObjectClassListView->RequestListRefresh();

	if (ObjectClassList.Num() > 0)
	{
		ObjectClassListView->SetItemSelection(ObjectClassList[0], true);
	}
}

FReply SCategoryEditDialog::OnRefreshClassItem()
{
	RefreshClassList();
	return FReply::Handled();
}

bool SCategoryEditDialog::IsClassItemsFill() const
{
	if (ObjectClassList.Num() > 0)
	{
		return true;
	}

	return false;
}

FReply SCategoryEditDialog::OnDeleteClassItem()
{
	TArray< TSharedPtr< FClassListItem > > SelectedItems = ObjectClassListView->GetSelectedItems();

	if (SelectedItems.Num() == 1)
	{
		FText Title = LOCTEXT("FClassDetails_DeleteClassItemTitle", "Delete class item from list");

		if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("FClassDetails_DeleteClassItem", "If you delete this class item, all child classes will be removed from the current category. \nWould you like to continue?"),&Title) == EAppReturnType::Yes)
		{
			TSharedPtr< FClassListItem > SelectedItem = SelectedItems[0];

			UClass* Class = SelectedItem->ClassSetup->Get();

			CategorySetup.Items.Remove(TSubclassOf<AActor>(Class));
			CategorySetup.Items.CompactStable();
			CategorySetup.Items.Shrink();
			RefreshClassList();
		}
	}
	return FReply::Handled();
}

bool SCategoryEditDialog::IsAnyClassItemSelected() const
{
	return ObjectClassListView->GetNumItemsSelected() > 0;
}

FReply SCategoryEditDialog::OnDeleteClassItems()
{
	if (ObjectClassList.Num() > 0)
	{
		FText Title = LOCTEXT("FClassItemDetails_DeleteClassItemsTitle", "Clear class item list");

		if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("FClassItemDetails_DeleteClassItems", "All classes will be removed from the current category. \nWould you like to continue?"), &Title) == EAppReturnType::Yes)
		{
			CategorySetup.Items.Empty();
			CategorySetup.Items.CompactStable();
			CategorySetup.Items.Shrink();
			RefreshClassList();
		}
	}
	return FReply::Handled();
}

void SCategoryEditDialog::OnClassPicked(UClass* InClass)
{
	FNotificationInfo DublicateSetCategoryName(NSLOCTEXT("CustomPlacementMode", "DublicateCategory", "Cannot add new element to the set while an element with the default value exists"));
	DublicateSetCategoryName.Image = FEditorStyle::GetBrush(TEXT("NotificationList.DefaultMessage"));
	DublicateSetCategoryName.FadeInDuration = 0.1f;
	DublicateSetCategoryName.FadeOutDuration = 0.5f;
	DublicateSetCategoryName.ExpireDuration = 1.5f;
	DublicateSetCategoryName.bUseThrobber = false;
	DublicateSetCategoryName.bUseSuccessFailIcons = true;
	DublicateSetCategoryName.bUseLargeFont = true;
	DublicateSetCategoryName.bFireAndForget = true;
	DublicateSetCategoryName.bAllowThrottleWhenFrameRateIsLow = false;

	if (!InClass)
	{
		SendToObjects(TEXT("None"));
	}
	else
	{
		UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("Picked Class - %s"), *InClass->GetPathName());
		SendToObjects(InClass->GetPathName());
	}


	TArray< TSharedPtr< FClassListItem > > SelectedItems = ObjectClassListView->GetSelectedItems();

	int index = 0;
	if (SelectedItems.Num() == 1)
	{
		TSharedPtr< FClassListItem > SelectedItem = SelectedItems[0];
		TSubclassOf<AActor> Actor = TSubclassOf<AActor>(InClass);
		UClass* OldClass = *SelectedItem->ClassSetup.Get();
		
		TSet<TSubclassOf<AActor>> OldItems;

		int32 i = 0;
		for (auto Iter : CategorySetup.Items)
		{
			OldItems.Add(Iter);
			if (Iter == *SelectedItem->ClassSetup)
			{
				index = i;
			}
			i++;
		}

		// Founded Class Item from array in current category
		TSubclassOf<AActor>* ExistItem = CategorySetup.Items.Find(SelectedItem->ClassSetup->Get());
		
		// Founded New Class Item from array in current category
		TSubclassOf<AActor>* SearchItem = CategorySetup.Items.Find(Actor.Get());

		PickerWindow->RequestDestroyWindow();
		if (!SearchItem)
		{
			if (ExistItem)
			{
				FNotificationInfo Info(NSLOCTEXT("CustomPlacementMode", "ItemsChange", "All problem items has been resolved"));
				Info.Image = FEditorStyle::GetBrush(TEXT("Cascade.AddLODBeforeCurrent"));
				Info.FadeInDuration = 0.1f;
				Info.FadeOutDuration = 0.5f;
				Info.ExpireDuration = 1.5f;
				Info.bUseThrobber = false;
				Info.bUseSuccessFailIcons = true;
				Info.bUseLargeFont = true;
				Info.bFireAndForget = false;
				Info.bAllowThrottleWhenFrameRateIsLow = false;
				TSharedPtr<SNotificationItem> NotificationCategoryItem;

				*ExistItem = Actor;

				if (CategorySetup.Items.Num() > 0)
				{
					FString Message;
					FMessageDialog* DialogMsg = nullptr;
					TSet<TSubclassOf<AActor>> NeedForDelete;

					for (auto& Item : CategorySetup.Items)
					{
						for (auto& SortedItem : CategorySetup.Items)
						{
							if (Item != SortedItem)
							{
								if (Item->IsChildOf(SortedItem))
								{
									TArray<FStringFormatArg> Arguments;
									Arguments.Add(FStringFormatArg(Item->GetName()));
									Arguments.Add(FStringFormatArg(SortedItem->GetName()));

									Message += FString::Format(TEXT("{0} is a child of {1}\n"), Arguments);
									NeedForDelete.Add(Item);
								}
							}
						}
					}

					if (!Message.IsEmpty())
					{
						Message += TEXT("All child Actors will be deleted\n");

						if (!DialogMsg)
						{
							DialogMsg = new FMessageDialog();
							FText Title = FText::FromString(TEXT("Error"));
							FText MessageText = FText::FromString(Message);
							if (DialogMsg->Open(EAppMsgType::OkCancel, MessageText, &Title) == EAppReturnType::Ok)
							{
								for (auto& ActorItem : NeedForDelete)
								{
									CategorySetup.Items.Remove(ActorItem);
									CategorySetup.Items.CompactStable();
									CategorySetup.Items.Shrink();

									if (!NotificationCategoryItem)
									{
										NotificationCategoryItem = FSlateNotificationManager::Get().AddNotification(Info);
									}

									NotificationCategoryItem->SetCompletionState(SNotificationItem::CS_Success);
									NotificationCategoryItem->ExpireAndFadeout();
								}
							}
							else
							{
								CategorySetup.Items = OldItems;

								if (!NotificationCategoryItem)
								{
									NotificationCategoryItem = FSlateNotificationManager::Get().AddNotification(Info);
								}

								NotificationCategoryItem->SetCompletionState(SNotificationItem::CS_Success);
								NotificationCategoryItem->ExpireAndFadeout();
							}
						}
					}
				}

			}
		}
		else if(*SearchItem != *SelectedItem->ClassSetup)
		{
			if (!NotificationSetAddDublicateCategoryName)
			{
				NotificationSetAddDublicateCategoryName = FSlateNotificationManager::Get().AddNotification(DublicateSetCategoryName);
			}

			else
			{
				if (!NotificationSetAddDublicateCategoryName->IsParentValid())
				{
					NotificationSetAddDublicateCategoryName = FSlateNotificationManager::Get().AddNotification(DublicateSetCategoryName);
				}
			}
		}

		RefreshClassList();
		ObjectClassListView->RequestListRefresh();

		if (ObjectClassList.Num() > 0)
		{
			if (ObjectClassList.Num() >= index + 1)
			{
				ObjectClassListView->SetItemSelection(ObjectClassList[index], true);
			}
			else
			{
				ObjectClassListView->SetItemSelection(ObjectClassList[ObjectClassList.Num()-1], true);
			}
		}
	}
}


FText SCategoryEditDialog::GetDisplayValueAsString() const
{
	static bool bIsReentrant = false;

	//Guard against re - entrancy which can happen if the delegate executed below(SelectedClass.Get()) forces a slow task dialog to open, thus causing this to lose context and regain focus later starting the loop over again
	if (!bIsReentrant)
	{
		TGuardValue<bool>(bIsReentrant, true);
		return FText::FromString(GetClassDisplayName(SelectedClass.Get()));
	}
	else
	{
		return FText::GetEmpty();
	}
}

void SCategoryEditDialog::SendToObjects(const FString& NewValue)
{
	UClass* NewClass = FindObject<UClass>(ANY_PACKAGE, *NewValue);
	if (!NewClass)
	{
		NewClass = LoadObject<UClass>(nullptr, *NewValue);
	}
	onSetClass.Execute(NewClass);
}

void SCategoryEditDialog::OnTextChanged(const FText& NewText)
{
	FString NewName = NewText.ToString();

	if (NewName.Len() == 0)
	{
		// no empty
		NameBox->SetError(LOCTEXT("CategoryNameValidationEmptyError", "Category name must not be empty"));
	}
	else if (NewName.Find(TEXT(" ")) != INDEX_NONE)
	{
		// no white space
		NameBox->SetError(LOCTEXT("CategoryNameValidationWhitespaceError", "No white space is allowed"));
	}
	else if (NewName.Len() > 15)
	{	
		NameBox->SetError(LOCTEXT("CategoryNameValidationCountCharError", "More than 15 characters in the category name are not allowed"));
	}
	else if (NewName == "RecentlyPlaced")
	{
		NameBox->SetError(LOCTEXT("CategoryNameValidationRecentlyError", "No RecentlyPlaced category is allowed"));
	}
	else if (NewName == "Basic")
	{	
		NameBox->SetError(LOCTEXT("CategoryNameValidationRecentlyError", "No Basic category is allowed"));
	}
	else
	{
		NameBox->SetError(FText::GetEmpty());
		NewNameEntered(NewText, ETextCommit::Default);
	}
}

void SCategoryEditDialog::OnCheckStateChangedBlueprintClassed(ECheckBoxState State)
{
	if(State == ECheckBoxState::Checked)
	{
		CategorySetup.bUseBlueprintClasses = true;
	}
	else
	{
		CategorySetup.bUseBlueprintClasses = false;
	}
}

void SCategoryEditDialog::OnCheckStateChangedBlueprintChildClassed(ECheckBoxState State)
{
	if(State == ECheckBoxState::Checked)
	{
		CategorySetup.bUseBlueprintChildClasses = true;
	}
	else
	{
		CategorySetup.bUseBlueprintChildClasses = false;
	}
}

void SCategoryEditDialog::OnCheckStateChangedCustomColorCategory(ECheckBoxState State)
{
	if (State == ECheckBoxState::Checked)
	{
		CategorySetup.bUseCustomColorCategories = true;
		//PreviewColorSection->SetEnabled(true);
		PreviewColorSection->SetVisibility(EVisibility::Visible);
	}
	else
	{
		CategorySetup.bUseCustomColorCategories = false;
		//PreviewColorSection->SetEnabled(false);
		PreviewColorSection->SetVisibility(EVisibility::Collapsed);
	}
}

void SCategoryEditDialog::NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo)
{
	{
		FName NewName = FName(*NewText.ToString());

		// we should accept NAME_None, that will invalidate "accept" button
		if (NewName != CategorySetup.CategoryName)
		{
			CategorySetup.CategoryName = NewName;
			NameBox->SetError(FText::GetEmpty());
		}
	}
}


FText SCategoryEditDialog::GetGuid() const
{
	if (!CategorySetup.CategoryID.IsValid())
	{
		return FText::GetEmpty();
	}

	return FText::FromString(CategorySetup.CategoryID.ToString());
}

FText SCategoryEditDialog::GetName() const
{
	if (CategorySetup.CategoryName == NAME_None)
	{
		return FText::GetEmpty();
	}

	return FText::FromName(CategorySetup.CategoryName);
}


void SDefaultCategoryEditDialog::Construct(const FArguments& InArgs)
{
	bApplyChange = false;

	if (InArgs._CategorySetup)
	{
		CategorySetup = *InArgs._CategorySetup;
	}
	if (InArgs._CategoryName != "")
	{
		CategoryName = InArgs._CategoryName;
	}
	if (InArgs._CategoryIndex)
	{
		CategoryIndex = InArgs._CategoryIndex;
	}

	Settings = InArgs._Settings;

	check(CategoryName != "");

	WidgetWindow = InArgs._WidgetWindow;


	FSlateFontInfo EditableFont = IDetailLayoutBuilder::GetDetailFont();
	EditableFont.Size = EditableFont.Size + 5;
	
	FSlateFontInfo EditableBoldFont = IDetailLayoutBuilder::GetDetailFontBold();
	EditableBoldFont.Size = EditableBoldFont.Size + 5;
	
	BackgroundImage = new FSlateBrush();
	BackgroundImage->TintColor = FSlateColor(CategorySetup.BackgroundColor);
	BackgroundImage->ImageSize = FVector2D(8.0f,32.0f);

	BackgroundPressedImage = new FSlateBrush();
	BackgroundPressedImage->TintColor = FSlateColor(CategorySetup.PressedSelectionColor);
	BackgroundPressedImage->ImageSize = FVector2D(8.0f, 32.0f);

	HoveredImage = new FSlateBrush();
	HoveredImage->TintColor = FSlateColor(CategorySetup.HoveredColor);
	HoveredImage->ImageSize = FVector2D(8.0f, 32.0f);


	

	this->ChildSlot
	[
		SNew(SVerticalBox)

		// Category Name EditBox
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(3.f, 5.f)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			//.FillWidth(1)
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SCategoryEditDialog_CategoryName", "Category Name"))
					.Font(EditableBoldFont)
				]
			]
			+ SHorizontalBox::Slot()
			//.AutoWidth()
			.HAlign(HAlign_Right)
			//.MaxWidth(250.0f)
			[
				SAssignNew(NameBox, STextBlock)
				//.MinDesiredWidth(250.f)
			.Text(this, &SDefaultCategoryEditDialog::GetName)
			.Font(EditableFont)
			]
		]
		// Category Visible
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(3.f, 10.f, 3.f, 5.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(3, 3)
			.FillWidth(1)
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SCategoryEditDialog_Visible", "Category visible"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked(CategorySetup.bVisible ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &SDefaultCategoryEditDialog::OnCheckStateChangedCategoryVisible)
			]
		]
		// Custom Category Color
		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(3.f, 10.f,3.f,5.f)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.Padding(3, 3)
			.FillWidth(1)
			.HAlign(HAlign_Fill)
			[
				SNew(SBox)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("SCategoryEditDialog_CustomColor", "Custom category color"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			]
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.HAlign(HAlign_Center)
			[
				SNew(SCheckBox)
				.IsChecked(CategorySetup.bUseCustomColor ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.OnCheckStateChanged(this, &SDefaultCategoryEditDialog::OnCheckStateChangedCustomColorCategory)
			]
		]

		+ SVerticalBox::Slot()
		.AutoHeight()
		.VAlign(VAlign_Center)
		.Padding(3,3)
		[
			SAssignNew(PreviewColorSection,SVerticalBox)
			//.IsEnabled(CategorySetup.bUseCustomColorCategories)
			.Visibility(CategorySetup.bUseCustomColor ? EVisibility::Visible : EVisibility::Collapsed)
			// Background
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(0, 3)
			.VAlign(VAlign_Center)
			[
				SNew(SHorizontalBox)
				//.IsEnabled(CategorySetup.bUseCustomColorCategories)
				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Background Color")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]

				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color with alpha unless it is ignored
						SAssignNew(BackgroundColorBox, SColorBlock)
						.Color(this, &SDefaultCategoryEditDialog::GetBackgroundColor)
						.ShowBackgroundForAlpha(true)
						.IgnoreAlpha(false)
						.OnMouseButtonDown(this, &SDefaultCategoryEditDialog::OnMouseButtonDownBackgroundColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
						//.IsEnabled(this, &FColorStructCustomization::IsValueEnabled, StructWeakHandlePtr)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color without alpha
						SNew(SColorBlock)
						.Color(this, &SDefaultCategoryEditDialog::GetBackgroundColor)
						.ShowBackgroundForAlpha(false)
						.IgnoreAlpha(true)
						.OnMouseButtonDown(this, &SDefaultCategoryEditDialog::OnMouseButtonDownBackgroundColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
					]
				]
			]
			// Hovered
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.Padding(0, 3)
			[
				SNew(SHorizontalBox)
				//.IsEnabled(CategorySetup.bUseCustomColorCategories)
				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Hovered Color")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]

				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color with alpha unless it is ignored
						SAssignNew(HoveredColorBox ,SColorBlock)
						.Color(this, &SDefaultCategoryEditDialog::GetHoveredColor)
						.ShowBackgroundForAlpha(true)
						.IgnoreAlpha(false)
						.OnMouseButtonDown(this, &SDefaultCategoryEditDialog::OnMouseButtonDownHoveredColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
						//.IsEnabled(this, &FColorStructCustomization::IsValueEnabled, StructWeakHandlePtr)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color without alpha
						SNew(SColorBlock)
						.Color(this, &SDefaultCategoryEditDialog::GetHoveredColor)
						.ShowBackgroundForAlpha(false)
						.IgnoreAlpha(true)
						.OnMouseButtonDown(this, &SDefaultCategoryEditDialog::OnMouseButtonDownHoveredColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
					]
				]
			]
			// Pressed
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.Padding(0, 3)
			[
				SNew(SHorizontalBox)
				//.IsEnabled(CategorySetup.bUseCustomColorCategories)
				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Pressed Color")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]

				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color with alpha unless it is ignored
						SAssignNew(PressedColorBox, SColorBlock)
						.Color(this, &SDefaultCategoryEditDialog::GetPressedColor)
						.ShowBackgroundForAlpha(true)
						.IgnoreAlpha(false)
						.OnMouseButtonDown(this, &SDefaultCategoryEditDialog::OnMouseButtonDownPressedColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
						//.IsEnabled(this, &FColorStructCustomization::IsValueEnabled, StructWeakHandlePtr)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color without alpha
						SNew(SColorBlock)
						.Color(this, &SDefaultCategoryEditDialog::GetPressedColor)
						.ShowBackgroundForAlpha(false)
						.IgnoreAlpha(true)
						.OnMouseButtonDown(this, &SDefaultCategoryEditDialog::OnMouseButtonDownPressedColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
					]
				]
			]
			// Text Color
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.Padding(0, 3)
			[
				SNew(SHorizontalBox)
				//.IsEnabled(CategorySetup.bUseCustomColorCategories)
				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Text color")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]

				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color with alpha unless it is ignored
						SAssignNew(TextColorBox,SColorBlock)
						.Color(this, &SDefaultCategoryEditDialog::GetTextColor)
						.ShowBackgroundForAlpha(true)
						.IgnoreAlpha(false)
						.OnMouseButtonDown(this, &SDefaultCategoryEditDialog::OnMouseButtonDownTextColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
						//.IsEnabled(this, &FColorStructCustomization::IsValueEnabled, StructWeakHandlePtr)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color without alpha
						SNew(SColorBlock)
						.Color(this, &SDefaultCategoryEditDialog::GetTextColor)
						.ShowBackgroundForAlpha(false)
						.IgnoreAlpha(true)
						.OnMouseButtonDown(this, &SDefaultCategoryEditDialog::OnMouseButtonDownTextColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
					]
				]
			]
			// Text Shadow Color
			+ SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(VAlign_Center)
			.Padding(0, 3)
			[
				SNew(SHorizontalBox)
				//.IsEnabled(CategorySetup.bUseCustomColorCategories)
				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Text shadow color")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]

				+ SHorizontalBox::Slot()
				.Padding(3, 3)
				.VAlign(VAlign_Center)
				.HAlign(HAlign_Fill)
				[
					SNew(SHorizontalBox)

					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color with alpha unless it is ignored
						SAssignNew(TextShadowColorBox,SColorBlock)
						.Color(this, &SDefaultCategoryEditDialog::GetTextShadowColor)
						.ShowBackgroundForAlpha(true)
						.IgnoreAlpha(false)
						.OnMouseButtonDown(this, &SDefaultCategoryEditDialog::OnMouseButtonDownTextShadowColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
						//.IsEnabled(this, &FColorStructCustomization::IsValueEnabled, StructWeakHandlePtr)
					]
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					[
						// Displays the color without alpha
						SNew(SColorBlock)
						.Color(this, &SDefaultCategoryEditDialog::GetTextShadowColor)
						.ShowBackgroundForAlpha(false)
						.IgnoreAlpha(true)
						.OnMouseButtonDown(this, &SDefaultCategoryEditDialog::OnMouseButtonDownTextShadowColorBlock)
						.Size(FVector2D(35.0f, 12.0f))
					]
				]
			]
			+ SVerticalBox::Slot()
			.Padding(3, 3)
			[
				//SAssignNew(PreviewColorSection,SHorizontalBox)
				//.IsEnabled(CategorySetup.bUseCustomColorCategories)
				//+ SHorizontalBox::Slot()
				//.Padding(3, 3)
				//.VAlign(VAlign_Center)
				//.HAlign(HAlign_Fill)
				//[
				//	SNew(STextBlock)
				//	.Text(FText::FromString(TEXT("Preview Tab")))
				//	.Font(IDetailLayoutBuilder::GetDetailFont())
				//]

				//+ SHorizontalBox::Slot()
				//.Padding(3, 3)
				//.HAlign(HAlign_Fill)
				//[
					SNew(SOverlay)
					+ SOverlay::Slot()
					.VAlign(VAlign_Fill)
					.HAlign(HAlign_Fill)
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("Checkerboard"))
					]
					+ SOverlay::Slot()
					.Padding(10,10)
					[
						SAssignNew(PreviewTab, SCheckBox)
						.Style(FEditorStyle::Get(), "PlacementBrowser.Tab")
						.UncheckedImage(BackgroundImage)
						.UncheckedHoveredImage(HoveredImage)
						.UncheckedPressedImage(BackgroundPressedImage)
						.CheckedImage(BackgroundPressedImage)
						.CheckedHoveredImage(BackgroundPressedImage)
						.CheckedPressedImage(BackgroundPressedImage)
						//.Style(TabStyle)
						//.IsEnabled(CategorySetup.bUseCustomColorCategories)
						//.OnCheckStateChanged(this, &SCustomPlacementModeTools::OnPlacementTabChanged, Info.UniqueHandle)
						//.IsChecked(this, &SCustomPlacementModeTools::GetPlacementTabCheckedState, Info.UniqueHandle)
						[
							SNew(SOverlay)

							+ SOverlay::Slot()
								.VAlign(VAlign_Center)
								[
									SNew(SSpacer)
									.Size(FVector2D(1, 40))
								]

							+ SOverlay::Slot()
								.Padding(FMargin(6, 0, 15, 0))
								.VAlign(VAlign_Center)
								[
									SNew(STextBlock)
									.TextStyle(FEditorStyle::Get(), "PlacementBrowser.Tab.Text")
									.ColorAndOpacity(this, &SDefaultCategoryEditDialog::GetSlateTextColor)
									.ShadowColorAndOpacity(/*CategorySetup.ColorSetting.TextShadowColor*/ this, &SDefaultCategoryEditDialog::GetTextShadowColor)
									.Text(this, &SDefaultCategoryEditDialog::GetName)
								]

							+ SOverlay::Slot()
								.VAlign(VAlign_Fill)
								.HAlign(HAlign_Left)
								[
									SNew(SImage)
									.Image(FEditorStyle::GetBrush("PlacementBrowser.ActiveTabBar"))
								]
						]
					]
				//]
			]
		]

		// accept or cancel button
		+ SVerticalBox::Slot()
		//.FillHeight(1)
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Bottom)
		.Padding(0,3,0,8)
		[
			SNew(SUniformGridPanel)
			.SlotPadding(FEditorStyle::GetMargin("StandardDialog.SlotPadding"))
			
			+ SUniformGridPanel::Slot(0, 0)
			[
				SNew(SButton)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.Text(LOCTEXT("SDefaultCategoryEditDialog_Accept", "Accept"))
				.OnClicked(this, &SDefaultCategoryEditDialog::OnAccept)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
				.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
			]
			+ SUniformGridPanel::Slot(1, 0)
			[
				SNew(SButton)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.HAlign(HAlign_Center)
				.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
				.Text(LOCTEXT("SDefaultCategoryEditDialog_Cancel", "Cancel"))
				.OnClicked(this, &SDefaultCategoryEditDialog::OnCancel)
				.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
				.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
			]
		]
	];
}



FText SDefaultCategoryEditDialog::GetName() const
{
	return FText::FromName(CategoryName);
}

void SDefaultCategoryEditDialog::OnCheckStateChangedCustomColorCategory(ECheckBoxState State)
{
	if (State == ECheckBoxState::Checked)
	{
		CategorySetup.bUseCustomColor = true;
		PreviewColorSection->SetVisibility(EVisibility::Visible);
	}
	else
	{
		CategorySetup.bUseCustomColor = false;
		PreviewColorSection->SetVisibility(EVisibility::Collapsed);
	}
}

void SDefaultCategoryEditDialog::OnCheckStateChangedCategoryVisible(ECheckBoxState State)
{
	if (State == ECheckBoxState::Checked)
	{
		CategorySetup.bVisible = true;
	}
	else
	{
		CategorySetup.bVisible = false;
	}
}

FReply SDefaultCategoryEditDialog::OnAccept()
{

	bApplyChange = true;
	CloseWindow();

	return FReply::Handled();
}

FReply SDefaultCategoryEditDialog::OnCancel()
{
	CloseWindow();
	return FReply::Handled();
}

void SDefaultCategoryEditDialog::CloseWindow()
{
	if (WidgetWindow.IsValid())
	{
		WidgetWindow.Pin()->RequestDestroyWindow();
	}
}

FReply SDefaultCategoryEditDialog::OnMouseButtonDownBackgroundColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

void SDefaultCategoryEditDialog::OnSetBackgroundColorFromColorPicker(FLinearColor NewColor)
{

}

void SDefaultCategoryEditDialog::OnBackgroundColorPickerCancelled(FLinearColor OriginalColor)
{

}

void SDefaultCategoryEditDialog::OnBackgroundColorPickerInteractiveBegin()
{

}

void SDefaultCategoryEditDialog::OnBackgroundColorPickerInteractiveEnd()
{

}

FLinearColor SDefaultCategoryEditDialog::GetBackgroundColor() const
{
	return CategorySetup.BackgroundColor;
}

FReply SDefaultCategoryEditDialog::OnMouseButtonDownHoveredColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

void SDefaultCategoryEditDialog::OnSetHoveredColorFromColorPicker(FLinearColor NewColor)
{

}

void SDefaultCategoryEditDialog::OnHoveredColorPickerCancelled(FLinearColor OriginalColor)
{

}

FLinearColor SDefaultCategoryEditDialog::GetHoveredColor() const
{
	return CategorySetup.HoveredColor;
}

FReply SDefaultCategoryEditDialog::OnMouseButtonDownPressedColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

void SDefaultCategoryEditDialog::OnSetPressedColorFromColorPicker(FLinearColor NewColor)
{

}

void SDefaultCategoryEditDialog::OnPressedColorPickerCancelled(FLinearColor OriginalColor)
{

}

FLinearColor SDefaultCategoryEditDialog::GetPressedColor() const
{
	return CategorySetup.PressedSelectionColor;
}

FReply SDefaultCategoryEditDialog::OnMouseButtonDownTextColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

void SDefaultCategoryEditDialog::OnSetTextColorFromColorPicker(FLinearColor NewColor)
{

}

void SDefaultCategoryEditDialog::OnTextColorPickerCancelled(FLinearColor OriginalColor)
{

}

FLinearColor SDefaultCategoryEditDialog::GetTextColor() const
{
	return CategorySetup.TextColor;
}

FReply SDefaultCategoryEditDialog::OnMouseButtonDownTextShadowColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	return FReply::Handled();
}

void SDefaultCategoryEditDialog::OnSetTextShadowColorFromColorPicker(FLinearColor NewColor)
{

}

void SDefaultCategoryEditDialog::OnTextShadowColorPickerCancelled(FLinearColor OriginalColor)
{

}

FLinearColor SDefaultCategoryEditDialog::GetTextShadowColor() const
{
	return CategorySetup.TextShadowColor;
}

FSlateColor SDefaultCategoryEditDialog::GetSlateTextColor() const
{
	return FSlateColor(CategorySetup.TextColor);
}

TSharedRef<IDetailCustomization> FSettingPlaceableCategoryItemCustomization::MakeInstance()
{ 
	return MakeShareable(new FSettingPlaceableCategoryItemCustomization());
}

void FSettingPlaceableCategoryItemCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& ObjectChannelCategory = DetailBuilder.EditCategory("Custom Categories");

	Settings = UCustomPlacementModeSettings::Get();
	Settings->LoadConfig();
	RefreshCategoryList();

	//TSharedPtr<SToolTip> ObjectChannelTooltip = IDocumentation::Get()->CreateToolTip(LOCTEXT("EditCollisionObject", "Edit collision object types."), NULL,TEXT("Test Link"), TEXT("Test Categories"));
	//TSharedPtr<SToolTip> TraceChannelTooltip = IDocumentation::Get()->CreateToolTip(LOCTEXT("EditCollisionChannel", "Edit collision trace channels."), NULL, TraceChannelDocLink, TEXT("TraceChannel"));
	//TSharedPtr<SToolTip> ProfileTooltip = IDocumentation::Get()->CreateToolTip(LOCTEXT("EditCollisionPreset", "Edit collision presets."), NULL, PresetsDocLink, TEXT("Preset"));

	FSlateFontInfo CategoryNameFont = IDetailLayoutBuilder::GetDetailFontBold();
	CategoryNameFont.Size += 3;

	// Customize collision section
	ObjectChannelCategory.AddCustomRow(LOCTEXT("CustomPlacementObjectCategories", "ObjectCategories"))
	[
		SNew(SVerticalBox)

		+ SVerticalBox::Slot()
			.Padding(5)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
					.Padding(2, 10)
					.FillWidth(1)
					[
						SNew(STextBlock)
						.AutoWrapText(true)
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.Text(LOCTEXT("ObjectCategory_Menu_Description", "You can have up a lot of custom categories. This is list of custom category for your project. If you delete the custom category that has been used by editor, it will removed from PlacementMode. Need Restart Editor."))
					]
			]
		+ SVerticalBox::Slot()
			.Padding(5)
			.AutoHeight()
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
					.Padding(2, 10)
					.FillWidth(1)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(CategoryNameFont)
						.Text(LOCTEXT("ObjectCategory_Menu_CategoryItems", "Custom Categories"))
					]
				+ SHorizontalBox::Slot()
					.Padding(2, 10)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.Text(LOCTEXT("CategoryMenu_NewCategory", "New Category..."))
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Success")
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
						.ToolTip(
							SNew(SToolTip)
							.Text(FText::FromString("Create new category"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						)
						.OnClicked(this, &FSettingPlaceableCategoryItemCustomization::OnNewCategory)
					]

				+ SHorizontalBox::Slot()
					.Padding(2, 10)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.Text(LOCTEXT("CategoryMenu_Edit", "Edit..."))
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
						.ToolTip(
							SNew(SToolTip)
							.Text(FText::FromString("Edit category"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						)
						.OnClicked(this, &FSettingPlaceableCategoryItemCustomization::OnEditCategory)
						.IsEnabled(this, &FSettingPlaceableCategoryItemCustomization::IsAnyCategorySelected)
					]

				+ SHorizontalBox::Slot()
					.Padding(2, 10)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.Text(LOCTEXT("CategoryMenu_Delete", "Delete..."))
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Danger")
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
						.ToolTip(
							SNew(SToolTip)
							.Text(FText::FromString("Delete category"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						)
						.OnClicked(this, &FSettingPlaceableCategoryItemCustomization::OnDeleteCategory)
						.IsEnabled(this, &FSettingPlaceableCategoryItemCustomization::IsAnyCategorySelected)
					]
				+ SHorizontalBox::Slot()
					.Padding(2, 10)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.Text(LOCTEXT("CategoryMenu_ItemUp", "Up"))
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
						.ToolTip(
							SNew(SToolTip)
							.Text(FText::FromString("Raise category"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						)
						.OnClicked(this, &FSettingPlaceableCategoryItemCustomization::OnUpCategory)
						.IsEnabled(this, &FSettingPlaceableCategoryItemCustomization::IsAnyCategorySelectedCanUp)
						.ContentPadding(2.0f)
						.Content()
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush("WorldBrowser.DirectionYNegative"))
						]
					]
				+ SHorizontalBox::Slot()
					.Padding(2, 10)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.Text(LOCTEXT("CategoryMenu_Down", "Down"))
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
						.ToolTip(
							SNew(SToolTip)
							.Text(FText::FromString("Lower category"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						)
						.OnClicked(this, &FSettingPlaceableCategoryItemCustomization::OnDownCategory)
						.IsEnabled(this, &FSettingPlaceableCategoryItemCustomization::IsAnyCategorySelectedCanDown)
						.ContentPadding(2.0f)
						.Content()
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush("WorldBrowser.DirectionYPositive"))
						]
						]
				+ SHorizontalBox::Slot()
					.Padding(2, 10)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.Text(LOCTEXT("CategoryMenu_Empty", "Empty"))
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Danger")
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
						.ToolTip(
							SNew(SToolTip)
							.Text(FText::FromString("Clear category list"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						)
						.ContentPadding( 4.0f )
						.Content()
						[
							SNew(SImage)
								.Image(FEditorStyle::GetBrush("PropertyWindow.Button_EmptyArray"))
						]
						.OnClicked(this, &FSettingPlaceableCategoryItemCustomization::OnDeleteCategoryItems)
						.IsEnabled(this, &FSettingPlaceableCategoryItemCustomization::IsCategoryItemsFill)
					]
				+ SHorizontalBox::Slot()
					.Padding(2, 10)
					.AutoWidth()
					.VAlign(VAlign_Center)
					[
						SNew(SButton)
						.Visibility(Settings->bShowRefreshButtons ? EVisibility::Visible : EVisibility::Collapsed)
						.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
						.Text(LOCTEXT("CategoryMenu_Refresh", "Refresh"))
						.ToolTip(
							SNew(SToolTip)
							.Text(FText::FromString("Refresh Items (Experimental)"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						)
						.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
						.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
						.OnClicked(this, &FSettingPlaceableCategoryItemCustomization::OnRefreshCategory)
						.ContentPadding(3.0f)
						.Content()
						[
							SNew(SImage)
							.Image(FEditorStyle::GetBrush("AnimEditor.RefreshButton"))
						]
					]
			]

		+ SVerticalBox::Slot()
			.Padding(5)
			.FillHeight(1)
			[
				SAssignNew(CustomObjectCategoryListView, SCustomCategoryListView)
				.ItemHeight(15.f)
				.ListItemsSource(&CutomObjectCategoryList)
				.OnGenerateRow(this, &FSettingPlaceableCategoryItemCustomization::HandleGenerateCategoryWidget)
				// Must be added
				//.OnMouseButtonDoubleClick(this, &FCollisionProfileDetails::OnObjectChannelListItemDoubleClicked)
				.SelectionMode(ESelectionMode::Single)
				.HeaderRow(
					SNew(SHeaderRow)
					// Index
					+ SHeaderRow::Column("CategoryIndex")
						.HAlignCell(HAlign_Left)
						.FixedWidth(50.0f)
						.HeaderContentPadding(FMargin(0, 3))
						[
							SNew(STextBlock)
							.Text(LOCTEXT("CategoryListHeader_Index", "ID"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					// Category Name
					+ SHeaderRow::Column("CategoryName")
						.HAlignCell(HAlign_Fill)
						.FillWidth(1)
						.HeaderContentPadding(FMargin(0, 3))
						[
							SNew(STextBlock)
							.Text(LOCTEXT("CategoryListHeader_CategoryName", "Name"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					// Use BP classes
					+ SHeaderRow::Column("UseBlueprintClasses")
						.HAlignCell(HAlign_Center)
						.FillWidth(1)
						.HeaderContentPadding(FMargin(0, 3))
						[
							SNew(STextBlock)
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("CategoryListHeader_UseBlueprintClasses", "Blueprint classes"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					// Use BP child classes
/*					+ SHeaderRow::Column("UseBlueprintChildClasses")
						.HAlignCell(HAlign_Center)
						.FillWidth(1)
						.HeaderContentPadding(FMargin(0, 3))
						[
							SNew(STextBlock)
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("CategoryListHeader_UseBlueprintChildClasses", "Blueprint child classes"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]*/
					// UseCustomColorCategories
					+ SHeaderRow::Column("UseCustomColorCategories")
						.HAlignCell(HAlign_Center)
						.FillWidth(1)
						.HeaderContentPadding(FMargin(0, 3))
						[
							SNew(STextBlock)
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("CategoryListHeader_UseCustomColorCategories", "Custom color"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					// Items Num
					+ SHeaderRow::Column("ClassItemsCount")
						.HAlignCell(HAlign_Center)
						.FillWidth(1)
						.HeaderContentPadding(FMargin(0, 3))
						[
							SNew(STextBlock)
							.Justification(ETextJustify::Center)
							.Text(LOCTEXT("CategoryListHeader_ItemsCount", "Class items count"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
				)
			]
	];


	TArray<TSharedRef<IPropertyHandle>> Properies;
	ObjectChannelCategory.GetDefaultProperties(Properies);

	TSharedRef<IPropertyHandle> Property = Properies[0];
	Property->MarkHiddenByCustomization();

	GenerateDefaultCategory(DetailBuilder);
}

void FSettingPlaceableCategoryItemCustomization::GenerateDefaultCategory(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& DefaultObjectCategory = DetailBuilder.EditCategory("Default Categories");

	//Settings = UCustomPlacementModeSettings::Get();
	//Settings->LoadConfig();
	
	RefreshDefaultCategoryList();

	//TSharedPtr<SToolTip> ObjectChannelTooltip = IDocumentation::Get()->CreateToolTip(LOCTEXT("EditCollisionObject", "Edit collision object types."), NULL,TEXT("Test Link"), TEXT("Test Categories"));
	//TSharedPtr<SToolTip> TraceChannelTooltip = IDocumentation::Get()->CreateToolTip(LOCTEXT("EditCollisionChannel", "Edit collision trace channels."), NULL, TraceChannelDocLink, TEXT("TraceChannel"));
	//TSharedPtr<SToolTip> ProfileTooltip = IDocumentation::Get()->CreateToolTip(LOCTEXT("EditCollisionPreset", "Edit collision presets."), NULL, PresetsDocLink, TEXT("Preset"));

	FSlateFontInfo CategoryNameFont = IDetailLayoutBuilder::GetDetailFontBold();
	CategoryNameFont.Size += 3;

	// Customize collision section
	DefaultObjectCategory.AddCustomRow(LOCTEXT("CustomPlacementDefaultObjectCategories", "DefaultObjectCategories"))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
				.Padding(5)
				.AutoHeight()
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.Padding(2, 10)
					.FillWidth(1)
					.VAlign(VAlign_Center)
					[
						SNew(STextBlock)
						.Font(CategoryNameFont)
					.Text(LOCTEXT("DefaultObjectCategory_Menu_CategoryItems", "Default Categories"))
					]
	+ SHorizontalBox::Slot()
		.Padding(2, 10)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
		.Text(LOCTEXT("DefaultCategoryMenu_Edit", "Edit..."))
		.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
		.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
		.ToolTip(
			SNew(SToolTip)
			.Text(FText::FromString("Edit category"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		)
		.OnClicked(this, &FSettingPlaceableCategoryItemCustomization::OnEditDefaultCategory)
		.IsEnabled(this, &FSettingPlaceableCategoryItemCustomization::IsAnyDefaultCategorySelected)
		]
	+ SHorizontalBox::Slot()
		.Padding(2, 10)
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			SNew(SButton)
			.Visibility(Settings->bShowRefreshButtons ? EVisibility::Visible : EVisibility::Collapsed)
		.ContentPadding(FEditorStyle::GetMargin("StandardDialog.ContentPadding"))
		.Text(LOCTEXT("DefaultCategoryMenu_Refresh", "Refresh"))
		.ToolTip(
			SNew(SToolTip)
			.Text(FText::FromString("Refresh Items (Experimental)"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		)
		.ButtonStyle(FEditorStyle::Get(), "FlatButton.Default")
		.TextStyle(FEditorStyle::Get(), "FlatButton.DefaultTextStyle")
		.OnClicked(this, &FSettingPlaceableCategoryItemCustomization::OnRefreshDefaultCategory)
		.ContentPadding(3.0f)
		.Content()
		[
			SNew(SImage)
			.Image(FEditorStyle::GetBrush("AnimEditor.RefreshButton"))
		]
		]
		]

	+ SVerticalBox::Slot()
		.Padding(5)
		.FillHeight(1)
		[
			SAssignNew(DefaultObjectCategoryListView, SDefaultCategoryListView)
			.ItemHeight(15.f)
		.ListItemsSource(&DefaultObjectCategoryList)
		.OnGenerateRow(this, &FSettingPlaceableCategoryItemCustomization::HandleGenerateDefaultCategoryWidget)
		// Must be added
		//.OnMouseButtonDoubleClick(this, &FCollisionProfileDetails::OnObjectChannelListItemDoubleClicked)
		.SelectionMode(ESelectionMode::Single)
		.HeaderRow(
			SNew(SHeaderRow)
			// Visibility
			+ SHeaderRow::Column("CategoryVisible")
			.FixedWidth(40.0f)
			.HAlignCell(HAlign_Left)
			//.AutoWidth()
			.HeaderContentPadding(FMargin(0, 3))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CategoryListHeader_Visible", "Visible"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			// Index
			+ SHeaderRow::Column("CategoryIndex")
			.HAlignCell(HAlign_Left)
			.FixedWidth(50.0f)
			.HeaderContentPadding(FMargin(0, 3))
			[
				SNew(STextBlock)
				.Text(LOCTEXT("CategoryListHeader_Index", "Index"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			// Category Name
			+ SHeaderRow::Column("CategoryName")
				.HAlignCell(HAlign_Fill)
				.FillWidth(1)
				.HeaderContentPadding(FMargin(0, 3))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CategoryListHeader_CategoryName", "Name"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			// UseCustomColor
			+ SHeaderRow::Column("UseCustomColor")
				.HAlignCell(HAlign_Center)
				.FillWidth(1)
				.HeaderContentPadding(FMargin(0, 3))
				[
					SNew(STextBlock)
					.Justification(ETextJustify::Center)
				.Text(LOCTEXT("DefaultCategoryListHeader_UseCustomColor", "Custom color"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			)
		]
	];


	TArray<TSharedRef<IPropertyHandle>> Properies;
	DefaultObjectCategory.GetDefaultProperties(Properies);

	for (TSharedRef<IPropertyHandle> Item : Properies)
	{
		Item->MarkHiddenByCustomization();
	}
}

TSharedRef<ITableRow> FSettingPlaceableCategoryItemCustomization::HandleGenerateCategoryWidget(TSharedPtr< FCustomCategoryListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SCustomCategoryListItem, OwnerTable)
		.CategorySetup(InItem->CategorySetup).CategoryIndex(InItem->CategoryIndex);
}

FReply FSettingPlaceableCategoryItemCustomization::OnNewCategory()
{
	TSharedRef<SWindow> WidgetWindow = SNew(SWindow)
		.Title(LOCTEXT("CategoryDetail_NewCategoryTitle", "New Category"))
		.Type(EWindowType::Normal)
		.AutoCenter(EAutoCenter::None)
		.ScreenPosition(FVector2D(CATEGORY_WINDOW_WIDTH, CATEGORY_WINDOW_HEIGHT / 2))
		.ClientSize(FVector2D(CATEGORY_WINDOW_WIDTH, CATEGORY_WINDOW_HEIGHT))
		.MinWidth(CATEGORY_WINDOW_WIDTH)
		//.MinHeight(CATEGORY_WINDOW_HEIGHT)
		.MaxHeight(CATEGORY_WINDOW_HEIGHT)
		.MaxWidth(CATEGORY_WINDOW_WIDTH)
		.SupportsMaximize(false)
		.SupportsMinimize(false)
		.SizingRule(ESizingRule::Autosized);

	TSharedPtr<SCategoryEditDialog> CategoryEditor;
	WidgetWindow->SetContent
	(
		SNew(SBorder)
		.Padding(FMargin(2.0f, 4.0f, 2.0f, 2.0f))
		//.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
		.BorderImage(FCoreStyle::Get().GetBrush("Docking.Border"))
		[
			SAssignNew(CategoryEditor, SCategoryEditDialog)
			.CategorySetup(NULL)
			//.CategoryIndex(nullptr)
			.Settings(Settings)
			.WidgetWindow(WidgetWindow)
			.OnValidateCategory(this, &FSettingPlaceableCategoryItemCustomization::IsValidCategorySetup)
		]
	);

	GEditor->EditorAddModalWindow(WidgetWindow);
	//FSlateApplication::Get().AddWindow( WidgetWindow, true );
	//int32 CategoryIndex = FindCategoryIndexFromName(CategorySetup.CategoryName);

	// add to collision profile
	if (CategoryEditor->bApplyChange &&
		ensure(IsValidCategorySetup(&(CategoryEditor->CategorySetup), CategoryEditor->CategoryIndex)))
	{
		Settings->PlaceableCategoryItems.Add(CategoryEditor->CategorySetup);
		Settings->SaveConfig();

		UpdateCategory();

		CustomObjectCategoryListView->SetItemSelection(CutomObjectCategoryList[CutomObjectCategoryList.Num() -1], true);
	}

	return FReply::Handled();
}

FReply FSettingPlaceableCategoryItemCustomization::OnEditCategory()
{
	TArray< TSharedPtr< FCustomCategoryListItem > > SelectedItems = CustomObjectCategoryListView->GetSelectedItems();

	if (SelectedItems.Num() == 1)
	{
		TSharedRef<SWindow> WidgetWindow = SNew(SWindow)
			.Title(LOCTEXT("CategoryDetail_EditCategoryTitle", "Edit Category"))
			.Type(EWindowType::Normal)
			.AutoCenter(EAutoCenter::None)
			.ScreenPosition(FVector2D(CATEGORY_WINDOW_WIDTH, CATEGORY_WINDOW_HEIGHT/2))
			.ClientSize(FVector2D(CATEGORY_WINDOW_WIDTH, CATEGORY_WINDOW_HEIGHT))
			.MinWidth(CATEGORY_WINDOW_WIDTH)
			//.MinHeight(CATEGORY_WINDOW_HEIGHT)
			.MaxHeight(CATEGORY_WINDOW_HEIGHT)
			.MaxWidth(CATEGORY_WINDOW_WIDTH)
			.SupportsMaximize(false)
			.SupportsMinimize(false)
			.SizingRule(ESizingRule::Autosized);

		TSharedPtr< FCustomCategoryListItem > SelectedItem = SelectedItems[0];
		int32 CategoryIndex = FindCategoryIndexFromName(SelectedItem->CategorySetup->CategoryName);

		TSharedPtr<SCategoryEditDialog> CategoryEditor;
		WidgetWindow->SetContent
		(
			SNew(SBorder)
			.Padding(FMargin(2.0f,4.0f,2.0f,2.0f))
			//.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
			.BorderImage(FCoreStyle::Get().GetBrush("Docking.Border"))
			[
				SAssignNew(CategoryEditor, SCategoryEditDialog)
				.CategorySetup(SelectedItem->CategorySetup.Get())
				.CategoryIndex(CategoryIndex)
				.Settings(Settings)
			.WidgetWindow(WidgetWindow)
			.OnValidateCategory(this, &FSettingPlaceableCategoryItemCustomization::IsValidCategorySetup)
			]
		);

		GEditor->EditorAddModalWindow(WidgetWindow);

		// add to collision profile
		if (CategoryEditor->bApplyChange &&
			ensure(IsValidCategorySetup(&(CategoryEditor->CategorySetup),CategoryIndex)))
		{
			FSettingPlaceableCategoryItem * Item = FindFromCategory(CategoryEditor->CategorySetup);
			if (Item)
			{
				UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("ItemUpdate - %s"), *Item->CategoryName.ToString());
				*Item = CategoryEditor->CategorySetup;
				// refresh view
				
				Settings->PlaceableCategoryItems[CategoryIndex] = CategoryEditor->CategorySetup;
				Settings->SaveConfig();
				
				UpdateCategory();
			}
			else
			{
				UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("Item invalid"));
			}
		}

		CustomObjectCategoryListView->SetItemSelection(CutomObjectCategoryList[CategoryIndex], true);
	}
	return FReply::Handled();
}

bool FSettingPlaceableCategoryItemCustomization::IsAnyCategorySelected() const
{
	return CustomObjectCategoryListView->GetNumItemsSelected() > 0;
}

bool FSettingPlaceableCategoryItemCustomization::IsAnyCategorySelectedCanUp() const
{
	TArray< TSharedPtr< FCustomCategoryListItem > > SelectedItems = CustomObjectCategoryListView->GetSelectedItems();
	
	int32 index = 0;
	if (SelectedItems.Num() == 1)
	{
		TSharedPtr< FCustomCategoryListItem > SelectedItem = SelectedItems[0];

		int32 i = 0;
		for (auto Iter = Settings->PlaceableCategoryItems.CreateIterator(); Iter; ++Iter)
		{
			if(Iter->CategoryName == SelectedItem->CategorySetup->CategoryName)
			{
				index = i;
			}
			i++;
		}
	}

	return (CustomObjectCategoryListView->GetNumItemsSelected() > 0 && index > 0 && CutomObjectCategoryList.Num() > 1);
}


FReply FSettingPlaceableCategoryItemCustomization::OnUpCategory()
{
	TArray< TSharedPtr< FCustomCategoryListItem > > SelectedItems = CustomObjectCategoryListView->GetSelectedItems();
	
	int32 index = 0;
	if (SelectedItems.Num() == 1)
	{
		TSharedPtr< FCustomCategoryListItem > SelectedItem = SelectedItems[0];

		int32 i = 0;
		for (auto Iter = Settings->PlaceableCategoryItems.CreateIterator(); Iter; ++Iter)
		{
			if(Iter->CategoryName == SelectedItem->CategorySetup->CategoryName)
			{
				index = i;
			}
			i++;
		}
	
		FSettingPlaceableCategoryItem Item = Settings->PlaceableCategoryItems[index - 1];
		Settings->PlaceableCategoryItems[index - 1] = Settings->PlaceableCategoryItems[index];
		Settings->PlaceableCategoryItems[index] = Item;

		Settings->SaveConfig();

		UpdateCategory();
		
		CustomObjectCategoryListView->SetItemSelection(CutomObjectCategoryList[index -1],true);
	}

	return FReply::Handled();
}


bool FSettingPlaceableCategoryItemCustomization::IsAnyCategorySelectedCanDown() const
{
	TArray< TSharedPtr< FCustomCategoryListItem > > SelectedItems = CustomObjectCategoryListView->GetSelectedItems();
	
	int32 index = 0;
	if (SelectedItems.Num() == 1)
	{
		TSharedPtr< FCustomCategoryListItem > SelectedItem = SelectedItems[0];

		int32 i = 0;
		for (auto Iter = Settings->PlaceableCategoryItems.CreateIterator(); Iter; ++Iter)
		{
			if(Iter->CategoryName == SelectedItem->CategorySetup->CategoryName)
			{
				index = i;
			}
			i++;
		}
	}

	return (CustomObjectCategoryListView->GetNumItemsSelected() > 0 && index < CutomObjectCategoryList.Num() -1 && CutomObjectCategoryList.Num() > 1);
}

FReply FSettingPlaceableCategoryItemCustomization::OnDownCategory()
{
	TArray< TSharedPtr< FCustomCategoryListItem > > SelectedItems = CustomObjectCategoryListView->GetSelectedItems();

	int32 index = 0;
	if (SelectedItems.Num() == 1)
	{
		TSharedPtr< FCustomCategoryListItem > SelectedItem = SelectedItems[0];

		int32 i = 0;
		for (auto Iter = Settings->PlaceableCategoryItems.CreateIterator(); Iter; ++Iter)
		{
			if (Iter->CategoryName == SelectedItem->CategorySetup->CategoryName)
			{
				index = i;
			}
			i++;
		}

		FSettingPlaceableCategoryItem Item = Settings->PlaceableCategoryItems[index + 1];
		Settings->PlaceableCategoryItems[index + 1] = Settings->PlaceableCategoryItems[index];
		Settings->PlaceableCategoryItems[index] = Item;

		Settings->SaveConfig();

		UpdateCategory();

		CustomObjectCategoryListView->SetItemSelection(CutomObjectCategoryList[index + 1], true);
	}

	return FReply::Handled();
}

bool FSettingPlaceableCategoryItemCustomization::IsCategoryItemsFill() const
{
	if (CutomObjectCategoryList.Num() > 0)
	{
		return true;
	}

	return false;
}

FReply FSettingPlaceableCategoryItemCustomization::OnDeleteCategoryItems()
{
	if (CutomObjectCategoryList.Num() > 0)
	{
		FText Title = LOCTEXT("FCategoryItemDetails_DeleteCategoryItemsTitle", "Clear category item list");

		if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("FCategoryItemDetails_DeleteCategoryItems", "All categories will be removed. \nWould you like to continue?"), &Title) == EAppReturnType::Yes)
		{
			Settings->PlaceableCategoryItems.Empty();
			Settings->PlaceableCategoryItems.Shrink();
			Settings->SaveConfig();
			RefreshCategoryList();
		}
	}
	return FReply::Handled();
}

FReply FSettingPlaceableCategoryItemCustomization::OnRefreshCategory()
{
	TArray< TSharedPtr< FCustomCategoryListItem > > SelectedItems = CustomObjectCategoryListView->GetSelectedItems();

	if (SelectedItems.Num() == 1)
	{
		TSharedPtr< FCustomCategoryListItem > SelectedItem = SelectedItems[0];
		int32 CategoryIndex = FindCategoryIndexFromName(SelectedItem->CategorySetup->CategoryName);
		UpdateCategory();
		CustomObjectCategoryListView->SetItemSelection(CutomObjectCategoryList[CategoryIndex], true);
	}

	return FReply::Handled();
}

bool FSettingPlaceableCategoryItemCustomization::IsValidCategorySetup(const FSettingPlaceableCategoryItem* Category, int32 CategoryIndex) const
{
	for(auto Iter = Settings->PlaceableCategoryItems.CreateConstIterator(); Iter; ++Iter)
	{
		if(Iter->CategoryID != Category->CategoryID)
		{
			// make sure name isn't same
			if(Iter->CategoryName == Category->CategoryName)
			{
				return false;
			}
		}
	}

	return true;
}

FReply FSettingPlaceableCategoryItemCustomization::OnDeleteCategory()
{
	TArray< TSharedPtr< FCustomCategoryListItem > > SelectedItems = CustomObjectCategoryListView->GetSelectedItems();

	if (SelectedItems.Num() == 1)
	{
		if (FMessageDialog::Open(EAppMsgType::YesNo, LOCTEXT("FCategoryDetails_DeleteCategory", "If you delete this category, all the objects that use this channel will be set to default. \nWould you like to continue?")) == EAppReturnType::Yes)
		{
			TSharedPtr< FCustomCategoryListItem > SelectedItem = SelectedItems[0];

			RemoveCategory(SelectedItem->CategorySetup->CategoryName);
			UpdateCategory();
		}
	}
	return FReply::Handled();
}

void FSettingPlaceableCategoryItemCustomization::RefreshCategoryList()
{
	CutomObjectCategoryList.Empty();

	int index = 0;
	for (auto Iter = Settings->PlaceableCategoryItems.CreateIterator(); Iter; ++Iter)
	{
		CutomObjectCategoryList.Add(MakeShareable(new FCustomCategoryListItem(MakeShareable(new FSettingPlaceableCategoryItem(*Iter)), MakeShareable(new int32(index)))));
		index++;
	}
}

void FSettingPlaceableCategoryItemCustomization::UpdateCategory()
{
	RefreshCategoryList();
	CustomObjectCategoryListView->RequestListRefresh();
}

FSettingPlaceableCategoryItem * FSettingPlaceableCategoryItemCustomization::FindFromCategory(FSettingPlaceableCategoryItem Category) const
{
	return Settings->PlaceableCategoryItems.FindByPredicate([Category](const FSettingPlaceableCategoryItem& Item)
	{
		return Item.CategoryID == Category.CategoryID;
	});
}

int32 FSettingPlaceableCategoryItemCustomization::FindCategoryIndexFromName(FName Name) const
{
	for (auto Iter = Settings->PlaceableCategoryItems.CreateIterator(); Iter; ++Iter)
	{
		if (Iter->CategoryName == Name)
		{
			return Iter.GetIndex();
		}
	}

	return INDEX_NONE;
}

void FSettingPlaceableCategoryItemCustomization::RemoveCategory(FName CategoryName) const
{
	for (auto Iter = Settings->PlaceableCategoryItems.CreateIterator(); Iter; ++Iter)
	{
		if (Iter->CategoryName == CategoryName)
		{
			Settings->PlaceableCategoryItems.RemoveAt(Iter.GetIndex());
			break;
		}
	}
}

TSharedRef<ITableRow> FSettingPlaceableCategoryItemCustomization::HandleGenerateDefaultCategoryWidget(TSharedPtr< FDefaultCategoryListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(SDefaultCategoryListItem, OwnerTable)
		.CategorySetup(InItem->CategorySetup).CategoryIndex(InItem->CategoryIndex).CategoryName(InItem->CategoryName);
}

void FSettingPlaceableCategoryItemCustomization::RefreshDefaultCategoryList()
{
	DefaultObjectCategoryList.Empty();

	DefaultObjectCategoryList.Add(MakeShareable(new FDefaultCategoryListItem(MakeShareable(new FSettingDefaultPlaceableCategoryItem(Settings->RecentlyPlaced)), MakeShareable(new FName("Recently Placed")), MakeShareable(new int32(0)))));
	DefaultObjectCategoryList.Add(MakeShareable(new FDefaultCategoryListItem(MakeShareable(new FSettingDefaultPlaceableCategoryItem(Settings->Basic)), MakeShareable(new FName("Basic")), MakeShareable(new int32(1)))));
	DefaultObjectCategoryList.Add(MakeShareable(new FDefaultCategoryListItem(MakeShareable(new FSettingDefaultPlaceableCategoryItem(Settings->Lights)), MakeShareable(new FName("Lights")), MakeShareable(new int32(2)))));
	DefaultObjectCategoryList.Add(MakeShareable(new FDefaultCategoryListItem(MakeShareable(new FSettingDefaultPlaceableCategoryItem(Settings->Visual)), MakeShareable(new FName("Visual")), MakeShareable(new int32(3)))));
	DefaultObjectCategoryList.Add(MakeShareable(new FDefaultCategoryListItem(MakeShareable(new FSettingDefaultPlaceableCategoryItem(Settings->Volumes)), MakeShareable(new FName("Volumes")), MakeShareable(new int32(4)))));
	DefaultObjectCategoryList.Add(MakeShareable(new FDefaultCategoryListItem(MakeShareable(new FSettingDefaultPlaceableCategoryItem(Settings->AllClasses)), MakeShareable(new FName("All Classes")), MakeShareable(new int32(5)))));
}

void FSettingPlaceableCategoryItemCustomization::UpdateDefaultCategory()
{
	RefreshDefaultCategoryList();
	DefaultObjectCategoryListView->RequestListRefresh();
}

FReply FSettingPlaceableCategoryItemCustomization::OnRefreshDefaultCategory()
{
	TArray< TSharedPtr< FDefaultCategoryListItem > > SelectedItems = DefaultObjectCategoryListView->GetSelectedItems();

	if (SelectedItems.Num() == 1)
	{
		TSharedPtr< FDefaultCategoryListItem > SelectedItem = SelectedItems[0];
		int32* CategoryIndex = SelectedItem->CategoryIndex.Get();
		UpdateDefaultCategory();
		DefaultObjectCategoryListView->SetItemSelection(DefaultObjectCategoryList[*CategoryIndex], true);
	}

	return FReply::Handled();
}

bool FSettingPlaceableCategoryItemCustomization::IsAnyDefaultCategorySelected() const
{
	return DefaultObjectCategoryListView->GetNumItemsSelected() > 0;
}

FReply FSettingPlaceableCategoryItemCustomization::OnEditDefaultCategory()
{
	TArray< TSharedPtr< FDefaultCategoryListItem > > SelectedItems = DefaultObjectCategoryListView->GetSelectedItems();

	if (SelectedItems.Num() == 1)
	{
		TSharedPtr< FDefaultCategoryListItem > SelectedItem = SelectedItems[0];
		
		int32* CategoryIndex = SelectedItem->CategoryIndex.Get();
		
		UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("ID - %d : Edit Default Category - %s"),*CategoryIndex,*SelectedItem->CategoryName->ToString());
		
		
		TSharedRef<SWindow> WidgetWindow = SNew(SWindow)
			.Title(LOCTEXT("DefaultCategoryDetail_EditCategoryTitle", "Edit Default Category"))
			.Type(EWindowType::Normal)
			.AutoCenter(EAutoCenter::None)
			.ScreenPosition(FVector2D(CATEGORY_WINDOW_WIDTH, CATEGORY_WINDOW_HEIGHT/2))
			.ClientSize(FVector2D(CATEGORY_WINDOW_WIDTH, CATEGORY_WINDOW_HEIGHT))
			.MinWidth(CATEGORY_WINDOW_WIDTH)
			//.MinHeight(CATEGORY_WINDOW_HEIGHT)
			.MaxHeight(CATEGORY_WINDOW_HEIGHT)
			.MaxWidth(CATEGORY_WINDOW_WIDTH)
			.SupportsMaximize(false)
			.SupportsMinimize(false)
			.SizingRule(ESizingRule::Autosized);

		TSharedPtr<SDefaultCategoryEditDialog> CategoryEditor;
		WidgetWindow->SetContent
		(
			SNew(SBorder)
			.Padding(FMargin(2.0f,4.0f,2.0f,2.0f))
			//.BorderImage(FCoreStyle::Get().GetBrush("NoBorder"))
			.BorderImage(FCoreStyle::Get().GetBrush("Docking.Border"))
			[
				SAssignNew(CategoryEditor, SDefaultCategoryEditDialog)
				.CategorySetup(SelectedItem->CategorySetup.Get())
				.CategoryName(*SelectedItem->CategoryName.Get())
				.CategoryIndex(*CategoryIndex)
				.Settings(Settings)
			.WidgetWindow(WidgetWindow)
			]
		);

		// If there is already a modal window active, parent this new modal window to the existing window so that it doesnt fall behind
		TSharedPtr<SWindow> ParentWindow = FSlateApplication::Get().GetActiveModalWindow();

		if (!ParentWindow.IsValid())
		{
			// Parent to the main frame window
			if (FModuleManager::Get().IsModuleLoaded("MainFrame"))
			{
				IMainFrameModule& MainFrame = FModuleManager::LoadModuleChecked<IMainFrameModule>("MainFrame");
				ParentWindow = MainFrame.GetParentWindow();
			}
		}

		//FSlateApplication::Get().AddWindowAsNativeChild(WidgetWindow, ParentWindow.ToSharedRef());
		GEditor->EditorAddModalWindow(WidgetWindow);

		// add to collision profile
		if (CategoryEditor->bApplyChange)
		{
			UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("ItemUpdate - %s"), *SelectedItem->CategoryName.Get()->ToString());

			if (*SelectedItem->CategoryName.Get() == "Recently Placed")
			{
				UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("Recently Update"));
				Settings->RecentlyPlaced = CategoryEditor->CategorySetup;
			}
			else if (*SelectedItem->CategoryName.Get() == "Basic")
			{
				UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("Basic Update"));
				Settings->Basic = CategoryEditor->CategorySetup;
			}
			else if (*SelectedItem->CategoryName.Get() == "Lights")
			{
				UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("Lights Update"));
				Settings->Lights = CategoryEditor->CategorySetup;
			}
			else if (*SelectedItem->CategoryName.Get() == "Visual")
			{
				UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("Visual Update"));
				Settings->Visual = CategoryEditor->CategorySetup;
			}
			else if (*SelectedItem->CategoryName.Get() == "Volumes")
			{
				UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("Volumes Update"));
				Settings->Volumes = CategoryEditor->CategorySetup;
			}
			else if (*SelectedItem->CategoryName.Get() == "All Classes")
			{
				UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("All Classes Update"));
				Settings->AllClasses = CategoryEditor->CategorySetup;
			}

			Settings->SaveConfig();
			UpdateDefaultCategory();
		}

		DefaultObjectCategoryListView->SetItemSelection(DefaultObjectCategoryList[*CategoryIndex], true);
	}	
	
	return FReply::Handled();
}

void SCustomCategoryListItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	CategorySetup = InArgs._CategorySetup;
	CategoryIndex = InArgs._CategoryIndex;

	check(CategorySetup.IsValid());
	check(CategoryIndex.IsValid());
	
	SMultiColumnTableRow< TSharedPtr<FCustomCategoryListItem> >::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}

TSharedRef<SWidget> SCustomCategoryListItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == TEXT("CategoryIndex"))
	{
		return	SNew(SBox)
			.HeightOverride(15)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(*CategoryIndex.Get())))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
	}
	else if (ColumnName == TEXT("CategoryName"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromName(CategorySetup->CategoryName))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
	}
	else if (ColumnName == TEXT("UseBlueprintClasses"))
	{
		return SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.HAlign(HAlign_Center)
				.IsChecked((CategorySetup->bUseBlueprintClasses) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.IsEnabled(false)
			];
	}
	//else if (ColumnName == TEXT("UseBlueprintChildClasses"))
	//{
	//	return SNew(SBox)
	//		.HeightOverride(20)
	//		.Padding(FMargin(3, 0))
	//		.VAlign(VAlign_Center)
	//		[
	//			SNew(SCheckBox)
	//			.HAlign(HAlign_Center)
	//			.IsChecked((CategorySetup->bUseBlueprintChildClasses) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
	//			.IsEnabled(false)
	//		];
	//}	
	else if (ColumnName == TEXT("UseCustomColorCategories"))
	{
		return SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.HAlign(HAlign_Center)
				.IsChecked((CategorySetup->bUseCustomColorCategories) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.IsEnabled(false)
			];
	}
	else if (ColumnName == TEXT("ClassItemsCount"))
	{
		FString ItemsCount = FString::FromInt(CategorySetup->Items.Num());
		
		return SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(ItemsCount))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
	}

	return SNullWidget::NullWidget;
}


FText SClassListItem::GetDisplayValueAsString() const
{
	static bool bIsReentrant = false;

	//Guard against re - entrancy which can happen if the delegate executed below(SelectedClass.Get()) forces a slow task dialog to open, thus causing this to lose context and regain focus later starting the loop over again
	if (!bIsReentrant)
	{
		TGuardValue<bool>(bIsReentrant, true);
		return FText::FromString(GetClassDisplayName(SelectedClass.Get()));
	}
	else
	{
		return FText::GetEmpty();
	}

}

void SClassListItem::SendToObjects(const FString& NewValue)
{
	UClass* NewClass = FindObject<UClass>(ANY_PACKAGE, *NewValue);
	if (!NewClass)
	{
		NewClass = LoadObject<UClass>(nullptr, *NewValue);
	}
	onSetClass.Execute(NewClass);
}

void SClassListItem::OnSetClass(const UClass* NewClass)
{
	SelectedClass = NewClass;
}

void SClassListItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	ClassSetup = InArgs._ClassSetup;
	ClassIndex = InArgs._ClassIndex;
	check(ClassSetup.IsValid());
	check(ClassIndex.IsValid());
	SMultiColumnTableRow< TSharedPtr<FClassListItem> >::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}

TSharedRef<SWidget> SClassListItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	SelectedClass = ClassSetup->Get();
	
	FString ClassType = TEXT("Native");
	
	ClassSetup->Get()->IsNative() ? ClassType = TEXT("Native") : ClassType = TEXT("Blueprint");
	
	if (ColumnName == TEXT("ClassIndex"))
	{
		return SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(*ClassIndex.Get())))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
	}
	else if (ColumnName == TEXT("ClassType"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(ClassType))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
	}	
	else if (ColumnName == TEXT("ClassName"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(GetClassDisplayName(ClassSetup->Get())))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			];
	}

	return SNullWidget::NullWidget;
}

#undef LOCTEXT_NAMESPACE
#undef RowWidth_Customization

void SDefaultCategoryListItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView)
{
	CategorySetup = InArgs._CategorySetup;
	CategoryName = InArgs._CategoryName;
	CategoryIndex = InArgs._CategoryIndex;

	check(CategorySetup.IsValid());
	check(CategoryName.IsValid());
	check(CategoryIndex.IsValid());

	SMultiColumnTableRow< TSharedPtr<FDefaultCategoryListItem> >::Construct(FSuperRowType::FArguments(), InOwnerTableView);
}

TSharedRef<SWidget> SDefaultCategoryListItem::GenerateWidgetForColumn(const FName& ColumnName)
{
	if (ColumnName == TEXT("CategoryVisible"))
	{
		return	SNew(SBox)
			//.AutoWidth()
			.WidthOverride(15)
			.HeightOverride(15)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				//.Style(FEditorStyle::Get(), "PlacementBrowser.Tab")
				.IsChecked(CategorySetup->bVisible ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
				.IsEnabled(false)
			];
	}
	else if (ColumnName == TEXT("CategoryIndex"))
	{
		return	SNew(SBox)
			.HeightOverride(15)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromString(FString::FromInt(*CategoryIndex.Get())))
			.Font(IDetailLayoutBuilder::GetDetailFont())
			];
	}
	else if (ColumnName == TEXT("CategoryName"))
	{
		return	SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromName(*CategoryName.Get()))
			.Font(IDetailLayoutBuilder::GetDetailFont())
			];
	}
	else if (ColumnName == TEXT("UseCustomColor"))
	{
		return SNew(SBox)
			.HeightOverride(20)
			.Padding(FMargin(3, 0))
			.VAlign(VAlign_Center)
			[
				SNew(SCheckBox)
				.HAlign(HAlign_Center)
			.IsChecked((CategorySetup->bUseCustomColor) ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
			.IsEnabled(false)
			];
	}

	return SNullWidget::NullWidget;
}
