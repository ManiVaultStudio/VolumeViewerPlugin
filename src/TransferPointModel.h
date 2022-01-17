#ifndef TransferPointModel_H
#define TransferPointModel_H

/** QT headers*/
#include <QWidget>
#include <QResizeEvent>
#include <QGraphicsItem>

/**  HDPS headers*/
#include <Dataset.h>
#include <PointData.h>
#include <TransferWidget.h>



class VolumeViewerPlugin;
using namespace hdps;
class TransferWidget;

class TransferPointModel : public QGraphicsItem
{
public:
    explicit TransferPointModel(TransferWidget *transferWidget);
    ~TransferPointModel();
    //void addEdge(Edge* edge);
    //QVector<Edge*> edges() const;

    enum { Type = UserType + 1 };
    int type() const override { return Type; }

    void calculateForces();
    //bool advancePosition();
    //void setPosition(int x, int y);

    QRectF boundingRect() const override;
    QPainterPath shape() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    //QVector<Edge*> edgeList;
    QPointF _newPosition;
    TransferWidget *_transferWidget;
};
#endif // TransferPointModel_H