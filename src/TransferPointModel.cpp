/** General headers */
#include <math.h>
#include <iostream>     
#include <algorithm> 
#include <vector>
#include <cmath>
/** Plugin headers */
#include <TransferPointModel.h>
#include <RendererSettingsAction.h>
#include <VolumeViewerPlugin.h>
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


TransferPointModel::TransferPointModel(TransferWidget *transferWidget) :
    _transferWidget(transferWidget),
    //_transferWidget(parent),
    _newPosition()
    
    

{
    
    setFlag(ItemIsMovable);
    setFlag(ItemSendsGeometryChanges);
    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);

}

TransferPointModel::~TransferPointModel()
{

}

//void TransferPointModel::setPosition(int x, int y) {
//    _newPosition.setX(x);
//    _newPosition.setY(y);
//}

void TransferPointModel::calculateForces() {
    if (!scene() || scene()->mouseGrabberItem() == this) {
        _newPosition = pos();
        return;
    }
    return;
}


QRectF TransferPointModel::boundingRect() const
{
    qreal adjust = 2;
    return QRectF(-10 - adjust, -10 - adjust, 23 + adjust, 23 + adjust);
}

QPainterPath TransferPointModel::shape() const
{
    QPainterPath path;
    path.addEllipse(-10, -10, 20, 20);
    return path;
}

void TransferPointModel::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget*)
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

QVariant TransferPointModel::itemChange(GraphicsItemChange change, const QVariant& value)
{
    switch (change) {
    case ItemPositionHasChanged:
        //for (Edge* edge : qAsConst(edgeList))
        //    edge->adjust();
        //_transferWidget->itemMoved();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}

void TransferPointModel::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    update();
    QGraphicsItem::mousePressEvent(event);
}

void TransferPointModel::mouseReleaseEvent(QGraphicsSceneMouseEvent* event)
{
    update();
    QGraphicsItem::mouseReleaseEvent(event);
}