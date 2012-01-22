/* rgb_led.ino
 * ~~~~~~~~~~~
 * Please do not remove the following notices.
 * License: GPLv3. http://geekscape.org/static/arduino_license.html
 * ----------------------------------------------------------------------------
 * By Marc MERLIN <marc_soft@merlins.org>
 */

byte rgbLedFadeInitialized = false;

// 255 is bright, 0 is dark, some RGB LEDs are the other way around.
#define RGB1_HIGH     255
#define RGB1_LOW      0

#define RGB1_BLUE     PIN_LED_BLUE
#define RGB1_GREEN    PIN_LED_GREEN
#define RGB1_RED      PIN_LED_RED

// How far we are in each light pattern.
uint8_t RGB1_led_stage = 0;

// map digital values for rgb led 1 and 2, i.e. what to write for full bright
// and off. Index 0 is not used in this code.
// 255 is bright, 0 is dark, some RGB LEDs are the other way around.
uint8_t RGB_HIGH[3] = { 128, 255 };
uint8_t RGB_LOW[3] = { 128, 0 };

//                        Blue	      Green,      Red
float RGB1a[3] =	{ RGB_LOW[1], RGB_LOW[1], RGB_LOW[1] };
float RGB1b[3];
float RGB1diff[3];
uint8_t RGB1pins[3] =	{ RGB1_BLUE,  RGB1_GREEN, RGB1_RED };

// How many intermediate shades of colors.
#define RGB1_COLOR_CHANGE_STEPS 70
// How many steps we hold the target colors.
#define RGB1_COLOR_HOLD_STEPS 30

// Keep track which preseeded color is next. Once we're past RGB1_INIT_COLOR_MAX
// we're done and we start picking random colors.
#define RGB1_INIT_COLOR_MAX 6
uint8_t RGB1_init_color_changes = 0;
// These colors are based on 0 bright, 255 dark. They are corrected in color
// picker if necessary.
uint8_t RGB1_init_colors[RGB1_INIT_COLOR_MAX][3] = 
		    { 
			{ 0   , 255 , 255  },
			{ 255 , 255 , 255  },
			{ 255 , 0   , 255  },
			{ 255 , 255 , 255  },
			{ 255 , 255 , 0    },
			{ 255 , 255 , 255  },
		    };

// How many times we get into the handler before we actually run its content
// At init time, we change quickly to do a nice init swoop on and off
// Once that's done, we slow down to one color change every 5 seconds.
#define RGB1_INIT_DELAY_FACTOR 2
#define RGB1_RUN_DELAY_FACTOR 6

// We have an interrupt every 10ms and RGB1_COLOR_CHANGE_STEPS changes to get to the
// next color, so that's 1 second with a delay factor of 1.
uint8_t RGB1_delay_factor = RGB1_INIT_DELAY_FACTOR;
// How many times we entered the loop.
uint8_t RGB1_loop_count = 1;

// Show how quickly the color steps are changed.
#define RGB1_COLOR_STEP_STATUS  PIN_OUTPUT_1
uint8_t rgb1_color_step_state = 0;
// Toggle when a new color is generated.
#define RGB1_NEW_COLOR_STATUS   PIN_OUTPUT_2
uint8_t rgb1_color_color_state = 0;

// Good values to make the LED colors darker are 0 to 5 ( 1, 2, 4, 8, 16, 32 )
// Unfortunately the PWM resolution is only 8 bits, so when you shift towards
// darker colors, there are few steps left adn the color change steps become
// visible.
uint8_t rgb1_darker_colors_shift = 0;


void rgbLedFadeInitialize(void) {
    pinMode(RGB1_BLUE,   OUTPUT);
    pinMode(RGB1_GREEN, OUTPUT);
    pinMode(RGB1_RED,  OUTPUT);

    pinMode(RGB1_COLOR_STEP_STATUS,  OUTPUT);
    pinMode(RGB1_NEW_COLOR_STATUS,  OUTPUT);

    randomSeed(analogRead(PIN_ANALOG_LIGHT));

    rgbLedFadeInitialized = true;
}

