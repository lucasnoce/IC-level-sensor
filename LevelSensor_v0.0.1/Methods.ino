/*  ======================================  */
/*                 SUMMARY:                 */
/*                                          */
/*    1 CALLBACK FUNCTIONS                  */
/*        1.1 MQTT                          */
/*    2 INITIALIZATION                      */
/*        2.1 SYSTEM                        */
/*            2.1.1 System Init             */
/*            2.1.2 System Deinit           */
/*        2.2 MQTT                          */
/*            2.2.1 MQTT Connect            */
/*            2.2.2 NBIoT Connect           */
/*    3 SYSTEM CHECK                        */
/*        3.1 BATTERY LEVEL                 */
/*    4 DISTANCE READING                    */
/*        4.1 READ LEVEL SENSOR             */
/*        4.2 DETERMINE SPEED OF SOUND      */
/*            4.2.1 Interpolate             */
/*            4.2.2 Calculate               */
/*        4.3 DISTANCE MEDIAN CALCULATION   */
/*        4.4 EXECUTE MEASUREMENT           */
/*    5 DATA OUTPUT                         */
/*        5.1 SERIAL                        */
/*        5.2 MQTT                          */
/*                                          */
/*  ======================================  */





/*  ======================================  */
/*    1 CALLBACK FUNCTIONS                  */


// 1.1 MQTT
void mqttCallback(char *topic, byte *payload, unsigned int len){
  #ifdef SERIAL_MON_DEBUG
  SerialMon.print(F("Message arrived ["));
  SerialMon.print(topic);
  SerialMon.print(F("]: "));
  SerialMon.write(payload, len);
  SerialMon.println();
  #endif  // SERIAL_MON_DEBUG
}





/*  ======================================  */
/*    2 INITIALIZATION                      */


// 2.1 SYSTEM

// 2.1.1 System Init
void systemInit(){
  // Serial Ports:
  #ifdef SERIAL_MON_DEBUG
  SerialMon.begin(115200);
  #endif  // SERIAL_MON_DEBUG

  SerialAT.begin(AM7020_BAUDRATE);

  // Peripherals:
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);

  pinMode(EXT_TRIG_PIN, OUTPUT);
  digitalWrite(EXT_TRIG_PIN, LOW);  // Osciloscope debug

  // Level Sensor init:
  pinMode(AJ_TRIG_PIN, OUTPUT);
  pinMode(AJ_ECHO_PIN, INPUT);
  digitalWrite(AJ_TRIG_PIN, LOW);

  // DHT init:
  dht.begin();
  
  // Battery Level init:
#ifdef READ_BAT_LEVEL_ADC
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  adcAttachPin(ADC_BAT_LEVEL_PIN);
#endif  // READ_BAT_LEVEL_ADC
}

// 2.1.2 System Deinit
void systemDeinit(){
  #ifdef SERIAL_MON_DEBUG
  SerialMon.flush();
  #endif  // SERIAL_MON_DEBUG

  SerialAT.flush();

  digitalWrite(LED_PIN, LOW);
  
  // ESP Deep Sleep Config:
  esp_sleep_enable_timer_wakeup((uint64_t) SLEEP_TIME_MIN * FACTOR_MIN_TO_S * FACTOR_S_TO_US);  // Set timer for SLEEP_TIME seconds
  esp_deep_sleep_start();  // Go to sleep
}


// 2.2 MQTT

// 2.2.1 MQTT Connect
void mqttConnect(void){
  #ifdef SERIAL_MON_DEBUG
  SerialMon.print(F("Connecting to "));
  SerialMon.print(MQTT_BROKER);
  SerialMon.print(F("..."));
  #endif  // SERIAL_MON_DEBUG

  // Connect to MQTT Broker
  String mqttid = ("MQTTID_" + String(random(65536)));
  while (!mqttClient.connect(mqttid.c_str(), MQTT_USERNAME, MQTT_PASSWORD)) {
    #ifdef SERIAL_MON_DEBUG
    SerialMon.println(F(" fail"));
    #endif  // SERIAL_MON_DEBUG
  }
  
  #ifdef SERIAL_MON_DEBUG
  SerialMon.println(F(" success"));
  #endif  // SERIAL_MON_DEBUG

//  mqttClient.subscribe(MQTT_TOPIC_SYS);
}

