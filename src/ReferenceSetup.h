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

    std::vector<QPixmap> illustrations;
    QLabel* imageLabel;

    // Measurements to define the next reference matrix
    std::optional<QVector3D> origin;
    std::optional<QVector3D> forwards;
    std::optional<QVector3D> up;

    //std::string fileLoc = "./tracker_ref_matrix.txt";

    void createUI();

    /*void saveReference() const;
    void setStoredReference() const;*/
};
