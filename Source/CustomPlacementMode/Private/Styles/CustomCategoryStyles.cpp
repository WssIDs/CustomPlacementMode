
#include "CustomCategoryStyles.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateStyleRegistry.h"

DEFINE_LOG_CATEGORY(CustomCategoryStylesLog)


#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)

TSharedPtr<FCustomCategoryStyle::FCustomCoreStyle> FCustomCategoryStyle::CustomCategoryStyleInstance = nullptr;
TWeakObjectPtr< UCustomPlacementModeSettings > FCustomCategoryStyle::Settings = nullptr;

/* START CORE STYLE */

TSharedPtr< ISlateStyle > FCustomEditorStyle::Instance = nullptr;

void FCustomEditorStyle::ResetToDefault()
{
	SetStyle(FCoreStyle::Create("CustomCategoryStyle"));
}

void FCustomEditorStyle::SetStyle(const TSharedRef< ISlateStyle >& NewStyle)
{
	if (Instance != NewStyle)
	{
		if (Instance.IsValid())
		{
			FSlateStyleRegistry::UnRegisterSlateStyle(*Instance.Get());
		}

		Instance = NewStyle;

		if (Instance.IsValid())
		{
			FSlateStyleRegistry::RegisterSlateStyle(*Instance.Get());
		}
		else
		{
			ResetToDefault();
		}
	}
}




/* FCustomCategoryStyle interface
 *****************************************************************************/

