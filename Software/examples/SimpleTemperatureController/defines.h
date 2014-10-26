


#define VERSION   5 // Version of firmware

/*******************************************
*                 DIVERS
*******************************************/
#define second() (millis()/1000)


/*******************************************
*                 IO
*******************************************/
#define RIGHT    21
#define DOWN     18
#define UP        2
#define OK       20
#define LEFT     19

#define BUZZER   42
#define FAN       5
#define RELAY    43

// pause after recognising a key-press:
#define DT_FAST 20 // ms
#define DT_SLOW 250 // ms


/*******************************************
*             DEFAULT VALUES
*******************************************/

#define TEMPERATURE_SET      20 // °C
#define REG_GAIN    5  // %PWM/K
#define RAMP_LIM 2.5 // K/s


/*******************************************
*             EEPROM
*******************************************/
#define BOOT_ADDR           1
#define BOOT_VALUE         (17+VERSION)

#define TEMPERATURE_SET_MSB_ADDR  5
#define TEMPERATURE_SET_LSB_ADDR  6

#define REG_GAIN_MSB_ADDR  7
#define REG_GAIN_LSB_ADDR  8

#define RAMP_LIM_MSB_ADDR 9
#define RAMP_LIM_LSB_ADDR 10


/*******************************************
*                 MAX6675
*******************************************/
#define UNIT      1    // Units to readout temp (0= F, 1= °C)
#define OFFSET  0.0    // Temperature compensation error



