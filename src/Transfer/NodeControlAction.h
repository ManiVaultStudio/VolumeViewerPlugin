#pragma once

#include "actions/GroupAction.h"
#include "actions/ToggleAction.h"
#include "actions/DecimalAction.h"
#include "actions/ColorAction.h"
#include <Transfer/TransferWidget.h>

using namespace hdps::gui;

class TransferFunctionControlAction;
class VolumeViewerPlugin;
class ViewerWidget;

/**
 * Slicing action class
 *
 * Action class for slicing parameters
 *
 * @author Thomas Kroes and Mitchell M. de Boer
 */
class NodeControlAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param rendererSettingsAction Reference to renderer settings action
     */
    NodeControlAction(TransferFunctionControlAction& transferFunctionControlAction, TransferWidget* transferWidget);

    void changeNodePosition(float xPosition, float yPosition);

    void changeNodeColor(QColor color);

public: /** Action getters */

    
    DecimalAction& getIntensityAction() { return _intensityAction; }
    DecimalAction& getValueAction() { return _valueAction; }
    ColorAction& getColorPickerAction() { return _nodeColorAction; }
    
    

    
protected:
    TransferFunctionControlAction&     _transferFunctionControlAction;        /** Reference to renderer settings action */
    ColorAction           _nodeColorAction;
    DecimalAction               _intensityAction;           /** X-axis position action */
    DecimalAction               _valueAction;           /** Y-axis position action */

};
