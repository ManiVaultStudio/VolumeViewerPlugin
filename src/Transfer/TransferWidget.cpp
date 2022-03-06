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
#include <actions/ColorAction.h>

/** QT headers */
#include <qwidget.h>
#include <qdialog.h>
#include <qgraphicsscene.h>
#include <qgraphicsview.h>
#include <QGraphicsRectItem>
#include <QPainter.h>
#include <QColorDialog>

#include <Windows.h>
#include <thread>

using namespace hdps;
using namespace hdps::gui;


TransferWidget::TransferWidget(TransferWidget2* parent)
    : QGraphicsView(parent),
    _parent(parent),
    _dataLoaded(false),
    _cursor(),
    _scene(nullptr),
    _nodePositions(),
    _nodeList(QVector<Node*> (2)),
    _edgeList(QVector<QGraphicsItem*> (1)),
    _nodeColorList(QVector<QColor>(2)),
    _currentNodePosition(),
    _currentNode(),
    _windowHeight(200),
    _windowWidth(400),
    _colorMap()

{
    // Create transferview graphicsscene.
    _scene = new TransferView(this);
    _scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    // Get boolian that shows if data has been loaded in !!!!! needs to be removed when extracted from volumeviewer.
    _dataLoaded = parent->getDataLoaded();
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    // Set the scene rectangle, negative values and increases of window height and width are used as padding to fascilitate axes.
    _scene->setSceneRect(-30, -20 , _windowWidth+50, _windowHeight+40);
    setScene(_scene);
    setCacheMode(CacheBackground);
    setViewportUpdateMode(BoundingRectViewportUpdate);
    setRenderHint(QPainter::Antialiasing);
    setTransformationAnchor(AnchorUnderMouse);
    scale(qreal(0.8), qreal(0.8));
    setWindowTitle(tr("Elastic Nodes"));
    
    // Initialize the border nodes and save them in the node vectors.
    Node* node1 = new Node(this);
    Node* node2 = new Node(this);
    _nodeList[0] = node1;
    _nodeList[1] = node2;
    _nodeColorList[0] = Qt::yellow;
    _nodeColorList[1] = Qt::yellow;
    _scene->addItem(_nodeList[0]);
    _scene->addItem(_nodeList[1]);    
    Edge* edge1 = new Edge(node1, node2);
    _edgeList[0] = edge1;
    _scene->addItem(_edgeList[0]);
    node1->setPos(0, _windowHeight);
    node2->setPos(_windowWidth, 0);
    _nodePositions = { {0,_windowHeight},{_windowWidth,0} };
}

// This function is used to gather all data created by this widget and cummulates this into a QImage with a height of 1 and a width of 256, this QImage is the created as a colormap.
void TransferWidget::createColorMap() {
    // Initialize the colormap.
    const auto noSteps = 256;
    QImage colorMap(noSteps, 1, QImage::Format::Format_ARGB32);

    // Sort the nodes in the x-value order.
    auto sortedNodeList = sortNodes();

    // Translate the windowsize to the size of the colormap.
    float stepsize = (float)noSteps /(float)_windowWidth;
    float alphaStepSize = (float)1/(float)_windowHeight;    
    int currentPosition = 0;
    int nextPosition = 0;

    // Walk over the nodeList in order to construct the colormap.
    for (int i = 0; i < _nodeList.size()-1; i++) {
        // Get the index of the current and the next node, orderd by x-value.
        int currentNodeIndex = sortedNodeList[i].second;
        int nextNodeIndex = sortedNodeList[i + 1].second;

        // Get the current and next node color information.
        auto currentColor = _nodeColorList[currentNodeIndex];
        auto nextColor = _nodeColorList[nextNodeIndex];
        auto currentAlpha = abs(_nodeList[currentNodeIndex]->y() - _windowHeight) * alphaStepSize;
        auto nextAlpha = abs(_nodeList[nextNodeIndex]->y() - _windowHeight) * alphaStepSize;
        currentPosition = sortedNodeList[i].first * stepsize;
        nextPosition = sortedNodeList[i + 1].first * stepsize;

        // Iterate over the pixelvalues inside the colormap to determine the values of the colors between the current and next nodes.
        for (int i = currentPosition; i < nextPosition; i++) {
            // Calculate linear interpolation between the 2 nodes of color and alpha of the current pixelvalue.
            double ratio = (double)(i-currentPosition) / (double)(nextPosition - currentPosition);
            double resultRed = currentColor.redF() + ratio * (nextColor.redF() - currentColor.redF());
            double resultGreen = currentColor.greenF() + ratio * (nextColor.greenF() - currentColor.greenF());
            double resultBlue = currentColor.blueF() + ratio * (nextColor.blueF() - currentColor.blueF());
            double alphaResult = currentAlpha + ratio * (nextAlpha - currentAlpha);

            // Create the found color and add to the colormap.
            QColor interpolatedColor;
            interpolatedColor.setBlueF(resultBlue);
            interpolatedColor.setRedF(resultRed);
            interpolatedColor.setGreenF(resultGreen);
            interpolatedColor.setAlphaF(alphaResult);
            colorMap.setPixelColor(i, 0, interpolatedColor);
        }
    }

    // Save the colormap.
    _colorMap = colorMap;
    
    // Emit a signal that the colormap has been changed.
    emit colorMapChanged(_colorMap);    
}

