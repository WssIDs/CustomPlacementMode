
#include "SCustomPlacementModeTools.h"
#include "Application/SlateApplicationBase.h"
#include "Widgets/Layout/SSpacer.h"
#include "Widgets/Images/SImage.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBar.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Input/SCheckBox.h"
#include "EditorStyleSet.h"
//#include "EditorModeManager.h"
#include "EditorModes.h"
#include "AssetThumbnail.h"
#include "LevelEditor.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "EditorClassUtils.h"
#include "Widgets/Input/SSearchBox.h"
#include "CustomPlacementMode.h"
#include "ICustomPlacementModeModule.h"
//#include "ActorFactories/ActorFactory.h"
//#include <ISlateStyle.h>
//#include <Private/SlateEditorStyle.h>
#include "Styles/CustomCategoryStyles.h"


DEFINE_LOG_CATEGORY(CustomPlacementModeToolsLog);

struct FCustomSortPlaceableItems
{
	static bool SortItemsByOrderThenName(const TSharedPtr<FCustomPlaceableItem>& A, const TSharedPtr<FCustomPlaceableItem>& B)
	{
		if (A->SortOrder.IsSet())
		{
			if (B->SortOrder.IsSet())
			{
				return A->SortOrder.GetValue() < B->SortOrder.GetValue();
			}
			else
			{
				return true;
			}
		}
		else if (B->SortOrder.IsSet())
		{
			return false;
		}
		else
		{
			return SortItemsByName(A, B);
		}
	}

	static bool SortItemsByName(const TSharedPtr<FCustomPlaceableItem>& A, const TSharedPtr<FCustomPlaceableItem>& B)
	{
		return A->DisplayName.CompareTo(B->DisplayName) < 0;
	}
};

namespace CustomPlacementViewFilter
{
	void GetBasicStrings(const FCustomPlaceableItem& InPlaceableItem, TArray<FString>& OutBasicStrings)
	{
		OutBasicStrings.Add(InPlaceableItem.DisplayName.ToString());

		const FString* SourceString = FTextInspector::GetSourceString(InPlaceableItem.DisplayName);
		if (SourceString)
		{
			OutBasicStrings.Add(*SourceString);
		}
	}
} // namespace PlacementViewFilter

/**
 * These are the asset thumbnails.
 */
class SCustomPlacementAssetThumbnail : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCustomPlacementAssetThumbnail)
		: _Width(32)
		, _Height(32)
		, _AlwaysUseGenericThumbnail(false)
		, _AssetTypeColorOverride()
	{}

	SLATE_ARGUMENT(uint32, Width)

		SLATE_ARGUMENT(uint32, Height)

		SLATE_ARGUMENT(FName, ClassThumbnailBrushOverride)

		SLATE_ARGUMENT(bool, AlwaysUseGenericThumbnail)

		SLATE_ARGUMENT(TOptional<FLinearColor>, AssetTypeColorOverride)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs, const FAssetData& InAsset)
	{
		Asset = InAsset;

		FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		TSharedPtr<FAssetThumbnailPool> ThumbnailPool = LevelEditorModule.GetFirstLevelEditor()->GetThumbnailPool();

		Thumbnail = MakeShareable(new FAssetThumbnail(Asset, InArgs._Width, InArgs._Height, ThumbnailPool));

		FAssetThumbnailConfig Config;
		Config.bForceGenericThumbnail = InArgs._AlwaysUseGenericThumbnail;
		Config.ClassThumbnailBrushOverride = InArgs._ClassThumbnailBrushOverride;
		Config.AssetTypeColorOverride = InArgs._AssetTypeColorOverride;
		ChildSlot
			[
				Thumbnail->MakeThumbnailWidget(Config)
			];
	}

private:

	FAssetData Asset;
	TSharedPtr< FAssetThumbnail > Thumbnail;
};





