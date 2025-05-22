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
    /** Number of times this exact pose has been read */
    int poseReads = 0;
    /** Maximum number of times the pose is read before being considered "not live" */
    const int poseIsOldThreshold = 5;
    virtual void OnTrackerData(const PSTech::pstsdk::TrackerData& td);
public:

    bool getPoseIsNew() const { return poseReads < poseIsOldThreshold; };
    QMatrix4x4 readPose();
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

    void checkTrackerStatus() const;
    void setTrackerReference(const QMatrix4x4& matrix, const bool& relative);

    bool getPoseIsNew() const {
        return listener.getPoseIsNew();
    };


private:
    PSTech::pstsdk::TargetStatuses targets;

    MyListener listener;
    PSTech::pstsdk::Tracker* _pst;

    int lerpStep = -1;
    int maxLerpSteps = 10;

    QMatrix4x4 lerpTransform;
    QMatrix4x4 oldPos;

    bool _connected = false;
};
