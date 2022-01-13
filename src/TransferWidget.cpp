/** General headers */
#include <math.h>
#include <iostream>     
#include <algorithm> 
#include <vector>
#include <cmath>
/** Plugin headers */
#include <TransferWidget.h>
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


TransferWidget::TransferWidget(VolumeViewerPlugin& VolumeViewerPlugin, QWidget* parent) :
    QWidget(parent),
    _dataLoaded(false),
    _transferScene(),
    _transferView()

{
    _transferScene = new QGraphicsScene(this);
    _transferScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    //_transferScene->setSceneRect(-50, -50, 100, 100);
    _transferView = new QGraphicsView(this);
    _transferView->setScene(_transferScene);
    //_transferView->setSceneRect(QRect(0, 0, 255, 120));
    _histogram = std::vector<int>(100, 0);
   

}

TransferWidget::~TransferWidget()
{

}

void TransferWidget::paintEvent(QPaintEvent *) {
    
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

        float windowWidth = this->width();
        int windowHeight = this->height();
        float binWidth = windowWidth / 100;

        int binIncrement = maxBin / windowHeight;
        float height = 0;

        QPainter painter(this);
        QBrush brush(Qt::lightGray, Qt::SolidPattern);
        painter.setPen(Qt::lightGray);
        
        
        //painter.drawRect(QRectF(QPoint(0, windowHeight), QSize(binWidth, -windowHeight*height)));
        
        for (int i = 0; i < 100; i++) {
            currentBin = _histogram[i];
            height = currentBin / maxBin;
            QRectF currentRect = QRectF(QPoint(i * binWidth, windowHeight), QSize(binWidth, -windowHeight * height));
            painter.drawRect(currentRect);
            painter.fillRect(currentRect, brush);
            
        }
    }
}

void TransferWidget::createHistogram(Points& data, int chosenDim) {
    
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

    float size = range[1] - (range[0]+1);
    float binSize = size / 100;
    

    for (int i = 0; i < numPoints * numDimensions; i++) {
        dim = i % numDimensions;
        if (chosenDim == dim) {
            currentPoint = data.getValueAt(i);
            if (currentPoint != range[0]) {
                int binIndex = (currentPoint ) / binSize;
                _histogram[binIndex]++;
            }
        }
    }
    
    _dataLoaded = true;
    this->update();
}