FCustomCategoryStyle::FCustomCoreStyle::FCustomCoreStyle(const TWeakObjectPtr< UCustomPlacementModeSettings >& InSettings)
	: FSlateStyleSet("CustomCategoryStyle")

	// These are the colors that are updated by the user style customizations
	, DefaultForeground_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, SelectionColor_LinearRef(MakeShareable(new FLinearColor(0.728f, 0.364f, 0.003f)))
	, SelectionColor_Pressed_LinearRef(MakeShareable(new FLinearColor(0.701f, 0.225f, 0.003f)))
	// Recently
	, RecentlyPlacedForeground_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, RecentlyPlacedSelectionColor_LinearRef(MakeShareable(new FLinearColor(0.728f, 0.364f, 0.003f)))
	, RecentlyPlacedSelectionColor_Pressed_LinearRef(MakeShareable(new FLinearColor(0.701f, 0.225f, 0.003f)))
	, RecentlyPlacedText_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, RecentlyPlacedTextShadow_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	// Basic
	, BasicForeground_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, BasicSelectionColor_LinearRef(MakeShareable(new FLinearColor(0.728f, 0.364f, 0.003f)))
	, BasicSelectionColor_Pressed_LinearRef(MakeShareable(new FLinearColor(0.701f, 0.225f, 0.003f)))
	, BasicText_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, BasicTextShadow_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	// Lights
	, LightsForeground_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, LightsSelectionColor_LinearRef(MakeShareable(new FLinearColor(0.728f, 0.364f, 0.003f)))
	, LightsSelectionColor_Pressed_LinearRef(MakeShareable(new FLinearColor(0.701f, 0.225f, 0.003f)))
	, LightsText_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, LightsTextShadow_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	// Visual
	, VisualForeground_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, VisualSelectionColor_LinearRef(MakeShareable(new FLinearColor(0.728f, 0.364f, 0.003f)))
	, VisualSelectionColor_Pressed_LinearRef(MakeShareable(new FLinearColor(0.701f, 0.225f, 0.003f)))
	, VisualText_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, VisualTextShadow_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	// Volumes
	, VolumesForeground_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, VolumesSelectionColor_LinearRef(MakeShareable(new FLinearColor(0.728f, 0.364f, 0.003f)))
	, VolumesSelectionColor_Pressed_LinearRef(MakeShareable(new FLinearColor(0.701f, 0.225f, 0.003f)))
	, VolumesText_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, VolumesTextShadow_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	// AllClasses
	, AllClassesForeground_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, AllClassesSelectionColor_LinearRef(MakeShareable(new FLinearColor(0.728f, 0.364f, 0.003f)))
	, AllClassesSelectionColor_Pressed_LinearRef(MakeShareable(new FLinearColor(0.701f, 0.225f, 0.003f)))
	, AllClassesText_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	, AllClassesTextShadow_LinearRef(MakeShareable(new FLinearColor(0.72f, 0.72f, 0.72f, 1.f)))
	// These are the Slate colors which reference those above; these are the colors to put into the style
	, DefaultForeground(DefaultForeground_LinearRef)
	, SelectionColor(SelectionColor_LinearRef)
	, SelectionColor_Pressed(SelectionColor_Pressed_LinearRef)
	// Default Category Colors
	// Recently
	, RecentlyPlacedForeground(RecentlyPlacedForeground_LinearRef)
	, RecentlyPlacedSelectionColor(RecentlyPlacedSelectionColor_LinearRef)
	, RecentlyPlacedSelectionColor_Pressed(RecentlyPlacedSelectionColor_Pressed_LinearRef)
	, RecentlyPlacedText(RecentlyPlacedText_LinearRef)
	, RecentlyPlacedTextShadow(RecentlyPlacedTextShadow_LinearRef)
	// Basic
	, BasicForeground(BasicForeground_LinearRef)
	, BasicSelectionColor(BasicSelectionColor_LinearRef)
	, BasicSelectionColor_Pressed(BasicSelectionColor_Pressed_LinearRef)
	, BasicText(BasicText_LinearRef)
	, BasicTextShadow(BasicTextShadow_LinearRef)
	// Light
	, LightsForeground(LightsForeground_LinearRef)
	, LightsSelectionColor(LightsSelectionColor_LinearRef)
	, LightsSelectionColor_Pressed(LightsSelectionColor_Pressed_LinearRef)
	, LightsText(LightsText_LinearRef)
	, LightsTextShadow(LightsTextShadow_LinearRef)
	// Visual
	, VisualForeground(VisualForeground_LinearRef)
	, VisualSelectionColor(VisualSelectionColor_LinearRef)
	, VisualSelectionColor_Pressed(VisualSelectionColor_Pressed_LinearRef)
	, VisualText(VisualText_LinearRef)
	, VisualTextShadow(VisualTextShadow_LinearRef)
	// Volumes
	, VolumesForeground(VolumesForeground_LinearRef)
	, VolumesSelectionColor(VolumesSelectionColor_LinearRef)
	, VolumesSelectionColor_Pressed(VolumesSelectionColor_Pressed_LinearRef)
	, VolumesText(VolumesText_LinearRef)
	, VolumesTextShadow(VolumesTextShadow_LinearRef)
	// AllClasses
	, AllClassesForeground(AllClassesForeground_LinearRef)
	, AllClassesSelectionColor(AllClassesSelectionColor_LinearRef)
	, AllClassesSelectionColor_Pressed(AllClassesSelectionColor_Pressed_LinearRef)
	, AllClassesText(AllClassesText_LinearRef)
	, AllClassesTextShadow(AllClassesTextShadow_LinearRef)
	, Settings(InSettings)
{
	if(InSettings->PlaceableCategoryItems.Num() > 0)
	{
		for (auto& Elem : InSettings->PlaceableCategoryItems)
		{
			CustomForeground_LinearRefs.Add(MakeShareable(new FLinearColor(0.1f, 0.1f, 0.1f, 0.08f)));
			CustomSelectionColor_LinearRefs.Add(MakeShareable(new FLinearColor(0.03f, 0.03f, 0.03f, 1.0f)));
			CustomSelectionColor_Pressed_LinearRefs.Add(MakeShareable(new FLinearColor(0.02f, 0.02f, 0.02f, 1.0f)));
			CustomTextColor_LinearRefs.Add(MakeShareable(new FLinearColor(1.0f, 1.0f, 1.0f, 0.9f)));
			CustomTextShadowColor_LinearRefs.Add(MakeShareable(new FLinearColor(0.0f, 0.0f, 0.0f, 0.9f)));
		}

		if(CustomForeground_LinearRefs.Num() > 0 &&
			CustomSelectionColor_LinearRefs.Num() > 0 &&
			CustomSelectionColor_Pressed_LinearRefs.Num() > 0)
		{
			for ( int i = 0; i < InSettings->PlaceableCategoryItems.Num(); i++)
			{
				CustomForegrounds.Add(CustomForeground_LinearRefs[i]);
				CustomSelectionColors.Add(CustomSelectionColor_LinearRefs[i]);
				CustomSelectionColors_Pressed.Add(CustomSelectionColor_Pressed_LinearRefs[i]);
				CustomTextColors.Add(CustomTextColor_LinearRefs[i]);
				CustomTextShadowColors.Add(CustomTextShadowColor_LinearRefs[i]);
			}
		}
	}
}

void SetColor(const TSharedRef< FLinearColor >& Source, const FLinearColor& Value)
{
	Source->R = Value.R;
	Source->G = Value.G;
	Source->B = Value.B;
	Source->A = Value.A;
}

void FCustomCategoryStyle::FCustomCoreStyle::SettingsChanged(UObject* ChangedObject, FPropertyChangedEvent& PropertyChangedEvent)
{
	if (ChangedObject == Settings.Get())
	{
		SyncSettings();
	}
}

