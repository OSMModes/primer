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
#include <avr/pgmspace.h>
#include "elapsedMillis.h"
#include "palette.h"
#include "pattern.h"

#define DEBUG
#define NUM_MODES 16
#define NUM_BUNDLES 4
#define EEPROM_VERSION 41

#define VERSION_ADDR 1023
#define BUNDLE_EEPROM_ADDR 900
uint16_t addrs[NUM_MODES] = {
  20, 60, 100, 140, 180, 220, 260, 300, 340, 380, 420, 460, 500, 540, 580, 620,
};

#define AMODE_OFF   0
#define AMODE_SPEED 1
#define AMODE_TILTX 2
#define AMODE_TILTY 3
#define AMODE_FLIPZ 4

#define ASENS_LOW     0
#define ASENS_MEDIUM  1
#define ASENS_HIGH    2

class Mode {
  public:
    Mode() : accel_mode(0), accel_sens(0), cur_variant(0), edit_color(0), acc_counter(0) {
      pattern[0] = Pattern();
      pattern[1] = Pattern();
    }

    void render(uint8_t& r, uint8_t& g, uint8_t& b) {
      if (cur_variant == 0) {
        pattern[1].render(r, g, b);
        pattern[0].render(r, g, b);
      } else {
        pattern[0].render(r, g, b);
        pattern[1].render(r, g, b);
      }
    }

    void init() {
      pattern[0].reset();
      pattern[1].reset();
      edit_color = 0;
    }

    void load(uint16_t addr) {
      accel_mode = EEPROM.read(addr);
      accel_sens = EEPROM.read(addr + 1);
      pattern[0].load(addr + 2);
      pattern[1].load(addr + 20);
    }

    void save(uint16_t addr) {
      EEPROM.update(addr, accel_mode);
      EEPROM.update(addr + 1, accel_sens);
      pattern[0].save(addr + 2);
      pattern[1].save(addr + 20);
    }

    void changeColor(int8_t v) {
      pattern[cur_variant].colors[edit_color] = (pattern[cur_variant].colors[edit_color] + v + NUM_COLORS) % NUM_COLORS;
    }

    void changeShade() {
      pattern[cur_variant].colors[edit_color] += 0x40;
    }

    void updateAccel(float fxg, float fyg, float fzg) {
      float pitch, level;
      uint8_t thresh;

      if (acc_counter < 0) acc_counter = 0;

      switch (accel_mode) {
        case AMODE_SPEED:
          if (accel_sens == ASENS_LOW) {
            level = 1.5; thresh = 25;
          } else if (accel_sens == ASENS_MEDIUM) {
            level = 1.4; thresh = 25;
          } else if (accel_sens == ASENS_HIGH) {
            level = 1.2; thresh = 5;
          }
          pitch = max(abs(fxg), max(abs(fyg), abs(fzg)));
          if (cur_variant == 0) {
            if (pitch > level) {
              acc_counter++;
            } else {
              acc_counter = 0;
            }
            if (acc_counter > thresh) {
              cur_variant = 1;
              acc_counter = thresh;
            }
          } else {
            if (pitch > 1.1) {
              acc_counter = thresh;
            } else {
              acc_counter--;
            }
            if (acc_counter <= 0) cur_variant = 0;
          }
          break;

        case AMODE_TILTX:
          pitch = (atan2(fxg, sqrt(fyg * fyg + fzg * fzg)) * 180.0) / M_PI;
          if (cur_variant == 0) {
            acc_counter += (pitch < -70) ? 1 : -1;
          } else {
            acc_counter += (pitch > 70) ? 1 : -1;
          }
          if (acc_counter > 60 >> accel_sens) {
            acc_counter = 0;
            cur_variant = (cur_variant == 0) ? 1 : 0;
          }
          break;

        case AMODE_TILTY:
          pitch = (atan2(fyg, sqrt(fxg * fxg + fzg * fzg)) * 180.0) / M_PI;
          if (cur_variant == 0) {
            acc_counter += (pitch < -70) ? 1 : -1;
          } else {
            acc_counter += (pitch > 70) ? 1 : -1;
          }
          if (acc_counter > 60 >> accel_sens) {
            acc_counter = 0;
            cur_variant = (cur_variant == 0) ? 1 : 0;
          }
          break;

        case AMODE_FLIPZ:
          if (cur_variant == 0) {
            acc_counter += (fzg < -0.9) ? 1 : -1;
          } else {
            acc_counter += (fzg > 0.9) ? 1 : -1;
          }
          if (acc_counter > 60 >> accel_sens) {
            acc_counter = 0;
            cur_variant = (cur_variant == 0) ? 1 : 0;
          }
          break;

        default:
          break;
      }
    }

    Pattern pattern[2];
    uint8_t cur_variant;
    uint8_t accel_mode, accel_sens;
    uint8_t edit_color;
    int16_t acc_counter;
};

Mode mode00 = Mode();
Mode mode01 = Mode();
Mode mode02 = Mode();
Mode mode03 = Mode();
Mode mode04 = Mode();
Mode mode05 = Mode();
Mode mode06 = Mode();
Mode mode07 = Mode();
Mode mode08 = Mode();
Mode mode09 = Mode();
Mode mode10 = Mode();
Mode mode11 = Mode();
Mode mode12 = Mode();
Mode mode13 = Mode();
Mode mode14 = Mode();
Mode mode15 = Mode();

Mode *modes[NUM_MODES] = {
  &mode00, &mode01, &mode02, &mode03, &mode04, &mode05, &mode06, &mode07,
  &mode08, &mode09, &mode10, &mode11, &mode12, &mode13, &mode14, &mode15,
};
Mode *mode;

