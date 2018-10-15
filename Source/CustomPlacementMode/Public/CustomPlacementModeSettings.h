#pragma once

#include "Engine/Engine.h"
#include "IPropertyTypeCustomization.h"
#include "CustomPlacementModeSettings.generated.h"


class SNotificationItem;

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
	UPROPERTY(EditAnywhere, config, AdvancedDisplay, meta = (DisplayName = "Text Shadow Color"))
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
	UPROPERTY(EditAnywhere, config, Category = Category, meta = (DisplayName = "Category Name"))
		FName CategoryName;

	/** List of classes and their children displayed in this category */
	UPROPERTY(EditAnywhere, config, meta = (OnlyPlaceable , AllowAbstract = true, ConfigRestartRequired = true))
		TSet<TSubclassOf<AActor>> Items;

	/** Color scheme settings used category */
	UPROPERTY(EditAnywhere, config, AdvancedDisplay, meta = (DisplayName = "Tab Color"))
		FColorCategoryItem ColorSetting;

	friend bool operator== (const FSettingPlaceableCategoryItem& First, const FSettingPlaceableCategoryItem& Second)
	{
		return (First.CategoryName == Second.CategoryName);
	}

	inline friend uint32 GetTypeHash(const FSettingPlaceableCategoryItem& In)
	{
		return GetTypeHash(In.CategoryName); 
	}

	FSettingPlaceableCategoryItem()
	{
		CategoryName = TEXT("Default");
		Items.Add(nullptr);
	};
};


USTRUCT(BlueprintType,Blueprintable)
struct FSettingDefaultPlaceableCategoryItem
{
	GENERATED_BODY()

public:	

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
	
	/* Allow color scheme for custom categories */
	UPROPERTY(EditAnywhere, config, Category = "Custom Categories", meta = (ConfigRestartRequired = true, DisplayName = "Use color scheme for custom categories"))
		uint32 bUseCustomColorCategories : 1;

	/* Add Blueprint classes to custom category. 1-2 seconds frieze when starting the editor */
	UPROPERTY(EditAnywhere, config, Category = "Custom Categories", meta = (ConfigRestartRequired = true))
		uint32 bUseBlueprintChildClasses : 1;

	/** Custom Category list */
	UPROPERTY(config, EditAnywhere, Category = "Custom Categories", meta = (ConfigRestartRequired = true))
		TSet<FSettingPlaceableCategoryItem> PlaceableCategoryItems;

	TArray<FSettingPlaceableCategoryItem> TempPlaceableCategoryItems;

	/** RecentlyPlaced category */
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true, ShowOnlyInnerProperties))
	FSettingDefaultPlaceableCategoryItem RecentlyPlaced;
	
	/** Basic category */
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true, ShowOnlyInnerProperties))
	FSettingDefaultPlaceableCategoryItem Basic;
	
	/** Lights category */
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true, ShowOnlyInnerProperties))
	FSettingDefaultPlaceableCategoryItem Lights;
	
	/** Visual category */	
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true, ShowOnlyInnerProperties))
	FSettingDefaultPlaceableCategoryItem Visual;

	/** Volumes category */
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true, ShowOnlyInnerProperties))
	FSettingDefaultPlaceableCategoryItem Volumes;

	/** AllClassed category */
	UPROPERTY(config, EditAnywhere, Category = "Default Categories", meta = (ConfigRestartRequired = true, ShowOnlyInnerProperties))
	FSettingDefaultPlaceableCategoryItem AllClasses;

	virtual void PreEditChange(class FEditPropertyChain & PropertyAboutToChange) override;

	/**
	* This alternate version of PostEditChange is called when properties inside structs are modified.  The property that was actually modified
	* is located at the tail of the list.  The head of the list of the UStructProperty member variable that contains the property that was modified.
	*/
	virtual void PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent) override;
	
	void CheckTestItems(UProperty* Property);

private:
	TSharedPtr<SNotificationItem> NotificationSetAddDublicateCategoryName;
};

DECLARE_LOG_CATEGORY_EXTERN(CustomPlacementModeSettingLog, Log, All);
