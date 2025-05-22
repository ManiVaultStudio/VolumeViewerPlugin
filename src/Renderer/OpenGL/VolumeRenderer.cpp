#include "VolumeRenderer.h"

#include <QImage>

#include <random>

#include <QMatrix4x4>

//#define CUBE


void Cube::create()
{
    initializeOpenGLFunctions();

    std::vector<mv::Vector3f> vertices;

    vertices.emplace_back(-0.5f, -0.5f, 0.5f);
    vertices.emplace_back(0.5f, -0.5f, 0.5f);
    vertices.emplace_back(-0.5f, 0.5f, 0.5f);
    vertices.emplace_back(0.5f, 0.5f, 0.5f);
    vertices.emplace_back(-0.5f, -0.5f, -0.5f);
    vertices.emplace_back(0.5f, -0.5f, -0.5f);
    vertices.emplace_back(-0.5f, 0.5f, -0.5f);
    vertices.emplace_back(0.5f, 0.5f, -0.5f);

    std::vector<mv::Vector3f> normals;

    normals.emplace_back(0, 0, 1);
    normals.emplace_back(1, 0, 0);
    normals.emplace_back(0, 0, -1);
    normals.emplace_back(-1, 0, 0);
    normals.emplace_back(0, 1, 0);
    normals.emplace_back(0, -1, 0);

    std::vector<int> indices;

    indices.push_back(0); indices.push_back(1); indices.push_back(2); indices.push_back(2); indices.push_back(1); indices.push_back(3);
    indices.push_back(1); indices.push_back(5); indices.push_back(3); indices.push_back(3); indices.push_back(5); indices.push_back(7);
    indices.push_back(5); indices.push_back(4); indices.push_back(7); indices.push_back(7); indices.push_back(4); indices.push_back(6);
    indices.push_back(4); indices.push_back(0); indices.push_back(6); indices.push_back(6); indices.push_back(0); indices.push_back(2);
    indices.push_back(2); indices.push_back(3); indices.push_back(6); indices.push_back(6); indices.push_back(3); indices.push_back(7);
    indices.push_back(5); indices.push_back(4); indices.push_back(1); indices.push_back(1); indices.push_back(4); indices.push_back(0);

    std::vector<mv::Vector3f> aVertices;
    std::vector<mv::Vector3f> aNormals;
    for (int i = 0; i < indices.size(); i++)
    {
        aVertices.push_back(vertices[indices[i]]);
        aNormals.push_back(normals[i / 6]);
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, aVertices.size() * sizeof(mv::Vector3f), aVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &nbo);
    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, aNormals.size() * sizeof(mv::Vector3f), aNormals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
    glEnableVertexAttribArray(1);
}



void VolumeRenderer::setData(std::vector<float>& data)
{
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    
    glGenBuffers(1, &cbo);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() / 3 * sizeof(float), nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);


    glGenBuffers(1, &highlightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() / 3 * sizeof(int), nullptr, GL_STATIC_DRAW);
    glVertexAttribIPointer(2, 1, GL_INT, 0, nullptr);
    glEnableVertexAttribArray(2);
   

    _numPoints = data.size() / 3;
}

void VolumeRenderer::setColors(std::vector<float>& colors)
{

    glBindVertexArray(vao);
    qDebug() << colors.size();
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);

    _hasColors = true;
}

