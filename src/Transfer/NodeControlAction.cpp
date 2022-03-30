#include "NodeControlAction.h"
#include "TransferFunctionControlAction.h"
#include "ViewerWidget.h"
#include "VolumeViewerPlugin.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>
#include <qbuttongroup.h>
#include <qpushbutton.h>

using namespace hdps;

NodeControlAction::NodeControlAction(TransferFunctionControlAction& transferFunctionControlAction, TransferWidget* transferWidget) :
    GroupAction(reinterpret_cast<QObject*>(&transferFunctionControlAction)),
    _transferFunctionControlAction(transferFunctionControlAction),
    _intensityAction(this, "Intensity control", 0.0f, 100.0f, 0.0f, 0.0f, 0),
    _valueAction(this, "Value control", 0.0f, 100.0f, 0.0f, 0.0f, 0),
    _nodeColorAction(this,"Node color selection", Qt::yellow)

{
    setText("Node Control");
    setMayReset(false);
    
}

void NodeControlAction::changeNodePosition(float xPosition, float yPosition) {
    _valueAction.setValue(xPosition);
    _intensityAction.setValue(yPosition);
}

void NodeControlAction::changeNodeColor(QColor color) {
    _nodeColorAction.setColor(color);
}