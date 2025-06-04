#include "ReferenceSetup.h"

#include <QVBoxLayout>


ReferenceSetupWidget::ReferenceSetupWidget(QWidget* parent, PSTracker* trackerPtr) : QWidget(parent, Qt::Window)
{
    setWindowTitle("PS Tracker Reference Setup");
    resize(400, 350);

    setFocusPolicy(Qt::FocusPolicy::ClickFocus);
    installEventFilter(this);

    tracker = trackerPtr;

    // When the object is instanciated (VolumeViewer is opened)
    // Load back the reference systemn used in the previosu session
    setStoredReference();

    createUI();

}


void ReferenceSetupWidget::createUI() {

    QVBoxLayout* vLayout = new QVBoxLayout(this);

    illustrations.push_back(QIcon(":images/Still.svg").pixmap(QSize(300, 300)));
    illustrations.push_back(QIcon(":images/Forward.svg").pixmap(QSize(300, 300)));
    illustrations.push_back(QIcon(":images/Up.svg").pixmap(QSize(300, 300)));

    imageLabel = new QLabel(this);
    QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Maximum);
    sizePolicy.setHeightForWidth(imageLabel->sizePolicy().hasHeightForWidth());
    imageLabel->setSizePolicy(sizePolicy);

    imageLabel->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    imageLabel->setAlignment(Qt::AlignmentFlag::AlignHCenter | Qt::AlignmentFlag::AlignBottom);

    imageLabel->setPixmap(illustrations[0]);
    vLayout->addWidget(imageLabel);





    instructions = new QLabel(this);
    instructions->setWordWrap(true);

    instructions->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
    instructions->setAlignment(Qt::AlignmentFlag::AlignTop | Qt::AlignmentFlag::AlignHCenter);

    vLayout->addWidget(instructions);


    errors = new QLabel(this);
    errors->setWordWrap(true);
    // Set text color fo error
    QPalette palette;
    QBrush brush(QColor(100, 0, 0, 255));
    brush.setStyle(Qt::SolidPattern);
    palette.setBrush(QPalette::Active, QPalette::WindowText, brush);
    palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush);
    errors->setPalette(palette);

    errors->hide();

    vLayout->addWidget(errors);



    setLayout(vLayout);
}

void ReferenceSetupWidget::show(){
    if (!tracker->getTrackerConnected())
        instructions->setText("Please connect the tracker and try again.");
    else
        continueCalib();
    
    QWidget::show();
    setFocus();
};

void ReferenceSetupWidget::resetState(){
    origin.reset();
    forwards.reset();
    up.reset();
    state = calibState::Idle;
}


bool ReferenceSetupWidget::eventFilter(QObject* target, QEvent* event) {

    switch (event->type())
    {
    case QEvent::FocusOut: {
        hide();
        break;
    }
    case QEvent::KeyPress:
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        if (key == 'Q') {
            hide();
        }
        if (key == 'R') {
            resetState();
            continueCalib();
        }
        else if (key == ' ')
        {
            if (state == calibState::Stopped) {
                resetState();
                hide();
            }
            else if (!keyEvent->isAutoRepeat())
                startMeasurement();

            return true;
        }

        break;
    }
    case QEvent::KeyRelease:
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();

        if (key == ' ')
        {
            if (!keyEvent->isAutoRepeat())
                stopMeasurement();
            
            return true;
        }

        break;
    }
    }

    return QObject::eventFilter(target, event);
}

