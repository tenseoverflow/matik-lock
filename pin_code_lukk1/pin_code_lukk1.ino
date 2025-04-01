#include <Wire.h>
#include <Keypad.h>   
#include <stdlib.h>   
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>  // Include EEPROM library

LiquidCrystal_I2C lcd(0x27, 16, 2); // LCD address 0x27 for a 16x2 display

int PIEZO = 6;
int motorPin1 = 7;

// Keypad setup
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

String userSetCode = "";  // User-defined code
String enteredCode = "";   // Stores user input
bool codeSet = false;      // Flag to track if the user has set a code
bool isUnlocked = false;   // Lock state
const String specialCode = "696969"; // Special code to generate new code

void setup() {
  Serial.begin(9600);
  pinMode(PIEZO, OUTPUT);
  pinMode(motorPin1, OUTPUT);
  
  lcd.init();
  lcd.backlight();
  lcd.clear();

  // Retrieve stored code from EEPROM (Address 0-5 for 6 characters)
  userSetCode = readCodeFromEEPROM();
  if (userSetCode != "") {
    lcd.print("Code Loaded");
    delay(2000);
    lcd.clear();
    lcd.print("Enter Code:");
    codeSet = true; // Code already set
  } else {
    lcd.print("Set 6-digit code:");
  }
}

void loop() {
  char key = keypad.getKey();

  if (key) {
    if (!codeSet) { 
      setNewCode(key);  // Handle code setup if not set
    } else {
      enterAndCheckCode(key);  // Once code is set, enter check mode
    }
  }
}

// **Function to let the user set their own 6-digit code**
void setNewCode(char key) {
  if (key == '#') {  
    if (userSetCode.length() == 6) {  // Accept only if exactly 6 digits
      codeSet = true;  
      storeCodeToEEPROM(userSetCode);  // Store the code in EEPROM
      lcd.clear();
      lcd.print("Code Set!");
      Serial.print("\nUser set code: ");
      Serial.println(userSetCode);
      delay(2000);
      lcd.clear();
      lcd.print("Enter Code:");
    } else {
      lcd.clear();
      lcd.print("Must be 6 digits!");
      delay(2000);
      lcd.clear();
      lcd.print("Set 6-digit code:");
      userSetCode = "";  // Reset input
    }
  } 
  else if (key == '*') {  
    userSetCode = "";  // Clear input
    lcd.clear();
    lcd.print("Set 6-digit code:");
  } 
  else if (isDigit(key)) {  
    userSetCode += key;  // Allow any length input
    Serial.print(key);
    lcd.setCursor(0, 1);
    lcd.print(userSetCode);
  }
}

// **Function to enter and check the code after setup**
void enterAndCheckCode(char key) {
  if (key == '#') {  
    if (enteredCode.length() != 6) {  // Only accept exactly 6-digit input
      lcd.clear();
      lcd.print("Must be 6 digits!");
      delay(2000);
      lcd.clear();
      lcd.print("Enter Code:");
      enteredCode = "";  
    } else {
      lcd.clear();
      checkCode();
    }
  } 
  else if (key == '*') {  
    enteredCode = "";
    lcd.clear();
    lcd.print("Enter Code:");
  } 
  else if (isDigit(key)) {  
    enteredCode += key;
    Serial.print(key);
    lcd.setCursor(0, 1);
    lcd.print(enteredCode);
  }
}

// **Function to check if input matches the stored code**
void checkCode() {
  if (enteredCode == userSetCode) {
    if (!isUnlocked) {
      Serial.println("\nDoor Unlocked.");
      lcd.print("Door Open");
      tone(PIEZO, 1000, 500);
      unlockMotor();
    } else {
      Serial.println("\nDoor Locked.");
      lcd.print("Door Locked");
      tone(PIEZO, 1000, 500);
      unlockMotor();
      lcd.clear();
    }
  } else if (enteredCode == specialCode) {
    // Special code entered, allow the user to set a new code
    Serial.println("\nSpecial Code Detected! Generate a New Code.");
    lcd.clear();
    lcd.print("Set New 6-Digit Code:");
    userSetCode = "";  // Reset the entered code for new setup
    codeSet = false;    // Reset the flag to re-enable setting a new code
  } else {
    Serial.println("\nIncorrect Code!");
    tone(PIEZO, 500, 500);
    lcd.print("Wrong Code");
    delay(1000);
  }
  enteredCode = "";
}

// **Motor control for lock/unlock**
void unlockMotor() {
  if (!isUnlocked) {
    digitalWrite(motorPin1, HIGH);
    isUnlocked = true;
  } else {
    digitalWrite(motorPin1, LOW);
    isUnlocked = false;
  }
}

// **Helper function to check if a key is a digit (0-9)**
bool isDigit(char key) {
  return key >= '0' && key <= '9';
}

// **Function to store the code in EEPROM**
void storeCodeToEEPROM(String code) {
  for (int i = 0; i < 6; i++) {
    EEPROM.write(i, code[i]);  // Store each character byte by byte
  }
}

// **Function to read the stored code from EEPROM**
String readCodeFromEEPROM() {
  String storedCode = "";
  for (int i = 0; i < 6; i++) {  // Assuming we're storing a 6-digit code
    char character = EEPROM.read(i);
    storedCode += character;
  }
  return storedCode;
}
