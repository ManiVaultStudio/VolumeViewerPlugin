#ifndef TransferWidget_H
#define TransferWidget_H

/** QT headers*/
#include <QWidget>
#include <QResizeEvent>
#include <QGraphicsView>
/**  HDPS headers*/
#include <Dataset.h>
#include <PointData.h>
#include <Transfer/Node.h>
//#include <Transfer/TransferWidget2.h>



//class VolumeViewerPlugin;
using namespace hdps;
class TransferWidget2;
class Node;
using namespace hdps::gui;

class TransferWidget : public QGraphicsView
{
    Q_OBJECT

public:
    TransferWidget(TransferWidget2* parent = nullptr);

    bool findNode(QPointF position, bool leftButton);
    void addNode(QPointF position);
    void removeNode(QPointF position);
    void redrawEdges();
    void setCurrentNode(QPointF newPosition);

    void itemMoved(QPointF position);
    std::vector<float> getCurrentNodeInfo();
    std::vector<std::pair<float, int>> sortNodes();

public slots:
    //void shuffle();
    //void zoomIn();
    //void zoomOut();

protected:
    //void keyPressEvent(QKeyEvent* event) override;
    //void timerEvent(QTimerEvent* event) override;
#if QT_CONFIG(wheelevent)
    //void wheelEvent(QWheelEvent* event) override;
#endif
    void drawBackground(QPainter* painter, const QRectF& rect) override;
    //void mousePressEvent(QMouseEvent* event);
    //void mousePressEvent(QObject* target, QEvent* event);
    //void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

    //void scaleView(qreal scaleFactor);

private:
    std::vector<int> _histogram;
    bool _dataLoaded;
    QCursor _cursor;
    QGraphicsScene* _scene;
    std::vector<std::pair<float, float>> _nodePositions;
    QVector<Node*> _nodeList;
    QVector<QGraphicsItem*> _edgeList;
    QPointF _currentNode;
    int _windowWidth;
    int _windowHeight;
    
    //QVector<Node *> test;
    
    //int timerId = 0;
    //Node centerNode;
};
#endif // TransferWidget_H