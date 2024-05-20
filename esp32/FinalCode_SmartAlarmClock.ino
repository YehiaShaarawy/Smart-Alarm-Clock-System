// ------------------------- Libraries  -------------------------
#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>  // Include the LiquidCrystal_I2C library

#include <WiFi.h>
#include <PubSubClient.h>

#include "pitches.h"

// ------------------------- Instances  -------------------------
const char *topic_sunrise = "SUNRISE";  // Topic for sunrise/sunset messages
const char *topic_stopwatch = "STOPWATCH";  // New topic for sunrise value upon LDR detection
const char *topic_alarm = "alarmesp32";

RTC_DS3231 rtc;                      // Create an instance of the RTC_DS3231 class
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Initialize the I2C LCD with its address

WiFiClient espClient;
PubSubClient client(espClient);

// ------------------------- Variables  -------------------------

String numbersClock = "";
String extractedNumber = "";

int timerMode=0;
long startTime;
double timeR;
int paused =0;

int displayMode = 0;

String sunrise = "ON";
String sunset = "OFF";

const int ldrPin = 4;
const int buzzer = 5;
int isoff = 0;

// WiFi
const char *ssid = "****";  // Enter your WiFi name
const char *password = "****";   // Enter WiFi password

// MQTT Broker
const char *mqtt_broker = "broker.emqx.io";
const char *topic = "CLOCK";
const char *mqtt_username = "brokeralarm";
const char *mqtt_password = "brokeralarm";
const int mqtt_port = 1883;  //<------changes everytime

// ------------------------- MQTT Connection -------------------------
void connectMQTT() {
  Serial.begin(115200);
  // Connecting to a Wi-Fi network
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println("Connected to the Wi-Fi network");
  // Connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
    Serial.printf("The client %s connects to the public MQTT broker\n", client_id.c_str());
    if (client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Public EMQX MQTT broker connected");
    } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
    }
  }
  // Publish and subscribe
  client.publish(topic, "Welcome To our Smart Alarm");
  client.subscribe(topic);
  client.subscribe(topic_sunrise);
  client.subscribe(topic_stopwatch);
  client.subscribe(topic_alarm);

  Serial.println("MQTT has been Connected!");
}
// ------------------------- Receive MQTT Messages ------------------------- 
void callback(char *topic, byte *payload, unsigned int length) {

  if(String(topic)=="STOPWATCH")
  {
    // Serial.print("Received [");
    // Serial.print(topic);
    // Serial.print("]: ");

    String received_msg_stopwatch = "";

    for (int i = 0; i < length; i++)
    {
      received_msg_stopwatch += (char)payload[i];
      Serial.print((char)payload[i]);
    }

    Serial.println();

    numbersClock = received_msg_stopwatch;

    if (received_msg_stopwatch == "s")
    {
      displayMode = 1;
      if (paused == 0){startTime = millis();}
      paused = 1;
      timerMode++;
      delay(400);
      Serial.println("ssssssssssstart");
    }
    if (received_msg_stopwatch == "p")
    {
      timerMode--;
      Serial.println("ppppppppause");
    }
    if (received_msg_stopwatch == "r")
    {
      numbersClock ="00:00:00";
      Serial.println("resst");

    }
    if (received_msg_stopwatch == "BACK")
    {
      displayMode = 0;
      timeR = 0;
      Serial.println("BAACKKK");

    }
  }else if (String(topic)=="alarmesp32"){
    // Serial.print("Received [");
    // Serial.print(topic);
    // Serial.print("]: ");

    String received_msg_alarm = "";

    for (int i = 0; i < length; i++)
    {
      received_msg_alarm += (char)payload[i];
      Serial.print((char)payload[i]);
    }

    Serial.println();

    if (received_msg_alarm == "BUZZ")
    {
      tone(buzzer,600,3000);
    }
  }
}
// ------------------------- Display Stopwatch -------------------------
void stopwatch() {
  lcd.clear();
  lcd.setCursor(6, 0);
  lcd.print("StopWatch");

  if (timerMode == 1) {
  lcd.setCursor(0, 1);
    numbersClock.trim();
    extractedNumber = "";
    for (char c : numbersClock) {
      if (isdigit(c)) {
        extractedNumber += c;
        if (extractedNumber.length() % 2 == 0 && extractedNumber.length() < numbersClock.length()) {
          if(!(extractedNumber.length()/2))
            extractedNumber += ":";
          
        }
      }
    }

    String formattedTime = formatTime(extractedNumber);
    // Serial.println(formattedTime);
    lcd.print(formattedTime);

  } else {
    lcd.setCursor(0, 1);
    lcd.print(numbersClock);
  }
}

