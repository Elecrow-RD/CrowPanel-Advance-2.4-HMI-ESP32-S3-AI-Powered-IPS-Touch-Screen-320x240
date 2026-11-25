#include <Arduino.h>
#include "LovyanGFX_Driver.h"

/*---------------------------------------------------------------
 * Visible PWM backlight test
 * The sketch holds GPIO38 at full brightness and draws the duty screen once.
 *--------------------------------------------------------------*/

// Provides LCD drawing so the test result is visible.
LGFX gfx;

// Sets the PWM carrier frequency in hertz.
const int pwmFreq = 5000;

// Uses 8-bit PWM, so duty cycle values range from 0 to 255.
const int pwmResolution = 8;

// GPIO38 controls the display backlight on this lesson board.
const int pwmPin = 38;

const int fullDuty = 255;

/**
 * @brief Draw the current PWM duty cycle on the LCD.
 *
 * The progress bar proves the sketch is running even when the hardware
 * backlight change is hard to see by eye.
 *
 * @param dutyCycle Current PWM duty cycle, from 0 to 255.
 * @return None.
 */
void drawDutyScreen(int dutyCycle) {
  int barWidth = (dutyCycle * 200) / 255;

  gfx.fillScreen(TFT_WHITE);
  gfx.setTextColor(TFT_BLACK, TFT_WHITE);
  gfx.setTextSize(2);
  gfx.setCursor(18, 45);
  gfx.println("PWM Backlight");
  gfx.setCursor(18, 75);
  gfx.println("GPIO38");

  gfx.setCursor(18, 120);
  gfx.print("Duty: ");
  gfx.print(dutyCycle);
  gfx.println(" / 255");

  gfx.drawRect(18, 165, 204, 24, TFT_BLACK);
  gfx.fillRect(20, 167, barWidth, 20, TFT_BLUE);
}

/**
 * @brief Initialize the LCD and attach PWM output to the backlight pin.
 *
 * Arduino calls this function once after reset. The backlight is first forced
 * on with a normal digital output, then LEDC PWM is attached for dimming.
 *
 * @param None.
 * @return None.
 */
void setup() {
  Serial.begin(115200);

  pinMode(pwmPin, OUTPUT);
  digitalWrite(pwmPin, HIGH);

  gfx.init();
  gfx.setRotation(1);
  bool pwmActive = ledcAttach(pwmPin, pwmFreq, pwmResolution) && ledcWrite(pwmPin, fullDuty);
  if (!pwmActive) {
    pinMode(pwmPin, OUTPUT);
    digitalWrite(pwmPin, HIGH);
    Serial.println("PWM setup failed; backlight held high.");
  }
  drawDutyScreen(fullDuty);
  Serial.println("Duty screen drawn.");
  if (pwmActive) {
    Serial.printf("PWM high counts: %lu / %u\n", (unsigned long)ledcRead(pwmPin), 1U << pwmResolution);
  }
}

void loop() {
  delay(1000);
}
