
#include "CustomPlacementModeModule.h"

#include "Engine/BrushBuilder.h"
#include "ActorCustomPlacementInfo.h"

#include "AssetRegistryModule.h"
#include "CustomPlacementMode.h"
#include "EditorModeRegistry.h"
#include "EditorModeManager.h"
#include "AssetToolsModule.h"

#include "Runtime/Core/Public/Misc/Optional.h"
#include "EditorModes.h"

#include "Launch/Resources/Version.h"

#include "ActorFactories/ActorFactory.h"

#include "ActorFactories/ActorFactoryTriggerBox.h"
#include "ActorFactories/ActorFactoryPlanarReflection.h"
#include "ActorFactories/ActorFactoryEmptyActor.h"
#include "ActorFactories/ActorFactoryCharacter.h"
#include "ActorFactories/ActorFactoryPawn.h"
#include "ActorFactories/ActorFactoryPointLight.h"
#include "ActorFactories/ActorFactoryPlayerStart.h"
#include "ActorFactories/ActorFactoryBasicShape.h"
#include "ActorFactories/ActorFactoryTriggerSphere.h"
#include "ActorFactories/ActorFactoryDirectionalLight.h"
#include "ActorFactories/ActorFactorySpotLight.h"
#include "ActorFactories/ActorFactorySkyLight.h"
#include "ActorFactories/ActorFactoryAtmosphericFog.h"
#include "ActorFactories/ActorFactoryBoxVolume.h"
#include "ActorFactories/ActorFactoryExponentialHeightFog.h"
#include "ActorFactories/ActorFactorySphereReflectionCapture.h"
#include "ActorFactories/ActorFactoryBoxReflectionCapture.h"
#include "ActorFactories/ActorFactoryDeferredDecal.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/AssetManager.h"

#if ENGINE_MINOR_VERSION >= 20
#include "ActorFactories/ActorFactoryRectLight.h"
#endif


#include "Styles/CustomCategoryStyles.h" 
#include "Kismet2/KismetEditorUtilities.h"
#include "PropertyEditorModule.h"
#include "PropertyEditorDelegates.h"
#include "Misc/ConfigCacheIni.h"
#include <EngineUtils.h>


DEFINE_LOG_CATEGORY(CustomPlacementModeModuleLog);

TOptional<FLinearColor> GetBasicShapeColorOverride()
{
	// Get color for basic shapes.  It should appear like all the other basic types
	static TOptional<FLinearColor> BasicShapeColorOverride;

	if (!BasicShapeColorOverride.IsSet())
	{
		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		TSharedPtr<IAssetTypeActions> AssetTypeActions;
		AssetTypeActions = AssetToolsModule.Get().GetAssetTypeActionsForClass(UClass::StaticClass()).Pin();
		if (AssetTypeActions.IsValid())
		{
			BasicShapeColorOverride = TOptional<FLinearColor>(AssetTypeActions->GetTypeColor());
		}
	}
	return BasicShapeColorOverride;
}

