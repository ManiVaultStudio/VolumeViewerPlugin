#include <Controllers/Pedal.h>



PedalManager::PedalManager(QObject* parent, QTimer* timer)
    : QObject(parent), joystick(nullptr), sdlEventTimer(timer)
{
    if (SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_EVENTS) < 0) {
        qDebug() << "SDL Init failed:" << SDL_GetError();
        return;
    }

    int* numJoysticks = new int();
    SDL_JoystickID* ids = SDL_GetJoysticks(numJoysticks);
    if (*numJoysticks < 1) {
        qDebug() << "No SDL joystick-compatible devices found.";
        return;
    }

    qDebug() << "Found" << *numJoysticks << "joystick(s)";
    joystick = SDL_OpenJoystick(ids[0]);
    if (!joystick) {
        qDebug() << "Failed to open joystick:" << SDL_GetError();
        return;
    }
    delete numJoysticks;

    // Setup event pump timer (could also be integrated with your main Qt event loop)
    connect(sdlEventTimer, &QTimer::timeout, this, &PedalManager::pumpSdlEvents);
}

PedalManager::~PedalManager() {
    sdlEventTimer->stop();
    if (joystick) {
        SDL_CloseJoystick(joystick);
    }
    SDL_Quit();  // If SDL is shared across other parts of your app, remove this
}

void PedalManager::pumpSdlEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_JOYSTICK_BUTTON_DOWN:
            emit pedalPressed(event.jbutton.button);
            break;
        case SDL_EVENT_JOYSTICK_BUTTON_UP:
            emit pedalReleased(event.jbutton.button);
            break;
        default:
            break;
        }
    }
}