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

#define STEREO

class Cube : public QOpenGLFunctions_3_3_Core
{
public:
    void create()
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
    void setTexels(int width, int height, int depth, std::vector<float>& texels);
    void setData(std::vector<float>& data);
    void setColors(std::vector<float>& colors);
    void setColormap(const QImage& colormap);
    void setCursorPoint(mv::Vector3f cursorPoint);
    void setEyeOffset(float eyeOffset) { _eyeOffset = eyeOffset; }
    void setInterlacingFlip(bool flipped) { _interlacing = flipped ? 1 : 0; }
    void reloadShader();

    void init();
    void resize(int w, int h);

    void render(GLuint framebuffer, mv::Vector3f camPos, mv::Vector2f camAngle, float aspect, QMatrix4x4 modelMatrix);
    void drawVolume(mv::ShaderProgram& shader);
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
    //GLuint _texture;

    mv::ShaderProgram _volumeShaderProgram;
    mv::ShaderProgram _pointsShaderProgram;
    mv::ShaderProgram _cubeShaderProgram;
    mv::ShaderProgram _stereoMergeProgram;

    mv::ShaderProgram _framebufferShaderProgram;

    GLuint vao;
    GLuint vbo;
    GLuint cbo;
    int _numPoints = 0;

    GLuint _cursorVao;
    GLuint _cursorVbo;
    mv::Vector3f _cursorPoint;
    float _eyeOffset = 0.065;
    int _interlacing = 0;

    bool _hasColors = false;

    mv::Texture2D _colormap;

    Cube _cube;

    QMatrix4x4 _projMatrix;
    QMatrix4x4 _leftProjMatrix;
    QMatrix4x4 _rightProjMatrix;
    QMatrix4x4 _viewMatrix;
    QMatrix4x4 _modelMatrix;
};
