#include "ColoringAction.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>
#include "RendererSettingsAction.h"
#include <QGraphicsScene>

using namespace mv::gui;

ColoringAction::ColoringAction(RendererSettingsAction& rendererSettingsAction, const QString& title) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction), title),
    _rendererSettingsAction(rendererSettingsAction),
    
    
    // color interpolation options with default Nearest neighbor interpolations
    _colorInterpolationAction(this, "Color Interpolation", {"Nearest Neighbor","Linear"}, "Nearest Neighbor"),
    
    // colormap options 
    _colorMapAction(this, "Transfer Function"),
    // Shading enable option.
    _shadingEnableAction(this, "shading enabled"),
    _disableSelectionAction(this,"disable selection"),
    // Shading parameters options.
    _ambientConstantAction(this, "Ambient constant", 0.0f, 1.0f, 0.9f, 4),
    
    _diffuseConstantAction(this, "Diffuse constant", 0.0f, 1.0f,  0.2f, 4),
    _specularConstantAction(this, "Specular constant", 0.0f, 1.0f,  0.1f, 4),
    _unloadColorMap(this, "unload Color Map"),
    _unloadOpacityData(this, "unload Opacity Data")

{
    
    //setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
    setText("Coloring parameters");
    addAction(&_colorInterpolationAction);
    addAction(&_colorMapAction);
    addAction(&_shadingEnableAction);
    addAction(&_disableSelectionAction);
    addAction(&_ambientConstantAction);
    addAction(&_diffuseConstantAction);
    addAction(&_specularConstantAction);
    addAction(&_unloadColorMap);
    addAction(&_unloadOpacityData);

    _ambientConstantAction.setDisabled(true);
    _diffuseConstantAction.setDisabled(true);
    _specularConstantAction.setDisabled(true);
}