void SCustomPlacementAssetEntry::Construct(const FArguments& InArgs, const TSharedPtr<const FCustomPlaceableItem>& InItem)
{
	bIsPressed = false;

	Item = InItem;

	TSharedPtr< SHorizontalBox > ActorType = SNew(SHorizontalBox);

	const bool bIsClass = Item->AssetData.GetClass() == UClass::StaticClass();
	const bool bIsActor = bIsClass ? CastChecked<UClass>(Item->AssetData.GetAsset())->IsChildOf(AActor::StaticClass()) : false;

	AActor* DefaultActor = nullptr;
	if (Item->Factory != nullptr)
	{
		DefaultActor = Item->Factory->GetDefaultActor(Item->AssetData);
	}
	else if (bIsActor)
	{
		DefaultActor = CastChecked<AActor>(CastChecked<UClass>(Item->AssetData.GetAsset())->ClassDefaultObject);
	}

	UClass* DocClass = nullptr;
	TSharedPtr<IToolTip> AssetEntryToolTip;
	if (DefaultActor != nullptr)
	{
		DocClass = DefaultActor->GetClass();
		AssetEntryToolTip = FEditorClassUtils::GetTooltip(DefaultActor->GetClass());
	}

	if (!AssetEntryToolTip.IsValid())
	{
		AssetEntryToolTip = FSlateApplicationBase::Get().MakeToolTip(Item->DisplayName);
	}

	const FButtonStyle& ButtonStyle = FEditorStyle::GetWidgetStyle<FButtonStyle>("PlacementBrowser.Asset");

	NormalImage = &ButtonStyle.Normal;
	HoverImage = &ButtonStyle.Hovered;
	PressedImage = &ButtonStyle.Pressed;

	// Create doc link widget if there is a class to link to
	TSharedRef<SWidget> DocWidget = SNew(SSpacer);
	if (DocClass != NULL)
	{
		DocWidget = FEditorClassUtils::GetDocumentationLinkWidget(DocClass);
		DocWidget->SetCursor(EMouseCursor::Default);
	}

	ChildSlot
		[
			SNew(SBorder)
			.BorderImage(this, &SCustomPlacementAssetEntry::GetBorder)
		.Cursor(EMouseCursor::GrabHand)
		.ToolTip(AssetEntryToolTip)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
		.Padding(0)
		.AutoWidth()
		[
			// Drop shadow border
			SNew(SBorder)
			.Padding(4)
		.BorderImage(FEditorStyle::GetBrush("ContentBrowser.ThumbnailShadow"))
		[
			SNew(SBox)
			.WidthOverride(35)
		.HeightOverride(35)
		[
			SNew(SCustomPlacementAssetThumbnail, Item->AssetData)
			.ClassThumbnailBrushOverride(Item->ClassThumbnailBrushOverride)
		.AlwaysUseGenericThumbnail(Item->bAlwaysUseGenericThumbnail)
		.AssetTypeColorOverride(Item->AssetTypeColorOverride)
		]
		]
		]

	+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.Padding(2, 0, 4, 0)
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
		.Padding(0, 0, 0, 1)
		.AutoHeight()
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "PlacementBrowser.Asset.Name")
		.Text(Item->DisplayName)
		.HighlightText(InArgs._HighlightText)
		]
		]

	+ SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.AutoWidth()
		[
			DocWidget
		]
		]
		];
		
}

FReply SCustomPlacementAssetEntry::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsPressed = true;

		return FReply::Handled().DetectDrag(SharedThis(this), MouseEvent.GetEffectingButton());
	}

	return FReply::Unhandled();
}

FReply SCustomPlacementAssetEntry::OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		bIsPressed = false;
	}

	return FReply::Unhandled();
}

FReply SCustomPlacementAssetEntry::OnDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	bIsPressed = false;

	if (FEditorDelegates::OnAssetDragStarted.IsBound())
	{
		TArray<FAssetData> DraggedAssetDatas;
		DraggedAssetDatas.Add(Item->AssetData);
		FEditorDelegates::OnAssetDragStarted.Broadcast(DraggedAssetDatas, Item->Factory);
		return FReply::Handled();
	}

	if (MouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		return FReply::Handled().BeginDragDrop(FAssetDragDropOp::New(Item->AssetData, Item->Factory));
	}
	else
	{
		return FReply::Handled();
	}
}