void FCustomPlacementModeModule::StartupModule()
{
	//AssetLoader = UAssetManager::GetStreamableManager();
	
	UE_LOG(CustomPlacementModeModuleLog, Log, TEXT("Streamable Manager init"));
	
	UE_LOG(CustomPlacementModeModuleLog, Log, TEXT("Custom PlacementMode Module Start"));
	FCustomCategoryStyle::Initialize();

	RegisterSettings();

	FEditorModeRegistry::Get().UnregisterMode(FBuiltinEditorModes::EM_Placement);

	TArray< FString > RecentlyPlacedAsStrings;
	GConfig->GetArray(TEXT("CustomPlacementMode"), TEXT("RecentlyPlaced"), RecentlyPlacedAsStrings, GEditorPerProjectIni);

	for (int Index = 0; Index < RecentlyPlacedAsStrings.Num(); Index++)
	{
		RecentlyPlaced.Add(FActorCustomPlacementInfo(RecentlyPlacedAsStrings[Index]));
	}

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	AssetRegistryModule.Get().OnAssetRemoved().AddRaw(this, &FCustomPlacementModeModule::OnAssetRemoved);
	AssetRegistryModule.Get().OnAssetRenamed().AddRaw(this, &FCustomPlacementModeModule::OnAssetRenamed);
	AssetRegistryModule.Get().OnAssetAdded().AddRaw(this, &FCustomPlacementModeModule::OnAssetAdded);


	TOptional<FLinearColor> BasicShapeColorOverride = GetBasicShapeColorOverride();


	RegisterPlacementCategory(
		FCustomPlacementCategoryInfo(
			NSLOCTEXT("CustomPlacementMode", "RecentlyPlaced", "Recently Placed"),
			FCustomBuiltInPlacementCategories::RecentlyPlaced(),
			TEXT("PMRecentlyPlaced"),
			TNumericLimits<int32>::Lowest(),
			false
		)
	);

	{
		int32 SortOrder = 0;
		FName CategoryName = FCustomBuiltInPlacementCategories::Basic();
		RegisterPlacementCategory(
			FCustomPlacementCategoryInfo(
				NSLOCTEXT("CustomPlacementMode", "Basic", "Basic"),
				CategoryName,
				TEXT("PMBasic"),
				10
			)
		);

		FCustomPlacementCategory* Category = Categories.Find(CategoryName);
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryEmptyActor::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryCharacter::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryPawn::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryPointLight::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryPlayerStart::StaticClass(), SortOrder += 10)));
		// Cube
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryBasicShape::StaticClass(), FAssetData(LoadObject<UStaticMesh>(nullptr, *UActorFactoryBasicShape::BasicCube.ToString())), FName("ClassThumbnail.Cube"), BasicShapeColorOverride, SortOrder += 10, NSLOCTEXT("PlacementMode", "Cube", "Cube"))));
		// Sphere
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryBasicShape::StaticClass(), FAssetData(LoadObject<UStaticMesh>(nullptr, *UActorFactoryBasicShape::BasicSphere.ToString())), FName("ClassThumbnail.Sphere"), BasicShapeColorOverride, SortOrder += 10, NSLOCTEXT("PlacementMode", "Sphere", "Sphere"))));
		// Cylinder
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryBasicShape::StaticClass(), FAssetData(LoadObject<UStaticMesh>(nullptr, *UActorFactoryBasicShape::BasicCylinder.ToString())), FName("ClassThumbnail.Cylinder"), BasicShapeColorOverride, SortOrder += 10, NSLOCTEXT("PlacementMode", "Cylinder", "Cylinder"))));
		// Cone
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryBasicShape::StaticClass(), FAssetData(LoadObject<UStaticMesh>(nullptr, *UActorFactoryBasicShape::BasicCone.ToString())), FName("ClassThumbnail.Cone"), BasicShapeColorOverride, SortOrder += 10, NSLOCTEXT("PlacementMode", "Cone", "Cone"))));
		// Plane
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryBasicShape::StaticClass(), FAssetData(LoadObject<UStaticMesh>(nullptr, *UActorFactoryBasicShape::BasicPlane.ToString())), FName("ClassThumbnail.Plane"), BasicShapeColorOverride, SortOrder += 10, NSLOCTEXT("PlacementMode", "Plane", "Plane"))));

		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryTriggerBox::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryTriggerSphere::StaticClass(), SortOrder += 10)));
	}

	{
		int32 SortOrder = 0;
		FName CategoryName = FCustomBuiltInPlacementCategories::Lights();
		RegisterPlacementCategory(
			FCustomPlacementCategoryInfo(
				NSLOCTEXT("CustomPlacementMode", "Lights", "Lights"),
				CategoryName,
				TEXT("PMLights"),
				20
			)
		);

		FCustomPlacementCategory* Category = Categories.Find(CategoryName);
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryDirectionalLight::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryPointLight::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactorySpotLight::StaticClass(), SortOrder += 10)));
#if ENGINE_MINOR_VERSION >= 20
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryRectLight::StaticClass(), SortOrder += 10)));
#endif
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactorySkyLight::StaticClass(), SortOrder += 10)));
	}

	{
		int32 SortOrder = 0;
		FName CategoryName = FCustomBuiltInPlacementCategories::Visual();
		RegisterPlacementCategory(
			FCustomPlacementCategoryInfo(
				NSLOCTEXT("CustomPlacementMode", "VisualEffects", "Visual Effects"),
				CategoryName,
				TEXT("PMVisual"),
				30
			)
		);

		UActorFactory* PPFactory = GEditor->FindActorFactoryByClassForActorClass(UActorFactoryBoxVolume::StaticClass(), APostProcessVolume::StaticClass());

		FCustomPlacementCategory* Category = Categories.Find(CategoryName);
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(PPFactory, FAssetData(APostProcessVolume::StaticClass()), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryAtmosphericFog::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryExponentialHeightFog::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactorySphereReflectionCapture::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryBoxReflectionCapture::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryPlanarReflection::StaticClass(), SortOrder += 10)));
		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryDeferredDecal::StaticClass(), SortOrder += 10)));
	}

	RegisterPlacementCategory(
		FCustomPlacementCategoryInfo(
			NSLOCTEXT("CustomPlacementMode", "Volumes", "Volumes"),
			FCustomBuiltInPlacementCategories::Volumes(),
			TEXT("PMVolumes"),
			40
		)
	);

	RegisterPlacementCategory(
		FCustomPlacementCategoryInfo(
			NSLOCTEXT("CustomPlacementMode", "AllClasses", "All Classes"),
			FCustomBuiltInPlacementCategories::AllClasses(),
			TEXT("PMAllClasses"),
			50
		)
	);


	UCustomPlacementModeSettings* Settings = GetMutableDefault<UCustomPlacementModeSettings>();

	if (Settings)
	{

		for (auto& SettingItem : Settings->PlaceableCategoryItems)
		{
			FName HandleName = SettingItem.CategoryName;
			FName DisplayName = HandleName;

			FString PrefixTag = TEXT("PM");
			FString TagName = PrefixTag.Append(HandleName.ToString());

			FCategoryItem Item = FCategoryItem(HandleName, DisplayName, TagName);
			Item.Items = SettingItem.Items;

			CustomCategories.Add(Item);
		}

		int32 Sort = 60;
		for (const FCategoryItem& Category : CustomCategories)
		{
			FFormatNamedArguments Arguments;
			Arguments.Add(TEXT("Handle"), FText::FromName(Category.HandleName));
			Arguments.Add(TEXT("Display"), FText::FromName(Category.DisplayName));


			RegisterPlacementCategory(
				FCustomPlacementCategoryInfo(
					FText::Format(NSLOCTEXT("CustomPlacementMode", "{Handle}", "{Display}"), Arguments),
					Category.HandleName,
					Category.TagName,
					Sort
				)
			);

			Sort += 10;
		}
	}

	FEditorModeRegistry::Get().RegisterMode<FCustomPlacementMode>(
		FBuiltinEditorModes::EM_Placement,
		NSLOCTEXT("CustomPlacementMode", "DisplayName", "Place"),
		FSlateIcon(FEditorStyle::GetStyleSetName(), "LevelEditor.PlacementMode", "LevelEditor.PlacementMode.Small"),
		true,0);
}