// 2.2.2 NBIoT Connect
void nbConnect(void){
  #ifdef SERIAL_MON_DEBUG
  SerialMon.println(F("Initializing Modem..."));
  #endif  // SERIAL_MON_DEBUG
  while (!modem.init() || !modem.nbiotConnect(APN, BAND)) {
    #ifdef SERIAL_MON_DEBUG
    SerialMon.print(F("."));
    #endif  // SERIAL_MON_DEBUG
  }

  #ifdef SERIAL_MON_DEBUG
  SerialMon.print(F("Waiting for network..."));
  #endif  // SERIAL_MON_DEBUG

  while (!modem.waitForNetwork()) {
    #ifdef SERIAL_MON_DEBUG
    SerialMon.print(F("."));
    #endif  // SERIAL_MON_DEBUG
  }

  #ifdef SERIAL_MON_DEBUG
  SerialMon.println(F(" success"));
  #endif  // SERIAL_MON_DEBUG
}





/*  ======================================  */
/*    3 SYSTEM CHECK                        */


// 3.1 BATTERY LEVEL
#ifdef READ_BAT_LEVEL_ADC
void readBatLevel(void){
  BatteryLevel = analogRead(ADC_BAT_LEVEL_PIN);
  // TODO
}
#else
void readBatLevel(void){
  BatteryLevel = (unsigned int)(BatteryLevel/4095)*100;
}
#endif  // READ_BAT_LEVEL_ADC





/*  ======================================  */
/*    4 DISTANCE READING                    */


// 4.1 READ LEVEL SENSOR
double readLevelSensor(double speedOfSound){
  digitalWrite(AJ_TRIG_PIN, HIGH);
  delayMicroseconds(PULSE_WIDTH_US);
  digitalWrite(AJ_TRIG_PIN, LOW);
  
  unsigned long duration = pulseIn(AJ_ECHO_PIN, HIGH);         // Reads the echoPin, returns the sound wave travel time in microseconds
  double distance = (duration * 0.0001 * speedOfSound) / 2.0;  // Speed of sound wave divided by 2 (go and back) [cm]

  return distance;
}


// 4.2 DETERMINE SPEED OF SOUND

// 4.2.1 Interpolate
#ifdef INTERPOLATE_SPEED_SOUND
double interpolateSpeedOfSound(double meanTemp, double meanUmid){
  double speedOfSound = 0.0;
  
  if (meanTemp >= 35.0){
    if (meanUmid <= 10.0)
      speedOfSound = SpeedOfSoundLUT[0][VEL_SOM_COLS_TEMP-1];
    else if (meanUmid >= 90.0)
      speedOfSound = SpeedOfSoundLUT[1][VEL_SOM_COLS_TEMP-1];
    else
      speedOfSound = SpeedOfSoundLUT[0][VEL_SOM_COLS_TEMP-1] + ((SpeedOfSoundLUT[1][VEL_SOM_COLS_TEMP-1] - SpeedOfSoundLUT[0][VEL_SOM_COLS_TEMP-1]) * (meanUmid/100));
  }
  else if (meanTemp < 10.0){
    if (meanUmid <= 10.0)
      speedOfSound = SpeedOfSoundLUT[0][0];
    else if (meanUmid >= 90.0)
      speedOfSound = SpeedOfSoundLUT[1][0];
    else
      speedOfSound = SpeedOfSoundLUT[0][0] + ((SpeedOfSoundLUT[1][0] - SpeedOfSoundLUT[0][0]) * (meanUmid/100));
  }
  else{
    for (int i=0; i<VEL_SOM_COLS_TEMP; i++){
      if (meanTemp >= (double) (10.0+((double)i/2.0)) && meanTemp < (double) (10.5+((double)i/2.0))){
        if (meanUmid <= 10.0)
          speedOfSound = SpeedOfSoundLUT[0][i];
        else if (meanUmid >= 90.0)
          speedOfSound = SpeedOfSoundLUT[1][i];
        else
          speedOfSound = SpeedOfSoundLUT[0][i] + ((SpeedOfSoundLUT[1][i] - SpeedOfSoundLUT[0][i]) * (meanUmid/100));
        break;
      }
    }
  }

  return speedOfSound;
}
#endif  // INTERPOLATE_SPEED_SOUND


