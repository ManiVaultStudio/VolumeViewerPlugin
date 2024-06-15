#pragma once

#include "graphics/Shader.h"
#include "graphics/Vector3f.h"
#include "graphics/Vector2f.h"
#include "graphics/Framebuffer.h"
#include "graphics/Texture.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QTimer>

#include <vector>


namespace PSTech
{
    namespace pstsdk
    {
        class Tracker;
    }
}

#define STEREO

/**
 * OpenGL Volume Renderer
 * This class provides a pure OpenGL renderer for volume data
 *
 * @author Julian Thijssen
 */
class VolumeRenderer : public QOpenGLFunctions_3_3_Core
{
public:
    void setTexels(int width, int height, int depth, std::vector<float>& texels);
    void setData(std::vector<float>& data);
    void setColors(std::vector<float>& colors);
    void setColormap(const QImage& colormap);
    void setCursorPoint(mv::Vector3f cursorPoint);
    void reloadShader();

    void init();
    void resize(int w, int h);

    void render(GLuint framebuffer, mv::Vector3f camPos, mv::Vector2f camAngle, float aspect);
    void drawVolume(mv::ShaderProgram& shader);

private:
    mv::Framebuffer _framebuffer;
    mv::Framebuffer _leftRenderFBO;
    mv::Framebuffer _rightRenderFBO;
    mv::Texture2D _colorAttachment;
    mv::Texture2D _leftColorAttachment;
    mv::Texture2D _rightColorAttachment;
    //GLuint _texture;

    mv::ShaderProgram _volumeShaderProgram;
    mv::ShaderProgram _pointsShaderProgram;
    mv::ShaderProgram _stereoMergeProgram;

    mv::ShaderProgram _framebufferShaderProgram;

    GLuint vao;
    GLuint vbo;
    GLuint cbo;
    int _numPoints = 0;

    GLuint _cursorVao;
    GLuint _cursorVbo;
    mv::Vector3f _cursorPoint;
    float eyeOffset = 0.065;

    bool _hasColors = false;

    mv::Texture2D _colormap;

    QMatrix4x4 _projMatrix;
    QMatrix4x4 _leftProjMatrix;
    QMatrix4x4 _rightProjMatrix;
    QMatrix4x4 _viewMatrix;
    QMatrix4x4 _modelMatrix;

    PSTech::pstsdk::Tracker* _pst;
};
