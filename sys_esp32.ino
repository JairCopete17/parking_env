/////////////////////////
/////// Entradas ////////
#define NR_OF_READERS 4
#define RST_PIN       22
#define SS_1_PIN      21
#define SS_2_PIN      5
#define SERVO_IN_PIN  4 
#define SERVO_OUT_PIN 2
/////////////////////////
//// Parking Scooter ////
#define SS_3_PIN      16
/////////////////////////
///// Parking Bici //////
#define SS_4_PIN      17
#define FIN_CAR_PIN   14
//#define ELECTOIMAN  A
/////////////////////////

#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN, SS_4_PIN};

MFRC522 mfrc522[NR_OF_READERS];
Servo servoIn;
Servo servoOut;
Servo pinza;

int cnt = 0;
int dist = 0;
int park_bici = 0;
int initposIn = 140;
int finalposIn = 35;
int initposOut = 90;
int finalposOut = 0;
int initposPinza = 140;
int finalposPinza = 35;
byte LecturaUID[4];
byte Usuario1[4]= {0x0C, 0x51, 0x8B, 0x17} ;    // UID de llavero
byte Usuario2[4]= {0xB4, 0x56, 0xC9, 0xE7} ;    // UID de carnet de la universidad Santiago
byte Usuario3[4]= {0xB2, 0xA9, 0xE6, 0xE9} ;    // UID de carnet de la universidad Jair

void setup() {
  Serial.begin(9600);
  SPI.begin();
  
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    mfrc522[reader].PCD_Init(ssPins[reader], RST_PIN);
    Serial.print(F("Reader "));
    Serial.print(reader);
    Serial.print(F(": "));
    mfrc522[reader].PCD_DumpVersionToSerial();
  }
  
  servoIn.attach(SERVO_IN_PIN, 500, 2400);
  servoIn.write(initposIn);
  servoOut.attach(SERVO_OUT_PIN, 500, 2400);
  servoOut.write(initposOut);
  pinza.attach(15, 500, 2400);
  pinza.write(initposPinza);

  pinMode(33, OUTPUT); // LED BICI
  pinMode(32, OUTPUT); // LED SCOOTER
  Serial.println("Inicio del registro");
}

void loop() {
  //Calculamos la distancia en cm
  dist = 0.01723 * readUltrasonicDistance(13, 12);
  if (dist <= 3) {                            //Mostramos la distancia
    Serial.println("Scooter parqueada");
    pinza.write(finalposPinza); 
    digitalWrite(32, HIGH);
  } else {
    pinza.write(initposPinza);
    digitalWrite(32, LOW);
  }
  
  int park_bici = digitalRead(FIN_CAR_PIN);
  if (!park_bici) {
    Serial.println("Bicicleta parqueada");
    digitalWrite(33, HIGH);
  } else digitalWrite(33, LOW);
  
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));
      Serial.print(reader);
      Serial.print(F(": Card UID:"));
      dump_byte_array(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();

      for (byte i = 0; i < mfrc522[reader].uid.size; i++) {
        LecturaUID[i] = mfrc522[reader].uid.uidByte[i];
      }

      if (reader == 0) {
        Serial.println("LECTOR DE SALIDA DETECTADO");
        servoOut.write(finalposOut);
        delay(2500);
        servoOut.write(initposOut);
      }
      
      if (reader == 1) {
        Serial.println("LECTOR DE ENTRADA DETECTADO");
        servoIn.write(finalposIn);
        delay(2500);
        servoIn.write(initposIn);
      }

      if (reader == 2) {
        Serial.println("LECTOR DE SCOOTER DETECTADO");
      }

      if (reader == 3) {
        Serial.println("LECTOR DE BICI DETECTADO");
      }
      
      mfrc522[reader].PICC_HaltA();
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader
}

void dump_byte_array(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
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