// 4.2.2 Calculate
#ifdef CALCULATE_SPEED_SOUND
float calculateSpeedOfSound(float meanTemp, float meanUmid, float meanPres){
  double meanTemp_K = meanTemp + 273.15;  // Convert to Kelvin
  meanPres *= 1000.0;                     // Convert to Pa
  
  // Molecular concentration of water vapour calculated from Rh
  // using Giacomos method by Davis (1991) as implemented in DTU report 11b-1997
  float ENH = (3.14*meanPres*pow(10,-8)) + 1.00062 + (meanTemp*meanTemp*5.600*pow(10,-7));
  
  float e = 2.71828182845904523536;
  float PSV1 = (meanTemp_K*meanTemp_K*1.2378847*pow(10,-5)) - (1.9121316*meanTemp_K/100.0);
  float PSV2 = 33.93711047 - (6343.1645/meanTemp_K);
  float PSV = pow(e,PSV1)*pow(e,PSV2);

  float Xw = (meanUmid*ENH*PSV/meanPres)/100.0;  // Mole fraction of water vapour
  float Xc = 0.000400;                           // Mole fraction of carbon dioxide

  float C1 = 0.603055*meanTemp + 331.5024 - (meanTemp*meanTemp*5.28*pow(10,-4)) + (0.1495874*meanTemp + 51.471935 - (meanTemp*meanTemp*7.82*pow(10,-4)))*Xw;
  float C2 = ((-1.82*pow(10,-7)) + (3.73*meanTemp*pow(10,-8)) - (meanTemp*meanTemp*2.93*pow(10,-10)))*meanPres + (-85.20931 - 0.228525*meanTemp + (meanTemp*meanTemp*5.91*pow(10,-5)))*Xc;
  float C3 = Xw*Xw*2.835149 - meanPres*meanPres*2.15*pow(10,-13) + Xc*Xc*29.179762 + 4.86*pow(10,-4)*Xw*meanPres*Xc;
  float speedOfSound = C1 + C2 - C3;
  
  return speedOfSound;
}
#endif  // CALCULATE_SPEED_SOUND


// 4.3 DISTANCE MEDIAN CALCULATION
double calculateDistanceMedian(uint8_t numReadings, double* reading, double* readingAux){
  uint16_t idx = 0;
  double aux = 0.0;
  double median = 0.0;
  
  for (uint16_t i=0; i<numReadings; i++){
    aux = 0.0;
    idx = 0;

    for (uint16_t j=0; j<numReadings; j++){
      if (aux < readingAux[j]){
        aux = readingAux[j];
        idx = j;
      }
    }
    
    reading[i] = aux;
    readingAux[idx] = 0.0;
  }

  if (numReadings %2 == 0) median = (reading[(numReadings/2)-1] + reading[(numReadings/2)])/2;
  else median = reading[(numReadings-1)/2];

  return median;
}


