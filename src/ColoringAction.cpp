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
    // colormap options with default Blue yellow red
    _colorAction(this, "Color Map", { "BuYlRd","Gray to White", "Qualitative" ,"GnYlRd", "Spectral"}, "BuYlRd", "BuYlRd"), 
    // color interpolation options with default Nearest neighbor interpolations
    _colorInterpolationAction(this, "Color Interpolation", {"Nearest Neigbor","Linear", "Cubic"}, "Nearest Neighbor", "Nearest Neighbor"),
    _colorMapAction(this, "Transfer Function")
{
    setText("Coloring parameters");
    setMayReset(false);

}
