#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_2_Core>

#include "VolumeRenderer.h"
#include "Tracker.h"

#include "graphics/Vector3f.h"
#include "graphics/Vector2f.h"

#include <vector>

#include <QTimer>
#define CONTROLS

#ifdef CONTROLS
#include "Controls.h"
#endif

#include "ReferenceSetup.h"

/**
 * OpenGL Volume Renderer Widget
 * This class provides a widget interface to the OpenGL Volume Renderer
 * 
 * @author Julian Thijssen
 */

struct sphericCoords {
    float distance {1}; // Distance to center
    float polar { 90 }; // Angle with the vertical in deg. Vertical is Y
    float azimuthal{ 0 }; // Angle or rotation around vertical in deg
};

enum class SelectionMode {
    Nearest, Sphere
};


class OpenGLRendererWidget : public QOpenGLWidget, QOpenGLFunctions_4_2_Core
{
    Q_OBJECT

public:
    OpenGLRendererWidget();


    VolumeRenderer& getVolumeRenderer() { return _volumeRenderer; }


    //void setTexels(int width, int height, int depth, std::vector<float>& texels);
    void setData(std::vector<float>& data);
    void setColors(std::vector<float>& colors);
    void setColormap(const QImage& colormap);
    void connectToTracker();
    void setEyeOffset(float eyeOffset);
    void setCamDist(float camDist);
    void setSelectionMode(const int32_t& mode);
    void openCalib() const { refWidget->show(); };

public:
    bool eventFilter(QObject* target, QEvent* event);

    /**
    * Returns vector of position of the camera in space, calculated from the spherical coordinates
    */
    mv::Vector3f getCamPos() const;

protected:

    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void cleanup();

    void showEvent(QShowEvent* event) Q_DECL_OVERRIDE
    {
        emit created();
        QWidget::showEvent(event);
    }

private slots:
    void updatePixelRatio();

signals:
    void created();
    void newSelection(const SelectionMode& type, const bool& replace);

private:
    VolumeRenderer _volumeRenderer;
    PSTracker _tracker;

    #ifdef CONTROLS
        ControlsWidget* _controls;
    #endif

    
    ReferenceSetupWidget* refWidget = nullptr;


    float _camStartDist = 1.0f;

    bool _isInitialized = false;

    // UI Controls
    sphericCoords viewPosSpheric;
    QPointF _previousMousePos;
    bool _mousePressed = false;

    bool _selecting = false;
    bool selectionReplaces = true;

    SelectionMode selectionMode;



    QTimer* _updateTimer = nullptr;

    float _pixelRatio = 1.0f; /** Current pixel ratio */
};
