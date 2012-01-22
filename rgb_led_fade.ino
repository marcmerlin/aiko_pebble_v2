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

// How many times we entered the loop.
uint8_t RGB1_loop_count = 0;

//                       Blue,Green,Red
// Current Color
float RGB1a[3] =        { 50, 50, 50 };
// Target Color
float RGB1b[3];
// What to add to RGB1a to get to RGB1b in 64 steps
float RGB1diff[3];
uint8_t RGB1pins[3] =     { RGB1_BLUE, RGB1_GREEN, RGB1_RED };


// Show how quickly the color steps are changed.
#define RGB1_COLOR_STEP_STATUS  PIN_OUTPUT_1
uint8_t rgb1_color_step_state = 0;
// Toggle when a new color is generated.
#define RGB1_NEW_COLOR_STATUS   PIN_OUTPUT_2
uint8_t rgb1_color_color_state = 0;


void rgbLedFadeInitialize(void) {
    pinMode(RGB1_BLUE,   OUTPUT);
    pinMode(RGB1_GREEN, OUTPUT);
    pinMode(RGB1_RED,  OUTPUT);

    pinMode(RGB1_COLOR_STEP_STATUS,  OUTPUT);
    pinMode(RGB1_NEW_COLOR_STATUS,  OUTPUT);

    randomSeed(analogRead(PIN_ANALOG_LIGHT));

    // White
    analogWrite(RGB1_GREEN, RGB1_HIGH);
    analogWrite(RGB1_RED, RGB1_HIGH);
    analogWrite(RGB1_BLUE, RGB1_HIGH);
    delay(100);

    // Blue
    analogWrite(RGB1_GREEN, RGB1_LOW);
    analogWrite(RGB1_RED, RGB1_LOW);
    delay(100);

    // Green
    analogWrite(RGB1_GREEN, RGB1_HIGH);
    analogWrite(RGB1_BLUE, RGB1_LOW);
    delay(100);

    // Red
    analogWrite(RGB1_RED, RGB1_HIGH);
    analogWrite(RGB1_GREEN, RGB1_LOW);
    delay(100);

    // Off
    analogWrite(RGB1_RED, RGB1_LOW);
    delay(100);

    rgbLedFadeInitialized = true;
}

// FIXME: the returned values do not need to be float, but the variables I use
// are float for smooth transitions, so it carries on to this function for now.
void LED_Color_Picker(char *caller, float *blue, float *red, float *green) {
    uint8_t led_color[3];
    float led[3] = { -1, -1, -1 };

    // This code was designed for 0 = bright, 255 = dark
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

    // Horrid patch to fix things after the fact for RGB LEDs where
    // bright = 255
    if (RGB1_HIGH == 255) {

	*blue = 255 - led[0];
	*red = 255 - led[1];
	*green = 255 - led[2];
    } else { 
	*blue = led[0];
	*red = led[1];
	*green = led[2];
    }
    if (caller != NULL)
    {
	Serial.print("LED color picker for ");
	Serial.print(caller);
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
    RGB1_loop_count = 0;

    // Show that we moved one step in the color change
    rgb1_color_step_state = !rgb1_color_step_state;
    digitalWrite(RGB1_COLOR_STEP_STATUS, rgb1_color_step_state);

    if (RGB1_led_stage == 0) {
	LED_Color_Picker(NULL, &RGB1b[0], &RGB1b[1], &RGB1b[2]);
	// Show that we moved one step in the color change
	rgb1_color_color_state = !rgb1_color_color_state;
	digitalWrite(RGB1_NEW_COLOR_STATUS, rgb1_color_color_state);
    }
    if (RGB1_led_stage <= 64)
    {
	for (uint8_t i = 0; i <= 2; i++)
	{
	    if (RGB1_led_stage == 0)
	    {
		RGB1diff[i] = (RGB1b[i] - RGB1a[i]) / 64;
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
    else if (RGB1_led_stage > 64 and RGB1_led_stage < 70)
    {
// 	Serial.print("Idle lcd stage ");
// 	Serial.println(int(RGB1_led_stage));
	RGB1_led_stage++;
    }
    else if (RGB1_led_stage == 70)
    {
	RGB1_led_stage = 0;
    }
}


/* ------------------------------------------------------------------------- */
// vim:sts=4:sw=4:syntax=cpp
