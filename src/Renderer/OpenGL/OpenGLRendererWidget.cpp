#include "OpenGLRendererWidget.h"

#include <QEvent>
#include <QMouseEvent>
#include <QWindow>
#include <QVBoxLayout>

#include <QMainWindow>
#include <algorithm>
#include <cmath>



OpenGLRendererWidget::OpenGLRendererWidget() :
    QOpenGLWidget(),
    _tracker(nullptr),
    renderingOrders(std::vector<std::vector<GLuint>>(2)),
    cameraInModelRef(std::vector<QMatrix4x4>(2)),
    msgLabel(new QLabel(this))
{
    // UI
    QVBoxLayout* layout = new QVBoxLayout(this);
    msgLabel->setAlignment(Qt::AlignBottom);
    msgLabel->setStyleSheet(QString::fromUtf8("color: rgb(235, 235, 235);"));
    layout->addWidget(msgLabel);
    setLayout(layout);


    setAcceptDrops(true);
    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    installEventFilter(this);


    connect(this, &OpenGLRendererWidget::created, this, [this]() {
        [[maybe_unused]] auto windowID = this->window()->winId(); // This is needed to produce a valid windowHandle on some systems

        QWindow* winHandle = windowHandle();

        // On some systems we might need to use a different windowHandle
        if (!winHandle)
        {
            const QWidget* nativeParent = nativeParentWidget();
            winHandle = nativeParent->windowHandle();
        }

        if (winHandle == nullptr)
        {
            qDebug() << "ScatterplotWidget: Not connecting updatePixelRatio - could not get window handle";
            return;
        }

        QObject::connect(winHandle, &QWindow::screenChanged, this, &OpenGLRendererWidget::updatePixelRatio, Qt::UniqueConnection);
    });

    #ifdef CONTROLS

    _controls = new ControlsWidget(nativeParentWidget());

    #endif

    refWidget = new ReferenceSetupWidget(this);

        
}

OpenGLRendererWidget::~OpenGLRendererWidget() {
    delete _tracker;
    if (depthWorker != nullptr) {
        depthWorker->terminate();
        depthWorker->wait();
    }
    stopFlashlightWorker();
    delete flashlightConfigWidget;
    delete pedal;
    delete refWidget;
}

//void OpenGLRendererWidget::setTexels(int width, int height, int depth, std::vector<float>& texels)
//{
//    makeCurrent();
//    _volumeRenderer.setTexels(width, height, depth, texels);
//}

void OpenGLRendererWidget::setData(std::vector<float>* data)
{

    // Testing : Cube point cloud
    /*data.clear();

    for (int i = 0; i <= 10; i++) {
        for (int j = 0; j <= 10; j++) {
            for (int k = 0; k <= 10; k++) {
                data.push_back(i / 10.f - 0.5f);
                data.push_back(j / 10.f - 0.5f);
                data.push_back(k / 10.f - 0.5f);
            }
        }
    }*/
    // Testing : Cube point cloud - END


    makeCurrent();
    _volumeRenderer.setData(data);

    points = data;

    for (int i = 0; i < 2; i++) {
        renderingOrders[i] = std::vector<GLuint>(data->size() / 3);

    }

    for (GLuint i = 0; i < renderingOrders[0].size(); i++) {
        renderingOrders[0][i] = i;
        renderingOrders[1][i] = i;
    }

    _volumeRenderer.setRenderOrder(0, renderingOrders[0]);
    _volumeRenderer.setRenderOrder(1, renderingOrders[1]);

    startDepthsortWorker();

    update();
}

