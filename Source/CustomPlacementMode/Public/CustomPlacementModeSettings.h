#pragma once

#include "Engine/Engine.h"
#include "IPropertyTypeCustomization.h"
#include <IDetailCustomization.h>
#include <STableRow.h>
#include <STableViewBase.h>
#include <SListView.h>
#include "Input/Reply.h"
#include <PropertyCustomizationHelpers.h>
#include "ClassViewerFilter.h"
#include "SlateOptMacros.h"
#include "ClassViewerModule.h"
#include "CustomPlacementModeSettings.generated.h"

class IDetailLayoutBuilder;
class SNotificationItem;
class SColorBlock;

template <typename ItemType> class SListView;


class FCustomPropertyEditorClassFilter : public IClassViewerFilter
{
public:
	/** The meta class for the property that classes must be a child-of. */
	const UClass* ClassPropertyMetaClass;

	/** The interface that must be implemented. */
	const UClass* InterfaceThatMustBeImplemented;

	/** Whether or not abstract classes are allowed. */
	bool bAllowAbstract;

	bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs ) override
	{
		bool bMatchesFlags = !InClass->HasAnyClassFlags(CLASS_Hidden|CLASS_HideDropDown|CLASS_Deprecated) &&
			(bAllowAbstract || !InClass->HasAnyClassFlags(CLASS_Abstract));

		if(bMatchesFlags && InClass->IsChildOf(ClassPropertyMetaClass)
			&& (!InterfaceThatMustBeImplemented || InClass->ImplementsInterface(InterfaceThatMustBeImplemented)))
		{
			return true;
		}

		return false;
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef< const IUnloadedBlueprintData > InClass, TSharedRef< FClassViewerFilterFuncs > InFilterFuncs) override
	{
		bool bMatchesFlags = !InClass->HasAnyClassFlags(CLASS_Hidden|CLASS_HideDropDown|CLASS_Deprecated) &&
			(bAllowAbstract || !InClass->HasAnyClassFlags(CLASS_Abstract));

		if(bMatchesFlags && InClass->IsChildOf(ClassPropertyMetaClass)
			&& (!InterfaceThatMustBeImplemented || InClass->ImplementsInterface(InterfaceThatMustBeImplemented)))
		{
			return true;
		}

		return false;
	}
};



USTRUCT(BlueprintType, Blueprintable)
struct FColorCategoryItem
{
	GENERATED_BODY()

public:

	/** The color used to represent no selection */
	UPROPERTY(EditAnywhere, config, AdvancedDisplay, meta = (DisplayName = "Background Color"))
		FLinearColor BackgroundColor;

	/** The color used to represent selection */
	UPROPERTY(EditAnywhere, config, AdvancedDisplay, meta = (DisplayName = "Hovered Color"))
		FLinearColor HoveredColor;

	/** The color used to represent a pressed item */
	UPROPERTY(EditAnywhere, config, AdvancedDisplay, meta = (DisplayName = "Pressed Selection Color"))
		FLinearColor PressedSelectionColor;

	/** The text color category */
	UPROPERTY(EditAnywhere, config, AdvancedDisplay, meta = (DisplayName = "Text Color"))
		FLinearColor TextColor;

	/** The text shadow color category */
	UPROPERTY(EditAnywhere, config, AdvancedDisplay, meta = (DisplayName = "Text Shadow Color", ConfigRestartRequired = true))
		FLinearColor TextShadowColor;

	FColorCategoryItem()
	{
		BackgroundColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.08f);
		HoveredColor = FLinearColor(0.03f, 0.03f, 0.03f, 1.0f);
		PressedSelectionColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
		TextColor = FLinearColor(1.0f, 1.0f, 1.0f, 0.9f);
		TextShadowColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.9f);
	}
};

USTRUCT(BlueprintType,Blueprintable)
struct FSettingPlaceableCategoryItem
{
	GENERATED_BODY()

public:

