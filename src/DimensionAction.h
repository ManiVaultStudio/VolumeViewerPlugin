#pragma once

#include "actions/GroupAction.h"
#include "actions/ToggleAction.h"
#include "actions/DecimalAction.h"

#include <PointData/DimensionPickerAction.h>

using namespace hdps::gui;

class RendererSettingsAction;
class VolumeViewerPlugin;
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
    Q_INVOKABLE DimensionAction(RendererSettingsAction& rendererSettingsAction, ViewerWidget* viewerWidet, const QString& title);

public: /** Action getters */

    DimensionPickerAction& getDimensionPickerAction() { return _dimensionAction; }

protected:
    RendererSettingsAction& _rendererSettingsAction;        /** Reference to renderer settings action */
    ViewerWidget* _viewerWidget;                            /** Pointer to the viewerWidget*/
    
    DimensionPickerAction       _dimensionAction;           /** Action to pick the current dimension*/
    
};

Q_DECLARE_METATYPE(DimensionAction)