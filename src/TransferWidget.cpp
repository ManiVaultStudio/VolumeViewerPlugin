/** General headers */
#include <math.h>
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


using namespace hdps;
using namespace hdps::gui;


TransferWidget::TransferWidget(VolumeViewerPlugin& VolumeViewerPlugin, QWidget* parent) :
    QWidget(parent),
    _transferScene(),
    _transferView()

{
    _transferScene = new QGraphicsScene(this);
    _transferScene->setItemIndexMethod(QGraphicsScene::NoIndex);
    //_transferScene->setSceneRect(-50, -50, 100, 100);
    _transferView = new QGraphicsView(this);
    _transferView->setScene(_transferScene);
    _transferView->setSceneRect(QRect(0, 0, 100, 100));

}

TransferWidget::~TransferWidget()
{

}

void TransferWidget::drawHistogram(Points& data, int chosenDim) {
    std::vector<int> histogram = createHistogram(data, chosenDim);

}

std::vector<int> TransferWidget::createHistogram(Points& data, int chosenDim) {
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
    std::vector<int> bins(100, 0);

    for (int i = 0; i < numPoints * numDimensions; i++) {
        dim = i % numDimensions;
        if (chosenDim == dim) {
            currentPoint = data.getValueAt(i);
            if (currentPoint != range[0]) {
                int binIndex = (currentPoint ) / binSize;
                bins[binIndex]++;
            }
        }
    }
    
    return bins;
}