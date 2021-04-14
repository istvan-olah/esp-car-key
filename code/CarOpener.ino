#define OPEN_LED 2
#define CLOSE_LED 4
#define OPEN 22
#define CLOSE 23
#define BUZZER 13
#define SETUP_BUTTON 21

#include <Preferences.h>
Preferences prefs;

String getValue(String data, char separator, int index);
void configure();
void saveSettings();
void loadSettings();

struct Settings {
  String bleAddress;
  int openDelay;
  int openTimeout;
  int closedDelay;
  int closedTimeout;
  int notFoundInARow;
  int onDelay;
};

Settings* settings = NULL;

#include <BluetoothSerial.h>

BluetoothSerial* SerialBT;

#include <BLEDevice.h>

static BLERemoteCharacteristic* pRemoteCharacteristic;

BLEScan* pBLEScan;
BLEScanResults foundDevices;

static BLEAddress *Server_BLE_Address;
String Scaned_BLE_Address;
void checkRestart();
boolean paired = false;
boolean found = false;
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks 
{
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      Server_BLE_Address = new BLEAddress(advertisedDevice.getAddress());
      Scaned_BLE_Address = Server_BLE_Address->toString().c_str();
      
      if (Scaned_BLE_Address == settings->bleAddress) {
        found=true;
      }
    }
};

void buzz() {
  digitalWrite(BUZZER, HIGH);
  delay(50);
  digitalWrite(BUZZER, LOW);
}

void setup() {
    loadSettings();

    pinMode(OPEN,OUTPUT);
    pinMode(CLOSE,OUTPUT);
    digitalWrite(OPEN, HIGH);
    digitalWrite(CLOSE, HIGH);
    pinMode(OPEN_LED,OUTPUT);
    pinMode(CLOSE_LED,OUTPUT);
    pinMode(BUZZER,OUTPUT);
    buzz();
    pinMode(SETUP_BUTTON,INPUT);
    if(digitalRead(SETUP_BUTTON) == HIGH) {
      configure();
      ESP.restart();
    }
    
    BLEDevice::init("");
    pBLEScan = BLEDevice::getScan(); //create new scan
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks()); //Call the class that is defined above 
    pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster

}

bool searchDevice() {
  found = false;
  pBLEScan->start(3); 
  return found;
}

void foundDevice() {
  paired = true;
  digitalWrite(OPEN_LED, HIGH);
  digitalWrite(OPEN, LOW);
  delay(settings->onDelay);
  digitalWrite(OPEN_LED, LOW);
  digitalWrite(OPEN, HIGH);
  buzz();
  delay(settings->openTimeout);
}

void deviceLost() {
  int retries = settings->notFoundInARow;
  while (retries > 0) {
    searchDevice();
    if (found) {
      return;  
    }
    retries--;
  }
  paired = false;
  digitalWrite(CLOSE_LED, HIGH);
  digitalWrite(CLOSE, LOW);
  delay(settings->onDelay);
  digitalWrite(CLOSE_LED, LOW);
  digitalWrite(CLOSE, HIGH);
  buzz();
  buzz();
}

void loop() {
  searchDevice();
  if(found && !paired) {
      foundDevice();
  }
  else if(!found && paired) {
      deviceLost();
  }
  if(paired){
    delay(settings->openDelay);
  } else {
    delay(settings->closedDelay);
  }
}