bool SCustomPlacementAssetEntry::IsPressed() const
{
	return bIsPressed;
}

const FSlateBrush* SCustomPlacementAssetEntry::GetBorder() const
{
	if (IsPressed())
	{
		return PressedImage;
	}
	else if (IsHovered())
	{
		return HoverImage;
	}
	else
	{
		return NormalImage;
	}
}





SCustomPlacementModeTools::~SCustomPlacementModeTools()
{
	if (ICustomPlacementModeModule::IsAvailable())
	{
		ICustomPlacementModeModule::Get().OnRecentlyPlacedChanged().RemoveAll(this);
		ICustomPlacementModeModule::Get().OnAllPlaceableAssetsChanged().RemoveAll(this);
		ICustomPlacementModeModule::Get().OnAllCustomPlaceableAssetsChanged().RemoveAll(this);
	}
}

void SCustomPlacementModeTools::Construct(const FArguments& InArgs)
{
	bCustomPlaceablesRefreshRequested = false;
	bPlaceablesFullRefreshRequested = false;
	bRecentlyPlacedRefreshRequested = false;
	bNeedsUpdate = true;

	FCustomPlacementMode* PlacementEditMode = (FCustomPlacementMode*)GLevelEditorModeTools().GetActiveMode(FBuiltinEditorModes::EM_Placement);
	PlacementEditMode->AddValidFocusTargetForPlacement(SharedThis(this));

	SearchTextFilter = MakeShareable(new FPlacementAssetEntryTextFilter(
		FPlacementAssetEntryTextFilter::FItemToStringArray::CreateStatic(&CustomPlacementViewFilter::GetBasicStrings)
	));

	TSharedRef<SVerticalBox> Tabs = SNew(SVerticalBox).Visibility(this, &SCustomPlacementModeTools::GetTabsVisibility);
	
	// Populate the tabs and body from the defined placeable items
	ICustomPlacementModeModule& PlacementModeModule = ICustomPlacementModeModule::Get();

	TWeakObjectPtr< UCustomPlacementModeSettings > Settings = nullptr;

#if WITH_EDITOR
	Settings = GetMutableDefault<UCustomPlacementModeSettings>();
#endif

	TArray<FCustomPlacementCategoryInfo> Categories;
	PlacementModeModule.GetSortedCategories(Categories);
	for (const FCustomPlacementCategoryInfo& Category : Categories)
	{
		if (IsCutomCategory(Category))
		{
			FSettingPlaceableCategoryItem* SettingCategoryItem = Settings->PlaceableCategoryItems.FindByPredicate([Category](const FSettingPlaceableCategoryItem& Item)
			{
				return Item.CategoryName == Category.UniqueHandle;
			});
			
			if(SettingCategoryItem)
			{
				if(SettingCategoryItem->bUseCustomColorCategories)
				{
					Tabs->AddSlot()
						.AutoHeight()
						[
							CreateCustomPlacementGroupTab(Category)
						];
				}
				else
				{
					Tabs->AddSlot()
						.AutoHeight()
						[
							CreatePlacementGroupTab(Category)
						];
				}
			}
			else
			{
				Tabs->AddSlot()
					.AutoHeight()
					[
						CreatePlacementGroupTab(Category)
					];
			}
		}
		else
		{
			if(Category.UniqueHandle == FCustomBuiltInPlacementCategories::RecentlyPlaced())
			{
				Tabs->AddSlot()
					.AutoHeight()
					[	
						GenerateColorsPlacementGroupTab(Settings->RecentlyPlaced,Category)
					];
			}
			else if(Category.UniqueHandle == FCustomBuiltInPlacementCategories::Basic())
			{
				Tabs->AddSlot()
					.AutoHeight()
					[
						GenerateColorsPlacementGroupTab(Settings->Basic,Category)
					];
			}
			else if(Category.UniqueHandle == FCustomBuiltInPlacementCategories::Lights())
			{
				Tabs->AddSlot()
					.AutoHeight()
					[
						GenerateColorsPlacementGroupTab(Settings->Lights,Category)
					];
			}
			else if(Category.UniqueHandle == FCustomBuiltInPlacementCategories::Visual())
			{
				Tabs->AddSlot()
					.AutoHeight()
					[
						GenerateColorsPlacementGroupTab(Settings->Visual,Category)
					];
			}
			else if(Category.UniqueHandle == FCustomBuiltInPlacementCategories::Volumes())
			{
				Tabs->AddSlot()
					.AutoHeight()
					[
						GenerateColorsPlacementGroupTab(Settings->Volumes,Category)
					];
			}
			else if(Category.UniqueHandle == FCustomBuiltInPlacementCategories::AllClasses())
			{
				Tabs->AddSlot()
					.AutoHeight()
					[
						GenerateColorsPlacementGroupTab(Settings->AllClasses,Category)
					];
			}
			else
			{
				UE_LOG(CustomPlacementModeToolsLog, Error, TEXT("%s - undefined category name (Category tab not create)"),*Category.UniqueHandle.ToString());
			}
		}
	}
	
	TSharedRef<SScrollBar> ScrollBar = SNew(SScrollBar)
		.Thickness(FVector2D(5, 5));

	ChildSlot
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
		.Padding(4)
		.AutoHeight()
		[
			SAssignNew(SearchBoxPtr, SSearchBox)
			.HintText(NSLOCTEXT("PlacementMode", "SearchPlaceables", "Search Classes"))
		.OnTextChanged(this, &SCustomPlacementModeTools::OnSearchChanged)
		.OnTextCommitted(this, &SCustomPlacementModeTools::OnSearchCommitted)
		]

	+ SVerticalBox::Slot()
		.Padding(0)
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			Tabs
		]

	+ SHorizontalBox::Slot()
		[
			SNew(SBorder)
			.Padding(FMargin(3))
		.BorderImage(FEditorStyle::GetBrush("ToolPanel.DarkGroupBorder"))
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Fill)
		[
			SNew(STextBlock)
			.Text(NSLOCTEXT("PlacementMode", "NoResultsFound", "No Results Found"))
		.Visibility(this, &SCustomPlacementModeTools::GetFailedSearchVisibility)
		]

	+ SOverlay::Slot()
		[
			SAssignNew(CustomContent, SBox)
		]

	+ SOverlay::Slot()
		[
			SAssignNew(DataDrivenContent, SBox)
			[
				SNew(SHorizontalBox)

				+ SHorizontalBox::Slot()
		[
			SAssignNew(ListView, SListView<TSharedPtr<FCustomPlaceableItem>>)
			.ListItemsSource(&FilteredItems)
		.OnGenerateRow(this, &SCustomPlacementModeTools::OnGenerateWidgetForItem)
		.ExternalScrollbar(ScrollBar)
		]

	+ SHorizontalBox::Slot()
		.AutoWidth()
		[
			ScrollBar
		]
			]
		]
		]
		]
		]
		];

	ActiveTabName = FCustomBuiltInPlacementCategories::Basic();
	bNeedsUpdate = true;

	PlacementModeModule.OnRecentlyPlacedChanged().AddSP(this, &SCustomPlacementModeTools::UpdateRecentlyPlacedAssets);
	PlacementModeModule.OnAllPlaceableAssetsChanged().AddSP(this, &SCustomPlacementModeTools::UpdatePlaceableAssets);
	PlacementModeModule.OnAllCustomPlaceableAssetsChanged().AddSP(this, &SCustomPlacementModeTools::UpdateCustomPlaceableAssets);

}

