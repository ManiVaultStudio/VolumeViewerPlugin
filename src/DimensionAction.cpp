#include "DimensionAction.h"
#include "RendererSettingsAction.h"
#include "ViewerWidget.h"
#include "VolumeViewerPlugin.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>

using namespace hdps;

DimensionAction::DimensionAction(RendererSettingsAction& rendererSettingsAction, ViewerWidget* viewerWidget) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction)),
    _rendererSettingsAction(rendererSettingsAction),
    
    _chosenDimensionAction(this, "Current Dimension", 0.0f, 100.0f, 0.0f, 0.0f, 0),
    
    _viewerWidget(nullptr)
{
    setText("Dimension parameters");
    setMayReset(false);
    
    _viewerWidget = viewerWidget;

    connect(&_chosenDimensionAction, &DecimalAction::valueChanged, this, [this](const float& value) {

    });
    
}