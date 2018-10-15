#pragma once


#include "CoreMinimal.h"
#include "Engine/StreamableManager.h"
//#include "Engine.h"
//#include "UnrealEd.h"
#include "ICustomPlacementModeModule.h"
#include "ISettingsModule.h"
#include "CustomPlacementModeSettings.h"
#include "ISettingsSection.h"
#include "ISettingsContainer.h"

#define LOCTEXT_NAMESPACE "CustomPlacementModeSettings"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlacementModeCategoryRefreshed, FName /*CategoryName*/);
DECLARE_MULTICAST_DELEGATE_TwoParams(FCategoryAsyncFindActorDelegate,FStringAssetReference, AActor*);


struct FCustomPlacementCategory : FCustomPlacementCategoryInfo
{
	FCustomPlacementCategory(const FCustomPlacementCategoryInfo& SourceInfo)
		: FCustomPlacementCategoryInfo(SourceInfo)
	{

	}

	FCustomPlacementCategory(FCustomPlacementCategory&& In)
		: FCustomPlacementCategoryInfo(MoveTemp(In))
		, Items(MoveTemp(In.Items))
	{}

	FCustomPlacementCategory& operator=(FCustomPlacementCategory&& In)
	{
		FCustomPlacementCategoryInfo::operator=(MoveTemp(In));
		Items = MoveTemp(In.Items);
		return *this;
	}

	TMap<FGuid, TSharedPtr<FCustomPlaceableItem>> Items;
};


static TOptional<FLinearColor> GetBasicShapeColorOverride();

class FCustomPlacementModeModule : public ICustomPlacementModeModule
{
public:

	/**
	 * Called right after the module's DLL has been loaded and the module object has been created
	 */
	virtual void StartupModule() override;

	virtual void ShutdownModule() override;
	
	//FStreamableManager& AssetLoader;

	/**
	 * Called before the module is unloaded, right before the module object is destroyed.
	 */
	virtual void PreUnloadCallback() override;

	DECLARE_DERIVED_EVENT(FCustomPlacementModeModule, ICustomPlacementModeModule::FOnRecentlyPlacedChanged, FOnRecentlyPlacedChanged);
	virtual FOnRecentlyPlacedChanged& OnRecentlyPlacedChanged() override { return RecentlyPlacedChanged; }

	DECLARE_DERIVED_EVENT(FCustomPlacementModeModule, ICustomPlacementModeModule::FOnAllPlaceableAssetsChanged, FOnAllPlaceableAssetsChanged);
	virtual FOnAllPlaceableAssetsChanged& OnAllPlaceableAssetsChanged() override { return AllPlaceableAssetsChanged; }

	DECLARE_DERIVED_EVENT(FCustomPlacementModeModule, ICustomPlacementModeModule::FOnAllCustomPlaceableAssetsChanged, FOnAllCustomPlaceableAssetsChanged);
	virtual FOnAllCustomPlaceableAssetsChanged& OnAllCustomPlaceableAssetsChanged() override { return AllCustomPlaceableAssetsChanged; }

	FOnPlacementModeCategoryRefreshed& OnPlacementModeCategoryRefreshed() { return PlacementModeCategoryRefreshed; }

	void BroadcastPlacementModeCategoryRefreshed(FName CategoryName) { PlacementModeCategoryRefreshed.Broadcast(CategoryName); }

	/**
	 * Add the specified assets to the recently placed items list
	 */
	virtual void AddToRecentlyPlaced(const TArray< UObject* >& PlacedObjects, UActorFactory* FactoryUsed = NULL) override;

	void OnAssetRemoved(const FAssetData& /*InRemovedAssetData*/);

	void OnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath);

	void OnAssetAdded(const FAssetData& AssetData);

	/**
	 * Add the specified asset to the recently placed items list
	 */
	virtual void AddToRecentlyPlaced(UObject* Asset, UActorFactory* FactoryUsed = NULL) override;

	/**
	 * Get a copy of the recently placed items list
	 */
	virtual const TArray< FActorCustomPlacementInfo >& GetRecentlyPlaced() const override
	{
		return RecentlyPlaced;
	}

	/** @return the event that is broadcast whenever the placement mode enters a placing session */
	DECLARE_DERIVED_EVENT(FCustomPlacementModeModule, ICustomPlacementModeModule::FOnStartedPlacingEvent, FOnStartedPlacingEvent);
	virtual FOnStartedPlacingEvent& OnStartedPlacing() override
	{
		return StartedPlacingEvent;
	}
	virtual void BroadcastStartedPlacing(const TArray< UObject* >& Assets) override
	{
		StartedPlacingEvent.Broadcast(Assets);
	}

	/** @return the event that is broadcast whenever the placement mode exits a placing session */
	DECLARE_DERIVED_EVENT(FCustomPlacementModeModule, ICustomPlacementModeModule::FOnStoppedPlacingEvent, FOnStoppedPlacingEvent);
	virtual FOnStoppedPlacingEvent& OnStoppedPlacing() override
	{
		return StoppedPlacingEvent;
	}
	virtual void BroadcastStoppedPlacing(bool bWasSuccessfullyPlaced) override
	{
		StoppedPlacingEvent.Broadcast(bWasSuccessfullyPlaced);
	}