// This function outputs node position infromation within a row of nodes.
std::vector<float> TransferWidget::getCurrentNodeInfo() {
    // Sort the nodeList.
    std::vector<std::pair<float, int>> sortedIndices = sortNodes();
    float previousNode;
    float nextNode;

    // Iterate over the nodes to find the current node inside that list, and save the neighbouring nodes.
    for (int i = 0; i < sortedIndices.size(); i++)
    {
        if (sortedIndices[i].first == _currentNodePosition.x()) {       
            // Locks the first node on its current x-position.
            if (i == 0 ) {
                previousNode = -1;
                nextNode = 1;
                break;
            }
            // Locks the final node on its current x-position.
            else if (i == sortedIndices.size()-1)
            {
                previousNode = _windowWidth-1;
                nextNode = _windowWidth + 1;
                break;
            } else {
                previousNode = sortedIndices[i - 1].first;
                nextNode = sortedIndices[i + 1].first;
                break;
            }
        }
    }
    
    std::vector<float> info = { previousNode , nextNode };
    return info;
}

// This function finds a node that has been clicked on and determine what to do when the click action has taken place
bool TransferWidget::findNode(QPointF position, std::string mouseButton) {
    bool found = false;
   
    // Iterate over all the nodes to find if a node excists on the current x position.
    for (int i = 0; i < _nodePositions.size(); i++) {
        // If there is a node in the x-position that has been clicked on. There is a boundary build-in in order to account for the visual circle.
        if (position.x() > _nodePositions[i].first - 10 && position.x() < _nodePositions[i].first + 10) {
            found = true;
            _currentNode = _nodeList[i];
            _currentNodePosition = QPointF(_nodePositions[i].first, _nodePositions[i].second);
            // If the left mousebutton is used, only return true, this is handled in node.cpp.
            if (mouseButton == "Left") {
                return found;
            }
            // If middle mouse button is used and it is not one of the boundary nodes, remove the node. !!!!!!! This is aimed to be removed and changed to a garbage icon !!!!!!
            else if (mouseButton == "Middle") {
                if (i == 0 || i == 1) {
                    QMessageBox msgBox;
                    msgBox.setText("This node cannot be removed");
                    msgBox.exec();
                    break;
                }
                removeNode(position);
            }
            // If the right mousbutton is used a color for the selected node can be chosen.
            else if (mouseButton == "Right") {
                QColor color = QColorDialog::getColor(Qt::white, this);
                if (color.isValid()) {
                    _nodeColorList[i] = color;
                    _nodeList[i]->update();
                    createColorMap();
                }
            }    
        }
    }
    // If the node has not been found add a node to the transferfunction.
    if (!found) {
        if (mouseButton == "Left") {
            addNode(position);
        }     
    }
    return found;
}

// This funcion is used to remove a node on a given position.
void TransferWidget::removeNode(QPointF position) {
    // Iterate over the node list to find the to be removed node.
    for (int i = 0; i < _nodePositions.size(); i++) {
        if (position.x() > _nodePositions[i].first - 10 && position.x() < _nodePositions[i].first + 10) {
            if (position.y() > _nodePositions[i].second - 10 && position.y() < _nodePositions[i].second + 10) {
                // Remove the node and corresponding data.
                _scene->removeItem(_nodeList[i]);
                _nodePositions.erase(_nodePositions.begin() + i);
                _nodeList.erase(_nodeList.begin() + i);
                _nodeColorList.erase(_nodeColorList.begin() + i);
                redrawEdges();
                break;
            }
        } 
    }
}

