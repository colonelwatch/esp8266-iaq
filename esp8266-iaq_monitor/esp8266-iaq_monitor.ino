#include <Hash.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "Adafruit_CCS811.h"
#include "DHT.h"

#define WIFI_SSID "YOUR WIFI SSID"
#define WIFI_PASSWORD "YOUR WIFI PASSWORD"
#define DHTPIN 14 // Called D5 on the NodeMCU
#define DHTTYPE DHT22

Adafruit_CCS811 ccs;
DHT dht(DHTPIN, DHTTYPE);
HTTPClient http;

String id;
unsigned long start_time, hours = 0;

void blink_pin(int pin, int n){
  for(int i = 0; i < n; i++){
    digitalWrite(pin, LOW);
    delay(250);
    digitalWrite(pin, HIGH);
    delay(250);
  }
}

void store_baseline(int baseline){
  EEPROM.begin(2);
  EEPROM.write(0, baseline & 0x00ff);
  EEPROM.write(1, baseline >> 8);
  EEPROM.end();
}

int recall_baseline(){
  EEPROM.begin(2);
  int baseline;
  baseline = EEPROM.read(1);
  baseline <<= 8;
  baseline |= EEPROM.read(0);
  EEPROM.end();
  return baseline;
}

void setup() {
  delay(500); // Gives time for the below println to appear in Serial Monitor
  Serial.begin(115200);
  Serial.println("\n\nESP8266 IAQ Monitor");

  id = sha1(WiFi.macAddress()).substring(0, 8);
  Serial.print("\nYour unique ID: ");
  Serial.println("esp8266-iaq-"+id);
  Serial.print("Follow your monitor at: ");
  Serial.println("http://dweet.io/follow/esp8266-iaq-"+id);
  Serial.print("API access to your monitor at: ");
  Serial.println("https://dweet.io/get/latest/dweet/for/esp8266-iaq-"+id);
  
  Serial.println("\nHold the FLASH button within three seconds to update the baseline.");
  pinMode(0, INPUT_PULLUP);
  pinMode(LED_BUILTIN, OUTPUT);
  ccs.begin();
  dht.begin();
  delay(3000);
  
  int baseline;
  if(digitalRead(0) == LOW){
    Serial.print("Initializing with baseline update (takes 20 minutes)...");
    blink_pin(LED_BUILTIN, 3);
    digitalWrite(LED_BUILTIN, LOW);
    
    start_time = millis();
    while(millis()-start_time < 20*60*1000) delay(10); // Times out after 20 minutes

    baseline = ccs.getBaseline();
    store_baseline(baseline);
  }
  else{
    Serial.print("Initializing with stored baseline (takes 20 minutes)...");
    digitalWrite(LED_BUILTIN, LOW);

    start_time = millis();
    while(millis()-start_time < 20*60*1000) delay(10);
    
    baseline = recall_baseline();
    ccs.setBaseline(baseline);
  }
  Serial.println("initialized with baseline 0x" + String(baseline, HEX) + "!");
  digitalWrite(LED_BUILTIN, HIGH);

  Serial.print("Connecting...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("connected!");

  start_time = millis(); // Prepares millis counter for the loop counter later
}

void loop() {
  if(millis()-start_time > 3600*1000){
    start_time = millis();
    hours++; // Monotonic hours incrementer (NOT synchronous)
  }
  
  if((hours%36 == 0 && hours < 500) || ((hours-500)%144 == 0 && hours > 500)) 
    store_baseline(ccs.getBaseline());
  
  if(ccs.available()){
    if(ccs.readData() == 0){
      float humidity = dht.readHumidity();
      float celsius = dht.readTemperature();

      ccs.setEnvironmentalData(humidity, celsius);
      int co2 = ccs.geteCO2();
      
      http.begin("http://dweet.io/dweet/for/esp8266-iaq-"+id+
        "?co2="+String(co2)+
        "&humidity="+String(humidity)+
        "&celsius="+String(celsius)
      );
      
      int responsecode = http.GET();
      Serial.print("Response code: ");
      Serial.println(responsecode);
      Serial.println(http.getString());
      
      http.end();

      blink_pin(LED_BUILTIN, 2); // Two blinks take 0.5 seconds
    }
    else Serial.println("SENSOR ERROR!");
  }
  delay(1500); // DHT22 needs two seconds to prepare
}
