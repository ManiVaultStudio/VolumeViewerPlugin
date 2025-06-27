#include "FullScreenWidget.h"

FullScreenWidget::FullScreenWidget(QWidget* parent) : QWidget(parent, Qt::Window) {
    layout = new QHBoxLayout();
    layout->setSpacing(10);
    layout->setContentsMargins(0, 0, 0, 0);
    setLayout(layout);

    QPalette pal = QPalette();

    pal.setColor(QPalette::Window, Qt::gray);

    setAutoFillBackground(true);
    setPalette(pal);

    bool found3DScreen = false;
    screen = QGuiApplication::screens()[0];
    for (QScreen* scr : QGuiApplication::screens()) {
        if (scr->model() == "D2343") {
            screen = scr;
        }
    }

};

void FullScreenWidget::childEvent(QChildEvent* event) {
    if (event->child() == layout) return;
    if (event->added() || event->removed()) {
        count += event->added() ? 1 : -1;
        if (count == 0) {
            hide();
        }

        emit numberChildrenChanged();
    }
}


void FullScreenWidget::addWidget(QWidget* widget) {
    layout->addWidget(widget);
    move(screen->geometry().x(), screen->geometry().y());
    //showFullScreen(); Stangely, displaying th widget full screen can cause bugs in certain confirurations
    // Like when selecting with GradientView on the side (graphical glitches)
    showMaximized();
}
