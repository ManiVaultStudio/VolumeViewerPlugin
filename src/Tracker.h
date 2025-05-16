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



class MyListener : public PSTech::pstsdk::Listener
{
    int controlTargetId;
    virtual void OnTrackerData(const PSTech::pstsdk::TrackerData& td);
public:
    void setControlTarget(const int &id) { controlTargetId = id; }
};

class PSTracker
{
public:
    void Connect();
    QMatrix4x4 GetTrackerMatrix();
    PSTracker();
    ~PSTracker();
    

private:
    PSTech::pstsdk::TargetStatuses targets;

    MyListener listener;
    PSTech::pstsdk::Tracker* _pst;


    bool _connected = false;
    QMatrix4x4 _defaultMatrix;
};