// This function is used to add a node at a given position.
void TransferWidget::addNode(QPointF position) {
    // Create a new node at the stated position and create data for this node.
    Node* newNode = new Node(this);
    _scene->addItem(newNode);
    newNode->setPos(position);
    _nodeList.push_back(newNode);
    _nodeColorList.push_back(Qt::yellow);
    _nodePositions.push_back(std::make_pair(position.x(), position.y()));
    redrawEdges();
    createColorMap();
}

// This function sorts the nodes in the x-value order.
std::vector<std::pair<float, int>> TransferWidget::sortNodes() {
    std::vector<std::pair<float, int>> sortedIndices;

    // Create unsorted node list.
    for (int i = 0; i < _nodePositions.size(); i++) {
        sortedIndices.push_back(std::make_pair(_nodePositions[i].first, i));
    }

    // Sort and return the list.
    sort(sortedIndices.begin(), sortedIndices.end());
    return sortedIndices;
}

// This function redraws the lines between the nodes that indicate the transferfunction.
void TransferWidget::redrawEdges() {
    // Remove all edges in the list.
    for (int i = 0; i < _edgeList.size(); i++) {
        _scene->removeItem(_edgeList[i]);
    }
    _edgeList.clear();

    // Get the sorted node list.
    std::vector<std::pair<float, int>> sortedIndices = sortNodes();

    // Add lines between all nodes.    
    for (int i = 0; i < sortedIndices.size() - 1; i++) {
        Edge* newEdge = new Edge(_nodeList[sortedIndices[i].second], _nodeList[sortedIndices[i + 1].second]);
        _edgeList.push_back(newEdge);
        _scene->addItem(_edgeList[i]);
    }

    // Recreate colormap after the edges and nodes have been changed.
    createColorMap();
}

// This function can be used in order to set the current node.
void TransferWidget::setCurrentNode(QPointF newPosition) {
    _currentNodePosition = newPosition;
    this->update();
}

// This function is used t when an item is moved to update its position and redraw the lines between the nodes.
void TransferWidget::itemMoved(QPointF newPosition) {
    for (int i = 0; i < _nodePositions.size(); i++) {
        if (_nodePositions[i].first == _currentNodePosition.x()) {
            _nodePositions[i].first = newPosition.x();
            _nodePositions[i].second = newPosition.y();
            _currentNodePosition = newPosition;
            redrawEdges();  
        }
    }

}

// This function draws the axis inside the transferfunction.
void TransferWidget::drawBackground(QPainter* painter,const QRectF& rect)
{
    Q_UNUSED(rect);

    QPen pen;
    pen.setWidth(2);
    painter->setPen(pen);

    QRectF axisRect(QPoint(0, _windowHeight), QPoint(_windowWidth, 0));
    painter->drawRect(axisRect);

    // Divide the windowheight in 4.
    auto lineHeightWidth = (float)_windowHeight / 4;

    painter->drawText(QPoint(-30, 0), "100%");
    painter->drawText(QPoint(-30, lineHeightWidth), "75%");
    painter->drawText(QPoint(-30, lineHeightWidth * 2), "50%");
    painter->drawText(QPoint(-30, lineHeightWidth * 3), "25%");
    painter->drawText(QPoint(-30, lineHeightWidth * 4), "0%");

    pen.setWidth(1);
    pen.setStyle(Qt::DashLine);
    pen.setColor(Qt::lightGray);

    QVector<qreal> dashes;
    qreal space = 5;
    dashes << 5 << space  << 5 << space ;

    pen.setDashPattern(dashes);
    painter->setPen(pen);

    // Draw dashed lines to indicate color alpha for the transferfunction.
    QLine dashedLine75(QPoint(0,lineHeightWidth), QPoint(_windowWidth,lineHeightWidth));
    painter->drawLine(dashedLine75);

    QLine dashedLine50(QPoint(0, lineHeightWidth*2), QPoint(_windowWidth, lineHeightWidth*2));
    painter->drawLine(dashedLine50);

    QLine dashedLine25(QPoint(0, lineHeightWidth*3), QPoint(_windowWidth, lineHeightWidth*3));
    painter->drawLine(dashedLine25);
}