	UPROPERTY(VisibleAnywhere, config, Category = "Custom Categories")
		uint32 bVisible;

	UPROPERTY(VisibleAnywhere, config, Category = "Custom Categories")
		FGuid CategoryID;

	UPROPERTY(EditAnywhere, config, Category = "Custom Categories", meta = (DisplayName = "Category Name"))
		FName CategoryName;

	/* Add Blueprint classes to custom category. 1-2 seconds frieze when starting the editor */
	UPROPERTY(EditAnywhere, config, Category = "Custom Categories")
		uint32 bUseBlueprintClasses : 1;

	/* Add Blueprint classes to custom category. 1-2 seconds frieze when starting the editor */
	UPROPERTY(EditAnywhere, config, Category = "Custom Categories")
		uint32 bUseBlueprintChildClasses : 1;

	/* Allow color scheme for custom categories */
	UPROPERTY(EditAnywhere, config, Category = "Custom Categories", meta = (ConfigRestartRequired = true, DisplayName = "Use color scheme for custom categories"))
		uint32 bUseCustomColorCategories : 1;

	/** List of classes and their children displayed in this category */
	UPROPERTY(EditAnywhere, config, meta = (OnlyPlaceable , AllowAbstract = true, ConfigRestartRequired = true))
		TSet<TSubclassOf<AActor>> Items;

	/** Color scheme settings used category */
	UPROPERTY(EditAnywhere, config, AdvancedDisplay, meta = (DisplayName = "Tab Color", EditCondition = "bUseCustomColorCategories"))
		FColorCategoryItem ColorSetting;

	bool operator==(const FSettingPlaceableCategoryItem& Other) const
	{
		return (CategoryName == Other.CategoryName && CategoryID == Other.CategoryID);
	}

	inline friend uint32 GetTypeHash(const FSettingPlaceableCategoryItem& In)
	{
		return GetTypeHash(In.CategoryName); 
	}

	FSettingPlaceableCategoryItem()
	{
		CategoryID = FGuid::NewGuid();
		CategoryName = TEXT("Default");
		//Items.Add(nullptr);
	};
};


USTRUCT(BlueprintType,Blueprintable)
struct FSettingDefaultPlaceableCategoryItem
{
	GENERATED_BODY()

public:	

	UPROPERTY(VisibleAnywhere, config, Category = "Default Categories")
		uint32 bVisible;

	/* Allow color scheme for default categories */
	UPROPERTY(EditAnywhere, config, Category = "Default Categories", meta = (ConfigRestartRequired = true, DisplayName = "Use color scheme"))
	uint32 bUseCustomColor:1;
	
	/** The color used to represent no selection */
	UPROPERTY(EditAnywhere, config, meta = (DisplayName = "Background Color", EditCondition = "bUseCustomColor"))
		FLinearColor BackgroundColor;

	/** The color used to represent selection */
	UPROPERTY(EditAnywhere, config, meta = (DisplayName = "Hovered Color", EditCondition = "bUseCustomColor"))
		FLinearColor HoveredColor;

	/** The color used to represent a pressed item */
	UPROPERTY(EditAnywhere, config, meta = (DisplayName = "Pressed Selection Color", EditCondition = "bUseCustomColor"))
		FLinearColor PressedSelectionColor;

	/** The text color category */
	UPROPERTY(EditAnywhere, config, meta = (DisplayName = "Text Color", EditCondition = "bUseCustomColor"))
		FLinearColor TextColor;

	/** The text shadow color category */
	UPROPERTY(EditAnywhere, config, meta = (DisplayName = "Text Shadow Color", EditCondition = "bUseCustomColor"))
		FLinearColor TextShadowColor;

	FSettingDefaultPlaceableCategoryItem()
	{
		BackgroundColor = FLinearColor(0.1f, 0.1f, 0.1f, 0.08f);
		HoveredColor = FLinearColor(0.03f, 0.03f, 0.03f, 1.0f);
		PressedSelectionColor = FLinearColor(0.02f, 0.02f, 0.02f, 1.0f);
		TextColor = FLinearColor(1.0f,1.0f,1.0f,0.9f);
		TextShadowColor = FLinearColor(0.0f,0.0f,0.0f,0.9f);
	};
};



