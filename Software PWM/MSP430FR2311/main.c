#include <msp430.h>

#define LED1 BIT0
#define LED2 BIT0
#define BTN1 BIT1

int duty = 5; // 50%
int dutyCount = 0;

void LEDSetup(void)
{
    P1DIR |= LED1;   // out
    P1OUT &= ~LED1;  // off
    P2DIR |= LED2;   // out
    P2OUT &= ~LED2;  // off
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
void TimerB0Setup(void){
    TB0CCTL0 = CCIE;                    // enable time interrupt
    TB0CCR0 = 500;                      // >.001 seconds
    TB0CTL = MC_0 | TBSSEL_2 | ID_3;    // no count | SMCLK | 2^1 division
}

void TimerB1Setup(void){
    TB1CCTL0 = CCIE;                    // enable time interrupt
    TB1CCR0 = 100;                      // really fast seconds
    TB1CTL = MC_1 | TBSSEL_2 | ID_0;    // up | SMCLK | no division
}

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;
    P1OUT |= LED1;  // flip LED
    LEDSetup(); // Initialize our LEDS
    ButtonSetup();  // Initialize our button
    TimerB0Setup(); // Initialize Timer0
    TimerB1Setup(); // Initialize Timer1
    __bis_SR_register(GIE + LPM0_bits); //Low power mode enable global interrupt
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT_1(void)
{
    if((P1IN & BTN1) == 0) // if the button is pressed
    {
        P1OUT |= LED1;  // flip LED
        if(duty <= 9)
        {
            duty = duty + 1;
        }
        else
        {
            duty = 0;
        }
    }
    else
    {
        P1OUT &= ~LED1;  // flip LED
    }
    // disable button interrupt and wait in case button bounce
    P1IE &= ~BTN1;    // disable interrupt for button
    P1IFG &= ~BTN1;   // clear interrupt flag
    TB0CTL |= MC_3;   // count up down
}

#pragma vector=TIMER0_B0_VECTOR
__interrupt void TIMER_B0 (void)
{
    TB0CTL &= ~MC_3;  // stop timer
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

#pragma vector= TIMER1_B0_VECTOR
__interrupt void Timer_B1 (void)
{
    if(dutyCount < duty)
    {
        P2OUT |= LED2;  //on LED2
    }
    else
    {
        P2OUT &= ~LED2;  //off LED2
    }
    if(dutyCount < 10)
    {
        dutyCount = dutyCount + 1;
    }
    else
    {
        dutyCount = 0;
    }
}