TSharedRef< SWidget > SCustomPlacementModeTools::CreatePlacementGroupTab(const FCustomPlacementCategoryInfo& Info)
{
	// CustomPlacementBrowser.Tab
	//.Style(FEditorStyle::Get(), "PlacementBrowser.Tab")

	return SNew(SCheckBox)
		.Style(FEditorStyle::Get(), "PlacementBrowser.Tab")
		.OnCheckStateChanged(this, &SCustomPlacementModeTools::OnPlacementTabChanged, Info.UniqueHandle)
		.IsChecked(this, &SCustomPlacementModeTools::GetPlacementTabCheckedState, Info.UniqueHandle)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		[
			SNew(SSpacer)
			.Size(FVector2D(1, 30))
		]

	+ SOverlay::Slot()
		.Padding(FMargin(6, 0, 15, 0))
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(FEditorStyle::Get(), "PlacementBrowser.Tab.Text")
		.Text(Info.DisplayName)
		]

	+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Left)
		[
			SNew(SImage)
			.Image(this, &SCustomPlacementModeTools::PlacementGroupBorderImage, Info.UniqueHandle)
		]
		];
}

TSharedRef< SWidget > SCustomPlacementModeTools::CreateCustomPlacementGroupTab(const FCustomPlacementCategoryInfo& Info)
{
	// CustomPlacementBrowser.Tab
	//.Style(FEditorStyle::Get(), "PlacementBrowser.Tab")
	FString Tab = TEXT("CustomPlacementBrowser.Tab");
	FString TabText = TEXT("CustomPlacementBrowser.Tab.Text");

	FName PlacementTab = FName(*Tab.Append(Info.UniqueHandle.ToString()));
	FName PlacementTabText = FName(*TabText.Append(Info.UniqueHandle.ToString()));

	return SNew(SCheckBox)
		.Style(FCustomCategoryStyle::Get(), PlacementTab)
		//.Style(FEditorStyle::Get(), "PlacementBrowser.Tab")
		.OnCheckStateChanged(this, &SCustomPlacementModeTools::OnPlacementTabChanged, Info.UniqueHandle)
		.IsChecked(this, &SCustomPlacementModeTools::GetPlacementTabCheckedState, Info.UniqueHandle)
		[
			SNew(SOverlay)

			+ SOverlay::Slot()
		.VAlign(VAlign_Center)
		[
			SNew(SSpacer)
			.Size(FVector2D(1, 30))
		]

	+ SOverlay::Slot()
		.Padding(FMargin(6, 0, 15, 0))
		.VAlign(VAlign_Center)
		[
			SNew(STextBlock)
			.TextStyle(FCustomCategoryStyle::Get(), PlacementTabText)
		.Text(Info.DisplayName)
		]

	+ SOverlay::Slot()
		.VAlign(VAlign_Fill)
		.HAlign(HAlign_Left)
		[
			SNew(SImage)
			.Image(this, &SCustomPlacementModeTools::PlacementGroupBorderImage, Info.UniqueHandle)
		]
		];
}


