#include <WiFi.h>
#include <ThingSpeak.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Arduino.h>

#include <DHT.h>
#define DHTPIN 2     
#define DHTTYPE DHT22   

DHT dht(DHTPIN, DHTTYPE);

// Data signal yang digunakan yaitu pin 4
#define ONE_WIRE_BUS 4

//prosedur untuk syntax onewire dan dallastemperature
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
 
int cuaca;
const int ANALOG_READ_PIN = 15; // or A0
//const int RESOLUTION = 12; // Could be 9-12

Adafruit_INA219 ina219;
 

//wifi 
const char* ssid = "Raspberrypi";   // your network SSID (name) 
const char* password = "1122334455";   // your network password

WiFiClient  client;

unsigned long myChannelNumber = 1811524;
const char * myWriteAPIKey = "PR2KTILD5ZV5RB14";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 10000;

void setup() {
  Serial.begin(115200);  //Initialize serial
  //Serial.begin(9600);  
  //delay(1000);
  dht.begin();
  sensors.begin();
      
  WiFi.mode(WIFI_STA);   
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  uint32_t currentFrequency; 

  if (! ina219.begin()) { 
    Serial.println("Failed to find INA219 chip");
 while (1) { delay(10); }
 }

 Serial.println("Measuring voltage and current with INA219 ...");

}

void loop() {

  //analogReadResolution(RESOLUTION);
  cuaca = analogRead(ANALOG_READ_PIN);
  
 float shuntvoltage = 0;
 float busvoltage = 0;
 float current_mA = 0;
 float loadvoltage = 0;
 float power_mW = 0;
  
 float tempe = dht.readTemperature();
 float hum = dht.readHumidity();
 sensors.requestTemperatures();
   

  
 if ((millis() - lastTime) > timerDelay) {
    
    // Connect or reconnect to WiFi
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }

shuntvoltage = ina219.getShuntVoltage_mV();
 busvoltage = ina219.getBusVoltage_V();
 current_mA = (ina219.getCurrent_mA()/1000);
 power_mW = ina219.getPower_mW();
 loadvoltage = busvoltage + (shuntvoltage / 1000);
 
  // Get a new temperature reading
  Serial.println();
  Serial.print("TempDHT: ");
  Serial.print(tempe);
  Serial.println(" ºC");
  Serial.print("TempDSB: ");
  Serial.print(sensors.getTempCByIndex(00));
  Serial.println(" ºC");
  Serial.print("Hum: ");
  Serial.print(hum);
  Serial.println(" %");
  Serial.println();
  
 Serial.print("Bus Voltage: "); 
 Serial.print(busvoltage); 
 Serial.println(" V");
 Serial.print("Current: "); 
 Serial.print(current_mA); 
 Serial.println(" mA");
 Serial.println();

  Serial.print("Cuaca: ");
  Serial.println(cuaca);
  
 delay(2000);

    ThingSpeak.setField(1, tempe);
    ThingSpeak.setField(2, hum);
    ThingSpeak.setField(3, busvoltage);
    ThingSpeak.setField(4, current_mA);
        
    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    //uncomment if you want to get temperature in Fahrenheit
    //int x = ThingSpeak.writeField(myChannelNumber, 1, temperatureF, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
  }
}