void OpenGLRendererWidget::startDepthsortWorker() {

    depthWorker = new DepthWorker(this, &renderingOrders, points, &cameraInModelRef, &camActive, &mtx);
    connect(depthWorker, &DepthWorker::resultReady, this, [this](const int& i) {
        std::lock_guard<std::mutex> lock(mtx);

        _volumeRenderer.setRenderOrder(i, renderingOrders[i]);
        //// Test : colouring points by drawing order
        //std::vector<float> colors = std::vector<float>(points->size() / 3);
        //for (GLuint i : renderingOrders[0]) {
        //    colors[renderingOrders[0][i]] = i / float(renderingOrders[0].size());
        //} 
        //_volumeRenderer.setColors(colors);
        //// Test : colouring points by drawing order - END


        });
    depthWorker->start();


    /*workerThread.push_back(new WorkerThread(this, &indicesEye2, points, localCamPosEye2));
    connect(workerThread[workerThread.size() - 1], &WorkerThread::resultReady, this, [this]() {
        std::lock_guard<std::mutex> lock(mtx);

        _volumeRenderer.setRenderOrderEye2(indicesEye2);

        });
    workerThread[workerThread.size() - 1]->start();*/
}

void OpenGLRendererWidget::startFlashlightWorker() {
    stopFlashlightWorker();

    flashlightWorker = new FlashlightWorker(this, &pointDistances, &cursor, points, &mtx);
    connect(flashlightWorker, &FlashlightWorker::resultReady, this, [this]() {
        emit flashlightReady();

        });
    flashlightWorker->start();
}

void OpenGLRendererWidget::stopFlashlightWorker() {
    if (flashlightWorker != nullptr) {
        flashlightWorker->terminate();
        flashlightWorker->wait();
    }
    delete flashlightWorker;
    flashlightWorker = nullptr;
}


void OpenGLRendererWidget::initiateFlashlightWidget(FlashlightWidget* flw) {
    delete flashlightConfigWidget;
    flashlightConfigWidget = flw;
    if (flashlightConfigWidget == nullptr) flashlightConfigWidget = new FlashlightWidget(this);

    getVolumeRenderer().setFlashlightScalars(
        flashlightConfigWidget->getDistanceCoeficient(),
        flashlightConfigWidget->getMinTransparency()
    );

    connect(flashlightConfigWidget, &FlashlightWidget::valueChanged, this, [this]() {
        getVolumeRenderer().setFlashlightScalars(
            flashlightConfigWidget->getDistanceCoeficient(), 
            flashlightConfigWidget->getMinTransparency()
        );
    });
    testReadyness();

}


void OpenGLRendererWidget::setColors(std::vector<float>& colorsScalars)
{
    makeCurrent();
    _volumeRenderer.setColors(colorsScalars);
}

void OpenGLRendererWidget::setAlphas(std::vector<float>& scalars)
{
    makeCurrent();
    _volumeRenderer.setAlphas(scalars);
}


void OpenGLRendererWidget::setColormap(const QImage& colormap)
{
#ifdef CONTROLS
    _controls->setImageColorMap(colormap);
#endif
    _volumeRenderer.setColormap(colormap);
    qDebug() << "Width : " << colormap.width();
    qDebug() << "Height : " << colormap.height();
}


void OpenGLRendererWidget::setTracker(PSTracker* tracker)
{
    _tracker = tracker;
    refWidget->setTracker(_tracker);
    testReadyness();
}

void OpenGLRendererWidget::connectTracker()
{
    try {
        _tracker->Connect();
        _tracker->Start();
    }
    catch (const char* err) {
        qDebug() << "Failed connection";
        msgLabel->setText(err);
    }
}

void OpenGLRendererWidget::setEyeOffset(float eyeOffset)
{
    _volumeRenderer.setEyeOffset(eyeOffset);
}


void OpenGLRendererWidget::setSelectionMode(const int32_t& mode) { 
    selectionMode = static_cast<SelectionMode>(mode); 
    _volumeRenderer.setSelectionMode(mode);
}

void OpenGLRendererWidget::getIdlePose(QMatrix4x4& pose)
{

    idleRotationAngle += 0.1f;
    if (idleRotationAngle > 360) idleRotationAngle = idleRotationAngle - 360;
    pose.setToIdentity();
    pose.translate(-renderDisplacement, 0, 0);
    pose.rotate(idleRotationAngle, 0, 1, 0);

}

void OpenGLRendererWidget::setCamDist(float camDist)
{
    //Reinitialise position and set correct distance
    viewPosSpheric.distance = camDist;
    _volumeRenderer.setHeadPosition(getCamPos());
}

