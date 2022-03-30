#include "Transfer/TransferView.h"
#include "Transfer/TransferWidget.h"

TransferView::TransferView(TransferWidget* parent)
    :QGraphicsScene(parent),
    _cursor(),
    _parent(parent)
{
    
}

void TransferView::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QPointF position = event->scenePos();
    
    QGraphicsScene::mousePressEvent(event);
    if (event->buttons() & Qt::LeftButton) {
        _parent->findNode(position, "Left");
       
    }
    else if (event->buttons() & Qt::RightButton) {
        _parent->findNode(position, "Right");
    }
    else if (event->buttons() & Qt::MiddleButton) {
        _parent->findNode(position, "Middle");
    }
}

