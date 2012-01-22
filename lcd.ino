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
const char menuline[] = "|         ";
const char hidemenuline[] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 0 };
const char nomenuline[] = "          ";
uint8_t menu_printed = 0;
uint8_t sub_menu_selected = 1;
// When we have a blinking cursor, after writing elsewhere, cursor needs to 
// be moved back to those coordinates.
uint8_t leave_cursor_at_x,leave_cursor_at_y = 0;


void lcdInitialize(void) {
    pinMode(PIN_LCD_BACKLIGHT, OUTPUT);
    analogWrite(PIN_LCD_BACKLIGHT, lcd_backlight);

    lcd.begin(20, 4);              // initialize the lcd 

    // load characters to the LCD
    lcd.createChar(0, heart);
    lcd.createChar(1, smiley);
    lcd.createChar(2, frownie);
    lcd.createChar(3, armsDown);  
    lcd.createChar(4, armsUp);  

    lcd.clear();

    lcd.print(F("I"));
    lcd.print(char(0));
    lcd.print(F("Pebblev2"));
}

void lcdHandler(void) {
    static char animatechar = 0;
    static uint8_t animate_x = 11;
    static uint8_t animate_oldx = 11;
    static uint8_t animate_dir = 1;
    static uint8_t delay_animation = 6;


    lcd.setCursor(9, 2);
    lcd.print(char((animatechar/delay_animation) + 3));
    animatechar = (animatechar + 1) % (delay_animation*2);

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
    lcd.setCursor(leave_cursor_at_x, leave_cursor_at_y);

    if (not in_menu)
    {
	if (! button_clicked)
	{
	    // Here are the things that print when the menu is not there.
	    lcd.setCursor(11, 0);
	    if (hour < 10) lcd.print("0");
	    lcd.print((int) hour);
	    lcd.print(":");
	    if (minute < 10) lcd.print("0");
	    lcd.print((int) minute);
	    lcd.print(":");
	    if (second < 10) lcd.print("0");
	    lcd.print((int) second);

	    lcd.setCursor(11, 1);
	    lcd.print("LED:");
	    lcd.print(RGB1_delay_factor);
	    lcd.print("/");
	    lcd.print(rgb1_darker_colors_shift);

	    // Change the overall darkness of the LEDs.
	    if (rotary_button_change)
	    {
		rgb1_darker_colors_shift += rotary_button_change;
		if (rgb1_darker_colors_shift==255) rgb1_darker_colors_shift = 0;
		if (rgb1_darker_colors_shift > 5)  rgb1_darker_colors_shift = 5;
	    }

	    lcd.setCursor(11, 2);
	    lcd.print("LCD: ");
	    lcd.print(lcd_backlight);
	    lcd.setCursor(leave_cursor_at_x, leave_cursor_at_y);

	    lcd.setCursor(animate_oldx, 3);
	    lcd.print(" ");
	    lcd.setCursor(animate_x, 3);
	    lcd.print(char((animatechar/delay_animation) + 1));
	    animate_oldx = animate_x;
	    if ((animatechar+delay_animation/2) % delay_animation == 0) animate_x += animate_dir;
	    if (animate_x == 11) animate_dir = 1;
	    if (animate_x == 19) animate_dir = -1;
	}
	else
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
	    leave_cursor_at_x = 11;
	    leave_cursor_at_y = 1;
	    lcd.setCursor(leave_cursor_at_x, leave_cursor_at_y);
	    lcd.blink();
	}
    }
    else
    {
	switch(menu_printed)
	{
	// Backlight / LED Delay / Exit
	case 1:
	    switch(sub_menu_selected)
	    {
	    // Backlight.
	    case 1:
		if (rotary_button_change == 1)
		{
		    sub_menu_selected = 3;
		    leave_cursor_at_x = 13;
		    leave_cursor_at_y = 3;
		    lcd.setCursor(leave_cursor_at_x, leave_cursor_at_y);
		}
		if (rotary_button_change == -1)
		{
		    sub_menu_selected = 2;
		    leave_cursor_at_x = 11;
		    leave_cursor_at_y = 2;
		    lcd.setCursor(leave_cursor_at_x, leave_cursor_at_y);
		}
		if (button_clicked)
		{
		    lcd.noBlink();
		    for (uint8_t l = 2; l < 4; l++)
		    {
			lcd.setCursor(10, l);
			lcd.print(menuline);
		    }
		    // Go to Backlight Menu.
		    menu_printed = 2;
		    lcd.setCursor(14, 2);
		    lcd.print(lcd_backlight);
		}
		break; 

	    // LED Delay
	    case 2:
		if (rotary_button_change == 1)
		{
		    sub_menu_selected = 1;
		    leave_cursor_at_x = 11;
		    leave_cursor_at_y = 1;
		    lcd.setCursor(leave_cursor_at_x, leave_cursor_at_y);
		}
		if (rotary_button_change == -1)
		{
		    sub_menu_selected = 3;
		    leave_cursor_at_x = 13;
		    leave_cursor_at_y = 3;
		    lcd.setCursor(leave_cursor_at_x, leave_cursor_at_y);
		}
		if (button_clicked)
		{
		    lcd.noBlink();
		    for (uint8_t l = 2; l < 4; l++)
		    {
			lcd.setCursor(10, l);
			lcd.print(menuline);
		    }
		    lcd.setCursor(11, 1);
		    lcd.print("LED Delay");
		    // Go to LED Delay menu.
		    menu_printed = 3;
		    lcd.setCursor(14, 2);
		    lcd.print(RGB1_delay_factor);
		}
		break; 

	    // Exit
	    case 3:
		if (rotary_button_change == 1)
		{
		    sub_menu_selected = 2;
		    leave_cursor_at_x = 11;
		    leave_cursor_at_y = 2;
		    lcd.setCursor(leave_cursor_at_x, leave_cursor_at_y);
		}
		if (rotary_button_change == -1)
		{
		    sub_menu_selected = 1;
		    leave_cursor_at_x = 11;
		    leave_cursor_at_y = 1;
		    lcd.setCursor(leave_cursor_at_x, leave_cursor_at_y);
		}
		if (button_clicked)
		{
		    lcd.noBlink();
		    in_menu = false;
		    for (uint8_t l = 0; l < 4; l++)
		    {
			lcd.setCursor(10, l);
			lcd.print(nomenuline);
		    }
		}
		break; 
	    }
	    break;

	// Backlight Menu
	case 2:
	    if (button_clicked)
	    {
		in_menu = false;
		for (uint8_t l = 0; l < 4; l++)
		{
		    lcd.setCursor(10, l);
		    lcd.print(nomenuline);
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
	    }
	    break; 

	// LED Delay Menu
	case 3:
	    if (button_clicked)
	    {
		in_menu = false;
		for (uint8_t l = 0; l < 4; l++)
		{
		    lcd.setCursor(10, l);
		    lcd.print(nomenuline);
		}
	    }
	    if (rotary_button_change)
	    {
		if (rotary_button_change == -1)
		{
		    RGB1_delay_factor= constrain(RGB1_delay_factor - 1, 0, 255);
		}
		if (rotary_button_change == 1)
		{
		    RGB1_delay_factor= constrain(RGB1_delay_factor + 1, 0, 255);
		}
		lcd.setCursor(14, 2);
		lcd.print(RGB1_delay_factor);
		lcd.print("  ");
	    }
	    break; 
	}
    }
    rotary_button_change = 0;
    button_clicked = false;
}

