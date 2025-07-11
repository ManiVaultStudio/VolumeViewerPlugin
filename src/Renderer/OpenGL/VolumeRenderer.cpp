#include "VolumeRenderer.h"

#include <QImage>

#include <random>

#include <QMatrix4x4>
#include <cstdlib>



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
    points = data;

    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);
    
    glGenBuffers(1, &cbo);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() / 3 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);


    glGenBuffers(1, &highlightVBO);
    glBindBuffer(GL_ARRAY_BUFFER, highlightVBO);
    glBufferData(GL_ARRAY_BUFFER, data.size() / 3 * sizeof(int), nullptr, GL_STATIC_DRAW);
    glVertexAttribIPointer(2, 1, GL_INT, 0, nullptr);
    glEnableVertexAttribArray(2);



    glGenBuffers(1, &ebo[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() / 3 * sizeof(int), nullptr, GL_DYNAMIC_DRAW);

    glGenBuffers(1, &ebo[1]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.size() / 3 * sizeof(int), nullptr, GL_DYNAMIC_DRAW);

    //filterPoints(2.f);
   

    _numPoints = data.size() / 3;
}


//void VolumeRenderer::filterPoints(const float& proba) {
//    _numPoints = 0;
//    rendered = std::vector<bool>(points.size(), false);
//    std::vector<GLuint> indices;
//    for (int i = 0; i < points.size(); i++) {
//        if (rand() % 100 < proba*100) {
//            rendered[i] = true;
//            indices.push_back(i);
//            _numPoints++;
//        }
//    }
//
//    glBindVertexArray(vao);
//    // 4. Upload sorted indices to EBO:
//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);
//};

void VolumeRenderer::setColors(std::vector<float>& colors)
{

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(float), colors.data(), GL_DYNAMIC_DRAW);
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


void VolumeRenderer::setPointOpacity(const float& value) {
    pointOpacity = value;
};


void VolumeRenderer::setRenderOrder(const int& eye, std::vector<GLuint>& indices) {
    qDebug() << "render order chamged";
    glBindVertexArray(vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[eye]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indices.size(), indices.data(), GL_DYNAMIC_DRAW);

    /*std::vector<float> colors = std::vector<float>(indices.size(), 0.f);
    for (int i = 0; i < indices.size(); i++) {
        colors[indices[i]] = float(i) / float(indices.size());
    }
    setcolors(colors);*/
}



std::vector<GLuint> VolumeRenderer::getRenderedPoints() {
    std::vector <GLuint> indices;
    for (int i = 0; i < rendered.size(); i++) {
        if (rendered[i]) {
            indices.push_back(i);
        }
    }
    return indices;
};

void VolumeRenderer::freezeCursor() {
    if (!cursorFrozen) {
        _frozenCursorPosition = _modelMatrix * _cursorPosition;
        cursorFrozen = true;
    }
};

void VolumeRenderer::unFreezeCursor() {
    if (cursorFrozen) {
        _cursorPosition = _modelMatrix.inverted() * _frozenCursorPosition;
        cursorFrozen = false;
    }
};

void VolumeRenderer::setHeadPosition(const QVector3D& headPos) { 
    headPosition = headPos;


    QVector3D offsetDir = QVector3D::crossProduct(headPosition, QVector3D(0, 1, 0));

    stereoCameras[0] = headPosition - offsetDir * _eyeDistance;

    stereoCameras[1] = headPosition + offsetDir * _eyeDistance;
}

QVector3D VolumeRenderer::getStereoCamera(const int& eye) const
{
    return stereoCameras[eye];
}

/**
* Get cursor position in the coordinate system of the data
*/
QVector3D VolumeRenderer::getCursor() const {
    if (cursorFrozen) return (_modelMatrix.inverted() * _frozenCursorPosition).toVector3DAffine();
    return _cursorPosition.toVector3DAffine();
};

void VolumeRenderer::incrementSelectRadius(const float& increment) 
{ 
    sphereSelectRadius += increment; 
    sphereSelectRadius = std::max(sphereSelectRadius, 0.0f);
}


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

    // Add depth texture to enable z buffering and testing
    _depthAttachment.create();
    _depthAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, 1, 1, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    _framebuffer.create();
    _framebuffer.bind();
    _framebuffer.addColorTexture(0, &_colorAttachment);
    _framebuffer.setTexture(GL_DEPTH_ATTACHMENT, _depthAttachment);
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

    _depthAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, w, h, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

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

