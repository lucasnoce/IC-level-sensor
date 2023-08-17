/*  ======================================  */
/*                 SUMMARY:                 */
/*                                          */
/*    1 NBIOT AND MQTT CONFIG:              */
/*        1.1 GSM                           */
/*        1.2 MQTT                          */
/*        1.3 TINYGSM CONFIG                */
/*        1.4 AM7020 MODULE SETUP           */
/*    2 SYSTEM CONFIG:                      */
/*        2.1 MACROS                        */
/*            2.1.1 Battery Level Read      */
/*            2.1.2 Speed of Sound          */
/*        2.2 PINOUT                        */
/*            2.2.1 NBIoT Module            */
/*            2.2.1 AJ-SR04M Module         */
/*            2.2.1 ADC Consumption Reading */
/*            2.2.2 Peripherals             */
/*        2.3 CONSTANTS AND PARAMS          */
/*            2.3.1 Input and Calibration   */
/*            2.3.2 Esp Sleep Params        */
/*            2.3.3 LUT Speed of Sound      */
/*                                          */
/*  ======================================  */


#ifndef _SYSCONFIG_H
#define _SYSCONFIG_H





/*  ======================================  */
/*    1 NBIOT AND MQTT CONFIG:              */

// 1.1 GSM
#define GSM_PIN "1010"                           // SIM card PIN
#define APN     "replace with your credentials"  // GPRS credentials
#define BAND    28  // TIM Brazil (Uplink: 703-748 MHz, Downlink: 758-803 MHz)

// 1.2 MQTT
#define MQTT_BROKER     "mqtt.tago.io"
#define MQTT_PORT       1883
#define MQTT_ID         "MQTT_1"
#define MQTT_USERNAME   "replace with username"
#define MQTT_PASSWORD   "replace with password"
#define MQTT_TOPIC_SYS  "syscheck"
#define MQTT_TOPIC_READ "readings"
#define MQTT_TOPIC_TEST "t"

// 1.3 TINYGSM CONFIG
#define TINY_GSM_MODEM_SIM7020
#define UPLOAD_INTERVAL         60000

// 1.4 AM7020 MODULE SETUP
/* AM7020 module setup: Serial port, baudrate, and reset pin */
/* ESP32 LABTEL Board */
#define ESP32_DEVKIT_V1_LABTEL
#define SerialMon              Serial
#define MONITOR_BAUDRATE       115200

#if (defined ARDUINO_AVR_UNO) || (defined ARDUINO_AVR_NANO)
    /* Arduino Uno */
    #include <SoftwareSerial.h>
    SoftwareSerial SoftSerial(8, 9);    //RX:8 TX:9    
    #define SerialAT            SoftSerial
    #define AM7020_BAUDRATE     38400
    #define AM7020_RESET        7

#elif defined ARDUINO_AVR_MEGA2560
    /* Arduino Mega2560 */
    #define SerialAT            Serial1
    #define AM7020_BAUDRATE     115200
    #define AM7020_RESET        7

#elif (defined ARDUINO_SAMD_MKRZERO)|| (defined ARDUINO_SAMD_MKRWIFI1010)
    /* Arduino MKR Series */
    #define SerialAT            Serial1
    #define AM7020_BAUDRATE     115200
    #define AM7020_RESET        A5

#elif defined ESP32_DEVKIT_V1_LABTEL
    /* ESP32 LABTEL Board */
    #define SerialAT            Serial2
    #define AM7020_BAUDRATE     115200
    #define AM7020_RESET        NBIOT_RST_PIN
     
#elif defined ARDUINO_ESP32_DEV
    /* ESP32 Boards */
    #define SerialAT            Serial2
    #define AM7020_BAUDRATE     115200
    #define AM7020_RESET        5

#endif  // Board definition






/*  ======================================  */
/*    2 SYS CONFIG:                         */


// 2.1 MACROS

// 2.1.1 Serial Debug
// #define SERIAL_MON_DEBUG

