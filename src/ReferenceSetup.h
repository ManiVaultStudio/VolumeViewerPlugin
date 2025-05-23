#pragma once

#include<Tracker.h>

#include <QWidget>
#include <QLabel>
#include <QInputEvent>
#include <optional>

enum calibState {
    Idle,
    Origin,
    Forwards,
    Up
};

class ReferenceSetupWidget : public QWidget {
    Q_OBJECT


public:
    ReferenceSetupWidget(QWidget* parent, PSTracker* trackerPtr);

    void show();

    void resetMeasures();
    
    bool eventFilter(QObject* target, QEvent* event);
    void continueCalib();
    void startMeasurement();
    void stopMeasurement();
    /*bool isSetup() {
        return origin != nullptr && forwards != nullptr && up != nullptr;
    }*/
private:
    QLabel* instructions;
    PSTracker* tracker;

    std::optional<QVector3D>  bufferVector;
    calibState state = calibState::Idle;

    // Measurements to define the next reference matrix
    std::optional<QVector3D> origin;
    std::optional<QVector3D> forwards;
    std::optional<QVector3D> up;
};