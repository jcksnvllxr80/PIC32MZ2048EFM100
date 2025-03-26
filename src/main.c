/*******************************************************************************
  Main Source File

  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    This file contains the "main" function for a project.

  Description:
    This file contains the "main" function for a project.  The
    "main" function calls the "SYS_Initialize" function to initialize the state
    machines of all modules in the system
 *******************************************************************************/

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include <string.h>
#include <stdio.h>
#include "sensors/ezohum.h"
#include "sensors/sht3x.h"
#include "utils/utils.h"
#include <xc.h> // Include for PIC32 devices
#include "definitions.h"                // SYS function prototypes

#define INIT_DELAY 1000
//#define MAIN_LOOP_DELAY 1000

// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main ( void )
{
    /* Initialize all modules */
    SYS_Initialize ( NULL );
    
    /* Initialize sensors */
    configureSHT3X();
    configureEZ0Hum();
    
    delay_ms(INIT_DELAY); // Delay after initialization

    while ( true )
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks ( );
        
        /* 5-pin header humidity and temperature via UART (polling) */
        doTempAndHumEZ0Hum();
        
        delay_ms(100);
        
        /* SMD humidity and temperature via i2c5 (polling) */
        doTempAndHumSHT3X();
        
//        delay_ms(MAIN_LOOP_DELAY); // Delay before the next loop
    }

    /* Execution should not come here during normal operation */
    return ( EXIT_FAILURE );
}
