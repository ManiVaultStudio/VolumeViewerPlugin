#pragma once

#include "actions/GroupAction.h"
#include "actions/ToggleAction.h"
#include "actions/DecimalAction.h"

using namespace mv::gui;

class RendererSettingsAction;
class VolumeViewerPlugin;

/**
 * Slicing action class
 *
 * Action class for slicing parameters
 *
 * @author Thomas Kroes and Mitchell M. de Boer
 */
class SlicingAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param rendererSettingsAction Reference to renderer settings action
     */
    Q_INVOKABLE SlicingAction(RendererSettingsAction& rendererSettingsAction, const QString& title);

public: /** Action getters */

    ToggleAction& getXAxisEnabledAction() { return _xAxisEnabledAction; }
    DecimalAction& getXAxisPositionAction() { return _xAxisPositionAction; }
    ToggleAction& getYAxisEnabledAction() { return _yAxisEnabledAction; }
    DecimalAction& getYAxisPositionAction() { return _yAxisPositionAction; } 
    ToggleAction& getZAxisEnabledAction() { return _zAxisEnabledAction; }
    DecimalAction& getZAxisPositionAction() { return _zAxisPositionAction; }
    bool& getXToggled() { return _xToggled; }
    bool& getYToggled() { return _yToggled; }
    bool& getZToggled() { return _zToggled; }
    
protected:
    RendererSettingsAction&     _rendererSettingsAction;        /** Reference to renderer settings action */
    ToggleAction                _xAxisEnabledAction;            /** X-axis enabled action */
    DecimalAction               _xAxisPositionAction;           /** X-axis position action */
    ToggleAction                _yAxisEnabledAction;            /** Y-axis enabled action */
    DecimalAction               _yAxisPositionAction;           /** Y-axis position action */
    ToggleAction                _zAxisEnabledAction;            /** Z-axis enabled action */
    DecimalAction               _zAxisPositionAction;           /** Z-axis position action */
    bool                        _xToggled;
    bool                        _yToggled;
    bool                        _zToggled;
};

Q_DECLARE_METATYPE(SlicingAction)
