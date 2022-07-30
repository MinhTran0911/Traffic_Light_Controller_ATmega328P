/* EEET2505 Lab 2 Exercise 3 Version 1
 * Author: Minh Tran (s3818343) - Khoa Tran (s3847766)
 * Created date: Nov 19, 2021
 * Last Modified date: Dec 9, 2021
 */

#include <avr/io.h>
#include <avr/interrupt.h>

#ifndef F_CPU
#define F_CPU 16000000UL
#endif


// This var keeps track of the current elapsed second
volatile int light_count = 0;


enum PortB_Assignment {
  N_GREEN = PORTB0,
  N_RED = PORTB1,
  E_RED = PORTB2,
  E_GREEN = PORTB3,
  S_GREEN = PORTB4,
  S_RED = PORTB5
};


enum PortC_Assignment {
  W_RED = PORTC0,
  W_GREEN = PORTC1
};


enum State {
  NORTH_SOUTH,
  EAST_WEST
};


// Function declarations
void delay_second(int sec);
void display_count(int sec);


int main(void) {
  // All ports B, C, D as outputs, initialize state
  enum State light_state = NORTH_SOUTH;
  DDRB = 0xFF;
  DDRC = 0xFF;
  DDRD = 0xFF;
  
  // Timer 1 CTC Mode and Prescaler 256
  TCCR1B |= (1 << WGM12) | (1 << CS12);
  OCR1A = 62499; // Timer count for 1 second
  sei();  // Enable global interrupt bit
  
  while (1) {
    switch (light_state) {
      case NORTH_SOUTH:
        // Turn on North & South Green lights and West & East Red lights
        // Other lights are turned off
        PORTB |= (1 << N_GREEN) | (1 << S_GREEN) | (1 << E_RED);
        PORTB &= ~((1 << N_RED) | (1 << S_RED) | (1 << E_GREEN));
        PORTC |= (1 << W_RED);
        PORTC &= ~(1 << W_GREEN);
        light_state = EAST_WEST;  // Set next state
        break;
      case EAST_WEST:
        // Turn on North & South Red lights and West & East Green lights
        // Other lights are turned off
        PORTB |= (1 << N_RED) | (1 << S_RED) | (1 << E_GREEN);
        PORTB &= ~((1 << N_GREEN) | (1 << S_GREEN) | (1 << E_RED));
        PORTC |= (1 << W_GREEN);
        PORTC &= ~(1 << W_RED);
        light_state = NORTH_SOUTH;  // Set next state
        break;
    }
    // Each state stays on for 3 secs
    delay_second(3);
  }
}


void delay_second(int sec) {
  /* This function delays the program by sec seconds
   * @param int sec: total number of second(s) to delay 
   * @return: None
   */
  light_count = sec;  // Set initial seconds to count down from
  display_count(light_count);  // Display the current second count on 7 seg LEDs
  TIMSK1 |= (1 << OCIE1A);  // Turn on Timer 1 Compare Match interrupt
  while (light_count >= 0) {}  // When current second count has not reached 0, keep system idle
  light_count = 0;  // Reset second count  
  TIMSK1 &= ~(1 << OCIE1A);  // Turn off Timer 1 Compare Match interrupt
  return; // When sec seconds has elapsed, return to main program
}


void display_count(int sec) {
  /* This function display the current count down number using seven-seg LEDs
   * @param int sec: current second
   * @return: None
   */
  // Seven Seg used is common anode 
  /* Array to store number patterns for 7 seg LEDs
   *  NUM[0] = Pattern number "0"
   *  NUM[1] = Pattern number "1"
   *  ...
   *  NUM[9] = Pattern number "9"
   */
  const int NUM[] = {0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8, 0x80, 0x90};
  PORTD = NUM[sec];  // Set Port D output to display the corresponding second on 7 seg LEDs
}


ISR (TIMER1_COMPA_vect) {
  // Every time the timer counts 1 second, decrement the delay time by 1
  light_count -= 1; 
  display_count(light_count);  // Display the current second count
}
