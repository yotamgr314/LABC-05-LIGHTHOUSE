/*
 * File:   LightHouse.c
 * Author: Bader Daka & Noor Haj Dawood
 * Created on December 30, 2021, 11:00 AM
 */

#include "System/system.h"
#include "System/delay.h"
#include "oledDriver/oledC.h"
#include "oledDriver/oledC_colors.h"
#include "oledDriver/oledC_shapes.h"
#include <xc.h>
#include <p24FJ256GA705.h>

static uint16_t current_background_color;

// Clear OLED screen function
static void oledC_clearScreen(void)
{
    uint8_t column;
    uint8_t row;

    oledC_setColumnAddressBounds(0, 96);
    oledC_setRowAddressBounds(0, 96);

    for (column = 0; column < 96; column++)
    {
        for (row = 0; row < 96; row++)
        {
            oledC_sendColorInt(current_background_color);
        }
    }
}

// Set OLED background color
static void oledC_setBackground(uint16_t color)
{
    if (current_background_color != color)
    {
        current_background_color = color;
        oledC_clearScreen();
    }
}

// User initialization function
void user_init()
{
    oledC_setBackground(OLEDC_COLOR_BLACK);

    // Timer 1 configuration
    T1CONbits.TON = 1;
    T1CONbits.TSIDL = 1;
    T1CONbits.TCS = 0;
    T1CONbits.TCKPS = 0b10; // Prescaler 1:64
    PR1 = 62499;            // 1-second interval

    // Interrupt configuration
    IFS0bits.T1IF = 0;     // Clear Timer 1 interrupt flag
    IPC0bits.T1IP = 0b001; // Set priority to 1
    IEC0bits.T1IE = 1;     // Enable Timer 1 interrupt
    INTCON2bits.GIE = 1;   // Enable global interrupts
}

char lighthouseFlag = 0;

// Lighthouse display function
void lighthouse()
{
    static int counter = 4;

    if (lighthouseFlag)
    {
        if (counter == 4)
        {
            int xStart, xFinish, yStart;
            oledC_clearScreen();
            for (xStart = xFinish = 144, yStart = 48; yStart < 97; ++yStart, --xStart, ++xFinish)
            {
                oledC_DrawRectangle(xStart / 3, yStart, xFinish / 3, 96, OLEDC_COLOR_BLUE);
            }
        }
        else
        {
            oledC_DrawRing(48, 48, 16 * counter, 30, OLEDC_COLOR_YELLOW);
            if (counter > 0)
            {
                oledC_DrawRing(48, 48, 16 * (counter - 1), 30, OLEDC_COLOR_BLACK);
            }
        }

        counter = (counter + 1) % 5;
        lighthouseFlag = 0;
    }
}

// Timer 1 Interrupt Service Routine
void __attribute__((__interrupt__)) _T1Interrupt(void)
{
    lighthouseFlag = 1;
    IFS0bits.T1IF = 0; // Clear Timer 1 interrupt flag
}

// Main function
int main(void)
{
    SYSTEM_Initialize();
    user_init();

    while (1)
    {
        lighthouse();
    }

    return 0;
}
