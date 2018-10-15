#pragma once

#include "CoreMinimal.h"
//#include "UObject/UObjectGlobals.h"
//#include "Modules/ModuleManager.h"

//#include "Styling/SlateColor.h"
//#include "Styling/SlateStyle.h"
//#include "Styling/SlateTypes.h"
//#include "ISettingsModule.h"
#include "CustomPlacementModeSettings.h"
#include "Styling/SlateStyle.h"

struct FPropertyChangedEvent;

struct FSlateDynamicImageBrush;

/**
 * A collection of named properties that guide the appearance of Slate.
 */
class FCustomEditorStyle
{
public:

	template< class T >
	static const T& GetWidgetStyle(FName PropertyName, const ANSICHAR* Specifier = NULL)
	{
		return Instance->GetWidgetStyle< T >(PropertyName, Specifier);
	}

	static float GetFloat(FName PropertyName, const ANSICHAR* Specifier = NULL)
	{
		return Instance->GetFloat(PropertyName, Specifier);
	}

	static FVector2D GetVector(FName PropertyName, const ANSICHAR* Specifier = NULL)
	{
		return Instance->GetVector(PropertyName, Specifier);
	}

	static const FLinearColor& GetColor(FName PropertyName, const ANSICHAR* Specifier = NULL)
	{
		return Instance->GetColor(PropertyName, Specifier);
	}

	static const FSlateColor GetSlateColor(FName PropertyName, const ANSICHAR* Specifier = NULL)
	{
		return Instance->GetSlateColor(PropertyName, Specifier);
	}

	static const FMargin& GetMargin(FName PropertyName, const ANSICHAR* Specifier = NULL)
	{
		return Instance->GetMargin(PropertyName, Specifier);
	}

	static const FSlateBrush* GetBrush(FName PropertyName, const ANSICHAR* Specifier = NULL)
	{
		return Instance->GetBrush(PropertyName, Specifier);
	}

	static const TSharedPtr< FSlateDynamicImageBrush > GetDynamicImageBrush(FName BrushTemplate, FName TextureName, const ANSICHAR* Specifier = NULL)
	{
		return Instance->GetDynamicImageBrush(BrushTemplate, TextureName, Specifier);
	}

	static const TSharedPtr< FSlateDynamicImageBrush > GetDynamicImageBrush(FName BrushTemplate, const ANSICHAR* Specifier, class UTexture2D* TextureResource, FName TextureName)
	{
		return Instance->GetDynamicImageBrush(BrushTemplate, Specifier, TextureResource, TextureName);
	}

	static const TSharedPtr< FSlateDynamicImageBrush > GetDynamicImageBrush(FName BrushTemplate, class UTexture2D* TextureResource, FName TextureName)
	{
		return Instance->GetDynamicImageBrush(BrushTemplate, TextureResource, TextureName);
	}

	static const FSlateSound& GetSound(FName PropertyName, const ANSICHAR* Specifier = NULL)
	{
		return Instance->GetSound(PropertyName, Specifier);
	}

	static FSlateFontInfo GetFontStyle(FName PropertyName, const ANSICHAR* Specifier = NULL)
	{
		return Instance->GetFontStyle(PropertyName, Specifier);
	}

	static const FSlateBrush* GetDefaultBrush()
	{
		return Instance->GetDefaultBrush();
	}

	static const FSlateBrush* GetNoBrush()
	{
		return FStyleDefaults::GetNoBrush();
	}

	static const FSlateBrush* GetOptionalBrush(FName PropertyName, const ANSICHAR* Specifier = NULL, const FSlateBrush* const DefaultBrush = FStyleDefaults::GetNoBrush())
	{
		return Instance->GetOptionalBrush(PropertyName, Specifier, DefaultBrush);
	}

	static void GetResources(TArray< const FSlateBrush* >& OutResources)
	{
		return Instance->GetResources(OutResources);
	}

	static ISlateStyle& Get()
	{
		return *(Instance.Get());
	}

	static const FName& GetStyleSetName()
	{
		return Instance->GetStyleSetName();
	}

	/**
	 * Concatenates two FNames.e If A and B are "Path.To" and ".Something"
	 * the result "Path.To.Something".
	 *
	 * @param A  First FName
	 * @param B  Second name
	 *
	 * @return New FName that is A concatenated with B.
	 */
	static FName Join(FName A, const ANSICHAR* B)
	{
		if (B == NULL)
		{
			return A;
		}
		else
		{
			return FName(*(A.ToString() + B));
		}
	}

