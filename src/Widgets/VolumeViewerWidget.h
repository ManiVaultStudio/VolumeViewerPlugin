#pragma once

#include <QWidget>

#include <vector>

class VolumeViewerPlugin;
class ViewerWidget;
class OpenGLRendererWidget;

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