TSharedRef< SWidget > SCustomPlacementModeTools::GenerateColorsPlacementGroupTab(const FSettingDefaultPlaceableCategoryItem Item, const FCustomPlacementCategoryInfo& Info)
{
	if(Item.bUseCustomColor)
	{
		return CreateCustomPlacementGroupTab(Info);
	}
	else
	{
		return CreatePlacementGroupTab(Info);
	}
}

FName SCustomPlacementModeTools::GetActiveTab() const
{
	return IsSearchActive() ? FCustomBuiltInPlacementCategories::AllClasses() : ActiveTabName;
}

void SCustomPlacementModeTools::UpdateFilteredItems()
{
	bNeedsUpdate = false;

	ICustomPlacementModeModule& PlacementModeModule = ICustomPlacementModeModule::Get();

	const FCustomPlacementCategoryInfo* Category = PlacementModeModule.GetRegisteredPlacementCategory(GetActiveTab());
	if (!Category)
	{
		return;
	}
	else if (Category->CustomGenerator)
	{
		CustomContent->SetContent(Category->CustomGenerator());

		CustomContent->SetVisibility(EVisibility::Visible);
		DataDrivenContent->SetVisibility(EVisibility::Collapsed);
	}
	else
	{
		FilteredItems.Reset();

		const FCustomPlacementCategoryInfo* CategoryInfo = PlacementModeModule.GetRegisteredPlacementCategory(GetActiveTab());
		if (!ensure(CategoryInfo))
		{
			return;
		}

		if (IsSearchActive())
		{
			auto Filter = [&](const TSharedPtr<FCustomPlaceableItem>& Item) { return SearchTextFilter->PassesFilter(*Item); };
			PlacementModeModule.GetFilteredItemsForCategory(CategoryInfo->UniqueHandle, FilteredItems, Filter);

			if (CategoryInfo->bSortable)
			{
				FilteredItems.Sort(&FCustomSortPlaceableItems::SortItemsByName);
			}
		}
		else
		{
			PlacementModeModule.GetItemsForCategory(CategoryInfo->UniqueHandle, FilteredItems);

			if (CategoryInfo->bSortable)
			{
				FilteredItems.Sort(&FCustomSortPlaceableItems::SortItemsByOrderThenName);
			}
		}

		CustomContent->SetVisibility(EVisibility::Collapsed);
		DataDrivenContent->SetVisibility(EVisibility::Visible);
		ListView->RequestListRefresh();
	}
}

