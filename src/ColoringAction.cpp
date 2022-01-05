#include "ColoringAction.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>
#include "RendererSettingsAction.h"
#include <QGraphicsScene>

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
    setMayReset(false);
    QGraphicsScene* transferWindow = new QGraphicsScene(this);
    transferWindow->setItemIndexMethod(QGraphicsScene::NoIndex);
    transferWindow->setSceneRect(-50, -50, 100, 100);

}