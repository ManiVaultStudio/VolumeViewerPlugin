/** General headers */
#include <math.h>
#include <iostream>     
#include <algorithm> 
#include <vector>
#include <cmath>
/** Plugin headers */
#include <Transfer/TransferWidget2.h>
#include <RendererSettingsAction.h>
#include <VolumeViewerPlugin.h>
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
#include <qslider.h>
#include <qdir.h>
#include <qbuttongroup.h>

#include <Transfer/TransferWidget.h>

using namespace hdps;
using namespace hdps::gui;


TransferWidget2::TransferWidget2(VolumeViewerPlugin& VolumeViewerPlugin, QWidget* parent)
    : QWidget(parent),
    _histogram(std::vector<int>(100, 0)),
    _dataLoaded(false),
    _transferWidget(),
    _transferFunctionControlAction(this,_transferWidget),
    _firstButton(new QPushButton(this)),
    _previousButton(new QPushButton(this)),
    _nextButton(new QPushButton(this)),
    _lastButton(new QPushButton(this)),
    _deleteButton(new QPushButton(this))
    
{
    this->setFixedSize(400, 800);
    _transferWidget = new TransferWidget(this);
    _transferWidget->setFixedHeight(260);

   
    auto vertLayout = new QVBoxLayout();
    auto horLayout = new QHBoxLayout();

    //auto buttons = new QButtonGroup(this);
    
    auto icon = QIcon("F:/LUMC/VolumeViewerPlugin/src/Transfer/control-stop-180.png");
    _firstButton.setIcon(icon);
    _firstButton.setStatusTip("Select first node");
    _firstButton.setToolTip("Select first node");
    _firstButton.setFixedWidth(20);
    _firstButton.setFixedHeight(20);

    
    icon = QIcon("F:/LUMC/VolumeViewerPlugin/src/Transfer/control-180.png");
    _previousButton.setIcon(icon);
    _previousButton.setStatusTip("Select previous node");
    _previousButton.setToolTip("Select previous node");
    _previousButton.setFixedWidth(20);
    _previousButton.setFixedHeight(20);

    
    icon = QIcon("F:/LUMC/VolumeViewerPlugin/src/Transfer/control.png");
    _nextButton.setIcon(icon);
    _nextButton.setStatusTip("Select next node");
    _nextButton.setToolTip("Select next node");
    _nextButton.setFixedWidth(20);
    _nextButton.setFixedHeight(20);

    
    icon = QIcon("F:/LUMC/VolumeViewerPlugin/src/Transfer/control-stop.png");
    _lastButton.setIcon(icon);
    _lastButton.setStatusTip("Select final node");
    _lastButton.setToolTip("Select final node");
    _lastButton.setFixedWidth(20);
    _lastButton.setFixedHeight(20);

    
    icon = QIcon("F:/LUMC/VolumeViewerPlugin/src/Transfer/cross.png");
    _deleteButton.setIcon(icon);
    _deleteButton.setStatusTip("Delete selected node");
    _deleteButton.setToolTip("Delete selected node");
    _deleteButton.setFixedWidth(20);
    _deleteButton.setFixedHeight(20);
    
    

    horLayout->addWidget(&_firstButton);
    horLayout->addWidget(&_previousButton);
    horLayout->addWidget(&_nextButton);
    horLayout->addWidget(&_lastButton);
    horLayout->addWidget(&_deleteButton);

   
    vertLayout->addWidget(_transferWidget, 1);
    vertLayout->addLayout(horLayout, 2);
    vertLayout->addWidget(_transferFunctionControlAction.createWidget(this),3);
    setLayout(vertLayout);
    
    
    
    
    
}

void TransferWidget2::createHistogram(Points& data, int chosenDim) {

    
    // get number of points from points dataset
    int numPoints = data.getNumPoints();

    // get number of dimensions from points dataset
    int numDimensions = data.getNumDimensions();

    int dim;
    int k = 0;
    std::vector<float> range(2, 0);
    float currentPoint;

    for (int i = 0; i < numPoints * numDimensions; i++) {
        dim = i % numDimensions;
        if (chosenDim == dim) {
            currentPoint = data.getValueAt(i);
            if (k == 0) {
                range[0] = currentPoint;
                range[1] = currentPoint;
            }
            else if (currentPoint < range[0]) {
                range[0] = currentPoint;
            }
            else if (currentPoint > range[1]) {
                range[1] = currentPoint;
            }
            k++;
        }
    }

    float size = range[1] - (range[0] + 1);
    float binSize = size / 100;
    std::cout << "created" << std::endl;

    for (int i = 0; i < numPoints * numDimensions; i++) {
        dim = i % numDimensions;
        if (chosenDim == dim) {
            currentPoint = data.getValueAt(i);
            if (currentPoint != range[0]) {
                int binIndex = (currentPoint) / binSize;
                _histogram[binIndex]++;
            }
        }
    }

    _dataLoaded = true;
    
    
}

std::vector<int> TransferWidget2::getHistogram() {
   
    return _histogram;
}

bool TransferWidget2::getDataLoaded() {
    return _dataLoaded;
}

TransferWidget& TransferWidget2::getTransferFunction() {
    return *_transferWidget;
}