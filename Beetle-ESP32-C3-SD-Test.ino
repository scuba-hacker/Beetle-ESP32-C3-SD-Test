// Mark B. Jones - Scuba Hacker! - 20 May 2023 - MIT Licence
// Test the Adafruit SPI Flash SD Card - XTSD 512MB with the Beetle ESP32-C3 dev board
// https://www.dfrobot.com/product-2566.html
// https://learn.adafruit.com/adafruit-spi-flash-sd-card
// https://randomnerdtutorials.com/esp32-microsd-card-arduino/

// The only change from the Arduino SD sample for ESP32 was to set the Clock Select pin to 2 in the line SD.begin(SD_CHIP_SELECT_PIN)
// I also made all the success/fail error messages include the name of the directory or file being manipulated

/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */
#include "FS.h"
#include "SD.h"
#include "SPI.h"

void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.printf("Failed to open directory: %s\n",root);
        return;
    }
    if(!root.isDirectory()){
        Serial.printf("Not a directory: %s\n",root);
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.path(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.printf("Dir created: %s\n",path);
    } else {
        Serial.printf("mkdir failed: %s\n",path);
    }
}

void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.printf("Dir removed: %s\n",path);
    } else {
        Serial.printf("rmdir failed: %s\n",path);
    }
}

void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.printf("Failed to open file for reading: %s\n",path);
        return;
    }

    Serial.printf("Read from file: %s\n",path);
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}

void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.printf("Failed to open file for writing: %s\n",path);
        return;
    }
    if(file.print(message)){
        Serial.printf("File written: %s\n",path);
    } else {
        Serial.printf("Write failed: %s\n",path);
    }
    file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.printf("Failed to open file for appending: %s\n",path);
        return;
    }
    if(file.print(message)){
        Serial.printf("Message appended: %s\n",path);
    } else {
        Serial.printf("Append failed: %s\n",path);
    }
    file.close();
}

void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.printf("File renamed: %s to %s\n",path1, path2);
    } else {
        Serial.printf("Rename file failed: %s to %s\n",path1, path2);
    }
}

void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.printf("File deleted: %s\n",path);
    } else {
        Serial.printf("Delete failed: %s\n",path);
    }
}

void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms file: %s\n",flen, end, path);
        file.close();
    } else {
        Serial.printf("Failed to open file for reading: %s\n",path);
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.printf("Failed to open file for writing: %s\n",path);
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms file: %s\n", 2048 * 512, end, path);
    file.close(); 
}

void setup(){
    // a bit of LED flashing to start with - confirmed that code was getting written correctly when I started the project
    const int led = 10;
    pinMode(led,OUTPUT);
    for (int i=0; i<5;i++)
    {
      digitalWrite(led,HIGH);
      delay(500);
      digitalWrite(led,LOW);
      delay(500);
    }

    Serial.begin(115200);

    Serial.println("Here we go...");

    const uint8_t SD_CHIP_SELECT_PIN = 2;
    if(!SD.begin(SD_CHIP_SELECT_PIN)){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);

    listDir(SD, "/", 0);
    createDir(SD, "/mydir");
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");
    listDir(SD, "/", 2);
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    readFile(SD, "/hello.txt");
    deleteFile(SD, "/foo.txt");
    renameFile(SD, "/hello.txt", "/foo.txt");
    readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
}

void loop(){

}
