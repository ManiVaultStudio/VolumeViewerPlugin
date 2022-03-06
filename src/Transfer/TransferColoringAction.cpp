#include "Transfer/TransferColoringAction.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>
#include "Transfer/TransferSettingsAction.h"
#include <QGraphicsScene>

using namespace hdps;

TransferColoringAction::TransferColoringAction(TransferSettingsAction& transferSettingsAction) :
    GroupAction(reinterpret_cast<QObject*>(&transferSettingsAction)),
    _transferSettingsAction(transferSettingsAction),
    
    
    
    // colormap options 
    _colorAction(this, "Transfer Function")
    

{
    setText("Coloring parameters");
    setMayReset(false);
    QGraphicsScene* transferWindow = new QGraphicsScene(this);
    transferWindow->setItemIndexMethod(QGraphicsScene::NoIndex);
    transferWindow->setSceneRect(-50, -50, 100, 100);

}