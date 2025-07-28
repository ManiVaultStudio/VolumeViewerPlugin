#include "FlashlightWidget.h"

#include <QVBoxLayout>
//
//
//ReferenceSetupWidget::ReferenceSetupWidget(QWidget* parent) :
//    QWidget(parent, Qt::Window),
//    tracker(nullptr)
//{
//    setWindowTitle("PS Tracker Reference Setup");
//    resize(400, 350);
//
//    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
//    installEventFilter(this);
//
//
//    createUI();
//
//}
//
//void ReferenceSetupWidget::setPedalManager(PedalManager* pdm) {
//
//    pedal = pdm;
//    connect(pedal, &PedalManager::pedalPressed, this, [this](int value) {
//        if (hasFocus()) {
//            if (value == 2) {
//                pressAction();
//            }
//        }
//    });
//    connect(pedal, &PedalManager::pedalReleased, this, [this](int value) {
//        if (hasFocus()) {
//            if (value == 2) {
//                releaseAction();
//            }
//        }
//    });
//
//}
//
//
//void ReferenceSetupWidget::createUI() {
//
//    QVBoxLayout* vLayout = new QVBoxLayout(this);
//    vLayout->setSpacing(10);
//
//    QSpacerItem* verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
//    vLayout->addItem(verticalSpacer);
//
//    illustrations.push_back(QIcon(":images/Still.svg").pixmap(QSize(300, 200)));
//    illustrations.push_back(QIcon(":images/Forward.svg").pixmap(QSize(300, 200)));
//    illustrations.push_back(QIcon(":images/Up.svg").pixmap(QSize(300, 200)));
//    illustrations.push_back(QIcon(":images/Connect.svg").pixmap(QSize(100, 100)));
//    illustrations.push_back(QIcon(":images/Valid.svg").pixmap(QSize(100, 100)));
//
//    imageLabel = new QLabel(this);
//    QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Maximum);
//    sizePolicy.setHeightForWidth(imageLabel->sizePolicy().hasHeightForWidth());
//    imageLabel->setSizePolicy(sizePolicy);
//
//    imageLabel->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
//    imageLabel->setAlignment(Qt::AlignmentFlag::AlignHCenter);
//    //imageLabel->setStyleSheet(QString::fromUtf8("background-color: rgb(85, 255, 0);"));
//    imageLabel->setPixmap(illustrations[0]);
//    vLayout->addWidget(imageLabel);
//
//
//
//
//
//    instructions = new QLabel(this);
//    instructions->setWordWrap(true);
//
//    instructions->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
//    instructions->setAlignment(Qt::AlignmentFlag::AlignHCenter);
//    instructions->setSizePolicy(sizePolicy);
//    instructions->setMinimumSize(QSize(0, 40));
//
//    vLayout->addWidget(instructions);
//
//
//    errors = new QLabel(this);
//    errors->setWordWrap(true);
//    // Set text color fo error
//    QPalette palette;
//    QBrush brush(QColor(100, 0, 0, 255));
//    brush.setStyle(Qt::SolidPattern);
//    palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
//    palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
//    errors->setPalette(palette);
//    errors->setSizePolicy(sizePolicy);
//
//    errors->hide();
//
//    vLayout->addWidget(errors);
//
//
//    QSpacerItem* verticalSpacer2 = new QSpacerItem(100, 40, QSizePolicy::Policy::Minimum, QSizePolicy::Policy::Expanding);
//    vLayout->addItem(verticalSpacer2);
//
//
//
//    setLayout(vLayout);
//}
//
//void ReferenceSetupWidget::show(){
//    if(tracker == nullptr){
//        instructions->setText("The ps-tech tracker is not detected");
//        imageLabel->setPixmap(illustrations[3]);
//    }
//    else if (!tracker->getTrackerConnected())
//    {
//        instructions->setText("Please click \"connect tracker\" and try again.");
//        imageLabel->setPixmap(illustrations[3]);
//    }
//    else
//        continueCalib();
//    
//    QWidget::show();
//    setFocus();
//};
//
//void ReferenceSetupWidget::resetState(){
//    origin.reset();
//    forwards.reset();
//    up.reset();
//    state = calibState::Idle;
//}
//
//void ReferenceSetupWidget::pressAction() {
//    if (state == calibState::Stopped) {
//        resetState();
//        hide();
//        parentWidget()->setFocus();
//    }
//    else
//        startMeasurement();
//}
//
//void ReferenceSetupWidget::releaseAction() {
//    stopMeasurement();
//}
//
//
//bool ReferenceSetupWidget::eventFilter(QObject* target, QEvent* event) {
//
//    switch (event->type())
//    {
//    case QEvent::FocusOut: {
//        hide();
//        resetState();
//        break;
//    }
//    case QEvent::KeyPress:
//    {
//        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
//        int key = keyEvent->key();
//
//        if (key == Qt::Key_Space)
//        {
//            if(!keyEvent->isAutoRepeat()) pressAction();
//
//            return true;
//        }
//
//        break;
//    }
//    case QEvent::KeyRelease:
//    {
//        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
//        int key = keyEvent->key();
//
//        if (key == Qt::Key_Space)
//        {
//            if (!keyEvent->isAutoRepeat()) releaseAction();
//            
//            return true;
//        }
//
//        break;
//    }
//    }
//
//    return QObject::eventFilter(target, event);
//}
//
//void ReferenceSetupWidget::continueCalib()
//{
//    if (!origin) {
//        qDebug() << "Suggesting origin";
//        state = calibState::Origin;
//        instructions->setText("Place the target in the middle of the are and press the left pedal to define the origin.");
//        imageLabel->setPixmap(illustrations[0]);
//        return;
//    }
//    if (!forwards) {
//        qDebug() << "Suggesting forwards";
//        state = calibState::Forwards;
//        instructions->setText("Place your tracker in the center of the area, hold the left pedal,"
//            "and move the tracker in a straight, forward direction, perpendicular to the screen. \n"
//            "Then, at the end of the trajectory, release the left pedal and move on to the next step.");
//        imageLabel->setPixmap(illustrations[1]);
//        return;
//    }
//    if (!up) {
//        qDebug() << "Suggesting up";
//        state = calibState::Up; 
//        instructions->setText("Place your tracker in the center of the area, hold the left pedal,"
//            "and move the tracker in a straight, upward direction, parallel to the screen. \n"
//            "Then, at the end of the trajectory, release the left pedal and move on to the next step.");
//        imageLabel->setPixmap(illustrations[2]);
//        return;
//    }
//
//    state = calibState::Stopped;
//
//    // We can now calculate the reference matrix
//    QMatrix4x4 ref;
//    ref.translate(*origin);
//    ref.rotate(QQuaternion::fromDirection(*forwards, *up)); // minus quaternion ?
//
//
//
//    try {
//        tracker->setTrackerReference(ref, true);
//        instructions->setText("The reference was set correctly ! \nYou can go back to the VolumViewer with the left pedal.");
//        imageLabel->setPixmap(illustrations[4]);
//    }
//    catch (PSTech::TrackerException err) {
//        show();
//
//        instructions->setText("Error, Press R to try angain.");
//        errors->setText("The data that was collected was invalid and the calibration failed.");
//    }
//
//
//}
//
//
//void ReferenceSetupWidget::startMeasurement()
//{
//
//    if (state == calibState::Stopped) return;
//
//    QMatrix4x4 pose;
//    tracker->GetTargetMatrix(targetIndex, pose);
//
//    errors->hide();
//    switch (state)
//    {
//    case calibState::Origin:
//    {
//        if (tracker->poseIsLive(targetIndex)) {
//            qDebug() << "Reading origin";
//            origin = pose.column(3).toVector3D();
//
//            state = calibState::Idle;
//            continueCalib();
//        }
//        else
//        {
//            errors->show();
//            errors->setText("The target is not detected.Make sure it is in view of the tracker and try again.");
//        }
//
//        break;
//    }
//    case calibState::Forwards:
//    {
//        if (tracker->poseIsLive(targetIndex)) {
//            qDebug() << "starting forward";
//            bufferVector = pose.column(3).toVector3D();
//        }
//        else
//        { 
//            errors->show();
//            errors->setText("The target is not detected. Make sure it is in view of the tracker and try again.");
//        }
//        break;
//    }
//    case calibState::Up:
//    {
//        if (tracker->poseIsLive(targetIndex)) {
//            qDebug() << "Starting up";
//            bufferVector = pose.column(3).toVector3D();
//        }
//        else 
//        {
//            errors->show();
//            errors->setText("The target is not detected. Make sure it is in view of the tracker and try again.");
//        }
//        break;
//    }
//    }
//
//}
//
//void ReferenceSetupWidget::stopMeasurement()
//{
//    // Proceed if a starting measurement was made
//    if (!bufferVector) return;
//
//    QMatrix4x4 pose;
//    tracker->GetTargetMatrix(targetIndex, pose);
//
//    switch (state)
//    {
//    case calibState::Forwards:
//    {
//        qDebug() << "Reading forwards";
//        // Just taking the last position, whether live or not
//        forwards = pose.column(3).toVector3D() - *bufferVector;
//        bufferVector.reset();
//        break;
//    }
//    case calibState::Up:
//    {
//        qDebug() << "Reading up";
//        // Just taking the last position, whether live or not
//        up = pose.column(3).toVector3D() - *bufferVector;
//        bufferVector.reset();
//        break;
//    }
//    }
//
//    state = calibState::Idle;
//    continueCalib();
//}
