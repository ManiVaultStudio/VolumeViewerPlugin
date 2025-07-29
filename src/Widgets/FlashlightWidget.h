#pragma once

#include <QWidget>

//#include <QInputEvent>
//#include <optional>
//#include <string>
//
//#include <iostream>
//#include <fstream>
//#include <filesystem>
//
//#include <vector>





class FlashlightWidget : public QWidget {
    Q_OBJECT


public:
    FlashlightWidget(QWidget* parent);

    float getDistanceCoeficient() const ;
    float getMinTransparency() const { return transparency; }

private:

    float spread = 0.6f; // Distance from cursor to reach minimum transparency
    float transparency = 0.025f; // Minimum transparency of the flashlight effect

    void createUI();

signals:
    void valueChanged();

};