// FIXME: the returned values do not need to be float, but the variables I use
// are float for smooth transitions, so it carries on to this function for now.
void LED_Color_Picker(uint8_t led, float *blue, float *red, float *green) {
    if ((led == 1 && RGB1_init_color_changes > RGB1_INIT_COLOR_MAX) )
    {
	uint8_t led_color[3];
	float led[3] = { -1, -1, -1 };

	// Keep the first LED bright.
	led_color[0] = random(0, 64);
	// The lower the previous LED is, the brigher that color was
	// and the darker we try to make the second one.
	led_color[1] = constrain(random(0, 255 + 512 - led_color[0]), 0, 255);
	// Don't make the last LED too bright if the others already are (we
	// don't want white) or too dark if the others are too dark already.
	led_color[2] = constrain(
	    random( constrain( (64-(led_color[0] + led_color[1])/2), 0, 255), 
		    255 + 768 - (led_color[0] + led_color[1])), 
	    0,
	    255 );

	// Since color randomization is weighed towards the first color
	// we randomize color attribution between the 3 colors.
	uint8_t i = 0;
	while(i  < 3)
	{
	    uint8_t color_guess = random(0, 3);
	    // Loop until we get a color slot that's been unused.
	    if (led[color_guess] != -1.0) continue;
	    led[color_guess] = led_color[i++];
	}

	*blue = led[0];
	*red = led[1];
	*green = led[2];
    } else if (led == 1) {
	*blue  = RGB1_init_colors[RGB1_init_color_changes][0];
	*red   = RGB1_init_colors[RGB1_init_color_changes][1];
	*green = RGB1_init_colors[RGB1_init_color_changes][2];
	// The init color changes need to happen in 1sec each (10 steps of 
	// 100ms). After that, we slow down to longer transitions.
	if (RGB1_init_color_changes == RGB1_INIT_COLOR_MAX)
	{
	    Serial.print(millis());
	    Serial.print(": LED1 done with init, changing update speed from ");
	    Serial.print(RGB1_delay_factor);
	    Serial.print(" to ");
	    Serial.println(RGB1_RUN_DELAY_FACTOR);
	    RGB1_delay_factor = RGB1_RUN_DELAY_FACTOR;
	}
	else
	{
	    Serial.print(millis());
	    Serial.print(": LED1 init value #");
	    Serial.println(RGB1_init_color_changes);
	}
	RGB1_init_color_changes++;
    }
    // Fix things after the fact for RGB LEDs are common cathode (bright = 255)
    if (RGB_HIGH[led] == 255) {
	*blue = (255 - *blue) / (1 << rgb1_darker_colors_shift);
	*red = (255 - *red) / (1 << rgb1_darker_colors_shift);
	*green = (255 - *green) / (1 << rgb1_darker_colors_shift);
    }
    // change to what you want depending on what you'd like to debug
    // 255 for disabled.
    if (led == 1)
    {
	Serial.print(millis());
	Serial.print(": LED color picker for LED ");
	Serial.print(int(led));
	Serial.print(" returned: ");
	Serial.print(*blue);
	Serial.print(", ");
	Serial.print(*red);
	Serial.print(", ");
	Serial.println(*green);
    }
}


void rgbLedFadeHandler(void) {
    if (! rgbLedFadeInitialized) rgbLedFadeInitialize();

    RGB1_loop_count++;
    if (RGB1_loop_count <= RGB1_delay_factor) return;
    RGB1_loop_count = 1;

    // Show that we moved one step in the color change
    rgb1_color_step_state = !rgb1_color_step_state;
    digitalWrite(RGB1_COLOR_STEP_STATUS, rgb1_color_step_state);

    if (RGB1_led_stage == 0) {
	LED_Color_Picker(1, &RGB1b[0], &RGB1b[1], &RGB1b[2]);
	// Show that we moved one step in the color change
	rgb1_color_color_state = !rgb1_color_color_state;
	digitalWrite(RGB1_NEW_COLOR_STATUS, rgb1_color_color_state);
    }
    if (RGB1_led_stage < RGB1_COLOR_CHANGE_STEPS)
    {
	for (uint8_t i = 0; i <= 2; i++)
	{
	    if (RGB1_led_stage == 0)
	    {
		RGB1diff[i] = (RGB1b[i] - RGB1a[i]) / RGB1_COLOR_CHANGE_STEPS;
	    }
	    RGB1a[i] = constrain(RGB1a[i] + RGB1diff[i], 0, 255);
//   	    Serial.print("Setting LED ");
//   	    Serial.print(int(i));
//   	    Serial.print(" to ");
//   	    Serial.print(int(RGB1a[i]));
//   	    Serial.print(" at led stage ");
//   	    Serial.println(int(RGB1_led_stage));
 	    analogWrite(RGB1pins[i],  int(RGB1a[i]));
	}
	RGB1_led_stage++;
    }
    else if (RGB1_led_stage < RGB1_COLOR_CHANGE_STEPS + RGB1_COLOR_HOLD_STEPS)
    {
	if (RGB1_led_stage == RGB1_COLOR_CHANGE_STEPS) 
	{
	    // Serial.print(millis());
	    // Serial.println(": Holding Color for LED 1");
	}
	RGB1_led_stage++;
    }
    else
    {
	Serial.print(millis());
	Serial.println(": Finished Color for LED 1");
	RGB1_led_stage = 0;
    }
}


/* ------------------------------------------------------------------------- */
// vim:sts=4:sw=4:syntax=cpp
