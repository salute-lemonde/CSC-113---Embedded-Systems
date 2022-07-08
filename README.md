# ClimateMonitoringDevice


This is a README file that explains the final project. 

The essence of this project is create an embedded IOT system for monitoring temperature and humidity conditions
that will connect to the internet via MQTT protocol and report anomalous weather conditions. 

Refer to final Project template for more details about the embedded system 

void setup_wifi() 
connects the system to the internet 

const char* ssid = "myssid";
const char* password = "mypassword";
replace the values there with your personal ssid and password in order to connect to the internet 

#define MQTT_SERIAL_PUBLISH_CH "myLine/input"
#define MQTT_SERIAL_SUBSCRIBE_CH "myLine/output"
this is the line that we are publishing, and are subrcibed to
it can be replace with a different value, however the same change must be made to the low on node red 

void reconnect()
reconnect the system to the internet suppose there is a break in connection while publishing/subscribing 

publishSerialData()
publishes data to the subscribed mqtt port 

bool isH()
takes a input of type float and checks if the values 
satisfies the conditions for an anomalous humidity. 
returns true if true and false otherwise. 

bool isT()
takes a input of type float and checks if the values 
satisfies the conditions for an anomalous temperature. 
returns true if true and false otherwise. 

std::string convert()
takes an input of data type float and converts it to string
useful for publishing float values across the internet 

float setnum()
sets a random number using the clock time as the seed
returns a value of type float. this is the anomalous
temperature/humidity level that is sent across the internet 

float setIndex(float h, float t)
sets a random value of type float based on the random humidity 
and temperature levels. this value represents the heat index that 
is sent across the internet 

void sleep()
this function serves the purpose of minimizing power consumption
this function is called from the user interface on node red via a button 
when called, the esp32 device is put into a "deep sleep" 
per protocol, and because it is not turned off in deep sleep, wi-fi and 
bluetooth has to be specifically turned off. 
more so, only sending the device into deep sleep without turning off bluetooth
or wi-fi does not actually minimize power consumption. 

these are pre-defined under the header files "esp_err.h" and "esp_task.h"
esp_err_t esp_bluedroid_disable();
esp_err_t esp_bt_controller_disable(); 
esp_err_t esp_wifi_stop(); 
they are used in the sleep() function to turn of the bluetooth and wifi in the esp 

void wakeUp() 
this function "wakes" the esp32 device and reports the source of trigger that woke it up 
by the program, the esp can be awoken by using a touch pin (literally touching the pin)
or through the user interface, by clicking on the wake button

void emitSound()
this is the function that controls the buzzer in alarm mode. 
in order to light the LED simultaneously with the buzzer, the function also controls 
the LED light. esentially it sends a high signal to the buzzer and led pins, then delays for 
a set time (passed into the function) and then sends a low signal. 

void callback()
this is the function that is called when there is data to be received from the 
user interface on node red. 

the date received is in bytes so we first convert it to readable form. 
std::string output(reinterpret_cast<const char *>(payload), length);

analyze the data and call various functions based on data 
evalOutput(output); 

void evalOutput()
takes an input of type string and calls calls various if it meets certain conditions 
if input = true, sets alarm mode to active using a flag; flag = true
if input = false, sets alarm mode to inactive using a flag; flag = false
if input = sleep, invokes sleep function 
if input = arise, invokes wake up function

void setOdd()
this block of code sets odd temepratures and is called every 50 seconds 
setnum() for temp. and humidity as well as setIndex() for heat level 
are called here. 

isH() and isT() are called here to check if the climate values are abnormal 
emitSound() is also called if anomalous temeperatures are recorded 

void measureTemp()
this function: 
- measures and records the temperature from the humidity/temperature sensor 
- computes a value for heat index 
- makes sure values are read from sensor or outputs an error message

this function does the most work (calls the most functions) thus it is used to 
simulate CPU usage. 

srtTime = millis() and stpTime = millis() are called at the beginning and end of the function 
respectively to record the time which is then scaled to simulate CPU usage. the logic is that 
time taken for this function will vary since every five seconds it
calls additional functions. thus the variance in time will give some variance in CPU usage 

void getCPUusage()
computes the CPU usage using values from the 
measureTemp() function and scales it to reflect CPU usage 
publishes values to node red 

Credits: 
/*
Deep Sleep with Touch Wake Up
=====================================
This code displays how to use deep sleep with
a touch as a wake up source and how to store data in
RTC memory to use it over reboots

This code is under Public Domain License.

Author:
Pranav Cherukupalli <cherukupallip@gmail.com>
*/






