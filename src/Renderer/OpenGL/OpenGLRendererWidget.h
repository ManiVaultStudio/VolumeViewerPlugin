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

#include "ReferenceSetup.h"
#include <Controllers/Pedal.h>
#include <Widgets/FullScreenWidget.h>

#include "VolumeRenderer.h"
#include "Controllers/Tracker.h"

#include "graphics/Vector3f.h"
#include "graphics/Vector2f.h"

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

class WorkerThread : public QThread
{
    Q_OBJECT
public:
    explicit WorkerThread(
        QObject* parent = nullptr, 
        std::vector<GLuint>* inds = nullptr,
        std::vector<float> pts = std::vector<float>(0), 
        QVector3D* cam = nullptr
    ) : QThread(parent) {
        indices = inds;
        points = pts;
        // Camera position relative to the cloud of points
        camPos = cam;
    }
protected:
    std::vector<float> points;
    std::vector<GLuint>* indices;
    QVector3D* camPos;
    void run() override {
        QString result;
        QVector3D camLastPos;
        for (int i = 0; i < indices->size(); i++) indices->at(i) = i;

        while (true) {
            if (*camPos != camLastPos) {
                // 3. Each frame, sort indices based on distance to camera:
                std::sort(indices->begin(), indices->end(), [&](const int& a, const int& b) {
                    return (
                        distance(*camPos, QVector3D(points[a * 3], points[a * 3 + 1], points[a * 3 + 2]))
                        > distance(*camPos, QVector3D(points[b * 3], points[b * 3 + 1], points[b * 3 + 2]))
                        );
                    });

                

                camLastPos = *camPos;
                emit resultReady();
            }
        }
    }
    float distance(const QVector3D& a, const QVector3D& b) {
        float dx = a[0] - b[0];
        float dy = a[1] - b[1];
        float dz = a[2] - b[2];
        return sqrtf(dx * dx + dy * dy + dz * dz);
    }
signals:
    void resultReady();
};




class OpenGLRendererWidget : public QOpenGLWidget, QOpenGLFunctions_4_2_Core
{
    Q_OBJECT

public:
    OpenGLRendererWidget();
    ~OpenGLRendererWidget();


    VolumeRenderer& getVolumeRenderer() { return _volumeRenderer; }


    //void setTexels(int width, int height, int depth, std::vector<float>& texels);
    void setData(std::vector<float>& data);
    void setColors(std::vector<float>& colors);
    void setColormap(const QImage& colormap);
    void setTracker(PSTracker* tracker = nullptr); /** Set tracker to pointer or to a new tracker object if not specified */
    void setEyeOffset(float eyeOffset);
    void setCamDist(float camDist);
    void setSelectionMode(const int32_t& mode);
    void openCalib() const { refWidget->setTargetIndex(pluginInstanceIndex);  refWidget->show(); }

    void requestTracker();
    PSTracker* getTracker() const { return _tracker; }

    void setFullScreenWidget(FullScreenWidget* widget);
    FullScreenWidget* getFullScreenWidget()const { return fullScreenWidget; };

    void setInstanceIndex(const int& index) { pluginInstanceIndex = index; }
    //void setNumberInstances(const int& index) { numberInstances = index; }
    void toggleFullScreen();
    bool getIsFullScreen() const { return isFullScreen; };
    PedalManager* getPedalManager() const { return pedal; }
    void setPedalManager(PedalManager* pds);
    void setUpdateTimer(QTimer* tmr);
    QTimer* getUpdateTimer() const { return _updateTimer; }

    void adjustInterlacing();

    void startThreading() {
        
        workerThread = new WorkerThread(this, &indices, points, localCamPos);
        connect(workerThread, &WorkerThread::resultReady, this, [this]() {
            _volumeRenderer.setRenderOrder(indices);
        });
        workerThread->start();
    }

public:
    bool eventFilter(QObject* target, QEvent* event);

    /**
    * Returns vector of position of the camera in space, calculated from the spherical coordinates
    */
    QVector3D getCamPos() const;

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

    void connectTracker();

private slots:
    void updatePixelRatio();

signals:
    void created();
    void newSelection(const SelectionMode& type, const bool& replace);

    void hasTracker(PSTracker* tracker);
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
    //int numberInstances = 0;

    FullScreenWidget* fullScreenWidget = nullptr;
    bool isFullScreen = false;

    QMatrix4x4 offset;

    std::vector<GLuint> indices;
    std::vector<float> points;

    WorkerThread* workerThread;
    QVector3D* localCamPos;

};

