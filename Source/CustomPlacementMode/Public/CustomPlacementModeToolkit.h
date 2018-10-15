#pragma once

#include "CoreMinimal.h"
//#include "Widgets/DeclarativeSyntaxSupport.h"
//#include "Widgets/SWidget.h"
#include "EditorModes.h"
#include "Toolkits/BaseToolkit.h"
//#include "EditorModeManager.h"
#include "SCustomPlacementModeTools.h"

class FCustomPlacementModeToolkit : public FModeToolkit
{
public:

	FCustomPlacementModeToolkit()
	{
		SAssignNew(PlacementModeTools, SCustomPlacementModeTools);
	}

	/** IToolkit interface */
	virtual FName GetToolkitFName() const override { return FName("CustomPlacementMode"); }
	virtual FText GetBaseToolkitName() const override { return NSLOCTEXT("BuilderModeToolkit", "DisplayName", "Builder"); }
	virtual class FEdMode* GetEditorMode() const override { return GLevelEditorModeTools().GetActiveMode(FBuiltinEditorModes::EM_Placement); }
	virtual TSharedPtr<class SWidget> GetInlineContent() const override { return PlacementModeTools; }

private:

	TSharedPtr<SCustomPlacementModeTools> PlacementModeTools;
};