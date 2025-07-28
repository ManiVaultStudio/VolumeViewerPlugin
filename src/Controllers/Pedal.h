#pragma once

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <SDL3/SDL.h>

class PedalManager : public QObject {
    Q_OBJECT

public:
    explicit PedalManager(QObject* parent = nullptr, QTimer* timer = nullptr);
    ~PedalManager();

signals:
    void pedalPressed(int button);
    void pedalReleased(int button);

private slots:
    void pumpSdlEvents();

private:
    SDL_Joystick* joystick;
    QTimer* sdlEventTimer;
};


//#include <QtGamepadLegacy/QGamepad> NOT WORKING
//#include <QtGamepadLegacy/QGamepadManager>
//#include <QDebug>
//
//class KinesisPedalDetector : public QObject {
//    Q_OBJECT
//
//private:
//    QGamepad* gamepad = nullptr;
//
//public:
//    KinesisPedalDetector(QObject* parent = nullptr) : QObject(parent) {
//        // Connect to gamepad button press (assuming pedal appears as a button)
//        QGamepadManager* manager = QGamepadManager::instance();
//
//        // Use first connected gamepad ID
//        auto gamepads = manager->connectedGamepads();
//        if (!gamepads.isEmpty()) {
//            int deviceId = *gamepads.begin();
//
//            qDebug() << "Gamepad connected : " << deviceId;
//
//            gamepad = new QGamepad(deviceId, this);
//
//            connect(gamepad, &QGamepad::buttonAChanged, this, &KinesisPedalDetector::onButtonAChanged);
//            // Connect other buttons/axes as needed
//        }
//        else {
//            qDebug() << "No gamepads connected";
//        }
//    }
//
//private slots:
//    void onButtonAChanged(bool pressed) {
//        qDebug() << "Pedal (button A) pressed? " << pressed;
//        if (pressed) {
//            emit pedalPressed(1);
//        }
//    }
//
//signals:
//    void pedalPressed(int value);
//
//};




//
//#include <hidapi.h> WORKING
//#include <QTimer>
//#include <QDebug>
//#include <QObject>
//
//class KinesisPedalDetector : public QObject {
//    Q_OBJECT
//
//private:
//    hid_device* device;
//    QTimer* pollTimer;
//    static const unsigned short KINESIS_VID = 0x0FC5;
//    static const unsigned short KINESIS_PID = 0xB030;
//    int pedalState = 0;
//
//public:
//    KinesisPedalDetector(QObject* parent = nullptr) : QObject(parent) {
//        device = nullptr;
//        pollTimer = nullptr;
//
//        // Initialize the HID API
//        if (hid_init() != 0) {
//            qDebug() << "Failed to initialize HID API";
//            return;
//        }
//
//        // Try to open the kinesis pedal device
//        device = hid_open(KINESIS_VID, KINESIS_PID, nullptr);
//
//        if (device) {
//            qDebug() << "Kinesis pedal connected successfully!";
//
//            // Get device info
//            wchar_t wstr[256];
//            if (hid_get_manufacturer_string(device, wstr, 256) == 0) {
//                QString manufacturer = QString::fromWCharArray(wstr);
//                qDebug() << "Manufacturer:" << manufacturer;
//            }
//
//            if (hid_get_product_string(device, wstr, 256) == 0) {
//                QString product = QString::fromWCharArray(wstr);
//                qDebug() << "Product:" << product;
//            }
//
//            // Set up polling timer
//            pollTimer = new QTimer(this);
//            connect(pollTimer, &QTimer::timeout, this,
//                &KinesisPedalDetector::checkPedalInput);
//            pollTimer->start(100); // Poll every 10ms
//        }
//        else {
//            qDebug() << "Failed to open Kinesis pedal device";
//            listAllHidDevices(); // Debug: show all available devices
//        }
//    }
//
//    ~KinesisPedalDetector() {
//        if (pollTimer) {
//            pollTimer->stop();
//        }
//        if (device) {
//            hid_close(device);
//        }
//        hid_exit();
//    }
//
//    bool isConnected() const {
//        return device != nullptr;
//    }
//
//    void listAllHidDevices() {
//        qDebug() << "=== Available HID Devices ===";
//        struct hid_device_info* devs, * cur_dev;
//
//        devs = hid_enumerate(0x0, 0x0);
//        cur_dev = devs;
//
//        while (cur_dev) {
//            qDebug() << QString("VID: 0x%1, PID: 0x%2")
//                .arg(cur_dev->vendor_id, 4, 16, QChar('0'))
//                .arg(cur_dev->product_id, 4, 16, QChar('0'));
//
//            if (cur_dev->manufacturer_string) {
//                QString manufacturer =
//                    QString::fromWCharArray(cur_dev->manufacturer_string);
//                qDebug() << "  Manufacturer:" << manufacturer;
//            }
//
//            if (cur_dev->product_string) {
//                QString product =
//                    QString::fromWCharArray(cur_dev->product_string);
//                qDebug() << "  Product:" << product;
//            }
//
//            cur_dev = cur_dev->next;
//        }
//        hid_free_enumeration(devs);
//        qDebug() << "=== End of HID Devices ===";
//    }
//
//private slots:
//    void checkPedalInput() {
//        if (!device) return;
//
//        unsigned char buffer[64];
//        int bytesRead = hid_read_timeout(device, buffer,
//            sizeof(buffer), 100);
//
//        if (bytesRead > 0) {
//            // Debug: print raw data
//            QString hexData;
//            for (int i = 0; i < bytesRead; i++) {
//                hexData += QString("%1 ").arg(buffer[i], 2, 16,
//                    QChar('0'));
//            }
//
//            processPedalData(buffer, bytesRead);
//        }
//    }
//
//    void processPedalData(unsigned char* data, int length) {
//        if (length < 3) return;
//
//        // Parse the pedal data - this will depend on your specific device
//        // Start with simple parsing and adjust based on debug output
//
//        //for (int i = 0; i < length; i++) {
//        //    if (data[i] != 0) {
//        //        qDebug() << "Non-zero byte at position" << i << ":" <<
//        //            data[i];
//        //        emit pedalPressed(i + 1, data[i]); // pedal ID, value
//        //    }
//        //}
//
//        // Result : The pedal info is encoded in the i=2 byte and takes values 1,2,4 from left to right
//        if (data[2] != pedalState) {
//            pedalState = data[2];
//            emit pedalPressed(data[2]); // pedal ID, value
//        }
//    }
//
//signals:
//    void pedalPressed(int value);
//};