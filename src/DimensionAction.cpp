#include "DimensionAction.h"
#include "RendererSettingsAction.h"
#include "ViewerWidget.h"
#include "VolumeViewerPlugin.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>

using namespace mv::gui;

DimensionAction::DimensionAction(RendererSettingsAction& rendererSettingsAction, ViewerWidget* viewerWidget, const QString& title) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction), title),
    _rendererSettingsAction(rendererSettingsAction),

    _viewerWidget(nullptr),
    // Action to change the current dimension
    _dimensionAction(this, "Data dimension")
{
    setText("Dimension parameters");
    
    
    addAction(&_dimensionAction);
    _viewerWidget = viewerWidget;

    
}

