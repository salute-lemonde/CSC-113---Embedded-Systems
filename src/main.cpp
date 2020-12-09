#include <Arduino.h>
#include "DHT.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <string> 
#include <iostream>
#include <sstream>
#include "esp_err.h"
#include "esp_task.h"

bool flag; 
#define LED_PIN1 32
#define LED_PIN2 26
#define BUZZER_PIN 33

#define DHTPIN 22 
#define DHTTYPE DHT11 

#define interval 50000 
#define fract 60

#define threshold 40

DHT dht(DHTPIN, DHTTYPE);

// Update these with values suitable for your network.
const char* ssid = "Springforte Lead";
const char* password = "12345678";
const char* mqtt_server = "test.mosquitto.org";
#define mqtt_port 1883

#define MQTT_SERIAL_PUBLISH_CH "myLine/input"
#define MQTT_SERIAL_SUBSCRIBE_CH "myLine/output"

WiFiClient wifiClient;

PubSubClient client(wifiClient);

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
} //setup_wifi()

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    //if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) 
    if (client.connect(clientId.c_str()) )
    {
      Serial.println("connected");
      // subscribe
      client.subscribe(MQTT_SERIAL_SUBSCRIBE_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
} //reconnect()

void publishSerialData(const char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  Serial.println(String("publishing: ") + serialData);

  client.publish(MQTT_SERIAL_PUBLISH_CH, serialData);
} //publishSerialData()

//returns true for odd levels of humidity
bool isH(float num) {
  if(num<30 || num>75)
    return true; 
  
  else 
    return false; 
} //bool isH()

//returns true for odd temperature levels
bool isT(float num) {
  if(num<27 || num>40)
    return true; 
  
  else 
    return false; 
} //bool isT

std::string convert (float number){
    std::ostringstream buff;
    buff<<number;
    return buff.str();   
} //convert()

float setnum() {
  srand((unsigned) time(0));
  float randomNumber = (float) rand()/RAND_MAX; 
  //( static_cast <float> (rand()/100.0) ); 
  
  return randomNumber*100; 
} //setnum()

float setIndex(float h, float t) {
  float nI = dht.computeHeatIndex(t, h, false);
 
  return nI; 
} //setIndex()

void sleep() {
  publishSerialData("Going to sleep now. Zzzzzzz."); 
  delay(1000); 
  esp_err_t esp_bluedroid_disable();
  esp_err_t esp_bt_controller_disable(); 
  esp_err_t esp_wifi_stop(); 
 
  Serial.flush();   //send out anything that might be in serial. 
  esp_deep_sleep_start(); 
} //sleep()

void wakeUp() {
  
  //enable bluetooth
  esp_err_t esp_bluedroid_enable(); 
 
  esp_sleep_wakeup_cause_t reason;
  reason = esp_sleep_get_wakeup_cause();
  
  switch(reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",reason); break;
  }
  Serial.println(reason); 
  publishSerialData("Awake..."); 

} //wakeUp()

void emitSound(unsigned long timeHighMs, unsigned long timeLowMs, unsigned long count)
{
  Serial.println("if we got here, that means my piezzo is not working"); 
  for(int i=0;i<count;i++) {
  
    digitalWrite(BUZZER_PIN, HIGH);   digitalWrite(LED_PIN1, HIGH);
    delay(timeHighMs);//wait for 1ms
    digitalWrite(BUZZER_PIN, LOW);    digitalWrite(LED_PIN1, LOW);
    delay(timeLowMs);//wait for 1ms
  }
} //emitsound()

void evalOutput (std::string str) {

  if(str == "true") {
    flag = true; 
    digitalWrite(LED_PIN2, HIGH);
    publishSerialData("Alarm State: ACTIVE");
    
  }
    
  if(str == "false") {
    flag = false; 
    digitalWrite(LED_PIN2, LOW);
    publishSerialData("Alarm State: INACTIVE");
    
  }

  if(str == "sleep") 
    sleep(); 
  
  if(str == "arise") 
    wakeUp(); 
}

std::string output;
void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    
    Serial.println();
    
    //collect incoming data and analyze 
    std::string output(reinterpret_cast<const char *>(payload), length);
    evalOutput(output); 
     
} //callback()

