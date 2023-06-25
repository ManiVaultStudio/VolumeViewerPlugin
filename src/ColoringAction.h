#pragma once

#include "actions/GroupAction.h"
#include "actions/ColorMapAction.h"
#include "actions/OptionAction.h"


using namespace hdps::gui;

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
    ColoringAction(RendererSettingsAction& rendererSettingsAction);
   
public: /** Action getters */

    OptionAction& getInterpolationAction() { return  _colorInterpolationAction; }
    ColorMapAction& getColorMapAction() { return  _colorMapAction; }
    ToggleAction& getShadingAction() { return  _shadingEnableAction; }
    
    DecimalAction& getAmbientAction() { return _ambientConstantAction; }
    DecimalAction& getDiffuseAction() { return _diffuseConstantAction; }
    DecimalAction& getSpecularAction() { return _specularConstantAction; }
    TriggerAction& getUnloadColorMap() { return  _unloadColorMap; }
    TriggerAction& getUnloadOpacityData() { return  _unloadOpacityData; }


protected:
    RendererSettingsAction&     _rendererSettingsAction;        /** Reference to renderer settings action */
    OptionAction                _colorInterpolationAction;      /** Option menu for selecting interpolation mode*/
    ColorMapAction              _colorMapAction;                 /** Color map Action */
    ToggleAction                _shadingEnableAction;      /** Option menu for selecting interpolation mode*/
    TriggerAction _unloadColorMap;
    TriggerAction _unloadOpacityData;
    DecimalAction                _ambientConstantAction;               /** Input box for ambient color constant.*/
    DecimalAction                _diffuseConstantAction;               /** Input box for diffuse color constant.*/
    DecimalAction                _specularConstantAction;               /** Input box for specular color constant.*/
};