# IC Level Sensor
An IoT device designed to measure water levels using an ultrasonic sensing module and transmit the acquired data via NB-IoT.

This device was developed as part of a Scientific Initiation Program (Research Scholarship). The final project report can be seen at: <https://drive.google.com/file/d/1ysTlvOdBs60-NlEjqb83ue079nzxteLz/view?usp=sharing>.

## How it works
The system employs an AJ-SR04M ultrasonic sensing module connected to the ESP32 microcontroller. This module measures the time interval it takes for an ultrasonic wave to travel from the transducer to the water level and back. By calculating the distance based on the time duration, the speed of sound, and dividing the result by 2, the accurate water level is determined. As the speed of sound varies based on factors such as temperature, humidity, and pressure, a DHT31 sensor is incorporated to measure temperature and humidity (pressure is assumed constant). This additional information allows for precise estimation of the speed of sound just before each distance measurement is taken.

To transmit data to the tago.io platform, a SIM7020 NB-IoT module is utilized, employing the MQTT protocol. This facilitates the configuration of a comprehensive dashboard displaying variations in water and battery levels, local temperature and humidity over time, making it an excellent solution for monitoring purposes.

The device is specifically designed for deployment in remote environments and operates on battery power. Due to this constraint, the system primarily operates in low-power mode and wakes up periodically. In this initial version of the device, the wake-up period is intentionally kept small, and collected data is transmitted during each wake-up event via NB-IoT. This design choice aims to facilitate easier debugging and testing of the device's functionality.

The following image shows the device under test.

<img src="https://github.com/lucasnoce/IC-level-sensor/assets/62445590/a2f5373a-8d72-4e88-a297-ce5423b017e0.png" width=50% height=50%>

## Ultrasonic Sensor Testing
To validate the ultrasonic sensor functionality and capabilities, a series of tests were conducted. For this, the sensor was attached to some water tanks through 3D printed supports. The water level was measured with a measuring tape and by the sensor and the results were compared to verify how far could the sensor take accurate measurements and also to determine if the sensor presented meaningful errors or offset. The results showed that the sensor presents a very low standard deviation between subsequent measurements and that its readings consistently differed from the measuring tape by around 0.2 to 0.3 cm.

![20221125_105805_1](https://github.com/lucasnoce/IC-level-sensor/assets/62445590/43e1aafb-87eb-4bf5-ba7e-0a565576eb2f)
![Image 1](https://github.com/lucasnoce/IC-level-sensor/assets/62445590/1dc228f9-d99c-4069-a7be-b6ec46d956a7)


## NB-IoT Consumption Research
During the development of the device, a research was conducted to determine the energy consumption of the NB-IoT module during data transmission. As mentioned, the device was designed to be battery powered, so knowing this consumption information is very important to determine battery capacity. The research showed that the module's average consumption during transmissions is pretty high (around 630 mJ for the standard data packet initially used). This information lead to the conclusion that the best approach for this device would be to transmit the measurement data in a much lower rate than it is sampled. This way the NB-IoT module could be set to sleep mode for some minutes and only woken up to send data a few times per hour, saving a lot of energy and extending battery life.

This research was published and presented at the XLI Brazilian Symposium of Telecommunications and Signal Processing (Simpósio Brasileiro de Telecomunicações e Processamento de Sinais - SBrT 2023) and can be accessed at: <https://biblioteca.sbrt.org.br/articles/4557>.
