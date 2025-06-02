#include "VolumeViewerWidget.h"

#include "VolumeViewerPlugin.h"

#include "Renderer/OpenGL/OpenGLRendererWidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <cmath>

VolumeViewerWidget::VolumeViewerWidget(QObject* parent, const QString& title) :
    _plugin(dynamic_cast<VolumeViewerPlugin*>(parent)),
    _openGLWidget(nullptr)
{

    setAcceptDrops(true);

    _openGLWidget = new OpenGLRendererWidget();

    auto* layout = new QVBoxLayout();
    layout->addWidget(_openGLWidget);

    setLayout(layout);
    
}

void VolumeViewerWidget::setData(Dataset<Points> points)
{
    switch (_plugin->getRendererBackend())
    {
    case VolumeViewerPlugin::RendererBackend::OpenGL:
    {
        int numDimensions = points->getNumDimensions();
        if (numDimensions != 3) qDebug() << "WARNING: DIMENSIONS ARE NOT 3";
        std::vector<float> values(points->getNumPoints() * points->getNumDimensions());

        QVector3D minCoord(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        QVector3D maxCoord(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
        _meanCoord = QVector3D(0, 0, 0);
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
            _meanCoord += QVector3D(x, y, z);
        }
        _meanCoord /= points->getNumPoints();
        QVector3D range = maxCoord - minCoord;
        _maxRange = std::max(range.x(), std::max(range.y(), range.z()));
        for (int i = 0; i < points->getNumPoints(); i++)
        {
            float x = points->getValueAt(i * numDimensions + 0);
            float y = points->getValueAt(i * numDimensions + 1);
            float z = points->getValueAt(i * numDimensions + 2);

            values[i * 3 + 0] = (x - _meanCoord.x()) / _maxRange;
            values[i * 3 + 1] = (y - _meanCoord.y()) / _maxRange;
            values[i * 3 + 2] = (z - _meanCoord.z()) / _maxRange;
        }

        getOpenGLWidget()->setData(values);

        //Initial render
        getOpenGLWidget()->update();

        

        break;
    }
    }
}

uint32_t VolumeViewerWidget::getClosestPoint(const QVector3D& cursor) const {

    auto dataset = _plugin->getDataset();
    int numDimensions = dataset->getNumDimensions();

    // Get reference to the indices of the selection set
    std::vector<std::uint32_t> localGlobalIndices;
    dataset->getGlobalIndices(localGlobalIndices);

    uint32_t indiceMin = 0;
    float distanceMin = FLT_MAX;


    for (std::uint32_t localIndex = 0; localIndex < dataset->getNumPoints(); localIndex++) {
        float x = dataset->getValueAt(localIndex * numDimensions + 0);
        float y = dataset->getValueAt(localIndex * numDimensions + 1);
        float z = dataset->getValueAt(localIndex * numDimensions + 2);

        x = (x - _meanCoord.x()) / _maxRange;
        y = (y - _meanCoord.y()) / _maxRange;
        z = (z - _meanCoord.z()) / _maxRange;


        const float distance = std::sqrt(std::pow(cursor[0] - x, 2)+ std::pow(cursor[1] - y, 2)+ std::pow(cursor[2] - z, 2));


        if (distance < distanceMin)
        {
            indiceMin = localIndex;
            distanceMin = distance;
        }

        
    }


    return localGlobalIndices[indiceMin];
}

std::vector<uint32_t> VolumeViewerWidget::getPointsInSphere(const QVector3D& cursor, const float& radius) const {
    std::vector<std::uint32_t> result;

    auto dataset = _plugin->getDataset();
    int numDimensions = dataset->getNumDimensions();

    // Get reference to the indices of the selection set
    std::vector<std::uint32_t> localGlobalIndices;
    dataset->getGlobalIndices(localGlobalIndices);


    for (std::uint32_t localIndex = 0; localIndex < dataset->getNumPoints(); localIndex++) {
        float x = dataset->getValueAt(localIndex * numDimensions + 0);
        float y = dataset->getValueAt(localIndex * numDimensions + 1);
        float z = dataset->getValueAt(localIndex * numDimensions + 2);

        x = (x - _meanCoord.x()) / _maxRange;
        y = (y - _meanCoord.y()) / _maxRange;
        z = (z - _meanCoord.z()) / _maxRange;


        const float distance = std::sqrt(std::pow(cursor[0] - x, 2) + std::pow(cursor[1] - y, 2) + std::pow(cursor[2] - z, 2));


        if (distance < radius)
        {
            result.push_back(localGlobalIndices[localIndex]);
        }


    }


    return result;
}