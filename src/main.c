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
//#include <xc.h> // Include for PIC32 devices
#include "definitions.h"                // SYS function prototypes

#define INIT_DELAY 1000
#define MAIN_LOOP_DELAY 1000
/* EZ0HUM commands (carriage return is a MUST in these commands) */
#define EZ0HUM_SINGLE_READ "R\r"
#define EZ0HUM_CONTINUOUS_MODE_OFF "C,0\r"
#define EZ0HUM_CONTINUOUS_WITH_PERIOD "C,2\r"
#define EZ0HUM_ENABLE_HUMIDITY_OUTPUT "O,HUM,1\r"
#define EZ0HUM_ENABLE_TEMPERATURE_OUTPUT "O,T,1\r"
#define EZ0HUM_ENABLE_DEW_POINT_OUTPUT "O,Dew,1\r"
#define EZ0HUM_DISABLE_RESPONSE_CODES "*OK,0\r"

#define EZ0HUM_READ_DELAY 300
// size of response with "*OK" (24) minus size of "*OK" (3) 
// since it gets turned off in configuration
#define EZ0HUM_BUFFER_LENGTH 24 - 3  

/* SHT3X commands */
#define SHT3X_READ_DELAY 300
#define SHT3X_ADDR 0x45 // I2C address of the SHT3x-DIS sensor; ADDR high
#define SHT3X_RESPONSE_LENGTH 6 // 6 bytes response for temperature and humidity
#define SHT3X_COMMAND_LENGTH 2

uint8_t resetSHT3XCommand[2] = {0x30, 0xA2};
//uint8_t periodicMeasurement[2] = {0x20, 0x32}; // high repeatability; 1 measure / 2 seconds
uint8_t getTempAndHumCmd[2] = {0x2C, 0x06}; // high repeatability; clock stretch
//uint8_t getTempAndHumCmd[2] = {0x2C, 0x0D}; // med repeatability; clock stretch
//uint8_t getTempAndHumCmd[2] = {0x2C, 0x10}; // low repeatability; clock stretch
//uint8_t getTempAndHumCmd[2] = {0x24, 0x00}; // high repeatability
//uint8_t getTempAndHumCmd[2] = {0x24, 0x0B}; // med repeatability
//uint8_t getTempAndHumCmd[2] = {0x24, 0x16}; // low repeatability

// Function prototypes
size_t ReadEZ0HumFixedSize(char *buffer, size_t expectedSize);
void delay_ms(unsigned int milliseconds);
void configureSHT3X();
void sendCommandToSHT3X(uint8_t* command);
void readResponseFromSHT3X(uint8_t* response);
void processSHT3XSensorData(uint8_t* tempAndHumResp);
void doTempAndHumSHT3X();
uint8_t calculateCRC(uint8_t* data, size_t length);
void calculateTemperature(uint8_t* data);
void calculateHumidity(uint8_t* data);
void configureEZ0Hum();
void doTempAndHumEZ0Hum();


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

void configureEZ0Hum() {

    char command0[] = EZ0HUM_DISABLE_RESPONSE_CODES;
    UART2_Write(command0, sizeof(command0) - 1);
    delay_ms(10);
    
//    char command1[] = EZ0HUM_CONTINUOUS_WITH_PERIOD;
    char command1[] = EZ0HUM_CONTINUOUS_MODE_OFF;
    UART2_Write(command1, sizeof(command1) - 1);
    delay_ms(10);
    
    char command2[] = EZ0HUM_ENABLE_HUMIDITY_OUTPUT;
    UART2_Write(command2, sizeof(command2) - 1);
    delay_ms(10);
    
    char command3[] = EZ0HUM_ENABLE_TEMPERATURE_OUTPUT;
    UART2_Write(command3, sizeof(command3) - 1);
    delay_ms(10);
    
    char command4[] = EZ0HUM_ENABLE_DEW_POINT_OUTPUT;
    UART2_Write(command4, sizeof(command4) - 1);
}

void configureSHT3X() {
    sendCommandToSHT3X(resetSHT3XCommand);
    delay_ms(10);
//    sendCommandToSHT3X(periodicMeasurement);
}

void doTempAndHumEZ0Hum() {
    char command[] = EZ0HUM_SINGLE_READ;
    UART2_Write(command, sizeof(command) - 1);  // excluding the null terminator

        // Buffer to store the response
    char responseBuffer[EZ0HUM_BUFFER_LENGTH + 1];  // Adjust size based on expected response length
    memset(responseBuffer, 0, sizeof(responseBuffer));  // Clear the buffer

    delay_ms(EZ0HUM_READ_DELAY);
    
    size_t bytesRead = ReadEZ0HumFixedSize(responseBuffer, sizeof(responseBuffer));

    // Check if any data was read
    if (bytesRead > 0) {
        printf("Received Data: %s\n", responseBuffer);  // Print the received data
    } else {
        printf("No data received or an error occurred.\n");
    }
}

