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

//#define STEREO

class Cube : public QOpenGLFunctions_3_3_Core
{
public:
    void create();

    GLuint vao;
    GLuint vbo;
    GLuint nbo;
    int numVerts = 36;
};

/**
 * OpenGL Volume Renderer
 * This class provides a pure OpenGL renderer for volume data
 *
 * @author Julian Thijssen
 */
class VolumeRenderer : public QOpenGLFunctions_3_3_Core
{
public:
    void setData(std::vector<float>& data);
    void setColors(std::vector<float>& colors);
    void setHighlights(std::vector<int>& highlights);
    void setColormap(const QImage& colormap);
    void setEyeOffset(float eyeOffset) { _eyeOffset = eyeOffset; }
    void setInterlacingFlip(const bool & flipped) { _interlacing = flipped ? 1 : 0; }
    void setSelectionColor(const QColor& color) { _selectionColor = color; }
    //void setCursorPosition(const QVector3D& pos) { _cursorPoint = mv::Vector3f(pos[0], pos[1], pos[2]); }
    QVector3D getCursor() const;
    bool getCursorFrozen() const { return cursorFrozen; }
    float getSelectRadius() const { return sphereSelectRadius; }
    void setSelectionMode(const int& mode) { selectionMode = mode; }
    void incrementSelectRadius(const float& increment);
    void freezeCursor();
    void unFreezeCursor();
    void reloadShader();

    void init();
    void resize(int w, int h);

    void render(GLuint framebuffer, mv::Vector3f camPos, float aspect, const bool& , const QMatrix4x4& modelMatrix);
    void drawVolume(mv::ShaderProgram& shader, const bool& live);
    void drawCube(mv::ShaderProgram& shader);

private:
    mv::Framebuffer _framebuffer;
    mv::Framebuffer _leftRenderFBO;
    mv::Framebuffer _rightRenderFBO;
    mv::Texture2D _colorAttachment;
    mv::Texture2D _leftColorAttachment;
    mv::Texture2D _rightColorAttachment;

    mv::Texture2D _leftDepthAttachment;
    mv::Texture2D _rightDepthAttachment;

    mv::ShaderProgram _volumeShaderProgram;
    mv::ShaderProgram _pointsShaderProgram;
    mv::ShaderProgram _cursorShaderProgram;
    mv::ShaderProgram _cubeShaderProgram;
    mv::ShaderProgram _stereoMergeProgram;

    mv::ShaderProgram _framebufferShaderProgram;

    GLuint vao;
    GLuint vbo;
    GLuint cbo;
    GLuint highlightVBO;
    int _numPoints = 0;

    GLuint _cursorVao;
    GLuint _cursorVbo;
    //mv::Vector3f _cursorPoint = mv::Vector3f(0.f,0.f,0.f);
    float _eyeOffset = 0.065;
    int _interlacing = 0;

    QColor _selectionColor = QColor(0,0,0);
    bool _hasColors = false;

    mv::Texture2D _colormap;
    //QSize cMapSize;

    Cube _cube;

    QMatrix4x4 _projMatrix;
    QMatrix4x4 _leftProjMatrix;
    QMatrix4x4 _rightProjMatrix;
    QMatrix4x4 _viewMatrix;
    QMatrix4x4 _modelMatrix;

    bool cursorFrozen = false;
    QVector4D _cursorPosition = QVector4D(0.f, 0.f, 0.f, 1.f); // In modelMatrix reference
    QVector4D _frozenCursorPosition; // In world reference

    int selectionMode = 0;
    float sphereSelectRadius = 0.05f;


    QMatrix4x4 identity = QMatrix4x4();
};