bool SCustomPlacementModeTools::IsSearchActive() const
{
	return !SearchTextFilter->GetRawFilterText().IsEmpty();
}

ECheckBoxState SCustomPlacementModeTools::GetPlacementTabCheckedState(FName CategoryName) const
{
	return ActiveTabName == CategoryName ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
}

bool SCustomPlacementModeTools::IsCutomCategory(FCustomPlacementCategoryInfo Category)
{
	FName CategoryName = Category.UniqueHandle;

	if (CategoryName != FCustomBuiltInPlacementCategories::RecentlyPlaced() &&
		CategoryName != FCustomBuiltInPlacementCategories::Basic() &&
		CategoryName != FCustomBuiltInPlacementCategories::Lights() &&
		CategoryName != FCustomBuiltInPlacementCategories::Visual() &&
		CategoryName != FCustomBuiltInPlacementCategories::Volumes() &&
		CategoryName != FCustomBuiltInPlacementCategories::AllClasses()
		)
	{
		return true;
	}

	return false;
}

EVisibility SCustomPlacementModeTools::GetFailedSearchVisibility() const
{
	if (!IsSearchActive() || FilteredItems.Num())
	{
		return EVisibility::Collapsed;
	}
	return EVisibility::Visible;
}

//EVisibility SCustomPlacementModeTools::GetListViewVisibility() const
//{
//
//}

EVisibility SCustomPlacementModeTools::GetTabsVisibility() const
{
	return IsSearchActive() ? EVisibility::Collapsed : EVisibility::Visible;
}

TSharedRef<ITableRow> SCustomPlacementModeTools::OnGenerateWidgetForItem(TSharedPtr<FCustomPlaceableItem> InItem, const TSharedRef<STableViewBase>& OwnerTable)
{
	return SNew(STableRow<TSharedPtr<FCustomPlaceableItem>>, OwnerTable)
		[
			SNew(SCustomPlacementAssetEntry, InItem.ToSharedRef())
			.HighlightText(this, &SCustomPlacementModeTools::GetHighlightText)
		];
}

void SCustomPlacementModeTools::OnPlacementTabChanged(ECheckBoxState NewState, FName CategoryName)
{
	if (NewState == ECheckBoxState::Checked)
	{
		ActiveTabName = CategoryName;
		
		UE_LOG(CustomPlacementModeToolsLog,Log,TEXT("TabName - %s"),*ActiveTabName.ToString());
		ICustomPlacementModeModule::Get().RegenerateItemsForCategory(ActiveTabName);

		bNeedsUpdate = true;
	}
}

