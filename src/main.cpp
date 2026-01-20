#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define DEVICE_NAME "ESP32_BLE_ELM327"
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHARACTERISTIC_RX "12345678-1234-1234-1234-1234567890ac"
#define CHARACTERISTIC_TX "12345678-1234-1234-1234-1234567890ad"

#define MAX_CMD_LEN 64 // plus grand pour éviter overflow si plusieurs commandes

BLECharacteristic *pTxCharacteristic = nullptr;

char rxBuffer[MAX_CMD_LEN];
uint8_t rxIndex = 0;

// ---------------- Server callbacks ----------------
class MyServerCallbacks : public BLEServerCallbacks
{
public:
    void onConnect(BLEServer *pServer) override
    {
        Serial.println("[BLE] Connected");
    }

    void onDisconnect(BLEServer *pServer) override
    {
        Serial.println("[BLE] Disconnected");
        pServer->getAdvertising()->start();
        Serial.println("[BLE] Ready for new connection");
    }
};

// ---------------- RX callbacks ----------------
class RXCallbacks : public BLECharacteristicCallbacks
{
public:
    void onWrite(BLECharacteristic *pChar) override
    {
        if (!pChar)
            return;

        String val = pChar->getValue();
        size_t len = val.length();
        const char *data = val.c_str();

        // Debug
        Serial.println("==== BLE RX ====");
        Serial.print("HEX : ");
        for (size_t i = 0; i < len; i++)
            Serial.printf("%02X ", (uint8_t)data[i]);
        Serial.println();

        Serial.print("ASCII : ");
        for (size_t i = 0; i < len; i++)
        {
            char c = data[i];
            if (c >= 32 && c <= 126)
                Serial.print(c);
            else
                Serial.print('.');
        }
        Serial.println("\n================");

        // Ajouter chaque caractère au buffer et traiter toutes les commandes présentes
        for (size_t i = 0; i < len; i++)
        {
            char c = data[i];

            if (c == '\n')
                continue; // ignorer LF
            if (c == '\r')
            {
                if (rxIndex > 0)
                {
                    rxBuffer[rxIndex] = 0; // null terminate
                    processCommand(rxBuffer, rxIndex);
                    rxIndex = 0; // reset pour la prochaine commande
                }
            }
            else
            {
                if (rxIndex < MAX_CMD_LEN - 1)
                {
                    rxBuffer[rxIndex++] = c;
                }
                else
                {
                    rxIndex = 0; // overflow -> reset
                }
            }
        }
    }

    void processCommand(char *cmd, uint8_t len)
    {
        delay(10);

        const char *response = nullptr;
        size_t respLen = 0;

        // Gestion des commandes ELM327 minimales
        if (strcmp(cmd, "ATZ") == 0)
        {
            response = "ELM327 v1.5\r>";
            respLen = 12;
            //respLen = strlen(response);
            delay(500); // petit délai pour reset
        }
        else if (strcmp(cmd, "ATI") == 0)
        {
            response = "ELM327 v1.5\r>";
            respLen = 12;
            //respLen = strlen(response);
        }
        else if (strcmp(cmd, "ATE0") == 0 || strcmp(cmd, "ATL0") == 0 ||
                 strcmp(cmd, "ATS0") == 0 || strcmp(cmd, "ATH1") == 0 ||
                 strcmp(cmd, "ATSP6") == 0 || strcmp(cmd, "ATCAF0") == 0)
        {
            response = "OK\r>";
            respLen = 4;
            //respLen = strlen(response);
        }
        else if (strcmp(cmd, "ATDPN") == 0)
        {
            response = "A6\r>";
            respLen = 4;
            //respLen = strlen(response);
        }
        else if (strcmp(cmd, "ATDP") == 0)
        {
            response = "ISO 15765-4 (CAN 11/500)\r>";
            respLen = 26;
            //respLen = strlen(response);
        }
        else if (strcmp(cmd, "ATRV") == 0)
        {
            response = "13.8V\r>";
            respLen = 7;
            //respLen = strlen(response);            
        }
        else if (strcmp(cmd, "0100") == 0)
        {
            response = "7E8 06 41 00 FF FF FF FF\r>";
            // respLen = 26;
            respLen = strlen(response);
        }
        else if (strcmp(cmd, "010C") == 0)
        {
            response = "7E8 04 41 0C 0B B8\r>";
            // respLen = 20;
            respLen = strlen(response);
        }
        else if (strcmp(cmd, "010D") == 0)
        {
            response = "7E8 03 41 0D 64\r>";
            // respLen = 17;
            respLen = strlen(response);
        }
        else
        {
            response = "?\r>";
            // respLen = 3;
            respLen = strlen(response);
        }

        if (response)
        {
            //respLen = strlen(response);
            sendResponse(response, respLen);
            respLen = 0; response = nullptr; // reset
        }
    }

    void sendResponse(const char *resp, size_t len)
    {
        Serial.println("==== BLE TX ====");
        Serial.print("HEX : ");
        for (size_t i = 0; i < len; i++)
            Serial.printf("%02X ", (uint8_t)resp[i]);
        Serial.println("\nLength: " + String(len));

        Serial.print("ASCII : ");
        for (size_t i = 0; i < len; i++)
        {
            char c = resp[i];
            if (c >= 32 && c <= 126)
                Serial.print(c);
            else
                Serial.print('.');
        }
        Serial.println("\n================");

        // Envoi via BLE
        if (pTxCharacteristic)
        {
            pTxCharacteristic->setValue((uint8_t *)resp, len);
            pTxCharacteristic->notify();
        }
    }
};

// ---------------- Setup ----------------
void setup()
{
    Serial.begin(115200);
    Serial.println("ESP32 BLE ELM327 Starting...");

    BLEDevice::init(DEVICE_NAME);
    BLEServer *server = BLEDevice::createServer();
    server->setCallbacks(new MyServerCallbacks());

    BLEService *service = server->createService(SERVICE_UUID);

    pTxCharacteristic = service->createCharacteristic(
        CHARACTERISTIC_TX,
        BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic->addDescriptor(new BLE2902());

    BLECharacteristic *rxCharacteristic = service->createCharacteristic(
        CHARACTERISTIC_RX,
        BLECharacteristic::PROPERTY_WRITE);
    rxCharacteristic->setCallbacks(new RXCallbacks());

    service->start();

    BLEAdvertising *adv = BLEDevice::getAdvertising();
    adv->addServiceUUID(SERVICE_UUID);
    adv->setScanResponse(false);
    adv->start();

    delay(500);
    Serial.println("BLE ready!");
}

void loop()
{
    delay(10);
}