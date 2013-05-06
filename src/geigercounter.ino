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

//#define DEBUG

#define GEIGER_INTERRUPT 0
#define DEBUG_PIN 7
#define LCD_PIN 4

#define REPORTS_PER_MINUTE 10

// ===========================================
// Globals
// ===========================================

volatile unsigned long pulses = 0;
unsigned long ring[REPORTS_PER_MINUTE] = {0};
byte pointer = 0;
unsigned long next_update = 0;

SoftwareSerial lcd(0, LCD_PIN);

// ===========================================
// Interrupt routines
// ===========================================

void pulse() {
    ++pulses;
}

// ===========================================
// Methods
// ===========================================

void clear_lcd() {
    lcd.write(254);
    lcd.write(128);
    lcd.write("                ");
    lcd.write("                ");
    lcd.write(254);
    lcd.write(128);
}

void showCPM() {

    // Calculating the watts to report
    ring[pointer++] = pulses;
    pulses = 0;
    if (pointer == REPORTS_PER_MINUTE) pointer=0;
    unsigned long cpm = 0;
    for (byte i=0; i < REPORTS_PER_MINUTE; i++) {
        cpm += ring[i];
    }

    float usvh = cpm * 0.0057;
    int usvh_i = usvh;
    int usvh_f = (usvh - usvh_i) * 1000;

    // Sending data
    clear_lcd();
    lcd.print("CPM: ");
    lcd.print(cpm, DEC);
    lcd.write(254);
    lcd.write(192);
    lcd.print("uSv/hr: ");
    lcd.print(usvh_i, DEC);
    lcd.print(".");
    lcd.print(usvh_f, DEC);

}

void setup() {

    pinMode(DEBUG_PIN, OUTPUT);
    digitalWrite(DEBUG_PIN, LOW);

    // Initilize LCD
    lcd.begin(9600);
    delay(500);

    Serial.begin(9600);

    // Send welcome message

    // Allow pulse to trigger interrupt on rising
    attachInterrupt(GEIGER_INTERRUPT, pulse, RISING);

    // Calculate next update
    next_update = millis() + 60000 / REPORTS_PER_MINUTE;

}

void loop() {

    // Check if I have to send a report
    if (millis() > next_update) {
        digitalWrite(DEBUG_PIN, HIGH);
        next_update = millis() + 60000 / REPORTS_PER_MINUTE;
        showCPM();
        digitalWrite(DEBUG_PIN, LOW);
    }

}
