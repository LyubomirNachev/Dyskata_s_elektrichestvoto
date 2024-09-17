#include <BLEDevice.h>

static BLEUUID SERVICE_UUID("63b5a895-bd3d-4d34-b360-607d92fb7f13");
static BLEUUID BRAKE_CHAR_UUID("38ee12c0-b3ff-40ec-a7fa-0c0307ad7032");
static BLEUUID STOP_CHAR_UUID("fb8cfd3d-260a-4e90-a55c-2ab854de8d0a");
static BLEUUID DIR_CHAR_UUID("4a5d63a1-1fae-493d-bf72-2b923b0756ee");
static BLEUUID SPEED_CHAR_UUID("81010060-39d8-494e-a11c-f071fa06fbbb");

#define STOP_PIN 25
#define BRAKE_PIN 32
#define DIR_PIN 33
#define SPEED_PIN 26 // brown cable
#define SPEED_MONITOR 27 //blue cable
#define POT_PIN 14

#define bleServerName "ESP32_Skateboard_contoller"
static BLEAdvertisedDevice *myDevice;

static boolean doConnect = false;
static boolean connected = false;
static boolean reconnect = false;

static BLEAddress *pServerAddress;
static BLERemoteCharacteristic* pBrakeCharacteristic;
static BLERemoteCharacteristic* pStopCharacteristic;
static BLERemoteCharacteristic* pDirCharacteristic;
static BLERemoteCharacteristic* pSpeedCharacteristic;

const uint8_t notificationOn[] = {0x1, 0x0, 0x0, 0x0};

uint8_t* Brake_info;
uint8_t* Stop_info;
uint8_t* Dir_info;
uint32_t* Speed_info;

boolean newBrake_info = false;
boolean newStop_info = false;
boolean newDir_info = false;
boolean newSpeed_info = false;

const int PWM_FREQ = 19000;     
const int PWM_RESOLUTION = 12; 
const int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1); 

bool connectToServer(BLEAddress pAddress);

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient *pclient) {}

  void onDisconnect(BLEClient *pclient) {
    connected = false;
    reconnect = true;
    Serial.println("onDisconnect");
  }
};

bool connectToServer(BLEAddress pAddress) {
  Serial.print("Forming a connection to ");

  BLEClient *pClient = BLEDevice::createClient();
  Serial.println(" - Created client");

  pClient->setClientCallbacks(new MyClientCallback());

  pClient->connect(pAddress);  
  Serial.println(" - Connected to server");

  BLERemoteService *pRemoteService = pClient->getService(SERVICE_UUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our service");

  pBrakeCharacteristic = pRemoteService->getCharacteristic(BRAKE_CHAR_UUID);
  pStopCharacteristic = pRemoteService->getCharacteristic(STOP_CHAR_UUID);
  pDirCharacteristic = pRemoteService->getCharacteristic(DIR_CHAR_UUID);
  pSpeedCharacteristic = pRemoteService->getCharacteristic(SPEED_CHAR_UUID);
  if (pBrakeCharacteristic == nullptr || pStopCharacteristic == nullptr || pDirCharacteristic == nullptr || pSpeedCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");
    pClient->disconnect();
    return false;
  }
  Serial.println(" - Found our characteristic");

  pBrakeCharacteristic->registerForNotify(BrakeNotifyCallback);
  pStopCharacteristic->registerForNotify(StopNotifyCallback);
  pDirCharacteristic->registerForNotify(DirNotifyCallback);
  pSpeedCharacteristic->registerForNotify(SpeedNotifyCallback);

  connected = true;
  return true;
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if (advertisedDevice.getName() == bleServerName) { 
      advertisedDevice.getScan()->stop(); 
      pServerAddress = new BLEAddress(advertisedDevice.getAddress()); 
      doConnect = true; 
      Serial.println("Device found. Connecting!");
    }
  }
};

static void BrakeNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                        uint8_t* pData, size_t length, bool isNotify) {
  Brake_info = pData;
  newBrake_info = true;
}
static void StopNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                    uint8_t* pData, size_t length, bool isNotify) {
  Stop_info = pData;
  newStop_info = true;
}
static void DirNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                    uint8_t* pData, size_t length, bool isNotify) {
  Dir_info = pData;
  newDir_info = true;
}
static void SpeedNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, 
                                    uint8_t* pData, size_t length, bool isNotify) {
  Speed_info = (uint32_t*)pData;
  newSpeed_info = true;
}

void setup() {
  Serial.begin(9600); 
  Serial.println("Starting Arduino BLE Client application...");
  BLEDevice::init("");

  BLEScan* pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  pBLEScan->start(30);


  pinMode(STOP_PIN, OUTPUT); // stop
  pinMode(BRAKE_PIN, OUTPUT); //brake
  pinMode(DIR_PIN, OUTPUT);  //dir
  pinMode(SPEED_PIN, OUTPUT); //pwm signal
  pinMode(SPEED_MONITOR, INPUT); //rpm out
  ledcAttach(SPEED_PIN, PWM_FREQ, PWM_RESOLUTION);
}

void loop() {
  if (doConnect == true || reconnect == true) {
    if (connectToServer(*pServerAddress)) {
      Serial.println("We are now connected to the BLE Server.");

      pBrakeCharacteristic->writeValue((uint8_t*)notificationOn, 2, true);
      pStopCharacteristic->writeValue((uint8_t*)notificationOn, 2, true);
      pDirCharacteristic->writeValue((uint8_t*)notificationOn, 2, true);
      pSpeedCharacteristic->writeValue((uint8_t*)notificationOn, 2, true);
      connected = true;
      reconnect = false;
    } else {
      Serial.println("We have failed to connect to the server; Restart your device to scan for nearby BLE server again.");
    }
    doConnect = false;
  }else{
    
  }
  if (newStop_info){
    newStop_info = false;
    Serial.print("Stop:");
    Serial.println(*Stop_info);
    digitalWrite(STOP_PIN, *Stop_info); 
  }else if(newBrake_info){
    newBrake_info=false;
    Serial.print("Brake:");
    Serial.println(*Brake_info);
    digitalWrite(BRAKE_PIN, *Brake_info); 
  }else if(newDir_info){
    newDir_info=false;
    Serial.print("Dir:");
    Serial.println(*Dir_info);
    digitalWrite(DIR_PIN, *Dir_info); 
  }else if(newSpeed_info){
    newSpeed_info=false;
    Serial.print("Speed:");
    Serial.println(*Speed_info);
    ledcWrite(SPEED_PIN, *Speed_info);
  }
}