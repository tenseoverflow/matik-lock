#include <Wire.h>
#include <Keypad.h>   
#include <stdlib.h>   
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // Set the LCD address to 0x27 for a 16x2 display

int PIEZO = 6;
int motorPin1 = 7;
int motorPin2 = 2;    

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
  pinMode(motorPin2, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();
  
  randomSeed(analogRead(0));
  generateCode();            // Generate a random 6-digit code
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (key == '#') {
      lcd.clear();
      checkCode();
    } else if (key == '*') {
      enteredCode = "";
      Serial.println("\nInput cleared");
      lcd.clear();
    } else {
      if (enteredCode.length() < 10) {  // code lentgh limited to 10
        enteredCode += key;
        Serial.print(key);
        lcd.print(key);
      }
    }
  }
}

void generateCode() {
  generatedCode = "";
  //for (int i = 0; i < 6; i++) {
  //  generatedCode += String(random(1, 10));
  //}
  bool codeGeneration = True;
  while (codeGeneration) {
    char key = keypad.getKey();

    if (key) {
    if (key == '#') {
      lcd.clear();
      if (len(enteredCode) == 6) {
        lcd.print("Code has been accepted.");
        codeGeneration = False;
        generatedCode = enteredCode;
      } else {
        lcd.print("Try again");
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
  

  Serial.print("\nGenerated Code: ");
  Serial.println(generatedCode);
  
}

void checkCode() {
  if (enteredCode == generatedCode) {
    if (!isUnlocked) {
      // õige kood
      Serial.println("\nDoor Unlocked.");
      lcd.print("Uks on lahti.");
      tone(PIEZO, 1000, 500); 
      unlockMotor();   
      Serial.println("The door is now open.");
    } else {
      // paneb lukku uuesti
      Serial.println("\nDoor Locked.");
      lcd.print("Uks lukustab.");
      tone(PIEZO, 1000, 500);    
      unlockMotor();  
      Serial.println("The door is now closed.");
      lcd.clear();
    }
  } else {
    // vale kood
    Serial.println("\nIncorrect Code!");
    tone(PIEZO, 500, 500);  
    lcd.print("Vale kood.");
    delay(1000);                   
  }
  enteredCode = "";
}

void unlockMotor() {
  if (!isUnlocked) {
    // Open the lock (run motor forward)
    digitalWrite(motorPin1, HIGH);
    digitalWrite(motorPin2, LOW);
    delay(2000);  // (Aeg, mis votab mootoril, et uks avada)
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    isUnlocked = true;
  } else {
    // Close the lock (run motor backward)
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, HIGH);
    delay(2000);  // (Aeg, mis votab mootoril, et uks lukustada)
    digitalWrite(motorPin1, LOW);
    digitalWrite(motorPin2, LOW);
    isUnlocked = false;
  }

}
