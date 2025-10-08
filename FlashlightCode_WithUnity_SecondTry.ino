#include <bluefruit.h>
#include "nrf_gpio.h"

// ==== Pin Assignments ====
const int buttonPin = 9;
const int potPin = A0;
const int redLEDPin = 6;
const int greenLEDPin = 5;
const int clkPin = 10;
const int dtPin = 11;

// ==== State Variables ====
bool flashlightOn = false;
int lastClk = HIGH;
int colorIndex = 0;

void setup() {
pinMode(buttonPin, INPUT_PULLUP);
pinMode(potPin, INPUT);
pinMode(clkPin, INPUT_PULLUP);
pinMode(dtPin, INPUT_PULLUP);
pinMode(greenLEDPin, OUTPUT);
pinMode(redLEDPin, OUTPUT);

digitalWrite(redLEDPin, HIGH);
digitalWrite(greenLEDPin, LOW);

Serial.begin(9600);
delay(100);

// Start BLE, but immediately release the SPI pins
Bluefruit.begin();

// --- RECLAIM pins 9–11 from SPI / BLE ---
nrf_gpio_cfg_input(9, NRF_GPIO_PIN_PULLUP);
nrf_gpio_cfg_input(10, NRF_GPIO_PIN_PULLUP);
nrf_gpio_cfg_input(11, NRF_GPIO_PIN_PULLUP);
pinMode(buttonPin, INPUT_PULLUP);
pinMode(clkPin, INPUT_PULLUP);
pinMode(dtPin, INPUT_PULLUP);
}

void loop() {
// ===== BUTTON TOGGLE (edge detection) =====
static bool lastButtonState = HIGH;
bool currentButtonState = digitalRead(buttonPin);

if (lastButtonState == HIGH && currentButtonState == LOW) {
flashlightOn = !flashlightOn;
}
lastButtonState = currentButtonState;

// ===== BRIGHTNESS =====
int brightness = analogRead(potPin) / 4;

// ===== LED STATUS =====
digitalWrite(greenLEDPin, flashlightOn);
digitalWrite(redLEDPin, !flashlightOn);

// ===== ROTARY ENCODER =====
int clkState = digitalRead(clkPin);
if (clkState == HIGH && lastClk == LOW) {
if (digitalRead(dtPin) == HIGH) colorIndex++;
else colorIndex--;
if (colorIndex < 0) colorIndex = 3;
if (colorIndex > 3) colorIndex = 0;
}
lastClk = clkState;

// ===== SERIAL =====
Serial.print(flashlightOn ? 1 : 0);
Serial.print(" ");
Serial.print(brightness);
Serial.print(" ");
Serial.println(colorIndex);

delay(10);
}
