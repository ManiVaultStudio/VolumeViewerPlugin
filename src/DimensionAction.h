#pragma once

#include "actions/GroupAction.h"
#include "actions/ToggleAction.h"
#include "actions/DecimalAction.h"

using namespace hdps::gui;

class RendererSettingsAction;
class Viewer3DPlugin;
class ViewerWidget;

/**
 * Slicing action class
 *
 * Action class for choosing dimensions
 *
 * @author Thomas Kroes and Mitchell M. de Boer
 */
class DimensionAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param rendererSettingsAction Reference to renderer settings action
     */
    DimensionAction(RendererSettingsAction& rendererSettingsAction, ViewerWidget* viewerWidet);

public: /** Action getters */

    DecimalAction& getChosenDimensionAction() { return _chosenDimensionAction; }

protected:
    RendererSettingsAction& _rendererSettingsAction;        /** Reference to renderer settings action */
    ViewerWidget* _viewerWidget;                            /** Pointer to the viewerWidget*/
    DecimalAction               _chosenDimensionAction;     /** X-axis position action */
    
};
