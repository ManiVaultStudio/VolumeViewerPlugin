#pragma once

#include<Tracker.h>

#include <QWidget>
#include <QLabel>
#include <QInputEvent>
#include <optional>



enum calibState {
    Idle,
    Stopped, // Needs to restart with R to scan
    Origin,
    Forwards,
    Up
};


class ReferenceSetupWidget : public QWidget {
    Q_OBJECT


public:
    ReferenceSetupWidget(QWidget* parent, PSTracker* trackerPtr);

    void show();

    void resetState();
    
    bool eventFilter(QObject* target, QEvent* event);
    void continueCalib();
    void startMeasurement();
    void stopMeasurement();

private:
    QLabel* instructions;
    QLabel* errors;
    PSTracker* tracker;

    std::optional<QVector3D>  bufferVector;
    calibState state = calibState::Idle;

    // Measurements to define the next reference matrix
    std::optional<QVector3D> origin;
    std::optional<QVector3D> forwards;
    std::optional<QVector3D> up;
};

