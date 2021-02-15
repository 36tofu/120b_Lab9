/*	Author: lab
 *  Partner(s) Name: 
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#endif

unsigned long _avr_timer_M = 1; //start count from here, down to 0. Dft 1ms
unsigned long _avr_timer_cntcurr = 0; //Current internal count of 1ms ticks


void TimerOn(){
	//AVR timer/counter controller register TCCR1
	TCCR1B = 0x0B; //bit 3 = 0: CTC mode (clear timer on compare)
	//AVR output compare register OCR1A
	OCR1A = 125; // Timer interrupt will be generated when TCNT1 == OCR1A
	//AVR timer interrupt mask register
	TIMSK1 = 0x02; //bit1: OCIE1A -- enables compare match interrupt
	//Init avr counter
	TCNT1 = 0;

	_avr_timer_cntcurr = _avr_timer_M;
	//TimerISR will be called every _avr_timer_cntcurr ms
	
	//Enable global interrupts 
	SREG |= 0x80; //0x80: 1000000

}

void TimerOff(){
	TCCR1B = 0x00; //bit3bit1bit0 = 000: timer off
}


ISR(TIMER1_COMPA_vect){
	_avr_timer_cntcurr--;
	if (_avr_timer_cntcurr == 0) {
			TimerISR();
			_avr_timer_cntcurr = _avr_timer_M;
			}
}

void TimerSet(unsigned long M) {
	_avr_timer_M = M;
	_avr_timer_cntcurr = _avr_timer_M;
}

unsigned char tmpDT1;
unsigned char tmpDT2;


typedef struct task {
  int state; // Current state of the task
  unsigned long period; // Rate at which the task should tick
  unsigned long elapsedTime; // Time since task's previous tick
  int (*TickFct)(int); // Function to call for task's tick
} task;

task tasks[3];

const unsigned char tasksNum = 3;
const unsigned long tasksPeriodGCD = 1;
const unsigned long periodBlinkLED = 1000;
const unsigned long periodThreeLEDs = 300;
const unsigned long periodCombined = 1;

enum BL_States { BL_SMStart, BL_s1 };
int TickFct_BlinkLED(int state);

int TickFct_ThreeLEDs(int state);
enum TL_States { TL_SMStart, TL_s1, TL_s2, TL_s3 };

enum combined_States { C_SMStart, C_s1 };
int TickFct_Combined(int state);

void TimerISR() {
  unsigned char i;
  for (i = 0; i < tasksNum; ++i) { // Heart of the scheduler code
     if ( tasks[i].elapsedTime >= tasks[i].period ) { // Ready
        tasks[i].state = tasks[i].TickFct(tasks[i].state);
        tasks[i].elapsedTime = 0;
     }
     tasks[i].elapsedTime += tasksPeriodGCD;
  }
}

int main() {

  DDRD = 0x0F;
  PORTD = 0x00;
  unsigned char i=0;
  tasks[i].state = BL_SMStart;
  tasks[i].period = periodBlinkLED;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_BlinkLED;
  ++i;
  tasks[i].state = TL_SMStart;
  tasks[i].period = periodThreeLEDs;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_ThreeLEDs;
  ++i;
  tasks[i].state = C_SMStart;
  tasks[i].period = periodCombined;
  tasks[i].elapsedTime = tasks[i].period;
  tasks[i].TickFct = &TickFct_Combined;
  TimerSet(tasksPeriodGCD);
  TimerOn();
  
  
  while(1) {
     //Sleep();
  }
  return 0;
}

int TickFct_BlinkLED(int state) {
  switch(state) { // Transitions
     case BL_SMStart: // Initial transition
        tmpDT1 = 0; // Initialization behavior
        state = BL_s1;
        break;
     case BL_s1:
        state = BL_s1;
        break;
     default:
        state = BL_SMStart;
   } // Transitions

  switch(state) { // State actions
     case BL_s1:
        tmpDT1 ^= 0x08;
        break;
     default:
        break;
  } // State actions
  //PORTD = tmpDT1;
  return state;
}

int TickFct_ThreeLEDs(int state) {
  switch(state) { // Transitions
     case TL_SMStart: // Initial transition
        state = TL_s1;
        break;
     case TL_s1:
        state = TL_s2;
        break;
     case TL_s2:
        state = TL_s3;
        break;
     case TL_s3:
        state = TL_s1;
        break;
     default:
        state = TL_SMStart;
   } // Transitions

  switch(state) { // State actions
     case TL_s1:
	tmpDT2 = 0x01;
        break;
     case TL_s2:
	tmpDT2 = 0x02;
        break;
     case TL_s3:
	tmpDT2 = 0x04;
        break;
     default:
        break;
  } // State actions
  //PORTD = tmpDT2;
  return state;
}

int TickFct_Combined(int state) {
  switch(state) { // Transitions
     case C_SMStart: // Initial transition
        state = C_s1;
        break;
     case C_s1:
        state = C_s1;
        break;
     default:
        state = C_SMStart;
   } // Transitions

  switch(state) { // State actions
     case C_SMStart: // Initial transition
     	PORTD = tmpDT1 | tmpDT2; 
        break;
     case C_s1:
     	PORTD = tmpDT1 | tmpDT2; 
        break;
     default:
     	PORTD = tmpDT1 | tmpDT2; 
        break;
  } // State actions
  return state;
}
