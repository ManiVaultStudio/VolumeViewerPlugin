#ifndef TRANSFERVIEW_H
#define TRANSFERVIEW_H
#include <QtGui>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QPoint>
#include <QGraphicsSceneMouseEvent>
#include <Transfer/TransferWidget.h>
class TransferView : public QGraphicsScene {
    Q_OBJECT
public:
    TransferView(TransferWidget* parent = nullptr);
protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event);

private:
    QCursor _cursor;
    TransferWidget* _parent;
signals:
    void pressed(QGraphicsSceneMouseEvent* p);
};
#endif // TRANSFERVIEW_H