void FCustomPlacementModeModule::ShutdownModule()
{
	UE_LOG(CustomPlacementModeModuleLog, Log, TEXT("Custom PlacementMode Module Shutdown"));
	
	FCustomCategoryStyle::Shutdown();

	if (UObjectInitialized())
	{
		UnregisterSettings();
	}
}

void FCustomPlacementModeModule::PreUnloadCallback()
{
	FEditorModeRegistry::Get().UnregisterMode(FBuiltinEditorModes::EM_Placement);

	FAssetRegistryModule* AssetRegistryModule = FModuleManager::GetModulePtr<FAssetRegistryModule>("AssetRegistry");
	if (AssetRegistryModule)
	{
		AssetRegistryModule->Get().OnAssetRemoved().RemoveAll(this);
		AssetRegistryModule->Get().OnAssetRenamed().RemoveAll(this);
		AssetRegistryModule->Get().OnAssetAdded().RemoveAll(this);
	}
}

void FCustomPlacementModeModule::AddToRecentlyPlaced(const TArray<UObject *>& PlacedObjects, UActorFactory* FactoryUsed /* = NULL */)
{
	FString FactoryPath;
	if (FactoryUsed != nullptr)
	{
		FactoryPath = FactoryUsed->GetPathName();
	}

	TArray< UObject* > FilteredPlacedObjects;
	for (UObject* PlacedObject : PlacedObjects)
	{
		// Don't include null placed objects that just have factories.
		if (PlacedObject == nullptr)
		{
			continue;
		}

		// Don't add brush builders to the recently placed.
		if (PlacedObject->IsA(UBrushBuilder::StaticClass()))
		{
			continue;
		}

		FilteredPlacedObjects.Add(PlacedObject);
	}

	// Don't change the recently placed if nothing passed the filter.
	if (FilteredPlacedObjects.Num() == 0)
	{
		return;
	}

	bool Changed = false;
	for (int Index = 0; Index < FilteredPlacedObjects.Num(); Index++)
	{
		Changed |= RecentlyPlaced.Remove(FActorCustomPlacementInfo(FilteredPlacedObjects[Index]->GetPathName(), FactoryPath)) > 0;
	}

	for (int Index = 0; Index < FilteredPlacedObjects.Num(); Index++)
	{
		if (FilteredPlacedObjects[Index] != NULL)
		{
			RecentlyPlaced.Insert(FActorCustomPlacementInfo(FilteredPlacedObjects[Index]->GetPathName(), FactoryPath), 0);
			Changed = true;
		}
	}

	for (int Index = RecentlyPlaced.Num() - 1; Index >= 20; Index--)
	{
		RecentlyPlaced.RemoveAt(Index);
		Changed = true;
	}

	if (Changed)
	{
		TArray< FString > RecentlyPlacedAsStrings;
		for (int Index = 0; Index < RecentlyPlaced.Num(); Index++)
		{
			RecentlyPlacedAsStrings.Add(RecentlyPlaced[Index].ToString());
		}

		GConfig->SetArray(TEXT("PlacementMode"), TEXT("RecentlyPlaced"), RecentlyPlacedAsStrings, GEditorPerProjectIni);
		RecentlyPlacedChanged.Broadcast(RecentlyPlaced);
	}
}

void FCustomPlacementModeModule::OnAssetRemoved(const FAssetData&)
{
	RecentlyPlacedChanged.Broadcast(RecentlyPlaced);
	AllPlaceableAssetsChanged.Broadcast();
	AllCustomPlaceableAssetsChanged.Broadcast();
}

void FCustomPlacementModeModule::OnAssetRenamed(const FAssetData& AssetData, const FString& OldObjectPath)
{
	for (auto& RecentlyPlacedItem : RecentlyPlaced)
	{
		if (RecentlyPlacedItem.ObjectPath == OldObjectPath)
		{
			RecentlyPlacedItem.ObjectPath = AssetData.ObjectPath.ToString();
			break;
		}
	}

	RecentlyPlacedChanged.Broadcast(RecentlyPlaced);
	AllPlaceableAssetsChanged.Broadcast();
	AllCustomPlaceableAssetsChanged.Broadcast();
}

void FCustomPlacementModeModule::OnAssetAdded(const FAssetData& AssetData)
{
	AllPlaceableAssetsChanged.Broadcast();
	AllCustomPlaceableAssetsChanged.Broadcast();
}

void FCustomPlacementModeModule::AddToRecentlyPlaced(UObject* Asset, UActorFactory* FactoryUsed /* = NULL */)
{
	TArray< UObject* > Assets;
	Assets.Add(Asset);
	AddToRecentlyPlaced(Assets, FactoryUsed);
}

bool FCustomPlacementModeModule::RegisterPlacementCategory(const FCustomPlacementCategoryInfo& Info)
{
	if (Categories.Contains(Info.UniqueHandle))
	{
		return false;
	}

	Categories.Add(Info.UniqueHandle, Info);
	return true;
}

void FCustomPlacementModeModule::UnregisterPlacementCategory(FName Handle)
{
	Categories.Remove(Handle);
}

