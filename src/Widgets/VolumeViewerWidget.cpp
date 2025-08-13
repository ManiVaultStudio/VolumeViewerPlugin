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

    layout = new QVBoxLayout(this);
    layout->addWidget(_openGLWidget);

    setLayout(layout);

    connect(_openGLWidget, &OpenGLRendererWidget::exitFullScreen, this, [this]() {
        layout->addWidget(_openGLWidget);
    });
    
}


void VolumeViewerWidget::setData(Dataset<Points> pointDataset)
{

    switch (_plugin->getRendererBackend())
    {
    case VolumeViewerPlugin::RendererBackend::OpenGL:
    {
        int numDimensions = pointDataset->getNumDimensions();
        if (numDimensions != 3) {
            qDebug() << "WARNING: DIMENSIONS ARE NOT 3";
            return;
        }
        points = std::vector<float>(pointDataset->getNumPoints() * pointDataset->getNumDimensions());

        // Determine data bounds and averages
        QVector3D minCoord(std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max());
        QVector3D maxCoord(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
        _meanCoord = QVector3D(0, 0, 0);
        for (int i = 0; i < pointDataset->getNumPoints(); i++)
        {
            float x = pointDataset->getValueAt(i * numDimensions + 0);
            float y = pointDataset->getValueAt(i * numDimensions + 1);
            float z = pointDataset->getValueAt(i * numDimensions + 2);

            if (x < minCoord.x()) minCoord.setX(x);
            if (x > maxCoord.x()) maxCoord.setX(x);
            if (y < minCoord.y()) minCoord.setY(y);
            if (y > maxCoord.y()) maxCoord.setY(y);
            if (z < minCoord.z()) minCoord.setZ(z);
            if (z > maxCoord.z()) maxCoord.setZ(z);
            _meanCoord += QVector3D(x, y, z);
        }
        _meanCoord /= pointDataset->getNumPoints();
        QVector3D range = maxCoord - minCoord;
        _maxRange = std::max(range.x(), std::max(range.y(), range.z()));

        // Rescale data
        for (int i = 0; i < pointDataset->getNumPoints(); i++)
        {
            float x = pointDataset->getValueAt(i * numDimensions + 0);
            float y = pointDataset->getValueAt(i * numDimensions + 1);
            float z = pointDataset->getValueAt(i * numDimensions + 2);

            points[i * 3 + 0] = (x - _meanCoord.x()) / _maxRange;
            points[i * 3 + 1] = (y - _meanCoord.y()) / _maxRange;
            points[i * 3 + 2] = (z - _meanCoord.z()) / _maxRange;
        }

        getOpenGLWidget()->setData(&points);

        //Initial render
        getOpenGLWidget()->update();



        break;
    }
    }

    // Start worker for point selection
    pointSelectorWorker = new PointSelector(this, &pointSelectionCursor, &points, &currentSelectionRadius);
    connect(pointSelectorWorker, &PointSelector::resultReady, this, [this](const std::vector<GLuint>& selection) {
        emit selectionReady(selection, currentSelectionReplaces);

    });
    pointSelectorWorker->start();

    connect(_openGLWidget, &OpenGLRendererWidget::newSelection, this, [this](const SelectionMode& type, const bool& replace) {

        // Perform selection of closest point
        const QVector3D cursor = _openGLWidget->getVolumeRenderer().getCursor();

        switch (type) {
        case SelectionMode::Nearest: {
            requestSelection(cursor, replace, -1);
            break;
        }
        case SelectionMode::Sphere: {
            //selection = _volumeViewerWidget->getPointsInSphere(cursor, getVolumeRenderer().getSelectRadius());
            requestSelection(cursor, replace, _openGLWidget->getVolumeRenderer().getSelectRadius());
            break;
        }
        }



    });
}


void VolumeViewerWidget::requestSelection(const QVector3D& cursor, bool replaces, float radius)
{
    currentSelectionRadius = radius;
    currentSelectionReplaces = replaces;
    pointSelectionCursor = cursor;
}



std::vector<float> VolumeViewerWidget::getPointDistances(const QVector3D& cursor) const {

    auto dataset = _plugin->getDataset();
    int numDimensions = dataset->getNumDimensions();


    std::vector<float> result = std::vector<float>(points.size() / 3, 0.0f);

    for (std::uint32_t localIndex = 0; localIndex < points.size() / numDimensions; localIndex++) {
        
        result[localIndex] = std::sqrt(
            std::pow(cursor[0] - points[localIndex * numDimensions + 0], 2)
            + std::pow(cursor[1] - points[localIndex * numDimensions + 1], 2)
            + std::pow(cursor[2] - points[localIndex * numDimensions + 2], 2)
        ); // Distance

    }


    return result;

}