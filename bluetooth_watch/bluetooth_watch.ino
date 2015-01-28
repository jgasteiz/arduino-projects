#include <SPI.h>
#include <ctype.h>
#include "Adafruit_BLE_UART.h"

#define ADAFRUITBLE_REQ 10
#define ADAFRUITBLE_RDY 2
#define ADAFRUITBLE_RST 9

Adafruit_BLE_UART uart = Adafruit_BLE_UART(ADAFRUITBLE_REQ, ADAFRUITBLE_RDY, ADAFRUITBLE_RST);

#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 8);

int LCD_WIDTH = 16;
int LCD_HEIGHT = 2;

int TIME_WIDTH = 8;

int TIME_START = LCD_WIDTH / 2 - TIME_WIDTH / 2;

int seconds = 0;
int minutes = 0;
int hours = 0;

/**************************************************************************/
/*!
    This function is called whenever select ACI events happen
*/
/**************************************************************************/
void aciCallback(aci_evt_opcode_t event)
{
  switch(event)
  {
    case ACI_EVT_DEVICE_STARTED:
      Serial.println(F("Advertising started"));
      break;
    case ACI_EVT_CONNECTED:
      Serial.println(F("Connected!"));
      break;
    case ACI_EVT_DISCONNECTED:
      Serial.println(F("Disconnected or advertising timed out"));
      break;
    default:
      break;
  }
}

/**************************************************************************/
/*!
    This function is called whenever data arrives on the RX channel
*/
/**************************************************************************/
void rxCallback(uint8_t *buffer, uint8_t len)
{
  Serial.print(F("Received "));
  
  Serial.print(len);
  Serial.print(F(" bytes: "));

  handleReceivedData(buffer, len);

  // Print the data as it comes
  Serial.print(F(" ["));
  for (int i = 0; i < len; i++) {
    Serial.print(" 0x");
    Serial.print((char)buffer[i], HEX);
  }
  Serial.println(F(" ]"));

  // Echo the same data back!
  uart.write(buffer, len);
  for (int i = 0; i < len; i++) {
    Serial.println(buffer[i]);
  }
}

/**************************************************************************/
/*!
    This function takes care of setting the received time
    if it's in the right format.
*/
/**************************************************************************/
void handleReceivedData(uint8_t *buffer, uint8_t len) {
  String received = "";

  for (int i = 0; i < len; i++) {
    received = received + (char)buffer[i];
    // Only room for 6 characters in LCD screen.
    if (i < 7) {
      receivedLCD = receivedLCD + (char)buffer[i];
    }
  }
  Serial.println(received);

  // Sets the time if it's valid.
  boolean isValid = isValidFormat(received);
  if (isValid) {
    Serial.println("Is valid!");
    setTime(received);
  } else {
    Serial.println("Is NOT valid!");
  }
}

/**************************************************************************/
/*!
    Given a valid formatted time string, sets the current time.
*/
/**************************************************************************/
void setTime(String receivedTime) {
  hours = receivedTime.substring(0, 2).toInt();
  minutes = receivedTime.substring(3, 5).toInt();
  seconds = 0;
}

/**************************************************************************/
/*!
    Returns true if the provided time has the format HH:MM
*/
/**************************************************************************/
boolean isValidFormat(String receivedTime) {
  String hours;
  String minutes;
  
  // HH:MM
  if (receivedTime.length() != 5) {
    return false;
  }

  // Get hours and minutes
  hours = receivedTime.substring(0, 2);
  minutes = receivedTime.substring(3, 5);
  
  // Check hours, minutes and seconds are all digits.
  if (!areAllDigits(hours) || !areAllDigits(minutes)) {
    return false;
  }
  
  // Check we get valid numbers.
  if (hours.toInt() > 23 || minutes.toInt() > 59) {
    return false;
  }
  
  return true;
}

/**************************************************************************/
/*!
    Returns true if for a given string all characters are digits.
*/
/**************************************************************************/
boolean areAllDigits(String digitsStr) {
  for (int i = 0; i < digitsStr.length(); i++) {
    if (isdigit(digitsStr.charAt(i)) == 0) {
      return false;
    }
  }
  return true;
}

/**************************************************************************/
/*!
    Returns a string representing the current time
*/
/**************************************************************************/
String getPrintableTime() {
  String secondsStr = String(seconds);
  String minutesStr = String(minutes);
  String hoursStr = String(hours);
  
  if (secondsStr.length() == 1) {
    secondsStr = "0" + secondsStr;
  }
  if (minutesStr.length() == 1) {
    minutesStr = "0" + minutesStr;
  }
  if (hoursStr.length() == 1) {
    hoursStr = "0" + hoursStr;
  }
  return hoursStr + ":" + minutesStr + ":" + secondsStr;
}

/**************************************************************************/
/*!
    Updates time every second.
*/
/**************************************************************************/
void updateTime() {
  seconds = seconds + 1; 
  if (seconds == 60) {
    seconds = 0;
    minutes = minutes + 1;
  }
  if (minutes == 60) {
    minutes = 0;
    hours = hours + 1;
  }
  if (hours == 24) {
    hours = 0;
  }
}

void setup() {
  lcd.begin(LCD_WIDTH, LCD_HEIGHT);
  lcd.setCursor(LCD_WIDTH / 2 - 2, 0);
  lcd.print("Time");
  
  Serial.begin(9600);
  while(!Serial); // Leonardo/Micro should wait for serial init
  Serial.println(F("Adafruit Bluefruit Low Energy nRF8001 Callback Echo demo"));

  uart.setRXcallback(rxCallback);
  uart.setACIcallback(aciCallback);
  uart.begin();
}

void loop() {
  uart.pollACI();
  
  lcd.setCursor(TIME_START, 1);
  lcd.print(getPrintableTime());
  updateTime();

  delay(1000);
}
