#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_2_Core>

#include <vector>

#include <QTimer>
#include <QHBoxLayout>


//#define CONTROLS

#ifdef CONTROLS
#include "Controls.h"
#endif

#include "Widgets/ReferenceSetupWidget.h"
#include <Controllers/Pedal.h>
#include <Widgets/FullScreenWidget.h>

#include "VolumeRenderer.h"
#include "Controllers/Tracker.h"

#include "graphics/Vector3f.h"
#include "graphics/Vector2f.h"

#include "PointOptimizer.h"

#include "Widgets/FlashlightWidget.h"


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
    ~OpenGLRendererWidget();


    VolumeRenderer& getVolumeRenderer() { return _volumeRenderer; }


    //void setTexels(int width, int height, int depth, std::vector<float>& texels);
    void setData(std::vector<float>* data);
    void setColors(std::vector<float>& colors);
    void setAlphas(std::vector<float>& alphas);

    void setColormap(const QImage& colormap);
    void setTracker(PSTracker* tracker = nullptr); /** Set tracker to pointer or to a new tracker object if not specified */
    void connectTracker();
    void setEyeOffset(float eyeOffset);
    void setCamDist(float camDist);
    void setSelectionMode(const int32_t& mode);
    void openCalib() const { refWidget->setTargetIndex(pluginInstanceIndex);  refWidget->show(); }

    void getIdlePose(QMatrix4x4& pose);

    PSTracker* getTracker() const { return _tracker; }

    void setFullScreenWidget(FullScreenWidget* widget);
    void setRenderDisplacement(const int& index, const int& indexMax);
    void setRenderDisplacement(const float&);
    void setDefaultRenderDisplacement();
    FullScreenWidget* getFullScreenWidget()const { return fullScreenWidget; };

    void setInstanceIndex(const int& index) { pluginInstanceIndex = index; }
    void setNumberPluginInstances(const int& value);
    //void setNumberInstances(const int& index) { numberInstances = index; }
    void toggleFullScreen();
    bool getIsFullScreen() const { return isFullScreen; };

    std::vector<float> getPointDistances() const { return pointDistances; };

    PedalManager* getPedalManager() const { return pedal; }
    void setPedalManager(PedalManager* pds);
    void setUpdateTimer(QTimer* tmr);
    QTimer* getUpdateTimer() const { return _updateTimer; }

    void adjustInterlacing();

    void startDepthsortWorker();

    void startFlashlightWorker();
    void stopFlashlightWorker();

    FlashlightWidget* getFlashlightWidget() const { return flashlightConfigWidget; }
    void initiateFlashlightWidget(FlashlightWidget* flw = nullptr);


public:
    bool eventFilter(QObject* target, QEvent* event);

    /**
    * Returns vector of position of the camera in space, calculated from the spherical coordinates
    */
    QVector3D getCamPos() const;

    void testReadyness();


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

    void flashlightReady();

    //void ready(PSTracker* tracker, FullScreenWidget* fsWidget, PedalManager* pedals, QTimer* updateTimer); // All shared variables are defined
    void ready(const OpenGLRendererWidget* emitter);
    void exitFullScreen();

private:
    VolumeRenderer _volumeRenderer;
    PSTracker* _tracker = nullptr;

    #ifdef CONTROLS
        ControlsWidget* _controls;
    #endif

    
    ReferenceSetupWidget* refWidget = nullptr;
    PedalManager* pedal;


    float _camStartDist = 1.0f;
    float idleRotationAngle = 0.0f;

    bool _isInitialized = false;

    // UI Controls
    sphericCoords viewPosSpheric;
    QPointF _previousMousePos;
    bool _mousePressed = false;

    bool _selecting = false;
    bool selectionReplaces = true;
    QElapsedTimer selectionInterval;

    SelectionMode selectionMode;

    QLabel* msgLabel;

    QTimer* _updateTimer = nullptr;

    float _pixelRatio = 1.0f; /** Current pixel ratio */

    int pluginInstanceIndex = -1;
    int numberPluginInstances = 0;
    float renderDisplacement = 0.0f;
    //int numberInstances = 0;

    FullScreenWidget* fullScreenWidget = nullptr;
    bool isFullScreen = false;

    QMatrix4x4 offset;

    std::vector<float>* points;
    QVector3D cursor;

    std::vector<std::vector<GLuint>> renderingOrders; // One for each eye
    std::vector<float> pointDistances; // vector to store point distances from cusor for flashlight effect
    std::vector<QMatrix4x4> cameraInModelRef; // One for each eye



    std::mutex mtx;


    DepthWorker* depthWorker = nullptr;
    FlashlightWorker* flashlightWorker = nullptr;
    FlashlightWidget* flashlightConfigWidget = nullptr;
};