/**
* Setting object used to hold both config settings and editable ones in one place
* To ensure the settings are saved to the specified config file make sure to add
* props using the globalconfig or config meta.
*/
UCLASS(config = EditorPerProjectUserSettings)
class CUSTOMPLACEMENTMODE_API UCustomPlacementModeSettings : public UObject
{
	GENERATED_BODY()

public:
	UCustomPlacementModeSettings();
	
	//UPROPERTY(config, EditAnywhere, Category = "Custom Categories", meta = (ConfigRestartRequired = true))
	//TArray<FSettingPlaceableCategoryItem> TestCategoryItems;

	/** Custom Category list */
	UPROPERTY(config, EditAnywhere, Category = "Custom Categories", meta = (ConfigRestartRequired = true))
	TArray<FSettingPlaceableCategoryItem> PlaceableCategoryItems;

	TArray<FSettingPlaceableCategoryItem> TempPlaceableCategoryItems;

	/** RecentlyPlaced category */
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true))
	FSettingDefaultPlaceableCategoryItem RecentlyPlaced;
	
	/** Basic category */
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true))
	FSettingDefaultPlaceableCategoryItem Basic;
	
	/** Lights category */
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true))
	FSettingDefaultPlaceableCategoryItem Lights;
	
	/** Visual category */	
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true))
	FSettingDefaultPlaceableCategoryItem Visual;

	/** Volumes category */
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true))
	FSettingDefaultPlaceableCategoryItem Volumes;

	/** AllClassed category */
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true))
	FSettingDefaultPlaceableCategoryItem AllClasses;

	UPROPERTY(config, EditAnywhere, Category = "Experimental", meta = (ConfigRestartRequired = true))
	uint32 bShowCategoryID : 1;

	UPROPERTY(config, EditAnywhere, Category = "Experimental", meta = (ConfigRestartRequired = true))
	uint32 bShowRefreshButtons : 1;

	/** Allow Abstract classes into ClassViewer */
	UPROPERTY(config, EditAnywhere, Category = "Experimental", AdvancedDisplay, meta = (ConfigRestartRequired = true))
	uint32 bAllowAbstract : 1;

	/** Parent class to show child classes into ClassViewer */
	UPROPERTY(config, EditAnywhere, Category = "Experimental", AdvancedDisplay, meta = (ConfigRestartRequired = true))
	TSubclassOf<AActor> ParentClass = AActor::StaticClass();

	/** use tree display mode in ClassViewer */
	UPROPERTY(config, EditAnywhere, Category = "Experimental", AdvancedDisplay, meta = (ConfigRestartRequired = true))
	uint32 bUseDisplayTree : 1;;

	virtual void PreEditChange(class FEditPropertyChain & PropertyAboutToChange) override;

	/**
	* This alternate version of PostEditChange is called when properties inside structs are modified.  The property that was actually modified
	* is located at the tail of the list.  The head of the list of the UStructProperty member variable that contains the property that was modified.
	*/
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	
	void CheckTestItems(UProperty* Property);

	/** Accessor and initializer **/
	static UCustomPlacementModeSettings* Get();

private:
	TSharedPtr<SNotificationItem> NotificationSetAddDublicateCategoryName;
};


/** Util to give better names for BP generated classes */
static FString GetClassDisplayName(const UObject* Object)
{
	const UClass* Class = Cast<UClass>(Object);
	if (Class != NULL)
	{
		UBlueprint* BP = UBlueprint::GetBlueprintFromClass(Class);
		if (BP != NULL)
		{
			return BP->GetName();
		}
	}
	return (Object) ? Object->GetName() : "None";
}

// Class containing the friend information - used to build the list view
class FCustomCategoryListItem
{
public:

