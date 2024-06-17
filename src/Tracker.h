#pragma once

#include <QMatrix4x4>

namespace PSTech
{
    namespace pstsdk
    {
        class Tracker;
    }
}

class PSTracker
{
public:
    void Connect();
    QMatrix4x4 GetTrackerMatrix();

private:
    PSTech::pstsdk::Tracker* _pst;
};
