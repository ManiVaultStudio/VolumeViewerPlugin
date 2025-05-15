#include "controls.h"

#include <QLabel>
#include <QGridLayout>
#include <QLayoutItem>
#include <QSlider>
#include <iostream>

#include <string>
#include <sstream>
#include <iomanip>


using namespace std;

ControlsWidget::ControlsWidget(QWidget* parent) : QWidget(parent, Qt::Window),
transformation(QMatrix4x4())
{
    transformation.setToIdentity();
    setupUI();

    setWindowTitle("New Window");
    resize(250, 500);

    show();

    connect(this, &ControlsWidget::valuesChanged, this, [this]() {updateTransformationLabels();});
}

void ControlsWidget::updateTransformationLabels() {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            stringstream ss;
            ss << fixed << setprecision(2) << transformation(i, j);

            labels[i + 4 * j]->setText(QString::fromStdString(ss.str()));
        }
    }
}

void ControlsWidget::setupUI() {
    QVBoxLayout* vLayout = new QVBoxLayout(this);

    QGridLayout* matrix = new QGridLayout(this);

    
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            labels[i + 4 * j] = new QLabel(this);
            labels[i + 4 * j]->setObjectName("lab"+ to_string(i) + to_string(j));
            matrix->addWidget(labels[i + 4 * j], i, j);
            labels[i + 4 * j]->setAlignment(Qt::AlignCenter);
        }
    }
    updateTransformationLabels();
    
    QFrame* matrixContainer = new QFrame(this);
    matrixContainer->setLayout(matrix);
    matrixContainer->setMaximumSize(QSize(16777215, 200));
    vLayout->addWidget(matrixContainer);


    QSizePolicy sizePolicyTitles(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Fixed);
    QLabel* titleTR = new QLabel(this);
    titleTR->setObjectName("titleTR");
    titleTR->setAlignment(Qt::AlignCenter);
    titleTR->setText("Translation");
    titleTR->setSizePolicy(sizePolicyTitles);
    vLayout->addWidget(titleTR);

    for (int i = 0; i < 3; i++) {
        values[i] = 0;

        QSlider* slider = new QSlider(this);
        slider->setObjectName("slider");
        slider->setOrientation(Qt::Orientation::Horizontal);
        slider->setMinimum(-100);
        slider->setMaximum(100);
        vLayout->addWidget(slider);

        connect(slider, &QSlider::valueChanged, this, [this, i](const int &value) {
            float amount = static_cast<float>(value - values[i])/100.0f;
            transformation.translate(
                i == 0 ? amount : 0,
                i == 1 ? amount : 0,
                i == 2 ? amount : 0
            );

            values[i] = value;
            emit valuesChanged();
        });
    }

    QLabel* titleRot = new QLabel(this);
    titleRot->setObjectName("titleRot");
    titleRot->setAlignment(Qt::AlignCenter);
    titleRot->setText("Rotation");
    titleRot->setSizePolicy(sizePolicyTitles);
    vLayout->addWidget(titleRot);

    for (int i = 0; i < 3; i++) {
        values[i+3] = 0;

        QSlider* slider = new QSlider(this);
        slider->setObjectName("slider");
        slider->setOrientation(Qt::Orientation::Horizontal);

        slider->setMinimum(-18);
        slider->setMaximum(18);

        vLayout->addWidget(slider);

        connect(slider, &QSlider::valueChanged, this, [this, i](const int& value) {
            transformation.rotate(
                static_cast<float>(value - values[i + 3])*10.0f,
                i == 0 ? 1 : 0,
                i == 1 ? 1 : 0,
                i == 2 ? 1 : 0
            );

            values[i+3] = value;
            emit valuesChanged();
        });
    }

    imageColorMap = new QLabel(this);
    


    setLayout(vLayout);
}


void ControlsWidget::rotateX(const float& angle) {
    // Rotating in world space
    QQuaternion quaternion = QQuaternion().fromAxisAndAngle(QVector3D(1, 0, 0), angle);
    QMatrix4x4 rotation = QMatrix4x4();
    rotation.setToIdentity();
    rotation.rotate(quaternion);
    transformation = rotation*transformation;

}

void ControlsWidget::rotateY(const float& angle) {
    // Rotating in world space
    QQuaternion quaternion = QQuaternion().fromAxisAndAngle(QVector3D(0, 1, 0), angle);
    QMatrix4x4 rotation = QMatrix4x4();
    rotation.setToIdentity();
    rotation.rotate(quaternion);
    transformation = rotation * transformation;
}

void ControlsWidget::setImageColorMap(const QImage& image) {
    imageColorMap->setPixmap(QPixmap::fromImage(image));
    imageColorMap->resize(image.width(), image.height());

}