// 4.4 EXECUTE MEASUREMENT
void executeMeasurement(uint16_t numReadings, uint16_t timeInterval){

  /*  Calibration due to Temperature and Umidity:  */
  
  // Readings:
  bool flagReadTempAndUmid = true;

  while (flagReadTempAndUmid){
    for (int i=0; i<NUM_CALIBRATIONS; i++){
      Temperature[i] = dht.readTemperature();
      delay(100);
      Umidity[i] = dht.readHumidity();
      delay(400);

#ifdef SERIAL_MON_DEBUG
      SerialMon.print("#");
#endif  // SERIAL_MON_DEBUG
  
      if (isnan(Temperature[i]) || isnan(Umidity[i]))
        if (i > 0) i--;
        else i = 0;
      else{
        // Mean:
        MeanTemp += (Temperature[i] / NUM_CALIBRATIONS);
        MeanUmid += (Umidity[i] / NUM_CALIBRATIONS);
      }
    }
  
    // Standard Deviation:
    for (int i=0; i<NUM_CALIBRATIONS; i++){
      StdDevTemp += ((Temperature[i] - MeanTemp) * (Temperature[i] - MeanTemp));
      StdDevUmid += ((Umidity[i] - MeanUmid) * (Umidity[i] - MeanUmid));
    }
    StdDevTemp = sqrt(StdDevTemp/NUM_CALIBRATIONS);
    StdDevUmid = sqrt(StdDevUmid/NUM_CALIBRATIONS);

    if (StdDevTemp >= MAX_STD_DEV_TEMP || StdDevUmid >= MAX_STD_DEV_UMID){  // read again because there was too much variation
    
#ifdef SERIAL_MON_DEBUG
      SerialMon.println();
#endif  // SERIAL_MON_DEBUG

      flagReadTempAndUmid = true;
      MeanTemp = 0.0;
      MeanUmid = 0.0;
      StdDevTemp = 0.0;
      StdDevUmid = 0.0;
      for (int i=0; i<NUM_CALIBRATIONS; i++){
        Temperature[i] = 0.0;
        Umidity[i] = 0.0;
      }
      delay(1000);  // TODO
    }
    else flagReadTempAndUmid = false;
  }

#ifdef SERIAL_MON_DEBUG
  SerialMon.println("\Temperature [*C]\Umidity [%]");
  for (int i=0; i<NUM_CALIBRATIONS; i++){
    SerialMon.print(Temperature[i]);
    SerialMon.print("\t");
    SerialMon.println(Umidity[i]);
  }
#endif  // SERIAL_MON_DEBUG
  
  // Speed of Sound determination:
#ifdef INTERPOLATE_SPEED_SOUND
  SpeedOfSound = interpolateSpeedOfSound(MeanTemp, MeanUmid);  // Interpolaçao para escolha da Velocidade do Som
#else
  SpeedOfSound = calculateSpeedOfSound(MeanTemp, MeanUmid, MEAN_PRESSURE_ITAJUBA);  // Interpolaçao para escolha da Velocidade do Som
#endif


  /*  Distance Readings:  */
  
  unsigned long timeBetweenReadings = millis();
  double reading[numReadings];
  double readingAux[numReadings];

#ifdef SERIAL_MON_DEBUG
  SerialMon.println("\nTempo [ms]\tDistancia [cm]");
#endif  // SERIAL_MON_DEBUG

  // Readings:
  for (uint16_t i=0; i<numReadings; i++){
    while (millis() - timeBetweenReadings < timeInterval);
    timeBetweenReadings = millis();
    
    reading[i] = readLevelSensor(SpeedOfSound);
    MeanDist += reading[i];
    readingAux[i] = reading[i];

#ifdef SERIAL_MON_DEBUG
    SerialMon.print(timeBetweenReadings);
    SerialMon.print("\t");
    SerialMon.println(reading[i],4);
#endif  //SERIAL_MON_DEBUG

  }
  
  // Mean:
  MeanDist /= numReadings;
  
  // Standard Deviation:
  for (uint16_t i=0; i<numReadings; i++){
    StdDevDist += ((reading[i] - MeanDist) * (reading[i] - MeanDist));
  }
  StdDevDist = sqrt(StdDevDist/numReadings);

  // Median:
  MedianDist = calculateDistanceMedian(numReadings, reading, readingAux);

}





