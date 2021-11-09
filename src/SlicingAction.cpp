#include "SlicingAction.h"
#include "RendererSettingsAction.h"
#include "ViewerWidget.h"
#include "VolumeViewerPlugin.h"
#include <QtCore>
#include <QtDebug>
#include <QFileDialog>
#include <qmessagebox.h>

using namespace hdps;

SlicingAction::SlicingAction(RendererSettingsAction& rendererSettingsAction,ViewerWidget* viewerWidget) :
    GroupAction(reinterpret_cast<QObject*>(&rendererSettingsAction)),
    _rendererSettingsAction(rendererSettingsAction),
    _xAxisEnabledAction(this, "x-axis enabled"),
    _xAxisPositionAction(this, "x-axis position", 0.0f, 100.0f, 0.0f, 0.0f, 0),
    _yAxisEnabledAction(this, "y-axis enabled"),
    _yAxisPositionAction(this, "y-axis position", 0.0f, 100.0f, 0.0f, 0.0f, 0),
    _zAxisEnabledAction(this, "z-axis enabled"),
    _zAxisPositionAction(this, "z-axis position", 0.0f, 100.0f, 0.0f, 0.0f, 0),
    _viewerWidget(nullptr)
{
    setText("Slicing parameters");
    setMayReset(false);
    //bool toggled = false;
    _xToggled = false; 
    _yToggled = false;
    _zToggled = false;
    _viewerWidget = viewerWidget;
    
    
    connect(&_xAxisEnabledAction, &ToggleAction::toggled, this, [this](bool toggled) {        
        _xToggled = toggled;
        if (toggled) {
            qDebug() << "X-Position slicing enabled";
        }       
    });

    connect(&_xAxisPositionAction, &DecimalAction::valueChanged, this, [this](const float& value) {
        if (_xToggled) {                      
            qDebug() << "X-Position slice changed to xMax =" << value;
        }
    });
    connect(&_yAxisEnabledAction, &ToggleAction::toggled, this, [this](bool toggled) {       
        _yToggled = toggled;
        if (toggled) {
            qDebug() << "y-Position slicing enabled";
        }
    });

    connect(&_yAxisPositionAction, &DecimalAction::valueChanged, this, [this](const float& value) {
        if (_yToggled) {
            int value = _yAxisPositionAction.getValue();
            qDebug() << "Y-Position slice changed to xMax =" << value;
        }
    });
    connect(&_zAxisEnabledAction, &ToggleAction::toggled, this, [this](bool toggled) {        
        _zToggled = toggled;
        if (toggled) {
            qDebug() << "Z-Position slicing enabled";
        }        
    });

    connect(&_zAxisPositionAction, &DecimalAction::valueChanged, this, [this](const float& value) {
        if (_zToggled) {         
            int value = _zAxisPositionAction.getValue();
            qDebug() << "Z-Position slice changed to xMax =" << value;
        }
    });
}
