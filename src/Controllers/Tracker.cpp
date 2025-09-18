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

        int index = idToIndex(td.targetlist[d].id);
        if (index > -1) {

            auto& mat = td.targetlist[d].pose;

    
            if (timers[index].isValid()) timers[index].restart();
            else timers[index].start();

            // Lock the thread to prevent other threads from modifying the ressource
            // Unlocked automaically when the mutex goes out of scope
            const std::lock_guard<std::mutex> lock(mtx);

            pstToQtMatrix(mat, targetMatrices[index]);
        
        }

    }

}

QMatrix4x4 MyListener::getTragetMatrix(const int& index) const {
    const std::lock_guard<std::mutex> lock(mtx);
    // If the viewer instance is greater than the number of targets, just use the last target
    return targetMatrices[(index) % targetMatrices.size()];
}

bool MyListener::poseIsLive(const int& index) const {
    return timers[index].isValid() ? timers[index].elapsed() < poseIsOldThreshold : false;
};

int MyListener::idToIndex(const int& id) const { 
  
    for (int i = 0; i < targetIdList.size(); i++) {
        if (targetIdList[i] == id) return i;
    }
    return -1;
};


void MyListener::addTarget(const int& id) {
    targetIdList.push_back(id);
    targetMatrices.push_back(QMatrix4x4());
    timers.push_back(QElapsedTimer());
}





/*
 * Implement the exit handler to shut-down the PST Tracker connection on application termination.
 */
static void Exithandler(int sig)
{
    std::cout << "Shutting down" << std::endl;
    PSTech::pstsdk::Tracker::Shutdown();
}



PSTracker::PSTracker(QObject *parent) : QObject(parent) {
    connectionTimer = new QTimer(this);
    connect(connectionTimer, &QTimer::timeout, this, [this]() {
        qDebug() << "Checking connection";
        if (!checkTrackerStatus()) {
            emit stopped();
        }
        connectionTimer->stop();
    });
    // Register the exit handler with the application
    #ifdef WIN32
        SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE);
    #else
        signal(SIGTERM, Exithandler);
        signal(SIGKILL, Exithandler);
        signal(SIGQUIT, Exithandler);
        signal(SIGINT, Exithandler);
    #endif
}

/// <summary>
/// Establsh connection with the PS Tech
/// </summary>
void PSTracker::Connect() {
    PSTech::pstsdk::EnableLogging();
    try {
        Exithandler(0);
        if (_pst != nullptr) {
            qDebug() << "Deleting old PST instance";
            delete _pst;
        }
        qDebug() << "Creating PST instance";
        _pst = new PSTech::pstsdk::Tracker();
    }
    catch (PSTech::TrackerException& e) {
        std::cout << "Could not connect to tracker." << std::endl;
        std::cout << e.full_description() << std::endl; // DEV 
        throw "The PS-tech tracker is not detected";
    }
}

PSTracker::~PSTracker() {
    Exithandler(0);
    delete _pst;
}




