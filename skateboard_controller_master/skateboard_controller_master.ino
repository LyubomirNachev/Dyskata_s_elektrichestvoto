#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>
#include <BLE2901.h>

#define SERVICE_UUID        "63b5a895-bd3d-4d34-b360-607d92fb7f13"
#define BRAKE_CHAR_UUID     "38ee12c0-b3ff-40ec-a7fa-0c0307ad7032"
#define STOP_CHAR_UUID      "fb8cfd3d-260a-4e90-a55c-2ab854de8d0a"
#define DIR_CHAR_UUID       "4a5d63a1-1fae-493d-bf72-2b923b0756ee"
#define SPEED_CHAR_UUID     "81010060-39d8-494e-a11c-f071fa06fbbb"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristicBrake = NULL;
BLECharacteristic *pCharacteristicStop = NULL;
BLECharacteristic *pCharacteristicDir = NULL;
BLECharacteristic *pCharacteristicSpeed = NULL;

#define BRAKE_PIN 27
#define STOP_PIN 25
#define DIR_PIN 35
#define SPEED_PIN 32
#define LED1_PIN 14

String serverName = "ESP32_Skateboard_contoller";
bool deviceConnected = false;

volatile int flag_brake = 0;
volatile int flag_stop = 0;
volatile int flag_dir = 0;
void IRAM_ATTR toggle_brake();
void IRAM_ATTR toggle_stop();
void IRAM_ATTR toggle_dir();

const int PWM_FREQ = 19000;     
const int PWM_RESOLUTION = 12; 
const int MAX_DUTY_CYCLE = (int)(pow(2, PWM_RESOLUTION) - 1); 

class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Connected Succesfully!");
    digitalWrite(LED1_PIN, HIGH); 
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected!!!"); 
  }
};

void setup() {
  pinMode(BRAKE_PIN, INPUT);
  pinMode(STOP_PIN, INPUT);
  pinMode(DIR_PIN, INPUT);
  pinMode(SPEED_PIN, INPUT);
  pinMode(LED1_PIN, OUTPUT);
  attachInterrupt(BRAKE_PIN, toggle_brake, RISING);
  attachInterrupt(STOP_PIN, toggle_stop, RISING);
  attachInterrupt(DIR_PIN, toggle_dir, RISING);

  Serial.begin(115200);
  Serial.println("BLE is intilized, waiting for connection.");

  BLEDevice::init(serverName);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristicBrake = pService->createCharacteristic(
                                         BRAKE_CHAR_UUID,
                                         BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE
                                       );
  pCharacteristicStop = pService->createCharacteristic(
                                         STOP_CHAR_UUID,
                                         BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE
                                       );     
  pCharacteristicDir = pService->createCharacteristic(
                                         DIR_CHAR_UUID,
                                         BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE
                                       );   
  pCharacteristicSpeed = pService->createCharacteristic(
                                         SPEED_CHAR_UUID,
                                         BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_NOTIFY | BLECharacteristic::PROPERTY_INDICATE
                                       );  
                                                                                                
  pService->start();
  pCharacteristicBrake->setValue((uint8_t *)&flag_brake, 1); 
  pCharacteristicBrake->notify(); 
  pCharacteristicStop->setValue((uint8_t *)&flag_stop, 1); 
  pCharacteristicStop->notify();    
  pCharacteristicDir->setValue((uint8_t *)&flag_dir, 1); 
  pCharacteristicDir->notify();   
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);
  BLEDevice::startAdvertising();
  Serial.println("Characteristics defined! Now you can read it in your phone!");
}

void IRAM_ATTR toggle_brake(){
 if (digitalRead(BRAKE_PIN) == HIGH && flag_brake == 0){
    flag_brake = 1;
    pCharacteristicBrake->setValue((uint8_t *)&flag_brake, 1);
    pCharacteristicBrake->notify();
    
    Serial.println("on");

  }else if(digitalRead(BRAKE_PIN) == HIGH && flag_brake == 1){
    flag_brake = 0;
    Serial.println("off");
    pCharacteristicBrake->setValue((uint8_t *)&flag_brake, 1); 
    pCharacteristicBrake->notify();
  }
}

void IRAM_ATTR toggle_stop(){
 if (digitalRead(STOP_PIN) == HIGH && flag_stop == 0){
    flag_stop = 1;
    pCharacteristicStop->setValue((uint8_t *)&flag_stop, 1); 
    pCharacteristicStop->notify();
  }else if(digitalRead(STOP_PIN) == HIGH && flag_stop == 1){
    flag_stop = 0;
    pCharacteristicStop->setValue((uint8_t *)&flag_stop, 1); 
    pCharacteristicStop->notify();

  }
}

void IRAM_ATTR toggle_dir(){
 if (digitalRead(DIR_PIN) == HIGH && flag_dir == 0){
    flag_dir = 1;
    pCharacteristicDir->setValue((uint8_t *)&flag_dir, 1); 
    pCharacteristicDir->notify();
  }else if(digitalRead(DIR_PIN) == HIGH && flag_dir == 1){
    flag_dir = 0;
    pCharacteristicDir->setValue((uint8_t *)&flag_dir, 1); 
    pCharacteristicDir->notify();
  }
}


void speed(){
    int pot_value = analogRead(SPEED_PIN);
    pot_value = map(pot_value, 0, 4095, 0, MAX_DUTY_CYCLE); 
    Serial.print("Decimal:");
    Serial.print(pot_value);
    Serial.print("Hex:");
    Serial.println(pot_value, HEX);

    pCharacteristicSpeed->setValue((uint8_t *)&pot_value, 4); 
    pCharacteristicSpeed->notify();
    delay(50);
}

void loop() {
  if (deviceConnected){
    speed();
  }else{
    digitalWrite(LED1_PIN, LOW);
    delay(100); 
    digitalWrite(LED1_PIN, HIGH); 
    delay(100); 
    BLEDevice::startAdvertising();
  }
}
