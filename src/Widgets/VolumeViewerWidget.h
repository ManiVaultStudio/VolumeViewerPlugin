#pragma once

#include "Dataset.h"
#include "PointData/PointData.h"

#include <QWidget>

#include <vector>

class VolumeViewerPlugin;
class ViewerWidget;
class OpenGLRendererWidget;

using namespace hdps;

/**
 * Volume Renderer Widget
 * Overarching widget class that contains the different renderer widgets
 * 
 * @author Julian Thijssen
 */
class VolumeViewerWidget : public QWidget
{
    Q_OBJECT

public:
    VolumeViewerWidget(QObject* parent, const QString& title);

    void setData(Dataset<Points> points);

public:
    ViewerWidget* getVTKWidget()
    {
        return _vtkWidget;
    }

    OpenGLRendererWidget* getOpenGLWidget()
    {
        return _openGLWidget;
    }

private:
    VolumeViewerPlugin*     _plugin;

    ViewerWidget*           _vtkWidget;
    OpenGLRendererWidget*   _openGLWidget;
};
