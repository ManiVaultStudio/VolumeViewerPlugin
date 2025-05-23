#pragma once

#ifdef WIN32
#define NOMINMAX
#include <windows.h>
#else
#include <csignal>
#endif

#include <QMatrix4x4>
#include <iostream>
#include <mutex>
#include <QElapsedTimer>


#include "pstsdk_cpp.h"
#include "TrackerExceptions.h"
#include "PstStringIoStream.h"

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

static inline void pstToQtMatrix(const PSTech::Utils::PstArray<float, 16>& inMatrix, QMatrix4x4& outMatrix) {
    for (int i = 0; i < 16; i++)
        outMatrix.data()[i] = inMatrix[i];
    outMatrix = outMatrix.transposed();
}

static PSTech::Utils::PstArray<float, 16> qtToPstMatrix(QMatrix4x4 inMatrix) {
    PSTech::Utils::PstArray<float, 16> outMatrix;
    inMatrix = inMatrix.transposed();
    for (int i = 0; i < 16; i++)
        outMatrix[i] = inMatrix.data()[i];
    return outMatrix;
}











class MyListener : public PSTech::pstsdk::Listener
{
    QMatrix4x4 targetMatrix;
    int controlTargetId = -1;
    int cursorTargetId = -1;

    /** Time elapsed since last pose */
    QElapsedTimer timer;
    /** Maximum amount of time for the last pose to be considered "not current", in milliseconds */
    const qint64 poseIsOldThreshold = 200;

    virtual void OnTrackerData(const PSTech::pstsdk::TrackerData& td);
public:
    bool poseIsLive() const;

    QMatrix4x4 getTragetMatrix() const;
    void setControlTarget(const int& id) { controlTargetId = id; }
    void setCursorTarget(const int& id) { cursorTargetId = id; }
};














class PSTracker
{
public:
    PSTracker();
    ~PSTracker();
    void Connect();
    
    QMatrix4x4 GetTargetMatrix();
    QMatrix4x4 GetReference() const;

    void checkTrackerStatus();
    void setTrackerReference(const QMatrix4x4& matrix, const bool& relative);

    bool poseIsLive() const { return poseAcurate && listener.poseIsLive(); }

    bool getTrackerConnected() const;


private:
    PSTech::pstsdk::TargetStatuses targets;

    MyListener listener;
    PSTech::pstsdk::Tracker* _pst;

    bool _connected = false;

    /** Timing of the animation after there has been a tracking lost */
    QElapsedTimer lerpTimer;
    /** Time to animate the target between the last live position to the new one, after there has been a tracking lost */
    const qint64 lerpDuraton = 300;
    bool poseAcurate = true;

    QMatrix4x4 lerpTrajectory;
    QMatrix4x4 oldPos;
};
