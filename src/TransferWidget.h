#ifndef TransferWidget_H
#define TransferWidget_H

/** QT headers*/
#include <QWidget>
#include <QResizeEvent>
#include <QGraphicsView>
/**  HDPS headers*/
#include <Dataset.h>
#include <PointData.h>
#include <TransferPointModel.h>



class VolumeViewerPlugin;
using namespace hdps;

class TransferWidget : public QGraphicsView
{
    Q_OBJECT

    public:
        explicit TransferWidget(VolumeViewerPlugin& VolumeViewerPlugin, QWidget* parent = nullptr);
        ~TransferWidget();

        void createHistogram(Points& data, int chosenDim);

        //void drawHistogram(Points& data, int chosenDim);

        void paintEvent(QPaintEvent*);


    private:
        QGraphicsScene*  _transferScene;
        QGraphicsView*   _transferView;
        bool             _dataLoaded;
        std::vector<int> _histogram;
       

    protected:

};
#endif // TransferWidget_H