void FCustomPlacementModeModule::GetSortedCategories(TArray<FCustomPlacementCategoryInfo>& OutCategories) const
{
	TArray<FName> SortedNames;
	Categories.GenerateKeyArray(SortedNames);

	SortedNames.Sort([&](const FName& A, const FName& B) {
		return Categories[A].SortOrder < Categories[B].SortOrder;
	});

	OutCategories.Reset(Categories.Num());
	for (const FName& Name : SortedNames)
	{
		OutCategories.Add(Categories[Name]);
	}
}

TOptional<FCustomPlacementModeID> FCustomPlacementModeModule::RegisterPlaceableItem(FName CategoryName, const TSharedRef<FCustomPlaceableItem>& InItem)
{
	FCustomPlacementCategory* Category = Categories.Find(CategoryName);
	if (Category && !Category->CustomGenerator)
	{
		FCustomPlacementModeID ID = CreateID(CategoryName);
		Category->Items.Add(ID.UniqueID, InItem);
		return ID;
	}
	return TOptional<FCustomPlacementModeID>();
}

void FCustomPlacementModeModule::UnregisterPlaceableItem(FCustomPlacementModeID ID)
{
	FCustomPlacementCategory* Category = Categories.Find(ID.Category);
	if (Category)
	{
		Category->Items.Remove(ID.UniqueID);
	}
}

void FCustomPlacementModeModule::GetItemsForCategory(FName CategoryName, TArray<TSharedPtr<FCustomPlaceableItem>>& OutItems) const
{
	const FCustomPlacementCategory* Category = Categories.Find(CategoryName);
	if (Category)
	{
		for (auto& Pair : Category->Items)
		{
			OutItems.Add(Pair.Value);
		}
	}
}

void FCustomPlacementModeModule::GetFilteredItemsForCategory(FName CategoryName, TArray<TSharedPtr<FCustomPlaceableItem>>& OutItems, TFunctionRef<bool(const TSharedPtr<FCustomPlaceableItem> &)> Filter) const
{
	const FCustomPlacementCategory* Category = Categories.Find(CategoryName);
	if (Category)
	{
		for (auto& Pair : Category->Items)
		{
			if (Filter(Pair.Value))
			{
				OutItems.Add(Pair.Value);
			}
		}
	}
}

void FCustomPlacementModeModule::RegenerateItemsForCategory(FName Category)
{
	if (Category == FCustomBuiltInPlacementCategories::RecentlyPlaced())
	{
		RefreshRecentlyPlaced();
	}
	else if (Category == FCustomBuiltInPlacementCategories::Volumes())
	{
		RefreshVolumes();
	}
	else if (Category == FCustomBuiltInPlacementCategories::AllClasses())
	{
		RefreshAllPlaceableClasses();
	}
	else if(Category == FCustomBuiltInPlacementCategories::Custom() || (Category != FCustomBuiltInPlacementCategories::RecentlyPlaced() && Category != FCustomBuiltInPlacementCategories::Volumes() && Category != FCustomBuiltInPlacementCategories::AllClasses()))
	{
		//RefreshAllClassesWithBlueprint();
		RefreshAllCustomClasses();
	}

	BroadcastPlacementModeCategoryRefreshed(Category);
}

TArray<FCategoryItem> FCustomPlacementModeModule::GetCustomCategories()
{
	return CustomCategories;
}

void FCustomPlacementModeModule::RefreshRecentlyPlaced()
{
	FName CategoryName = FCustomBuiltInPlacementCategories::RecentlyPlaced();

	FCustomPlacementCategory* Category = Categories.Find(CategoryName);
	if (!Category)
	{
		return;
	}

	Category->Items.Reset();


	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	for (const FActorCustomPlacementInfo& RecentlyPlacedItem : RecentlyPlaced)
	{
		UObject* Asset = FindObject<UObject>(nullptr, *RecentlyPlacedItem.ObjectPath);

		// If asset is pending delete, it will not be marked as RF_Standalone, in which case we skip it
		if (Asset != nullptr && Asset->HasAnyFlags(RF_Standalone))
		{
			FAssetData AssetData = AssetRegistryModule.Get().GetAssetByObjectPath(*RecentlyPlacedItem.ObjectPath);

			if (AssetData.IsValid())
			{
				UActorFactory* Factory = FindObject<UActorFactory>(nullptr, *RecentlyPlacedItem.Factory);
				Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(Factory, AssetData)));
			}
		}
	}
}

void FCustomPlacementModeModule::RefreshVolumes()
{
	FName CategoryName = FCustomBuiltInPlacementCategories::Volumes();

	FCustomPlacementCategory* Category = Categories.Find(CategoryName);
	if (!Category)
	{
		return;
	}

	Category->Items.Reset();

	// Add loaded classes
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		const UClass* Class = *ClassIt;

		if (!Class->HasAllClassFlags(CLASS_NotPlaceable) &&
			!Class->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) &&
			Class->IsChildOf(AVolume::StaticClass()) &&
			Class->ClassGeneratedBy == nullptr)
		{
			UActorFactory* Factory = GEditor->FindActorFactoryByClassForActorClass(UActorFactoryBoxVolume::StaticClass(), Class);
			Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(Factory, FAssetData(Class))));
		}
	}
}

