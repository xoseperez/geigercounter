 /*

Moteino Geiger Counter

Copyright (C) 2017 by Xose Pérez <xose dot perez at gmail dot com>

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

#include "settings.h"
#include <RFM69Manager.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// -----------------------------------------------------------------------------
// Globals
// -----------------------------------------------------------------------------

RFM69Manager radio;
volatile boolean flag = false;
volatile unsigned long pulses = 0;
unsigned long ring[RING_SIZE] = {0};
byte ring_pointer = 0;
unsigned long cpm = 0;
boolean warmup = true;

LiquidCrystal_I2C lcd(0x20, 16, 2);

// -----------------------------------------------------------------------------
// Hardware
// -----------------------------------------------------------------------------

void countEvent() {
    ++pulses;
}

void blink(byte times, byte mseconds) {
    pinMode(LED_PIN, OUTPUT);
    for (byte i=0; i<times; i++) {
        if (i>0) delay(mseconds);
        digitalWrite(LED_PIN, HIGH);
        delay(mseconds);
        digitalWrite(LED_PIN, LOW);
    }
}

void i2cScan() {

    unsigned char address;
    unsigned char error;
    int nDevices = 0;
    char buffer[40];

    for (address = 1; address < 127; address++) {

        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0) {
            sprintf(buffer, "[I2C] Device found at address 0x%02X", address);
            Serial.println(buffer);
            nDevices++;
        }

    }

    if (nDevices == 0) Serial.println("[I2C] No devices found");

}

void hardwareSetup() {

    Serial.begin(SERIAL_BAUD);
    pinMode(LED_PIN, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(COUNTER_PIN), countEvent, RISING);

    Wire.begin();
    i2cScan();
    lcd.init();
    lcd.backlight();

    // show warmup message
    lcd.home();
    lcd.print(F("Warming up..."));

}

/**
 * update
 *
 * Updates the ring and CPM count.
 * Shows the results on the LCD and sends them through radio every minute.
 *
 * @return void
 */
void update() {

    // calculate the moving sum of pulses
    cpm = cpm - ring[ring_pointer] + pulses;

    // store the current period value
    ring[ring_pointer] = pulses;

    // reset the interrupt counter
    pulses = 0;

    // move the pointer to the next position in the ring
    ring_pointer = (ring_pointer + 1) % RING_SIZE;

    // show data in LCD
    if (warmup) {

        lcd.setCursor(0, 1);
        lcd.print(cpm, DEC);

    } else {

        // calculate the uSv/h
        float usvh = CPM_TO_USVH * cpm;

        lcd.clear();
        lcd.print(F("CPM:   "));
        lcd.print(cpm, DEC);
        lcd.setCursor(0, 1);
        lcd.print(F("uSv/h: "));
        lcd.print(usvh, 3);

    }

    // send data through radio everytime the ring loops back
    if (ring_pointer == 0) {

        // send pulses via radio
        sendValues();

        // notify sending
        blink(1, NOTIFICATION_TIME);

        // finish the warmup after the first full period
        warmup = false;

    }

}

// -----------------------------------------------------------------------------
// RFM69
// -----------------------------------------------------------------------------

void radioSetup() {
    radio.initialize(FREQUENCY, NODEID, NETWORKID, ENCRYPTKEY, GATEWAYID, ATC_RSSI);
    radio.sleep();
}

void sendValues() {

    char buffer[10];

    sprintf(buffer, "%lu", cpm);
    radio.send((char *) "CPM", buffer, (uint8_t) 2);

    float usvh = CPM_TO_USVH * cpm;
    dtostrf(usvh, 8, 3, buffer);
    char * p = buffer;
    while ((unsigned char) *p == ' ') ++p;
    radio.send((char *) "USH", p, (uint8_t) 2);

    radio.sleep();

}

// -----------------------------------------------------------------------------
// Common methods
// -----------------------------------------------------------------------------

void setup() {
    hardwareSetup();
    radioSetup();
}

void loop() {
    static unsigned long last = 0;
    if (millis() - last > UPDATE_INTERVAL) {
        last = millis();
        update();
    }
}
