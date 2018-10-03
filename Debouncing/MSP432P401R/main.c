#include <msp432.h>

#define LED1 BIT0
#define BTN1 BIT1

void main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // Stop watchdog timer

    P1DIR |= LED1; // out
    P1DIR &= ~BTN1;  // in
    P1REN |= BTN1;   // enable resistor
    P1OUT |= BTN1;   // pull up
    P1SEL0 = 0;      // mux bit0 button
    P1SEL1 = 0;      // mux bit1 button

    P1IE |= BTN1;    // enable interrupt button
    P1IES |= BTN1;   // set as falling edge
    P1IFG &= ~BTN1;  // clear interrupt flag
    NVIC->ISER[1] = 1 << ((PORT1_IRQn) & 31);   //enable port 1 interrupt

    TA0CCTL0 = CCIE;                    // enable time interrupt
    TA0CCR0 = 18750;                    // 0.02 seconds
    TA0CTL = MC_0 | TASSEL_2 | ID_3;    // no count | SMCLK | 2^1 division
    NVIC->ISER[0] = 1 << ((TA0_0_IRQn) & 31);

    __enable_irq();                      // Enable global interrupt
    SCB->SCR |= SCB_SCR_SLEEPONEXIT_Msk; // Dont wake on ISR exit
    __DSB();                             // Sleep on exit immediately
    __sleep();                           // Sleep
}

//interrupt routine
void PORT1_IRQHandler(void)
{
    if((P1IN & BTN1) == 0) // if the button is pressed
    {
        P1OUT ^= LED1;  // flip LED
    }
    // disable button interrupt and wait in case button bounce
    P1IE &= ~BTN1;    // disable interrupt for button
    P1IFG &= ~BTN1;   // clear interrupt flag
    TA0CTL |= MC_3;   // count up down
}

void TA0_0_IRQHandler(void)
{
    TA0CTL &= ~MC_3;  // stop timer
    if ((P1IN & BTN1) == 0) // if the button is pressed
    {
        P1IES &= ~BTN1;   // set as rising edge
    }
    else
    {
        P1IES |= BTN1;    // set as falling edge
    }
    P1IE |= BTN1;     // enable interrupt for button
}
