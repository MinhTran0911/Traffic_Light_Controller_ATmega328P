/* EEET2505 Lab 2 Exercise 3 Version 2
 * Author: Minh Tran (s3818343) - Khoa Tran (s3847766)
 * Created date: Nov 23, 2021
 * Last Modified date: Dec 11, 2021
 */


#ifndef F_CPU
#define F_CPU 16000000UL
#endif


volatile int light_count = 0;
volatile bool S2_traffic = false;
volatile bool S4_traffic = false;


enum State {
  S1_S3,
  S2,
  S4
};


// Mapping LEDs to corresponding ports
enum PORTB_ASSIGNMENT {
  SEG_G = PORTB0,
  S4_RED = PORTB1,
  S4_LEFT = PORTB2,
  S1_GREEN = PORTB3,
  S2_GREEN = PORTB4,
  S1_RED = PORTB5
};


enum PORTC_ASSIGNMENT {
  S4_GREEN = PORTC0,
  S3_RED = PORTC1,
  S3_LEFT = PORTC2,
  S3_GREEN = PORTC3,
  S4_INDICATOR = PORTC4,
  S2_INDICATOR = PORTC5
};


enum PORTD_ASSIGNMENT {
  SEG_B = PORTD0,
  SEG_A = PORTD1,
  SEG_C = PORTD4,
  SEG_D = PORTD5,
  SEG_E = PORTD6,
  SEG_F = PORTD7
};


// Function declarations
void delay_second(int sec);
void display_count(int sec);


int main(void) {
  // All ports B, C, D0, D1, D4-D7 as outputs
  enum State light_state = S1_S3;  // Initial state
  DDRB = 0xFF;
  DDRC = 0xFF;
  DDRD = 0xFF;
  // Port D2 and D3 as inputs
  DDRD &= ~((1 << DDD2) | (1 << DDD3));
  
  // Enable ext. interrupt INT0 and INT1 with falling edge sensing
  EIMSK |= (1 << INT0) | (1 << INT1);
  EICRA |= (1 << ISC01) | (1 << ISC11);
  sei();  // Enable global interrupt bit
  
  // Clear interrupt flags
  EIFR |= (1 << INTF0) | (1 << INTF1);
  TIFR1 |= (1 << OCF1A);
  
  S2_traffic = false;
  S4_traffic = false;

  PORTC &= ~((1 << S2_INDICATOR) | (1 << S4_INDICATOR));
  display_count(-1);
  
  while (1) {                                                                                                                 
    switch (light_state) {
      case S1_S3:
        // Default state, when there's no traffic on S2 and S4, S1 and S3 stays green since not clashing
        // Turn on S1 & S3 lanes' green lights and S2 & S4 red lights
        // Other lights are off
        PORTB |= (1 << S1_GREEN) | (1 << S4_RED);
        PORTC |= (1 << S3_GREEN) | (1 << S3_LEFT);
        PORTB &= ~((1 << S1_RED) | (1 << S2_GREEN) | (1 << S4_LEFT));
        PORTC &= ~((1 << S3_RED) | (1 << S4_GREEN));

        if (S2_traffic || S4_traffic) {
          delay_second(3);
          if (S2_traffic) light_state = S2;
          else light_state = S4;
        }
        break;
      case S2:
        // Turn on S2 green light and S1 green light since S1 and S2 don't clash
        // S3 and S4 red lights are also on
        // S4 turn left light is on since not clashing
        // Other lights are off
        PORTB |= (1 << S1_GREEN) | (1 << S2_GREEN) | (1 << S4_RED) | (1 << S4_LEFT);
        PORTC |= (1 << S3_RED);
        PORTB &= ~(1 << S1_RED);
        PORTC &= ~((1 << S3_GREEN) | (1 << S3_LEFT) | (1 << S4_GREEN));
        delay_second(2);  // Time for this state is 2 seconds before moving back to S1_S3 state
        S2_traffic = false;
        PORTC &= ~(1 << S2_INDICATOR); // Turn off S2 traffic indicator when S2 is served 
        if (S4_traffic) light_state = S4;  // If there is traffic on S4, set next state to S4
        else light_state = S1_S3;
        break;
      case S4:
        // Turn on S4 green light and S1 S2 S3 red lights
        // S3 turn right light is on since not clashing with S4 lane
        // Other lights are off
        PORTB |= (1 << S1_RED) | (1 << S4_LEFT);
        PORTC |= (1 << S3_RED) | (1 << S3_LEFT) | (1 << S4_GREEN);
        PORTB &= ~((1 << S1_GREEN) | (1 << S2_GREEN) | (1 << S4_RED));
        PORTC &= ~(1 << S3_GREEN);
        delay_second(2);  // Time for this state is 2 seconds
        S4_traffic = false;
        PORTC &= ~(1 << S4_INDICATOR);  // Turn off S4 traffic indicator when S4 is served
        light_state = S1_S3;  // Set next state
        break;
      default:
        light_state = S1_S3; // Set default state to avoid undefined state
    }
  }
}