void doTempAndHumSHT3X() {
    uint8_t tempAndHumResp[SHT3X_RESPONSE_LENGTH]; // Buffer to store the response

    sendCommandToSHT3X(getTempAndHumCmd);
    
    // Wait for the sensor to process the command (delay for reading)
    delay_ms(SHT3X_READ_DELAY);

    // Read the response from the sensor
    readResponseFromSHT3X(tempAndHumResp);
    processSHT3XSensorData(tempAndHumResp);
}

void processSHT3XSensorData(uint8_t* tempAndHumResp) {
    // Extract raw temperature and humidity
    uint8_t temp_data[2] = {tempAndHumResp[0], tempAndHumResp[1]};
    uint8_t temp_crc = tempAndHumResp[2];
    uint8_t hum_data[2] = {tempAndHumResp[3], tempAndHumResp[4]};
    uint8_t hum_crc = tempAndHumResp[5];

    // Calculate CRC for temperature and humidity
    uint8_t calculated_temp_crc = calculateCRC(temp_data, 2);
    uint8_t calculated_hum_crc = calculateCRC(hum_data, 2);

    // Verify CRC
    if (calculated_temp_crc == temp_crc) {
        printf("Temperature CRC is valid.\n");
        calculateTemperature(temp_data);
    } else {
        printf("Temperature CRC is invalid!\n");
    }

    if (calculated_hum_crc == hum_crc) {
        printf("Humidity CRC is valid.\n");
        calculateHumidity(hum_data);
    } else {
        printf("Humidity CRC is invalid!\n");
        return;
    }
}

void calculateTemperature(uint8_t* temperature_data) {
    // Combine bytes into raw temperature value
    uint16_t raw_temperature = (temperature_data[0] << 8) | temperature_data[1];

    // Debug: Print raw temperature value
    printf("Raw Temperature: %u\n", raw_temperature);

    // Calculate temperature
    float temperature = -45.0 + (175.0 * ((float)raw_temperature / 65535.0f));

    // Print the temperature with 2 decimal places
    printf("Temperature: %.2f?C\n", temperature);
}

void calculateHumidity(uint8_t* humidity_data) {
    // Combine bytes into raw humidity value
    uint16_t raw_humidity = (humidity_data[0] << 8) | humidity_data[1];

    // Debug: Print raw humidity value
    printf("Raw Humidity: %u\n", raw_humidity);

    // Calculate humidity
    float humidity = 100.0 * ((float)raw_humidity / 65535.0f);

    // Print the humidity with 2 decimal places
    printf("Humidity: %.2f%%\n", humidity);
}

uint8_t calculateCRC(uint8_t* data, size_t length) {
    uint8_t crc = 0xFF; // Initialize CRC to 0xFF
    const uint8_t polynomial = 0x31; // Polynomial: x^8 + x^5 + x^4 + 1

    for (size_t i = 0; i < length; i++) {
        crc ^= data[i]; // XOR-in the next data byte

        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ polynomial; // Shift left and XOR with polynomial
            } else {
                crc <<= 1; // Just shift left
            }
        }
    }

    return crc; // Return the calculated CRC
}

size_t ReadEZ0HumFixedSize(char* buffer, size_t expectedSize) {
    if (UART2_Read(buffer, expectedSize)) {
        return 1;  // Success
    }
    return 0;  // Failed to read expected bytes
}

void sendCommandToSHT3X(uint8_t* command) {
    while (I2C5_IsBusy()); // Wait if the I2C bus is busy

    if (!I2C5_Write(SHT3X_ADDR, command, SHT3X_COMMAND_LENGTH)) {
        // Handle error (e.g., NACK or bus collision)
        I2C_ERROR error = I2C5_ErrorGet();
        printf("I2C Write Error: %d\n", error);
    }
}

void readResponseFromSHT3X(uint8_t* response) {
    while (I2C5_IsBusy()); // Wait if the I2C bus is busy

    if (!I2C5_Read(SHT3X_ADDR, response, SHT3X_RESPONSE_LENGTH)) {
        // Handle error (e.g., NACK or bus collision)
        I2C_ERROR error = I2C5_ErrorGet();
        printf("I2C Read Error: %d\n", error);
    }
}

void delay_ms(unsigned int milliseconds) {
    unsigned int count;
    while (milliseconds--) {
        for (count = 0; count < 1666; count++) {
            __asm__("NOP"); // No operation (adjust for your clock speed)
        }
    }
}

/*******************************************************************************
 End of File
*/