const PROGMEM uint8_t factory_bundles[NUM_BUNDLES][NUM_MODES] = {
  {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15},
  {3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 0, 0, 0, 0, 0, 0},
  {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0},
  {12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};
const PROGMEM uint8_t factory_bundle_slots[NUM_BUNDLES] = {16, 9, 16, 4};

uint8_t bundles[NUM_BUNDLES][NUM_MODES];
uint8_t bundle_slots[NUM_BUNDLES];
uint8_t cur_bundle = 0;
uint8_t bundle_idx = 0;

elapsedMicros limiter = 0;

uint8_t accel_counter = 0;
int8_t xg, yg, zg;
float fxg, fyg, fzg;

uint8_t button_state = 0;
uint8_t new_state = 0;
uint8_t config_state = 0;
uint32_t since_trans = 0;

bool conjure = false;
bool conjure_toggle = false;

bool bpm_enabled = false;
uint32_t bpm_tracker = 0;
uint32_t bpm_trigger = 64000;
uint8_t times_clicked = 0;
uint32_t bpm_pressed = 0;
uint32_t total_time = 0;

// ********************************************************************
// **** SETUP CODE ****************************************************
// ********************************************************************
#define PRESS_DELAY 100   // 0.05s
#define SHORT_HOLD  1000  // 0.5s
#define LONG_HOLD   2000  // 1.0s
#define FLASH_TIME  500  // 0.5s

#define S_PLAY_OFF                  0
#define S_PLAY_PRESSED              1
#define S_PLAY_SLEEP_WAIT           2
#define S_PLAY_CONJURE_WAIT         3
#define S_PLAY_CONFIG_WAIT          4

#define S_WAKEUP_WAIT               9

#define S_CONFIG_MENU_OFF           10
#define S_CONFIG_MENU_PRESSED       11
#define S_CONFIG_MENU_EDIT_WAIT     12
#define S_CONFIG_MENU_EXIT_WAIT     13

#define S_COLOR_SELECT_OFF          20
#define S_COLOR_SELECT_PRESSED      21
#define S_COLOR_SELECT_SHADE_WAIT   23

#define S_COLOR_CONFIRM_OFF         30
#define S_COLOR_CONFIRM_PRESSED     31
#define S_COLOR_CONFIRM_REJECT_WAIT 32
#define S_COLOR_CONFIRM_EXIT_WAIT   33

#define S_PATTERN_SELECT_OFF        40
#define S_PATTERN_SELECT_PRESSED    41
#define S_PATTERN_SELECT_WAIT       42

#define S_ACC_MODE_SELECT_OFF       50
#define S_ACC_MODE_SELECT_PRESSED   51
#define S_ACC_MODE_SELECT_WAIT      52

#define S_ACC_SENS_SELECT_OFF       60
#define S_ACC_SENS_SELECT_PRESSED   61
#define S_ACC_SENS_SELECT_WAIT      62

#define S_BUNDLE_SELECT_START       70
#define S_BUNDLE_SELECT_OFF         71
#define S_BUNDLE_SELECT_PRESSED     72
#define S_BUNDLE_SELECT_WAIT        73
#define S_BUNDLE_SELECT_EDIT        74
#define S_BUNDLE_SELECT_BPM         75
#define S_BPM_SET_OFF               76
#define S_BPM_SET_PRESSED           77
#define S_MASTER_RESET_WAIT         78

#define S_BUNDLE_EDIT_OFF           80
#define S_BUNDLE_EDIT_PRESSED       81
#define S_BUNDLE_EDIT_WAIT          82
#define S_BUNDLE_EDIT_SAVE          83

#define S_SAVE_MODE                 90
#define S_SAVE_BUNDLES              91

#define PIN_R 9
#define PIN_G 6
#define PIN_B 5
#define PIN_BUTTON 2
#define PIN_LDO A3
#define MMA7660_ADDRESS 0x4C

#define CONFIG_PALETTE0    0
#define CONFIG_PALETTE1    1
#define CONFIG_PATTERN0    2
#define CONFIG_PATTERN1    3
#define CONFIG_ACC_MODE    4
#define CONFIG_ACC_SENS    5

#define FRAME_TICKS 32000

void saveBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    EEPROM.update(BUNDLE_EEPROM_ADDR + 64 + b, bundle_slots[b]);
    for (uint8_t i = 0; i < NUM_MODES; i++) {
      EEPROM.update(BUNDLE_EEPROM_ADDR + (b * NUM_MODES) + i, bundles[b][i]);
    }
  }
}

void loadBundles() {
  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    bundle_slots[b] = EEPROM.read(BUNDLE_EEPROM_ADDR + 64 + b);
    for (uint8_t i = 0; i < NUM_MODES; i++) {
      bundles[b][i] = EEPROM.read(BUNDLE_EEPROM_ADDR + (b * NUM_MODES) + i);
    }
  }
}

void saveModes() {
  for (uint8_t i = 0; i < NUM_MODES; i++) modes[i]->save(addrs[i]);
  EEPROM.update(VERSION_ADDR, EEPROM_VERSION);
}

void loadModes() {
  for (uint8_t i = 0; i < NUM_MODES; i++) modes[i]->load(addrs[i]);
}

void clearMemory() {
  for (int i = 0 ; i < EEPROM.length() ; i++) {
    EEPROM.write(i, 0);
  }
}

