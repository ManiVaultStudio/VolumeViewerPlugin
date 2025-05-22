#pragma once

#include "Dataset.h"
#include "PointData/PointData.h"

#include <QWidget>
#include <QVector3D>

#include "graphics/Vector3f.h"

#include <vector>

class VolumeViewerPlugin;
class OpenGLRendererWidget;

using namespace mv;

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

    uint32_t getClosestPoint(const QVector3D& cursor) const;

public:
    OpenGLRendererWidget* getOpenGLWidget()
    {
        return _openGLWidget;
    }


private:

    VolumeViewerPlugin*     _plugin;

    OpenGLRendererWidget*   _openGLWidget;

    QVector3D _meanCoord;
    float _maxRange;
};
