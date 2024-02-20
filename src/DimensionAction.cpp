#include "DimensionAction.h"
#include "RendererSettingsAction.h"
#include "VolumeViewerPlugin.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>

using namespace mv::gui;

DimensionAction::DimensionAction(RendererSettingsAction& rendererSettingsAction, const QString& title) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction), title),
    _rendererSettingsAction(rendererSettingsAction),

    // Action to change the current dimension
    _dimensionAction(this, "Data dimension")
{
    setText("Dimension parameters");
    
    addAction(&_dimensionAction);
}
