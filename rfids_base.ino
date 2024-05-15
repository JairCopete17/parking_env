#include <SPI.h>
#include <MFRC522.h>
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
}

void loop () {
  if (!client.connected()) reconnect();
  client.loop();

  if (mfrc522_1.PICC_IsNewCardPresent()) {
    if (mfrc522_1.PICC_ReadCardSerial()) {
      // RFID Entrada
      Serial.print("RFID Entrada:");
      client.publish("parking/entry", "true");
      dumpByteArray(mfrc522_1.uid.uidByte, mfrc522_1.uid.size);
      delay(500);
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
      client.publish("parking/exit", "true");
      dumpByteArray(mfrc522_2.uid.uidByte, mfrc522_2.uid.size);
      delay(500);
      client.publish("parking/exit", "false");
    }
  }
  
  if (mfrc522_3.PICC_IsNewCardPresent()) {
    if (mfrc522_3.PICC_ReadCardSerial()) {
      // RFID Parking Bici
      Serial.print("RFID Parking Bici:");
      client.publish("parking/bicicleta", "true");
      dumpByteArray(mfrc522_3.uid.uidByte, mfrc522_3.uid.size);
      delay(500);
      client.publish("parking/bicicleta", "false");
    }
  }
  
  if (mfrc522_4.PICC_IsNewCardPresent()) {
    if (mfrc522_4.PICC_ReadCardSerial()) {
      // RFID Parking Scooter
      Serial.print("RFID Parking Scooter:");
      client.publish("parking/scooter", "true");
      dumpByteArray(mfrc522_4.uid.uidByte, mfrc522_4.uid.size);
      delay(500);
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
