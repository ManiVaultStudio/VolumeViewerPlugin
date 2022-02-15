#ifndef TransferWidget2_H
#define TransferWidget2_H

/** QT headers*/
#include <QWidget>
#include <QResizeEvent>
#include <QGraphicsView>
/**  HDPS headers*/
#include <Dataset.h>
#include <PointData.h>
//#include <Transfer/Node.h>
#include <Transfer/TransferWidget.h>


class VolumeViewerPlugin;
using namespace hdps;
class TransferWidget;

class TransferWidget2 : public QWidget
{
    Q_OBJECT

public:
    TransferWidget2(VolumeViewerPlugin& volumeViewer, QWidget* parent = nullptr);
    void TransferWidget2::createHistogram(Points& data, int chosenDim);

    //void itemMoved();
    std::vector<int> TransferWidget2::getHistogram();



    bool TransferWidget2::getDataLoaded();

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
    //void drawBackground(QPainter* painter, const QRectF& rect) override;

    //void scaleView(qreal scaleFactor);

private:
    std::vector<int> _histogram;
    bool _dataLoaded;
    TransferWidget* _transferWidget;
    //int timerId = 0;
    //Node centerNode;
    
};
#endif // TransferWidget2_H