/*
The MIT License (MIT)

Copyright (c) 2015 John Joseph Miller

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <Arduino.h>
#include <EEPROM.h>
#include <Wire.h>
#include <avr/sleep.h>
#include <avr/power.h>
#include "elapsedMillis.h"
#include "anim.h"

#define DEBUG

#define NUM_MODES 12
#define NUM_BUNDLES 4
const uint8_t current_version = 100;

// MODE CONFIGURATION
// Mode mode# = Mode(MEMORY_ADDRESS, AMODE_WHAT, ASENS_WHAT,
//   PRIMEA, NUM_COLORS, COLOR1, COLOR2, COLOR3, COLOR4, COLOR5, COLOR6, COLOR7, COLOR8, COLOR9, COLOR10, COLOR11, COLOR12,
//   PRIMEB, NUM_COLORS, COLOR1, COLOR2, COLOR3, COLOR4, COLOR5, COLOR6, COLOR7, COLOR8, COLOR9, COLOR10, COLOR11, COLOR12);

Mode mode0 = Mode(520, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode mode1 = Mode(550, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode mode2 = Mode(580, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode mode3 = Mode(610, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode mode4 = Mode(640, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode mode5 = Mode(670, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode mode6 = Mode(700, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode mode7 = Mode(730, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode mode8 = Mode(760, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode mode9 = Mode(790, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode modeA = Mode(820, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode modeB = Mode(850, AMODE_SPEED, ASENS_HIGH,
  PRIME_STROBE,    12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E,
  PRIME_STROBIE,   12, 0x08, 0x0A, 0x0C, 0x0E, 0x10, 0x12, 0x14, 0x16, 0x18, 0x1A, 0x1C, 0x1E);

Mode *modes[12] = {&mode0, &mode1, &mode2, &mode3, &mode4, &mode5, &mode6, &mode7, &mode8, &mode9, &modeA, &modeB};
Mode *mode;

uint8_t bundles[NUM_BUNDLES][NUM_MODES] = {
  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11},
  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11},
  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11},
  { 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11},
};
uint8_t bundle_slots[NUM_BUNDLES] = {12, 12, 12, 12};
uint8_t cur_bundle = 0;
uint8_t bundle_idx = 0;

uint8_t edit_color;

elapsedMicros limiter = 0;

uint8_t accel_counter = 0;
float fxg, fyg, fzg = 0.0;

uint8_t button_state = 0;
uint8_t config_state = 0;
uint16_t since_press = 0;

bool conjure = false;
bool conjure_toggle = false;


// ********************************************************************
// **** SETUP CODE ****************************************************
// ********************************************************************
#define PIN_R 9
#define PIN_G 6
#define PIN_B 5
#define PIN_BUTTON 2
#define PIN_LDO A3
#define MMA7660_ADDRESS 0x4C

#define CONFIG_PALETTE0    0
#define CONFIG_PALETTE1    1
#define CONFIG_PRIME0      2
#define CONFIG_PRIME1      3
#define CONFIG_ACC_MODE    4
#define CONFIG_ACC_SENS    5

#define BUNDLE_EEPROM_ADDR 900

void saveBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    EEPROM.update(BUNDLE_EEPROM_ADDR + 50 + b, bundle_slots[b]);
    for (uint8_t i = 0; i < NUM_MODES; i++) {
      EEPROM.update(BUNDLE_EEPROM_ADDR + (b * NUM_MODES) + i, bundles[b][i]);
    }
  }
}

void loadBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    bundle_slots[b] = EEPROM.read(BUNDLE_EEPROM_ADDR + 50 + b);
    for (uint8_t i = 0; i < NUM_MODES; i++) {
      bundles[b][i] = EEPROM.read(BUNDLE_EEPROM_ADDR + (b * NUM_MODES) + i);
    }
  }
}

void saveModes() {
  for (uint8_t i = 0; i < NUM_MODES; i++) modes[i]->save();
}

void loadModes() {
  for (uint8_t i = 0; i < NUM_MODES; i++) modes[i]->load();
}

void setup() {
#ifndef DEBUG
  power_usart0_disable();
#endif
  power_spi_disable();

  Serial.begin(57600);
  Serial.println(F("\nWelcome to Primer!"));

  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_LDO, OUTPUT);
  digitalWrite(PIN_LDO, HIGH);

  if (current_version != EEPROM.read(512)) {
    Serial.println(F("Version mismatch. Writing new EEPROM."));
    saveModes();
    saveBundles();
    EEPROM.update(512, current_version);
  } else {
    Serial.println(F("Version match. Reading saved settings."));
    loadModes();
    loadBundles();
  }
  Serial.print(F("Version ")); Serial.println(current_version);

  for (uint8_t i = 0; i < NUM_MODES; i++) modes[i]->init();
  accInit();

  noInterrupts();
  ADCSRA = 0; // Disable ADC
  TCCR0B = (TCCR0B & 0b11111000) | 0b001;  // no prescaler ~1/64ms
  TCCR1B = (TCCR1B & 0b11111000) | 0b001;  // no prescaler ~1/64ms
  interrupts();

  delay(4000);
  limiter = 0;
  resetMode();
  randomSeed(analogRead(0));
}

void loop() {
  uint8_t r, g, b;

  handlePress(digitalRead(PIN_BUTTON) == LOW);

  if (accel_counter >= 10) accel_counter = 0;
  if (accel_counter == 0) {
    accUpdate();
    if (button_state < 10 || button_state >= 70) mode->updateAcc(fxg, fyg, fzg);
  }
  accel_counter++;

  if (button_state < 10) {
    mode->render(&r, &g, &b);
  } else if (button_state < 20) {
    switch (config_state) {
      case CONFIG_PALETTE0:
        r = 255; g = 0; b = 0;
        break;
      case CONFIG_PALETTE1:
        r = 0; g = 0; b = 255;
        break;
      case CONFIG_PRIME0:
        r = 255; g = 0; b = 255;
        break;
      case CONFIG_PRIME1:
        r = 0; g = 255; b = 255;
        break;
      case CONFIG_ACC_MODE:
        r = 0; g = 255; b = 0;
        break;
      default: // case CONFIG_ACC_SENS:
        r = 255; g = 255; b = 0;
        break;
    }
  } else if (button_state < 30) {
    unpackColor(mode->palette[mode->cur_variant][edit_color], &r, &g, &b);
  } else if (button_state < 50) {
    mode->render(&r, &g, &b);
  } else if (button_state < 60) {
    switch (mode->acc_mode) {
      case AMODE_SPEED:
        r = 255; g = 0; b = 0;
        break;
      case AMODE_TILTX:
        r = 0; g = 0; b = 255;
        break;
      case AMODE_TILTY:
        r = 255; g = 255; b = 0;
        break;
      case AMODE_FLIPZ:
        r = 0; g = 255; b = 0;
        break;
      default: // case AMODE_OFF
        r = 64; g = 64; b = 64;
        break;
    }
  } else if (button_state < 70) {
    switch (mode->acc_sensitivity) {
      case ASENS_LOW:
        r = 0; g = 0; b = 255;
        break;
      case ASENS_MEDIUM:
        r = 255; g = 0; b = 255;
        break;
      default: // case ASENS_HIGH:
        r = 255; g = 0; b = 0;
        break;
    }
  } else {
    mode->render(&r, &g, &b);
  }

  if (conjure && conjure_toggle) {
    r = 0; g = 0; b = 0;
  }

  writeFrame(r, g, b);
}

void flash(uint8_t r, uint8_t g, uint8_t b, uint8_t flashes) {
  for (uint8_t i = 0; i < flashes; i++) {
    for (uint8_t j = 0; j < 100; j++) {
      if (j < 50) {
        writeFrame(r, g, b);
      } else {
        writeFrame(0, 0, 0);
      }
    }
  }
}

void writeFrame(uint8_t r, uint8_t g, uint8_t b) {
  while (limiter < 64000) {}
  limiter = 0;

  analogWrite(PIN_R, r);
  analogWrite(PIN_G, g);
  analogWrite(PIN_B, b);
}

void resetMode() {
  bundle_idx = 0;
  mode = modes[bundles[cur_bundle][bundle_idx]];
  mode->init();
  fxg = fyg = fzg = 0.0;
}

void incMode() {
  bundle_idx = (bundle_idx + 1) % bundle_slots[cur_bundle];
  mode = modes[bundles[cur_bundle][bundle_idx]];
  mode->init();
  fxg = fyg = fzg = 0.0;
}


// ********************************************************************
// **** ACCEL CODE ****************************************************
// ********************************************************************
void accInit() {
  Wire.begin();
  accSend(0x07, 0x00);
  accSend(0x06, 0x10);
  accSend(0x08, 0x00);
  accSend(0x07, 0x01);
}

void accStandby() {
  Wire.begin();
  accSend(0x07, 0x10);
}

void accSend(uint8_t reg_address, uint8_t data) {
  Wire.beginTransmission(MMA7660_ADDRESS);
  Wire.write(reg_address);
  Wire.write(data);
  Wire.endTransmission();
}

float translateAccel(int8_t g, float fg, float alpha) {
  if (g >= 64) {
    // Out of bounds, don't alter fg
    return fg;
  } else if (g >= 32) {
    // Translate 32  - 63 to -32 - -1
    g = -64 + g;
  }

  return ((g / 20.0) * alpha) + (fg * (1.0 - alpha));
}

void accUpdate() {
  float alpha;
  switch (mode->acc_sensitivity) {
    case ASENS_LOW:
      alpha = 0.1;
      break;
    case ASENS_MEDIUM:
      alpha = 0.5;
      break;
    default: // case ASENS_HIGH:
      alpha = 0.9;
      break;
  }

  Wire.beginTransmission(MMA7660_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(MMA7660_ADDRESS, 3);

  if (Wire.available()) {
    fxg = translateAccel(Wire.read(), fxg, alpha);
    fyg = translateAccel(Wire.read(), fyg, alpha);
    fzg = translateAccel(Wire.read(), fzg, alpha);
  }
}


// ********************************************************************
// **** BUTTON CODE ***************************************************
// ********************************************************************
#define S_PLAY_OFF                    0
#define S_PLAY_PRESSED                1
#define S_PLAY_SLEEP_WAIT             2
#define S_PLAY_CONJURE_WAIT           3
#define S_PLAY_CONFIG_WAIT            4

#define S_CONFIG_SELECT_OFF           10
#define S_CONFIG_SELECT_PRESSED       11
#define S_CONFIG_SELECT_EDIT_WAIT     12
#define S_CONFIG_SELECT_EXIT_WAIT     13

#define S_COLOR_SELECT_OFF            20
#define S_COLOR_SELECT_PRESSED        21
#define S_COLOR_SELECT_PRESS_WAIT     22
#define S_COLOR_SELECT_SHADE_WAIT     23
#define S_COLOR_SELECT_RELEASE_WAIT   24

#define S_COLOR_CONFIRM_OFF           30
#define S_COLOR_CONFIRM_PRESSED       31
#define S_COLOR_CONFIRM_REJECT_WAIT   32
#define S_COLOR_CONFIRM_EXIT_WAIT     33

#define S_PRIME_SELECT_OFF            40
#define S_PRIME_SELECT_PRESSED        41
#define S_PRIME_SELECT_WAIT           42

#define S_ACC_MODE_SELECT_OFF         50
#define S_ACC_MODE_SELECT_PRESSED     51
#define S_ACC_MODE_SELECT_WAIT        52

#define S_ACC_SENS_SELECT_OFF         60
#define S_ACC_SENS_SELECT_PRESSED     61
#define S_ACC_SENS_SELECT_WAIT        62

#define S_BUNDLE_SELECT_OFF           70
#define S_BUNDLE_SELECT_PRESSED       71
#define S_BUNDLE_SELECT_WAIT          72
#define S_BUNDLE_SELECT_EDIT          73

#define S_BUNDLE_EDIT_OFF             80
#define S_BUNDLE_EDIT_PRESSED         81
#define S_BUNDLE_EDIT_WAIT            82
#define S_BUNDLE_EDIT_SAVE            83

void handlePress(bool pressed) {
  switch (button_state) {
    //******************************************************
    //** PLAY **********************************************
    //******************************************************
    case S_PLAY_OFF:
      if (pressed) {
        since_press = 0;
        button_state = S_PLAY_PRESSED;
      }
      break;

    case S_PLAY_PRESSED:
      if (!pressed) {
        if (conjure) {
          conjure_toggle = !conjure_toggle;
          if (conjure_toggle) { Serial.println(F("conjure_toggle on")); }
          else {                Serial.println(F("conjure_toggle off")); }
        } else {
          incMode();
          Serial.print(F("switched to mode ")); Serial.println(bundles[cur_bundle][bundle_idx]);
        }
        button_state = S_PLAY_OFF;
      } else if (since_press > 1000) {
        Serial.print(F("will sleep... "));
        flash(128, 128, 128, 5); since_press = 500;
        button_state = S_PLAY_SLEEP_WAIT;
      }
      break;

    case S_PLAY_SLEEP_WAIT:
      if (!pressed) {
        Serial.println(F("sleeping"));
        enterSleep();
      } else if (since_press > 1500) {
        Serial.print(F("toggle conjure... "));
        flash(0, 0, 128, 5); since_press = 500;
        button_state = S_PLAY_CONJURE_WAIT;
      }
      break;

    case S_PLAY_CONJURE_WAIT:
      if (!pressed) {
        if (conjure) { Serial.println(F("conjure off")); }
        else {         Serial.println(F("conjure on")); }
        conjure = !conjure;
        button_state = S_PLAY_OFF;
      } else if (since_press > 1500) {
        Serial.print(F("config... "));
        flash(128, 128, 0, 5);
        button_state = S_PLAY_CONFIG_WAIT;
      }
      break;

    case S_PLAY_CONFIG_WAIT:
      if (!pressed) {
        mode->init();
        Serial.println(F("configure palette A"));
        config_state = CONFIG_PALETTE0;
        button_state = S_CONFIG_SELECT_OFF;
      }
      break;

    //******************************************************
    //** CONFIG SELECT *************************************
    //******************************************************
    case S_CONFIG_SELECT_OFF:
      if (pressed) {
        since_press = 0;
        button_state = S_CONFIG_SELECT_PRESSED;
      }
      break;

    case S_CONFIG_SELECT_PRESSED:
      if (!pressed) {
        config_state = (config_state + 1) % 6;
        switch (config_state) {
          case CONFIG_PALETTE0:
            Serial.println(F("configure palette A"));
            break;
          case CONFIG_PALETTE1:
            Serial.println(F("configure palette B"));
            break;
          case CONFIG_PRIME0:
            Serial.println(F("configure prime A"));
            break;
          case CONFIG_PRIME1:
            Serial.println(F("configure prime B"));
            break;
          case CONFIG_ACC_MODE:
            Serial.println(F("configure acc mode"));
            break;
          default: // case CONFIG_ACC_SENS:
            Serial.println(F("configure acc sensitivity"));
            break;
        }
        button_state = S_CONFIG_SELECT_OFF;
      } else if (since_press > 1000) {
        Serial.print(F("will enter config... "));
        flash(128, 128, 0, 5); since_press = 500;
        button_state = S_CONFIG_SELECT_EDIT_WAIT;
      }
      break;

    case S_CONFIG_SELECT_EDIT_WAIT:
      if (!pressed) {
        switch (config_state) {
          case CONFIG_PALETTE0:
            Serial.println(F("configuring palette A"));
            mode->cur_variant = 0;
            edit_color = 0;
            button_state = S_COLOR_SELECT_OFF;
            break;
          case CONFIG_PALETTE1:
            Serial.println(F("configuring palette B"));
            mode->cur_variant = 1;
            edit_color = 0;
            button_state = S_COLOR_SELECT_OFF;
            break;
          case CONFIG_PRIME0:
            Serial.println(F("configuring prime A"));
            mode->cur_variant = 0;
            button_state = S_PRIME_SELECT_OFF;
            break;
          case CONFIG_PRIME1:
            Serial.println(F("configuring prime B"));
            mode->cur_variant = 1;
            button_state = S_PRIME_SELECT_OFF;
            break;
          case CONFIG_ACC_MODE:
            Serial.println(F("configuring acc mode"));
            button_state = S_ACC_MODE_SELECT_OFF;
            break;
          default: // case CONFIG_ACC_SENS:
            Serial.println(F("configuring acc sensitivity"));
            button_state = S_ACC_SENS_SELECT_OFF;
            break;
        }
      } else if (since_press > 1500) {
        Serial.print(F("will exit... "));
        flash(128, 128, 128, 5);
        button_state = S_CONFIG_SELECT_EXIT_WAIT;
      }
      break;

    case S_CONFIG_SELECT_EXIT_WAIT:
      if (!pressed) {
        mode->init();
        Serial.println(F("exit config"));
        button_state = S_PLAY_OFF;
      }
      break;

    //******************************************************
    //** COLOR SELECT **************************************
    //******************************************************
    case S_COLOR_SELECT_OFF:
      if (pressed) {
        since_press = 0;
        button_state = S_COLOR_SELECT_PRESSED;
      }
      break;

    case S_COLOR_SELECT_PRESSED:
      if (!pressed) {
        button_state = S_COLOR_SELECT_PRESS_WAIT;
      } else if (since_press > 1000) {
        Serial.print(F("will select... "));
        flash(64, 64, 64, 5); since_press = 500;
        button_state = S_COLOR_SELECT_SHADE_WAIT;
      }
      break;

    case S_COLOR_SELECT_PRESS_WAIT:
      if (pressed) {
        mode->palette[mode->cur_variant][edit_color] = (mode->palette[mode->cur_variant][edit_color] - 1) % 63;
        Serial.print(F("dec color ")); Serial.println(mode->palette[mode->cur_variant][edit_color] & 0b00111111);
        button_state = S_COLOR_SELECT_RELEASE_WAIT;
      } else if (since_press > 350) {
        mode->palette[mode->cur_variant][edit_color] = (mode->palette[mode->cur_variant][edit_color] + 1) % 63;
        Serial.print(F("inc color ")); Serial.println(mode->palette[mode->cur_variant][edit_color] & 0b00111111);
        button_state = S_COLOR_SELECT_OFF;
      }
      break;

    case S_COLOR_SELECT_SHADE_WAIT:
      if (!pressed) {
        Serial.print(F("now confirm mode ")); Serial.print(bundles[cur_bundle][bundle_idx]); Serial.print(F(" palette "));
        if (mode->cur_variant == 0) { Serial.print(F("A color ")); }
        else {                        Serial.print(F("B color ")); }
        Serial.println(edit_color);

        mode->num_colors[mode->cur_variant] = edit_color + 1;
        button_state = S_COLOR_CONFIRM_OFF;
      } else if (since_press > 1000) {
        mode->palette[mode->cur_variant][edit_color] += 0b01000000;
        Serial.print(F("shade ")); Serial.println(mode->palette[mode->cur_variant][edit_color] >> 6);
        flash(64, 64, 64, 5); since_press = 500;
      }
      break;

    case S_COLOR_SELECT_RELEASE_WAIT:
      if (!pressed) {
        button_state = S_COLOR_SELECT_OFF;
      }
      break;

    //******************************************************
    //** COLOR CONFIRM *************************************
    //******************************************************
    case S_COLOR_CONFIRM_OFF:
      if (pressed) {
        since_press = 0;
        button_state = S_COLOR_CONFIRM_PRESSED;
      }
      break;

    case S_COLOR_CONFIRM_PRESSED:
      if (!pressed) {
        Serial.print(F("confirm mode ")); Serial.print(bundles[cur_bundle][bundle_idx]); Serial.print(F(" palette "));
        Serial.print(F("confirmed color ")); Serial.println(edit_color);
        if (edit_color == PALETTE_SIZE - 1) {
          mode->save();
          mode->reset();
          Serial.print(F("saving mode ")); Serial.print(bundles[cur_bundle][bundle_idx]); Serial.print(F(" palette "));
          if (mode->cur_variant == 0) { Serial.print(F("A with ")); }
          else {                        Serial.print(F("B with ")); }
          Serial.print(edit_color + 1); Serial.println(F(" colors"));

          flash(128, 128, 128, 5);
          button_state = S_PLAY_OFF;
        } else {
          edit_color++;
          button_state = S_COLOR_SELECT_OFF;
        }
      } else if (since_press > 1000) {
        Serial.print(F("will reject... "));
        flash(128, 0, 0, 5); since_press = 500;
        button_state = S_COLOR_CONFIRM_REJECT_WAIT;
      }
      break;

    case S_COLOR_CONFIRM_REJECT_WAIT:
      if (!pressed) {
        Serial.print(F("reject color ")); Serial.println(edit_color);
        if (edit_color == 0) {
          Serial.println(F("chose color 0"));
          button_state = S_COLOR_SELECT_OFF;
        } else {
          edit_color--;
          Serial.print(F("confirm color ")); Serial.println(edit_color);
          mode->num_colors[mode->cur_variant] = edit_color - 1;
          button_state = S_COLOR_CONFIRM_OFF;
        }
      } else if (since_press > 1500) {
        Serial.print(F("will save... "));
        flash(128, 128, 128, 5);
        button_state = S_COLOR_CONFIRM_EXIT_WAIT;
      }
      break;

    case S_COLOR_CONFIRM_EXIT_WAIT:
      if (!pressed) {
        mode->save();
        mode->init();
        Serial.print(F("saving mode ")); Serial.print(bundles[cur_bundle][bundle_idx]); Serial.print(F(" palette "));
        if (mode->cur_variant == 0) { Serial.print(F("A with ")); }
        else {                        Serial.print(F("B with ")); }
        Serial.print(edit_color + 1); Serial.println(F(" colors"));
        button_state = S_PLAY_OFF;
      }
      break;


    //******************************************************
    //** PRIME SELECT **************************************
    //******************************************************
    case S_PRIME_SELECT_OFF:
      if (pressed) {
        since_press = 0;
        button_state = S_PRIME_SELECT_PRESSED;
      }
      break;

    case S_PRIME_SELECT_PRESSED:
      if (!pressed) {
        mode->prime[mode->cur_variant] = (mode->prime[mode->cur_variant] + 1) % 16;
        Serial.print(F("switching mode ")); Serial.print(bundles[cur_bundle][bundle_idx]); Serial.print(F(" prime "));
        if (mode->cur_variant == 0) { Serial.print(F("A to ")); }
        else {                        Serial.print(F("B to ")); }
        printPrime(mode->prime[mode->cur_variant]); Serial.println();

        mode->reset();
        button_state = S_PRIME_SELECT_OFF;
      } else if (since_press >= 1500) {
        Serial.print(F("will save.. "));
        flash(128, 128, 128, 5);
        button_state = S_PRIME_SELECT_WAIT;
      }
      break;

    case S_PRIME_SELECT_WAIT:
      if (!pressed) {
        mode->save();
        mode->init();
        Serial.print(F("saving mode ")); Serial.print(bundles[cur_bundle][bundle_idx]); Serial.print(F(" prime "));
        if (mode->cur_variant == 0) { Serial.print(F("A as ")); }
        else {                        Serial.print(F("B as ")); }
        printPrime(mode->prime[mode->cur_variant]); Serial.println();

        button_state = S_PLAY_OFF;
      }
      break;


    //******************************************************
    //** ACCELEROMETER MODE SELECT *************************
    //******************************************************
    case S_ACC_MODE_SELECT_OFF:
      if (pressed) {
        since_press = 0;
        button_state = S_ACC_MODE_SELECT_PRESSED;
      }
      break;

    case S_ACC_MODE_SELECT_PRESSED:
      if (!pressed) {
        mode->acc_mode = (mode->acc_mode + 1) % 5;
        Serial.print(F("switched mode ")); Serial.print(bundles[cur_bundle][bundle_idx]); Serial.print(F(" acc mode to "));
        printAccMode(mode->acc_mode); Serial.println();
        button_state = S_ACC_MODE_SELECT_OFF;
      } else if (since_press >= 1500) {
        Serial.print(F("will save..."));
        flash(128, 128, 128, 5);
        button_state = S_ACC_MODE_SELECT_WAIT;
      }
      break;

    case S_ACC_MODE_SELECT_WAIT:
      if (!pressed) {
        mode->save();
        mode->init();
        Serial.print(F("saved mode ")); Serial.print(bundles[cur_bundle][bundle_idx]); Serial.print(F(" acc mode as "));
        printAccMode(mode->acc_mode); Serial.println();
        button_state = S_PLAY_OFF;
      }
      break;


    //******************************************************
    //** ACCELEROMETER SENSITITITY SELECT ******************
    //******************************************************
    case S_ACC_SENS_SELECT_OFF:
      if (pressed) {
        since_press = 0;
        button_state = S_ACC_SENS_SELECT_PRESSED;
      }
      break;

    case S_ACC_SENS_SELECT_PRESSED:
      if (!pressed) {
        mode->acc_sensitivity = (mode->acc_sensitivity + 1) % 3;
        Serial.print(F("switched mode ")); Serial.print(bundles[cur_bundle][bundle_idx]); Serial.print(F(" acc sensitivity to "));
        printAccSensitivity(mode->acc_sensitivity); Serial.println();
        button_state = S_ACC_SENS_SELECT_OFF;
      } else if (since_press >= 1500) {
        Serial.print(F("will save.. "));
        flash(128, 128, 128, 5);
        button_state = S_ACC_SENS_SELECT_WAIT;
      }
      break;

    case S_ACC_SENS_SELECT_WAIT:
      if (!pressed) {
        mode->save();
        mode->init();
        Serial.print(F("saved mode ")); Serial.print(bundles[cur_bundle][bundle_idx]); Serial.print(F(" acc sensitivity as "));
        printAccSensitivity(mode->acc_sensitivity); Serial.println();
        button_state = S_PLAY_OFF;
      }
      break;


    //******************************************************
    //** BUNDLE SELECT *************************************
    //******************************************************
    case S_BUNDLE_SELECT_OFF:
      if (pressed) {
        since_press = 0;
        button_state = S_BUNDLE_SELECT_PRESSED;
      }
      break;

    case S_BUNDLE_SELECT_PRESSED:
      if (!pressed) {
        cur_bundle = (cur_bundle + 1) % NUM_BUNDLES;
        resetMode();
        Serial.print(F("switched to bundle ")); Serial.println(cur_bundle);
        button_state = S_BUNDLE_SELECT_OFF;
      } else if (since_press >= 1500) {
        Serial.print(F("will select bundle... "));
        flash(0, 0, 128, 5); since_press = 500;
        button_state = S_BUNDLE_SELECT_WAIT;
      }
      break;

    case S_BUNDLE_SELECT_WAIT:
      if (!pressed) {
        Serial.print(F("selected bundle ")); Serial.println(cur_bundle);
        button_state = S_PLAY_OFF;
      } else if (since_press >= 1500) {
        Serial.print(F("will edit bundle... "));
        flash(128, 128, 0, 5);
        button_state = S_BUNDLE_SELECT_EDIT;
      }
      break;

    case S_BUNDLE_SELECT_EDIT:
      if (!pressed) {
        Serial.print(F("edit bundle ")); Serial.println(cur_bundle);
        resetMode();
        button_state = S_BUNDLE_EDIT_OFF;
      }
      break;


    //******************************************************
    //** BUNDLE EDIT ***************************************
    //******************************************************
    case S_BUNDLE_EDIT_OFF:
      if (pressed) {
        since_press = 0;
        button_state = S_BUNDLE_EDIT_PRESSED;
      }
      break;

    case S_BUNDLE_EDIT_PRESSED:
      if (!pressed) {
        bundles[cur_bundle][bundle_idx] = (bundles[cur_bundle][bundle_idx] + 1) % NUM_MODES;
        Serial.print(F("switching bundle ")); Serial.print(cur_bundle); Serial.print(F(" slot ")); Serial.print(bundle_idx);
        Serial.print(F(" to ")); Serial.println(bundles[cur_bundle][bundle_idx]);
        button_state = S_BUNDLE_EDIT_OFF;
      } else if (since_press >= 1500) {
        Serial.print(F("will set... "));
        flash(128, 0, 128, 5); since_press = 500;
        button_state = S_BUNDLE_EDIT_WAIT;
      }
      break;

    case S_BUNDLE_EDIT_WAIT:
      if (!pressed) {
        Serial.println(F("set bundle ")); Serial.print(cur_bundle); Serial.print(F(" slot ")); Serial.print(bundle_idx);
        Serial.print(F(" to ")); Serial.println(bundles[cur_bundle][bundle_idx]);
        bundle_idx++;
        if (bundle_idx == NUM_MODES - 1) {
          saveBundles();
          resetMode();
          Serial.print(F("saved bundle ")); Serial.print(cur_bundle); Serial.print(F(". using "));
          Serial.print(bundle_idx + 1); Serial.println(F(" slots"));
          button_state = S_PLAY_OFF;
        } else {
          button_state = S_BUNDLE_EDIT_OFF;
        }
      } else if (since_press >= 1500) {
        Serial.print(F("will save... "));
        flash(128, 128, 128, 5);
        button_state = S_BUNDLE_EDIT_SAVE;
      }
      break;

    case S_BUNDLE_EDIT_SAVE:
      if (!pressed) {
        saveBundles();
        resetMode();
        bundle_slots[cur_bundle] = bundle_idx + 1;
        Serial.print(F("saved bundle ")); Serial.print(cur_bundle); Serial.print(F(". using "));
        Serial.print(bundle_idx + 1); Serial.println(F(" slots"));
        button_state = S_PLAY_OFF;
      }
      break;


    default:
      button_state = S_PLAY_OFF;
      break;
  }
  since_press++;
}


// ********************************************************************
// **** SLEEP CODE ****************************************************
// ********************************************************************
void enterSleep() {
  Serial.println(F("Going to sleep"));

  writeFrame(0, 0, 0);
  accStandby();
  digitalWrite(PIN_LDO, LOW);
  delay(4000);

  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  noInterrupts();
  attachInterrupt(0, pushInterrupt, FALLING);
  EIFR = bit(INTF0);
  MCUCR = bit(BODS) | bit(BODSE);
  MCUCR = bit(BODS);
  interrupts ();
  sleep_cpu ();

  // Wait until button is releaed
  uint16_t held_count = 0;
  limiter = 0;
  while (digitalRead(PIN_BUTTON) == LOW) {
    if (limiter > 64000) {
      limiter = 0;
      held_count++;
    }
    if (held_count == 1500) {
      Serial.print(F("will select bundle... "));
      flash(0, 0, 128, 5);
    }
  }

  if (held_count > 1500) {
    Serial.println(F("select bundle"));
    button_state = S_BUNDLE_SELECT_OFF;
  } else {
    Serial.println(F("play"));
    button_state = S_PLAY_OFF;
  }

  digitalWrite(PIN_LDO, HIGH);
  accInit();
  resetMode();
  conjure = conjure_toggle = false;
  delay(4000);
}

void pushInterrupt() {
  sleep_disable();
  detachInterrupt(0);
}
