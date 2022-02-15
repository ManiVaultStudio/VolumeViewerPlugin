#ifndef Node_H
#define Node_H

/** QT headers*/
#include <QWidget>
#include <QResizeEvent>
#include <QGraphicsItem>

/**  HDPS headers*/
#include <Dataset.h>
#include <PointData.h>
//#include <Transfer/TransferWidget.h>
#include <Transfer/Edge.h>



//class VolumeViewerPlugin;
using namespace hdps;
//class TransferWidget;

class Node : public QGraphicsItem
{
public:
    Node(TransferWidget *transferWidget);
    ~Node();
    void addEdge(Edge *edge);
    QVector<Edge *> edges() const;

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    void calculateForces();
    //bool advancePosition();
    void setPosition(int x, int y);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem *option, QWidget *widget) override;
    void pressed(QGraphicsSceneMouseEvent* event);


protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;

    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;

private:
    QVector<Edge *> edgeList;
    QPointF _newPosition;
    QCursor _cursor;
    TransferWidget *_transferWidget;

    
};
#endif // Node_H