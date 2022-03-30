#ifndef CustomColorMapEditor_H
#define CustomColorMapEditor_H

/** QT headers*/
#include <QWidget>
#include <QResizeEvent>
#include <QGraphicsView>
/**  HDPS headers*/
#include <Dataset.h>
#include <PointData.h>
//#include <Transfer/Node.h>
#include <Transfer/TransferWidget.h>
#include <Transfer/TransferFunctionControlAction.h>

class VolumeViewerPlugin;
using namespace hdps;
class TransferWidget;

class CustomColorMapEditor : public QWidget
{
    Q_OBJECT

public:
    CustomColorMapEditor(VolumeViewerPlugin& volumeViewer, QWidget* parent = nullptr);
    void CustomColorMapEditor::createHistogram(Points& data, int chosenDim);

    //void itemMoved();
    std::vector<int> CustomColorMapEditor::getHistogram();
    TransferWidget& CustomColorMapEditor::getTransferFunction();

    TransferFunctionControlAction& getTransferFunctionControlAction() {
        return _transferFunctionControlAction;
    }

    bool CustomColorMapEditor::getDataLoaded();

    QPushButton& getDeleteButton() {
        return _deleteButton;
    };
    QPushButton& getFirstButton() {
        return _firstButton;
    };
    QPushButton& getPreviousButton() {
        return _previousButton;
    };
    QPushButton& getNextButton() {
        return _nextButton;
    };
    QPushButton& getLastButton() {
        return _lastButton;
    };





private:
    std::vector<int> _histogram;
    bool _dataLoaded;
    TransferWidget* _transferWidget;
    QPushButton _firstButton;
    QPushButton _previousButton;
    QPushButton _nextButton;
    QPushButton _lastButton;
    QPushButton _deleteButton;
    
        
    
    TransferFunctionControlAction _transferFunctionControlAction;
    

    
};
#endif // CustomColorMapEditor_H