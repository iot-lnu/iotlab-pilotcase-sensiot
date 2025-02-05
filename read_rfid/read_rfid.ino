#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

#define PN532_SCK  (2)
#define PN532_MOSI (3)
#define PN532_SS   (4)
#define PN532_MISO (5)

#define PN532_IRQ   (2)
#define PN532_RESET (3)  // Not connected by default on the NFC Shield
#define GLASS_AMOUNT 30

uint8_t glass_count = 0;
uint8_t keya[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t glassCount;
uint8_t prevId;
uint8_t success;
uint8_t uid[] = { 0, 0, 0, 0, 0, 0, 0 };  // Buffer to store the returned UID
uint8_t uidLength;                        // Length of the UID (4 or 7 bytes depending on ISO14443A card type)

Adafruit_PN532 nfc(PN532_IRQ, PN532_RESET);

void setup(void) {

  Serial.begin(115200);
  
  // Sending data out on the serial
  Serial2.begin(115200);

  nfc.begin();

  uint32_t versiondata = nfc.getFirmwareVersion();
  if (! versiondata) {
    Serial.print("Didn't find PN53x board");
    while (1); // halt
  }

  Serial.println("Waiting for a tag ...");
}

void write_to_tag(uint8_t type, uint8_t uidLength, void *uid) {

  uint8_t success;

  if (uidLength == 4) {  // UID length indicates Mifare Classic
    // Authenticate block 4 (Block 4 of sector 1) with key A
    Serial.println(nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya));
    
    if (nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya)) {
      // Prepare data to write
      uint8_t data[16] = { 0x1 };
      // Write the data to block 4
      success = nfc.mifareclassic_WriteDataBlock(4, data);
      if (success) {
        Serial.println("Data written to tag successfully!");
      } else {
        Serial.println("Failed to write to the card.");
      }
    } else {
      Serial.println("Authentication failed.");
    }
  }
}

uint8_t read_from_tag(uint8_t type, uint8_t uidLength, void *uid) {
  
  uint8_t success;
  if (uidLength == 4) { 
    // Authenticate block 4 with key A
    
    if (nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya)) {
      
      // Prepare buffer to store read data
      uint8_t data[16];
      
      // Read the data from block 4
      success = nfc.mifareclassic_ReadDataBlock(4, data);
      if (success) {
        Serial.println("Read data from tag:");
        nfc.PrintHex(data, 16);
        // Optionally, print data as ASCII
        Serial.print("ASCII: ");
        for (uint8_t i = 0; i < 16; i++) {
          if (data[i] >= 32 && data[i] <= 126) {
            Serial.print((char)data[i]);  // Print readable characters as ASCII
          } else {
            Serial.print('.');  // Non-printable characters as dot
          }
        }
        Serial.println();
      } else {
        Serial.println("Failed to read from the card.");
      }
    } else {
      Serial.println("Authentication failed.");
    }
  }
}

void loop(void) {


  uint16_t totalPeople;
  
  success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);

  if (success) {
    if (nfc.mifareclassic_AuthenticateBlock(uid, uidLength, 4, 0, keya)) {

      /* Just used for writing the tags
      // Prepare data to write
      uint8_t data[16] = { 0x01 };
        
      // Write the data to block 4
      success = nfc.mifareclassic_WriteDataBlock(4, data);
      */

      // Prepare buffer to store read data
      uint8_t data_read[16];
      
      // Read the data from block 4
      nfc.mifareclassic_ReadDataBlock(4, data_read);

      // Store the tag id
      uint8_t id = data_read[0];
      
      // Check if the previous tag id is the same as the current one.
      if (id != prevId) {
        prevId = id;

        Serial.println("Success auth");
        glassCount += 1;
        totalPeople = glassCount * GLASS_AMOUNT;
        
        // Send the data to the esp32 over uart
        Serial2.print(totalPeople);
        Serial2.print('\n');
      } else {
        Serial.println("ID is the same, skipping");
      }

      /* Convert decimal to binary */
      // uint8_t val = 0;
      // uint8_t remainder = totalPeople;
      // uint8_t b[8] = {0,0,0,0,0,0,0,0};
      // size_t cnt = 0;

      // while(remainder > 0) {
      //   b[cnt] = remainder % 2;
      //   remainder /= 2;
      //   cnt++;
      // }

      // for(size_t i; i < sizeof(b); ++i) {
      //   Serial.println(b[i]);
      // }

      // Clear uid
      memset(uid, 0, 7 * sizeof(char));

      delay(2000);
    }
  }
}

