#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

const int NR_OF_READERS = 4;
const int RST_PIN = 22;
const int SS_1_PIN = 21;
const int SS_2_PIN = 5;
const int SS_3_PIN = 16;
const int SS_4_PIN = 17;
byte ssPins[] = {SS_1_PIN, SS_2_PIN, SS_3_PIN, SS_4_PIN};
MFRC522 mfrc522[NR_OF_READERS];

// sda sck 1 trama = voltage + osciloscopio
// sda sck 2 trama = voltage + osciloscopio
// sda sck 3 trama = voltage + osciloscopio

const int SERVO_IN_PIN = 4;
const int SERVO_OUT_PIN = 2;
const int PINZA_SCOO_PIN = 15;
const int PINZA_BICI_PIN = 14; // !!!
Servo servoIn;
Servo servoOut;
Servo pinza;
Servo pinzaBici;

const int FIN_CAR_PIN = 35;     // !!!
const int LED_ROJO_PIN = 25;
const int LED_VERDE_PIN = 26;
const int LED_AMARILLO_PIN = 27; // !!!

int cnt = 0;
int dist = 0;
int park_bici = 0;
int initposIn = 140;
int finalposIn = 35;
int initposOut = 90;
int finalposOut = 0;
int initposPinzaScooter = 140;
int finalposPinzaScooter = 35;
int initposPinzaBici = 140;
int finalposPinzaBici = 35;
byte LecturaUID[4];

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
  pinza.attach(PINZA_SCOO_PIN, 500, 2400);
  pinza.write(initposPinzaScooter);
  pinzaBici.attach(PINZA_BICI_PIN, 500, 2400);
  pinzaBici.write(initposPinzaBici);
  
  pinMode(33, OUTPUT); // LED BICI
  pinMode(32, OUTPUT); // LED SCOOTER
  
  pinMode(LED_ROJO_PIN, OUTPUT); // LED ROJO
  pinMode(LED_VERDE_PIN, OUTPUT); // LED VERDE
  pinMode(LED_AMARILLO_PIN, OUTPUT); // LED AMARILLO
  
  Serial.println("Inicio del registro");
}

void loop() {
  //Calculamos la distancia en cm
  dist = 0.01723 * readUltrasonicDistance(13, 12);
  
  for (uint8_t reader = 0; reader < NR_OF_READERS; reader++) {
    if (mfrc522[reader].PICC_IsNewCardPresent() && mfrc522[reader].PICC_ReadCardSerial()) {
      Serial.print(F("Reader "));   
      Serial.print(reader);
      Serial.print(F(": Card UID:"));
      dumpByteArray(mfrc522[reader].uid.uidByte, mfrc522[reader].uid.size);
      Serial.println();

      for (byte i = 0; i < mfrc522[reader].uid.size; i++) {
        LecturaUID[i] = mfrc522[reader].uid.uidByte[i];
      }

      if (reader == 0) {
        Serial.println("LECTOR DE SALIDA DETECTADO");
        servoOut.write(finalposOut);
        digitalWrite(25, LOW); delay(50);       // LED ROJO
        digitalWrite(27, LOW); delay(50);       // LED AMARILLO
        digitalWrite(26, LOW); delay(50);       // LED VERDE
        delay(2500);
        servoOut.write(initposOut);       
      }
      
      if (reader == 1) {
        Serial.println("LECTOR DE ENTRADA DETECTADO");
        servoIn.write(finalposIn);
        delay(2500);
        servoIn.write(initposIn);

        delay(3000);
        digitalWrite(25, HIGH); delay(50);       // LED ROJO
        delay(1000); 
        digitalWrite(37, HIGH); delay(50);       // LED AMARILLO
        digitalWrite(26, HIGH); delay(50);       // LED VERDE
      }

      if (reader == 2) {
        Serial.println("LECTOR DE SCOOTER DETECTADO");
        ///////////////////////////////////////////
        int park_bici = digitalRead(FIN_CAR_PIN);
        if (!park_bici) {
          Serial.println("Bicicleta parqueada");
          pinzaBici.write(finalposPinzaBici);
          digitalWrite(33, HIGH);
        } else {
          pinzaBici.write(initposPinzaBici);
          digitalWrite(33, LOW);
        }
        ///////////////////////////////////////////
      }

      if (reader == 3) {
        Serial.println("LECTOR DE BICI DETECTADO");
        ///////////////////////////////////////////
        if (dist <= 3) {
          Serial.println("Scooter parqueada");
          pinza.write(finalposPinzaScooter); 
          digitalWrite(32, HIGH);
        } else {
          pinza.write(initposPinzaScooter);
          digitalWrite(32, LOW);
        } 
        ///////////////////////////////////////////
      }
      
      mfrc522[reader].PICC_HaltA();
      mfrc522[reader].PCD_StopCrypto1();
    } //if (mfrc522[reader].PICC_IsNewC
  } //for(uint8_t reader
}

void dumpByteArray(byte *buffer, byte bufferSize) {
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
