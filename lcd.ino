/* lcd.ino
 * ~~~~~~~
 * Please do not remove the following notices.
 * License: GPLv3. http://geekscape.org/static/arduino_license.html
 * ----------------------------------------------------------------------------
 *
 * By Marc MERLIN <marc_soft@merlins.org>, using the SR_LCD3 instantiation
 * of LiquidCrystal from
 * https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
 */

#include <LiquidCrystal_SR_LCD3.h>

byte lcdInitialized = false;
LiquidCrystal_SR_LCD3 lcd(PIN_LCD_DATA, PIN_LCD_CLOCK, PIN_LCD_STROBE);

void lcdInitialize(void) {
  pinMode(PIN_LCD_BACKLIGHT, OUTPUT);
  analogWrite(PIN_LCD_BACKLIGHT, DEFAULT_LCD_BACKLIGHT);

  lcd.begin(20, 4);              // initialize the lcd 
  lcd.home ();                   // go home

  lcdInitialized = true;
}

void lcdHandler(void) {
  if (! lcdInitialized) {
    lcdInitialize();
    lcd.clear();
    lcd.print(F("Pebble v2"));
  }

  lcd.setCursor(12, 0);
  if (hour < 10) lcd.print("0");
  lcd.print((int) hour);
  lcd.print(":");
  if (minute < 10) lcd.print("0");
  lcd.print((int) minute);
  lcd.print(":");
  if (second < 10) lcd.print("0");
  lcd.print((int) second);

  lcd.setCursor(0, 1);
  if (digitalRead(PIN_ROTARY_PUSH)) {
    lcd.print("knob:  ");
  }
  else {
    lcd.print("KNOB:  ");
  }
  lcd.print((int) rotaryEncoderValue);
  lcd.print("  ");

  lcd.setCursor(13, 1);
  lcd.print((int) temperatureWhole);
  lcd.print(".");
  if (temperatureFraction < 10) lcd.print("0");
  lcd.print((int) temperatureFraction);
  lcd.print(" C  ");

  lcd.setCursor(0, 2);
  lcd.print("Lux: ");
  lcd.print((int) lightValue);
  lcd.print("  ");

  // Show which color the LED is slowly changing to.
  char outputValChar[3];
  lcd.setCursor(0, 3);
  lcd.print(F("RGB#"));
  sprintf(outputValChar, "%02X", (int) RGB1b[2]);
  lcd.print(outputValChar);
  sprintf(outputValChar, "%02X", (int) RGB1b[1]);
  lcd.print(outputValChar);
  sprintf(outputValChar, "%02X", (int) RGB1b[0]);
  lcd.print(outputValChar);
}

#if 0
void lcdWrite(
  byte value,
  byte dataFlag) {

  digitalWrite(PIN_LCD_STROBE, LOW);

  byte output = value >> 4;                        // Most Significant Nibble

  if (dataFlag) {
    output = (output | LCD_RS_HIGH) & LCD_RW_LOW;  // Command or Data ?
  }

  for (byte loop1 = 0; loop1 < 2; loop1 ++) {   // First MSN, then LSN
    for (byte loop2 = 0; loop2 < 3; loop2 ++) { // LCD ENABLE LOW -> HIGH -> LOW
      output = (loop2 == 1) ?
       (output | LCD_ENABLE_HIGH) : (output & LCD_ENABLE_LOW);

      shiftOut(PIN_LCD_DATA, PIN_LCD_CLOCK, LSBFIRST, output);
      digitalWrite(PIN_LCD_STROBE, HIGH);
      delayMicroseconds(10);
      digitalWrite(PIN_LCD_STROBE,LOW);
    }
delay(1);
    output = value & 0x0F;                           // Least Significant Nibble

    if (dataFlag) {
      output = (output | LCD_RS_HIGH) & LCD_RW_LOW;  // Command or Data ?
    }
  }
}

void lcdClear(void) {
  lcdWrite(LCD_COMMAND_CLEAR, false);
  delay(2);
}

void lcdPosition(
  byte row,        // Must be either 0 (first row), 1, 2 or 3 (last row)
  byte column) {   // Must be between 0 and 19

  if (row > 1) column += 20;
  row = (row & 1)  ?  LCD_ODD_ROW_OFFSET  :  0;

  lcdWrite(LCD_COMMAND_SET_DDRAM_ADDRESS | row | column, false);
  delayMicroseconds(40);
}

void lcdWriteString(
  char message[]) {

  while (*message) lcdWrite((*message ++), true);
}

int estimateDigits(int nr) {
  int dec = 10;
  int temp = 1;
  int div = nr/dec;
  while (div > 0) {
    dec *= 10;
    div = nr/dec;
    temp++;
  }
  return temp;
}

int pow(int base, int expo) {
  int temp = 1;
  for (int c = 1; c <= expo; c++) {
    temp *= base;
  }
  return temp;
}

void lcdWriteNumber(int nr, int digits) {
  for (int i = digits-1; i >= 0; i--) {
    int dec = pow(10,i);
    int div = nr/dec;
    lcdWrite(div+48, true);
    if (div > 0) {
      nr -= div*dec;
    }
  }
}

void lcdWriteNumber(int nr) {
  int value = nr;

  if (value < 0) {
    lcdWrite('-', true);
    value = - nr;
  }

  int digits = estimateDigits(value);
  lcdWriteNumber(value, digits);
}
#endif

/* ------------------------------------------------------------------------- */
