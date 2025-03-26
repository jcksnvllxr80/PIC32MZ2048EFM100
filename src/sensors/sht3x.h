#include "definitions.h"

/* SHT3X commands */
#define SHT3X_READ_DELAY 300
#define SHT3X_ADDR 0x45         // I2C address of the SHT3x-DIS sensor; ADDR high
#define SHT3X_RESPONSE_LENGTH 6 // 6 bytes response for temperature and humidity
#define SHT3X_COMMAND_LENGTH 2

void configureSHT3X();
void sendCommandToSHT3X(uint8_t *command);
void readResponseFromSHT3X(uint8_t *response);
void processSHT3XSensorData(uint8_t *tempAndHumResp);
void doTempAndHumSHT3X();
uint8_t calculateCRC(uint8_t *data, size_t length);
void calculateTemperature(uint8_t *data);
void calculateHumidity(uint8_t *data);