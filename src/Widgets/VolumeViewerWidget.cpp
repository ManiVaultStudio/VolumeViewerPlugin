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

void VolumeViewerWidget::setData(Dataset<Points> points)
{
    switch (_plugin->getRendererBackend())
    {
    case VolumeViewerPlugin::RendererBackend::VTK:
    {

        break;
    }
    case VolumeViewerPlugin::RendererBackend::OpenGL:
    {
        int numDimensions = points->getNumDimensions();
        if (numDimensions != 3) qDebug() << "WARNING: DIMENSIONS ARE NOT 3";
        std::vector<float> values(points->getNumPoints() * points->getNumDimensions());

        QVector3D minCoord(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        QVector3D maxCoord(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
        QVector3D meanCoord(0, 0, 0);
        for (int i = 0; i < points->getNumPoints(); i++)
        {
            float x = points->getValueAt(i * numDimensions + 0);
            float y = points->getValueAt(i * numDimensions + 1);
            float z = points->getValueAt(i * numDimensions + 2);

            if (x < minCoord.x()) minCoord.setX(x);
            if (x > maxCoord.x()) maxCoord.setX(x);
            if (y < minCoord.y()) minCoord.setY(y);
            if (y > maxCoord.y()) maxCoord.setY(y);
            if (z < minCoord.z()) minCoord.setZ(z);
            if (z > maxCoord.z()) maxCoord.setZ(z);
            meanCoord += QVector3D(x, y, z);
        }
        meanCoord /= points->getNumPoints();
        QVector3D range = maxCoord - minCoord;
        float maxRange = std::max(range.x(), std::max(range.y(), range.z()));
        for (int i = 0; i < points->getNumPoints(); i++)
        {
            float x = points->getValueAt(i * numDimensions + 0);
            float y = points->getValueAt(i * numDimensions + 1);
            float z = points->getValueAt(i * numDimensions + 2);

            values[i * 3 + 0] = (x - meanCoord.x()) / maxRange;
            values[i * 3 + 1] = (y - meanCoord.y()) / maxRange;
            values[i * 3 + 2] = (z - meanCoord.z()) / maxRange;
        }

        qDebug() << minCoord << maxCoord;
        int xSize = (maxCoord.x() - minCoord.x()) * 10 + 1;
        int ySize = (maxCoord.y() - minCoord.y()) * 10 + 1;
        int zSize = (maxCoord.z() - minCoord.z()) * 10 + 1;
        qDebug() << xSize << ySize << zSize;
        std::vector<float> texels(xSize * ySize * zSize, 0);

        for (int i = 0; i < points->getNumPoints(); i++)
        {
            int x = points->getValueAt(i * numDimensions + 0) * 10;
            int y = points->getValueAt(i * numDimensions + 1) * 10;
            int z = points->getValueAt(i * numDimensions + 2) * 10;
            //qDebug() << x << y << z;
            texels[x * ySize * zSize + y * zSize + z] = 1;
        }

        getOpenGLWidget()->setData(values);

        //Initial render
        getOpenGLWidget()->update();

        break;
    }
    }
}
