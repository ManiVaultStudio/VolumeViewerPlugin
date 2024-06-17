#include "VolumeRenderer.h"

#include <QImage>

#include <random>

#include <QMatrix4x4>

void VolumeRenderer::setTexels(int width, int height, int depth, std::vector<float>& texels)
{
    //std::vector<float> texels(width * height * depth, 0);

    //for (int z = 0; z < depth; z++)
    //{
    //    for (int x = 0; x < width; x++)
    //    {
    //        for (int y = 0; y < height; y++)
    //        {
    //            if (z > 25 && z < 75)
    //                texels[z * width * height + x * height + y] = 1;
    //        }
    //    }
    //}
    //for (int i = 0; i < texels.size(); i+= 1)
    //{
    //    if (texels[i] == 1)
    //        qDebug() << texels[i];
    //}
    //glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);
    //glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32F, width, height, depth);
    //glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, depth, GL_RED, GL_FLOAT, texels.data());
    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void VolumeRenderer::setData(std::vector<float>& data)
{
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &cbo);
    glBindBuffer(GL_ARRAY_BUFFER, cbo);
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 0, 0);
    //glEnableVertexAttribArray(1);

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

void VolumeRenderer::setColormap(const QImage& colormap)
{
    _colormap.loadFromImage(colormap);
    qDebug() << "Colormap is set!";
}

void VolumeRenderer::setCursorPoint(mv::Vector3f cursorPoint)
{
    _cursorPoint = cursorPoint;
    qDebug() << _cursorPoint.x << _cursorPoint.y << _cursorPoint.z;
}

void VolumeRenderer::reloadShader()
{
    _pointsShaderProgram.loadShaderFromFile(":shaders/points.vert", ":shaders/VolumeDraw.frag");

    qDebug() << "Shaders reloaded";
}

void VolumeRenderer::init()
{
    initializeOpenGLFunctions();
    
    glClearColor(20 / 255.0f, 20 / 255.0f, 20/255.0f, 1.0f);

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

    _leftRenderFBO.create();
    _leftRenderFBO.bind();
    _leftRenderFBO.addColorTexture(0, &_leftColorAttachment);
    _leftRenderFBO.validate();

    // Make float buffer to support low alpha blending
    _rightColorAttachment.create();
    _rightColorAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    _rightRenderFBO.create();
    _rightRenderFBO.bind();
    _rightRenderFBO.addColorTexture(0, &_rightColorAttachment);
    _rightRenderFBO.validate();

    bool loaded = true;
    //loaded &= _volumeShaderProgram.loadShaderFromFile("volume.vert", "volume.frag");
    loaded &= _pointsShaderProgram.loadShaderFromFile(":shaders/points.vert", ":shaders/VolumeDraw.frag");
    loaded &= _framebufferShaderProgram.loadShaderFromFile(":shaders/Quad.vert", ":shaders/Texture.frag");
    loaded &= _stereoMergeProgram.loadShaderFromFile(":shaders/Quad.vert", ":shaders/StereoMerge.frag");

    if (!loaded) {
        qCritical() << "Failed to load one of the Volume Renderer shaders";
    }

    //glGenTextures(1, &_texture);

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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(0);

    /////////////
    //int width = 200;
    //int height = 400;
    //int depth = 100;
    //std::vector<float> texels(width * height * depth, 0);

    //std::default_random_engine generator;
    //std::uniform_real_distribution<float> distribution(0, 1);

    //for (int z = 0; z < depth; z++)
    //{
    //    for (int x = 0; x < width; x++)
    //    {
    //        for (int y = 0; y < height; y++)
    //        {
    //            float rand = distribution(generator);

    //            //texels[x * height * depth + y * depth + z] = rand;
    //            if (z > 25 && z < 75)
    //                texels[z * width * height + x * height + y] = 1;
    //        }
    //    }
    //}

    //glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);
    //glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_R32F, width, height, depth);
    //glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, width, height, depth, GL_RED, GL_FLOAT, texels.data());
    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
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

    glViewport(0, 0, w, h);
}

