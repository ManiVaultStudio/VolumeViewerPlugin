#pragma once

#include "actions/GroupsAction.h"

#include "SlicingAction.h"
#include "ColoringAction.h"
#include "DimensionAction.h"
#include "SelectedPointsAction.h"
#include "ColoringActionPointcloud.h"

class VolumeViewer;

using namespace hdps::gui;

/**
 * Renderer settings action class
 *
 * Action class for rendering settings
 *
 * @author Thomas Kroes
 */
class RendererSettingsAction : public GroupsAction
{
    //Q_OBJECT

public:

    /** 
     * Constructor
     * @param parent Pointer to parent object
     */
    Q_INVOKABLE RendererSettingsAction(QObject* parent, const QString& title);

public: /** Action getters */
    DimensionAction& getDimensionAction() { return _dimensionAction; }

    SlicingAction& getSlicingAction() { return _slicingAction; }

    ColoringAction& getColoringAction() { return _coloringAction; }

    SelectedPointsAction& getSelectedPointsAction() { return _selectedPointsAction; }
    RendererSettingsAction& getRenderSettingsAction() { return *this; }
    

protected:
    DimensionAction _dimensionAction;
    SlicingAction   _slicingAction;     /** Slicing action */
    ColoringAction  _coloringAction;    /** Coloring action */
    SelectedPointsAction _selectedPointsAction; /** Action dealing with data selection*/
    
    
};
