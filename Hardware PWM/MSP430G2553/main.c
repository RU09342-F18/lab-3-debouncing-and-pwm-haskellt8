#include <msp430.h>

#define LED1 BIT0
#define BTN1 BIT3

void LEDSetup(void)
{
    P1DIR |= LED1;   // out
    P1OUT &= ~LED1;  // off
}
void ButtonSetup(void)
{
    P1DIR &= ~BTN1;  // in
    P1REN |= BTN1;   // enable resistor
    P1OUT |= BTN1;   // pull up

    P1IE |= BTN1;    // enable interrupt for button
    P1IES |= BTN1;   // set as falling edge
    P1IFG &= ~BTN1;  // clear interrupt flag
}
void TimerA0Setup(void){
    TA0CCTL0 = CCIE;                    // enable time interrupt
    TA0CCR0 = 500;                      // <.001 seconds
    TA0CTL = MC_0 | TASSEL_2 | ID_3;    // no count | SMCLK | 2^3 division
}

void TimerA1Setup(void){
    TA1CCR0 = 100-1;                   // Timer is set to .1 kHz
    TA1CCR1 = 50;                      // CCR1 value set to the PWM value
    TA1CCTL1 = OUTMOD_7;               // reset
    TA1CTL = MC_1 | TASSEL_2 | ID_0;   // up count | SMCLK | 2^0 division
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

    LEDSetup(); // Initialize our LEDS
    ButtonSetup();  // Initialize our button
    TimerA0Setup(); // Initialize Timer0
    TimerA1Setup(); // Initialize Timer1
    __bis_SR_register(GIE + LPM0_bits); //Low power mode enable global interrupt
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT_1(void)
{
    if((P1IN & BTN1) == 0) // if the button is pressed
    {
        P1OUT |= LED1;  // flip LED
        if(TA0CCR1 <= 90) // if not 100%
        {
            TA0CCR1 = TA0CCR1 + 10;
        }
        else
        {
            TA0CCR1 = 0;
        }
    }
    else
    {
        P1OUT &= ~LED1;  // flip LED
    }
    // disable button interrupt and wait in case button bounce
    P1IE &= ~BTN1;    // disable interrupt for button
    P1IFG &= ~BTN1;   // clear interrupt flag
    TA0CTL |= MC_3;   // count up down
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void TIMER_A0 (void)
{
    TA0CTL &= ~MC_3;  // stop timer
    if ((P1IN & BTN1) == 0) // if the button is pressed
    {
        P1IES &= ~BTN1;   // set as rising edge
    }
    else
    {
        P1IES |= BTN1;    // set as falling edge
        P1OUT &= ~LED1;   // flip LED in case too much bounce
    }
    P1IE |= BTN1;     // enable interrupt for button
}