	/**
	* Constructor takes the required details
	*/
	FCustomCategoryListItem(TSharedPtr<FSettingPlaceableCategoryItem> InCategorySetup, TSharedPtr<int32> InCategoryIndex)
		: CategorySetup(InCategorySetup), CategoryIndex(InCategoryIndex)
	{}

	TSharedPtr<FSettingPlaceableCategoryItem> CategorySetup;
	TSharedPtr<int32> CategoryIndex;
};

/**
* Implements the FriendsList
*/
class SCustomCategoryListItem
	: public SMultiColumnTableRow< TSharedPtr<class FCustomCategoryListItem> >
{
public:

	SLATE_BEGIN_ARGS(SCustomCategoryListItem) { }
	// for now this is okay to be all argument and it's better for now
	// in the future if we can change this in multiple places, we'll have to change this to be attribute
	SLATE_ARGUMENT(TSharedPtr<FSettingPlaceableCategoryItem>, CategorySetup)
	SLATE_ARGUMENT(TSharedPtr<int32>, CategoryIndex)
	SLATE_END_ARGS()

public:

	/**
	* Constructs the application.
	*
	* @param InArgs - The Slate argument list.
	*/
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:

	TSharedPtr<FSettingPlaceableCategoryItem> CategorySetup;
	TSharedPtr<int32> CategoryIndex;
};


typedef  SListView< TSharedPtr< FCustomCategoryListItem > > SCustomCategoryListView;


// Class containing the friend information - used to build the list view
class FDefaultCategoryListItem
{
public:

	/**
	* Constructor takes the required details
	*/
	FDefaultCategoryListItem(TSharedPtr<FSettingDefaultPlaceableCategoryItem> InCategorySetup, TSharedPtr<FName> inCategoryName, TSharedPtr<int32> InCategoryIndex)
		: CategorySetup(InCategorySetup), CategoryName(inCategoryName) , CategoryIndex(InCategoryIndex)
	{}

	TSharedPtr<FSettingDefaultPlaceableCategoryItem> CategorySetup;
	TSharedPtr<FName> CategoryName;
	TSharedPtr<int32> CategoryIndex;
};

/**
* Implements the FriendsList
*/
class SDefaultCategoryListItem
	: public SMultiColumnTableRow< TSharedPtr<class FDefaultCategoryListItem> >
{
public:

	SLATE_BEGIN_ARGS(SDefaultCategoryListItem) { }
	// for now this is okay to be all argument and it's better for now
	// in the future if we can change this in multiple places, we'll have to change this to be attribute
	SLATE_ARGUMENT(TSharedPtr<FSettingDefaultPlaceableCategoryItem>, CategorySetup)
		SLATE_ARGUMENT(TSharedPtr<FName>, CategoryName)
		SLATE_ARGUMENT(TSharedPtr<int32>, CategoryIndex)
		SLATE_END_ARGS()

public:

	/**
	* Constructs the application.
	*
	* @param InArgs - The Slate argument list.
	*/
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:

	TSharedPtr<FSettingDefaultPlaceableCategoryItem> CategorySetup;
	TSharedPtr<FName> CategoryName;
	TSharedPtr<int32> CategoryIndex;
};


typedef  SListView< TSharedPtr< FDefaultCategoryListItem > > SDefaultCategoryListView;


/* *************** */
/* Init Class List */
/* *************** */

// Class containing the friend information - used to build the list view
class FClassListItem
{
public:

	/**
	* Constructor takes the required details
	*/
	FClassListItem(TSharedPtr<TSubclassOf<AActor>> InCategorySetup, TSharedPtr<int32> InCategoryIndex)
		:ClassSetup(InCategorySetup),ClassIndex(InCategoryIndex)
	{}

	TSharedPtr<TSubclassOf<AActor>> ClassSetup;
	TSharedPtr<int32> ClassIndex;
};

