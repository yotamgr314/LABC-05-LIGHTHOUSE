#include "System/system.h"     // System-level initialization and utility functions
#include "oledDriver/oledC.h"   // OLED display driver functions
#include <xc.h>                 // Microcontroller-specific header file
#include <stddef.h>             // For the definition of NULL

// Global variable to track the toggle state of the display (0 = BLACK, 1 = WHITE)
static volatile uint8_t toggle_flag = 0;

// Global variable to hold the current background color of the OLED screen
static uint16_t current_background_color;

// Function to initialize Timer 1
void InitializeTimer1(void)
{
    // Disable Timer 1 during configuration
    T1CONbits.TON = 0;

    // Continue operation in Idle mode (Timer 1 keeps running when the CPU enters Idle mode)
    T1CONbits.TSIDL = 0;

    // Disable gated time accumulation
    T1CONbits.TGATE = 0;

    // Set prescaler to 1:256 (reduces the timer input clock frequency to Fcy/256)
    T1CONbits.TCKPS = 3;

    // Select the internal clock source (Fosc/2) as the timer clock
    T1CONbits.TCS = 0;

    // Set the period register to 15625 for a 1-second interval
    // Fcy = 4 MHz / 256 = 15625 Hz (1 second interval = 15625 cycles)
    PR1 = 15625;

    // Clear Timer 1 count register
    TMR1 = 0;

    // Clear Timer 1 interrupt flag (to ensure no pending interrupts)
    IFS0bits.T1IF = 0;

    // Enable Timer 1 interrupt (allows Timer 1 to trigger the interrupt service routine)
    IEC0bits.T1IE = 1;

    // Set Timer 1 interrupt priority to 4 (medium priority)
    IPC0bits.T1IP = 4;

    // Enable Timer 1 (start counting)
    T1CONbits.TON = 1;
}

// Function to clear the OLED screen with the current background color
static void oledC_clearScreen(void)
{
    uint8_t column;  // Variable to track the column address
    uint8_t row;     // Variable to track the row address

    // Set the column address range from 0 to 96 (the width of the OLED screen)
    oledC_setColumnAddressBounds(0, 96);

    // Set the row address range from 0 to 96 (the height of the OLED screen)
    oledC_setRowAddressBounds(0, 96);

    // Loop through all columns and rows to fill the screen with the current background color
    for (column = 0; column < 96; column++)
    {
        for (row = 0; row < 96; row++)
        {
            // Send the current background color to the OLED screen
            oledC_sendColorInt(current_background_color);
        }
    }
}

// Interrupt Service Routine (ISR) for Timer 1
// This function is triggered automatically when Timer 1 overflows (every 1 second)
void __attribute__((__interrupt__, auto_psv)) _T1Interrupt(void)
{
    // Toggle the OLED display between WHITE and BLACK
    if (toggle_flag)
    {
        // Send a command to turn the OLED display to WHITE
        oledC_sendCommand(OLEDC_CMD_SET_DISPLAY_MODE_ON, NULL, 0);
    }
    else
    {
        // Send a command to turn the OLED display to BLACK (inverse mode)
        oledC_sendCommand(OLEDC_CMD_SET_DISPLAY_MODE_INVERSE, NULL, 0);
    }

    // Toggle the flag for the next interrupt (switch between WHITE and BLACK)
    toggle_flag = !toggle_flag;

    // Clear the Timer 1 interrupt flag to rearm the interrupt
    IFS0bits.T1IF = 0;
}

// Function to initialize user-defined settings (such as clearing the screen and initializing Timer 1)
void User_Initialize(void)
{
    // Clear the OLED screen with the current background color
    oledC_clearScreen();

    // Initialize Timer 1 to generate 1-second interrupts
    InitializeTimer1();
}

int main(void)
{
    // Call the system-level initialization function (sets up clocks, ports, etc.)
    SYSTEM_Initialize();

    // Call the user-defined initialization function (clears the screen and initializes Timer 1)
    User_Initialize();

    // Enter an infinite loop (the program will stay here forever, letting the ISR handle the display toggling)
    while (1)
    {
        // The Timer 1 interrupt service routine will handle the lighthouse toggling
    }

    return 0;
}
