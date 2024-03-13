#include <SPI.h>
#include <MFRC522.h>
#include <ESP32Servo.h>

#define RST_PIN   22
#define SS_PIN    21
#define SERVO_PIN 2

MFRC522 mfrc522(SS_PIN, RST_PIN);
Servo myservo;

int cnt = 0;
int initpos = 140;
byte LecturaUID[4];
byte Usuario1[4]= {0x0C, 0x51, 0x8B, 0x17} ;    // UID de llavero
byte Usuario2[4]= {0xB4, 0x56, 0xC9, 0xE7} ;    // UID de carnet de la universidad Santiago

void setup() {
  Serial.begin(9600);
  SPI.begin();
  mfrc522.PCD_Init();
  myservo.attach(SERVO_PIN, 500, 2400);
  myservo.write(initpos);
  Serial.println("Inicio del registro");
}

void loop() {
  // Si no hay una tarjeta presente, retorna al loop esperando por una tarjeta
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    //Serial.println("Esperando tarjeta");
    return;
  }
  // Si no puede obtener datos de la tarjeta, retorna al loop esperando por otra tarjeta
  if ( ! mfrc522.PICC_ReadCardSerial()) return;

  Serial.print("UID:");       // show UID
  for (byte i = 0; i<mfrc522.uid.size; i++){
    if (mfrc522.uid.uidByte[i] < 0x10)  Serial.print(" 0");
      else  Serial.print(" ");
    Serial.print(mfrc522.uid.uidByte[i], HEX);
    LecturaUID[i] = mfrc522.uid.uidByte[i];       
  }
  Serial.print("\t");
  
  if(comparaUID(LecturaUID, Usuario1)) {
    Serial.println("Bienvenido Jair");
    myservo.write(35);
    delay(5000);
    myservo.write(initpos);
  }
  else if(comparaUID(LecturaUID, Usuario2)) Serial.println("Bienvenido Santiago");
  else Serial.println("Usuario no registrado");
  
  mfrc522.PICC_HaltA();
}

boolean comparaUID(byte lectura[],byte usuario[]){
  for (byte i=0; i < mfrc522.uid.size; i++){    
    // Si el UID leido es distinto a algun usuario registrado, retorna falso
    if(lectura[i] != usuario[i])  return(false);
  }
  return(true); // Si coincide, retorna verdadero
}
