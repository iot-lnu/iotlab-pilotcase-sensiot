#include <Wire.h>
#include "MFRC522_I2C.h"
#include <MKRNB.h>
#include <FlashStorage.h>
#include <time.h>

#define RST_PIN 6 // Arduino UNO Pin
#define TXD_PIN 4 
#define RXD_PIN 5
#define PINNUMBER ""
#define HOST "139.162.164.160"
#define ENDPOINT "/listen"

#define I2C_ADDR 0x28

#define SERVER_PORT 80
#define METHOD "POST"
#define DEBUG 1
#define TOKEN "YWRtaW46c2VjcmV0X3N0dWZm"

// Check the CPU type, flash storage is currently only supported on SAMD21 and SAMD51
#if defined(__SAMD21__) 
  #define CPU_OTSAMD21
#elif defined(__SAMD51__) 
  #define CPU_OTSAMD51
#endif

NBClient client;
GPRS gprs;
NB nbAccess;
MFRC522_I2C mfrc522(I2C_ADDR, RST_PIN);

volatile uint32_t dishCount = 30;
byte UID[10];
char uid_str[21];

FlashStorage(f_dishCount, uint32_t);

static void send_data();

void setup() {
  Serial.begin(115200);
  Serial.println("Starting...");

  // Read the configuration from flash
  //dishCount = f_dishCount.read();

  #ifdef DEBUG
  Serial.print("Initial Dish count from flash: ");
  Serial.println(dishCount);
  #endif

  boolean connected = false;

  // Modem and network initialization
  while (!connected) {
    if ((nbAccess.begin(PINNUMBER) == NB_READY) &&
        (gprs.attachGPRS() == GPRS_READY)) {
      connected = true;
    } else {
      #ifdef DEBUG
      Serial.println("Not connected");
      #endif
      delay(1000);
    }
  }

  Wire.begin();       // Initialize I2C
  mfrc522.PCD_Init(); // Init MFRC522

  #ifdef DEBUG  
  Serial.println("All initialized");
  Serial.print("Dish count: ");
  Serial.println(dishCount);
  #endif
}

void loop() {
  // Look for new cards, and select one if present
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial()) {
    delay(100);
    return;
  }

  // If the new card has the same UID as the last one, do nothing
  if (memcmp(UID, mfrc522.uid.uidByte, mfrc522.uid.size) == 0) {
      #ifdef DEBUG
      Serial.println("UIDs are the same. Doing nothing");
      #endif
  } else {
    //dishCount++;

    // Only save every 10th dish count to save write cycles
    if(dishCount % 10 == 0) {
      f_dishCount.write(dishCount);
    }

    #ifdef DEBUG
    Serial.println("UIDs are not the same. Counting the dish");
    Serial.print("Dish count: ");
    Serial.println(dishCount);
    #endif

    // Save the UID 
    memcpy(UID, mfrc522.uid.uidByte, mfrc522.uid.size);

    for (int i = 0; i < mfrc522.uid.size; i++) {
        // Format each byte as two uppercase hexadecimal digits
        sprintf(uid_str + i * 2, "%02X", UID[i]);
    }

    #ifdef DEBUG
    Serial.print("UID: ");
    for (size_t i = 0; i < mfrc522.uid.size; ++i) {
        Serial.print(UID[i], HEX);
    }

    Serial.println();
    #endif

    send_data();
    
    delay(200);
    Serial.println("Waiting for the next tray...");
  }
}


static bool connect_to_server() {
  if (client.connect(HOST, SERVER_PORT)) {
    Serial.println("Connected to the server");
    return true;
  } else {
    Serial.println("Connection failed");
    return false;
  }
}

static void send_data() {

    connect_to_server();

    Serial.println("Sending data to the server");

    char requestBody[256];
    snprintf(requestBody, sizeof(requestBody), "{\"plates\": %lu, \"uid\": \"%s\"}", dishCount, uid_str);

    // Prepare the host header
    char hostHeader[128];
    snprintf(hostHeader, sizeof(hostHeader), "Host: %s", HOST);

    // Prepare the Authorization header
    char authHeader[256];
    snprintf(authHeader, sizeof(authHeader), "Authorization: Basic %s", TOKEN);

    // Construct HTTP POST request
    client.println("POST " ENDPOINT " HTTP/1.1");
    client.println(hostHeader);
    client.println("Content-Type: application/json");
    client.println(authHeader);    
    client.print("Content-Length: ");
    client.println(strlen(requestBody));
    client.println(); 
    client.println(requestBody);  
    

    Serial.println("Connection closed");
    Serial.println(requestBody);
    Serial.println("Data sent to the server");
}

void ShowReaderDetails() {
  // Get the MFRC522 software version
  byte v = mfrc522.PCD_ReadRegister(mfrc522.VersionReg);
  Serial.print(F("MFRC522 Software Version: 0x"));
  Serial.print(v, HEX);
  if (v == 0x91)
    Serial.print(F(" = v1.0"));
  else if (v == 0x92)
    Serial.print(F(" = v2.0"));
  else
    Serial.print(F(" (unknown)"));
  Serial.println("");
  // Output a warning if communication fails
  if ((v == 0x00) || (v == 0xFF)) {
    Serial.println(F("WARNING: Communication failure, is the MFRC522 properly connected?"));
  }
}
