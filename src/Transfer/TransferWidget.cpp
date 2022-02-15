#include <Transfer/TransferWidget.h>
/** General headers */
#include <math.h>
#include <iostream>     
#include <algorithm> 
#include <vector>
#include <cmath>
/** Plugin headers */


#include <RendererSettingsAction.h>
#include <VolumeViewerPlugin.h>
#include <Transfer/TransferView.h>
/** HDPS headers */
#include <Dataset.h>
#include <PointData.h>
//#include <Transfer/Node.h>
/** QT headers */
#include <qwidget.h>
#include <qdialog.h>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <QGraphicsRectItem>
#include <QPainter.h>

#include <Windows.h>
#include <thread>

using namespace hdps;
using namespace hdps::gui;


TransferWidget::TransferWidget(TransferWidget2* parent)
    : QGraphicsView(parent),
    _histogram(std::vector<int>(100,0)),
    _dataLoaded(false),
    _cursor(),
    _scene(nullptr),
    _nodePositions(),
    _nodeList(QVector<Node*> (2)),
    _edgeList(QVector<QGraphicsItem*> (1)),
    _currentNode(),
    _windowHeight(150),
    _windowWidth(325)

{
    
    _scene = new TransferView(this);
    _scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    auto Size = parent->size();
    _histogram = parent->getHistogram();
    _dataLoaded = parent->getDataLoaded();
    qDebug() << "height= " << Size.height();
    qDebug() << "width= " << Size.width();
    _scene->setSceneRect(0, 0 , _windowWidth, _windowHeight);
    setScene(_scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    scale(qreal(0.8), qreal(0.8));
    //setMinimumSize(400, 400);
    setWindowTitle(tr("Elastic Nodes"));
    

    Node* node1 = new Node(this);
    Node* node2 = new Node(this);
    _nodeList[0] = node1;
    _nodeList[1] = node2;
    
    _scene->addItem(_nodeList[0]);
    _scene->addItem(_nodeList[1]);
    
    Edge* edge1 = new Edge(node1, node2);
    _edgeList[0] = edge1;
    _scene->addItem(_edgeList[0]);

    node1->setPos(0, _windowHeight);
    node2->setPos(_windowWidth, 0);
    _nodePositions = { {0,_windowHeight},{_windowWidth,0} };
    //_nodePositions[1] = _windowWidth;
}

std::vector<float> TransferWidget::getCurrentNodeInfo() {
    std::vector<std::pair<float, int>> sortedIndices = sortNodes();

    float previousNode;
    float nextNode;

    for (int i = 0; i < sortedIndices.size(); i++)
    {
        if (sortedIndices[i].first == _currentNode.x()) {
            
            if (i == 0 ) {
                previousNode = -1;
                nextNode = 1;
                break;
            } else if (i == sortedIndices.size()-1)
            {
                previousNode = 324;
                nextNode = 326;
                break;
            } else {
                previousNode = sortedIndices[i-1].first;
                nextNode = sortedIndices[i+1].first;
                
                break;
            }
        }
    }
    
    std::vector<float> info = { previousNode , nextNode };
    return info;
}

bool TransferWidget::findNode(QPointF position, bool leftButton) {
   
    bool found = false;
   
    for (int i = 0; i < _nodePositions.size(); i++) {
        if (position.x() > _nodePositions[i].first - 10 && position.x() < _nodePositions[i].first + 10) {
            found = true;
            if (leftButton) {
                break;
            }
            else {
                if (i == 0 || i == 1) {
                    QMessageBox msgBox;
                    msgBox.setText("This node cannot be removed");
                    msgBox.exec();
                    break;
                }
                removeNode(position);
            }
            
        }
    }
    if (!found) {
        if (leftButton) {
            addNode(position);
        }     
    }

    return found;
}
void TransferWidget::removeNode(QPointF position) {
    for (int i = 0; i < _nodePositions.size(); i++)
    {
        if (position.x() > _nodePositions[i].first - 10 && position.x() < _nodePositions[i].first + 10)
        {
            if (position.y() > _nodePositions[i].second - 10 && position.y() < _nodePositions[i].second + 10) {
                _scene->removeItem(_nodeList[i]);
                _nodePositions.erase(_nodePositions.begin() + i);
                _nodeList.erase(_nodeList.begin() + i);
                redrawEdges();
                break;
            }
        }
    }
}
void TransferWidget::addNode(QPointF position) {

    Node* newNode = new Node(this);
    _scene->addItem(newNode);
    newNode->setPos(position);
    _nodeList.push_back(newNode);
    
    _nodePositions.push_back(std::make_pair(position.x(), position.y()));
    redrawEdges();
   
}

std::vector<std::pair<float, int>> TransferWidget::sortNodes() {
    std::vector<std::pair<float, int>> sortedIndices;


    for (int i = 0; i < _nodePositions.size(); i++) {
        //sortedIndices[i] = i;
        sortedIndices.push_back(std::make_pair(_nodePositions[i].first, i));
    }

    sort(sortedIndices.begin(), sortedIndices.end());
    return sortedIndices;
}

void TransferWidget::redrawEdges() {
    for (int i = 0; i < _edgeList.size(); i++) {
        _scene->removeItem(_edgeList[i]);
    }
    _edgeList.clear();

    
    std::vector<std::pair<float, int>> sortedIndices = sortNodes();
    
    for (int i = 0; i < sortedIndices.size() - 1; i++) {
        Edge* newEdge = new Edge(_nodeList[sortedIndices[i].second], _nodeList[sortedIndices[i + 1].second]);
        _edgeList.push_back(newEdge);
        _scene->addItem(_edgeList[i]);
        
    }

}

void TransferWidget::setCurrentNode(QPointF newPosition) {
    _currentNode = newPosition;
   
}

void TransferWidget::itemMoved(QPointF newPosition) {
    _nodePositions;
    //qDebug() << "itemMoved";
    for (int i = 0; i < _nodePositions.size(); i++) {
        if (_nodePositions[i].first == _currentNode.x())
        {
            //qDebug() << "itemMoved";
            _nodePositions[i].first = newPosition.x();
            _nodePositions[i].second = newPosition.y();
            _currentNode = newPosition;
            redrawEdges();
        }
    }

}

//void TransferWidget::mousePressEvent(QMouseEvent *event) {
//    //qDebug() << mapFromGlobal(_cursor.pos());
//    
//}

void TransferWidget::drawBackground(QPainter* painter, const QRectF& rect)
{
    //Q_UNUSED(rect);
    
    //// Shadow
    //QRectF sceneRect = this->sceneRect();
    //QRectF rightShadow(sceneRect.right(), sceneRect.top() + 5, 5, sceneRect.height());
    //QRectF bottomShadow(sceneRect.left() + 5, sceneRect.bottom(), sceneRect.width(), 5);
    //if (rightShadow.intersects(rect) || rightShadow.contains(rect))
    //    painter->fillRect(rightShadow, Qt::darkGray);
    //if (bottomShadow.intersects(rect) || bottomShadow.contains(rect))
    //    painter->fillRect(bottomShadow, Qt::darkGray);

    //// Fill
    //QLinearGradient gradient(sceneRect.topLeft(), sceneRect.bottomRight());
    //gradient.setColorAt(0, Qt::white);
    //gradient.setColorAt(1, Qt::lightGray);
    //painter->fillRect(rect.intersected(sceneRect), gradient);
    //painter->setBrush(Qt::NoBrush);
    //painter->drawRect(sceneRect);

    //// Text
    //QRectF textRect(sceneRect.left() + 4, sceneRect.top() + 4,
    //    sceneRect.width() - 4, sceneRect.height() - 4);
    //QString message(tr("Click and drag the nodes around, and zoom with the mouse "
    //    "wheel or the '+' and '-' keys"));

    //QFont font = painter->font();
    //font.setBold(true);
    //font.setPointSize(14);
    //painter->setFont(font);
    //painter->setPen(Qt::lightGray);
    //painter->drawText(textRect.translated(2, 2), message);
    //painter->setPen(Qt::black);
    //painter->drawText(textRect, message);

    float windowWidth = this->width();
    int windowHeight = this->height();
    //QPainter painter(this);
    painter->setPen(Qt::black);
    //painter->drawEllipse(QPoint(0, windowHeight), 7, 7);
    //painter->drawEllipse(QPoint(windowWidth, 0), 7, 7);
    //TransferPointModel *point = new TransferPointModel(this);

    std::cout << "im here" << std::endl;
    if (_dataLoaded) {
        //std::cout << _transferView->size() << std::endl;
        float maxBin = 0;
        float currentBin = 0;
        for (int i = 0; i < 100; i++) {
            currentBin = _histogram[i];
            if (currentBin > maxBin) {
                maxBin = currentBin;
            }
        }


        float binWidth = windowWidth / 100;

        int binIncrement = maxBin / windowHeight;
        float height = 0;


        QBrush brush(Qt::lightGray, Qt::SolidPattern);
        painter->setPen(Qt::lightGray);


        //painter.drawRect(QRectF(QPoint(0, windowHeight), QSize(binWidth, -windowHeight*height)));

        for (int i = 0; i < 100; i++) {
            currentBin = _histogram[i];
            height = currentBin / maxBin;
            QRectF currentRect = QRectF(QPoint(i * binWidth, windowHeight), QSize(binWidth, -windowHeight * height));
            painter->drawRect(currentRect);
            painter->fillRect(currentRect, brush);

        }
    }
}