bool PSTracker::checkTrackerStatus() {
    if (_pst == nullptr) {
        throw "The PS-tech tracker is not detected";
    }

    qDebug() << "PS Tech system check : ";
    PSTech::pstsdk::StatusMessage msg = _pst->Systemcheck();
    switch (msg) {
    case PSTech::pstsdk::StatusMessage::OK: {
        qDebug() << "PS Tech system is running OK";
        return true;
    }
    case PSTech::pstsdk::StatusMessage::NOT_INITIALIZED: {
        qDebug() << "PS Tech system is NOT_INITIALIZED";
        break;
    }
    case PSTech::pstsdk::StatusMessage::DISCONNECTED: {
        qDebug() << "PS Tech system is DISCONNECTED";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_GENERAL: {
        qDebug() << "PS Tech: Unspecified grabber error ";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_TIMEOUT: {
        qDebug() << "PS Tech : Grabber timeout error";
        if (!triedAutoReboot) {
            triedAutoReboot = true;
            qDebug() << "tryingAutoReboot";
            Start();
        }
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_NOCAMS_FOUND: {
        qDebug() << "PS Tech : Grabber could not detect any cameras";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_NOTENOUGHTCAMS_FOUND: {
        qDebug() << "PS Tech : Grabber could not detect sufficient cameras";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_INITERROR: {
        qDebug() << "PS Tech : Grabber did not initialize correctly";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_CANNOT_START_CAMS: {
        qDebug() << "PS Tech : Grabber could not start cameras";
        break;
    }
    case PSTech::pstsdk::StatusMessage::ERR_CANNOT_SETUP_CAMS: {
        qDebug() << "PS Tech : Grabber failed setting up cameras";
        break;
    }

    }

    _connected = false;
    return false;

}

bool PSTracker::getTrackerConnected() const {
    return _connected;
};

/// <summary>
/// Start the PS Tech server and listen at target movements
/// </summary>
void PSTracker::Start()
{
    if (_pst == nullptr) throw "The PS-tech tracker is not detected";

    if (_connected){
        return;
    }

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


        triedAutoReboot = false;

        _pst->DisableImageTransfer();



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
            if (target.status) {
                 targets.push_back(target);
                 listener.addTarget(target.id);
                 poseAcurate.push_back(true);
            }
            //std::cout << target.name << " " << target.status << std::endl;
        }

        if (targets.size() == 0) throw "No active target. Please activate the desired targets in the PST Client app.";
 


        // Perform a system check to see if the tracker server is running OK and print the result.
        if (checkTrackerStatus()) {
            _connected = true;
            emit connected();

            qDebug() << "Tracker started";
        }
        else {
            qDebug() << "Failed starting tracker";
        }

    }
    catch (PSTech::TrackerException& e)
    {
        // Catch PSTech::TrackerException exceptions and print error messages.
        std::cout << e.full_description() << "\n";

        Exithandler(0);

        throw e;
        //return;
    }
    
}

/// <summary>
/// Get the last pose data for a specified target, using interpolation to smooth out tracking losts.
/// </summary>
/// <param name="index">Target whose pose is requested</param>
/// <param name="pose">out parameter where the last pose data recorded is written</param>
/// <returns>Whether the pose is live (current) or not</returns>
bool PSTracker::GetTargetMatrix(const int& index, QMatrix4x4& pose)
{

    if (_connected/* && !listener.getIsIdle(index)*/)
    {
        pose = listener.getTragetMatrix(index);


        if (listener.getIsIdle(index)) {
            if(!connectionTimer->isActive()) connectionTimer->start(5*1000);

            return false;
        }
     
        if (pose.column(3).toVector3D().length() < 0.01f) {
            // Strange bug where jumps to the origin happen here and there, just ignore these positions
            return true;
        }

        // Prepare for interpolation for when tracking goes back live
        if (!listener.poseIsLive(index)) {
            oldPos = pose;
            lerpTimer.invalidate();
            poseAcurate[index] = false;
        }

        // When the tracking goes back live, interpolate between the old and the new position
        if (!poseAcurate[index] && listener.poseIsLive(index)) { // Is live and ready to interpolate
            if (!lerpTimer.isValid()) {
                lerpTrajectory = pose - oldPos;
                lerpTimer.start();
            }

            if (lerpTimer.hasExpired(lerpDuraton)) {
                lerpTimer.invalidate();
                poseAcurate[index] = true;
            }
            else {
                pose -= lerpTrajectory / static_cast<float>(lerpDuraton) * (lerpDuraton - lerpTimer.elapsed());
            }
        }


        return true;

    }

    if (!connectionTimer->isActive()) connectionTimer->start(5 * 1000);

    return false;
}

QMatrix4x4 PSTracker::GetReference() const {
    QMatrix4x4 reference;
    pstToQtMatrix(_pst->GetReference(), reference);
    return reference;
}


void PSTracker::setTrackerReference(const QMatrix4x4& matrix, const bool& relative = false) {
    _pst->SetReference(qtToPstMatrix(matrix), relative);
}

