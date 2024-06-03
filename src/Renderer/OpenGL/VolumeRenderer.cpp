#include "VolumeRenderer.h"

#include <QImage>

#include <random>

#include <QMatrix4x4>

////////////////
#define CONNECTED

#include "pstsdk_cpp.h"
#include "TrackerExceptions.h"
#include "PstStringIoStream.h"

#include <mutex>
////////////////

#ifdef WIN32
#include <windows.h>
#else
#include <csignal>
#endif

/*
 * Define handler functions required to ensure a clean shutdown of the PST Tracker when the
 * application is terminated.
 */

static void Exithandler(int sig);

#ifdef WIN32
BOOL WINAPI ConsoleHandler(DWORD CEvent)
{
    Exithandler(CEvent);
    return TRUE;
}
#endif

/* End of handler functions */

std::mutex mtx;
QMatrix4x4 trackerMatrix;

///////////////////////
/*
 * Helper function for clear printing of 4x4 matrices.
 */
static inline void PrintMatrix(const PSTech::Utils::PstArray<float, 16>& mat)
{
    for (int y = 0; y < 4; ++y)
    {
        for (int x = 0; x < 4; ++x)
        {
            std::cout << mat[x + y * 4] << "\t";
        }
        std::cout << "\n";
    }
}

/* Control variable for main loop */
static bool running = true;

/* Number of data points to grab before application termination */
static const uint32_t numberOfSamplesToGrab = 100;


/*
 * Implementation of the PSTech::pstsdk::Listener class to receive tracking data.
 * The OnTrackerData() callback function receives the data as soon as it becomes
 * available and prints the tracking target pose to the command line.
 */
class MyListener : public PSTech::pstsdk::Listener
{
    virtual void OnTrackerData(PSTech::pstsdk::TrackerData& td)
    {
        static uint32_t samplesGrabbed = 0;
        //if (samplesGrabbed++ >= numberOfSamplesToGrab)
        //    running = false;

        for (int d = 0; d < td.targetlist.size(); ++d)
        {
            auto& mat = td.targetlist[d].pose;
            //std::cout << "Pose for " << td.targetlist[d].name << "\n";
            //std::cout << " ID " << td.targetlist[d].id << "\n";
            //PrintMatrix(mat);

            if (td.targetlist[d].id == 3)
            {
                const std::lock_guard<std::mutex> lock(mtx);

                for (int i = 0; i < 16; i++)
                    trackerMatrix.data()[i] = mat[i];
                trackerMatrix = trackerMatrix.transposed();
            }
        }
    }
} listener;

/*
 * Implement the exit handler to shut-down the PST Tracker connection on application termination.
 */
static void Exithandler(int sig)
{
    PSTech::pstsdk::Tracker::Shutdown();
    running = false;
}


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
    _pointsShaderProgram.loadShaderFromFile(":shaders/points.vert", ":shaders/points.frag");
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

    bool loaded = true;
    //loaded &= _volumeShaderProgram.loadShaderFromFile("volume.vert", "volume.frag");
    loaded &= _pointsShaderProgram.loadShaderFromFile(":shaders/points.vert", ":shaders/points.frag");
    loaded &= _framebufferShaderProgram.loadShaderFromFile(":shaders/Quad.vert", ":shaders/Texture.frag");

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

        // Register the exit handler with the application
#ifdef WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);
#else
    signal(SIGTERM, Exithandler);
    signal(SIGKILL, Exithandler);
    signal(SIGQUIT, Exithandler);
    signal(SIGINT, Exithandler);
