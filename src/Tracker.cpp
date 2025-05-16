#include "Tracker.h"



#include <thread>




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

void MyListener::OnTrackerData(const PSTech::pstsdk::TrackerData& td)
{


    for (int d = 0; d < td.targetlist.size(); ++d)
    {

        auto& mat = td.targetlist[d].pose;

        if (td.targetlist[d].id == controlTargetId)
        {
            const std::lock_guard<std::mutex> lock(mtx);

            for (int i = 0; i < 16; i++)
                trackerMatrix.data()[i] = mat[i];
            trackerMatrix = trackerMatrix.transposed();
        }
    }


}






/*
 * Implement the exit handler to shut-down the PST Tracker connection on application termination.
 */
static void Exithandler(int sig)
{
    std::cout << "Shutting down" << std::endl;
    PSTech::pstsdk::Tracker::Shutdown();
}



PSTracker::PSTracker() {
    // Register the exit handler with the application
    #ifdef WIN32
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);
    #else
        signal(SIGTERM, Exithandler);
        signal(SIGKILL, Exithandler);
        signal(SIGQUIT, Exithandler);
        signal(SIGINT, Exithandler);
    #endif
    try {
        _pst = new PSTech::pstsdk::Tracker();
    } catch (PSTech::TrackerException& e) {
        std::cout << "Could not connect to tracker." << std::endl;
       std::cout << e.full_description() << std::endl; // DEV 
        // throw e;
    }
}

PSTracker::~PSTracker() {
    Exithandler(0);
    delete _pst;
}


void PSTracker::Connect()
{


    // Implement error handling of PSTech::TrackerException exceptions to prevent 
    // improper PST Tracker shutdown on errors.
    try
    {


        // Check if calibration information is available for all cameras. When this is not the case, provide a warning.
        if (_pst->GetUncalibratedCameraUrls(true).size() > 0)
        {
            qDebug() << "\n No calibration information could be found in the configuration directory. "
                "Please use the PST Server and PST Client application to initialize the PST Tracker and create/import a tracking target. "
                "More information can be found in the Initialization section of the PST SDK manual and the PST Manual.\n\n";

            return;
        }

        // Print version number of the tracker server being used.
        std::cout << "Running PST Server version " << _pst->GetVersionInfo() << "\n";


        // Register the listener object to the tracker server.
        _pst->AddTrackerListener(&listener);

        // Start the tracker server.
        _pst->Start();
        std::cout << "Put the Reference card in front of the PST in order to see tracking results.\n\n";



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

        // Retrieve the list of registered tracking targets and print their names and current status (active or not).
        PSTech::pstsdk::TargetStatuses allTargets = _pst->GetTargetList(); // Some may be inactive
        
        if (allTargets.size() == 0) throw "No Target registered in the tracker. Please add at least one target in the PST Client app.";

        for (PSTech::pstsdk::TargetStatus target : allTargets) {
            if (target.status) targets.push_back(target);
        }

        if (targets.size() == 0) throw "No active target. Please activate the desired targets in the PST Client app.";
        listener.setControlTarget(targets[0].id);
 


    }
    catch (PSTech::TrackerException& e)
    {
        // Catch PSTech::TrackerException exceptions and print error messages.
        std::cout << e.full_description() << "\n";

        Exithandler(0);

        throw e;
        //return;
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
