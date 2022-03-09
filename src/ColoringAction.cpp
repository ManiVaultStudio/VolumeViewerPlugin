#include "ColoringAction.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>
#include "RendererSettingsAction.h"

using namespace hdps;

ColoringAction::ColoringAction(RendererSettingsAction& rendererSettingsAction) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction)),
    _rendererSettingsAction(rendererSettingsAction),
    
    // color interpolation options with default Nearest neighbor interpolations
    _colorInterpolationAction(this, "Color Interpolation", {"Nearest Neigbor","Linear", "Cubic"}, "Nearest Neighbor", "Nearest Neighbor"),
    // colormap options 

    _colorMapAction(this, "Transfer Function")
{
    setText("Coloring parameters");
}