void VolumeRenderer::setHighlights(std::vector<int>& highlights) {
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
    glBufferData(GL_ARRAY_BUFFER, highlights.size() * sizeof(int), highlights.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
}

void VolumeRenderer::setColormap(const QImage& colormap)
{
    _colormap.loadFromImage(colormap);
    //cMapSize = colormap.size();
    qDebug() << "Colormap is set!";
}

void VolumeRenderer::freezeCursor() {
    _frozenCursorPosition = _modelMatrix * _cursorPosition;
    cursorFrozen = true;
};

void VolumeRenderer::unFreezeCursor() {
    _cursorPosition = _modelMatrix.inverted() * _frozenCursorPosition;
    cursorFrozen = false;
};

/**
* Get cursor position in the coordinate system of the data
*/
QVector3D VolumeRenderer::getCursor() const {
    if (cursorFrozen) return (_modelMatrix.inverted() * _frozenCursorPosition).toVector3D();
    return _cursorPosition.toVector3D();
};


void VolumeRenderer::reloadShader()
{
    _pointsShaderProgram.loadShaderFromFile(":shaders/points.vert", ":shaders/VolumeDraw.frag");

    qDebug() << "Shaders reloaded";
}

void VolumeRenderer::init()
{
    identity.setToIdentity();
    initializeOpenGLFunctions();
    
    glClearColor(40.f / 255.0f, 40.f / 255.0f, 40.f / 255.0f, 1.0f);

    // Make float buffer to support low alpha blending
    _colorAttachment.create();
    _colorAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _framebuffer.create();
    _framebuffer.bind();
    _framebuffer.addColorTexture(0, &_colorAttachment);
    _framebuffer.validate();

    // Make float buffer to support low alpha blending
    _leftColorAttachment.create();
    _leftColorAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _leftDepthAttachment.create();
    _leftDepthAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _leftRenderFBO.create();
    _leftRenderFBO.bind();
    _leftRenderFBO.addColorTexture(0, &_leftColorAttachment);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _leftDepthAttachment.getHandle(), 0);
    _leftRenderFBO.validate();

    // Make float buffer to support low alpha blending
    _rightColorAttachment.create();
    _rightColorAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _rightDepthAttachment.create();
    _rightDepthAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _rightRenderFBO.create();
    _rightRenderFBO.bind();
    _rightRenderFBO.addColorTexture(0, &_rightColorAttachment);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, _rightDepthAttachment.getHandle(), 0);
    _rightRenderFBO.validate();

    bool loaded = true;
    loaded &= _pointsShaderProgram.loadShaderFromFile(":shaders/points.vert", ":shaders/VolumeDraw.frag");
    loaded &= _cubeShaderProgram.loadShaderFromFile(":shaders/CubeDraw.vert", ":shaders/CubeDraw.frag");
    loaded &= _framebufferShaderProgram.loadShaderFromFile(":shaders/QuadPST.vert", ":shaders/TexturePST.frag");
    loaded &= _stereoMergeProgram.loadShaderFromFile(":shaders/QuadPST.vert", ":shaders/StereoMerge.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the Volume Renderer shaders";
    }


    glGenVertexArrays(1, &vao);

    qDebug() << "Initialized volume renderer";

    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    glPointSize(3);

    glGenVertexArrays(1, &_cursorVao);
    glBindVertexArray(_cursorVao);

    glGenBuffers(1, &_cursorVbo);
    glBindBuffer(GL_ARRAY_BUFFER, _cursorVbo);
    glBufferData(GL_ARRAY_BUFFER, 0 * sizeof(float), nullptr, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    _cube.create();
}

void VolumeRenderer::resize(int w, int h)
{
    qDebug() << "Resize called";
    _colorAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);

    _leftColorAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);

    _rightColorAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);

    _leftDepthAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    _rightDepthAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glViewport(0, 0, w, h);
}

