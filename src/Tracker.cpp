#include "Tracker.h"



#include <thread>
#include <iostream>




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



void MyListener::OnTrackerData(const PSTech::pstsdk::TrackerData& td)
{


    for (int d = 0; d < td.targetlist.size(); ++d)
    {

        auto& mat = td.targetlist[d].pose;

        if (td.targetlist[d].id == controlTargetId)
        {
            if (timer.isValid()) timer.restart();
            else timer.start();

            // Lock the thread to prevent other threads from modifying the ressource
            // Unlocked automaically when the mutex goes out of scope
            const std::lock_guard<std::mutex> lock(mtx);

            pstToQtMatrix(mat, targetMatrix);
        }

    }

}
QMatrix4x4 MyListener::getTragetMatrix() const {
    return targetMatrix;
}

bool MyListener::poseIsLive() const {
    return timer.isValid() ? timer.elapsed() < poseIsOldThreshold : false;
};





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
    if (_pst != nullptr) delete _pst;
}

void PSTracker::checkTrackerStatus() {
    qDebug() << "PS Tech system check : ";
    PSTech::pstsdk::StatusMessage msg = _pst->Systemcheck();
    switch (msg) {
    case PSTech::pstsdk::StatusMessage::OK: {
        qDebug() << "PS Tech system is running OK";
        _connected = true;
        return;
    }
    case PSTech::pstsdk::StatusMessage::NOT_INITIALIZED: {
        throw "PS Tech system is NOT_INITIALIZED";
        break;
    }
    case PSTech::pstsdk::StatusMessage::DISCONNECTED: {
        throw "PS Tech system is DISCONNECTED";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_GENERAL: {
        throw "PS Tech: Unspecified grabber error ";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_TIMEOUT: {
        throw "PS Tech : Grabber timeout error";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_NOCAMS_FOUND: {
        throw "PS Tech : Grabber could not detect any cameras ";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_NOTENOUGHTCAMS_FOUND: {
        throw "PS Tech : Grabber could not detect sufficient cameras ";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_INITERROR: {
        throw "PS Tech : Grabber did not initialize correctly ";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_CANNOT_START_CAMS: {
        throw "PS Tech : Grabber could not start cameras ";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_CANNOT_SETUP_CAMS: {
        throw "PS Tech : Grabber failed setting up cameras ";
        break;
    }

    }

    _connected = false;

    throw "Unknown issue with the PS Tech";
}

bool PSTracker::getTrackerConnected() const {
    return _connected;
};


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

        // Perform a system check to see if the tracker server is running OK and print the result.
        checkTrackerStatus();

        // Set the reference to match OpenGL's axis. Be careful, PST axis names in the api are not the same as OpenGL's
        // OpenGL X = PST X (Horizontal)
        // OpenGL Y = PST Z (Away from the tracker)
        // OpenGL Z = PST Y (Vertical)
        PSTech::Utils::PstArray<float, 16> reference{  -1.0f, 0.0f, 0.0f, 0.f,
                                                       0.0f, 0.0f, 1.0f, 0.f,
                                                       0.0f, 1.0f, 0.0f, 0.f,
                                                       0.0f, 0.0f, 0.0f, 1.f };
        _pst->SetReference(reference);


        // Activate filtering to reduce movement jitter due to imprecisions in pstech and hand
        // osition filter is less strong to improve fine positionning when selecting
        _pst->EnableTremorFilter();
        _pst->SetPositionFilter(0.08);
        _pst->SetOrientationFilter(0.1);




        // Set the frame rate to 30 Hz.
        _pst->SetFramerate(120);

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
        if(targets.size() >= 2) listener.setCursorTarget(targets[1].id);
 


    }
    catch (PSTech::TrackerException& e)
    {
        // Catch PSTech::TrackerException exceptions and print error messages.
        std::cout << e.full_description() << "\n";

        Exithandler(0);

        throw e;
        //return;
    }
    
    qDebug() << "Connected to tracker!";
}

float t = 0;

QMatrix4x4 PSTracker::GetTargetMatrix()
{

    if (_connected)
    {
        QMatrix4x4 currPose = listener.getTragetMatrix();

        // Prepare for interpolation for when tracking goes back live
        if (!listener.poseIsLive()) {
            oldPos = currPose;
            lerpTimer.invalidate();
            poseAcurate = false;
        }

        // When the tracking goes back live, interpolate between the old and the new position
        if (!poseAcurate && listener.poseIsLive()) { // Is live and ready to interpolate
            if (!lerpTimer.isValid()) {
                lerpTrajectory = currPose - oldPos;
                lerpTimer.start();
            }

            if (lerpTimer.hasExpired(lerpDuraton)) {
                lerpTimer.invalidate();
                poseAcurate = true;
            }
            else {
                currPose -= lerpTrajectory / static_cast<float>(lerpDuraton) * (lerpDuraton - lerpTimer.elapsed());
            }
        }
        
        return currPose;

    }
    else
    {
        t += 0.1f;
        if (t > 360) t = t - 360;
        QMatrix4x4 _defaultMatrix;
        _defaultMatrix.setToIdentity();
        _defaultMatrix.rotate(t, 0, 1, 0);
        return _defaultMatrix;
    }
}

QMatrix4x4 PSTracker::GetReference() const {
    QMatrix4x4 reference;
    pstToQtMatrix(_pst->GetReference(), reference);
    return reference;
}


void PSTracker::setTrackerReference(const QMatrix4x4& matrix, const bool& relative = false) {
    _pst->SetReference(qtToPstMatrix(matrix), relative);
}
