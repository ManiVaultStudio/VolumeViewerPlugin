#pragma once

#include "graphics/Shader.h"
#include "graphics/Vector3f.h"
#include "graphics/Vector2f.h"
#include "graphics/Framebuffer.h"
#include "graphics/Texture.h"

#include <QOpenGLFunctions_3_3_Core>
#include <QMatrix4x4>
#include <QTimer>
#include <QThread>

#include <vector>





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
    void setData(std::vector<float>* data);
    void setColors(std::vector<float>& colors);
    void setAlphas(std::vector<float>& alphas);
    void setHighlights(std::vector<int>& highlights);
    void setPointOpacity(const float& value);
    void setFlashlightState(const bool& state) { showingFlashlight = state; }
    void setFlashlightScalars(const float& slope, const float& min);
    void setIsFlashlightSource(const bool& state) { isFlashlightSouce = state; }
    void setColormap(const QImage& colormap);
    void setEyeOffset(float eyeOffset);
    void setHeadPosition(const QVector3D& headPos);
    void updateCameras();
    std::vector<QMatrix4x4>& getViewMatrices() { return _viewMatrices; }
    void setInterlacing(const int& interl) { _interlacing = interl; }
    void setFov(const float& value);
    void setSelectionColor(const QColor& color) { _selectionColor = color; }
    void setStereo(const bool& val);
    bool isStereo() const { return stereo; }
    void updateProjectionMatrix();
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

    void render(GLuint framebuffer, const bool& live, const QMatrix4x4& modelMatrix);
    void drawVolume(mv::ShaderProgram& shader, const bool& live, int eye = -1);
    void drawCube(mv::ShaderProgram& shader);
    void drawCursor(int eye = -1);

    void setRenderOrder(const int& eye, std::vector<GLuint>& indices);
    //void filterPoints(const float& proba);
    std::vector<GLuint> getRenderedPoints();




private:
    mv::Framebuffer _framebuffer;
    mv::Framebuffer _leftRenderFBO;
    mv::Framebuffer _rightRenderFBO;
    mv::Texture2D _colorAttachment;
    mv::Texture2D _depthAttachment;
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
    GLuint alpha_cbo;
    GLuint highlightVBO;
    int _numPoints = 0;

    std::vector<GLuint> ebo = std::vector<GLuint>(2);

    GLuint _cursorVao;
    GLuint _cursorVbo;
    //mv::Vector3f _cursorPoint = mv::Vector3f(0.f,0.f,0.f);
    float _eyeDistance = 0.08; //0.063;
    int _interlacing = 1;
    float fovyr = 1.0472;
    float zNearMono = 0.1f;
    float zNearStereo = 0.35f;
    float aspect = 1;

    QColor _selectionColor = QColor(0,0,0);
    bool _hasColors = false;
    bool _hasAlphas = false;
    bool isFlashlightSouce = false;
    bool showingFlashlight = false;
    float flashlightSlope = -1.f;
    float flashlightMin = 0.1f;

    mv::Texture2D _colormap;
    //QSize cMapSize;

    Cube _cube;

    bool stereo = false;
    QVector3D headPosition;
    std::vector<QVector3D> stereoCameras = std::vector<QVector3D>(2);

    QMatrix4x4 _projMatrix;
    QMatrix4x4 _leftProjMatrix;
    QMatrix4x4 _rightProjMatrix;
    /// <summary>
    /// First matrix for the center of the head, then second and thirst for left and right eye
    /// </summary>
    std::vector<QMatrix4x4> _viewMatrices = std::vector<QMatrix4x4>(3);
    QMatrix4x4 _modelMatrix;

    bool cursorFrozen = false;
    QVector4D _cursorPosition = QVector4D(0.f, 0.f, 0.f, 1.f); // In modelMatrix reference
    QVector4D _frozenCursorPosition; // In world reference

    int selectionMode = 0;
    float sphereSelectRadius = 0.05f;


    QMatrix4x4 identity = QMatrix4x4();

    //std::vector<float>* points;
    // Points can be hidden by flipping the corresponding bool to false
    std::vector<bool> rendered;

    float pointOpacity = 1.0f;

    float heightOfNearPlane;
};
