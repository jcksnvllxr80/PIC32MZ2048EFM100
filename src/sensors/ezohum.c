#include <string.h>
#include <stdio.h>
#include "../utils/utils.h"
#include "ezohum.h"

size_t ReadEZ0HumFixedSize(char *buffer, size_t expectedSize)
{
  if (UART2_Read(buffer, expectedSize))
  {
    return 1; // Success
  }
  return 0; // Failed to read expected bytes
}

void configureEZ0Hum()
{

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

void doTempAndHumEZ0Hum()
{
  char command[] = EZ0HUM_SINGLE_READ;
  UART2_Write(command, sizeof(command) - 1); // excluding the null terminator

  // Buffer to store the response
  char responseBuffer[EZ0HUM_BUFFER_LENGTH + 1];     // Adjust size based on expected response length
  memset(responseBuffer, 0, sizeof(responseBuffer)); // Clear the buffer

  delay_ms(EZ0HUM_READ_DELAY);

  size_t bytesRead = ReadEZ0HumFixedSize(responseBuffer, sizeof(responseBuffer));

  // Check if any data was read
  if (bytesRead > 0)
  {
    printf("Received Data: %s\n", responseBuffer); // Print the received data
  }
  else
  {
    printf("No data received or an error occurred.\n");
  }
}