void resetMemory() {
  Serial.print(F("Writing factory settings v ")); Serial.print(EEPROM_VERSION); Serial.println(F(" to EEPROM."));
  clearMemory();

  mode00.accel_mode = AMODE_SPEED;
  mode00.accel_sens = ASENS_LOW;
  mode00.pattern[0] = Pattern(PATTERN_DOPS,      12, 0x01, 0x08, 0x01, 0x0C, 0x01, 0x10, 0x01, 0x14, 0x01, 0x18, 0x01, 0x1C, 0x00, 0x00, 0x00, 0x00);
  mode00.pattern[1] = Pattern(PATTERN_DASHDOPS,  13, 0x01, 0x9E, 0x9C, 0x9A, 0x98, 0x96, 0x94, 0x92, 0x90, 0x8E, 0x8C, 0x8A, 0x88, 0x00, 0x00, 0x00);

  mode01.accel_mode = AMODE_SPEED;
  mode01.accel_sens = ASENS_MEDIUM;
  mode01.pattern[0] = Pattern(PATTERN_BLINKE,     7, 0x01, 0xC7, 0xC6, 0xC5, 0xC4, 0xC3, 0xC2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode01.pattern[1] = Pattern(PATTERN_STROBIE,    8, 0x0A, 0x0D, 0x10, 0x13, 0x16, 0x19, 0x1C, 0x1F, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode02.accel_mode = AMODE_SPEED;
  mode02.accel_sens = ASENS_HIGH;
  mode02.pattern[0] = Pattern(PATTERN_HYPER,      3, 0x0E, 0x1E, 0x16, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode02.pattern[1] = Pattern(PATTERN_STROBE,     1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode03.accel_mode = AMODE_TILTX;
  mode03.accel_sens = ASENS_LOW;
  mode03.pattern[0] = Pattern(PATTERN_EDGE,      12, 0x1E, 0x16, 0x19, 0x88, 0x9F, 0x9E, 0x9D, 0x9C, 0xDB, 0xDA, 0xDA, 0xDA, 0x00, 0x00, 0x00, 0x00);
  mode03.pattern[1] = Pattern(PATTERN_EDGE,      12, 0x16, 0x5E, 0x5B, 0x94, 0x95, 0x96, 0x97, 0x98, 0xD9, 0xDA, 0xDA, 0xDA, 0x00, 0x00, 0x00, 0x00);

  mode04.accel_mode = AMODE_TILTX;
  mode04.accel_sens = ASENS_MEDIUM;
  mode04.pattern[0] = Pattern(PATTERN_TRACER,     9, 0xC2, 0x48, 0x4B, 0x4E, 0x51, 0x54, 0x57, 0x5A, 0x5D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode04.pattern[1] = Pattern(PATTERN_TRACER,     9, 0xC6, 0x48, 0x4B, 0x4E, 0x51, 0x54, 0x57, 0x5A, 0x5D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode05.accel_mode = AMODE_TILTX;
  mode05.accel_sens = ASENS_HIGH;
  mode05.pattern[0] = Pattern(PATTERN_CANDY,      8, 0x08, 0x0B, 0x08, 0x1D, 0x08, 0x0E, 0x08, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode05.pattern[1] = Pattern(PATTERN_CANDY,      8, 0x18, 0x1B, 0x18, 0x15, 0x18, 0x1E, 0x18, 0x12, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode06.accel_mode = AMODE_TILTY;
  mode06.accel_sens = ASENS_LOW;
  mode06.pattern[0] = Pattern(PATTERN_CHASE,      5, 0x1A, 0x1D, 0x08, 0x0B, 0x0E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode06.pattern[1] = Pattern(PATTERN_CHASE,      5, 0x12, 0x15, 0x18, 0x1B, 0x1E, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode07.accel_mode = AMODE_TILTY;
  mode07.accel_sens = ASENS_MEDIUM;
  mode07.pattern[0] = Pattern(PATTERN_RIBBON,    12, 0x08, 0x0A, 0x00, 0x00, 0x10, 0x12, 0x00, 0x00, 0x18, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode07.pattern[1] = Pattern(PATTERN_RIBBON,    12, 0x08, 0x00, 0x0C, 0x00, 0x10, 0x00, 0x14, 0x00, 0x18, 0x00, 0x1C, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode08.accel_mode = AMODE_TILTY;
  mode08.accel_sens = ASENS_HIGH;
  mode08.pattern[0] = Pattern(PATTERN_COMET,      5, 0x1E, 0x1F, 0x08, 0x09, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode08.pattern[1] = Pattern(PATTERN_COMET,      5, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode09.accel_mode = AMODE_FLIPZ;
  mode09.accel_sens = ASENS_LOW;
  mode09.pattern[0] = Pattern(PATTERN_PULSE,      6, 0x56, 0x00, 0x58, 0x00, 0x5A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode09.pattern[1] = Pattern(PATTERN_PULSE,      6, 0x5E, 0x00, 0x48, 0x00, 0x4A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode10.accel_mode = AMODE_FLIPZ;
  mode10.accel_sens = ASENS_MEDIUM;
  mode10.pattern[0] = Pattern(PATTERN_LEGO,       4, 0x48, 0x4A, 0x4C, 0x4D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode10.pattern[1] = Pattern(PATTERN_LEGO,       4, 0x58, 0x5A, 0x5C, 0x5D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode11.accel_mode = AMODE_FLIPZ;
  mode11.accel_sens = ASENS_HIGH;
  mode11.pattern[0] = Pattern(PATTERN_MORPH,      4, 0x10, 0x18, 0x08, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode11.pattern[1] = Pattern(PATTERN_MORPH,      4, 0x10, 0x08, 0x18, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode12.accel_mode = AMODE_OFF;
  mode12.accel_sens = ASENS_LOW;
  mode12.pattern[0] = Pattern(PATTERN_STROBIE,    2, 0x08, 0xD8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode12.pattern[1] = Pattern(PATTERN_STROBE,     1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode13.accel_mode = AMODE_OFF;
  mode13.accel_sens = ASENS_LOW;
  mode13.pattern[0] = Pattern(PATTERN_STROBIE,    2, 0x18, 0xC8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode13.pattern[1] = Pattern(PATTERN_STROBE,     1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode14.accel_mode = AMODE_OFF;
  mode14.accel_sens = ASENS_LOW;
  mode14.pattern[0] = Pattern(PATTERN_STROBIE,    2, 0x1A, 0xCE, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode14.pattern[1] = Pattern(PATTERN_STROBE,     1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  mode15.accel_mode = AMODE_OFF;
  mode15.accel_sens = ASENS_LOW;
  mode15.pattern[0] = Pattern(PATTERN_STROBIE,    2, 0x0E, 0xDA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
  mode15.pattern[1] = Pattern(PATTERN_STROBE,     1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

  for (uint8_t b = 0; b < NUM_BUNDLES; b++) {
    bundle_slots[b] = pgm_read_byte(&factory_bundle_slots[b]);
    for (uint8_t s = 0; s < NUM_MODES; s++) {
      bundles[b][s] = pgm_read_byte(&factory_bundles[b][s]);
    }
  }

  saveModes();
  saveBundles();
  EEPROM.update(VERSION_ADDR, EEPROM_VERSION);
}

void setup() {
  power_spi_disable();

  Serial.begin(57600);
  randomSeed(analogRead(0));

  pinMode(PIN_R, OUTPUT);
  pinMode(PIN_G, OUTPUT);
  pinMode(PIN_B, OUTPUT);
  pinMode(PIN_BUTTON, INPUT);
  pinMode(PIN_LDO, OUTPUT);
  digitalWrite(PIN_LDO, HIGH);

  Serial.println(F("\nWelcome to Primer!"));
  if (EEPROM_VERSION != EEPROM.read(VERSION_ADDR)) {
    Serial.println(F("Version mismatch. Clearning EEPROM."));
    resetMemory();
  } else {
    Serial.print(F("Version match. Reading saved settings v ")); Serial.print(EEPROM_VERSION); Serial.println(F(" from EEPROM."));
    loadModes();
    loadBundles();
  }

  for (uint8_t i = 0; i < NUM_MODES; i++) modes[i]->init();
  accInit();

  noInterrupts();
  ADCSRA = 0; // Disable ADC
  TCCR0B = (TCCR0B & 0b11111000) | 0b001;  // no prescaler ~64/ms
  TCCR1B = (TCCR1B & 0b11111000) | 0b001;  // no prescaler ~32/ms
  bitSet(TCCR1B, WGM12); // enable fast PWM                ~64/ms
  interrupts();

  delay(4000);
  limiter = 0;
  resetMode();
}

void readMode() {
  mode->pattern[0].pattern = Serial.read();
  /* modes[idx]->accel_mode = Serial.read(); */
  /* modes[idx]->accel_sens = Serial.read(); */
  /* for (uint8_t v = 0; v < 2; v++) { */
    /* modes[idx]->pattern[v].pattern = Serial.read(); */
    /* modes[idx]->pattern[v].num_colors = Serial.read(); */
    /* for (uint8_t s = 0; s < 16; s++) { */
    /*   modes[idx]->pattern[v].colors[s][0] = Serial.read(); */
    /*   modes[idx]->pattern[v].colors[s][1] = Serial.read(); */
    /*   modes[idx]->pattern[v].colors[s][2] = Serial.read(); */
    /* } */
  /* } */
}

void handleSerial() {
  if (Serial.available() >= 1) {
    switch (Serial.read()) {
      case 'L':
        readMode();
        break;
      default:
        break;
    }
  }
}

void loop() {
  uint8_t r, g, b;
  handleSerial();
  handlePress(digitalRead(PIN_BUTTON) == LOW);
  if (accel_counter >= 20) accel_counter = 0;
  if (accel_counter == 0) {
    accUpdate();
  } else if (accel_counter == 1) {
    fxg = translateAccel(xg);
  } else if (accel_counter == 2) {
    fyg = translateAccel(yg);
  } else if (accel_counter == 3) {
    fzg = translateAccel(zg);
  } else if (accel_counter == 4) {
    if (button_state < 10 || button_state >= 70) mode->updateAccel(fxg, fyg, fzg);
  }
  accel_counter++;

  if (button_state == S_PLAY_OFF) {
    mode->render(r, g, b);
  } else if (button_state == S_BUNDLE_SELECT_START || button_state == S_MASTER_RESET_WAIT) {
    r = 0; g = 0; b = 0;
  } else if (button_state < 10) {
    r = 0; g = 0; b = 0;
  } else if (button_state < 20) {
    switch (config_state) {
      case CONFIG_PALETTE0:
        r = 255; g = 0; b = 0;
        break;
      case CONFIG_PALETTE1:
        r = 0; g = 0; b = 255;
        break;
      case CONFIG_PATTERN0:
        r = 255; g = 0; b = 255;
        break;
      case CONFIG_PATTERN1:
        r = 0; g = 255; b = 255;
        break;
      case CONFIG_ACC_MODE:
        r = 0; g = 255; b = 0;
        break;
      default:
        r = 255; g = 255; b = 0;
        break;
    }
  } else if (button_state < 30) {
    unpackColor(mode->pattern[mode->cur_variant].colors[mode->edit_color], r, g, b);
  } else if (button_state < 50) {
    mode->render(r, g, b);
  } else if (button_state < 60) {
    switch (mode->accel_mode) {
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
    switch (mode->accel_sens) {
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
    mode->render(r, g, b);
  }

  if (conjure && conjure_toggle) {
    r = 0; g = 0; b = 0;
  }

  if (button_state == S_PLAY_OFF && bpm_enabled && bpm_tracker > bpm_trigger) {
    incMode();
    Serial.print(F("bpm next mode: ")); printCurMode(); Serial.println();
  }

  writeFrame(r, g, b);
  bpm_tracker++;
}

void flash(uint8_t r, uint8_t g, uint8_t b, uint8_t flashes) {
  for (uint8_t i = 0; i < flashes; i++) {
    for (uint8_t j = 0; j < 100; j++) {
      if (j < 50) { writeFrame(r, g, b); }
      else {        writeFrame(0, 0, 0); }
    }
  }
  since_trans += flashes * 100;
}

void writeFrame(uint8_t r, uint8_t g, uint8_t b) {
  while (limiter < FRAME_TICKS) {}
  limiter = 0;

  analogWrite(PIN_R, (r >> 1) + (r >> 2));
  analogWrite(PIN_G, (g >> 1) + (g >> 2));
  analogWrite(PIN_B, (b >> 1) + (b >> 2));
}

void resetMode() {
  bundle_idx = 0;
  mode = modes[bundles[cur_bundle][bundle_idx]];
  bpm_tracker = 0;
  mode->init();
  mode->cur_variant = 0;
  fxg = fyg = fzg = 0.0;
}

void incMode() {
  bundle_idx = (bundle_idx + 1) % bundle_slots[cur_bundle];
  mode = modes[bundles[cur_bundle][bundle_idx]];
  bpm_tracker = 0;
  mode->init();
  mode->cur_variant = 0;
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

float translateAccel(int8_t g) {
  if (g >= 64) {
    // Out of bounds, don't alter fg
    return 0.0;
  } else if (g >= 32) {
    // Translate 32  - 63 to -32 - -1
    g = -64 + g;
  }

  return (g / 20.0);
}

void accUpdate() {
  Wire.beginTransmission(MMA7660_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(MMA7660_ADDRESS, 3);

  if (Wire.available()) {
    xg = Wire.read();
    yg = Wire.read();
    zg = Wire.read();
  }
}


// ********************************************************************
// **** BUTTON CODE ***************************************************
// ********************************************************************
void printCurBundle() {
  Serial.print(F("bundle ")); Serial.print(cur_bundle + 1);
  Serial.print(F(" slots-")); Serial.print(bundle_slots[cur_bundle]);
}

void printCurMode() {
  Serial.print(F("bundle ")); Serial.print(cur_bundle + 1);
  Serial.print(F(" slot ")); Serial.print(bundle_idx + 1);
  Serial.print(F(": mode ")); Serial.print(bundles[cur_bundle][bundle_idx] + 1);
}

void printPalette() {
  Serial.print(F("mode ")); Serial.print(bundles[cur_bundle][bundle_idx] + 1);
  if (mode->cur_variant == 0) { Serial.print(F(" palette A")); }
  else {                        Serial.print(F(" palette B")); }
}

void printPaletteSlot() {
  printPalette();
  Serial.print(F(" slot ")); Serial.print(mode->edit_color + 1);
  Serial.print(F(": i-")); Serial.print(mode->pattern[mode->cur_variant].colors[mode->edit_color] & 0b00111111, HEX);
  Serial.print(F(" s-")); Serial.print(mode->pattern[mode->cur_variant].colors[mode->edit_color] >> 6);
}

void printAccMode() {
  Serial.print(F("mode ")); Serial.print(bundles[cur_bundle][bundle_idx]);
  switch (mode->accel_mode) {
    case AMODE_SPEED:
      Serial.print(F("accel mode: SPEED"));
      break;
    case AMODE_TILTX:
      Serial.print(F("accel mode: TILTX"));
      break;
    case AMODE_TILTY:
      Serial.print(F("accel mode: TILTY"));
      break;
    case AMODE_FLIPZ:
      Serial.print(F("accel mode: FLIPZ"));
      break;
    default: // case AMODE_OFF
      Serial.print(F("accel mode: OFF"));
      break;
  }
}

void printAccSensitivity() {
  Serial.print(F("mode ")); Serial.print(bundles[cur_bundle][bundle_idx]);
  switch (mode->accel_sens) {
    case ASENS_LOW:
      Serial.print(F("accel sensitivity: LOW"));
      break;
    case ASENS_MEDIUM:
      Serial.print(F("accel sensitivity: MEDIUM"));
      break;
    default: // case ASENS_HIGH:
      Serial.print(F("accel sensitivity: HIGH"));
      break;
  }
}

void printPattern() {
  Serial.print(F("mode ")); Serial.print(bundles[cur_bundle][bundle_idx] + 1);
  if (mode->cur_variant == 0) { Serial.print(F(" pattern A: ")); }
  else {                        Serial.print(F(" pattern B: ")); }
  switch (mode->pattern[mode->cur_variant].pattern) {
    case PATTERN_STROBE:
      Serial.print(F("STROBE"));
      break;
    case PATTERN_HYPER:
      Serial.print(F("HYPER STROBE"));
      break;
    case PATTERN_DOPS:
      Serial.print(F("DOPS"));
      break;
    case PATTERN_STROBIE:
      Serial.print(F("STROBIE"));
      break;
    case PATTERN_PULSE:
      Serial.print(F("PULSE"));
      break;
    case PATTERN_SEIZURE:
      Serial.print(F("SEIZURE STROBE"));
      break;
    case PATTERN_TRACER:
      Serial.print(F("TRACER"));
      break;
    case PATTERN_DASHDOPS:
      Serial.print(F("DASH DOPS"));
      break;
    case PATTERN_BLINKE:
      Serial.print(F("BLINK-E"));
      break;
    case PATTERN_EDGE:
      Serial.print(F("EDGE"));
      break;
    case PATTERN_LEGO:
      Serial.print(F("LEGO"));
      break;
    case PATTERN_CHASE:
      Serial.print(F("CHASE"));
      break;
    case PATTERN_MORPH:
      Serial.print(F("MORPH STROBE"));
      break;
    case PATTERN_RIBBON:
      Serial.print(F("RIBBON"));
      break;
    case PATTERN_COMET:
      Serial.print(F("COMET STROBE"));
      break;
    default: // case PATTERN_CANDY:
      Serial.print(F("CANDY STROBE"));
      break;
  }
}

void handlePress(bool pressed) {
  switch (button_state) {
    //******************************************************
    //** PLAY **********************************************
    //******************************************************
    case S_PLAY_OFF:
      if (since_trans == 0) {
        if (bpm_enabled) { Serial.print(F("playing bpm: ")); }
        else {             Serial.print(F("playing: ")); }
        printCurMode(); Serial.println();
      }
      if (pressed && since_trans >= PRESS_DELAY) {
        new_state = S_PLAY_PRESSED;
      }
      break;

    case S_PLAY_PRESSED:
      if (since_trans >= SHORT_HOLD) {
        new_state = S_PLAY_SLEEP_WAIT;
      } else if (!pressed) {
        if (conjure) {
          conjure_toggle = !conjure_toggle;
          if (conjure_toggle) { Serial.println(F("conjure: light off")); }
          else {                Serial.println(F("conjure: light on")); }
        } else {
          incMode();
        }
        new_state = S_PLAY_OFF;
      }
      break;

    case S_PLAY_SLEEP_WAIT:
      if (since_trans == 0) {
        Serial.print(F("sleep... "));
        flash(128, 128, 128, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_CONFIG_WAIT;
      } else if (!pressed) {
        Serial.println(F("sleeping"));
        enterSleep();
        new_state = S_WAKEUP_WAIT;
      }
      break;

    case S_PLAY_CONFIG_WAIT:
      if (since_trans == 0) {
        Serial.print(F("config... "));
        flash(128, 128, 0, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_CONJURE_WAIT;
      } else if (!pressed) {
        Serial.print(F("config: mode ")); printCurMode(); Serial.println();
        mode->edit_color = 0;
        config_state = CONFIG_PALETTE0;
        new_state = S_CONFIG_MENU_OFF;
      }
      break;

    case S_PLAY_CONJURE_WAIT:
      if (since_trans == 0) {
        Serial.print(F("toggle conjure... "));
        flash(0, 0, 128, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_PLAY_SLEEP_WAIT;
      } else if (!pressed) {
        conjure = !conjure;
        conjure_toggle = false;
        if (conjure) { Serial.println(F("conjure on")); }
        else {         Serial.println(F("conjure off")); }
        new_state = S_PLAY_OFF;
      }
      break;

    case S_WAKEUP_WAIT:
      if (since_trans == 0) {
      }
      if (!pressed) {
        new_state = S_PLAY_OFF;
      } else if (since_trans >= LONG_HOLD) {
        new_state = S_BUNDLE_SELECT_START;
      }
      break;


    //******************************************************
    //** CONFIG SELECT *************************************
    //******************************************************
    case S_CONFIG_MENU_OFF:
      if (since_trans == 0) {
        printConfigMode();
      }

      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_CONFIG_MENU_PRESSED;
      }
      break;

    case S_CONFIG_MENU_PRESSED:
      if (!pressed) {
        config_state = (config_state + 1) % 6;
        new_state = S_CONFIG_MENU_OFF;
      } else if (since_trans >= SHORT_HOLD) {
        new_state = S_CONFIG_MENU_EDIT_WAIT;
      }
      break;

    case S_CONFIG_MENU_EDIT_WAIT:
      if (since_trans == 0) {
        Serial.print(F("will enter config... "));
        flash(128, 128, 0, 5);
      }
      if (!pressed) {
        Serial.print(F("configuring: "));
        if (config_state < 2) {
          mode->edit_color = 0;
          mode->cur_variant = config_state % 2;
          printPaletteSlot();
          new_state = S_COLOR_SELECT_OFF;
        } else if (config_state < 4) {
          mode->cur_variant = config_state % 2;
          printPattern();
          new_state = S_PATTERN_SELECT_OFF;
        } else if (config_state == 4) {
          printAccMode();
          new_state = S_ACC_MODE_SELECT_OFF;
        } else {
          printAccSensitivity();
          new_state = S_ACC_SENS_SELECT_OFF;
        }
        Serial.println();
        mode->init();
      } else if (since_trans >= LONG_HOLD) {
        new_state = S_CONFIG_MENU_EXIT_WAIT;
      }
      break;

    case S_CONFIG_MENU_EXIT_WAIT:
      if (since_trans == 0) {
        Serial.print(F("will exit... "));
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        Serial.println(F("exit config menu"));
        mode->init();
        new_state = S_PLAY_OFF;
      }
      break;

    //******************************************************
    //** COLOR SELECT **************************************
    //******************************************************
    case S_COLOR_SELECT_OFF:
      if (since_trans == 0) {
        Serial.print(F("edit: ")); printPaletteSlot(); Serial.println();
      }

      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_COLOR_SELECT_PRESSED;
      }
      break;

    case S_COLOR_SELECT_PRESSED:
      if (since_trans >= SHORT_HOLD) {
        Serial.print(F("select: ")); printPaletteSlot(); Serial.println();
        flash(64, 64, 64, 5);
        new_state = S_COLOR_SELECT_SHADE_WAIT;
      } else if (!pressed) {
        mode->changeColor(1);
        Serial.print(F("next "));
        new_state = S_COLOR_SELECT_OFF;
      }
      break;

    case S_COLOR_SELECT_SHADE_WAIT:
      if (since_trans == 0) {
        Serial.print(F("select: ")); printPaletteSlot(); Serial.println();
        flash(64, 64, 64, 5);
      }
      if (since_trans >= LONG_HOLD) {
        mode->changeShade();
      } else if (!pressed) {
        mode->pattern[mode->cur_variant].num_colors = mode->edit_color + 1;
        Serial.print(F("selected: ")); printPaletteSlot(); Serial.println();
        new_state = S_COLOR_CONFIRM_OFF;
      }
      break;

    //******************************************************
    //** COLOR CONFIRM *************************************
    //******************************************************
    case S_COLOR_CONFIRM_OFF:
      if (since_trans == 0) {
        Serial.print(F("confirm: ")); printPaletteSlot(); Serial.println();
      }
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_COLOR_CONFIRM_PRESSED;
      }
      break;

    case S_COLOR_CONFIRM_PRESSED:
      if (since_trans >= LONG_HOLD) {
        new_state = S_COLOR_CONFIRM_REJECT_WAIT;
      } else if (!pressed) {
        mode->edit_color++;
        Serial.print(F("confirmed: ")); printPaletteSlot(); Serial.println();
        if (mode->edit_color == PALETTE_SIZE) {
          Serial.print(F("saving: ")); Serial.print(mode->pattern[mode->cur_variant].num_colors); Serial.println(F(" slots"));
          flash(128, 128, 128, 5);
          new_state = S_SAVE_MODE;
        } else {
          new_state = S_COLOR_SELECT_OFF;
        }
      }
      break;

    case S_COLOR_CONFIRM_REJECT_WAIT:
      if (since_trans == 0) {
        Serial.print(F("will reject... "));
        flash(128, 0, 0, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_COLOR_CONFIRM_EXIT_WAIT;
      } else if (!pressed) {
        Serial.println(F("reject"));
        if (mode->edit_color == 0) {
          new_state = S_COLOR_SELECT_OFF;
        } else {
          mode->edit_color--;
          mode->pattern[mode->cur_variant].num_colors--;
          new_state = S_COLOR_CONFIRM_OFF;
        }
      }
      break;

    case S_COLOR_CONFIRM_EXIT_WAIT:
      if (since_trans == 0) {
        Serial.print(F("will save... "));
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        Serial.print(F("confirmed: ")); printPaletteSlot(); Serial.println();
        mode->pattern[mode->cur_variant].num_colors = mode->edit_color + 1;
        Serial.print(F("saving: ")); Serial.print(mode->pattern[mode->cur_variant].num_colors); Serial.println(F(" slots"));
        new_state = S_SAVE_MODE;
      }
      break;


    //******************************************************
    //** PATTERN SELECT ************************************
    //******************************************************
    case S_PATTERN_SELECT_OFF:
      if (since_trans == 0) {
        Serial.print(F("editing: ")); printPattern(); Serial.println();
      }
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_PATTERN_SELECT_PRESSED;
      }
      break;

    case S_PATTERN_SELECT_PRESSED:
      if (since_trans >= LONG_HOLD) {
        new_state = S_PATTERN_SELECT_WAIT;
      } else if (!pressed) {
        mode->pattern[mode->cur_variant].pattern = (mode->pattern[mode->cur_variant].pattern + 1) % NUM_PATTERNS;
        Serial.print(F("switched: ")); printPattern(); Serial.println();
        mode->init();
        new_state = S_PATTERN_SELECT_OFF;
      }
      break;

    case S_PATTERN_SELECT_WAIT:
      if (since_trans == 0) {
        Serial.print(F("will save.. "));
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        Serial.print(F("saving: ")); printPattern(); Serial.println();
        new_state = S_SAVE_MODE;
      }
      break;


    //******************************************************
    //** ACCELEROMETER MODE SELECT *************************
    //******************************************************
    case S_ACC_MODE_SELECT_OFF:
      if (since_trans == 0) {
        Serial.print(F("accel mode: ")); printAccMode(); Serial.println();
      }
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_ACC_MODE_SELECT_PRESSED;
      }
      break;

    case S_ACC_MODE_SELECT_PRESSED:
      if (!pressed) {
        mode->accel_mode = (mode->accel_mode + 1) % 5;
        Serial.print(F("switched: ")); printAccMode(); Serial.println();
        new_state = S_ACC_MODE_SELECT_OFF;
      } else if (since_trans >= LONG_HOLD) {
        new_state = S_ACC_MODE_SELECT_WAIT;
      }
      break;

    case S_ACC_MODE_SELECT_WAIT:
      if (since_trans == 0) {
        Serial.print(F("will save..."));
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        Serial.print(F("saved: ")); printAccMode(); Serial.println();
        new_state = S_SAVE_MODE;
      }
      break;


    //******************************************************
    //** ACCELEROMETER SENSITITITY SELECT ******************
    //******************************************************
    case S_ACC_SENS_SELECT_OFF:
      if (since_trans == 0) {
        Serial.print(F("accel sens: ")); printAccSensitivity(); Serial.println();
      }
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_ACC_SENS_SELECT_PRESSED;
      }
      break;

    case S_ACC_SENS_SELECT_PRESSED:
      if (!pressed) {
        mode->accel_sens = (mode->accel_sens + 1) % 3;
        Serial.print(F("switched: ")); printAccSensitivity(); Serial.println();
        new_state = S_ACC_SENS_SELECT_OFF;
      } else if (since_trans >= LONG_HOLD) {
        new_state = S_ACC_SENS_SELECT_WAIT;
      }
      break;

    case S_ACC_SENS_SELECT_WAIT:
      if (since_trans == 0) {
        Serial.print(F("will save.. "));
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        Serial.print(F("saved: ")); printAccSensitivity(); Serial.println();
        new_state = S_SAVE_MODE;
      }
      break;


    //******************************************************
    //** BUNDLE SELECT *************************************
    //******************************************************
    case S_BUNDLE_SELECT_START:
      if (since_trans == 0) {
        Serial.println(F("will select... "));
        flash(0, 0, 128, 5);
      }
      if (!pressed) {
        bpm_enabled = false;
        new_state = S_BUNDLE_SELECT_OFF;
      } else if (since_trans > 6000) {
        new_state = S_MASTER_RESET_WAIT;
      }
      break;

    case S_BUNDLE_SELECT_OFF:
      if (since_trans == 0) {
        Serial.print(F("selecting: ")); printCurBundle(); Serial.println();
      }
      if (pressed && since_trans >= PRESS_DELAY) {
        new_state = S_BUNDLE_SELECT_PRESSED;
      }
      break;

    case S_BUNDLE_SELECT_PRESSED:
      if (since_trans >= SHORT_HOLD) {
        new_state = S_BUNDLE_SELECT_WAIT;
      } else if (!pressed) {
        cur_bundle = (cur_bundle + 1) % NUM_BUNDLES;
        Serial.print(F("switched: bundle ")); printCurBundle(); Serial.println();
        resetMode();
        new_state = S_BUNDLE_SELECT_OFF;
      }
      break;

    case S_BUNDLE_SELECT_WAIT:
      if (since_trans == 0) {
        Serial.print(F("will select bundle... "));
        flash(0, 0, 128, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_BUNDLE_SELECT_EDIT;
      } else if (!pressed) {
        Serial.print(F("select: bundle ")); Serial.println(cur_bundle + 1);
        new_state = S_PLAY_OFF;
      }
      break;

    case S_BUNDLE_SELECT_EDIT:
      if (since_trans == 0) {
        Serial.print(F("will edit bundle... "));
        flash(128, 128, 0, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_BUNDLE_SELECT_BPM;
      } else if (!pressed) {
        resetMode();
        Serial.println(F("edit"));
        new_state = S_BUNDLE_EDIT_OFF;
      }
      break;

    case S_BUNDLE_SELECT_BPM:
      if (since_trans == 0) {
        Serial.print(F("will toggle bpm switching... "));
        flash(0, 128, 0, 5);
      }
      if (!pressed) {
        total_time = 0;
        times_clicked = 0;
        bpm_pressed = 0;
        Serial.println(F("setting bpm"));
        bpm_trigger = 0;
        new_state = S_BPM_SET_OFF;
      }
      break;

    case S_BPM_SET_OFF:
      if (pressed && since_trans > PRESS_DELAY) {
        Serial.print(F("click #")); Serial.print(times_clicked);
        if (times_clicked != 0) {
          bpm_trigger += bpm_pressed;
          Serial.print(F(" this click ")); Serial.print(bpm_pressed);
          Serial.print(F(" total ")); Serial.print(bpm_trigger);
        }
        if (times_clicked >= 4) {
          bpm_trigger *= 4;
          Serial.print(F(" bpm trigger is ")); Serial.print(bpm_trigger);
        }
        Serial.println();
        times_clicked++;
        bpm_pressed = 0;
        new_state = S_BPM_SET_PRESSED;
      } else if (since_trans > 20000) {
        bpm_trigger = 64000;
        new_state = S_PLAY_OFF;
      }
      break;

    case S_BPM_SET_PRESSED:
      if (!pressed) {
        if (times_clicked >= 5) {
          bpm_tracker = 0;
          bpm_enabled = true;
          new_state = S_PLAY_OFF;
        } else {
          new_state = S_BPM_SET_OFF;
        }
      }
      break;

    case S_MASTER_RESET_WAIT:
      if (since_trans == 0) {
        Serial.print(F("will master reset... "));
        flash(128, 0, 0, 5);
      }
      if (!pressed) {
        Serial.println(F("master reset"));
        resetMemory();
        resetMode();
        new_state = S_PLAY_OFF;
      }
      break;


    //******************************************************
    //** BUNDLE EDIT ***************************************
    //******************************************************
    case S_BUNDLE_EDIT_OFF:
      if (since_trans == 0) {
        Serial.print(F("editing: ")); printCurMode(); Serial.println();
      }
      if (pressed && since_trans > PRESS_DELAY) {
        new_state = S_BUNDLE_EDIT_PRESSED;
      }
      break;

    case S_BUNDLE_EDIT_PRESSED:
      if (since_trans >= LONG_HOLD) {
        new_state = S_BUNDLE_EDIT_WAIT;
      } else if (!pressed) {
        bundles[cur_bundle][bundle_idx] = (bundles[cur_bundle][bundle_idx] + 1) % NUM_MODES;
        mode = modes[bundles[cur_bundle][bundle_idx]];
        mode->init();
        Serial.print(F("switched: ")); printCurMode(); Serial.println();
        new_state = S_BUNDLE_EDIT_OFF;
      }
      break;

    case S_BUNDLE_EDIT_WAIT:
      if (since_trans == 0) {
        Serial.print(F("will set... "));
        flash(128, 0, 128, 5);
      }
      if (since_trans >= LONG_HOLD) {
        new_state = S_BUNDLE_EDIT_SAVE;
      } else if (!pressed) {
        bundle_idx++;
        Serial.print(F("set: ")); printCurMode(); Serial.println();
        if (bundle_idx == NUM_MODES) {
          bundle_slots[cur_bundle] = bundle_idx;
          Serial.print(F("saved: ")); printCurBundle(); Serial.println();
          new_state = S_SAVE_BUNDLES;;
        } else {
          mode = modes[bundles[cur_bundle][bundle_idx]];
          mode->init();
          new_state = S_BUNDLE_EDIT_OFF;
        }
      }
      break;

    case S_BUNDLE_EDIT_SAVE:
      if (since_trans == 0) {
        Serial.print(F("will save... "));
        flash(128, 128, 128, 5);
      }
      if (!pressed) {
        bundle_slots[cur_bundle] = bundle_idx + 1;
        Serial.print(F("saved: ")); printCurBundle(); Serial.println();
        new_state = S_SAVE_BUNDLES;
      }
      break;


    case S_SAVE_MODE:
      mode->save(addrs[bundles[cur_bundle][bundle_idx]]);
      mode->init();
      new_state = S_PLAY_OFF;
      break;

    case S_SAVE_BUNDLES:
      saveBundles();
      resetMode();
      new_state = S_PLAY_OFF;
      break;

    default:
      new_state = S_PLAY_OFF;
      break;
  }

  if (button_state != new_state) {
    button_state = new_state;
    since_trans = 0;
  } else {
    since_trans++;
  }
  bpm_pressed++;
}

void printConfigMode() {
  Serial.print(F("mode ")); Serial.print(bundles[cur_bundle][bundle_idx] + 1);
  switch (config_state) {
    case CONFIG_PALETTE0:
      Serial.println(F(" configure palette A"));
      break;
    case CONFIG_PALETTE1:
      Serial.println(F(" configure palette B"));
      break;
    case CONFIG_PATTERN0:
      Serial.println(F(" configure pattern A"));
      break;
    case CONFIG_PATTERN1:
      Serial.println(F(" configure pattern B"));
      break;
    case CONFIG_ACC_MODE:
      Serial.println(F(" configure acc mode"));
      break;
    default: // case CONFIG_ACC_SENS:
      Serial.println(F(" configure acc sensitivity"));
      break;
  }
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