/**
* Implements the FriendsList
*/
class SClassListItem
	: public SMultiColumnTableRow< TSharedPtr<class FClassListItem> >
{
public:

	SLATE_BEGIN_ARGS(SClassListItem) { }
	// for now this is okay to be all argument and it's better for now
	// in the future if we can change this in multiple places, we'll have to change this to be attribute
	SLATE_ARGUMENT(TSharedPtr<TSubclassOf<AActor>>, ClassSetup)
	SLATE_ARGUMENT(TSharedPtr<int32>,ClassIndex)
		SLATE_END_ARGS()

public:

	/**
	* Constructs the application.
	*
	* @param InArgs - The Slate argument list.
	*/
	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);
	virtual TSharedRef<SWidget> GenerateWidgetForColumn(const FName& ColumnName) override;

private:

	/**
	 * Generates a class picker with a filter to show only classes allowed to be selected.
	 *
	 * @return The Class Picker widget.
	 */
	//TSharedRef<SWidget> GenerateClassPicker();

	/**
	 * Callback function from the Class Picker for when a Class is picked.
	 *
	 * @param InClass			The class picked in the Class Picker
	 */
	//void OnClassPicked(UClass* InClass);

	/**
	 * Gets the active display value as a string
	 */
	FText GetDisplayValueAsString() const;

	/** Used when the property deals with Classes and will display a Class Picker. */
	TSharedPtr<class SComboButton> ComboButton;
	/** The property editor we were constructed for, or null if we're editing using the construction arguments */
	//TSharedPtr<class FPropertyEditor> PropertyEditor;
	void SendToObjects(const FString& NewValue);

	/** Attribute used to get the currently selected class (required if PropertyEditor == null) */
	TAttribute<const UClass*> SelectedClass;
	/** Delegate used to set the currently selected class (required if PropertyEditor == null) */
	FOnSetClass onSetClass;
	void OnSetClass(const UClass* NewClass);

	TSharedPtr<TSubclassOf<AActor>> ClassSetup;
	TSharedPtr<int32> ClassIndex;
};


typedef  SListView< TSharedPtr< FClassListItem > > SClassListView;

DECLARE_DELEGATE_RetVal_TwoParams(bool, FOnValidateCategory, const FSettingPlaceableCategoryItem*, int32)

BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION
//====================================================================================
// SCategoryEditDialog 
//=====================================================================================

class SCategoryEditDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SCategoryEditDialog)
		: _CategorySetup(NULL)
		, _CategoryIndex(INDEX_NONE)
		, _Settings(NULL)
	{}

	SLATE_ARGUMENT(FSettingPlaceableCategoryItem*, CategorySetup)
		SLATE_ARGUMENT(int32, CategoryIndex)
		SLATE_ARGUMENT(TSharedPtr<SWindow>, WidgetWindow)
		SLATE_ARGUMENT(UCustomPlacementModeSettings*, Settings)
		SLATE_EVENT(FOnValidateCategory, OnValidateCategory)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	// Category ID
	FText GetGuid() const;

	// Category Name
	FText GetName() const;
	void NewNameEntered(const FText& NewText, ETextCommit::Type CommitInfo);
	void OnTextChanged(const FText& NewText);

	// CheckState Blueprint classes
	void OnCheckStateChangedBlueprintClassed(ECheckBoxState State);
	// CheckState Blueprint child classes
	void OnCheckStateChangedBlueprintChildClassed(ECheckBoxState State);
	// 	// CheckState Custom color category
	void OnCheckStateChangedCustomColorCategory(ECheckBoxState State);

	// window handler
	FReply 					OnAccept();
	FReply 					OnCancel();
	bool 					IsAcceptAvailable() const;
	void 					CloseWindow();

	// utility functions
	FSettingPlaceableCategoryItem GetCategorySetup()
	{
		return CategorySetup;
	}

	// utility functions
	int32 GetCategoryIndex()
	{
		return CategoryIndex;
	}

	// data to return
	bool									bApplyChange;

	FSettingPlaceableCategoryItem			CategorySetup;
	int32									CategoryIndex;

