#pragma once

#include "graphics/Shader.h"
#include "graphics/Vector3f.h"
#include "graphics/Vector2f.h"

#include <QOpenGLFunctions_4_2_Core>
#include <QMatrix4x4>

#include <vector>

/**
 * OpenGL Volume Renderer
 * This class provides a pure OpenGL renderer for volume data
 *
 * @author Julian Thijssen
 */
class VolumeRenderer : public QOpenGLFunctions_4_2_Core
{
public:
    void setTexels(int width, int height, int depth, std::vector<float>& texels);
    void setData(std::vector<float>& data);
    void setColors(std::vector<float>& colors);

    void init();

    void render(hdps::Vector3f camPos, hdps::Vector2f camAngle, float aspect);

private:
    GLuint _texture;

    hdps::ShaderProgram _volumeShaderProgram;
    hdps::ShaderProgram _pointsShaderProgram;

    GLuint vao;
    GLuint vbo;
    GLuint cbo;
    int _numPoints = 0;

    bool _hasColors = false;

    QMatrix4x4 _projMatrix;
    QMatrix4x4 _viewMatrix;
    QMatrix4x4 _modelMatrix;
};
