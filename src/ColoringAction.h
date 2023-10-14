#pragma once

#include "actions/GroupAction.h"
#include "actions/ColorMap1DAction.h"
#include "actions/OptionAction.h"


using namespace mv::gui;

class RendererSettingsAction;

/**
 * Coloring action class
 *
 * Action class for coloring parameters
 *
 * @author Thomas Kroes and Mitchell M. de Boer
 */
class ColoringAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Constructor
     * @param rendererSettingsAction Reference to renderer settings action
     */
    Q_INVOKABLE ColoringAction(RendererSettingsAction& rendererSettingsAction, const QString& title);
   
public: /** Action getters */

    OptionAction& getInterpolationAction() { return  _colorInterpolationAction; }
    ColorMap1DAction& getColorMapAction() { return  _colorMapAction; }
    ToggleAction& getShadingAction() { return  _shadingEnableAction; }
    
    DecimalAction& getAmbientAction() { return _ambientConstantAction; }
    DecimalAction& getDiffuseAction() { return _diffuseConstantAction; }
    DecimalAction& getSpecularAction() { return _specularConstantAction; }
    TriggerAction& getUnloadColorMap() { return  _unloadColorMap; }
    TriggerAction& getUnloadOpacityData() { return  _unloadOpacityData; }
    ToggleAction& getdisableSelectionAction() { return _disableSelectionAction; }


protected:
    RendererSettingsAction&     _rendererSettingsAction;        /** Reference to renderer settings action */
    OptionAction                _colorInterpolationAction;      /** Option menu for selecting interpolation mode*/
    ColorMap1DAction              _colorMapAction;                 /** Color map Action */
    ToggleAction                _shadingEnableAction;      /** Option menu for selecting interpolation mode*/
    ToggleAction                _disableSelectionAction;
    TriggerAction _unloadColorMap;
    TriggerAction _unloadOpacityData;
    
    
    DecimalAction                _ambientConstantAction;               /** Input box for ambient color constant.*/
    DecimalAction                _diffuseConstantAction;               /** Input box for diffuse color constant.*/
    DecimalAction                _specularConstantAction;               /** Input box for specular color constant.*/

    friend class ScatterplotPlugin;
    friend class mv::AbstractActionsManager;
};

Q_DECLARE_METATYPE(ColoringAction)
