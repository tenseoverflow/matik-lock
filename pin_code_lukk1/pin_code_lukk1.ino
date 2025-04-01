
#include <Wire.h>
#include <Keypad.h>   
#include <stdlib.h>   
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16x2 display

int PIEZO = 6;
int motorPin1 = 7;

bool LCDOccupied = false;

// Keypad
const byte ROWS = 4;  
const byte COLS = 4;  
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};
byte rowPins[ROWS] = {13, 12, 11, 10};
byte colPins[COLS] = {9, 8, A0, A1};    

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

String generatedCode = "";
String enteredCode = "";
bool isUnlocked = false;  // Lock State

void setup() {
  Serial.begin(9600);
  pinMode(PIEZO, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  randomSeed(analogRead(0));

  loadStoredCode();  // Load stored code from EEPROM

    if (generatedCode.length() != 6) {
        Serial.println("No valid code found, generating new one...");
        generateCode();
    }

}

void loop() {
  char key = keypad.getKey();

  if (key) {

    if (LCDOccupied) {
      lcd.clear();
      LCDOccupied = false;
    }

    if (key == '#') {
      lcd.clear();
      checkCode();
    } else if (key == '*') {
      enteredCode = "";
      Serial.println("\nInput cleared");
      lcd.clear();
    } else {
      if (enteredCode.length() < 10) {  // code length limited to 10
        enteredCode += key;
        Serial.print(key);
        lcd.print(key);
      }
    }
  }
}

void generateCode() {
  generatedCode = "";
  enteredCode = "";

  bool codeGeneration = true;
  while (codeGeneration) {
    char key = keypad.getKey();

    if (key) {
      if (LCDOccupied) {
        lcd.clear();
        LCDOccupied = false;
      }

      if (key == '#') {
        lcd.clear();
        if (enteredCode.length() == 6) {
          lcd.print("Code has been accepted.");
          codeGeneration = false;
          generatedCode = enteredCode;
          LCDOccupied = true;
        } else {
          lcd.print("Try again");
          LCDOccupied = true;
        }
      } else if (key == '*') {
        enteredCode = "";
        Serial.println("\nInput cleared");
        lcd.clear();
      } else {
        if (enteredCode.length() < 6) {  // code lentgh limited to 10
          enteredCode += key;
          Serial.print(key);
          lcd.print(key);
        }
      }
    }  
  }
  
  // Convert generatedCode to char array for EEPROM saving
  char codeArray[7]; // 6 digits + null terminator
  generatedCode.toCharArray(codeArray, 7);
  saveToEEPROM(0, codeArray);

  Serial.print("\nGenerated Code: ");
  Serial.println(generatedCode);

  LCDOccupied = true;
}

void checkCode() {
  if (enteredCode == generatedCode) {
    if (!isUnlocked) {
      // õige kood
      lcd.print("Uks on lahti.");
      tone(PIEZO, 1000, 500); 
      unlockMotor();   
      Serial.println("The door is now open.");

      LCDOccupied = true;
    } else {
      // paneb lukku uuesti
      lcd.print("Uks lukustab.");
      tone(PIEZO, 1000, 500);    
      unlockMotor();  
      Serial.println("The door is now closed.");
     
      LCDOccupied = true;
    }
  } else {
    // vale kood
    lcd.print("Vale kood.");
    tone(PIEZO, 500, 500);  
    Serial.println("\nIncorrect Code!");

    LCDOccupied = true;
  }
  enteredCode = "";
}

void unlockMotor() {
  if (!isUnlocked) {
    // Open the lock (run motor forward)
    digitalWrite(motorPin1, HIGH);
    isUnlocked = true;
  } else {
    digitalWrite(motorPin1, LOW);
    isUnlocked = false;
  }
}

void saveToEEPROM(int address, const char* str) {
    for (int i = 0; i < 6; i++) {
        EEPROM.write(address + i, str[i]);  // Save each character
    }
    EEPROM.write(address + 6, '\0');  // Null terminator for safety
}

void readFromEEPROM(int address, char* buffer, int length) {
    for (int i = 0; i < length; i++) {
        buffer[i] = EEPROM.read(address + i);
    }
    buffer[length] = '\0';  // Ensure null termination
}

void loadStoredCode() {
    char storedCode[7];  // 6 digits + null terminator
    readFromEEPROM(0, storedCode, 6);
    generatedCode = String(storedCode);

    Serial.print("Stored Code: ");
    Serial.println(generatedCode);
}
