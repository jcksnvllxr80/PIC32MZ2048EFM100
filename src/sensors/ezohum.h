#include "definitions.h"

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

// Function prototypes
size_t ReadEZ0HumFixedSize(char *buffer, size_t expectedSize);
void configureEZ0Hum();
void doTempAndHumEZ0Hum();