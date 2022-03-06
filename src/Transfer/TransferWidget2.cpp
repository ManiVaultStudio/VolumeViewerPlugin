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

#include <Transfer/TransferWidget.h>

using namespace hdps;
using namespace hdps::gui;


TransferWidget2::TransferWidget2(VolumeViewerPlugin& VolumeViewerPlugin, QWidget* parent)
    : QWidget(parent),
    _histogram(std::vector<int>(100, 0)),
    _dataLoaded(false),
    _transferWidget()
{
    _transferWidget = new TransferWidget(this);
    this->setFixedSize(400, 300);
    
    
    
    
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