void OpenGLRendererWidget::setFullScreenWidget(FullScreenWidget* widget) { 
    fullScreenWidget = widget;
    connect(fullScreenWidget, &FullScreenWidget::numberChildrenChanged, this, [this]() {
        int index = fullScreenWidget->indexOf(this);
        if (isFullScreen && index == -1) { // When just added, the widget will not be in the fs widget yet, so putting it at the end
            index = fullScreenWidget->getCount()-1;
        }
        if (index > -1) { // If this instance is displayed in fullscreen widget
            setRenderDisplacement(index, fullScreenWidget->getCount() - 1);
        }
        else {
            setDefaultRenderDisplacement();
        }
    });
    testReadyness();
}

void OpenGLRendererWidget::setDefaultRenderDisplacement() {
    setRenderDisplacement(pluginInstanceIndex, numberPluginInstances - 1);
}

/// <summary>
/// Calculate and set horizontal displacement to use several plugin instances with several targets
/// </summary>
/// <param name="index"></param>
/// <param name="indexMax"></param>
void OpenGLRendererWidget::setRenderDisplacement(const int& index, const int& indexMax) {
    float displacement = index > -1 ? float(index) - indexMax / 2.f : 0.f;
    setRenderDisplacement(displacement / 0.5f);
}

void OpenGLRendererWidget::setRenderDisplacement(const float& dx) {
    offset.setToIdentity();
    offset.translate(dx , 0, 0);
    renderDisplacement = dx;
}


void OpenGLRendererWidget::setNumberPluginInstances(const int& value) { 
    numberPluginInstances = value; 
    setDefaultRenderDisplacement();
}

void OpenGLRendererWidget::toggleFullScreen() {
    if (isFullScreen) {
        isFullScreen = false;
        // Leave fullscreen
        emit exitFullScreen();
        msgLabel->setText("");
    } else {
        isFullScreen = true;
        // Go fullscreen
        fullScreenWidget->addWidget(this);
        msgLabel->setText("ESC - Escape full screen");
    }

    
    resetViewPos();

    _volumeRenderer.setStereo(screen()->model() == "D2343");
    camActive[1] = isFullScreen;
    
}

void OpenGLRendererWidget::resetViewPos()
{
    viewPosSpheric.azimuthal = 0;
    viewPosSpheric.polar = 90;
    viewPosSpheric.distance = 1;

    _volumeRenderer.setHeadPosition(getCamPos());
}



void OpenGLRendererWidget::setPedalManager(PedalManager* pds)
{
    pedal = pds;
    refWidget->setPedalManager(pedal);
    connect(pedal, &PedalManager::pedalPressed, this, [this](int value) {
        // Last pedal for selecting
        if (value == 2 && _volumeRenderer.getCursorFrozen()) {
            _selecting = true;
            emit newSelection(selectionMode, selectionReplaces);
        }
        else if (value == pluginInstanceIndex) {
            _volumeRenderer.freezeCursor();
            setFocus();
        }

    });


    connect(pedal, &PedalManager::pedalReleased, this, [this](int value) {
        // Last pedal for selecting
        if (value == 2) {
            _selecting = false;
        }
        else if (value == pluginInstanceIndex) {
            _volumeRenderer.unFreezeCursor();
        }

    });

    testReadyness();
}

void OpenGLRendererWidget::adjustInterlacing() {
    const int screenBottomCoordinate = mapToGlobal(QPointF(0, height())).y();
    getVolumeRenderer().setInterlacing((screenBottomCoordinate+1) % 2);
}

void OpenGLRendererWidget::initializeGL()
{

    initializeOpenGLFunctions();

    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &OpenGLRendererWidget::cleanup);
    qDebug() << "VolumeRendererWidget: InitializeGL";
    // Initialize renderers
    _volumeRenderer.init();
    qDebug() << "VolumeRendererWidget: InitializeGL Done";
    // OpenGL is initialized
    _isInitialized = true;

    setCamDist(_camStartDist);

    selectionInterval.start();

    

    // Every 3 seconds, recalculate the correct interlacing in case the window was moved
    QTimer* updateTimerLong = new QTimer(this);
    connect(updateTimerLong, &QTimer::timeout, this, [this]() { adjustInterlacing(); });
    updateTimerLong->start(3000);


}