void ReferenceSetupWidget::continueCalib()
{
    if (!origin) {
        qDebug() << "Suggesting origin";
        state = calibState::Origin;
        instructions->setText("Place the tracker in the middle of the are and press space to define the origin.");
        imageLabel->setPixmap(illustrations[0]);
        return;
    }
    if (!forwards) {
        qDebug() << "Suggesting forwards";
        state = calibState::Forwards;
        instructions->setText("Place your tracker in the center of the area, hold space,"
            "and move the tracker in a straight, forward direction, perpendicular to the screen. \n"
            "Then, at the end of the trajectory, release the spacebar and move on to the next step.");
        imageLabel->setPixmap(illustrations[1]);
        return;
    }
    if (!up) {
        qDebug() << "Suggesting up";
        state = calibState::Up; 
        instructions->setText("Place your tracker in the center of the area, hold space,"
            "and move the tracker in a straight, upward direction, parallel to the screen. \n"
            "Then, at the end of the trajectory, release the spacebar and move on to the next step.");
        imageLabel->setPixmap(illustrations[2]);
        return;
    }

    state = calibState::Stopped;

    // We can now calculate the reference matrix
    QMatrix4x4 ref;
    ref.translate(*origin);
    ref.rotate(QQuaternion::fromDirection(*forwards, *up)); // minus quaternion ?

    try {
        tracker->setTrackerReference(ref, true);
        saveReference();
        instructions->setText("The reference was set correctly ! \nYou can go back to the VolumViewer with space,"
            "or you can start again by pressing the R key.");
    }
    catch (PSTech::TrackerException err) {
        show();

        instructions->setText("Error, Press R to try angain.");
        errors->setText("The data that was collected was invalid and the calibration failed.");
    }


}


void ReferenceSetupWidget::startMeasurement()
{

    if (state == calibState::Stopped) return;

    errors->hide();
    switch (state)
    {
    case calibState::Origin:
    {
        if (tracker->poseIsLive()) {
            qDebug() << "Reading origin";
            origin = tracker->GetTargetMatrix().column(3).toVector3D();

            state = calibState::Idle;
            continueCalib();
        }
        else
        {
            errors->show();
            errors->setText("The target is not detected.Make sure it is in view of the tracker and try again. \n"
                "Place the tracker in the middle of the area and press space to define the origin.");
        }

        break;
    }
    case calibState::Forwards:
    {
        if (tracker->poseIsLive()) {
            qDebug() << "starting forward";
            bufferVector = tracker->GetTargetMatrix().column(3).toVector3D();
        }
        else
        { 
            errors->show();
            errors->setText("The target is not detected. Make sure it is in view of the tracker and try again.");
        }
        break;
    }
    case calibState::Up:
    {
        if (tracker->poseIsLive()) {
            qDebug() << "Starting up";
            bufferVector = tracker->GetTargetMatrix().column(3).toVector3D();
        }
        else 
        {
            errors->show();
            errors->setText("The target is not detected. Make sure it is in view of the tracker and try again.");
        }
        break;
    }
    }

}

void ReferenceSetupWidget::stopMeasurement()
{
    // Proceed if a starting measurement was made
    if (!bufferVector) return;

    switch (state)
    {
    case calibState::Forwards:
    {
        qDebug() << "Reading forwards";
        // Just taking the last position, whether live or not
        forwards = tracker->GetTargetMatrix().column(3).toVector3D() - *bufferVector;
        bufferVector.reset();
        break;
    }
    case calibState::Up:
    {
        qDebug() << "Reading up";
        // Just taking the last position, whether live or not
        up = tracker->GetTargetMatrix().column(3).toVector3D() - *bufferVector;
        bufferVector.reset();
        break;
    }
    }

    state = calibState::Idle;
    continueCalib();
}


/**
* Stores the absolute reference that's contained in the tracker
*/
void ReferenceSetupWidget::saveReference() const {
    QMatrix4x4 matrixRef;

    matrixRef = tracker->GetReference();

    // Create and open a text file
    std::ofstream refFile(fileLoc);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            refFile << matrixRef(i,j);
        }
    }

    // Close the file
    refFile.close();
};

void ReferenceSetupWidget::setStoredReference() const {
    if (std::filesystem::is_regular_file(fileLoc)) {
        QMatrix4x4 ref;
        std::string lineText;

        std::ifstream MyReadFile(fileLoc);
        int i = 0;
        while (getline(MyReadFile, lineText) && i < 16) {
            ref(i / 3, i % 3);
            i++;
        }
        qDebug() << ref;

        tracker->setTrackerReference(ref, false);
    }
};