#endif

    // Implement error handling of PSTech::TrackerException exceptions to prevent 
    // improper PST Tracker shutdown on errors.
    try
    {
#ifdef CONNECTED
        // Create an instance of the Tracker object using the default configuration path and file names.
#ifdef WIN32
        _pst = new PSTech::pstsdk::Tracker();
#else
        // On Linux, specify the type of grabber that needs to be used as the last parameter: 
        // "basler_ace" for PST HD or "basler_dart" for PST Pico
        PSTech::pstsdk::Tracker pst("", "config.cfg", "models.db", argv[1]);
#endif

        // Check if calibration information is available for all cameras. When this is not the case, provide a warning.
        if (_pst->GetUncalibratedCameraUrls(true).size() > 0)
        {
            std::cout << "\nNo calibration information could be found in the configuration directory. "
                "Please use the PST Server and PST Client application to initialize the PST Tracker and create/import a tracking target. "
                "More information can be found in the Initialization section of the PST SDK manual and the PST Manual.\n\n";
            std::cout << "Press enter to continue...\n";
            std::cin.get();
            return;
        }

        // Print version number of the tracker server being used.
        std::cout << "Running PST Server version " << _pst->GetVersionInfo() << "\n";

        // Register the listener object to the tracker server.
        _pst->AddTrackerListener(&listener);

        std::cout << "Put the Reference card in front of the PST in order to see tracking results.\n\n";

        // Start the tracker server.
        _pst->Start();

        // Perform a system check to see if the tracker server is running OK and print the result.
        std::cout << "System check: " << (int)_pst->Systemcheck() << "\n";
        if (_pst->Systemcheck() == PSTech::pstsdk::StatusMessage::OK)
        {
            std::cout << "System is running OK." << std::endl;
        }
        // Set the frame rate to 30 Hz.
        _pst->SetFramerate(60);

        // Print the new frame rate to see if it was set correctly. Note that for PST HD and Pico
        // trackers the frame rate actually being set can differ from the value provided to SetFramerate().
        std::cout << "Frame rate set to " << _pst->GetFramerate() << "\n";
#endif
    }
    catch (PSTech::TrackerException& e)
    {
        // Catch PSTech::TrackerException exceptions and print error messages.
        std::cout << e.full_description() << "\n";

        // Pause command line to see error message.
        std::cout << "Press enter to continue...\n";
        std::cin.get();
    }
}

void VolumeRenderer::resize(int w, int h)
{
    qDebug() << "Resize called";
    _colorAttachment.bind();
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, nullptr);

    glViewport(0, 0, w, h);
}

void VolumeRenderer::render(GLuint framebuffer, mv::Vector3f camPos, mv::Vector2f camAngle, float aspect)
{
    _framebuffer.bind();
    glDrawBuffer(GL_COLOR_ATTACHMENT0);

    glEnable(GL_BLEND);
    glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD);
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

    glClear(GL_COLOR_BUFFER_BIT);

#ifdef VOLUME
    _volumeShaderProgram.bind();

    glActiveTexture(GL_TEXTURE0);
    _volumeShaderProgram.uniform1i("tex", 0);

    glBindTexture(GL_TEXTURE_2D_ARRAY, _texture);

    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
#else
    _pointsShaderProgram.bind();

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

    //_projMatrix.data()[12] = 1;

    _viewMatrix.setToIdentity();
    //_viewMatrix.rotate(camAngle.y * (180 / 3.14159), 0, 1, 0);
    //_viewMatrix.rotate(camAngle.x * (180 / 3.14159), 1, 0, 0);
    //_viewMatrix.translate(-camPos.x, -camPos.y, -camPos.z);
    _viewMatrix.lookAt(QVector3D(camPos.x, camPos.y, camPos.z), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    
    _modelMatrix.setToIdentity();
    //_modelMatrix.translate(0, 0, -3);

    // Read tracker matrix
    {
        const std::lock_guard<std::mutex> lock(mtx);

        _modelMatrix = trackerMatrix;
    }
    _modelMatrix.data()[12] *= 10;
    _modelMatrix.data()[13] *= 10;
    _modelMatrix.data()[14] *= 10;

    //for (int i = 0; i < 16; i++)
    //    qDebug() << _modelMatrix.data()[i];
    _pointsShaderProgram.uniformMatrix4f("projMatrix", _projMatrix.data());
    _pointsShaderProgram.uniformMatrix4f("viewMatrix", _viewMatrix.data());
    _pointsShaderProgram.uniformMatrix4f("modelMatrix", _modelMatrix.data());
    
    glPointSize(3);
    glBindVertexArray(vao);

    _pointsShaderProgram.uniform1i("hasColors", false);

    glDrawArrays(GL_POINTS, 0, _numPoints);

    if (_hasColors)
    {
        _pointsShaderProgram.uniform1i("hasColors", _hasColors);

        if (_colormap.isCreated())
        {
            _colormap.bind(0);
            _pointsShaderProgram.uniform1i("colormap", 0);
        }

        glDrawArrays(GL_POINTS, 0, _numPoints);
    }

    // Draw the cursor
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
    glDrawBuffer(GL_BACK);

    glDisable(GL_BLEND);

    glClear(GL_COLOR_BUFFER_BIT);

    _framebufferShaderProgram.bind();

    _colorAttachment.bind(0);
    _framebufferShaderProgram.uniform1i("tex", 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);
#endif
}