void OpenGLRendererWidget::setUpdateTimer(QTimer* tmr) {
    _updateTimer = tmr;
    connect(_updateTimer, &QTimer::timeout, this, [this]() { update(); });
    testReadyness();
};



void OpenGLRendererWidget::resizeGL(int w, int h)
{
    _pixelRatio = devicePixelRatio();

    _volumeRenderer.resize(w * _pixelRatio, h * _pixelRatio);

    adjustInterlacing();
}

void OpenGLRendererWidget::paintGL()
{

    if (_selecting && selectionInterval.elapsed() > 1.f) {
        selectionInterval.restart();
        emit newSelection(selectionMode, selectionReplaces);
    }

    QMatrix4x4 pose;
    
#ifdef CONTROLS
    if (_controls->getCursorFrozen()) {
        if (!_volumeRenderer.getCursorFrozen()) {
            _volumeRenderer.freezeCursor();
        }
    }
    else
    {
        if (_volumeRenderer.getCursorFrozen()) {
            _volumeRenderer.unFreezeCursor();
        }
    }

    pose = _controls->getControlMatrix();

#else
    if ( pluginInstanceIndex > -1){ 
        bool pulledImage = false;
        if (_tracker != nullptr && _tracker->getTrackerConnected()) {
            pulledImage = _tracker->GetTargetMatrix(pluginInstanceIndex, pose);

            // Exagerrate translations to move more freely
            pose.data()[12] *= 10;
            pose.data()[13] *= 10;
            pose.data()[14] *= 10;
        }

        if(!pulledImage) {
            getIdlePose(pose); // automatic rotate
            //pose.setToIdentity(); // without rotate, might not needed
        }
    }
#endif

    std::unique_lock<std::mutex> lock(mtx);
    pose = offset * pose;
    QMatrix4x4 invertedPose = pose.inverted();

    std::vector<QMatrix4x4> views = _volumeRenderer.getViewMatrices();

    if (!_volumeRenderer.isStereo()) {
        cameraInModelRef[0] = invertedPose * views[0].inverted();
    }
    else {

        cameraInModelRef[0] = invertedPose * views[1].inverted();
        cameraInModelRef[1] = invertedPose * views[2].inverted();

    }

    lock.unlock();
#ifdef CONTROLS
    _volumeRenderer.render(defaultFramebufferObject(), true, pose);
#else
    if (_tracker != nullptr && pluginInstanceIndex > -1) {
        bool live = _tracker->getTrackerConnected() ? _tracker->poseIsLive(pluginInstanceIndex) : true;
        _volumeRenderer.render(defaultFramebufferObject(), live, pose);

    }
    
#endif
    if (getVolumeRenderer().getCursorFrozen()) {
        cursor = getVolumeRenderer().getCursor();
    }
}

void OpenGLRendererWidget::cleanup()
{
    _isInitialized = false;
#ifdef CONTROLS
    if(_controls != nullptr) delete _controls;
#endif
    delete _tracker;

    makeCurrent();
}

