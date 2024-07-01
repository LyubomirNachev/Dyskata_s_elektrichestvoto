/*
  Program to control LED (ON/OFF) from ESP32 using Serial Bluetooth
  by Daniel Carrasco -> https://www.electrosoftcloud.com/
*/
#include "BluetoothSerial.h" // We will include the Serial Bluetooth header
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
BluetoothSerial BT; // Bluetooth Object
String clientName = "ESP32_Skateboard";
bool connected;
int value;
int flag = 0;
int flag_stop = 0;
int flag_dir = 0;
void IRAM_ATTR toggle_brake();
void IRAM_ATTR toggle_stop();
void IRAM_ATTR toggle_dir();
uint8_t interval = 1000;
unsigned long previousTime = 0;
unsigned long currentTime;
void setup() {
  pinMode(27, INPUT);
  pinMode(25, INPUT);
  pinMode(35, INPUT);
  pinMode(32, INPUT);
  pinMode(14, OUTPUT);
  attachInterrupt(27, toggle_brake, RISING);
  attachInterrupt(25, toggle_stop, RISING);
  attachInterrupt(35, toggle_dir, RISING);
  Serial.begin(115200); // Initializing serial connection for debugging
  BT.begin("ESP32_client", true); // Name of your Bluetooth Device and in master mode
  Serial.println("Bluetooth Device is in master mode. Connecting to the host...");
  connected = BT.connect(clientName);
  if(connected) {
    Serial.println("Connected Succesfully!");
    digitalWrite(14, HIGH); 
  } else {
    while(!BT.connected(100)) {
      Serial.println("Failed to connect. Make sure remote device is available and in range, then restart app."); 
      digitalWrite(14, LOW);
      delay(100); 
      digitalWrite(14, HIGH); 
      delay(100); 
    }
  }
}

void IRAM_ATTR toggle_brake(){
 if (digitalRead(27) == HIGH && flag == 0){
    BT.write(97); // Send a in ASCII
    flag = 1;
    //delay(10);
  }else if(digitalRead(27) == HIGH && flag == 1){
    BT.write(98); // Send b in ASCII
    flag = 0;
    //delay(10);
  }
}

void IRAM_ATTR toggle_stop(){
 if (digitalRead(25) == HIGH && flag_stop == 0){
    BT.write(99); // Send c in ASCII
    flag_stop = 1;
    //delay(10);
  }else if(digitalRead(25) == HIGH && flag_stop == 1){
    BT.write(100); // Send d in ASCII
    flag_stop = 0;
    //delay(10);
  }
}

void IRAM_ATTR toggle_dir(){
 if (digitalRead(35) == HIGH && flag_dir == 0){
    BT.write(101); // Send e in ASCII
    flag_dir = 1;
    //delay(10);
  }else if(digitalRead(35) == HIGH && flag_dir == 1){
    BT.write(102); // Send f in ASCII
    flag_dir = 0;
    //delay(10);
  }
}


void speed(){
  currentTime=millis();
  if (currentTime - previousTime >= interval){
    value = analogRead(32);
    if (value < 97 || value > 102){ 
      BT.write(value);
    }
    previousTime = currentTime;
  }
}

void loop() {
  speed();
}