void FCustomPlacementModeModule::RefreshAllPlaceableClasses()
{
	FName CategoryName = FCustomBuiltInPlacementCategories::AllClasses();

	// Unregister old stuff
	FCustomPlacementCategory* Category = Categories.Find(CategoryName);
	if (!Category)
	{
		return;
	}

	Category->Items.Reset();

	// Manually add some special cases that aren't added below
	Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryEmptyActor::StaticClass())));
	Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryCharacter::StaticClass())));
	Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryPawn::StaticClass())));
	Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryBasicShape::StaticClass(), FAssetData(LoadObject<UStaticMesh>(nullptr, *UActorFactoryBasicShape::BasicCube.ToString())), FName("ClassThumbnail.Cube"), GetBasicShapeColorOverride())));
	Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryBasicShape::StaticClass(), FAssetData(LoadObject<UStaticMesh>(nullptr, *UActorFactoryBasicShape::BasicSphere.ToString())), FName("ClassThumbnail.Sphere"), GetBasicShapeColorOverride())));
	Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryBasicShape::StaticClass(), FAssetData(LoadObject<UStaticMesh>(nullptr, *UActorFactoryBasicShape::BasicCylinder.ToString())), FName("ClassThumbnail.Cylinder"), GetBasicShapeColorOverride())));
	Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryBasicShape::StaticClass(), FAssetData(LoadObject<UStaticMesh>(nullptr, *UActorFactoryBasicShape::BasicCone.ToString())), FName("ClassThumbnail.Cone"), GetBasicShapeColorOverride())));
	Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(*UActorFactoryBasicShape::StaticClass(), FAssetData(LoadObject<UStaticMesh>(nullptr, *UActorFactoryBasicShape::BasicPlane.ToString())), FName("ClassThumbnail.Plane"), GetBasicShapeColorOverride())));

	// Make a map of UClasses to ActorFactories that support them
	const TArray< UActorFactory *>& ActorFactories = GEditor->ActorFactories;
	TMap<UClass*, UActorFactory*> ActorFactoryMap;
	for (int32 FactoryIdx = 0; FactoryIdx < ActorFactories.Num(); ++FactoryIdx)
	{
		UActorFactory* ActorFactory = ActorFactories[FactoryIdx];

		if (ActorFactory)
		{
			ActorFactoryMap.Add(ActorFactory->GetDefaultActorClass(FAssetData()), ActorFactory);
		}
	}

	FAssetData NoAssetData;
	FText UnusedErrorMessage;

	// Add loaded classes
	for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
	{
		// Don't offer skeleton classes
		bool bIsSkeletonClass = FKismetEditorUtilities::IsClassABlueprintSkeleton(*ClassIt);

		if (!ClassIt->HasAllClassFlags(CLASS_NotPlaceable) &&
			!ClassIt->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) &&
			ClassIt->IsChildOf(AActor::StaticClass()) &&
			(!ClassIt->IsChildOf(ABrush::StaticClass()) || ClassIt->IsChildOf(AVolume::StaticClass())) &&
			!bIsSkeletonClass)
		{
			UActorFactory* ActorFactory = ActorFactoryMap.FindRef(*ClassIt);

			const bool IsVolume = ClassIt->IsChildOf(AVolume::StaticClass());

			if (IsVolume)
			{
				ActorFactory = GEditor->FindActorFactoryByClassForActorClass(UActorFactoryBoxVolume::StaticClass(), *ClassIt);
			}
			else if (ActorFactory && !ActorFactory->CanCreateActorFrom(NoAssetData, UnusedErrorMessage))
			{
				continue;
			}

			Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(ActorFactory, FAssetData(*ClassIt))));
		}
	}
}

void FCustomPlacementModeModule::RefreshAllCustomClasses()
{
	for (auto& CategoryItem : CustomCategories)
	{
		FName CategoryName = CategoryItem.HandleName;

		// Unregister old stuff
		FCustomPlacementCategory* Category = Categories.Find(CategoryName);
		if (!Category)
		{
			return;
		}

		Category->Items.Reset();

		// Make a map of UClasses to ActorFactories that support them
		const TArray< UActorFactory *>& ActorFactories = GEditor->ActorFactories;
		TMap<UClass*, UActorFactory*> ActorFactoryMap;
		for (int32 FactoryIdx = 0; FactoryIdx < ActorFactories.Num(); ++FactoryIdx)
		{
			UActorFactory* ActorFactory = ActorFactories[FactoryIdx];

			if (ActorFactory)
			{
				ActorFactoryMap.Add(ActorFactory->GetDefaultActorClass(FAssetData()), ActorFactory);
			}
		}

		FAssetData NoAssetData;
		FText UnusedErrorMessage;

		// Add loaded classes
		for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
		{
			// Don't offer skeleton classes
			bool bIsSkeletonClass = FKismetEditorUtilities::IsClassABlueprintSkeleton(*ClassIt);

			for (auto& Elem : CategoryItem.Items)
			{
				if (!ClassIt->HasAllClassFlags(CLASS_NotPlaceable) &&
					!ClassIt->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) &&
					ClassIt->IsChildOf(Elem) && !bIsSkeletonClass /*&& ClassIt->IsNative() */
					)
				{
					UActorFactory* ActorFactory = ActorFactoryMap.FindRef(*ClassIt);

					if (ActorFactory && !ActorFactory->CanCreateActorFrom(NoAssetData, UnusedErrorMessage))
					{
						continue;
					}

					Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(ActorFactory, FAssetData(*ClassIt))));
				}
			}
		}

		UCustomPlacementModeSettings* Settings = GetMutableDefault<UCustomPlacementModeSettings>();

		if (Settings->bUseBlueprintChildClasses)
		{

			BPItems = RefreshAllCustomClassesWithBlueprint(CategoryItem);

			UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("Update Category - %s"), *Category->UniqueHandle.ToString());
			for (int32 i = 0; i < BPItems.Num(); i++)
			{
				TSoftClassPtr<AActor> Item = BPItems[i];

				//CategoryClass = Item;
				//CategoryItemsToStream.AddUnique(Item.ToSoftObjectPath());
				//UClass* BPClass = AssetLoader->SynchronousLoad(Item);
				UClass* BPClass = Item.LoadSynchronous();
				//UAssetManager::GetStreamableManager().LoadSynchronous(Item.ToSoftObjectPath());
				//UAssetManager::GetStreamableManager().RequestAsyncLoad(Item.ToSoftObjectPath(),FStreamableDelegate::CreateRaw(this,&FCustomPlacementModeModule::AddBPItemsToCategory,Item,Category),100,true);			
				//AssetLoader->RequestAsyncLoad(Item.ToSoftObjectPath(),FStreamableDelegate::CreateRaw(this, &FCustomPlacementModeModule::AddBPItemsToCategory),100,true);
				//UClass* BPClass = Item.Get();

				if (BPClass)
				{
					if (!BPClass->HasAllClassFlags(CLASS_NotPlaceable))
					{
						UActorFactory* ActorFactory = GEditor->FindActorFactoryForActorClass(BPClass);
						Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(ActorFactory, FAssetData(BPClass))));

						UE_LOG(CustomPlacementModeSettingLog, Log, TEXT("[%d] Asset AddedToCategory - %s"), i, *BPClass->GetName());
					}
				}
				//AssetLoader->Unload(Item.ToSoftObjectPath());
			}

			//StreamHandle = AssetLoader->RequestAsyncLoad(CategoryItemsToStream, FStreamableDelegate::CreateRaw(this, &FCustomPlacementModeModule::AddBPItemsToCategory));
		}
	}

}