private:

	/** The property editor we were constructed for, or null if we're editing using the construction arguments */
	TSharedPtr<class FPropertyEditor> PropertyEditor;

	UCustomPlacementModeSettings* Settings;

	/**
	* Generates a widget for a channel item.
	* @param InItem - the ChannelListItem
	* @param OwnerTable - the owning table
	* @return The table row widget
	*/
	TSharedRef<ITableRow> HandleGenerateClassWidget(TSharedPtr< FClassListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	TWeakPtr<SWindow>						WidgetWindow;
	FOnValidateCategory						OnValidateCategory;
	TSharedPtr<SEditableTextBox>			NameBox;
	TSharedPtr<SWindow>						PickerWindow;

	TSharedPtr<SVerticalBox>				PreviewColorSection;
	TSharedPtr<SCheckBox>					PreviewTab;

	FSlateBrush* BackgroundImage;
	FSlateBrush* BackgroundPressedImage;
	FSlateBrush* HoveredImage;

	// Edit Class Item

	void OnClassListItemDoubleClicked(TSharedPtr< FClassListItem > SelectedItem);
	/**
	 * Callback function from the Class Picker for when a Class is picked.
	 *
	 * @param InClass			The class picked in the Class Picker
	 */
	void OnClassPicked(UClass* InClass);

	// Add Class Item

	FReply OnNewClassItem();
	void OnNewClassPicked(UClass* InClass);

	// Refresh Class Item

	FReply OnRefreshClassItem();
	bool IsClassItemsFill() const;

	// Delete Class Item

	FReply OnDeleteClassItem();
	bool IsAnyClassItemSelected() const;

	// Delete Class Items

	FReply OnDeleteClassItems();

	void RefreshClassList();

	/**
	 * Gets the active display value as a string
	 */
	FText GetDisplayValueAsString() const;

	void SendToObjects(const FString& NewValue);

	/** Attribute used to get the currently selected class (required if PropertyEditor == null) */
	TAttribute<const UClass*> SelectedClass;

	/** Delegate used to set the currently selected class (required if PropertyEditor == null) */
	FOnSetClass onSetClass;
	void OnSetClass(const UClass* NewClass);

	TSharedPtr<SClassListView>	ObjectClassListView;
	TArray< TSharedPtr< FClassListItem > >	ObjectClassList;

	TSharedPtr<SColorBlock>					BackgroundColorBox;
	FReply OnMouseButtonDownBackgroundColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	void OnSetBackgroundColorFromColorPicker(FLinearColor NewColor);

	void OnBackgroundColorPickerCancelled(FLinearColor OriginalColor);
	void OnBackgroundColorPickerInteractiveBegin();
	void OnBackgroundColorPickerInteractiveEnd();
	// Background Color
	FLinearColor GetBackgroundColor() const;

	TSharedPtr<SColorBlock>					HoveredColorBox;
	FReply OnMouseButtonDownHoveredColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	void OnSetHoveredColorFromColorPicker(FLinearColor NewColor);
	void OnHoveredColorPickerCancelled(FLinearColor OriginalColor);
	// Hovered Color
	FLinearColor GetHoveredColor() const;

	TSharedPtr<SColorBlock>					PressedColorBox;
	FReply OnMouseButtonDownPressedColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	void OnSetPressedColorFromColorPicker(FLinearColor NewColor);
	void OnPressedColorPickerCancelled(FLinearColor OriginalColor);
	// Pressed Color
	FLinearColor GetPressedColor() const;

	//TSharedPtr<SColorBlock>					ShadowColorBox;
	//TSharedPtr<SColorBlock>					ShadowColorBox;
	// Text Shadow Color

	TSharedPtr<SColorBlock>					TextColorBox;
	FReply OnMouseButtonDownTextColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	void OnSetTextColorFromColorPicker(FLinearColor NewColor);
	void OnTextColorPickerCancelled(FLinearColor OriginalColor);
	// Text Color
	FLinearColor GetTextColor() const;

	TSharedPtr<SColorBlock>					TextShadowColorBox;
	FReply OnMouseButtonDownTextShadowColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	void OnSetTextShadowColorFromColorPicker(FLinearColor NewColor);
	void OnTextShadowColorPickerCancelled(FLinearColor OriginalColor);
	// Text Shadow Color
	FLinearColor GetTextShadowColor() const;

	// PreviewTab Text Shadow Color
	FSlateColor GetSlateTextColor() const;

	TSharedPtr<SNotificationItem> NotificationSetAddDublicateCategoryName;
};