// 2.1.3 Battery Level Read
//#define READ_BAT_LEVEL_ADC

// 2.1.4 Speed of Sound Determination
// #define INTERPOLATE_SPEED_SOUND
#define CALCULATE_SPEED_SOUND

#if defined(INTERPOLATE_SPEED_SOUND) && defined(CALCULATE_SPEED_SOUND)
#error "Invalid method of determining SpeedOfSound (INTERPOLATE_SPEED_SOUND or CALCULATE_SPEED_SOUND)"
#elif !defined(INTERPOLATE_SPEED_SOUND) && !defined(CALCULATE_SPEED_SOUND)
#error "Undefind method of determining SpeedOfSound (INTERPOLATE_SPEED_SOUND or CALCULATE_SPEED_SOUND)"
#endif


// 2.2 PINOUT

// 2.2.1 NBIoT Module
#define NBIOT_URX_PIN     16     // uC RX pin connected to NBIoT Module TX
#define NBIOT_UTX_PIN     17     // uC TX pin connected to NBIoT Module RX
#define NBIOT_RST_PIN     26     // NBIoT Module RST pin

// 2.2.2 AJ-SR04M Module
#define AJ_ECHO_PIN       19     // JSN-SR04T Echo pin
#define AJ_TRIG_PIN       18     // JSN-SR04T Trigger pin

// 2.2.3 DHT Sensor
#define DHT_PIN           21     // DHT data pin
#define DHT_TYPE          DHT22  // DHT type

// 2.2.4 ADC Consumption Reading
#define ADC_BAT_LEVEL_PIN ??     // ADC battery reading pin

// 2.2.5 Peripherals
#define LED_PIN           25     // LED pin
#define EXT_TRIG_PIN      33     // LED pin


// 2.3 CONSTANTS AND PARAMS

// 2.3.1 Input and Calibration
#define NUM_CALIBRATIONS       10     // Number of Temperature/Humidity Readings in calibration process
#define NUM_READINGS           10     // Number of Distance Readings in each measurement
#define TIME_INTERVAL_READINGS 100    // Time interval in between readings [ms]
#define NUM_TX_VARIABLES       8      // Number of transmitted variables
#define MEAN_PRESSURE_ITAJUBA  101.6  // Mean Pressure in Itajuba city[kPa]
#define MAX_STD_DEV_TEMP       2.0    // Maximum Standard Deviation tolerated in Temperature values
#define MAX_STD_DEV_HUMI       3.0    // Maximum Standard Deviation tolerated in Humidity values

#define PULSE_WIDTH_US         10     // Minimum pulse width for AJ-SR04M reading

// 2.3.2 Esp Sleep Params
#define SLEEP_TIME_MIN             10       // Time beetwen readings in minutes
#define FACTOR_MIN_TO_S            60       // Factor for conversion from minutes to seconds
#define FACTOR_S_TO_US  (uint64_t) 1000000  // Factor for conversion from seconds to microseconds
#define BUFFER_SIZE                10       // Size of buffer that store last measurements (255 max)

#if defined(SLEEP_TIME_MIN) && SLEEP_TIME_MIN < 1
#undef SLEEP_TIME_MIN
#define SLEEP_TIME_MIN 1
#elif !defined(SLEEP_TIME_MIN)
#define SLEEP_TIME_MIN 1
#endif  // SLEEP_TIME_MIN

// 2.3.3 LUT Speed of Sound
#define SPEED_SOUND_COLS_TEMP 51
#define SPEED_SOUND_LINS_UR   2

#if defined(INTERPOLATE_SPEED_SOUND) && SPEED_SOUND_LINS_UR <= 2
#define INTERPOLATE_SPEED_SOUND_OK
#elif defined(INTERPOLATE_SPEED_SOUND)
#error "Invalid SPEED_SOUND_LINS_UR"
#endif




#endif /* _SYSCONFIG_H */