void setOdd() {
  float newh = setnum(); 
  float newt = setnum(); 
  float newhI = setIndex(newh, newt); 
    
  if(newhI>100) {  //scale it if greater than 100
    //newhI = fmod(newhI, 100); 
    Serial.print("this is the heat index"); 
    Serial.println(newhI); 
    newhI = newhI/6; 
    std::string newsI = convert(newhI); 
    newsI = newsI + "H"; 
    publishSerialData(newsI.c_str());
  }

  if(isH(newh)) {
    std::string newsh = convert(newh);
    newsh = newsh + "%"; 
    publishSerialData("WARNING: unusual humidity levels");
    //client.publish(MQTT_SERIAL_PUBLISH_CH, "WARNING: unusual humidity levels");
    publishSerialData(newsh.c_str()); 
        
    //Serial.print("newh: ");   Serial.println(newh);  
    if(flag) 
      emitSound(1000, 1000, 1); 

    else {
      digitalWrite(LED_PIN1, HIGH);
      delay(1000);
      digitalWrite(LED_PIN1, LOW);
    }
      
  }

  if(isT(newt)) {
    std::string newst = convert(newt);
    newst = newst + "°C"; 
    publishSerialData("WARNING: unusual temperature levels");
    publishSerialData(newst.c_str()); 
    //Serial.print("newt: ");   Serial.println(newt);  
    if(flag) 
      emitSound(1000, 1000, 1);
    
    else {
      digitalWrite(LED_PIN1, HIGH);
      delay(1000);
      digitalWrite(LED_PIN1, LOW);
    }
  }

}

unsigned long srtTime;        //startTime  
unsigned long stpTime;        //stopTime
unsigned long lastcTime;      //last time an odd temperature was set 
float count = 0.00;           //to control how climate levels are published 
void measureTemp() {
  //the most work is done in this block of code. collect CPU usage here 
  srtTime = millis(); 
    
  float h = dht.readHumidity();
  delay(300);   

  float t = dht.readTemperature();
  delay(300); 

  float f = dht.readTemperature(true);
  delay(300); 

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }

  // Compute heat index in Fahrenheit (the default)
  //float hif = dht.computeHeatIndex(f, h);

  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  //simulating anomalous temperatures 
  //sets a random temperature every 50 seconds        
  if(millis() > lastcTime + interval) {
		lastcTime = millis();
    setOdd(); 
  }

  //publish exact recorded temperatures if time interval is not up 
  else {
    if(fmod(count, 2) == 0) {
      std::string sh = convert(h);
      sh = sh + "%"; 

      std::string st = convert(t);
      st = st + "°C"; 

      std::string shic = convert(hic);
      shic = shic + "H"; 

      publishSerialData(sh.c_str()); 
      publishSerialData(st.c_str());
      publishSerialData(shic.c_str());
    }
    count += 0.5; 
  }

  /*
  Serial.print(F("Humidity: "));
  Serial.print(h);
  Serial.print(F("%  Temperature: "));
  Serial.print(t);
  Serial.print(F("°C "));
  Serial.print(f);
  Serial.print(F("°F  Heat index: "));
  Serial.print(hic);
  Serial.print(F("°C "));
  Serial.print(hif);
  Serial.println(F("°F"));
  */
  stpTime = millis(); 

} //measureTemp()
 
 
void getCPUusage() {
  float oldU = 0.00; 
  long diff = (stpTime - srtTime) / 100; 
  if(diff > diff + (oldU/fract) || diff < diff - (oldU/fract)) {
    std::string usage = convert(diff);
    usage = usage + "U"; 
    publishSerialData(usage.c_str());
  }
  oldU = diff;  
}

void setup() {
  //initialize pins 
  pinMode(LED_PIN1, OUTPUT);
  pinMode(LED_PIN2, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  esp_sleep_enable_touchpad_wakeup();  //configure touchpad as a wakeup source 
  touchAttachInterrupt(T2, wakeUp, threshold);   //setup interrupt on touch pad 2

  Serial.begin(115200);
  Serial.setTimeout(500);    //wait for serial data
 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();

  Serial.setTimeout(5000);
  Serial.println("before dht begin");
  dht.begin();
  //publishSerialData("false"); 
}

bool state; 
void loop() {
  
  if(state) {
    measureTemp();
    getCPUusage(); 
  }
 
  client.loop();
  if (Serial.available())
  {
    String str = Serial.readStringUntil('\n');
    publishSerialData(str.c_str());

    if(str == "S" || str == "s") {
      publishSerialData("initializing...");
      publishSerialData("measuring...");
      state = true; 
    }
  }
 }


