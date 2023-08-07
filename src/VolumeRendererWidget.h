#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_2_Core>

#include "VolumeRenderer.h"

#include "graphics/Vector3f.h"
#include "graphics/Vector2f.h"

#include <vector>

class VolumeRendererWidget : public QOpenGLWidget, QOpenGLFunctions_4_2_Core
{
    Q_OBJECT

public:
    VolumeRendererWidget();

    void setTexels(int width, int height, int depth, std::vector<float>& texels);
    void setData(std::vector<float>& data);
    void setColors(std::vector<float>& colors);

public:
    bool eventFilter(QObject* target, QEvent* event);

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void cleanup();

private:
    VolumeRenderer _volumeRenderer;

    hdps::Vector3f _camPos;
    float _camDist = 0.8f;
    hdps::Vector2f _camAngle = hdps::Vector2f(3.14159f / 2, 0);

    QPointF _previousMousePos;
    bool _mousePressed = false;

    bool _isInitialized = false;
};