public:

	virtual bool RegisterPlacementCategory(const FCustomPlacementCategoryInfo& Info) override;

	virtual const FCustomPlacementCategoryInfo* GetRegisteredPlacementCategory(FName CategoryName) const override
	{
		return Categories.Find(CategoryName);
	}

	virtual void UnregisterPlacementCategory(FName Handle) override;

	virtual void GetSortedCategories(TArray<FCustomPlacementCategoryInfo>& OutCategories) const override;

	virtual TOptional<FCustomPlacementModeID> RegisterPlaceableItem(FName CategoryName, const TSharedRef<FCustomPlaceableItem>& InItem);

	virtual void UnregisterPlaceableItem(FCustomPlacementModeID ID) override;

	virtual void GetItemsForCategory(FName CategoryName, TArray<TSharedPtr<FCustomPlaceableItem>>& OutItems) const;

	virtual void GetFilteredItemsForCategory(FName CategoryName, TArray<TSharedPtr<FCustomPlaceableItem>>& OutItems, TFunctionRef<bool(const TSharedPtr<FCustomPlaceableItem>&)> Filter) const;

	virtual void RegenerateItemsForCategory(FName Category) override;

	virtual TArray<FCategoryItem> GetCustomCategories() override;

	virtual bool SupportsDynamicReloading() override
	{
		return true;
	}

	// Callback for when the settings were saved.
	bool HandleSettingsSaved()
	{
		UCustomPlacementModeSettings* Settings = GetMutableDefault<UCustomPlacementModeSettings>();
		bool ResaveSettings = false;

		// You can put any validation code in here and resave the settings in case an invalid
		// value has been entered

		if (ResaveSettings)
		{
			Settings->SaveConfig();
		}

		return true;
	}

	void RegisterSettings()
	{
		// Registering some settings is just a matter of exposing the default UObject of
		// your desired class, feel free to add here all those settings you want to expose
		// to your LDs or artists.

		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			// Create the new category
			ISettingsContainerPtr SettingsContainer = SettingsModule->GetContainer("Project");

			SettingsContainer->DescribeCategory("Plugins",
				LOCTEXT("RuntimeWDCategoryName", "Plugins"),
				LOCTEXT("RuntimeWDCategoryName", "Plugins"));
			
			// Register the settings
			ISettingsSectionPtr SettingsSection = SettingsModule->RegisterSettings("Project", "Plugins", "Custom Placement",
				LOCTEXT("ProjectGeneralSettingsName", "Custom Placement"),
				LOCTEXT("ProjectGeneralSettingsDescription", "Configure the Custom Placement Mode plug-in"),
				GetMutableDefault<UCustomPlacementModeSettings>()
			);

			// Register the save handler to your settings, you might want to use it to
			// validate those or just act to settings changes.
			if (SettingsSection.IsValid())
			{
				SettingsSection->OnModified().BindRaw(this, &FCustomPlacementModeModule::HandleSettingsSaved);
			}
		}
	}

	void UnregisterSettings()
	{
		// Ensure to unregister all of your registered settings here, hot-reload would
		// otherwise yield unexpected results.

		if (ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
		{
			SettingsModule->UnregisterSettings("Project", "Plugins","Custom Placement");
		}
	}

private:

	void RefreshRecentlyPlaced();

	void RefreshVolumes();

	void RefreshAllPlaceableClasses();

	void RefreshAllCustomClasses();
	void RefreshAllCustomClasses(FName CategoryName);

	TArray<TAssetSubclassOf<AActor>> RefreshAllCustomClassesWithBlueprint(FCategoryItem CategoryItem);
	TArray<TAssetSubclassOf<AActor>> RefreshAllCustomClassesWithBlueprint(FName CategoryName);
	TArray<TAssetSubclassOf<AActor>> RefreshAllClassesWithBlueprint();

	void  AddBPItemsToCategory(TSoftClassPtr<AActor> Reference, FCustomPlacementCategory* Category);
	FGuid CreateID();

	FCustomPlacementModeID CreateID(FName InCategory);

private:

	TArray<FCategoryItem> CustomCategories;

	TMap<FName, FCustomPlacementCategory> Categories;
	
	TArray<TAssetSubclassOf<AActor>> BPItems;	
	TArray<FSoftObjectPath> CategoryItemsToStream;
	FCustomPlacementCategory* CurrentCategory;
	TSharedPtr<FStreamableHandle> StreamHandle;

	TArray< FActorCustomPlacementInfo >	RecentlyPlaced;
	FOnRecentlyPlacedChanged		RecentlyPlacedChanged;

	FOnAllPlaceableAssetsChanged	AllPlaceableAssetsChanged;
	FOnAllCustomPlaceableAssetsChanged	AllCustomPlaceableAssetsChanged;

	FOnPlacementModeCategoryRefreshed PlacementModeCategoryRefreshed;

	FOnStartedPlacingEvent			StartedPlacingEvent;
	FOnStoppedPlacingEvent			StoppedPlacingEvent;

	TArray< TSharedPtr<FExtender> > ContentPaletteFiltersExtenders;
	TArray< TSharedPtr<FExtender> > PaletteExtenders;
};

#undef LOCTEXT_NAMESPACE

DECLARE_LOG_CATEGORY_EXTERN(CustomPlacementModeModuleLog, Log, All);