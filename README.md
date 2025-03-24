# PIC32MZ2048EFM100

* PIC32MZ2048EFM100 microcontroller
* blink a blue/red common anode LED using TMR_2 interrupt
* SHT3X temperature/humidity sensor on I2C_5
* EZ0HUM temperature/humidity sensor on UART_6
* MPLABX IDE v6.25
* compiler: XC32 v4.60

## CRC Checks for the SHT3x sensor use the NRSC-5 CRC calculation

verify the calculated results in the code with this site: https://crccalc.com/?crc=6596&method=CRC-8/NRSC-5&datatype=hex&outtype=hex