const FSlateBrush* SCustomPlacementModeTools::PlacementGroupBorderImage(FName CategoryName) const
{
	if (ActiveTabName == CategoryName)
	{
		static FName PlacementBrowserActiveTabBarBrush("PlacementBrowser.ActiveTabBar");
		return FEditorStyle::GetBrush(PlacementBrowserActiveTabBarBrush);
	}
	else
	{
		return nullptr;
	}
}

void SCustomPlacementModeTools::UpdateRecentlyPlacedAssets(const TArray< FActorCustomPlacementInfo >& RecentlyPlaced)
{
	if (GetActiveTab() == FCustomBuiltInPlacementCategories::RecentlyPlaced())
	{
		bRecentlyPlacedRefreshRequested = true;
	}
}

void SCustomPlacementModeTools::UpdatePlaceableAssets()
{
	if (GetActiveTab() == FCustomBuiltInPlacementCategories::AllClasses())
	{
		bPlaceablesFullRefreshRequested = true;
	}
}

void SCustomPlacementModeTools::UpdateCustomPlaceableAssets()
{
	FName CategoryTab = GetActiveTab();

	if (CategoryTab != FCustomBuiltInPlacementCategories::RecentlyPlaced() ||
		CategoryTab != FCustomBuiltInPlacementCategories::Basic() ||
		CategoryTab != FCustomBuiltInPlacementCategories::Lights() ||
		CategoryTab != FCustomBuiltInPlacementCategories::Visual() ||
		CategoryTab != FCustomBuiltInPlacementCategories::Volumes() ||
		CategoryTab != FCustomBuiltInPlacementCategories::AllClasses()
		)
	{
		bCustomPlaceablesRefreshRequested = true;
	}
}

void SCustomPlacementModeTools::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (bPlaceablesFullRefreshRequested)
	{
		ICustomPlacementModeModule::Get().RegenerateItemsForCategory(FCustomBuiltInPlacementCategories::AllClasses());
		bPlaceablesFullRefreshRequested = false;
		bNeedsUpdate = true;
	}

	if (bRecentlyPlacedRefreshRequested)
	{
		ICustomPlacementModeModule::Get().RegenerateItemsForCategory(FCustomBuiltInPlacementCategories::RecentlyPlaced());
		bRecentlyPlacedRefreshRequested = false;
		bNeedsUpdate = true;
	}

	if (bCustomPlaceablesRefreshRequested)
	{
		ICustomPlacementModeModule::Get().RegenerateItemsForCategory(FCustomBuiltInPlacementCategories::Custom());
		//ICustomPlacementModeModule::Get().RegenerateItemsForCategory(ActiveTabName);
		bCustomPlaceablesRefreshRequested = false;
		bNeedsUpdate = true;
	}

	if (bNeedsUpdate)
	{
		UpdateFilteredItems();
	}
}

FReply SCustomPlacementModeTools::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent)
{
	FReply Reply = FReply::Unhandled();

	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		FCustomPlacementMode* PlacementEditMode = (FCustomPlacementMode*)GLevelEditorModeTools().GetActiveMode(FBuiltinEditorModes::EM_Placement);
		PlacementEditMode->StopPlacing();
		Reply = FReply::Handled();
	}

	return Reply;
}

void SCustomPlacementModeTools::OnSearchChanged(const FText& InFilterText)
{
	// If the search text was previously empty we do a full rebuild of our cached widgets
	// for the placeable widgets.
	if (!IsSearchActive())
	{
		bPlaceablesFullRefreshRequested = true;
		bCustomPlaceablesRefreshRequested = true;
	}
	else
	{
		bNeedsUpdate = true;
	}

	SearchTextFilter->SetRawFilterText(InFilterText);
	SearchBoxPtr->SetError(SearchTextFilter->GetFilterErrorText());
}

void SCustomPlacementModeTools::OnSearchCommitted(const FText& InFilterText, ETextCommit::Type InCommitType)
{
	OnSearchChanged(InFilterText);
}

FText SCustomPlacementModeTools::GetHighlightText() const
{
	return SearchTextFilter->GetRawFilterText();
}