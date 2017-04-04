/*

  Geiger-Muller Counter
  Copyright (C) 2013 by Xose Pérez <xose dot perez at gmail dot com>

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <SoftwareSerial.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// ===========================================
// Configuration
// ===========================================

// pin definitions
#define GEIGER_INTERRUPT 0
#define DEBUG_PIN 9
#define XBEE_PIN 12

// count data for a whole minute (60000 milliseconds)
// split into 10 chunks of 6 seconds each
#define PERIOD_LENGTH 60000
#define UPDATES_PER_PERIOD 10

// conversion factor from CPM to uSv/h based on data from Libellium for the SBM-20 tube
// http://www.cooking-hacks.com/index.php/documentation/tutorials/geiger-counter-arduino-radiation-sensor-board
#define CPM_TO_USVH 0.0057


// ===========================================
// Globals
// ===========================================

// pulses in the current subperiod
volatile unsigned long pulses = 0;

// stores the pulses for a set of 6 seconds periods
unsigned long ring[UPDATES_PER_PERIOD] = {0};

// pointer to the next cell in the ring to update
byte pointer = 0;

// keeps the sum of counts for the ring
unsigned long cpm = 0;

// time of the next update
unsigned long next_update = 0;

// during the first minute after a reset, the display shows a "warming up" message
boolean warmup = true;

// serial link to the radio
SoftwareSerial xbee(0, XBEE_PIN);

// LCD management
// Thanks to Riva for pointing out the wrong pin order
// http://arduino.cc/forum/index.php?topic=164722.0
// 0 -> RS
// 1 -> RW
// 2 -> EN
// 3 -> LED
// 4- > D4
// 5 -> D5
// 6 -> D6
// 7 -> D7
//
// Constructor with backlight control
// LiquidCrystal_I2C(uint8_t lcd_Addr, uint8_t En, uint8_t Rw, uint8_t Rs,.
//                  uint8_t d4, uint8_t d5, uint8_t d6, uint8_t d7,
//                  uint8_t backlighPin, t_backlighPol pol);
LiquidCrystal_I2C lcd(0x20, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

// ===========================================
// Interrupt routines
// ===========================================

/**
 * pulse
 *
 * Called on every LOW-to-HIGH transition on the 
 * GEIGER_INTERRUPT pin. It updates the current count.
 *
 * @return void
 */
void pulse() {
    ++pulses;
}

// ===========================================
// Methods
// ===========================================

/**
 * update
 *
 * Updates the ring and CPM count.
 * Shows the results on the LCD and sends them through radio every minute.
 *
 * @return void
 */
void update() {

    // calculate the moving sum of counts
    cpm = cpm - ring[pointer] + pulses;

    // store the current period value
    ring[pointer] = pulses;

    // reset the interrupt counter
    pulses = 0;

    // move the pointer to the next position in the ring
    pointer = (pointer + 1) % UPDATES_PER_PERIOD;

    // calculate the uSv/h 
    float usvh = cpm * CPM_TO_USVH;

    // show data in LCD
    if (warmup) {

        lcd.setCursor(0, 1);
        lcd.print(cpm, DEC);

    } else {

        lcd.clear();
        lcd.print(F("CPM:   "));
        lcd.print(cpm, DEC);
        lcd.setCursor(0, 1);
        lcd.print(F("uSv/h: "));
        lcd.print(usvh, 3);

    }

    // send data through radio everytime the ring loops back
    if (pointer == 0) {

        digitalWrite(DEBUG_PIN, HIGH);

        xbee.print(F("cpm:"));
        xbee.println(cpm, DEC);
        delay(20);
        xbee.print(F("usvh:"));
        xbee.println(usvh, 3);
        delay(20);

        digitalWrite(DEBUG_PIN, LOW);

        // finish the warmup after the first full period
        warmup = false;

    }

}

/**
 * setup
 *
 * Configures pins, LCD and XBee. Sets up interrups.
 *
 * @return void
 */
void setup() {

    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, LOW);

    // initilize LCD and XBEE
    lcd.begin(16,2);
    lcd.setBacklight(BACKLIGHT_ON);
    xbee.begin(9600);
    delay(500);

    // show warmup message
    lcd.home();
    lcd.print(F("Warming up..."));

    // send welcome message
    xbee.println(F("status:1"));

    // allow pulse to trigger interrupt on rising
    attachInterrupt(GEIGER_INTERRUPT, pulse, RISING);

    // calculate next update
    next_update = millis() + PERIOD_LENGTH / UPDATES_PER_PERIOD;

}

/**
 * loop
 *
 * Continuously checks for the next update time.
 *
 * @return void
 */
void loop() {

    // check if I have to update the info
    if (millis() > next_update) {
        next_update = millis() + PERIOD_LENGTH / UPDATES_PER_PERIOD;
        update();
    }

}
