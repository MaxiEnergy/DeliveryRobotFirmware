#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// задаем имена для портов
#define IN1 7
#define IN2 5
#define IN3 1
#define IN4 0

// ШИМ каналы
#define CHANNEL_IN1 0
#define CHANNEL_IN2 1
#define CHANNEL_IN3 2
#define CHANNEL_IN4 3

// Частота ШИМ и разрядность
const int freq = 5000;
const int resolution = 8;
int ledPin = 10;

bool deviceConnected;

BLECharacteristic* pMoveCharacteristic;

BLEServer* pServer;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
        deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
        deviceConnected = false;
    }
};

class MoveCharacteristicCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) {
        std::string value = pCharacteristic->getValue();
        if (value.length() > 0) {
            switch (value[0]) {
                case 0x01: // Влево
                    ledcWrite(CHANNEL_IN1, 0);
                    ledcWrite(CHANNEL_IN2, 255); 
                    ledcWrite(CHANNEL_IN3, 0);
                    ledcWrite(CHANNEL_IN4, 255);
                    break;
                case 0x02: // Вправо
                    ledcWrite(CHANNEL_IN1, 255); 
                    ledcWrite(CHANNEL_IN2, 0);
                    ledcWrite(CHANNEL_IN3, 255);
                    ledcWrite(CHANNEL_IN4, 0);
                    break;
                case 0x05: // Назад
                    ledcWrite(CHANNEL_IN1, 255); 
                    ledcWrite(CHANNEL_IN2, 0);
                    ledcWrite(CHANNEL_IN3, 0);
                    ledcWrite(CHANNEL_IN4, 255);
                    break;
                case 0x04: // Вперед
                    ledcWrite(CHANNEL_IN1, 0);
                    ledcWrite(CHANNEL_IN2, 255); 
                    ledcWrite(CHANNEL_IN3, 255);
                    ledcWrite(CHANNEL_IN4, 0);
                    break;
                case 0x00:
                    ledcWrite(CHANNEL_IN1, 0);
                    ledcWrite(CHANNEL_IN2, 0);
                    ledcWrite(CHANNEL_IN3, 0);
                    ledcWrite(CHANNEL_IN4, 0);
                    break;
                case 0x03:
                    digitalWrite(ledPin, HIGH);
                    break;
                case 0x06:
                    digitalWrite(ledPin, LOW);
                    break;
                default:
                    ledcWrite(CHANNEL_IN1, 0);
                    ledcWrite(CHANNEL_IN2, 0);
                    ledcWrite(CHANNEL_IN3, 0);
                    ledcWrite(CHANNEL_IN4, 0);
                    digitalWrite(ledPin, LOW);
                    break;
            }
        }
    }
};

void setup() {
    pinMode(IN1, OUTPUT);
    pinMode(IN2, OUTPUT);
    pinMode(IN3, OUTPUT);
    pinMode(IN4, OUTPUT);
    pinMode(ledPin, OUTPUT);

    // Настройка ШИМ
    ledcSetup(CHANNEL_IN1, freq, resolution);
    ledcSetup(CHANNEL_IN2, freq, resolution);
    ledcSetup(CHANNEL_IN3, freq, resolution);
    ledcSetup(CHANNEL_IN4, freq, resolution);
    
    // Привязка пинов к ШИМ каналам
    ledcAttachPin(IN1, CHANNEL_IN1);
    ledcAttachPin(IN2, CHANNEL_IN2);
    ledcAttachPin(IN3, CHANNEL_IN3);
    ledcAttachPin(IN4, CHANNEL_IN4);

    // Номер ровера
    int roverNumber = 1; // Задайте номер для вашего ровера, например 1 для "Rover Toy 001"

    // Генерация имени и UUID на основе номера ровера
    char roverName[12];
    sprintf(roverName, "Rover Toy %03d", roverNumber);
    
    char serviceUUID[37];
    char characteristicUUID[37];
    sprintf(serviceUUID, "0000170D-%04d-1000-8000-00805f9b34fb", roverNumber);
    sprintf(characteristicUUID, "00002A60-%04d-1000-8000-00805f9b34fb", roverNumber);

    // Инициализация BLE
    BLEDevice::init(roverName);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService* pMoveService = pServer->createService(BLEUUID(serviceUUID));
    pMoveCharacteristic = pMoveService->createCharacteristic(BLEUUID(characteristicUUID), BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
    pMoveCharacteristic->setCallbacks(new MoveCharacteristicCallbacks());
    pMoveService->start();
    BLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->addServiceUUID(pMoveService->getUUID());
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();
}

void loop() {
    if (!deviceConnected) {
        BLEDevice::startAdvertising();
    } 
    delay(1000);
}