	static void ResetToDefault();

protected:

	static void SetStyle(const TSharedRef< class ISlateStyle >& NewStyle);

private:

	/** Singleton instance of the slate style */
	static TSharedPtr< class ISlateStyle > Instance;
};









class FCustomCategoryStyle: public FCustomEditorStyle
{
public:
	// Initializes the value of MenuStyleInstance and registers it with the Slate Style Registry.
	static void Initialize()
	{
		Settings = nullptr;

#if WITH_EDITOR
		Settings = GetMutableDefault<UCustomPlacementModeSettings>();
		//ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");


		//if (SettingsModule != nullptr)
		//{
		//	SettingsModule->RegisterSettings("Editor", "CustomPlacementModeSettings", "Appearance",
		//		NSLOCTEXT("CustomEditorStyle", "Appearance_UserSettingsName", "Appearance"),
		//		NSLOCTEXT("CustomEditorStyle", "Appearance_UserSettingsDescription", "Customize the look of the custom category."),
		//		Settings
		//	);
		//}
#endif

		CustomCategoryStyleInstance = Create(Settings);
		SetStyle(CustomCategoryStyleInstance.ToSharedRef());
	}

	// Unregisters the Slate Style Set and then resets the MenuStyleInstance pointer.
	static void Shutdown()
	{
		//ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings");

		//if (SettingsModule != nullptr)
		//{
		//	SettingsModule->UnregisterSettings("Editor", "CustomPlacementModeSettings", "Appearance");
		//}

		ResetToDefault();

		ensure(CustomCategoryStyleInstance.IsUnique());
		CustomCategoryStyleInstance.Reset();
	}

	static void SyncCustomizations()
	{
		FCustomCategoryStyle::CustomCategoryStyleInstance->SyncSettings();
	}


//// SlateStyleSet

	class FCustomCoreStyle : public FSlateStyleSet
	{
	public:
		FCustomCoreStyle(const TWeakObjectPtr< UCustomPlacementModeSettings >& InSettings);

		void Initialize();
		void SetupCustomCategoryStyles();

		void SettingsChanged(UObject* ChangedObject, FPropertyChangedEvent& PropertyChangedEvent);
		void SyncSettings();

		// These are the colors that are updated by the user style customizations
		const TSharedRef< FLinearColor > DefaultForeground_LinearRef;
		const TSharedRef< FLinearColor > SelectionColor_LinearRef;
		const TSharedRef< FLinearColor > SelectionColor_Pressed_LinearRef;


		/** Default Category Colors */
		const TSharedRef< FLinearColor > RecentlyPlacedForeground_LinearRef;
		const TSharedRef< FLinearColor > RecentlyPlacedSelectionColor_LinearRef;
		const TSharedRef< FLinearColor > RecentlyPlacedSelectionColor_Pressed_LinearRef;
		const TSharedRef< FLinearColor > RecentlyPlacedText_LinearRef;
		const TSharedRef< FLinearColor > RecentlyPlacedTextShadow_LinearRef;

		const TSharedRef< FLinearColor > BasicForeground_LinearRef;
		const TSharedRef< FLinearColor > BasicSelectionColor_LinearRef;
		const TSharedRef< FLinearColor > BasicSelectionColor_Pressed_LinearRef;
		const TSharedRef< FLinearColor > BasicText_LinearRef;
		const TSharedRef< FLinearColor > BasicTextShadow_LinearRef;

		const TSharedRef< FLinearColor > LightsForeground_LinearRef;
		const TSharedRef< FLinearColor > LightsSelectionColor_LinearRef;
		const TSharedRef< FLinearColor > LightsSelectionColor_Pressed_LinearRef;
		const TSharedRef< FLinearColor > LightsText_LinearRef;
		const TSharedRef< FLinearColor > LightsTextShadow_LinearRef;

		const TSharedRef< FLinearColor > VisualForeground_LinearRef;
		const TSharedRef< FLinearColor > VisualSelectionColor_LinearRef;
		const TSharedRef< FLinearColor > VisualSelectionColor_Pressed_LinearRef;
		const TSharedRef< FLinearColor > VisualText_LinearRef;
		const TSharedRef< FLinearColor > VisualTextShadow_LinearRef;

