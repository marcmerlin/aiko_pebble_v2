/* rotary_encoder.ino
 * ~~~~~~~~~~~~~~~~~~
 * Please do not remove the following notices.
 * License: GPLv3. http://geekscape.org/static/arduino_license.html
 * ----------------------------------------------------------------------------
 * See http://www.circuitsathome.com/mcu/reading-rotary-encoder-on-arduino
 *
 * A (relatively) simple sketch for using PCint for interrupt on encoder
 * rotation on the pebble v2.
 * Brett Downing 2012
 * 
 * One thing you'll notice about the pebble board is the enormous number of
 * encoder errrors it generates even though position measurement is rock solid.
 * (it returns to zero at the same angle) The reason for this is contact bounce.
 * This sketch, run on high quality optical encoder modules ($30 from sparkfun)
 * results in almost zero errors.
 * 
 * The contact bounce is a very fast signal, much faster than the encoder
 * can actually rotate by two quadrants.  With the interrupt running as
 * fast as it does, the gray code signal is almost entirely bounce immune.
 * 
 * ----------------------------------------------------------------------------
 * Adapted by Marc MERLIN <marc_soft@merlins.org> for pebble v2 and aiko.
 * 
 * - clockwise is +1, not -1
 * - merged code in one file, cleaned up.
 * 
 * Note that each rotation is really actually 2 clicks, so code using this
 * should likely count 2 clicks as one rotation step.
 */

#define PORTD_ENCODER_A_MASK (1<<PIN_ROTARY_A)
#define PORTD_ENCODER_B_MASK (1<<PIN_ROTARY_B)
#define PORTD_ENCODERS_MASK (PORTD_ENCODER_B_MASK | PORTD_ENCODER_A_MASK)

// D8 is bit 0 on port B.
#define PORTB_BUTTON_MASK (1<< (PIN_ROTARY_PUSH-8))

byte PINB_input = 0x00;  // states read immediately at interrupt
byte PINB_old_states = 0x00;  // the states at last interrupt
byte PINB_bits_changed = 0x00;  // XOR of the old and new states
boolean PortBInterruptFired = false;

byte PIND_input = 0x00;  // states read immediately at interrupt
byte PIND_old_states = 0x00;  // the states at last interrupt
byte PIND_bits_changed = 0x00;  // XOR of the old and new states
boolean PortDInterruptFired = false;

int8_t  rotary_button_change = 0;   // 0 if read, or -1/+1 if pending processing
int16_t encoderErrors = 0;  // when 2 pins change at once (undefined direction)
boolean button_status, old_button_status = 1;


boolean rotaryEncoderInitialized = false;

/* For interrupts info, read
 * http://forums.trossenrobotics.com/tutorials/how-to-diy-128/an-introduction-to-interrupts-3248/
 *
 * Pin to interrupt map, see http://arduino.cc/playground/Main/PinChangeInt
 * D0-D7           = PCINT 16-23 = PCIR2 = Port D = PCIE2 = pcmsk2
 * D8-D13          = PCINT 0-5   = PCIR0 = Port B = PCIE0 = pcmsk0
 * A0-A5 (D14-D19) = PCINT 8-13  = PCIR1 = Port C = PCIE1 = pcmsk1
 */

#define BUTTON_PUSHED 0
#define BUTTON_NOTPUSHED 1

// Interrupt Service Routine for PORTB. D8 is PCINT0
ISR(PCINT0_vect) {
    PINB_input = PINB;  // This macro/function returns pin states for port D
    PINB_bits_changed = PINB_input ^ PINB_old_states; 

    // Make sure that a change received was actually on the pins we care about
    // and not other ones on that bank:
    if (PINB_bits_changed & PORTB_BUTTON_MASK) 
    {
	button_status = PINB_input & PORTB_BUTTON_MASK;
    }
    PINB_old_states = PINB_input;
    PortBInterruptFired = true;
}

// Interrupt Service Routine for PORTD. D6 and D7 are PCINT22 and PCINT23
ISR(PCINT2_vect) {
    PIND_input = PIND;  // This macro/function returns pin states for port D
    PIND_bits_changed = PIND_input ^ PIND_old_states; 

    // Make sure that a change received was actually on the pins we care about
    // and not other ones on that bank:
    if (PIND_bits_changed & PORTD_ENCODERS_MASK) 
    {
	PortDInterruptFired = true;  //the encoders did something

	// Did both pins change at once? If so, that's an error condition: 
	// This is a small error if the interrupt is fast.
	if ((PIND_bits_changed & PORTD_ENCODERS_MASK) == PORTD_ENCODERS_MASK) {  
	    encoderErrors++;
	} 
        // The next condition simply resolves a direction. 
	// The encoder DID move sensibly, but which way?
	else 
	{
          /* This code code relies on the two pins being on the same port.
          I'm pretty sure it's the minimum number of cycles required to fully
          decode quadrature down to the quadrant level.  I stripped it out of
          another project where speed was an issue, so I apologise for the
          difficult to read line. */
	  if (( ( (PIND_input >> 1) ^ PIND_input ^ PIND_bits_changed) & 
		PORTD_ENCODER_A_MASK) != 0x00) {
	      rotary_button_change = -1;
	  } else {
	      rotary_button_change = 1;
	  }
	}
    }
    PIND_old_states = PIND_input;
    PortDInterruptFired = true;
}

void rotaryEncoderInitialize(void) {
    pinMode(PIN_ROTARY_A,    INPUT);
    pinMode(PIN_ROTARY_B,    INPUT);
    pinMode(PIN_ROTARY_PUSH, INPUT);

    // Pin change interrupt control register - enables interrupt vectors
    // Bit 2 = enable PC vector 2 (PCINT23..16)
    // Bit 1 = enable PC vector 1 (PCINT14..8)
    // Bit 0 = enable PC vector 0 (PCINT7..0)
    
    // The values below are correct for pebble v2
    // see "/usr/lib/avr/include/avr/iom328p.h" for aliases for other pins etc.
    PCICR  |= (1 << PCIE0);   // enable port-change int on PORTB
    PCICR  |= (1 << PCIE2);   // enable port-change int on PORTD

    // Pin change mask registers decide which pins are enabled as triggers
    PCMSK0 |= (1 << PCINT0);  // port change interrupt 0  for pin 8
    PCMSK2 |= (1 << PCINT22); // port change interrupt 22 for pin 6
    PCMSK2 |= (1 << PCINT23); // port change interrupt 23 for pin 7

    rotaryEncoderInitialized = true;
}

void rotaryEncoderHandler(void) {
    static char clickchar = char(0);
    if (! rotaryEncoderInitialized) rotaryEncoderInitialize();

    if (button_status != old_button_status)
    {
	if (button_status == BUTTON_NOTPUSHED && old_button_status == BUTTON_PUSHED)
	{
	    button_clicked = true;
	    // Little hack to cycle the <heart> character.
	    // lcd.setCursor(1, 0);
	    // lcd.print(clickchar);
	    // clickchar = (clickchar + 1) % 5;
	}
	old_button_status = button_status;
    }

    if (PortDInterruptFired == true)
    {
	PortDInterruptFired = false;    

	Serial.print("Encoder Input: ");
	// There is a race condition here. This value is consumed by lcd.ino
	// and reset to '0'. This could happen quicker than the print below.
	Serial.println(rotary_button_change);
	Serial.print("Encoder Errors: ");
	Serial.println(encoderErrors);
    }
}



/* ------------------------------------------------------------------------- */
