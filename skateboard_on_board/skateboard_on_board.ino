/*
  Program to control LED (ON/OFF) from ESP32 using Serial Bluetooth
  by Daniel Carrasco -> https://www.electrosoftcloud.com/
*/
#include "BluetoothSerial.h" // We will include the Serial Bluetooth header
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif
#define LED1 25
#define LED2 32
#define LED3 33

BluetoothSerial BT; // Bluetooth Object
void setup() {
  Serial.begin(9600); // Initializing serial connection for debugging
  BT.begin("ESP32_Skateboard"); // Name of your Bluetooth Device and in slave mode
  Serial.println("Bluetooth Device is Ready to Pair");
  pinMode (LED1, OUTPUT); // stop
  pinMode (LED2, OUTPUT); //brake
  pinMode (LED3, OUTPUT);  //dir
}
void loop() {
  if (BT.available()) // Check if we receive anything from Bluetooth
  {
    int incoming = BT.read(); // Read what we recevive 
    Serial.print("Received: ");
    Serial.println(incoming);
    if (incoming == 97){ // a in ASCII
      digitalWrite(LED2, HIGH); // LED On
      BT.println("LED turned ON"); // Send the text via BT Serial
    }
    if (incoming == 98){ // b in ASCII
      digitalWrite(LED2, LOW); // LED Off
      BT.println("LED turned OFF"); // Send the text via BT Serial
    }
    if (incoming == 99){ // c in ASCII
      digitalWrite(LED1, HIGH); // LED On
      BT.println("LED turned ON");
    }
    if (incoming == 100){ // d in ASCII
      digitalWrite(LED1, LOW); // LED Off
      BT.println("LED turned OFF");
    }
    if (incoming == 101){ // e in ASCII
      digitalWrite(LED3, HIGH); // LED On
      BT.println("LED turned ON");
    }
    if (incoming == 102){ // f in ASCII
      digitalWrite(LED3, LOW); // LED Off
      BT.println("LED turned OFF");
    }
  }
  delay(20);
}