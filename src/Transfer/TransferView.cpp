#include "Transfer/TransferView.h"
#include "Transfer/TransferWidget.h"
//#include <QMouseEvent>
//#include "Transfer/Node.h"

TransferView::TransferView(TransferWidget* parent)
    :QGraphicsScene(parent),
    _cursor(),
    _parent(parent)
{
    
}

void TransferView::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    QPointF position = event->scenePos();
    
    //Node::mousePressEvent(event, position);
    if (event->buttons() & Qt::LeftButton) {
        _parent->findNode(position, true);
    }
    else if (event->buttons() & Qt::RightButton) {
        _parent->findNode(position, false);
    }

}

//QPointF TransferView::findPosition() {
//
//}