void VolumeRenderer::render(GLuint framebuffer, mv::Vector3f camPos, mv::Vector2f camAngle, float aspect, QMatrix4x4 modelMatrix)
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
    _pointsShaderProgram.bind();

    //#ifndef STEREO
    _projMatrix.setToIdentity();
    float fovyr = 1.57079633;
    float zNear = 0.1f;
    float zFar = 100;
    _projMatrix.data()[0] = (float)(1 / tan(fovyr / 2)) / aspect;
    _projMatrix.data()[5] = (float)(1 / tan(fovyr / 2));
    _projMatrix.data()[10] = (zNear + zFar) / (zNear - zFar);
    _projMatrix.data()[11] = -1;
    _projMatrix.data()[14] = (2 * zNear * zFar) / (zNear - zFar);
    _projMatrix.data()[15] = 0;
    //#else
    //    _leftProjMatrix.setToIdentity();
    //    float convergence_distance = 1.0f;
    //    float parallax_factor = 0.05f;
    //
    //    float fovyr = 1.57079633;
    //    float zNear = 0.1f;
    //    float zFar = 100;
    //    float stereo_offset = eye * zNear * parallax_factor / convergence_distance;
    //
    //    _projMatrix.data()[0] = (float)(1 / tan(fovyr / 2)) / aspect;
    //    _projMatrix.data()[5] = (float)(1 / tan(fovyr / 2));
    //    _projMatrix.data()[10] = (zNear + zFar) / (zNear - zFar);
    //    _projMatrix.data()[11] = -1;
    //    _projMatrix.data()[14] = (2 * zNear * zFar) / (zNear - zFar);
    //    _projMatrix.data()[15] = 0;
    //#endif

        //_projMatrix.data()[12] = 1;
    
    _modelMatrix.setToIdentity();

#ifdef PSTECH
    // Read tracker matrix
    {
        const std::lock_guard<std::mutex> lock(mtx);

        _modelMatrix = trackerMatrix;
    }
#endif
    _modelMatrix.data()[12] *= 10;
    _modelMatrix.data()[13] *= 10;
    _modelMatrix.data()[14] *= 10;

#ifndef STEREO
    _viewMatrix.setToIdentity();
    _viewMatrix.lookAt(QVector3D(camPos.x, camPos.y, camPos.z), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    _framebuffer.bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    drawVolume(_pointsShaderProgram, false, 0);
#else
    _viewMatrix.setToIdentity();
    _viewMatrix.lookAt(QVector3D(camPos.x - _eyeOffset, camPos.y, camPos.z), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    _leftRenderFBO.bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    drawVolume(_pointsShaderProgram);

    _viewMatrix.setToIdentity();
    _viewMatrix.lookAt(QVector3D(camPos.x + _eyeOffset, camPos.y, camPos.z), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    _rightRenderFBO.bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    drawVolume(_pointsShaderProgram);
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
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

    // Draw the cursor
    _pointsShaderProgram.bind();
    _pointsShaderProgram.uniform1i("isCursor", 1);
    glBindVertexArray(_cursorVao);
    glBindBuffer(GL_ARRAY_BUFFER, _cursorVbo);
    glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), &_cursorPoint, GL_STATIC_DRAW);

    glEnable(GL_POINT_SMOOTH);
    glPointSize(15);
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

void VolumeRenderer::drawVolume(mv::ShaderProgram& shader)
{
    glClear(GL_COLOR_BUFFER_BIT);

    _modelMatrix.setToIdentity();

    shader.uniformMatrix4f("projMatrix", _projMatrix.data());
    shader.uniformMatrix4f("viewMatrix", _viewMatrix.data());
    shader.uniformMatrix4f("modelMatrix", _modelMatrix.data());

    glPointSize(3);
    glBindVertexArray(vao);

    shader.uniform1i("hasColors", false);

    glDrawArrays(GL_POINTS, 0, _numPoints);

    if (_hasColors)
    {
        shader.uniform1i("hasColors", _hasColors);

        if (_colormap.isCreated())
        {
            _colormap.bind(0);
            shader.uniform1i("colormap", 0);
        }

        glDrawArrays(GL_POINTS, 0, _numPoints);
    }
}
