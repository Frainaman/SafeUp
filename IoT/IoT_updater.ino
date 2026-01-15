#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <MFRC522.h>

const char* ssid = "XXXX";
const char* password = "XXXX";

String serverUrl = "http://XXX.XXX.XXX.XXX:5000/check_firmware";

const String MASTER_UID = "1B D2 AB 02";

#define SS_PIN  5
#define RST_PIN 22
#define LED_PIN 15


const char* currentVersion = "0.9";

MFRC522 rfid(SS_PIN, RST_PIN);

void eseguiOTA(const char* downloadUrl) {
  Serial.println("---------------------------------------------");
  Serial.println(">>> AVVIO DOWNLOAD FIRMWARE (OTA) <<<");

  WiFiClientSecure client;
  client.setInsecure();
  httpUpdate.rebootOnUpdate(false);

  Serial.print("Scaricando da: "); Serial.println(downloadUrl);

  t_httpUpdate_return ret = httpUpdate.update(client, downloadUrl);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("ERRORE AGGIORNAMENTO (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;
    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("Nessun aggiornamento necessario.");
      break;
    case HTTP_UPDATE_OK:
      Serial.println("AGGIORNAMENTO COMPLETATO CON SUCCESSO!");
      Serial.println("Riavvio il sistema tra 3 secondi...");
      delay(3000);
      ESP.restart();
      break;
  }
}

void gestisciRispostaServer(String payload) {
  StaticJsonDocument<1024> doc;
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("Errore lettura JSON: ");
    Serial.println(error.c_str());
    return;
  }

  const char* blockchainVersion = doc["version"];
  const char* downloadUrl = doc["url"];

  Serial.println("\n--- CONFRONTO VERSIONI ---");
  Serial.print("Versione Attuale (ESP32): "); Serial.println(currentVersion);
  Serial.print("Versione Blockchain:      "); Serial.println(blockchainVersion);


  if (String(blockchainVersion) != "" && String(blockchainVersion) != String(currentVersion)) {
    Serial.println("⚡ NUOVA VERSIONE TROVATA! Procedo...");

    for(int i=0; i<5; i++) { digitalWrite(LED_PIN, HIGH); delay(100); digitalWrite(LED_PIN, LOW); delay(100); }

    eseguiOTA(downloadUrl);
  } else {
    Serial.println("Il sistema è già aggiornato all'ultima versione sicura.");
    digitalWrite(LED_PIN, HIGH); delay(1000); digitalWrite(LED_PIN, LOW);
  }
}


void checkMiddleware() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;

    Serial.print("Contattando Middleware: "); Serial.println(serverUrl);

    if (http.begin(client, serverUrl)) {
      int httpCode = http.GET();

      if (httpCode > 0) {
        String payload = http.getString();
        Serial.println("Dati ricevuti dalla Blockchain!");
        gestisciRispostaServer(payload);
      } else {
        Serial.print("Errore HTTP: "); Serial.println(httpCode);
        Serial.println("Il server Node.js è acceso?");
      }
      http.end();
    } else {
      Serial.println("Impossibile connettersi all'indirizzo IP specificato.");
    }
  } else {
    Serial.println("WiFi perso!");
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  pinMode(LED_PIN, OUTPUT);

  Serial.println("\n\n--- AVVIO SISTEMA SECURE OTA ---");
  Serial.print("Connessione WiFi a: "); Serial.print(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));
  }
  digitalWrite(LED_PIN, LOW);

  Serial.println("\n✅ WiFi Connesso!");
  Serial.print("IP ESP32: "); Serial.println(WiFi.localIP());
  Serial.println("----------------------------------------");
  Serial.println("🔒 SISTEMA BLOCCATO. Passare carta MASTER per sbloccare.");
}

void loop() {
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  String uidletta = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidletta.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    uidletta.concat(String(rfid.uid.uidByte[i], HEX));
  }
  uidletta.toUpperCase();
  String codicePulito = uidletta.substring(1);

  if (codicePulito == MASTER_UID) {
    Serial.println("\n✅ AUTORIZZAZIONE RICEVUTA (Master Key)");
    digitalWrite(LED_PIN, HIGH);

    checkMiddleware();

    delay(2000);
    digitalWrite(LED_PIN, LOW);
  } else {
    Serial.print("ACCESSO NEGATO: "); Serial.println(codicePulito);
    for(int i=0; i<3; i++){ digitalWrite(LED_PIN, HIGH); delay(100); digitalWrite(LED_PIN, LOW); delay(100); }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
}