void FCustomPlacementModeModule::RefreshAllCustomClasses(FName CategoryName)
{
	for (auto& CategoryItem : CustomCategories)
	{
		if(CategoryName == CategoryItem.HandleName)
		{
			// Unregister old stuff
			FCustomPlacementCategory* Category = Categories.Find(CategoryName);
			if (!Category)
			{
				return;
			}

			Category->Items.Reset();

			// Make a map of UClasses to ActorFactories that support them
			const TArray< UActorFactory *>& ActorFactories = GEditor->ActorFactories;
			TMap<UClass*, UActorFactory*> ActorFactoryMap;
			for (int32 FactoryIdx = 0; FactoryIdx < ActorFactories.Num(); ++FactoryIdx)
			{
				UActorFactory* ActorFactory = ActorFactories[FactoryIdx];

				if (ActorFactory)
				{
					ActorFactoryMap.Add(ActorFactory->GetDefaultActorClass(FAssetData()), ActorFactory);
				}
			}

			FAssetData NoAssetData;
			FText UnusedErrorMessage;

			// Add loaded classes
			for (TObjectIterator<UClass> ClassIt; ClassIt; ++ClassIt)
			{
				// Don't offer skeleton classes
				bool bIsSkeletonClass = FKismetEditorUtilities::IsClassABlueprintSkeleton(*ClassIt);

				for (auto& Elem : CategoryItem.Items)
				{
					if (!ClassIt->HasAllClassFlags(CLASS_NotPlaceable) &&
						!ClassIt->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists) &&
						ClassIt->IsChildOf(Elem) && !bIsSkeletonClass && ClassIt->IsNative()
						)
					{
						UActorFactory* ActorFactory = ActorFactoryMap.FindRef(*ClassIt);

						if (ActorFactory && !ActorFactory->CanCreateActorFrom(NoAssetData, UnusedErrorMessage))
						{
							continue;
						}

						Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(ActorFactory, FAssetData(*ClassIt))));
					}
				}
			}

			BPItems = RefreshAllCustomClassesWithBlueprint(CategoryItem);

			UE_LOG(CustomPlacementModeSettingLog,Log,TEXT("Update Category - %s"),*Category->UniqueHandle.ToString());
			for (int32 i = 0; i < BPItems.Num(); i++)
			{
				TSoftClassPtr<AActor> Item = BPItems[i];
				//CategoryClass = Item;
				//CategoryItemsToStream.AddUnique(Item.ToSoftObjectPath());
				//UClass* BPClass = AssetLoader->SynchronousLoad(Item);
				//UClass* BPClass = Item.LoadSynchronous();
				//AssetLoader->SimpleAsyncLoad(Item.ToSoftObjectPath());

				UAssetManager::GetStreamableManager().RequestAsyncLoad(Item.ToSoftObjectPath(),FStreamableDelegate::CreateRaw(this,&FCustomPlacementModeModule::AddBPItemsToCategory,Item,Category));
				

				//UAssetManager::GetStreamableManager().SynchronousLoad(Item.ToSoftObjectPath());
			
				//UClass* BPClass = Item.Get();
				//
				//if (BPClass)
				//{
				//	if (!BPClass->HasAllClassFlags(CLASS_NotPlaceable))
				//	{
				//		UActorFactory* ActorFactory = GEditor->FindActorFactoryForActorClass(BPClass);
				//		Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(ActorFactory, FAssetData(BPClass))));

				//		UE_LOG(CustomPlacementModeModuleLog, Log, TEXT("[%d] Asset AddedToCategory - %s"), i, *BPClass->GetName());
				//	}
				//}
				
				//AssetLoader->Unload(Item.ToSoftObjectPath());
			}
			
			//UAssetManager::GetStreamableManager().RequestAsyncLoad(CategoryItemsToStream,FStreamableDelegate::CreateRaw(this, &FCustomPlacementModeModule::AddBPItemsToCategory, Item, Category));
		}
	}
}

