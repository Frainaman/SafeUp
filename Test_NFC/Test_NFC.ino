#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <HTTPUpdate.h>  // <--- NUOVO: Serve per l'aggiornamento
#include <ArduinoJson.h> // <--- NUOVO: Serve per leggere la risposta
#include <SPI.h>
#include <MFRC522.h>

// --- I TUOI DATI ---
const char* ssid = "SKYWIFI_AB6LJ";      
const char* password = "NzXuAMwLV2cMIr"; 

// Link server (Ngrok)
const char* serverUrl = "https://fractus-jeremiah-nonrupturable.ngrok-free.dev/check_firmware";

// Versione attuale caricata su QUESTO Arduino
const char* currentVersion = "1.0"; 
// ------------------------------

// CONFIGURAZIONE HARDWARE
#define SS_PIN  5
#define RST_PIN 22
#define LED_PIN 15 
const String MASTER_UID = "1B D2 AB 02"; 

MFRC522 rfid(SS_PIN, RST_PIN);

// ============================================================
// 1. FUNZIONE PER SCARICARE E INSTALLARE IL FIRMWARE
// ============================================================
void eseguiOTA(const char* downloadUrl) {
  Serial.println("---------------------------------------------");
  Serial.println(">>> AVVIO PROCEDURA DI AGGIORNAMENTO (OTA) <<<");
  
  // Per scaricare il file .bin serve un client. 
  // Usiamo Secure per sicurezza, impostandolo come 'insecure' per evitare blocchi certificati
  WiFiClientSecure client;
  client.setInsecure();

  // Disabilita il riavvio automatico per poter stampare il messaggio di fine
  httpUpdate.rebootOnUpdate(false);

  Serial.print("Download da: "); Serial.println(downloadUrl);

  // Esegue l'update
  t_httpUpdate_return ret = httpUpdate.update(client, downloadUrl);

  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf("❌ ERRORE AGGIORNAMENTO (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      Serial.println("⚠️ Nessun aggiornamento necessario (Server ha risposto 304).");
      break;

    case HTTP_UPDATE_OK:
      Serial.println("✅ AGGIORNAMENTO COMPLETATO!");
      Serial.println("Riavvio il sistema tra 3 secondi...");
      delay(3000);
      ESP.restart(); // Riavvia l'ESP32
      break;
  }
}

// ============================================================
// 2. FUNZIONE CHE LEGGE IL JSON E DECIDE SE AGGIORNARE
// ============================================================
void gestisciRispostaServer(String payload) {
  StaticJsonDocument<512> doc;
  
  // Decodifica il JSON
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print("❌ Errore lettura JSON: ");
    Serial.println(error.c_str());
    return;
  }

  // Estrae i dati dal JSON
  const char* serverVersion = doc["version"];
  const char* downloadUrl = doc["url"];
  
  Serial.print("Versione Locale: "); Serial.println(currentVersion);
  Serial.print("Versione Server: "); Serial.println(serverVersion);

  // Confronta le stringhe
  if (String(serverVersion) != String(currentVersion)) {
    Serial.println("⚡ Nuova versione trovata! Inizio aggiornamento...");
    eseguiOTA(downloadUrl); // <--- CHIAMA LA FUNZIONE OTA
  } else {
    Serial.println("👍 Il sistema è già aggiornato.");
  }
}

// ============================================================
// 3. FUNZIONE DI CONNESSIONE AL SERVER (Modificata)
// ============================================================
void checkMiddleware() {
  if (WiFi.status() == WL_CONNECTED) {
    
    WiFiClientSecure client;
    client.setInsecure(); 
    
    HTTPClient http;
    
    Serial.print("Chiamata a: "); Serial.println(serverUrl);
    
    if (http.begin(client, serverUrl)) { 
      
      int httpCode = http.GET(); 

      if (httpCode > 0) {
        String payload = http.getString();
        Serial.println("\n--- RISPOSTA DAL SERVER ---");
        Serial.println(payload);
        Serial.println("---------------------------");
        
        // *** QUI LA MODIFICA: Passiamo i dati alla funzione di gestione ***
        gestisciRispostaServer(payload);
        // ******************************************************************

      } else {
        Serial.print("❌ Errore HTTP: "); Serial.println(httpCode);
        Serial.println(http.errorToString(httpCode));
      }
      http.end();
      
    } else {
      Serial.println("❌ Impossibile connettersi al server");
    }
  } else {
    Serial.println("⚠️ WiFi perso!");
  }
}

// ============================================================
// SETUP STANDARD
// ============================================================
void setup() {
  Serial.begin(115200);
  
  SPI.begin();
  rfid.PCD_Init();
  rfid.PCD_SetAntennaGain(rfid.RxGain_max);
  pinMode(LED_PIN, OUTPUT);

  // Lampeggio iniziale
  for(int i=0; i<3; i++){ digitalWrite(LED_PIN, HIGH); delay(100); digitalWrite(LED_PIN, LOW); delay(100); }

  Serial.print("\nConnessione a "); Serial.print(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\n✅ WiFi Connesso!");
  Serial.print("IP ESP32: "); Serial.println(WiFi.localIP());
  
  Serial.print("Versione Firmware Attuale: "); Serial.println(currentVersion);
  Serial.println("--- PRONTO: PASSA LA CARTA ---");
}

// ============================================================
// LOOP STANDARD
// ============================================================
void loop() {
  // Cerca nuova carta
  if (!rfid.PICC_IsNewCardPresent()) return;
  if (!rfid.PICC_ReadCardSerial()) return;

  // Leggi UID
  String uidletta = "";
  for (byte i = 0; i < rfid.uid.size; i++) {
    uidletta.concat(String(rfid.uid.uidByte[i] < 0x10 ? " 0" : " "));
    uidletta.concat(String(rfid.uid.uidByte[i], HEX));
  }
  uidletta.toUpperCase();
  String codicePulito = uidletta.substring(1);

  Serial.print("UID Letto: "); Serial.println(codicePulito);

  if (codicePulito == MASTER_UID) {
    Serial.println("✅ Accesso OK -> Controllo aggiornamenti...");
    digitalWrite(LED_PIN, HIGH); 
    
    checkMiddleware(); // <--- Parte tutto da qui
    
    delay(1000); 
    digitalWrite(LED_PIN, LOW);
  } 
  else {
    Serial.println("⛔ Accesso Negato");
    for(int i=0; i<2; i++){ digitalWrite(LED_PIN, HIGH); delay(200); digitalWrite(LED_PIN, LOW); delay(200); }
  }

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();
  delay(1000); 
}