void FCustomCategoryStyle::FCustomCoreStyle::SyncSettings()
{
	if (Settings.IsValid())
	{
		SetColor(RecentlyPlacedForeground_LinearRef, Settings->RecentlyPlaced.BackgroundColor);
		SetColor(RecentlyPlacedSelectionColor_LinearRef, Settings->RecentlyPlaced.HoveredColor);
		SetColor(RecentlyPlacedSelectionColor_Pressed_LinearRef, Settings->RecentlyPlaced.PressedSelectionColor);
		SetColor(RecentlyPlacedText_LinearRef, Settings->RecentlyPlaced.TextColor);
		SetColor(RecentlyPlacedTextShadow_LinearRef, Settings->RecentlyPlaced.TextShadowColor);

		SetColor(BasicForeground_LinearRef, Settings->Basic.BackgroundColor);
		SetColor(BasicSelectionColor_LinearRef, Settings->Basic.HoveredColor);
		SetColor(BasicSelectionColor_Pressed_LinearRef, Settings->Basic.PressedSelectionColor);
		SetColor(BasicText_LinearRef, Settings->Basic.TextColor);
		SetColor(BasicTextShadow_LinearRef, Settings->Basic.TextShadowColor);

		SetColor(LightsForeground_LinearRef, Settings->Lights.BackgroundColor);
		SetColor(LightsSelectionColor_LinearRef, Settings->Lights.HoveredColor);
		SetColor(LightsSelectionColor_Pressed_LinearRef, Settings->Lights.PressedSelectionColor);
		SetColor(LightsText_LinearRef, Settings->Lights.TextColor);
		SetColor(LightsTextShadow_LinearRef, Settings->Lights.TextShadowColor);

		SetColor(VisualForeground_LinearRef, Settings->Visual.BackgroundColor);
		SetColor(VisualSelectionColor_LinearRef, Settings->Visual.HoveredColor);
		SetColor(VisualSelectionColor_Pressed_LinearRef, Settings->Visual.PressedSelectionColor);
		SetColor(VisualText_LinearRef, Settings->Visual.TextColor);
		SetColor(VisualTextShadow_LinearRef, Settings->Visual.TextShadowColor);

		SetColor(VolumesForeground_LinearRef, Settings->Volumes.BackgroundColor);
		SetColor(VolumesSelectionColor_LinearRef, Settings->Volumes.HoveredColor);
		SetColor(VolumesSelectionColor_Pressed_LinearRef, Settings->Volumes.PressedSelectionColor);
		SetColor(VolumesText_LinearRef, Settings->Volumes.TextColor);
		SetColor(VolumesTextShadow_LinearRef, Settings->Volumes.TextShadowColor);

		SetColor(AllClassesForeground_LinearRef, Settings->AllClasses.BackgroundColor);
		SetColor(AllClassesSelectionColor_LinearRef, Settings->AllClasses.HoveredColor);
		SetColor(AllClassesSelectionColor_Pressed_LinearRef, Settings->AllClasses.PressedSelectionColor);
		SetColor(AllClassesText_LinearRef, Settings->AllClasses.TextColor);
		SetColor(AllClassesTextShadow_LinearRef, Settings->AllClasses.TextShadowColor);

		TArray< FLinearColor > Backgrounds;
		TArray< FLinearColor > Selections;
		TArray< FLinearColor > Selections_Pressed;
		TArray< FLinearColor > TextColors;
		TArray< FLinearColor > TextShadowColors;

		if(Settings->PlaceableCategoryItems.Num() > 0)
		{
			if(Settings->PlaceableCategoryItems.Num() == CustomForeground_LinearRefs.Num())
			{
				for (auto& Elem : Settings->PlaceableCategoryItems)
				{
					Backgrounds.Add(Elem.ColorSetting.BackgroundColor);
					Selections.Add(Elem.ColorSetting.HoveredColor);
					Selections_Pressed.Add(Elem.ColorSetting.PressedSelectionColor);
					TextColors.Add(Elem.ColorSetting.TextColor);
					TextShadowColors.Add(Elem.ColorSetting.TextShadowColor);
				}

				for (int i = 0; i < Settings->PlaceableCategoryItems.Num(); i++)
				{
					SetColor(CustomForeground_LinearRefs[i], Backgrounds[i]);
					SetColor(CustomSelectionColor_LinearRefs[i], Selections[i]);
					SetColor(CustomSelectionColor_Pressed_LinearRefs[i], Selections_Pressed[i]);
					SetColor(CustomTextColor_LinearRefs[i], TextColors[i]);
					SetColor(CustomTextShadowColor_LinearRefs[i], TextShadowColors[i]);
				}
			}
		}
	}
}


