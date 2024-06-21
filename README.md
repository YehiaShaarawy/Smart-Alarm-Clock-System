# Smart-Alarm-Clock-System
 Traditional alarm clocks can be disruptive and often fail
to wake users in a gentle, natural way. Additionally,
people may forget to set their alarms, leading to missed
appointments and disrupted routines. A smart alarm
clock that integrates with the environment can offer a
more effective and pleasant waking experience.

<img width="890" alt="SCR-20240621-pxnq" src="https://github.com/YehiaSharawy/Smart-Alarm-Clock-System/assets/65984199/17136911-b5b5-4455-8ad2-9123a9b67b38">



## Hardware Components:
1. BreadBoard
2. Microcontroller "ESP32"
3. Real Time Clock
4. LCD Screen
5. LDR Sensor
6. Buzzer

## Communication between Hardware and Software:
1. IOT Protocol & Broker: MQTT
2. Serverless: EMQX

## Step-By-Step
1. Start the serverless server "EMQX" -> https://www.emqx.com/en
2. Open a broker app "MQTTX" and start a connection
3. Restart the esp32 and volia

### Documentation that helped
1. https://www.emqx.com/en/blog/esp32-connects-to-the-free-public-mqtt-broker
2. https://docs.emqx.com/en/cloud/latest/connect_to_deployments/mqttx.html
3. https://mqttx.app/docs/get-started