void VolumeRenderer::render(GLuint framebuffer, mv::Vector3f camPos, float aspect, const bool& live, const QMatrix4x4& modelFrameMatrix)
{
    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    #ifdef VOLUME

        _volumeShaderProgram.bind();

        glActiveTexture(GL_TEXTURE0);
        _volumeShaderProgram.uniform1i("tex", 0);

        glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    #else


    _projMatrix.setToIdentity();
    float fovyr = 1.0472;// 1.57079633;
    float zNear = 0.1f;
    float zFar = 100;
    _projMatrix.data()[0] = (float)(1 / tan(fovyr / 2)) / aspect;
    _projMatrix.data()[5] = (float)(1 / tan(fovyr / 2));
    _projMatrix.data()[10] = (zNear + zFar) / (zNear - zFar);
    _projMatrix.data()[11] = -1;
    _projMatrix.data()[14] = (2 * zNear * zFar) / (zNear - zFar);
    _projMatrix.data()[15] = 0;

    _modelMatrix = modelFrameMatrix;

    // Exagerrate translations to move more freely
    _modelMatrix.data()[12] *= 10;
    _modelMatrix.data()[13] *= 10;
    _modelMatrix.data()[14] *= 10;
  
    _pointsShaderProgram.bind();
    _pointsShaderProgram.uniform1i("distanceEffect", cursorFrozen);

    _pointsShaderProgram.uniform3f("cursor", _frozenCursorPosition[0], _frozenCursorPosition[1], _frozenCursorPosition[2]);


    #ifndef STEREO
        _viewMatrix.setToIdentity();
        _viewMatrix.lookAt(QVector3D(camPos.x, camPos.y, camPos.z), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
        _framebuffer.bind();
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        #ifdef CUBE
            drawCube(_pointsShaderProgram);
        #else
            drawVolume(_pointsShaderProgram, live);
        #endif
    #else
        QVector3D viewPoint = QVector3D(camPos.x, camPos.y, camPos.z);
        QVector3D offsetDir = QVector3D::crossProduct(
            viewPoint,
            QVector3D(0,1,0)
        );


        _viewMatrix.setToIdentity();
        _viewMatrix.lookAt(viewPoint + offsetDir * _eyeOffset, QVector3D(0, 0, 0), QVector3D(0, 1, 0));
        _leftRenderFBO.bind();
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        #ifdef CUBE
            drawCube(_pointsShaderProgram);
        #else
            drawVolume(_pointsShaderProgram, live);
        #endif


        _viewMatrix.setToIdentity();
        _viewMatrix.lookAt(viewPoint - offsetDir * _eyeOffset, QVector3D(0, 0, 0), QVector3D(0, 1, 0));
        _rightRenderFBO.bind();
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        #ifdef CUBE
            drawCube(_pointsShaderProgram);
        #else
            drawVolume(_pointsShaderProgram, live);
        #endif


        // If stereo rendering is on, combine both left and right textures
        glDisable(GL_BLEND);


        _framebuffer.bind();
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glClear(GL_COLOR_BUFFER_BIT);

        _stereoMergeProgram.bind();
        _leftColorAttachment.bind(0);
        _rightColorAttachment.bind(1);
        _stereoMergeProgram.uniform1i("leftImage", 0);
        _stereoMergeProgram.uniform1i("rightImage", 1);
        _stereoMergeProgram.uniform1i("interlacing", _interlacing);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    #endif


   


    // Draw the cursor
    mv::Vector3f cursorPosition;
    _pointsShaderProgram.bind();
    _pointsShaderProgram.uniform1i("isCursor", 1);
    glBindVertexArray(_cursorVao);
    glBindBuffer(GL_ARRAY_BUFFER, _cursorVbo);
    if (cursorFrozen) {
        cursorPosition = mv::Vector3f(_frozenCursorPosition[0], _frozenCursorPosition[1], _frozenCursorPosition[2]);
        _pointsShaderProgram.uniformMatrix4f("modelMatrix", identity.data());
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), &cursorPosition, GL_STATIC_DRAW);

    }
    else {
        cursorPosition = mv::Vector3f(_cursorPosition[0], _cursorPosition[1], _cursorPosition[2]);
        _pointsShaderProgram.uniformMatrix4f("modelMatrix", _modelMatrix.data());
        glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), &cursorPosition, GL_STATIC_DRAW);
    }

    glEnable(GL_POINT_SMOOTH);
    glPointSize(10);
    glDrawArrays(GL_POINTS, 0, 1);
    _pointsShaderProgram.uniform1i("isCursor", 0);
    glDisable(GL_POINT_SMOOTH);
 



    ///////////////////////////////////////////////////////////////////////
    // Draw the color framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glClear(GL_COLOR_BUFFER_BIT);

    _framebufferShaderProgram.bind();

    _colorAttachment.bind(0);



    _framebufferShaderProgram.uniform1i("tex", 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

#endif
    {
       GLenum error = glGetError();
        if (error != GL_NO_ERROR)
        {
            std::cout << "Error: " << error << std::endl;
        }
    }

}

void VolumeRenderer::drawCube(mv::ShaderProgram& shader)
{
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    shader.uniformMatrix4f("projMatrix", _projMatrix.data());
    shader.uniformMatrix4f("viewMatrix", _viewMatrix.data());
    shader.uniformMatrix4f("modelMatrix", _modelMatrix.data()); // For the remote work, use identity matrix instead of the tracker's

    glBindVertexArray(_cube.vao);

    glDrawArrays(GL_TRIANGLES, 0, _cube.numVerts);

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}

void VolumeRenderer::drawVolume(mv::ShaderProgram& shader, const bool& live)
{
    if (_numPoints > 0) {
        GLenum error = glGetError();
        glClear(GL_COLOR_BUFFER_BIT);
        shader.uniformMatrix4f("projMatrix", _projMatrix.data());
        shader.uniformMatrix4f("viewMatrix", _viewMatrix.data());
        shader.uniformMatrix4f("modelMatrix", _modelMatrix.data());
    
        glPointSize(3);
        glBindVertexArray(vao);
        shader.uniform1i("hasColors", false);
        shader.uniform3f("selectionColor", _selectionColor.redF(), _selectionColor.greenF(), _selectionColor.blueF());
        shader.uniform1i("live", live);

        if (_hasColors)
        {
            shader.uniform1i("hasColors", true);
            if (_colormap.isCreated())
            {
                _colormap.bind(0);
                shader.uniform1i("colormap", 0);
                //shader.uniform2f("mapSize", cMapSize.width(), cMapSize.height());
            }
        }

        glDrawArrays(GL_POINTS, 0, _numPoints);
    
    }
}
