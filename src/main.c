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
#include <xc.h> // Include for PIC32 devices
#include "definitions.h"                // SYS function prototypes

#define MAIN_LOOP_DELAY 100
#define EZ0HUM_SINGLE_READ "R"
#define EZ0HUM_READ_DELAY 1000
#define SHT3X_READ_DELAY 100
#define SHT3X_ADDR 0x45 // I2C address of the SHT3x-DIS sensor; ADDR high

size_t ReadEZ0HumUntil(char* buffer, size_t maxSize, char terminator);
void delay_ms(unsigned int milliseconds);
void sendCommandToSHT3X(uint8_t* command);
void readResponseFromSHT3X(uint8_t* response);
void processSHT3XSensorData(uint8_t* tempAndHumResp);
uint8_t calculateCRC(uint8_t* data, size_t length);
void calculateTemperature(uint8_t* data);
void calculateHumidity(uint8_t* data);
void doTempAndHumSEZ0Hum();
void doTempAndHumSHT3X();

uint8_t resetSHT3XCommand[2] = {0x30, 0xA2};
uint8_t getTempAndHumCmd[2] = {0x2C, 0x06}; // high repeatability clock stretch
// uint8_t getTempAndHumCmd[2] = {0x2C, 0x0D}; // med repeatability clock stretch
// uint8_t getTempAndHumCmd[2] = {0x2C, 0x10}; // low repeatability clock stretch
// uint8_t getTempAndHumCmd[2] = {0x24, 0x00}; // high repeatability
// uint8_t getTempAndHumCmd[2] = {0x24, 0x0B}; // med repeatability
// uint8_t getTempAndHumCmd[2] = {0x24, 0x16}; // low repeatability


// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

int main(void)
{
    /* Initialize all modules */
    SYS_Initialize(NULL);
    sendCommandToSHT3X(resetSHT3XCommand);

    while (true)
    {
        /* Maintain state machines of all polled MPLAB Harmony modules. */
        SYS_Tasks();
        
        /* 5-pin header humidity and temperature via UART */
        doTempAndHumSEZ0Hum();
        
        /* SMB humidity and temperature via i2c5 */
        doTempAndHumSHT3X();
        
        delay_ms(MAIN_LOOP_DELAY); // Delay before the next loop
    }

    /* Execution should not come here during normal operation */

    return EXIT_FAILURE;
}

void doTempAndHumSEZ0Hum() {
    char command[] = EZ0HUM_SINGLE_READ;
    UART6_Write(command, sizeof(command) - 1);  // excluding the null terminator

        // Buffer to store the response
    char responseBuffer[8];  // Adjust size based on expected response length
    memset(responseBuffer, 0, sizeof(responseBuffer));  // Clear the buffer

    delay_ms(EZ0HUM_READ_DELAY);
    
    // Call UART6_ReadUntil to read data until a newline character is received
    size_t bytesRead = ReadEZ0HumUntil(responseBuffer, sizeof(responseBuffer), '\n');

    // Check if any data was read
    if (bytesRead > 0) {
        printf("Received Data: %s\n", responseBuffer);  // Print the received data
    } else {
        printf("No data received or an error occurred.\n");
    }
}

void doTempAndHumSHT3X() {
    uint8_t tempAndHumResp[6];           // Buffer to store the response

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
    uint8_t hum_data[2] = {tempAndHumResp[4], tempAndHumResp[3]};
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
    printf("Temperature: %.2f°C\n", temperature);
}

void calculateHumidity(uint8_t* humidity_data) {
    // Combine bytes into raw humidity value
    uint16_t raw_humidity = (humidity_data[3] << 8) | humidity_data[4];

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

size_t ReadEZ0HumUntil(char* buffer, size_t maxSize, char terminator) {
    size_t index = 0;
    while (index < maxSize) {
        char c;
        if (!UART6_Read(&c, 1)) {
            break;  // Stop if no more data
        }
        buffer[index++] = c;
        if (c == terminator) {
            break;  // Stop if terminator is received
        }
    }
    return index;  // Return the number of bytes read
}

void sendCommandToSHT3X(uint8_t* command) {
    while (I2C5_IsBusy()); // Wait if the I2C bus is busy

    if (!I2C5_Write(SHT3X_ADDR, command, sizeof(command))) {
        // Handle error (e.g., NACK or bus collision)
        I2C_ERROR error = I2C5_ErrorGet();
        printf("I2C Write Error: %d\n", error);
    }
}

void readResponseFromSHT3X(uint8_t* response) {
    while (I2C5_IsBusy()); // Wait if the I2C bus is busy

    if (!I2C5_Read(SHT3X_ADDR, response, sizeof(response))) {
        // Handle error (e.g., NACK or bus collision)
        I2C_ERROR error = I2C5_ErrorGet();
        printf("I2C Read Error: %d\n", error);
    }
}

void delay_ms(unsigned int milliseconds) {
    unsigned int count;
    while (milliseconds--) {
        for (count = 0; count < 1000; count++) {
            __asm__("NOP"); // No operation (adjust for your clock speed)
        }
    }
}

/*******************************************************************************
 End of File
*/

