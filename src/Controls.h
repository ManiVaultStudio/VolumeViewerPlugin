#pragma once

#include <QWidget>
#include <QLabel>
#include <QMatrix4x4>


class ControlsWidget : public QWidget {
	Q_OBJECT

public:
    ControlsWidget(QWidget* parent);
    ControlsWidget() : ControlsWidget(nullptr) {};
    void setupUI();
    QMatrix4x4 getControlMatrix() const { return transformation; };
    void updateTransformationLabels();
    void rotateX(const float& amount);
    void rotateY(const float& amount);
    void setImageColorMap(const QImage& image);
	
private:
    QLabel* labels[16];
    int values[6];
    QMatrix4x4 transformation; // Matrix of reference of the tracked object
    QLabel* imageColorMap;

signals:
    void valuesChanged();
};