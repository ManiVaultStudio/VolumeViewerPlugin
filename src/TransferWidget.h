#ifndef TransferWidget_H
#define TransferWidget_H

/** QT headers*/
#include <QWidget>
#include <QResizeEvent>
/**  HDPS headers*/
#include <Dataset.h>
#include <PointData.h>


class VolumeViewerPlugin;
using namespace hdps;

class TransferWidget : public QWidget
{
    Q_OBJECT

    public:
        explicit TransferWidget(VolumeViewerPlugin& VolumeViewerPlugin, QWidget* parent = nullptr);
        ~TransferWidget();

        std::vector<int> createHistogram(Points& data, int chosenDim);

        void drawHistogram(Points& data, int chosenDim);

    private:
        QGraphicsScene* _transferScene;
        QGraphicsView* _transferView;

    protected:

};
#endif // TransferWidget_H