#include <stdio.h>
#include "../utils/utils.h"
#include "sht3x.h"

uint8_t resetSHT3XCommand[2] = {0x30, 0xA2};
// uint8_t periodicMeasurement[2] = {0x20, 0x32}; // high repeatability; 1 measure / 2 seconds
uint8_t getTempAndHumCmd[2] = {0x2C, 0x06}; // high repeatability; clock stretch
// uint8_t getTempAndHumCmd[2] = {0x2C, 0x0D}; // med repeatability; clock stretch
// uint8_t getTempAndHumCmd[2] = {0x2C, 0x10}; // low repeatability; clock stretch
// uint8_t getTempAndHumCmd[2] = {0x24, 0x00}; // high repeatability
// uint8_t getTempAndHumCmd[2] = {0x24, 0x0B}; // med repeatability
// uint8_t getTempAndHumCmd[2] = {0x24, 0x16}; // low repeatability

void configureSHT3X() {
    sendCommandToSHT3X(resetSHT3XCommand);
    delay_ms(10);
//    sendCommandToSHT3X(periodicMeasurement);
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