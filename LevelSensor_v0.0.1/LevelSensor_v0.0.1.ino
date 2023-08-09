/* 
 *    Universidade Federal de Itajubá - UNIFEI
 *    LabTel / LAIoT
 *    Lucas Arturo Noce
 *    Jun/2023
 *    
 *    Project:     Ultrasonic Level Sensor AJ-SR04M with ESP32 via NBIoT
 *    Description: This program was developed to transmit water level
 *                 readings via NBIoT measured by ultrasonic sensor.
 * 
 *    Notes: AJ-SR04M in Testing Mode 1 (R19 = open circuit)
 */





/*  ========================================  */
/*                 LIBRARIES:                 */

#include "DHT.h"
#include "SysConfig.h"
#include <PubSubClient.h>
#include <TinyGsmClient.h>





/*  ========================================  */
/*       GLOBAL VARIABLES AND OBJECTS:        */

/*  ******************  */
/*  MQTT Transmission:  */

// TinyGSM objects:
TinyGsm       modem(SerialAT, AM7020_RESET);
TinyGsmClient tcpClient(modem);
PubSubClient  mqttClient(MQTT_BROKER, MQTT_PORT, tcpClient);


/*  *******************  */
/*  Sensors and System:  */

// DHT sensor (Temperature e Humidity):
DHT dht(DHT_PIN, DHT_TYPE);

// System variables:
RTC_DATA_ATTR uint8_t WakeUpID;

double SpeedOfSound = 0.0;
double Temperature[NUM_CALIBRATIONS];
double Humidity[NUM_CALIBRATIONS];

double MeanTemp = 0.0;
double MeanHumi = 0.0;
double StdDevTemp = 0.0;
double StdDevHumi = 0.0;

double MeanDist = 0.0;
double MedianDist = 0.0;
double StdDevDist = 0.0;

// Speed of Sound LUT (from 10 to 35 °C, with UR at 10% and 90%):
#ifdef INTERPOLATE_SPEED_SOUND_OK
const double SpeedOfSoundLUT[SPEED_SOUND_LINS_UR][SPEED_SOUND_COLS_TEMP] = {
  {337.52, 337.82, 338.12, 338.42, 338.72, 339.02, 339.32, 339.62, 339.92, 340.22, 340.51, 340.81, 341.11, 341.41, 341.70, 342.00, 342.30, 342.60, 342.89, 343.19, 343.48, 343.78, 344.08, 344.37, 344.67, 344.96, 345.26, 345.55, 345.85, 346.14, 346.44, 346.73, 347.03, 347.32, 347.62, 347.91, 348.21, 348.50, 348.79, 349.09, 349.38, 349.67, 349.97, 350.26, 350.55, 350.85, 351.14, 351.43, 351.73, 352.02, 352.31},
  {338.04, 338.36, 338.67, 338.99, 339.31, 339.63, 339.95, 340.27, 340.59, 340.91, 341.24, 341.56, 341.88, 342.20, 342.53, 342.85, 343.18, 343.50, 343.83, 344.16, 344.49, 344.82, 345.14, 345.47, 345.81, 346.14, 346.47, 346.80, 347.14, 347.47, 347.81, 348.15, 348.49, 348.83, 349.17, 349.51, 349.85, 350.20, 350.54, 350.89, 351.23, 351.58, 351.93, 352.29, 352.64, 352.99, 353.35, 353.71, 354.07, 354.43, 354.79}
};
#endif

#ifdef READ_BAT_LEVEL_ADC
unsigned int BatteryLevel = 0;
#else
unsigned int BatteryLevel = 4095;
#endif  // READ_BAT_LEVEL_ADC





/*  ========================================  */
/*                   SETUP:                   */

void setup(){
  systemInit();
  
  #ifdef SERIAL_MON_DEBUG
  SerialMon.print("               ********************               \n");
  SerialMon.print("Iniciando leitura n° ");
  SerialMon.println(WakeUpID);
  #endif  // SERIAL_MON_DEBUG
  WakeUpID++;

  readBatLevel();
  executeMeasurement(NUM_READINGS, TIME_INTERVAL_READINGS);
  // delay(10);
  printResultsMQTT();
  // delay(10);
  // printResultsSerial();

  systemDeinit();
}





/*  ========================================  */
/*                   LOOP:                    */

void loop(){}
