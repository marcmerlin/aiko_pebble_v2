/* lcd.ino
 * ~~~~~~~
 * Please do not remove the following notices.
 * License: GPLv3. http://geekscape.org/static/arduino_license.html
 * ----------------------------------------------------------------------------
 *
 * By Marc MERLIN <marc_soft@merlins.org>, using the SR_LCD3 instantiation
 * I wrote of LiquidCrystal from
 * https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
 */

#include <LiquidCrystal_SR_LCD3.h>

LiquidCrystal_SR_LCD3 lcd(PIN_LCD_DATA, PIN_LCD_CLOCK, PIN_LCD_STROBE);

// Create a set of new characters
// FIXME, use PROGMEM
byte heart[8] = {
  0b00000, 0b01010, 0b11111, 0b11111, 0b11111, 0b01110, 0b00100, 0b00000
};

byte smiley[8] = {
  0b00000, 0b00000, 0b01010, 0b00000, 0b00000, 0b10001, 0b01110, 0b00000
};

byte frownie[8] = {
  0b00000, 0b00000, 0b01010, 0b00000, 0b00000, 0b00000, 0b01110, 0b10001
};

byte armsDown[8] = {
  0b00100, 0b01010, 0b00100, 0b00100, 0b01110, 0b10101, 0b00100, 0b01010
};

byte armsUp[8] = {
  0b00100, 0b01010, 0b00100, 0b10101, 0b01110, 0b00100, 0b00100, 0b01010
};

// FIXME, change to PROGMEM
const char menuline[] = { 255, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', 0 };
const char hidemenuline[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0 };
const char nomenuline[] = "          ";
uint8_t menu_printed = 0;
uint8_t sub_menu_selected = 1;

void lcdInitialize(void) {
    pinMode(PIN_LCD_BACKLIGHT, OUTPUT);
    analogWrite(PIN_LCD_BACKLIGHT, lcd_backlight);

    // load characters to the LCD
    lcd.createChar(0, heart);
    lcd.createChar(1, smiley);
    lcd.createChar(2, frownie);
    lcd.createChar(3, armsDown);  
    lcd.createChar(4, armsUp);  

    lcd.begin(20, 4);              // initialize the lcd 
    lcd.clear();
    lcd.print(F("I"));
    lcd.print(char(0));
    lcd.print(F("Pebblev2"));
}

void lcdHandler(void) {
    lcd.setCursor(0, 1);
    lcd.print("T:");
    lcd.print((int) temperatureWhole);
    lcd.print(".");
    if (temperatureFraction < 10) lcd.print("0");
    lcd.print((int) temperatureFraction);
    lcd.print(char(0xDF));
    lcd.print("C ");

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

    if (not in_menu)
    {
	if (button_clicked)
	{

	    menu_printed = 1;
	    for (uint8_t l = 0; l < 4; l++)
	    {
		lcd.setCursor(10, l);
		lcd.print(menuline);
		in_menu = true;
	    }
	    lcd.setCursor(12, 0);
	    lcd.print("Change");
	    lcd.setCursor(11, 1);
	    lcd.print("Backlight");
	    lcd.setCursor(11, 2);
	    lcd.print("LED Delay");
	    lcd.setCursor(13, 3);
	    lcd.print("Exit");
	    lcd.setCursor(11, 1);
	    lcd.blink();
	}
	else
	{
	    lcd.setCursor(12, 0);
	    if (hour < 10) lcd.print("0");
	    lcd.print((int) hour);
	    lcd.print(":");
	    if (minute < 10) lcd.print("0");
	    lcd.print((int) minute);
	    lcd.print(":");
	    if (second < 10) lcd.print("0");
	    lcd.print((int) second);
	}
    }
    else
    {
	switch(menu_printed)
	{
	case 1:
	    switch(sub_menu_selected)
	    {
	    case 1:
		if (rotary_button_change == -1)
		{
		    sub_menu_selected = 3;
		    lcd.setCursor(13, 3);
		}
		if (rotary_button_change == 1)
		{
		    sub_menu_selected = 2;
		    lcd.setCursor(11, 2);
		}
		if (button_clicked)
		{
		    for (uint8_t l = 2; l < 4; l++)
		    {
			lcd.setCursor(10, l);
			lcd.print(menuline);
		    }
		    menu_printed = 2;
		    lcd.setCursor(14, 2);
		    lcd.print(lcd_backlight);
		}
		break; 

	    case 2:
		if (rotary_button_change == -1)
		{
		    sub_menu_selected = 1;
		    lcd.setCursor(11, 1);
		}
		if (rotary_button_change == 1)
		{
		    sub_menu_selected = 3;
		    lcd.setCursor(13, 3);
		}
		if (button_clicked)
		{
		    for (uint8_t l = 2; l < 4; l++)
		    {
			lcd.setCursor(10, l);
			lcd.print(menuline);
		    }
		    lcd.setCursor(11, 1);
		    lcd.print("LED Delay");
		    menu_printed = 3;
		    lcd.setCursor(15, 2);
		    lcd.print(RGB1_delay_factor);
		}
		break; 

	    case 3:
		if (rotary_button_change == -1)
		{
		    sub_menu_selected = 2;
		    lcd.setCursor(11, 2);
		}
		if (rotary_button_change == 1)
		{
		    sub_menu_selected = 1;
		    lcd.setCursor(11, 1);
		}
		if (button_clicked)
		{
		    for (uint8_t l = 0; l < 4; l++)
		    {
			lcd.setCursor(10, l);
			lcd.print(nomenuline);
			in_menu = false;
			lcd.noBlink();
		    }
		}
		break; 
	    }
	    break;

	case 2:
	    if (button_clicked)
	    {
		for (uint8_t l = 0; l < 4; l++)
		{
		    lcd.setCursor(10, l);
		    lcd.print(nomenuline);
		    in_menu = false;
		    lcd.noBlink();
		}
	    }
	    if (rotary_button_change)
	    {
		if (rotary_button_change == -1)
		{
		    lcd_backlight = constrain(lcd_backlight - 10, 0, 255);
		}
		if (rotary_button_change == 1)
		{
		    lcd_backlight = constrain(lcd_backlight + 10, 0, 255);
		}
		lcd.setCursor(14, 2);
		lcd.print(lcd_backlight);
		lcd.print("  ");
		analogWrite(PIN_LCD_BACKLIGHT, lcd_backlight);
		lcd.setCursor(14, 2);
	    }
	    break; 

	case 3:
	    if (button_clicked)
	    {
		for (uint8_t l = 0; l < 4; l++)
		{
		    lcd.setCursor(10, l);
		    lcd.print(nomenuline);
		    in_menu = false;
		    lcd.noBlink();
		}
	    }
	    if (rotary_button_change)
	    {
		if (rotary_button_change == -1)
		{
		    RGB1_delay_factor = constrain(RGB1_delay_factor - 3, 0, 255);
		}
		if (rotary_button_change == 1)
		{
		    RGB1_delay_factor = constrain(RGB1_delay_factor + 3, 0, 255);
		}
		lcd.setCursor(15, 2);
		lcd.print(RGB1_delay_factor);
		lcd.print("  ");
	    }
	    break; 
	}
    }
    rotary_button_change = 0;
    button_clicked = false;
}

