#pragma once

#include "Dataset.h"
#include "PointData/PointData.h"

#include <QWidget>
#include <QVector3D>
#include <QLayout>

#include "graphics/Vector3f.h"

#include <vector>

#include "Renderer/OpenGL/PointOptimizer.h"

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

    //uint32_t getClosestPoint(const QVector3D& cursor) const;
    void requestSelection(const QVector3D& cursor, bool replaces, float radius);
    std::vector<float> getPointDistances(const QVector3D& cursor) const;


public:
    OpenGLRendererWidget* getOpenGLWidget()
    {
        return _openGLWidget;
    }

signals:
    void selectionReady(std::vector<GLuint>, const bool& replaces);

private:

    VolumeViewerPlugin*     _plugin;

    std::vector<float> points;

    OpenGLRendererWidget*   _openGLWidget;

    QVector3D _meanCoord;
    float _maxRange;

    QLayout* layout;


    PointSelector* pointSelectorWorker = nullptr;
    QVector3D pointSelectionCursor;
    bool currentSelectionReplaces = false;
    float currentSelectionRadius = -1.f;
};
