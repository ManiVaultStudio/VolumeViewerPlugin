#include "SelectedPointsAction.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>
#include "RendererSettingsAction.h"
#include <QGraphicsScene>
#include <PositionAction.h>
#include <ThresholdAction.h>

using namespace hdps;

SelectedPointsAction::SelectedPointsAction(RendererSettingsAction& rendererSettingsAction, const QString& title) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction), title),
    _rendererSettingsAction(rendererSettingsAction),
    
    // color interpolation options with default nearest neighbor interpolations
    _backgroundShowAction(this, "Surrounding data", {"Show background","Hide background"},  "Show background"),
    _pointCloudShowAction(this, "Surrounding data", {"Full render","Point cloud"},  "Point cloud"),
    _backgroundAlphaAction(this, "Background alpha", 0.0f, 1.0f, 0.02f,  3),
    _selectionAlphaAction(this, "Selection alpha", {"Opaque","Use transfer function"},  "Opaque"),
    _selectPointAction(this, "Select point"),
    _positionAction(*this, title),
    
    _thresholdAction(*this, title)
    
{
    
    setText("Selected data options");
    addAction(&_backgroundShowAction);
    addAction(&_pointCloudShowAction);
    addAction(&_backgroundAlphaAction);
    addAction(&_selectionAlphaAction);
    addAction(&_selectPointAction);
    addAction(&_positionAction);
    addAction(&_thresholdAction);
}