void FCustomPlacementModeModule::AddBPItemsToCategory(TSoftClassPtr<AActor> Reference, FCustomPlacementCategory* Category)
{	
	UE_LOG(CustomPlacementModeModuleLog,Log,TEXT("Category - %s"), *Category->UniqueHandle.ToString());		
	UE_LOG(CustomPlacementModeModuleLog,Log,TEXT("AssetsLoaded - %s"), *Reference.ToString());	
	
	UClass* BPClass = Reference.Get();	
	
	if (BPClass)
	{
		if (!BPClass->HasAllClassFlags(CLASS_NotPlaceable))
		{
			UActorFactory* ActorFactory = GEditor->FindActorFactoryForActorClass(BPClass);
			Category->Items.Add(CreateID(), MakeShareable(new FCustomPlaceableItem(ActorFactory, FAssetData(BPClass))));
			
			UE_LOG(CustomPlacementModeModuleLog,Log,TEXT("AssetsAdded - %s"), *Reference.ToString());
			//UE_LOG(CustomPlacementModeModuleLog, Log, TEXT("[%d] Asset AddedToCate1111gory - %s"), i, *BPClass->GetName());
		}
	}	
	
	UE_LOG(CustomPlacementModeModuleLog,Log,TEXT("Category - %s"), *Category->UniqueHandle.ToString());	

	//CategoryItemsToStream.Empty();
	//BPItems.Empty();
}

TArray<TAssetSubclassOf<AActor>> FCustomPlacementModeModule::RefreshAllCustomClassesWithBlueprint(FCategoryItem CategoryItem)
{
	// Load the asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked< FAssetRegistryModule >(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	// The asset registry is populated asynchronously at startup, so there's no guarantee it has finished.
	// This simple approach just runs a synchronous scan on the entire content directory.
	// Better solutions would be to specify only the path to where the relevant blueprints are,
	// or to register a callback with the asset registry to be notified of when it's finished populating.
	TArray< FString > ContentPaths;
	ContentPaths.Add(TEXT("/Game"));
	AssetRegistry.ScanPathsSynchronous(ContentPaths);

	TArray<TAssetSubclassOf<AActor>> SubClasses;

	FName CategoryName = CategoryItem.HandleName;

	// Unregister old stuff
	FCustomPlacementCategory* Category = Categories.Find(CategoryName);
	if (!Category)
	{
		return SubClasses;
	}

	//Category->Items.Reset();

	for (auto& Elem : CategoryItem.Items)
	{
		if (Elem)
		{
			FName BaseClassName = Elem->GetFName();

			// Use the asset registry to get the set of all class names deriving from Base
			TSet< FName > DerivedNames;
			{
				TArray< FName > BaseNames;
				BaseNames.Add(BaseClassName);

				TSet< FName > Excluded;
				AssetRegistry.GetDerivedClassNames(BaseNames, Excluded, DerivedNames);
			}

			FString Path;

			FARFilter Filter;
			Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
			Filter.bRecursiveClasses = true;
			if (!Path.IsEmpty())
			{
				Filter.PackagePaths.Add(*Path);
			}
			Filter.bRecursivePaths = true;

			TArray< FAssetData > AssetList;
			AssetRegistry.GetAssets(Filter, AssetList);


			// Iterate over retrieved blueprint assets
			for (auto const& Asset : AssetList)
			{
				// Get the the class this blueprint generates (this is stored as a full path)
				if (auto GeneratedClassPathPtr = Asset.TagsAndValues.Find(TEXT("GeneratedClass")))
				{
					// Convert path to just the name part
					const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(*GeneratedClassPathPtr);
					const FString ClassName = FPackageName::ObjectPathToObjectName(ClassObjectPath);

					// Check if this class is in the derived set
					if (!DerivedNames.Contains(*ClassName))
					{
						continue;
					}

					// Store using the path to the generated class
					SubClasses.Add(TAssetSubclassOf< AActor >(FStringAssetReference(ClassObjectPath)));
				}
			}
		}
	}

	return SubClasses;
}