void delay_second(int sec) {
  /* This function delays the program by sec seconds
   * @param int sec: total number of second(s) to delay 
   * @return: None
   */
  // Timer 1 CTC Mode and Prescaler 256
  TCCR1B |= (1 << WGM12) | (1 << CS12);
  OCR1A = 62499; // Timer count for 1 second w/ prescaler 256
  TCNT1 = 0;  // Reset Timer/Counter 1
  
  light_count = sec;  // Set initial second to count down from
  display_count(light_count);  // Display current second count on 7 seg LEDs
  TIMSK1 |= (1 << OCIE1A); // Turn on Timer 1 Output Compare interrupt
  while (light_count >= 0) {}  // While current second count has not reached 0, keep system idle
  
  light_count = 0;  // Reset current second counter
  TIMSK1 &= ~(1 << OCIE1A);  // Turn off Timer 1 Output Compare interrupt
  TCCR1B &= ~((1 << CS12) | (1 << CS11) | (1 << CS10));  // Turn off Timer 1
  TCNT1 = 0;  // Reset Timer/Counter 1
  return; // When sec seconds has elapsed, return to main program
}


void display_count(int sec) {
  /* This function display the current count down number using seven-seg LEDs
   * @param int sec: current second
   * @return: None
   */
  // 7-seg LED is common anode
  switch(sec) {
    case 0:
      PORTD &= ~((1 << SEG_F) | (1 << SEG_E) | (1 << SEG_D) | (1 << SEG_C) | (1 << SEG_B) | (1 << SEG_A));
      PORTB |= (1 << SEG_G);
      break; 
    case 1:
      PORTD &= ~((1 << SEG_B) | (1 << SEG_C));
      PORTB |= (1 << SEG_G);
      PORTD |= (1 << SEG_F) | (1 << SEG_E) | (1 << SEG_D) | (1 << SEG_A);
      break;
    case 2:
      PORTB &= ~(1 << SEG_G);
      PORTD &= ~((1 << SEG_E) | (1 << SEG_D) | (1 << SEG_B) | (1 << SEG_A));
      PORTD |= (1 << SEG_F) | (1 << SEG_C);
      break;
    case 3:
      PORTB &= ~(1 << SEG_G);
      PORTD &= ~((1 << SEG_D) | (1 << SEG_C) | (1 << SEG_B) | (1 << SEG_A));
      PORTD |= (1 << SEG_F) | (1 << SEG_E);
      break;
    case 4:
      PORTB &= ~(1 << SEG_G);
      PORTD &= ~((1 << SEG_F) | (1 << SEG_C) | (1 << SEG_B));
      PORTD |= (1 << SEG_E) | (1 << SEG_D) | (1 << SEG_A);
      break;
    case 5:
      PORTB &= ~(1 << SEG_G);
      PORTD &= ~((1 << SEG_F) | (1 << SEG_D) | (1 << SEG_C) | (1 << SEG_A));
      PORTD |= (1 << SEG_E) | (1 << SEG_B);
      break;
    case 6:
      PORTB &= ~(1 << SEG_G);
      PORTD &= ~((1 << SEG_F) | (1 << SEG_E) | (1 << SEG_D) | (1 << SEG_C) | (1 << SEG_A));
      PORTD |= (1 << SEG_B);
      break;
    case 7:
      PORTD &= ~((1 << SEG_C) | (1 << SEG_B) | (1 << SEG_A));
      PORTB |= (1 << SEG_G);
      PORTD |= (1 << SEG_F) | (1 << SEG_E) | (1 << SEG_D);
      break;
    case 8:
      PORTB &= ~(1 << SEG_G);
      PORTD &= ~((1 << SEG_F) | (1 << SEG_E) | (1 << SEG_D) | (1 << SEG_C) | (1 << SEG_B) | (1 << SEG_A));
      break;
    case 9:
      PORTB &= ~(1 << SEG_G);
      PORTD &= ~((1 << SEG_F) | (1 << SEG_D) | (1 << SEG_C) | (1 << SEG_B) | (1 << SEG_A));
      PORTD |= (1 << SEG_E);
      break;
    default:
      // All LEDs off
      PORTB |= (1 << SEG_G);
      PORTD |= (1 << SEG_F) | (1 << SEG_E) | (1 << SEG_D) | (1 << SEG_C) | (1 << SEG_B) | (1 << SEG_A);
  }
}


ISR (TIMER1_COMPA_vect) {
  // Every time the timer counts 1 second, decrement the delay time by 1
  light_count -= 1; 
  display_count(light_count);  // Display current second count on 7 seg LEDs
}    


ISR (INT0_vect) {
  sei();
  S2_traffic = true;
  // Turn on LED indicating traffic on lane S2
  PORTC |= (1 << S2_INDICATOR);
}


ISR (INT1_vect) {
  sei();
  S4_traffic = true;
  // Turn on LED indicating traffic on lane S4
  PORTC |= (1 << S4_INDICATOR);
}