DECLARE_DELEGATE_RetVal_TwoParams(bool, FOnValidateDefaultCategory, const FSettingDefaultPlaceableCategoryItem*, int32)

//====================================================================================
// SDefaultCategoryEditDialog 
//=====================================================================================

class SDefaultCategoryEditDialog : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SDefaultCategoryEditDialog)
		: _CategorySetup(NULL)
		, _CategoryName(NAME_None)
		, _CategoryIndex(INDEX_NONE)
		, _Settings(NULL)
	{}

	SLATE_ARGUMENT(FSettingDefaultPlaceableCategoryItem*, CategorySetup)
		SLATE_ARGUMENT(FName, CategoryName)
		SLATE_ARGUMENT(int32, CategoryIndex)
		SLATE_ARGUMENT(TSharedPtr<SWindow>, WidgetWindow)
		SLATE_ARGUMENT(UCustomPlacementModeSettings*, Settings)
		//SLATE_EVENT(FOnValidateDefaultCategory, OnValidateDefaultCategory)
	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

	// Category Name
	FText GetName() const;

	// 	// CheckState Custom color category
	void OnCheckStateChangedCustomColorCategory(ECheckBoxState State);

	void OnCheckStateChangedCategoryVisible(ECheckBoxState State);

	// window handler
	FReply 					OnAccept();
	FReply 					OnCancel();
	void 					CloseWindow();

	// utility functions
	FSettingDefaultPlaceableCategoryItem GetCategorySetup()
	{
		return CategorySetup;
	}

	// utility functions
	int32 GetCategoryIndex()
	{
		return CategoryIndex;
	}
	
	FName GetCategoryName()
	{
		return CategoryName;
	}

	// data to return
	bool									bApplyChange;

	FSettingDefaultPlaceableCategoryItem	CategorySetup;
	FName									CategoryName;
	int32									CategoryIndex;

private:

	/** The property editor we were constructed for, or null if we're editing using the construction arguments */
	TSharedPtr<class FPropertyEditor> PropertyEditor;

	UCustomPlacementModeSettings* Settings;

	TWeakPtr<SWindow>						WidgetWindow;
	//FOnValidateDefaultCategory						OnValidateDefaultCategory;
	TSharedPtr<STextBlock>			NameBox;
	TSharedPtr<SWindow>						PickerWindow;

	TSharedPtr<SVerticalBox>				PreviewColorSection;
	TSharedPtr<SCheckBox>					PreviewTab;

	FSlateBrush* BackgroundImage;
	FSlateBrush* BackgroundPressedImage;
	FSlateBrush* HoveredImage;


	TSharedPtr<SColorBlock>					BackgroundColorBox;
	FReply OnMouseButtonDownBackgroundColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	void OnSetBackgroundColorFromColorPicker(FLinearColor NewColor);
	void OnBackgroundColorPickerCancelled(FLinearColor OriginalColor);
	void OnBackgroundColorPickerInteractiveBegin();
	void OnBackgroundColorPickerInteractiveEnd();
	// Background Color
	FLinearColor GetBackgroundColor() const;

	TSharedPtr<SColorBlock>					HoveredColorBox;
	FReply OnMouseButtonDownHoveredColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	void OnSetHoveredColorFromColorPicker(FLinearColor NewColor);
	void OnHoveredColorPickerCancelled(FLinearColor OriginalColor);
	// Hovered Color
	FLinearColor GetHoveredColor() const;

	TSharedPtr<SColorBlock>					PressedColorBox;
	FReply OnMouseButtonDownPressedColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	void OnSetPressedColorFromColorPicker(FLinearColor NewColor);
	void OnPressedColorPickerCancelled(FLinearColor OriginalColor);
	// Pressed Color
	FLinearColor GetPressedColor() const;


	TSharedPtr<SColorBlock>					TextColorBox;
	FReply OnMouseButtonDownTextColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	void OnSetTextColorFromColorPicker(FLinearColor NewColor);
	void OnTextColorPickerCancelled(FLinearColor OriginalColor);
	// Text Color
	FLinearColor GetTextColor() const;

	TSharedPtr<SColorBlock>					TextShadowColorBox;
	FReply OnMouseButtonDownTextShadowColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	void OnSetTextShadowColorFromColorPicker(FLinearColor NewColor);
	void OnTextShadowColorPickerCancelled(FLinearColor OriginalColor);
	// Text Shadow Color
	FLinearColor GetTextShadowColor() const;

	// PreviewTab Text Shadow Color
	FSlateColor GetSlateTextColor() const;
};


