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

// ===========================================
// Configuration
// ===========================================

#define GEIGER_INTERRUPT 0
#define DEBUG_PIN 7
#define LCD_PIN 4
#define XBEE_PIN 9

#define PERIOD_LENGTH 60000
#define UPDATES_PER_PERIOD 10
#define CPM_TO_USVH_RATIO 0.0057

// ===========================================
// Globals
// ===========================================

volatile unsigned long pulses = 0;
unsigned long ring[UPDATES_PER_PERIOD] = {0};
byte pointer = 0;
unsigned long next_update = 0;
boolean warmup = true;

SoftwareSerial lcd(0, LCD_PIN);
SoftwareSerial xbee(0, XBEE_PIN);

// ===========================================
// Interrupt routines
// ===========================================

void pulse() {
    ++pulses;
}

// ===========================================
// Methods
// ===========================================

void lcd_setCursor(byte col, byte row) {
    lcd.write(254);
    lcd.write(128 + 64 * row + col);
}

void lcd_clear() {
    lcd_setCursor(0, 0);
    lcd.write("                ");
    lcd.write("                ");
    lcd_setCursor(0, 0);
}

void showCPM() {

    // Calculating the CPM and uSvr/hr
    ring[pointer] = pulses;
    pulses = 0;
    pointer = (pointer + 1) % UPDATES_PER_PERIOD;
    
    unsigned long cpm = 0;
    for (byte i=0; i < UPDATES_PER_PERIOD; i++) {
        cpm += ring[i];
    }
    float usvh = cpm * CPM_TO_USVH_RATIO;

    // Showing data in LCD
    if (warmup) {
        lcd_setCursor(0, 1);
        lcd.print(cpm, DEC);
    } else {
        lcd_clear();
        lcd.print(F("CPM:    "));
        lcd.print(cpm, DEC);
        lcd_setCursor(0, 1);
        lcd.print(F("uSv/h: "));
        lcd.print(usvh, 3);
    }

    // Sending data through the XBee
    if (pointer == 0) {

        digitalWrite(DEBUG_PIN, HIGH);
        xbee.print(F("cpm:"));
        xbee.println(cpm, DEC);
        delay(20);
        //xbee.print(F("usvh:"));
        //xbee.println(usvh, 3);
        //delay(20);
        digitalWrite(DEBUG_PIN, LOW);

        // Finish the warmup after the first full period
        warmup = false;

    }

}

void setup() {

    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, LOW);

    // Initilize LCD and XBEE
    lcd.begin(9600);
    xbee.begin(9600);
    delay(500);

    // Show warmup message
    lcd_clear();
    lcd.println(F("Warming up..."));

    // Send welcome message
    xbee.println(F("status:1"));

    // Allow pulse to trigger interrupt on rising
    attachInterrupt(GEIGER_INTERRUPT, pulse, RISING);

    // Calculate next update
    next_update = millis() + PERIOD_LENGTH / UPDATES_PER_PERIOD;

}

void loop() {

    // Check if I have to send a report
    if (millis() > next_update) {
        next_update = millis() + PERIOD_LENGTH / UPDATES_PER_PERIOD;
        showCPM();
    }

}