		const TSharedRef< FLinearColor > VolumesForeground_LinearRef;
		const TSharedRef< FLinearColor > VolumesSelectionColor_LinearRef;
		const TSharedRef< FLinearColor > VolumesSelectionColor_Pressed_LinearRef;
		const TSharedRef< FLinearColor > VolumesText_LinearRef;
		const TSharedRef< FLinearColor > VolumesTextShadow_LinearRef;

		const TSharedRef< FLinearColor > AllClassesForeground_LinearRef;
		const TSharedRef< FLinearColor > AllClassesSelectionColor_LinearRef;
		const TSharedRef< FLinearColor > AllClassesSelectionColor_Pressed_LinearRef;
		const TSharedRef< FLinearColor > AllClassesText_LinearRef;
		const TSharedRef< FLinearColor > AllClassesTextShadow_LinearRef;


		TArray<TSharedRef< FLinearColor >> CustomForeground_LinearRefs;
		TArray<TSharedRef< FLinearColor >> CustomSelectionColor_LinearRefs;
		TArray<TSharedRef< FLinearColor >> CustomSelectionColor_Pressed_LinearRefs;
		TArray<TSharedRef< FLinearColor >> CustomTextColor_LinearRefs;
		TArray<TSharedRef< FLinearColor >> CustomTextShadowColor_LinearRefs;

		// These are the Slate colors which reference those above; these are the colors to put into the style
		const FSlateColor DefaultForeground;
		const FSlateColor SelectionColor;
		const FSlateColor SelectionColor_Pressed;


		/** Default Category Colors */
		const FSlateColor RecentlyPlacedForeground;
		const FSlateColor RecentlyPlacedSelectionColor;
		const FSlateColor RecentlyPlacedSelectionColor_Pressed;
		const FSlateColor RecentlyPlacedText;
		const FSlateColor RecentlyPlacedTextShadow;

		const FSlateColor BasicForeground;
		const FSlateColor BasicSelectionColor;
		const FSlateColor BasicSelectionColor_Pressed;
		const FSlateColor BasicText;
		const FSlateColor BasicTextShadow;

		const FSlateColor LightsForeground;
		const FSlateColor LightsSelectionColor;
		const FSlateColor LightsSelectionColor_Pressed;
		const FSlateColor LightsText;
		const FSlateColor LightsTextShadow;

		const FSlateColor VisualForeground;
		const FSlateColor VisualSelectionColor;
		const FSlateColor VisualSelectionColor_Pressed;
		const FSlateColor VisualText;
		const FSlateColor VisualTextShadow;

		const FSlateColor VolumesForeground;
		const FSlateColor VolumesSelectionColor;
		const FSlateColor VolumesSelectionColor_Pressed;
		const FSlateColor VolumesText;
		const FSlateColor VolumesTextShadow;

		const FSlateColor AllClassesForeground;
		const FSlateColor AllClassesSelectionColor;
		const FSlateColor AllClassesSelectionColor_Pressed;
		const FSlateColor AllClassesText;
		const FSlateColor AllClassesTextShadow;

		// These are the Slate colors which reference those above; these are the colors to put into the style
		TArray<FSlateColor> CustomForegrounds;
		TArray<FSlateColor> CustomSelectionColors;
		TArray<FSlateColor> CustomSelectionColors_Pressed;
		TArray<FSlateColor> CustomTextColors;
		TArray<FSlateColor> CustomTextShadowColors;
		 
		FTextBlockStyle NormalText;

		TWeakObjectPtr< UCustomPlacementModeSettings > Settings;

	};


///////////////////////////////////////////////////////////

	static TSharedRef< class FCustomCategoryStyle::FCustomCoreStyle > Create(const TWeakObjectPtr< UCustomPlacementModeSettings >& InCustomization)
	{
		TSharedRef< class FCustomCategoryStyle::FCustomCoreStyle > NewStyle = MakeShareable(new FCustomCategoryStyle::FCustomCoreStyle(InCustomization));
		NewStyle->Initialize();

#if WITH_EDITOR
		FCoreUObjectDelegates::OnObjectPropertyChanged.AddSP(NewStyle, &FCustomCategoryStyle::FCustomCoreStyle::SettingsChanged);
#endif

		return NewStyle;
	}

	// Singleton instance used for our Style Set.
	static TSharedPtr< FCustomCategoryStyle::FCustomCoreStyle > CustomCategoryStyleInstance;
	static TWeakObjectPtr< UCustomPlacementModeSettings > Settings;

};

DECLARE_LOG_CATEGORY_EXTERN(CustomCategoryStylesLog, Log, All)