END_SLATE_FUNCTION_BUILD_OPTIMIZATION

class FSettingPlaceableCategoryItemCustomization : public IDetailCustomization
{

public:
	static TSharedRef<IDetailCustomization> MakeInstance();
		
	/** IPropertyTypeCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;


	void GenerateDefaultCategory(IDetailLayoutBuilder& DetailBuilder);

private:
	/**
	* Generates a widget for a channel item.
	* @param InItem - the ChannelListItem
	* @param OwnerTable - the owning table
	* @return The table row widget
	*/
	TSharedRef<ITableRow> HandleGenerateCategoryWidget(TSharedPtr< FCustomCategoryListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	FReply	OnNewCategory();
	FReply	OnEditCategory();
	bool	IsAnyCategorySelected() const;
	FReply	OnDeleteCategory();
	bool	IsAnyCategorySelectedCanUp() const;
	FReply	OnUpCategory();
	bool	IsAnyCategorySelectedCanDown() const;
	FReply	OnDownCategory();

	bool IsCategoryItemsFill() const;
	FReply OnDeleteCategoryItems();
	
	/* Refresh category */
	FReply	OnRefreshCategory();

	bool	IsValidCategorySetup(const FSettingPlaceableCategoryItem* Category, int32 CategoryIndex) const;

	void RefreshCategoryList();

	UCustomPlacementModeSettings* Settings;

	void UpdateCategory();
	FSettingPlaceableCategoryItem * FindFromCategory(FSettingPlaceableCategoryItem Category) const;
	int32 FindCategoryIndexFromName(FName Name) const;
	void RemoveCategory(FName CategoryName) const;

	TSharedPtr<SCustomCategoryListView>	CustomObjectCategoryListView;
	TArray< TSharedPtr< FCustomCategoryListItem > >	CutomObjectCategoryList;


	/**
	* Generates a widget for a channel item.
	* @param InItem - the ChannelListItem
	* @param OwnerTable - the owning table
	* @return The table row widget
	*/
	TSharedRef<ITableRow> HandleGenerateDefaultCategoryWidget(TSharedPtr< FDefaultCategoryListItem> InItem, const TSharedRef<STableViewBase>& OwnerTable);

	FReply	OnEditDefaultCategory();
	bool	IsAnyDefaultCategorySelected() const;

	/* Refresh category */
	FReply	OnRefreshDefaultCategory();

	void RefreshDefaultCategoryList();
	void UpdateDefaultCategory();

	TSharedPtr<SDefaultCategoryListView>	DefaultObjectCategoryListView;
	TArray< TSharedPtr< FDefaultCategoryListItem > >	DefaultObjectCategoryList;
};


DECLARE_LOG_CATEGORY_EXTERN(CustomPlacementModeSettingLog, Log, All);
