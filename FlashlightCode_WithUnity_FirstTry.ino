#include <bluefruit.h>

// ==== Variables ====
const int buttonPin = 9; 
const int potPin = A0;
const int redLEDPin = 6;
const int greenLEDPin = 5;  
const int clkPin = 10;
const int dtPin = 11;

// ==== States ====
bool flashlightOn = false;  
int lastButtonState = HIGH;  
int lastClk = HIGH;          
int colorIndex = 0;          

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);
  pinMode(potPin, INPUT);
  pinMode(clkPin, INPUT_PULLUP);
  pinMode(dtPin, INPUT_PULLUP);
  pinMode(greenLEDPin, OUTPUT);
  pinMode(redLEDPin, OUTPUT);

  // starts with red on
  digitalWrite(redLEDPin, HIGH);
  digitalWrite(greenLEDPin, LOW);

  Serial.begin(9600);
}

void loop() {
  //=============button=============
  if (digitalRead(buttonPin) == LOW) {
    flashlightOn = !flashlightOn;
    delay(200);
  }

  int brightness = analogRead(potPin) / 4; // 0–255

  // LEDs to show Unity connection state
  if (flashlightOn) {
    digitalWrite(greenLEDPin, HIGH);
    digitalWrite(redLEDPin, LOW);
  } else {
    digitalWrite(greenLEDPin, LOW);
    digitalWrite(redLEDPin, HIGH);
  }

  // rotary encoder for color
  int clkState = digitalRead(clkPin);
  if (clkState != lastClk) {
    if (digitalRead(dtPin) != clkState) {
      colorIndex++;
    } else {
      colorIndex--;
    }
    if (colorIndex < 0) colorIndex = 0;
    if (colorIndex > 3) colorIndex = 3;
  }
  lastClk = clkState;

  // ---- SERIAL OUTPUT (numeric only!) ----
  // Format: onOff brightness colorIndex
  Serial.print(flashlightOn ? 1 : 0);
  Serial.print(" ");
  Serial.print(brightness);
  Serial.print(" ");
  Serial.println(colorIndex);

  delay(100);
}