void FCustomCategoryStyle::FCustomCoreStyle::Initialize()
{
	//@Todo slate: splitting game and style atlases is a better solution to avoiding editor textures impacting game atlas pages. Tho this would still be a loading win.
	// We do WITH_EDITOR and well as !GIsEditor because in UFE !GIsEditor is true, however we need the styles.
#if WITH_EDITOR
	if (!GIsEditor)
	{
		return;
	}
#endif

	SyncSettings();

	SetContentRoot(FPaths::ProjectPluginsDir() / "CustomPlacementMode" / "Content" / "Slate");

	SetupCustomCategoryStyles();
}


void FCustomCategoryStyle::FCustomCoreStyle::SetupCustomCategoryStyles()
{
	////////////// Init
	
	// Normal Text
	NormalText = FTextBlockStyle()
		.SetFont(DEFAULT_FONT("Regular", 9))
		.SetColorAndOpacity(FSlateColor::UseForeground())
		.SetShadowOffset(FVector2D::ZeroVector)
		.SetShadowColorAndOpacity(FLinearColor::Black)
		.SetHighlightColor(FLinearColor(0.02f, 0.3f, 0.0f))
		.SetHighlightShape(BOX_BRUSH("Common/TextBlockHighlightShape", FMargin(3.f / 8.f)));




	Set("CustomPlacementBrowser.Tab", FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, DefaultForeground))
		.SetUncheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, SelectionColor_Pressed)) // TabActive
		.SetUncheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, SelectionColor))
		.SetCheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, SelectionColor_Pressed)) // TabActive
		.SetCheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, SelectionColor_Pressed)) // TabActive
		.SetCheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, SelectionColor_Pressed)) // TabActive
		.SetPadding(0));
		
		
	/** Recently Placed */
	Set("CustomPlacementBrowser.TabRecentlyPlaced", FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, RecentlyPlacedForeground))
		.SetUncheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, RecentlyPlacedSelectionColor_Pressed)) // TabActive
		.SetUncheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, RecentlyPlacedSelectionColor))
		.SetCheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, RecentlyPlacedSelectionColor_Pressed)) // TabActive
		.SetCheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, RecentlyPlacedSelectionColor_Pressed)) // TabActive
		.SetCheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, RecentlyPlacedSelectionColor_Pressed)) // TabActive
		.SetPadding(0));

	Set( "CustomPlacementBrowser.Tab.TextRecentlyPlaced", FTextBlockStyle( NormalText )
		.SetFont( DEFAULT_FONT( "Bold", 10 ) )
		.SetColorAndOpacity( RecentlyPlacedText )
		.SetShadowOffset( FVector2D( 1, 1 ) )
		.SetShadowColorAndOpacity( RecentlyPlacedTextShadow.GetSpecifiedColor() ));


	/** Basic */
	Set("CustomPlacementBrowser.TabBasic", FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, BasicForeground))
		.SetUncheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, BasicSelectionColor_Pressed)) // TabActive
		.SetUncheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, BasicSelectionColor))
		.SetCheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, BasicSelectionColor_Pressed)) // TabActive
		.SetCheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, BasicSelectionColor_Pressed)) // TabActive
		.SetCheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, BasicSelectionColor_Pressed)) // TabActive
		.SetPadding(0));

	Set("CustomPlacementBrowser.Tab.TextBasic", FTextBlockStyle(NormalText)
		.SetFont(DEFAULT_FONT("Bold", 10))
		.SetColorAndOpacity(BasicText)
		.SetShadowOffset(FVector2D(1, 1))
		.SetShadowColorAndOpacity(BasicTextShadow.GetSpecifiedColor()));

	/** Lights */
	Set("CustomPlacementBrowser.TabLights", FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, LightsForeground))
		.SetUncheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, LightsSelectionColor_Pressed)) // TabActive
		.SetUncheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, LightsSelectionColor))
		.SetCheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, LightsSelectionColor_Pressed)) // TabActive
		.SetCheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, LightsSelectionColor_Pressed)) // TabActive
		.SetCheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, LightsSelectionColor_Pressed)) // TabActive
		.SetPadding(0));		
		
	Set("CustomPlacementBrowser.Tab.TextLights", FTextBlockStyle(NormalText)
		.SetFont(DEFAULT_FONT("Bold", 10))
		.SetColorAndOpacity(LightsText)
		.SetShadowOffset(FVector2D(1, 1))
		.SetShadowColorAndOpacity(LightsTextShadow.GetSpecifiedColor()));

	/** Visual */
	Set("CustomPlacementBrowser.TabVisual", FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VisualForeground))
		.SetUncheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VisualSelectionColor_Pressed)) // TabActive
		.SetUncheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VisualSelectionColor))
		.SetCheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VisualSelectionColor_Pressed)) // TabActive
		.SetCheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VisualSelectionColor_Pressed)) // TabActive
		.SetCheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VisualSelectionColor_Pressed)) // TabActive
		.SetPadding(0));	

	Set("CustomPlacementBrowser.Tab.TextVisual", FTextBlockStyle(NormalText)
		.SetFont(DEFAULT_FONT("Bold", 10))
		.SetColorAndOpacity(VisualText)
		.SetShadowOffset(FVector2D(1, 1))
		.SetShadowColorAndOpacity(VisualTextShadow.GetSpecifiedColor()));

	/** Volumes */
	Set("CustomPlacementBrowser.TabVolumes", FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VolumesForeground))
		.SetUncheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VolumesSelectionColor_Pressed)) // TabActive
		.SetUncheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VolumesSelectionColor))
		.SetCheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VolumesSelectionColor_Pressed)) // TabActive
		.SetCheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VolumesSelectionColor_Pressed)) // TabActive
		.SetCheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, VolumesSelectionColor_Pressed)) // TabActive
		.SetPadding(0));	

	Set("CustomPlacementBrowser.Tab.TextVolumes", FTextBlockStyle(NormalText)
		.SetFont(DEFAULT_FONT("Bold", 10))
		.SetColorAndOpacity(VolumesText)
		.SetShadowOffset(FVector2D(1, 1))
		.SetShadowColorAndOpacity(VolumesTextShadow.GetSpecifiedColor()));

	/** AllClasses */
	Set("CustomPlacementBrowser.TabAllClasses", FCheckBoxStyle()
		.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
		.SetUncheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, AllClassesForeground))
		.SetUncheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, AllClassesSelectionColor_Pressed)) // TabActive
		.SetUncheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, AllClassesSelectionColor))
		.SetCheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, AllClassesSelectionColor_Pressed)) // TabActive
		.SetCheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, AllClassesSelectionColor_Pressed)) // TabActive
		.SetCheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, AllClassesSelectionColor_Pressed)) // TabActive
		.SetPadding(0));			




	Set("CustomPlacementBrowser.Tab.TextAllClasses", FTextBlockStyle(NormalText)
		.SetFont(DEFAULT_FONT("Bold", 10))
		.SetColorAndOpacity(AllClassesText)
		.SetShadowOffset(FVector2D(1, 1))
		.SetShadowColorAndOpacity(AllClassesTextShadow.GetSpecifiedColor()));

	TArray< FName > Categories;

	if(Settings->PlaceableCategoryItems.Num() > 0)
	{
		for (auto& Elem : Settings->PlaceableCategoryItems)
		{
			FName Item = Elem.CategoryName;

			Categories.Add(Item);
		}

		int i = 0;
		for (auto& Elem : Settings->PlaceableCategoryItems)
		{
			FString Tab = TEXT("CustomPlacementBrowser.Tab");
			FName PlacementTab = FName(*Tab.Append(Categories[i].ToString()));

			FString TabText = TEXT("CustomPlacementBrowser.Tab.Text");
			FName PlacementTabText = FName(*TabText.Append(Categories[i].ToString()));

			Set(PlacementTab, FCheckBoxStyle()
				.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
				.SetUncheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, CustomForegrounds[i]))
				.SetUncheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, CustomSelectionColors_Pressed[i])) // TabActive
				.SetUncheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, CustomSelectionColors[i]))
				.SetCheckedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, CustomSelectionColors_Pressed[i])) // TabActive
				.SetCheckedHoveredImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, CustomSelectionColors_Pressed[i])) // TabActive
				.SetCheckedPressedImage(BOX_BRUSH("Common/Selection", 8.0f / 32.0f, CustomSelectionColors_Pressed[i])) // TabActive
				.SetPadding(0));

			Set(PlacementTabText, FTextBlockStyle(NormalText)
				.SetFont(DEFAULT_FONT("Bold", 10))
				.SetColorAndOpacity(CustomTextColors[i])
				.SetShadowOffset(FVector2D(1, 1))
				.SetShadowColorAndOpacity(CustomTextShadowColors[i].GetSpecifiedColor()));
				
			i++;
		}
	}
}


#undef BOX_BRUSH
#undef DEFAULT_FONT