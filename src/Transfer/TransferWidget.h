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

    /**Function used to find a node at a specific location and determine what to do based on a mouseclick.*/
    bool findNode(QPointF position, std::string mouseButton); 

    /**Function used to add a node at a given position. */
    void addNode(QPointF position);

    /** Function used to remove a node at a given position. */
    void removeNode(QPointF position);

    /** Function used to redraw the lines between the nodes. */
    void redrawEdges();

    /** Set the current node. */
    void setCurrentNode(QPointF newPosition);
    
    /** Function to alter the saved node data when one has been moved. */
    void itemMoved(QPointF position);
     
    /** Function used to get the x-values of surrounding nodes. */
    std::vector<float> getCurrentNodeInfo();

    /** Function used to sort the nodes based on x-values. */
    std::vector<std::pair<float, int>> sortNodes();

    /** Function used to draw the axes in the transferfunction. */
    void drawBackground(QPainter* painter, const QRectF& rect);

    /** Function used to recreate the colormap. */
    void createColorMap();

    /** Get a list of positions of the nodes. */
    std::vector<std::pair<float, float>> getNodeList() {
        return _nodePositions;
    }

    /** Get a list of the color of the nodes. */
    QVector<QColor> getColorList() {
        return _nodeColorList;
    }

    /** Get the color map. */
    QImage getColorMap() {
        return _colorMap;
    }


protected:
    

private:

    TransferWidget2* _parent; 
    bool _dataLoaded;
    QCursor _cursor;
    QGraphicsScene* _scene;
    std::vector<std::pair<float, float>> _nodePositions;
    QVector<Node*> _nodeList;
    QVector<QGraphicsItem*> _edgeList;
    QPointF _currentNodePosition;
    Node* _currentNode;
    int _windowWidth;
    int _windowHeight;
    QVector<QColor> _nodeColorList;
    QImage _colorMap;

signals:

    void colorMapChanged(const QImage& colorMap); // Signal when the colormap has been altered.
    
};
#endif // TransferWidget_H