String formatTime(String numbersClock) {
  if (numbersClock.length() < 6) {
    while (numbersClock.length() < 6) {
      numbersClock = "0" + numbersClock;
    }
  }

  String hours = numbersClock.substring(0, 2);
  String minutes = numbersClock.substring(2, 4);
  String seconds = numbersClock.substring(4, 6);

  return hours + ":" + minutes + ":" + seconds;
}

// ------------------------- Display Date & Time -------------------------
void date_Time() {
  DateTime now = rtc.now();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Date: ");
  lcd.print(now.year(), DEC);
  lcd.print("/");
  printTwoDigits(now.month());
  lcd.print("/");
  printTwoDigits(now.day());

  lcd.setCursor(0, 1);
  lcd.print("Time: ");
  printTwoDigits(now.hour() - 12);
  lcd.print(":");
  printTwoDigits(now.minute());
  lcd.print(":");
  printTwoDigits(now.second());
  lcd.print("  ");

    // Prepare data for publishing (e.g., format as string)
  String message = "Date: " + String(now.year(), DEC) + "/" + 
                    String(now.month(), DEC) + "/" + String(now.day(), DEC) + 
                    "\nTime: " + String(now.hour() - 12, DEC) + ":" + 
                    String(now.minute(), DEC) + ":" + String(now.second(), DEC);
  client.publish(topic, message.c_str());

  delay(1000);
}

void printTwoDigits(int number) {
  if (number < 10) {
    lcd.print("0");
  }
  lcd.print(number);
}



void setup() {
  Serial.begin(115200);
  Wire.begin();

  pinMode(buzzer, OUTPUT);
  noTone(buzzer);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  lcd.setCursor(5, 0);
  lcd.print("Welcome");

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }
  rtc.adjust(DateTime(__DATE__, __TIME__));
}

void loop() {
  // ---------------------LDR --------------------------- //
  int ldrValue = analogRead(ldrPin);

  int threshold = 1000;

  if (ldrValue > threshold) {
    if (isoff == 0) {
      Serial.println("Sunrise Detected!");
      Serial.println(ldrValue);
      tone(buzzer, NOTE_C4, 500);
      delay(600);
      tone(buzzer, NOTE_C4, 500);
      delay(600);
      tone(buzzer, NOTE_G4, 500);
      delay(600);
      tone(buzzer, NOTE_G4, 500);
      delay(600);
      tone(buzzer, NOTE_A4, 500);
      delay(600);
      tone(buzzer, NOTE_A4, 500);
      delay(600);
      tone(buzzer, NOTE_G4, 1000);
      delay(1200);
      tone(buzzer, NOTE_F4, 500);
      delay(600);
      tone(buzzer, NOTE_F4, 500);
      delay(600);
      tone(buzzer, NOTE_E4, 500);
      delay(600);
      tone(buzzer, NOTE_E4, 500);
      delay(600);
      tone(buzzer, NOTE_D4, 500);
      delay(600);
      tone(buzzer, NOTE_D4, 500);
      delay(600);
      tone(buzzer, NOTE_C4, 1000);
      delay(1200);
      isoff++;
    }
  }
  //  ------------------------- MQTT Connection -------------------------
  if (!client.connected()){
    connectMQTT();
  }else{
    client.loop();
  }
  //  ------------------------- Sunrise -------------------------
  if (ldrValue > threshold) {
    client.publish(topic_sunrise, sunrise.c_str());  // Publish sunrise to SUNRISE topic
    Serial.println("Sunrise Detected!");
  }else {
    // client.publish(topic_sunrise, sunset.c_str());  // Publish sunrise to SUNRISE topic
    // Serial.println("Sunset Detected!");
    
  }

  //  ------------------------- Display Mode -------------------------
  if(displayMode == 1){
    stopwatch();
    delay(100);
  }else{
    date_Time();
  }
}