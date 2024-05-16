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
const char* mqtt_topic = "topic/servo";

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
byte LecturaUID[4];
byte Usuario1[4]= {0x22, 0x57, 0x12, 0xE9} ;    // UID de carnet de la universidad Christian
byte Usuario2[4]= {0x29, 0x9A, 0xCF, 0xB2} ;    // UID de tarjeta blanca
//byte Usuario3[4]= {0xB2, 0xA9, 0xE6, 0xE8} ;    // UID de carnet de la universidad Jair
//byte Usuario4[4]= {0xB4, 0x56, 0xC9, 0xE7} ;    // UID de carnet de la universidad Santiago

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
  client.setCallback(callback);

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
      for (byte i = 0; i<mfrc522_1.uid.size; i++){
        if (mfrc522_1.uid.uidByte[i] < 0x10)  Serial.print(" 0");
          else  Serial.print(" ");
        Serial.print(mfrc522_1.uid.uidByte[i], HEX);
        LecturaUID[i] = mfrc522_1.uid.uidByte[i];       
      }
      Serial.println();

      if(comparaUID1(LecturaUID, Usuario1)) {
        client.publish("parking/entry", "225712E9");
        servoIn.write(finalposIn);
        delay(2500);
        servoIn.write(initposIn);
        client.publish("parking/entry", "false");
      }
      else if(comparaUID1(LecturaUID, Usuario2)) {
        client.publish("parking/entry", "299ACFB2");
        servoIn.write(finalposIn);
        delay(2500);
        servoIn.write(initposIn);
        client.publish("parking/entry", "false");
      }
      else client.publish("parking/entry", "false");
    }
  }
  
  if (mfrc522_2.PICC_IsNewCardPresent()) {
    if (mfrc522_2.PICC_ReadCardSerial()) {
      // RFID Salida
      Serial.print("RFID Salida:");
      for (byte i = 0; i<mfrc522_2.uid.size; i++){
        if (mfrc522_2.uid.uidByte[i] < 0x10)  Serial.print(" 0");
          else  Serial.print(" ");
        Serial.print(mfrc522_2.uid.uidByte[i], HEX);
        LecturaUID[i] = mfrc522_2.uid.uidByte[i];       
      }
      Serial.println();

      if(comparaUID2(LecturaUID, Usuario1)) {
        client.publish("parking/exit", "225712E9");
        servoIn.write(finalposOut);
        delay(2500);
        servoIn.write(initposOut);
        client.publish("parking/exit", "false");
      }
      else if(comparaUID2(LecturaUID, Usuario2)) {
        client.publish("parking/exit", "299ACFB2");
        servoIn.write(finalposOut);
        delay(2500);
        servoIn.write(initposOut);
        client.publish("parking/exit", "false");
      }
      else client.publish("parking/exit", "false");
    }
  }
  
  if (mfrc522_3.PICC_IsNewCardPresent()) {
    if (mfrc522_3.PICC_ReadCardSerial()) {
      // RFID Parking Bici
      Serial.print("RFID Parking Bici:");
      for (byte i = 0; i<mfrc522_3.uid.size; i++){
        if (mfrc522_3.uid.uidByte[i] < 0x10)  Serial.print(" 0");
          else  Serial.print(" ");
        Serial.print(mfrc522_3.uid.uidByte[i], HEX);
        LecturaUID[i] = mfrc522_3.uid.uidByte[i];       
      }
      Serial.println();

      if(comparaUID3(LecturaUID, Usuario1)) {
        client.publish("parking/bicicleta", "225712E9");
        park_bici = digitalRead(FIN_CAR_PIN);
        if (!park_bici) {
          client.publish("parking/fdc", "true");
          pinzaBici.write(finalposPinzaBici);
        } else {
          client.publish("parking/fdc", "false");
          pinzaBici.write(initposPinzaBici);
        }
        client.publish("parking/bicicleta", "false");
      }
      else if(comparaUID3(LecturaUID, Usuario2)) {
        client.publish("parking/bicicleta", "299ACFB2");
        park_bici = digitalRead(FIN_CAR_PIN);
        if (!park_bici) {
          client.publish("parking/fdc", "true");
          pinzaBici.write(finalposPinzaBici);
        } else {
          client.publish("parking/fdc", "false");
          pinzaBici.write(initposPinzaBici);
        }
        client.publish("parking/bicicleta", "false");
      }
      else client.publish("parking/bicicleta", "false");
    }
  }
  
  if (mfrc522_4.PICC_IsNewCardPresent()) {
    if (mfrc522_4.PICC_ReadCardSerial()) {
      // RFID Parking Scooter
      Serial.print("RFID Parking Scooter:");
      for (byte i = 0; i<mfrc522_4.uid.size; i++){
        if (mfrc522_4.uid.uidByte[i] < 0x10)  Serial.print(" 0");
          else  Serial.print(" ");
        Serial.print(mfrc522_4.uid.uidByte[i], HEX);
        LecturaUID[i] = mfrc522_4.uid.uidByte[i];       
      }
      Serial.println();

      if(comparaUID4(LecturaUID, Usuario1)) {
        client.publish("parking/scooter", "225712E9");
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
      else if(comparaUID4(LecturaUID, Usuario2)) {
        client.publish("parking/scooter", "299ACFB2");
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
      else client.publish("parking/scooter", "false");
    }
  }
}

void dumpByteArray(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Mensaje recibido [");
  Serial.print(topic);
  Serial.print("] ");
  String message;
  for (int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println(message);

  // Si el mensaje recibido es "ON", activa el servo
  if (message.equals("ON")) {
    pinzaBici.write(finalposPinzaBici);
    delay(100);
    pinzaBici.write(initposPinzaBici);
    delay(100);
    pinzaBici.write(finalposPinzaBici);
    delay(100);
    pinzaBici.write(initposPinzaBici);
  }
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

boolean comparaUID1(byte lectura[],byte usuario[]){
  for (byte i=0; i < mfrc522_1.uid.size; i++){    
    // Si el UID leido es distinto a algun usuario registrado, retorna falso
    if(lectura[i] != usuario[i])  return(false);
  }
  return(true); // Si coincide, retorna verdadero
}

boolean comparaUID2(byte lectura[],byte usuario[]){
  for (byte i=0; i < mfrc522_2.uid.size; i++){    
    // Si el UID leido es distinto a algun usuario registrado, retorna falso
    if(lectura[i] != usuario[i])  return(false);
  }
  return(true); // Si coincide, retorna verdadero
}

boolean comparaUID3(byte lectura[],byte usuario[]){
  for (byte i=0; i < mfrc522_3.uid.size; i++){    
    // Si el UID leido es distinto a algun usuario registrado, retorna falso
    if(lectura[i] != usuario[i])  return(false);
  }
  return(true); // Si coincide, retorna verdadero
}

boolean comparaUID4(byte lectura[],byte usuario[]){
  for (byte i=0; i < mfrc522_4.uid.size; i++){    
    // Si el UID leido es distinto a algun usuario registrado, retorna falso
    if(lectura[i] != usuario[i])  return(false);
  }
  return(true); // Si coincide, retorna verdadero
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
      client.subscribe(mqtt_topic);
    } else {
      Serial.print("falló, rc=");
      Serial.print(client.state());
      Serial.println(" intentar de nuevo en 5 segundos");
      delay(5000);
    }
  }
}