TArray<TAssetSubclassOf<AActor>> FCustomPlacementModeModule::RefreshAllCustomClassesWithBlueprint(FName CategoryName)
{
	// Load the asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked< FAssetRegistryModule >(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	// The asset registry is populated asynchronously at startup, so there's no guarantee it has finished.
	// This simple approach just runs a synchronous scan on the entire content directory.
	// Better solutions would be to specify only the path to where the relevant blueprints are,
	// or to register a callback with the asset registry to be notified of when it's finished populating.
	TArray< FString > ContentPaths;
	ContentPaths.Add(TEXT("/Game"));
	AssetRegistry.ScanPathsSynchronous(ContentPaths);

	TArray<TAssetSubclassOf<AActor>> SubClasses;
	
	// Unregister old stuff
	FCustomPlacementCategory* Category = Categories.Find(CategoryName);
	if (!Category)
	{
		return SubClasses;
	}

	//Category->Items.Reset();
	FCategoryItem* CategoryItem = CustomCategories.FindByPredicate([CategoryName](const FCategoryItem& InItem)
	 {
		 return InItem.HandleName == CategoryName;
	 });

 //= CustomCategories.FindByPredicate(CategoryName);

	for (auto& Elem : CategoryItem->Items)
	{
		if (Elem)
		{
			FName BaseClassName = Elem->GetFName();

			// Use the asset registry to get the set of all class names deriving from Base
			TSet< FName > DerivedNames;
			{
				TArray< FName > BaseNames;
				BaseNames.Add(BaseClassName);

				TSet< FName > Excluded;
				AssetRegistry.GetDerivedClassNames(BaseNames, Excluded, DerivedNames);
			}

			FString Path;

			FARFilter Filter;
			Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
			Filter.bRecursiveClasses = true;
			if (!Path.IsEmpty())
			{
				Filter.PackagePaths.Add(*Path);
			}
			Filter.bRecursivePaths = true;

			TArray< FAssetData > AssetList;
			AssetRegistry.GetAssets(Filter, AssetList);


			// Iterate over retrieved blueprint assets
			for (auto const& Asset : AssetList)
			{
				// Get the the class this blueprint generates (this is stored as a full path)
				if (auto GeneratedClassPathPtr = Asset.TagsAndValues.Find(TEXT("GeneratedClass")))
				{
					// Convert path to just the name part
					const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(*GeneratedClassPathPtr);
					const FString ClassName = FPackageName::ObjectPathToObjectName(ClassObjectPath);

					// Check if this class is in the derived set
					if (!DerivedNames.Contains(*ClassName))
					{
						continue;
					}

					// Store using the path to the generated class
					SubClasses.Add(TAssetSubclassOf< AActor >(FStringAssetReference(ClassObjectPath)));

					UE_LOG(CustomPlacementModeModuleLog, Warning, TEXT("Asset founded - %s, ClassName - %s"), *ClassObjectPath, *ClassName);
				}
			}
		}
	}

	return SubClasses;
}

TArray<TAssetSubclassOf<AActor>> FCustomPlacementModeModule::RefreshAllClassesWithBlueprint()
{
	// Load the asset registry module
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked< FAssetRegistryModule >(FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();
	// The asset registry is populated asynchronously at startup, so there's no guarantee it has finished.
	// This simple approach just runs a synchronous scan on the entire content directory.
	// Better solutions would be to specify only the path to where the relevant blueprints are,
	// or to register a callback with the asset registry to be notified of when it's finished populating.
	TArray< FString > ContentPaths;
	ContentPaths.Add(TEXT("/Game"));
	AssetRegistry.ScanPathsSynchronous(ContentPaths);

	TArray<TAssetSubclassOf<AActor>> SubClasses;

	//FName CategoryName = CategoryItem.HandleName;

	// Unregister old stuff
	//FCustomPlacementCategory* Category = Categories.Find(CategoryName);
	//if (!Category)
	//{
	//	return SubClasses;
	//}

	//Category->Items.Reset();

	//for (auto& Elem : CategoryItem.Items)
	//{
		//if (Elem)
		//{
			FName BaseClassName = AActor::StaticClass()->GetFName();

			// Use the asset registry to get the set of all class names deriving from Base
			TSet< FName > DerivedNames;
			{
				TArray< FName > BaseNames;
				BaseNames.Add(BaseClassName);

				TSet< FName > Excluded;
				AssetRegistry.GetDerivedClassNames(BaseNames, Excluded, DerivedNames);
			}

			FString Path;

			FARFilter Filter;
			Filter.ClassNames.Add(UBlueprint::StaticClass()->GetFName());
			Filter.bRecursiveClasses = true;
			if (!Path.IsEmpty())
			{
				Filter.PackagePaths.Add(*Path);
			}
			Filter.bRecursivePaths = true;

			TArray< FAssetData > AssetList;
			AssetRegistry.GetAssets(Filter, AssetList);


			// Iterate over retrieved blueprint assets
			for (auto const& Asset : AssetList)
			{
				// Get the the class this blueprint generates (this is stored as a full path)
				if (auto GeneratedClassPathPtr = Asset.TagsAndValues.Find(TEXT("GeneratedClass")))
				{
					// Convert path to just the name part
					const FString ClassObjectPath = FPackageName::ExportTextPathToObjectPath(*GeneratedClassPathPtr);
					const FString ClassName = FPackageName::ObjectPathToObjectName(ClassObjectPath);

					// Check if this class is in the derived set
					if (!DerivedNames.Contains(*ClassName))
					{
						continue;
					}

					// Store using the path to the generated class
					SubClasses.Add(TAssetSubclassOf< AActor >(FStringAssetReference(ClassObjectPath)));
				}
			}
		//}
	//}

	//for (auto& Item : SubClasses)
	//{
	//	TSoftClassPtr<AActor> ActorItem = Item;

	//	UClass* BPClass = ActorItem.LoadSynchronous();
	//	if (BPClass)
	//	{
	//		UE_LOG(CustomPlacementModeModuleLog, Log, TEXT("Asset - %s"), *BPClass->GetName());
	//	}
	//}

	return SubClasses;
}

FGuid FCustomPlacementModeModule::CreateID()
{
	return FGuid::NewGuid();
}

FCustomPlacementModeID FCustomPlacementModeModule::CreateID(FName InCategory)
{
	FCustomPlacementModeID NewID;
	NewID.UniqueID = CreateID();
	NewID.Category = InCategory;
	return NewID;
}

IMPLEMENT_MODULE(FCustomPlacementModeModule, CustomCategoryModule);