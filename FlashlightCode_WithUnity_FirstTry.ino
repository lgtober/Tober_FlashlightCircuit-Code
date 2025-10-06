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
