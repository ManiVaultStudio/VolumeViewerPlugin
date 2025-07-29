#include "FlashlightWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>



FlashlightWidget::FlashlightWidget(QWidget* parent) :
    QWidget(parent, Qt::Window)
{
    setWindowTitle("Flashlight customization");
    resize(300, 200);

    createUI();

}
float FlashlightWidget::getDistanceCoeficient() const
{

    return (transparency - 1) / spread;
}


void FlashlightWidget::createUI() {

    QVBoxLayout* vLayout = new QVBoxLayout(this);
    vLayout->setSpacing(10);

    QSpacerItem* verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
    vLayout->addItem(verticalSpacer);


    QLabel* instructions = new QLabel(this);
    instructions->setText("Spread");
    vLayout->addWidget(instructions);

    QSlider* horizontalSlider = new QSlider(this);
    QSizePolicy sizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(horizontalSlider->sizePolicy().hasHeightForWidth());
    horizontalSlider->setMaximum(100);
    horizontalSlider->setValue(spread*100/2);
    horizontalSlider->setOrientation(Qt::Orientation::Horizontal);
    horizontalSlider->setSizePolicy(sizePolicy);
    vLayout->addWidget(horizontalSlider);
    connect(horizontalSlider, &QSlider::valueChanged, this, [this](const int& value) {
        spread = value / 100.f * 2; 
        emit valueChanged();
        });

    QLabel* instructions2 = new QLabel(this);
    instructions2->setText("Transparency level");
    vLayout->addWidget(instructions2);


    QSlider* horizontalSlider2 = new QSlider(this);
    QSizePolicy sizePolicy2(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    sizePolicy2.setHorizontalStretch(0);
    sizePolicy2.setVerticalStretch(0);
    sizePolicy2.setHeightForWidth(horizontalSlider2->sizePolicy().hasHeightForWidth());
    horizontalSlider2->setOrientation(Qt::Orientation::Horizontal);
    horizontalSlider2->setMaximum(100);
    horizontalSlider2->setValue(std::sqrt(transparency*100));
    horizontalSlider2->setSizePolicy(sizePolicy);
    vLayout->addWidget(horizontalSlider2);

    connect(horizontalSlider2, &QSlider::valueChanged, this, [this](const int& value) {
        transparency = std::pow(value/100.f,2);
        emit valueChanged();
        });


    QSpacerItem* verticalSpacer2 = new QSpacerItem(100, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
    vLayout->addItem(verticalSpacer2);



    setLayout(vLayout);
}
