#include <Transfer/Node.h>
/** General headers */
#include <math.h>
#include <iostream>     
#include <algorithm> 
#include <vector>
#include <cmath>
/** Plugin headers */

#include <RendererSettingsAction.h>
//#include <VolumeViewerPlugin.h>
/** HDPS headers */
#include <Dataset.h>
#include <PointData.h>
/** QT headers */
#include <qwidget.h>
#include <qdialog.h>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <QGraphicsRectItem>
#include <QPainter.h>


using namespace hdps;
using namespace hdps::gui;


Node::Node(TransferWidget *transferWidget) :
    _transferWidget(transferWidget),
    //_transferWidget(parent),
    _newPosition(),
    _cursor()
    
    

{
    
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);

}

Node::~Node()
{

}

void Node::setPosition(int x, int y) {
    _newPosition.setX(x);
    _newPosition.setY(y);
}

void Node::calculateForces() {
    if (!scene() || scene()->mouseGrabberItem() == this) {
        _newPosition = pos();
        return;
    }
    return;
}


QRectF Node::boundingRect() const
{
    qreal adjust = 2;
    return QRectF(-10 - adjust, -10 - adjust, 23 + adjust, 23 + adjust);
}

QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(-10, -10, 20, 20);
    return path;
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::darkGray);
    painter->drawEllipse(-7, -7, 20, 20);

    QRadialGradient gradient(-3, -3, 10);
    if (option->state & QStyle::State_Sunken) {
        gradient.setCenter(3, 3);
        gradient.setFocalPoint(3, 3);
        gradient.setColorAt(1, QColor(Qt::yellow).lighter(120));
        gradient.setColorAt(0, QColor(Qt::darkYellow).lighter(120));
    }
    else {
        gradient.setColorAt(0, Qt::yellow);
        gradient.setColorAt(1, Qt::darkYellow);
    }
    painter->setBrush(gradient);

    painter->setPen(QPen(Qt::black, 0));
    painter->drawEllipse(-10, -10, 20, 20);
}

QVariant Node::itemChange(GraphicsItemChange change, const QVariant& value)
{
    switch (change) {
    case ItemPositionHasChanged:
        for (Edge* edge : qAsConst(edgeList))
            edge->adjust();
            
            
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}
void Node::pressed(QGraphicsSceneMouseEvent* event) {
    this->mousePressEvent(event);
}

void Node::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    
    update();
    _transferWidget->setCurrentNode(this->scenePos());
    QGraphicsItem::mousePressEvent(event);
}

void Node::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseMoveEvent(event);
    std::vector<float> bounds =_transferWidget->getCurrentNodeInfo();

    if (x() < bounds[0]+1)
    {
        setPos(bounds[0] + 1, y());
    }
    else if (x()  > bounds[1] - 1)
    {
        setPos(bounds[1] - 1, y());
    }

    if (y() < 0)
    {
        setPos(x(), 0);
    }
    else if (y()  > scene()->height())
    {
        setPos(x(), scene()->height() );
    }
    _transferWidget->itemMoved(this->scenePos());
}



void Node::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
   
    update();
    QGraphicsItem::mouseReleaseEvent(event);
    _transferWidget->redrawEdges();
}

void Node::addEdge(Edge* edge)
{
    edgeList << edge;
    edge->adjust();
}

QVector<Edge*> Node::edges() const
{
    return edgeList;
}