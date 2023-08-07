#include "VolumeViewerWidget.h"

#include "VolumeViewerPlugin.h"

#include "ViewerWidget.h"
#include "Renderer/OpenGL/OpenGLRendererWidget.h"

#include <QEvent>
#include <QMouseEvent>

VolumeViewerWidget::VolumeViewerWidget(QObject* parent, const QString& title) :
    _plugin(dynamic_cast<VolumeViewerPlugin*>(parent)),
    _vtkWidget(nullptr),
    _openGLWidget(nullptr)
{
    setAcceptDrops(true);

    _vtkWidget = new ViewerWidget(*_plugin);
    _openGLWidget = new OpenGLRendererWidget();

    auto* layout = new QVBoxLayout();
    layout->addWidget(_openGLWidget);

    setLayout(layout);
}