bool OpenGLRendererWidget::eventFilter(QObject* target, QEvent* event)
{
    switch (event->type())
    {
        case QEvent::KeyPress:
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            switch (keyEvent->key()) {
                case Qt::Key_F: {
                    if (!keyEvent->isAutoRepeat())
                        _volumeRenderer.freezeCursor();

                    return true;

                }
                case Qt::Key_Escape: {
                    if (isFullScreen)
                        toggleFullScreen();

                    return true;

                }

                case Qt::Key_Space: {
                    if (!keyEvent->isAutoRepeat())
                        if (_volumeRenderer.getCursorFrozen()) {
                            _selecting = true;
                            emit newSelection(selectionMode, selectionReplaces);
                        }

                    return true;

                }


                case Qt::Key_Shift: { // SHIFT (L and R)
                    if (!keyEvent->isAutoRepeat())
                        selectionReplaces = false;

                    return true;

                }


            }

      
            break;
        }
        case QEvent::KeyRelease:
        {
            QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
            switch (keyEvent->key()) {
                case Qt::Key_F:
                {
                    if (!keyEvent->isAutoRepeat())
                        _volumeRenderer.unFreezeCursor();
                    
                    return true;
                }

                case Qt::Key_Space: {
                    if (!keyEvent->isAutoRepeat())
                        _selecting = false;

                    return true;

                }

                case Qt::Key_Shift: { // SHIFT (L and R)
                    if (!keyEvent->isAutoRepeat())
                        selectionReplaces = true;
                        qDebug() << "No selecting";

                    return true;

                }

                case Qt::Key_R: {
                    if (!keyEvent->isAutoRepeat())
                    {
                        makeCurrent();
                        _volumeRenderer.reloadShader();
                        qDebug() << "Shaders reloaded";
                    }

                    return true;
                }
            }
            
            break;
        }
        case QEvent::Wheel:
        {
            auto wheelEvent = static_cast<QWheelEvent*>(event);

            QPoint numPixels = wheelEvent->pixelDelta();
            QPoint numDegrees = wheelEvent->angleDelta() / 8;
            float distance = 0.f;
            if (!numPixels.isNull()) {
                distance = numPixels.y();
            }
            else if (!numDegrees.isNull()) {
                distance = numDegrees.y() / 15.f;
            }

            if (_volumeRenderer.getCursorFrozen() && selectionMode == SelectionMode::Sphere) {
                _volumeRenderer.incrementSelectRadius(distance/100.f);
                return true;
            }
            
            float scaling = pow(2, -distance / 5.f);

            setCamDist(viewPosSpheric.distance* scaling);

            return true;
        }
        case QEvent::MouseButtonPress:
        {
            qDebug() << "Mouse press";
            auto mouseEvent = static_cast<QMouseEvent*>(event);

            QPointF mousePos = QPointF(mouseEvent->position().x(), mouseEvent->position().y());
            _previousMousePos = mousePos;

            _mousePressed = true;

            return true;
        }
        case QEvent::MouseMove:
        {
            if (!_mousePressed || isFullScreen)
                break;

            auto mouseEvent = static_cast<QMouseEvent*>(event);

            QPointF mousePos = QPointF(mouseEvent->position().x(), mouseEvent->position().y());

            QPointF diff = mousePos - _previousMousePos;


          

            viewPosSpheric.azimuthal -= diff.x();
            viewPosSpheric.polar -= diff.y();


            viewPosSpheric.polar = std::clamp<float>(viewPosSpheric.polar, 0.1f, 179.9f);

            _volumeRenderer.setHeadPosition(getCamPos());

            _previousMousePos = mousePos;

            return true;
     
        }
    }
    return QObject::eventFilter(target, event);
}

void OpenGLRendererWidget::updatePixelRatio()
{
    float pixelRatio = devicePixelRatio();

    // we only update if the ratio actually changed
    if (_pixelRatio != pixelRatio)
    {
        _pixelRatio = pixelRatio;
        resizeGL(width(), height());
        update();
    }
}

QVector3D OpenGLRendererWidget::getCamPos() const {

    QMatrix4x4 transform = QMatrix4x4();
    transform.setToIdentity();
    transform.scale(viewPosSpheric.distance);
    transform.rotate(90, 0, -1, 0);
    transform.rotate(viewPosSpheric.azimuthal, 0, 1, 0);
    transform.rotate(viewPosSpheric.polar, 0, 0, 1);

    return (transform * QVector4D(0, 1, 0, 1)).toVector3DAffine();
}

void OpenGLRendererWidget::testReadyness()
{
    if (_tracker != nullptr && fullScreenWidget != nullptr && pedal != nullptr && _updateTimer != nullptr && flashlightConfigWidget != nullptr) {
        emit ready(this);
    }
}
