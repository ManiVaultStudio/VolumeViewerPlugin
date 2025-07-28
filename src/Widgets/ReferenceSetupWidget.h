#pragma once

#include<Controllers/Tracker.h>

#include <QWidget>
#include <QLabel>
#include <QInputEvent>
#include <optional>
#include <string>

#include <iostream>
#include <fstream>
#include <filesystem>

#include <vector>

#include <Controllers/Pedal.h>



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
    ReferenceSetupWidget(QWidget* parent);

    void show();

    void resetState();
    
    bool eventFilter(QObject* target, QEvent* event);
    void continueCalib();
    void startMeasurement();
    void stopMeasurement();
    void setTracker(PSTracker* trackerPtr) { tracker = trackerPtr; }
    void setPedalManager(PedalManager* pdm);
    void setTargetIndex(const int& index) { targetIndex = index; }

private:
    void pressAction();
    void releaseAction();

    QLabel* instructions;
    QLabel* errors;
    PSTracker* tracker;
    PedalManager* pedal = nullptr;

    int targetIndex = 0;

    std::optional<QVector3D>  bufferVector;
    calibState state = calibState::Idle;

    std::vector<QPixmap> illustrations;
    QLabel* imageLabel;

    // Measurements to define the next reference matrix
    std::optional<QVector3D> origin;
    std::optional<QVector3D> forwards;
    std::optional<QVector3D> up;

    void createUI();

};
