#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define RST_PIN_1       22
#define SS_PIN_1        21
#define RST_PIN_2       17
#define SS_PIN_2        16
#define RST_PIN_3       32
#define SS_PIN_3        15
#define RST_PIN_4       5
#define SS_PIN_4        4

const char* ssid = "iPhone de Jose";
const char* password = "sapaperra";

// Configuración MQTT
const char* mqttServer = "172.20.10.2";
const int mqttPort = 1883;
WiFiClient espClient;
PubSubClient client(espClient);

MFRC522 mfrc522_1(SS_PIN_1, RST_PIN_1);
MFRC522 mfrc522_2(SS_PIN_2, RST_PIN_2);
MFRC522 mfrc522_3(SS_PIN_3, RST_PIN_3);
MFRC522 mfrc522_4(SS_PIN_4, RST_PIN_4);

///////////////////////////////////////////
// Servo setup
const int SERVO_IN_PIN = 14;
const int SERVO_OUT_PIN = 12;
const int PINZA_SCOO_PIN = 27;
const int PINZA_BICI_PIN = 33;
const int FIN_CAR_PIN = 2;

Servo servoIn;
Servo servoOut;
Servo pinzaScoo;
Servo pinzaBici;

int initposIn = 140;
int finalposIn = 35;
int initposOut = 90;
int finalposOut = 0;
int initposPinzaScoo = 140;
int finalposPinzaScoo = 35;
int initposPinzaBici = 140;
int finalposPinzaBici = 35;
///////////////////////////////////////////

int distancia = 0;
int park_bici = 0;

void setup () {
  Serial.begin(115200);
  while(!Serial);
  SPI.begin();
  mfrc522_1.PCD_Init();
  mfrc522_2.PCD_Init();
  mfrc522_3.PCD_Init();
  mfrc522_4.PCD_Init();
  mfrc522_1.PCD_DumpVersionToSerial();
  mfrc522_2.PCD_DumpVersionToSerial();
  mfrc522_3.PCD_DumpVersionToSerial();
  mfrc522_4.PCD_DumpVersionToSerial();
  
  setup_wifi();
  client.setServer(mqttServer, mqttPort);

  servoIn.attach(SERVO_IN_PIN, 500, 2400);
  servoIn.write(initposIn);
  servoOut.attach(SERVO_OUT_PIN, 500, 2400);
  servoOut.write(initposOut);
  pinzaScoo.attach(PINZA_SCOO_PIN, 500, 2400);
  pinzaScoo.write(initposPinzaScoo);
  pinzaBici.attach(PINZA_BICI_PIN, 500, 2400);
  pinzaBici.write(initposPinzaBici);
}

void loop () {
  if (!client.connected()) reconnect();
  client.loop();
  
  distancia = 0.01723 * readUltrasonicDistance(25, 26);
  
  if (mfrc522_1.PICC_IsNewCardPresent()) {
    if (mfrc522_1.PICC_ReadCardSerial()) {
      // RFID Entrada
      Serial.print("RFID Entrada:");
      dumpByteArray(mfrc522_1.uid.uidByte, mfrc522_1.uid.size);
      client.publish("parking/entry", "true");
      servoIn.write(finalposIn);
      delay(2500);
      servoIn.write(initposIn);
      client.publish("parking/entry", "false");
      // String output = dumpByteArrayToString(mfrc522_1.uid.uidByte, mfrc522_1.uid.size);
      // client.publish("parking/uid", output); // hardcoded comparauid to print
      // Serial.println(output);
    }
  }
  
  if (mfrc522_2.PICC_IsNewCardPresent()) {
    if (mfrc522_2.PICC_ReadCardSerial()) {
      // RFID Salida
      Serial.print("RFID Salida:");
      dumpByteArray(mfrc522_2.uid.uidByte, mfrc522_2.uid.size);
      client.publish("parking/exit", "true");
      servoIn.write(finalposOut);
      delay(2500);
      servoIn.write(initposOut);
      client.publish("parking/exit", "false");
    }
  }
  
  if (mfrc522_3.PICC_IsNewCardPresent()) {
    if (mfrc522_3.PICC_ReadCardSerial()) {
      // RFID Parking Bici
      Serial.print("RFID Parking Bici:");
      dumpByteArray(mfrc522_3.uid.uidByte, mfrc522_3.uid.size);
      client.publish("parking/bicicleta", "true");
      park_bici = digitalRead(FIN_CAR_PIN);
      if (!park_bici) {
        client.publish("parking/fdc", "true");
        pinzaBici.write(finalposPinzaBici);
      } else {
        client.publish("parking/fdc", "true");
        pinzaBici.write(initposPinzaBici);
      }
      client.publish("parking/bicicleta", "false");
    }
  }
  
  if (mfrc522_4.PICC_IsNewCardPresent()) {
    if (mfrc522_4.PICC_ReadCardSerial()) {
      // RFID Parking Scooter
      Serial.print("RFID Parking Scooter:");
      dumpByteArray(mfrc522_4.uid.uidByte, mfrc522_4.uid.size);
      client.publish("parking/scooter", "true");
        if (distancia <= 3) {
          client.publish("parking/ultrasonido", "true");
          pinzaScoo.write(finalposPinzaScoo); 
          digitalWrite(13, HIGH);
        } else {
          client.publish("parking/ultrasonido", "false");
          pinzaScoo.write(initposPinzaScoo);
          digitalWrite(13, LOW);
        } 
      client.publish("parking/scooter", "false");
    }
  }
}

void dumpByteArray(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

String dumpByteArrayToString(byte *buffer, byte bufferSize) {
  String output = "";
  for (byte i = 0; i < bufferSize; i++) {
    output += (buffer[i] < 0x10 ? " 0" : " ");
    output += String(buffer[i], HEX);
  }
  return output;
}

long readUltrasonicDistance(int triggerPin, int echoPin) {
  pinMode(triggerPin, OUTPUT);        // Iniciamos el pin del emisor de reuido en salida
  digitalWrite(triggerPin, LOW);      // Apagamos el emisor de sonido
  delayMicroseconds(2);               // Retrasamos la emision de sonido por 2us
  digitalWrite(triggerPin, HIGH);     // Comenzamos a emitir sonido
  delayMicroseconds(10);              // Retrasamos la emision de sonido por 10uS
  digitalWrite(triggerPin, LOW);      // Apagamos el emisor de sonido
  pinMode(echoPin, INPUT);            // Comenzamos a escuchar el sonido
  return pulseIn(echoPin, HIGH);      // Calculamos el tiempo que tardo en regresar el sonido
}

void setup_wifi() {
  Serial.println("Conectando a WiFi...");
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("WiFi conectado");
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conexión MQTT...");
    if (client.connect("ESP32Client")) {
      Serial.println("conectado");
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentar de nuevo en 5 segundos");
      delay(5000);
    }
  }
}
