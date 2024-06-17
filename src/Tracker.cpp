#include "Tracker.h"

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

void PSTracker::Connect()
{
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
        //std::cout << "Press enter to continue...\n";
        //std::cin.get();
        return;
    }
    _connected = true;
    qDebug() << "Connected to tracker!";
}

float t = 0;

QMatrix4x4 PSTracker::GetTrackerMatrix()
{
    if (_connected)
    {
        // Read tracker matrix
        const std::lock_guard<std::mutex> lock(mtx);

        return trackerMatrix;
    }
    else
    {
        t += 0.1f;
        if (t > 360) t = t - 360;
        _defaultMatrix.setToIdentity();
        _defaultMatrix.rotate(t, 0, 1, 0);
        return _defaultMatrix;
    }
}
