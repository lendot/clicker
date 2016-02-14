#include <avr/sleep.h>
#include <avr/interrupt.h>

#define VIBRATE 3 // Vibrate pin number
#define SWITCH 4 // Increment button pin number
#define SWITCH_INTERRUPT_PIN PCINT4 // ATtiny85 interrupt pin for switch
#define IDLE_TIMEOUT 10000 // how long to stay idle before timing out, in ms
#define HOLD_TIMEOUT 1500 // how long for button long press before counter increments

int last_button_state = LOW;
long idle_end = 0;
long hold_end = 0;
int counter = 0;

void setup() {
  pinMode(SWITCH,INPUT);
  pinMode(VIBRATE,OUTPUT);
  digitalWrite(VIBRATE,LOW);
}

void increment()
{
  int i;
  counter++;
  for (i=0;i<3;i++) {
    digitalWrite(VIBRATE,HIGH);
    delay(300);  
    digitalWrite(VIBRATE,LOW);
    delay(150);
  }
}

void sleep()
{
    GIMSK |= _BV(PCIE);                     // Enable Pin Change Interrupts
    PCMSK |= _BV(SWITCH_INTERRUPT_PIN);     // Pin to use as interrupt pin
    ADCSRA &= ~_BV(ADEN);                   // ADC off
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);    // replaces above statement

    sleep_enable();                         // Sets the Sleep Enable bit in the MCUCR Register (SE BIT)
    sei();                                  // Enable interrupts
    sleep_cpu();                            // sleep

    cli();                                  // Disable interrupts
    PCMSK &= ~_BV(SWITCH_INTERRUPT_PIN);    // Turn off interrupt pin setting
    sleep_disable();                        // Clear SE bit
    ADCSRA |= _BV(ADEN);                    // ADC on

    sei();                                  // Enable interrupts
}

void loop() {
  int button_state=digitalRead(SWITCH);
  if (button_state != last_button_state) {
     if (button_state == LOW) {
      hold_end = 0;
      idle_end = millis() + IDLE_TIMEOUT;
     }
     else {
      idle_end = 0;
      hold_end = millis() + HOLD_TIMEOUT;
     }
     last_button_state = button_state;
  }
  if (button_state == LOW) {
    if (idle_end && millis() >= idle_end) {
      // we've been idle long enough to go to sleep
      idle_end = 0;
      sleep();
    }
  }
  else {
    if (hold_end && millis() >= hold_end) {
      // button's been held down long enough to increment
      hold_end = 0;
      increment();
    }
  }
}

ISR(PCINT0_vect)
{
  // ISR for button press
}