void VolumeRenderer::render(GLuint framebuffer, float aspect, const bool& live, const QMatrix4x4& modelFrameMatrix)
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
    float zNear = 0.4f;
    float zFar = 100;
    _projMatrix.data()[0] = (float)(1 / tan(fovyr / 2)) / aspect;
    _projMatrix.data()[5] = (float)(1 / tan(fovyr / 2));
    _projMatrix.data()[10] = (zNear + zFar) / (zNear - zFar);
    _projMatrix.data()[11] = -1;
    _projMatrix.data()[14] = (2 * zNear * zFar) / (zNear - zFar);
    _projMatrix.data()[15] = 0;

    float fovy = 60; // degrees
    int viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    heightOfNearPlane = (float) abs(viewport[3] - viewport[1]) / (2 * tan(fovyr));

    _modelMatrix = modelFrameMatrix;

  
    _pointsShaderProgram.bind();

    _pointsShaderProgram.uniform4f("cursor", _frozenCursorPosition[0], _frozenCursorPosition[1], _frozenCursorPosition[2], _frozenCursorPosition[3]);



    

    if(!stereo){
        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
        _viewMatrix.setToIdentity();
        _viewMatrix.lookAt(headPosition, QVector3D(0, 0, 0), QVector3D(0, 1, 0));
        _framebuffer.bind();
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        #ifdef CUBE
            drawCube(_pointsShaderProgram);
        #else

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            drawCursor();
            drawVolume(_pointsShaderProgram, _viewMatrix, live);
        #endif
   

    }
    else {

        QMatrix4x4 singleCamRef = QMatrix4x4();
        singleCamRef.setToIdentity();
        singleCamRef.lookAt(headPosition, QVector3D(0, 0, 0), QVector3D(0, 1, 0));


        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[0]);
        _viewMatrix.setToIdentity();
        _viewMatrix.lookAt(stereoCameras[0], QVector3D(0, 0, 0), QVector3D(0, 1, 0));
        _leftRenderFBO.bind();
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
#ifdef CUBE
        drawCube(_pointsShaderProgram);
#else

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawCursor();
        drawVolume(_pointsShaderProgram, singleCamRef, live);
#endif


        glBindVertexArray(vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo[1]);
        _viewMatrix.setToIdentity();
        _viewMatrix.lookAt(stereoCameras[1], QVector3D(0, 0, 0), QVector3D(0, 1, 0));
        _rightRenderFBO.bind();
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
#ifdef CUBE
        drawCube(_pointsShaderProgram);
#else

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawCursor();
        drawVolume(_pointsShaderProgram, singleCamRef, live);
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

        glEnable(GL_BLEND);

    }


   


    



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

void VolumeRenderer::drawCursor()
{

    _pointsShaderProgram.uniformMatrix4f("projMatrix", _projMatrix.data());
    _pointsShaderProgram.uniformMatrix4f("viewMatrix", _viewMatrix.data());

    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

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

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}

void VolumeRenderer::drawCube(mv::ShaderProgram& shader)
{
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    shader.uniformMatrix4f("projMatrix", _projMatrix.data());
    shader.uniformMatrix4f("viewMatrix", _viewMatrix.data());
    shader.uniformMatrix4f("modelMatrix", _modelMatrix.data()); // For the remote work, use identity matrix instead of the tracker's

    glBindVertexArray(_cube.vao);

    glDrawArrays(GL_TRIANGLES, 0, _cube.numVerts);

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
}

void VolumeRenderer::drawVolume(mv::ShaderProgram& shader, const QMatrix4x4& camRef, const bool& live)
{
    if (_numPoints > 0) {

        //glEnable(GL_POINT_SMOOTH);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
        glDepthMask(pointOpacity < 0.2 ? GL_FALSE : GL_TRUE);

        shader.uniformMatrix4f("projMatrix", _projMatrix.data());
        shader.uniformMatrix4f("viewMatrix", _viewMatrix.data());
        shader.uniformMatrix4f("modelMatrix", _modelMatrix.data());
    
        glPointSize(3);
        glBindVertexArray(vao);

        shader.uniform1i("selecting", cursorFrozen);
        shader.uniform1f("heightOfNearPlane", heightOfNearPlane);
        shader.uniform1i("hasColors", false);
        shader.uniform3f("selectionColor", _selectionColor.redF(), _selectionColor.greenF(), _selectionColor.blueF());
        shader.uniform1i("live", live);
        shader.uniform1i("selectMode", selectionMode);
        shader.uniform1f("selectRadius", sphereSelectRadius);
        shader.uniform1f("baseOpacity", pow(pointOpacity,2));

        shader.uniformMatrix4f("cameraRef", camRef.data());

        if (_hasColors)
        {
            shader.uniform1i("hasColors", true);
            if (_colormap.isCreated())
            {
                _colormap.bind(0);
                shader.uniform1i("colormap", 0);
            }
        }

        //glDrawArrays(GL_POINTS, 0, _numPoints);
        glDrawElements(GL_POINTS, _numPoints, GL_UNSIGNED_INT, 0);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glDisable(GL_POINT_SMOOTH);
        glDepthMask(GL_TRUE);
    }
}