/*  ======================================  */
/*    5 DATA OUTPUT                         */


// 5.1 SERIAL
void printResultsSerial(){
#ifdef SERIAL_MON_DEBUG
  SerialMon.println("\n*****  Calibration Data:  *****");
  SerialMon.print("Mean Temperature (*C):\t"); SerialMon.println(MeanTemp, 2);
  SerialMon.print("Standard Deviation:\t"); SerialMon.println(StdDevTemp, 2);
  SerialMon.print("\nMean Umidity (%):\t"); SerialMon.println(MeanUmid, 2);
  SerialMon.print("Standard Deviation:\t"); SerialMon.println(StdDevUmid, 2);
  SerialMon.print("\nSpeed of Sound (m/s):\t"); SerialMon.println(SpeedOfSound, 2);
//  delay(1);
  SerialMon.println("\n*****  Distance Data:  *****");
  SerialMon.print("Mean:\t"); SerialMon.println(MeanDist, 2);
  SerialMon.print("Median:\t"); SerialMon.println(MedianDist, 2);
  SerialMon.print("Standard Deviation:\t"); SerialMon.println(StdDevDist, 2);
//  delay(1);
  SerialMon.println("\n*****  Consumption Data:  *****");
  SerialMon.print("Battery (V):\t"); SerialMon.println(BatteryLevel);
//  delay(1);
#endif  // SERIAL_MON_DEBUG
}

// 5.2 MQTT
void printResultsMQTT(){
  digitalWrite(EXT_TRIG_PIN, HIGH);  // Osciloscope debug

#ifdef SERIAL_MON_DEBUG
  SerialMon.print("**************************************************\n");
  SerialMon.print("Initializing GSM conection...\n");
#endif  // SERIAL_MON_DEBUG
  
  nbConnect();
  mqttClient.setCallback(mqttCallback);
  mqttClient.setKeepAlive(1);
  mqttClient.setSocketTimeout(1);

  int countMqttConnect = 0;
  bool flagMqttConnected = false;
  while (flagMqttConnected == false){
    if (!mqttClient.connected()){
      countMqttConnect++;
      if (!modem.isNetworkConnected()){
        nbConnect();
      }

#ifdef SERIAL_MON_DEBUG
      SerialMon.println(F("=== MQTT NOT CONNECTED ==="));
#endif  // SERIAL_MON_DEBUG

      mqttConnect();
    }
    else{
      flagMqttConnected = true;
      countMqttConnect = 0;
    }
    
    if (countMqttConnect >= 10){

#ifdef SERIAL_MON_DEBUG
      SerialMon.println("MQTT Connection Failed - Payload NOT Sent!");
#endif  // SERIAL_MON_DEBUG

      return;
    }
//    delay(100);
  }

#ifdef SERIAL_MON_DEBUG
  SerialMon.println(F("=== MQTT CONNECTED ==="));
  SerialMon.println("Sending payload...");
#endif  // SERIAL_MON_DEBUG
  
  const byte pubDataSize = 92;
  char pubData[pubDataSize];

  snprintf(pubData, pubDataSize, "");

  snprintf(pubData, pubDataSize, "{\"variable\":\"leveldata\",\"value\":\"%i %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %i\"}",
    WakeUpID, MeanTemp, StdDevTemp, MeanUmid, StdDevUmid, SpeedOfSound, MeanDist, MedianDist, StdDevDist, BatteryLevel);
  
  mqttClient.publish(MQTT_TOPIC_READ, pubData);

  mqttClient.disconnect();

#ifdef SERIAL_MON_DEBUG
  SerialMon.println("Payload sent!");
  SerialMon.println(F("=== MQTT DISCONNECTED ==="));
#endif  // SERIAL_MON_DEBUG

  digitalWrite(EXT_TRIG_PIN, LOW);  // Osciloscope debug
}
