/////////////////////////
/////// Entradas ////////
#define NR_OF_READERS 2
#define RST_PIN       22
#define SS_1_PIN      21
#define SS_2_PIN      5
#define SERVO_IN_PIN  4 
#define SERVO_OUT_PIN 2
/////////////////////////
/////// Parking 1 ///////
#define GREEN_1_PIN   14
#define RED_1_PIN     12
#define YELLOW_1_PIN  13
/////////////////////////
/////// Parking 2 ///////
#define GREEN_2_PIN   25
#define RED_2_PIN     26
#define YELLOW_2_PIN  27
/////////////////////////
///// Parking Bici //////
#define RED_BICI_PIN  33
//#define FIN_CAR_PIN A
//#define SS_3_PIN    A
//#define ELECTOIMAN  A
/////////////////////////
//// Parking Scooter ////
#define RED_SCTR_PIN  32
#define TRIG_PIN      1
#define ECHO_PIN      3
//#define PINZA_PIN     15
//#define RFID        A

#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

byte ssPins[] = {SS_1_PIN, SS_2_PIN};
MFRC522 mfrc522[NR_OF_READERS];
Servo servoIn;
Servo servoOut;

int cnt = 0;
int initposIn = 140;
int finalposIn = 35;
int initposOut = 90;
int finalposOut = 0;
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
  
//  pinMode(GREEN_1_PIN, OUTPUT);
//  pinMode(RED_1_PIN, OUTPUT);
//  pinMode(YELLOW_1_PIN, OUTPUT);
//  pinMode(GREEN_2_PIN, OUTPUT);
//  pinMode(RED_2_PIN, OUTPUT);
//  pinMode(YELLOW_2_PIN, OUTPUT);
//  pinMode(RED_BICI_PIN, OUTPUT);
//  pinMode(RED_SCTR_PIN, OUTPUT);
//  pinMode(TRIG_PIN, OUTPUT);
//  pinMode(ECHO_PIN, INPUT);
//  digitalWrite(TRIG_PIN, LOW);
  Serial.println("Inicio del registro");
}

void loop() {
  //ultrasonido();
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

void ultrasonido(){
  long t;
  long d;

  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  t = pulseIn(ECHO_PIN, HIGH); // Obtenemos el ancho del pulso
  d = t/59;                    // Escalamos el tiempo a una distancia en cm

  Serial.print("Distancia: ");
  Serial.print(d);             // Enviamos serialmente el valor de la distancia
  Serial.print("cm");
  Serial.println();
  delay(10);
}
