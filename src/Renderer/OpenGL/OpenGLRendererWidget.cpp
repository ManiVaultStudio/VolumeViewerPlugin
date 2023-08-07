#include "OpenGLRendererWidget.h"

#include <QEvent>
#include <QMouseEvent>

OpenGLRendererWidget::OpenGLRendererWidget()
{
    setAcceptDrops(true);

    installEventFilter(this);
}

void OpenGLRendererWidget::setTexels(int width, int height, int depth, std::vector<float>& texels)
{
    makeCurrent();
    _volumeRenderer.setTexels(width, height, depth, texels);
}

void OpenGLRendererWidget::setData(std::vector<float>& data)
{
    makeCurrent();
    _volumeRenderer.setData(data);
}

void OpenGLRendererWidget::setColors(std::vector<float>& colors)
{
    makeCurrent();
    _volumeRenderer.setColors(colors);
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
}

void OpenGLRendererWidget::resizeGL(int w, int h)
{
    //_windowSize.setWidth(w);
    //_windowSize.setHeight(h);

    //_pointRenderer.resize(QSize(w, h));
    //_densityRenderer.resize(QSize(w, h));

    //// Set matrix for normalizing from pixel coordinates to [0, 1]
    //toNormalisedCoordinates = Matrix3f(1.0f / w, 0, 0, 1.0f / h, 0, 0);

    //// Take the smallest dimensions in order to calculate the aspect ratio
    //int size = w < h ? w : h;

    //float wAspect = (float)w / size;
    //float hAspect = (float)h / size;
    //float wDiff = ((wAspect - 1) / 2.0);
    //float hDiff = ((hAspect - 1) / 2.0);

    //toIsotropicCoordinates = Matrix3f(wAspect, 0, 0, hAspect, -wDiff, -hDiff);
}

void OpenGLRendererWidget::paintGL()
{
    int w = width();
    int h = height();

    float aspect = (float)w / h;

    _volumeRenderer.render(_camPos, _camAngle, aspect);
}

void OpenGLRendererWidget::cleanup()
{
    _isInitialized = false;

    makeCurrent();
}

bool OpenGLRendererWidget::eventFilter(QObject* target, QEvent* event)
{
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    {
        auto mouseEvent = static_cast<QMouseEvent*>(event);

        QPointF mousePos = QPointF(mouseEvent->position().x(), mouseEvent->position().y());
        _previousMousePos = mousePos;

        _mousePressed = true;
        break;
    }
    case QEvent::MouseMove:
    {
        if (!_mousePressed)
            break;

        auto mouseEvent = static_cast<QMouseEvent*>(event);

        QPointF mousePos = QPointF(mouseEvent->position().x(), mouseEvent->position().y());

        QPointF diff = mousePos - _previousMousePos;

        _camAngle.y += diff.x() * 0.01f;
        _camAngle.x -= diff.y() * 0.01f;
        if (_camAngle.x > 3.14150) _camAngle.x = 3.14150;
        if (_camAngle.x < 0.0001) _camAngle.x = 0.0001;

        _camPos.x = _camDist * sin(_camAngle.x) * cos(_camAngle.y);
        _camPos.y = _camDist * cos(_camAngle.x);
        _camPos.z = _camDist * sin(_camAngle.x) * sin(_camAngle.y);

        update();

        _previousMousePos = mousePos;

        break;
    }
    }
    return QObject::eventFilter(target, event);
}
