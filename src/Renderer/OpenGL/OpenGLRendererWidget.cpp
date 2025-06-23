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
    pedal = new PedalManager();

    refWidget = new ReferenceSetupWidget(this, pedal);


    connect(pedal, &PedalManager::pedalPressed, this, [this](int value) {
        qDebug() << "Pd " << value;
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
            qDebug() << "Release ";
            _selecting = false;
        }
        else if (value == pluginInstanceIndex) {
            _volumeRenderer.unFreezeCursor();
        }

    });


        
}

OpenGLRendererWidget::~OpenGLRendererWidget() {
    delete _tracker;
}

//void OpenGLRendererWidget::setTexels(int width, int height, int depth, std::vector<float>& texels)
//{
//    makeCurrent();
//    _volumeRenderer.setTexels(width, height, depth, texels);
//}

void OpenGLRendererWidget::setData(std::vector<float>& data)
{
    makeCurrent();
    _volumeRenderer.setData(data);
    update();
}

void OpenGLRendererWidget::setColors(std::vector<float>& colors)
{
    makeCurrent(); 
    _volumeRenderer.setColors(colors);
}

void OpenGLRendererWidget::setColormap(const QImage& colormap)
{
#ifdef CONTROLS
    _controls->setImageColorMap(colormap);
#endif
    _volumeRenderer.setColormap(colormap);
}

void OpenGLRendererWidget::setTracker(PSTracker* tracker)
{
    _tracker = tracker;
}

void OpenGLRendererWidget::connectTracker()
{
    if(_tracker == nullptr) _tracker = new PSTracker();
    try {
        _tracker->initPST();
        _tracker->Connect();
        refWidget->setTracker(_tracker);
        msgLabel->setText("Connected to tracker");
        emit hasTracker(_tracker);
    }
    catch (const char* err) {
        msgLabel->setText(err);
    }

}

void OpenGLRendererWidget::requestTracker() {
    if (_tracker == nullptr) connectTracker();
    else emit hasTracker(_tracker);
}

void OpenGLRendererWidget::setEyeOffset(float eyeOffset)
{
    _volumeRenderer.setEyeOffset(eyeOffset);
}


void OpenGLRendererWidget::setSelectionMode(const int32_t& mode) { 
    selectionMode = static_cast<SelectionMode>(mode); 
    _volumeRenderer.setSelectionMode(mode);
}

void OpenGLRendererWidget::setCamDist(float camDist)
{
    //Reinitialise position and set correct distance
    viewPosSpheric.distance = camDist;
}

void OpenGLRendererWidget::setFullScreenWidget(FullScreenWidget* widget) { 
    fullScreenWidget = widget;
    connect(fullScreenWidget, &FullScreenWidget::numberChildrenChanged, this, [this]() {
        int index = fullScreenWidget->indexOf(this);
        if (isFullScreen && index == -1) {
            // Strangely, indexOf deosn't find the widget when it was just added. This is a hack
            index = fullScreenWidget->getCount()-1;
        }
        // If the widget is not full screen, it's displacement will be 0
        float displacement = index > -1 ? float(index) - (fullScreenWidget->getCount()-1) / 2.f : 0.f;
        qDebug() << "instance " << pluginInstanceIndex << ", index " << index << ", dispalcement : " << displacement;
        offset.setToIdentity();
        offset.translate(displacement/5.f, 0, 0);
    });
}

void OpenGLRendererWidget::toggleFullScreen() {
    if (isFullScreen) {
        isFullScreen = false;
        // Leave fullscreen
        emit exitFullScreen();
        msgLabel->setText("");
        offset.setToIdentity();
    } else {
        isFullScreen = true;
        // Go fullscreen
        fullScreenWidget->addWidget(this);
        msgLabel->setText("ESC - Escape full screen");
    }

    _volumeRenderer.setStereo(screen()->model() == "D2343");
    
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

    _updateTimer = new QTimer(this);
    connect(_updateTimer, &QTimer::timeout, this, [this]() { update(); });
    _updateTimer->start(16);


    // Every 3 seconds, recalculate the correct interlacing in case the window was moved
    QTimer* updateTimerLong = new QTimer(this);
    connect(updateTimerLong, &QTimer::timeout, this, [this]() { adjustInterlacing(); });
    updateTimerLong->start(3000);

}

void OpenGLRendererWidget::resizeGL(int w, int h)
{
    _pixelRatio = devicePixelRatio();

    _volumeRenderer.resize(w * _pixelRatio, h * _pixelRatio);

    adjustInterlacing();
}

void OpenGLRendererWidget::paintGL()
{

    int w = width();
    int h = height();

    float aspect = (float)w / h;


    if (_selecting && selectionInterval.elapsed() > 1.f) {
        selectionInterval.restart();
        emit newSelection(selectionMode, selectionReplaces);
    }

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
    _volumeRenderer.render(defaultFramebufferObject(), getCamPos(), mf::Vector3f(0.f,0.f,0.f), aspect, true, _controls->getControlMatrix());
#else
    QMatrix4x4 pose;
    if (_tracker != nullptr && _tracker->getTrackerConnected() && pluginInstanceIndex > -1 && _tracker->GetTargetMatrix(pluginInstanceIndex, pose)) {
        pose = offset * pose;
        _volumeRenderer.render(defaultFramebufferObject(), getCamPos(), aspect, _tracker->poseIsLive(pluginInstanceIndex), pose);
    }
#endif
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

            viewPosSpheric.distance *= scaling;

            return true;
        }
        /*case QEvent::MouseButtonPress:
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
            if (!_mousePressed)
                break;

            auto mouseEvent = static_cast<QMouseEvent*>(event);

            QPointF mousePos = QPointF(mouseEvent->position().x(), mouseEvent->position().y());

            QPointF diff = mousePos - _previousMousePos;


          

            viewPosSpheric.azimuthal -= diff.x();
            viewPosSpheric.polar -= diff.y();


            viewPosSpheric.polar = std::clamp<float>(viewPosSpheric.polar, 0.1f, 179.9f);


            update();

            _previousMousePos = mousePos;

            return true;
     
        }*/
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

    return (transform * QVector4D(0, 1, 0, 1)).toVector3D();
}
