/* aiko_pebble_v2.ino
 * ~~~~~~~~~~~~~~~~~~
 * Please do not remove the following notices.
 * License: GPLv3. http://geekscape.org/static/arduino_license.html
 * Version: 0.0
 *
 * Compile with a target of Arduino Uno.
 *
 * Documentation
 * ~~~~~~~~~~~~~
 * Project:  http://arduinominiconf.org
 * Software: http://geekscape.github.com/aiko_pebble_v2
 * Hardware  https://github.com/lukeweston/pebble20
 * ----------------------------------------------------------------------------
 *
 * Third-Party libraries
 * ~~~~~~~~~~~~~~~~~~~~~
 * These libraries are not included in the Arduino IDE and
 * need to be downloaded and installed separately.
 *
 * - Aiko framework
 *   https://github.com/geekscape/aiko_arduino
 *
 * To Do
 * ~~~~~
 * - Serial input changes relay outputs.
 * - Blink pin 13 LED.
 * - Use rotary encoder for simple menu and input for changing values.
 * - Touch panel alters RGB LED.
 * - ZigBee mesh networking.
 * - Code performance analysis (rotary encoder requires fast polling).
 */

#include <AikoCommands.h>
#include <AikoEvents.h>

using namespace Aiko;
using namespace Command;

#include "aiko_pebble_v2.h"

void setup(void) {
  Serial.begin(DEFAULT_BAUD_RATE);
  // Call LCD init early so that others can write to it.
  lcdInitialize();

  Events.addHandler(clockHandler,         1000);
  Events.addHandler(lightHandler,         1000);  // Aim for 250 ms
  Events.addHandler(rotaryEncoderHandler,  100);
  Events.addHandler(serialHandler,          30);  // Sufficient for 38,400 baud
  Events.addHandler(temperatureHandler,   1000);  // Aim for 250 ms
  Events.addHandler(touchPanelHandler,     250);  // Aim for  50 ms
  Events.addHandler(rgbLedFadeHandler,      10);  // Can run slower.
  // Call LCD last so that it can display variables that just changed.
  Events.addHandler(lcdHandler,           1000);  // Aim for 100 ms

  printFreeRam();
}

void loop(void) {
  Events.loop();
}

/* ------------------------------------------------------------------------- */
// http://jeelabs.org/2011/05/22/atmega-memory-use

int freeRam(void) {
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}

void printFreeRam () {
  Serial.print("Free RAM: ");
  Serial.println(freeRam());
}

/* ------------------------------------------------------------------------- */
