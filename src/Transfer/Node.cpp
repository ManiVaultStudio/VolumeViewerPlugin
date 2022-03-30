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

QRectF Node::boundingRect() const
{
    qreal adjust = 2;
    return QRectF(-10 - adjust, -10 - adjust, 30 + adjust, 30 + adjust);
}

QPainterPath Node::shape() const
{
    QPainterPath path;
    path.addEllipse(-10, -10, 28, 28);
    return path;
}

void Node::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
{
    auto nodePositions=_transferWidget->getNodeList();
    auto colorList = _transferWidget->getColorList();
    QColor color;
    for (int i = 0; i < nodePositions.size(); i++)
    {
        if (this->scenePos().x() == nodePositions[i].first && this->scenePos().y() == nodePositions[i].second) {
            color = colorList[i];
            
        }
    }

    

    painter->setPen(Qt::NoPen);
    
    if (this->scenePos() == _transferWidget->getCurrentNodePosition()) {
        painter->setBrush(Qt::darkGray);
        painter->drawEllipse(-7, -7, 28, 28);

        QRadialGradient gradient(-3, -3, 10);
        if (option->state & QStyle::State_Sunken) {
            gradient.setCenter(3, 3);
            gradient.setFocalPoint(3, 3);
            gradient.setColorAt(1, color.lighter(120));
            gradient.setColorAt(0, color.lighter(120));
        }
        else {
            gradient.setColorAt(0, color);
            gradient.setColorAt(1, color);
        }
        painter->setBrush(gradient);
        painter->setPen(QPen(Qt::black, 0));
        painter->drawEllipse(-13, -13, 28, 28);
    }
    else {
        painter->setBrush(Qt::darkGray);
        painter->drawEllipse(-7, -7, 20, 20);

        QRadialGradient gradient(-3, -3, 10);
        if (option->state & QStyle::State_Sunken) {
            gradient.setCenter(3, 3);
            gradient.setFocalPoint(3, 3);
            gradient.setColorAt(1, color.lighter(120));
            gradient.setColorAt(0, color.lighter(120));
        }
        else {
            gradient.setColorAt(0, color);
            gradient.setColorAt(1, color);
        }
        painter->setBrush(gradient);
        painter->setPen(QPen(Qt::black, 0));
        painter->drawEllipse(-10, -10, 20, 20);
    }
    
    
   
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
    else if (y()  > (scene()->height()-40))
    {
        setPos(x(), scene()->height()-40 );
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