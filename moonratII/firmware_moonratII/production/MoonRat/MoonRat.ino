//  Moonrat Control Code
//  Copyright (C) 2024 Robert L. Read, Horacio García-Vázquez
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.

// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

// Define control strategy 
// #define STRATEGY_THERMOSTAT 1
#define STRATEGY_PID 2
// #define STRATEGY_FUZZY 3

#if defined(STRATEGY_FUZZY)
#include <Fuzzy.h>
#endif
#if defined(STRATEGY_PID)
#include <PID_v1.h>
#endif
// The thermostat function requires no code!

#include <stdio.h>
#include <math.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Persistence.h"
#include "utility.h"
#include <DailyStruggleButton.h>

// // Set controller type here
// #define R3 //Uncomment for Arduino UNO R3
#define R4 //Uncomment for Arduino UNO R4
#ifdef R3 //Configuration for Arduino UNO R3
//  #include <progmemAssert.h>
#endif

const int DEBUG = 2;

const int DEBUG_BUTTONS = 0;

#define BAUD_RATE 115200
//OLED VARIABLES
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

OneWire ourWire(A0);

//* Digital sensor
DallasTemperature sensor(&ourWire);

// I/0 VARIABLES
// NOTE: Rob thinks this is a TMP36
// const int sensorPin = A0; //TMP37 SOC pin
int HEATER_PIN = 10; //Heater (termopad) pin
#define BUZZER_PIN  9 // Buzzer pin
#define BUTTON_SL 5 //Button = SELECT pin
#define BUTTON_UP 7 //Button =  UP pin
#define BUTTON_DN 6 //Button = DOWN pin
#define TRIGGER_PIN 11 // used to set a time signal for triggering oscilloscope
#define DEBOUNCING_TIME_MS 100

DailyStruggleButton upButton;
DailyStruggleButton dnButton;
DailyStruggleButton slButton;


bool returnToMain = false;
// replace this with an enum
bool inMainMenu = true;
bool showingGraph = false;


bool sel_pressed = false;
bool up_pressed = false;
bool dn_pressed = false;
bool up = false;
bool dn = false;
bool sel = false;

int menuSelection = 0;

#define NUM_MENU_SELECTIONS 4





int LOG_VERBOSE = 5;
int LOG_DEBUG   = 4;
int LOG_WARNING = 3;
int LOG_MAJOR   = 2;
int LOG_ERROR   = 1;
int LOG_PANIC   = 0;

int LOG_LEVEL   = 5;

const int DEBUG_TEMP = 1;

// These are Menu items
#define TEMPERATURE_M 0
#define GRAPH_1_M 1
#define SET_TEMP_M 2
#define STOP_M 3


#define MAX_TEMPERATURE_C 42.0

#ifdef USE_LOW_TEMP
float targetTemperatureC = 30.0;  // Celcius
#else
float targetTemperatureC = 35.0;  // Celcius
#endif

int rawTemp = 0;
float temperatureC;  //The variable we will use to store temperature in degrees.

// Variables to represent the state of the machine
// BASIC_STATE (this is different from the GUI State)
#define PREPARING 0
#define INCUBATING 1
#define FINISHED 2
int basic_state = INCUBATING;

// When Incubating, when are in two modes: warm-up phase or balance phase.
// The warm-up phase occurs when you we are first turned on or badly too cool.
// the balance phase is when we want slow stable mainteance of the target temperature.
// I am aribritraily defining warm-up phase as 4C below target phase.
#define WARM_UP_TEMP_DIFF_C 4.0
int warm_up_phase = true;

//menu variables



float CurrentTempC;
float OldErrorInput = 0.0;
float calibration;

// GRAPH VARIABLES
const int numPoints = 60;
// float TempHistory[numPoints] = {0};
const int barWidth = SCREEN_WIDTH / numPoints;

// MENU VARIABLES
int selectedOption = 0;
// WARNING! FIX!
// This code does not compile (at least in IDE 2.3.2). I will ask Horacio about this...
// int tempMaxOptions[] = {29, 35, 37, 41.5};
int tempMaxOptions[] = {29, 35, 37, 41};
int timeMaxOptions[] = {12, 24, 36, 48};
int tempMin = 0;
int tempMax = 0;
int timeMin = 0;
int timeMax = timeMaxOptions[3];

int totalOptions = 4; // Change this to the total number of options in the menu

extern bool currently_heating;
extern uint32_t time_incubation_started_ms;
extern uint32_t time_heater_turned_on_ms;
extern uint32_t time_spent_heating_ms;
extern uint32_t time_spent_incubating_ms;
extern uint32_t time_of_last_entry;

// KALMAN FILTER VARIABLES
float Q = 1E-9;       // Process variance
float R = 1.12E-5;     // Reading variance
float Pc = 0.0;
float G = 0.0;
float P = 1.0;
float Xp = 0.0;
float Zp = 0.0;
float FilteredTemp = 0.0;

float computeKalmanFilter(float currentTempC,float filteredTemp) {
      // Kalman Filter
      Pc = P + Q;
      G = Pc/(Pc + R);
      P = (1-G) * Pc;
      Xp = filteredTemp;
      Zp = Xp;
      return G*(currentTempC-Zp)+Xp;
}

// TIMER VARIABLES
unsigned long timeNow = 0;
// unsigned long timeLast = 0;
int startingHour = 0;
int seconds = 0;
int secondsSinceTempUpdate = 0;
int minutes = 0;
int hours = startingHour;

int secondsLastDisplay = 0;

int secondsToUpdateTemp = 2;
int secondsToUpdateDisplay = 10; 

#if defined(STRATEGY_PID)
  //* PID controller
  float Kp = 312.7907;
  float Ki = 520.0;
  float Kd = 230.0;
  double setPoint; // Desired reference for the controller
  double controlInput; // Sensor's information in voltage
  double controlOutput; // Control's output signal

  PID moonPID(&controlInput, &controlOutput, &setPoint, Kp, Ki, Kd, DIRECT);

#endif



//display variables
#define WIDTH 128
#define HEIGHT 64
#define SPLIT 16  // location of the graphing area (vertially, down from the top)
#define LEFT_MARGIN 9
#define LINE_HEIGHT 9
// #define OLED_RESET 4

// graph stuff
int center = SPLIT + ((64 - SPLIT) / 2);  // (of ex axis?)
int graphwidth = 128 - LEFT_MARGIN;
int xaxisleft = LEFT_MARGIN * 2;
int xaxismid = LEFT_MARGIN + (graphwidth / 2) - 4;
int xaxisright = 128 - 24;


static const unsigned char PROGMEM image_data_Saraarray[] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xe0, 0x03, 0xe0, 0x1e, 0x01, 0x00, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x60, 0x1e, 0x01, 0x00, 0x03, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xfc, 0x38, 0xff, 0xc7, 0xc3, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0x18, 0xff, 0xcf, 0xe7, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0x88, 0xff, 0xcf, 0xe7, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0x88, 0xff, 0xcf, 0xe7, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xc8, 0xff, 0xcf, 0xe7, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xc8, 0xff, 0xcf, 0xe3, 0xc0, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0x88, 0xff, 0xcf, 0xe0, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0x88, 0xff, 0xcf, 0xe0, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0x18, 0xff, 0xc7, 0xe7, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0x78, 0x38, 0xff, 0xc7, 0xe7, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x78, 0xff, 0xcf, 0xe7, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0x07, 0xf8, 0xff, 0xcf, 0xe7, 0xff, 0x1f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xfc, 0xff, 0x8f, 0xe7, 0xff, 0x3f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xfc, 0x7f, 0x8f, 0xe7, 0xfe, 0x3f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0xff, 0xfe, 0x3e, 0x1f, 0xe7, 0xf8, 0x7f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0x00, 0x3f, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0xff, 0xc0, 0xff, 0x00, 0x03, 0x3f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xff, 0xfe, 0x7f, 0xdf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf7, 0xff, 0xfe, 0x7f, 0xcf, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xe0, 0x3f, 0x81, 0xfc, 0x07, 0x81, 0xf8, 0x1f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xe0, 0x1f, 0x81, 0xfc, 0x07, 0x81, 0xf8, 0x0f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xe0, 0xff, 0x1f, 0xe3, 0xfe, 0x3f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf0, 0x7f, 0x1f, 0xe3, 0xfe, 0x3f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf0, 0x7f, 0x1f, 0xe3, 0xfe, 0x7f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf0, 0x3f, 0x1f, 0xf1, 0xfc, 0x7f, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf3, 0x1f, 0x1f, 0xf1, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf3, 0x1f, 0x1f, 0xf9, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf3, 0x8f, 0x1f, 0xf8, 0xf8, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf3, 0xc7, 0x1f, 0xfc, 0xf9, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf3, 0xe3, 0x1f, 0xfc, 0x71, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf3, 0xe3, 0x1f, 0xfc, 0x73, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf3, 0xf1, 0x1f, 0xfe, 0x23, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf3, 0xf8, 0x1f, 0xfe, 0x23, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf3, 0xfc, 0x1f, 0xff, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xfc, 0x7f, 0xf3, 0xfc, 0x1f, 0xff, 0x07, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf8, 0x3f, 0xe3, 0xfe, 0x1f, 0xff, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf0, 0x1f, 0x80, 0x7f, 0x1f, 0xff, 0x8f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xf0, 0x3f, 0x80, 0xff, 0x9f, 0xff, 0x9f, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

void fastBeep() {
    analogWrite(BUZZER_PIN, 100); //Put to 100 after tests
    delay(50);
    analogWrite(BUZZER_PIN,0);
}


double read_tempC() {
  sensor.requestTemperatures();  // Send the command to get temperatures
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  float tempC = sensor.getTempCByIndex(0);

  // Check if reading was successful
  if (tempC != DEVICE_DISCONNECTED_C) {
    //    Serial.print(F("Temperature for the device 1 (index 0) is: "));
    //   Serial.println(tempC);
  } else {
    Serial.println(F("Error: Could not read temperature data"));
  }
  return tempC;
}

void serialSplash() {
  //Serial splash
  Serial.println(F("==================================="));
  Serial.println(MODEL_NAME);
  Serial.print(PROG_NAME);
  Serial.println(FIRMWARE_VERSION);
  Serial.print(F("Compiled at: "));
  Serial.println(F(__DATE__ " " __TIME__));  //compile date that is used for a unique identifier
  Serial.println(LICENSE);
  Serial.println(F("==================================="));
  Serial.println();
}

void showCurStatus(float temp) {
  display.clearDisplay();  //removes current plots
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print(F("T (C):"));
  display.println(temp);
  display.print(F("Power (WattHours):"));
    float avg_watts = 0.0;
  display.println(wattHours(avg_watts));
  display.display();
}

void showReport(float temp) {
  Serial.print(F("Target (C) : "));
  Serial.print(targetTemperatureC);
  Serial.print(F(" "));
  Serial.print(F("Temp : "));
  Serial.println(temp);
  Serial.print(F("Power (WattHours): "));
  float avg_watts = 0.0;
  Serial.println(wattHours(avg_watts));
  Serial.print(F("Average Watts: "));
  Serial.println(avg_watts);
}
void showSetTempMenu(float target) {
  display.clearDisplay();  //removes current plots
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(F("Up (U) to increase,"));
  display.println(F("Dn (D) to decrease,"));
  display.print(F("T (C):"));
  display.println(target);
  display.display();
  inMainMenu = false;
}


void showGraph(int eeindex) {

  Serial.print(F("eeindex: "));
  Serial.println(eeindex);

  //  int graphMaxTemp = targetTemperatureC + 2; //initializing max temp
  //  int graphMinTemp = targetTemperatureC - 2; //initializing min temp
  //  int graphMaxTemp1 = targetTemperatureC + 20; //initializing max temp
  //  int graphMinTemp1 = targetTemperatureC - 20; //initializing min temp
  //  int maxtarget = targetTemperatureC + 2; //max range for target temp
  //  int mintarget = targetTemperatureC - 2; //min range for target temp
  //TODO find out have to shift x axis values
  //Graph

  int num_samples_drawn = min(graphwidth, eeindex);
  // I guess these are supposed to be in hours...
  float timeleft = 0;                                                                 //(xaxisright-xaxisleft)*FIVE_MINUTES;
  float timemid = (((float)(num_samples_drawn) / 2) * DATA_RECORD_PERIOD) / 3600000;  //(xaxisright-xaxismid)*FIVE_MINUTES;
  float timeright = ((float)(num_samples_drawn)*DATA_RECORD_PERIOD) / 3600000;        //xaxisright*FIVE_MINUTES;

  int diff1;
  int diff2;

  // index begins at the end minus graph width
  int startIndex = (eeindex > graphwidth) ? eeindex - graphwidth : 0;
  //plots 119 points fron the EEPROM Contents at a period of 5 minutes
  // This logic should render the last 119 samples, I suppose
  float maxTempToGraph = -1.0;
  float minTempToGraph = 5000.0;
  for (int i = startIndex; i < eeindex; i++) {
    float cTempC = readIndex(i);
    minTempToGraph = min(minTempToGraph, cTempC);
    maxTempToGraph = max(maxTempToGraph, cTempC);
  }

  int graphMaxTemp = ceil(maxTempToGraph);
  int graphMinTemp = floor(minTempToGraph);

  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0, 0);
  display.print(F("# sam:"));
  display.println(eeindex);

  display.setTextSize(1);
  display.setCursor(1, SPLIT);
  display.println(graphMaxTemp);
  display.setCursor(1, 56);
  display.println(graphMinTemp);
  display.setCursor(LEFT_MARGIN * 2, 56);
  display.println(timeleft);
  display.setCursor(xaxismid, 56);
  display.println(timemid);
  display.setCursor(xaxisright, 56);
  display.println(timeright);

  // Pixels per degree
  float scale = (64.0 - SPLIT) / (float)(graphMaxTemp - graphMinTemp);  //creates scale y axis pixels

  float meanTemp = (((float)graphMaxTemp + (float)graphMinTemp) / 2.0);
  float middle = meanTemp - (float)graphMinTemp;
  int j = 0;

  for (int i = startIndex; i < eeindex; i++) {
    float cTempC = readIndex(i);
    float ypos = (cTempC - graphMinTemp);

    int x = LEFT_MARGIN + j;
    int y = (64 - (scale * ypos));
    int midy = (64 - (scale * middle));
    display.drawPixel(x, y, SSD1306_WHITE);  //plots temparature
    display.drawPixel(x, midy, SSD1306_WHITE);
    j++;
  }  

  display.display();
  inMainMenu = false;
  showingGraph = true;
}


void displayTempMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("SELECT TARGET TEMP:"));
  display.setCursor(10, 12);
  display.println(F("29 C"));
  display.setCursor(10, 24);
  display.println(F("35 C"));
  display.setCursor(10, 36);
  display.println(F("37 C"));
  display.setCursor(10, 48);
  display.println(F("41 C"));
  
  int circleY = 15 + selectedOption * 12; // Position of filled circle
  display.fillCircle(5, circleY, 3, SSD1306_WHITE);
  display.display();
  inMainMenu = false;
}


void displayTimeMenu() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("SELECT INC TIME:"));
  display.setCursor(10, 12);
  display.println(F("12 HR"));
  display.setCursor(10, 24);
  display.println(F("24 HR"));
  display.setCursor(10, 36);
  display.println(F("36 HR"));
  display.setCursor(10, 48);
  display.println(F("48 HR"));
  
  int circleY = 15 + selectedOption * 12; // Position of filled circle
  display.fillCircle(5, circleY, 3, SSD1306_WHITE);
  display.display();
}



void setMaxTemp() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("Target Temp.");
  display.setCursor(0, 16);
  display.print(tempMax);
  display.println("C");
  display.setCursor(0, 32);
  display.print("Inc Time");
  display.setCursor(0, 48);
  display.print(timeMax);
  display.println("C");
  display.display();
}
void displayExitScreen() {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(SSD1306_WHITE);
        display.setCursor(10, 8);
        display.println(F("INCUBTATION TIME"));
        display.setCursor(30, 17);
        display.println(F("COMPLETED!"));
        display.setCursor(18, 30);
        display.println(F("TO EXIT, PRESS"));
        display.setCursor(38, 39);
        display.println(F("UP/DOWN"));
        display.setCursor(18, 48);
        display.println(F("SIMULTANEOUSLY"));
        display.display();

}


#if defined(STRATEGY_FUZZY)
// fuzzy
Fuzzy *fuzzy = new Fuzzy();

// fuzzyInput(Error)
FuzzySet *negative = new FuzzySet(-50, -50, -5,0);
FuzzySet *zero = new FuzzySet(-3, 0, 0, 3);
FuzzySet *positive = new FuzzySet(0, 5, 50, 50);

// fuzzyInput(DiffError)
FuzzySet *dnegative = new FuzzySet(-50, -50, -2, 0);
FuzzySet *dzero = new FuzzySet(-1, 0, 0, 1);
FuzzySet *dpositive = new FuzzySet(0, 2, 50, 50);

// fuzzyOutput(Frequency)
FuzzySet *fast = new FuzzySet(10, 200, 255, 255);
FuzzySet *average = new FuzzySet(0, 5, 5, 20);
FuzzySet *slow = new FuzzySet(0, 0, 1, 5);

void setupFuzzy() {
 // fuzzyInput (Error)
  FuzzyInput *Error = new FuzzyInput(1);

  Error->addFuzzySet(negative);
  Error->addFuzzySet(zero);
  Error->addFuzzySet(positive);
  fuzzy->addFuzzyInput(Error);

  // fuzzyInput (DiffError)
  FuzzyInput *DiffError = new FuzzyInput(2);

  DiffError->addFuzzySet(dnegative);
  DiffError->addFuzzySet(dzero);
  DiffError->addFuzzySet(dpositive);
  fuzzy->addFuzzyInput(DiffError);

  // fuzzyOutput (Frequency)
  FuzzyOutput *Frequency = new FuzzyOutput(1);

  Frequency->addFuzzySet(fast);
  Frequency->addFuzzySet(average);
  Frequency->addFuzzySet(slow);
  fuzzy->addFuzzyOutput(Frequency);

    // Building fuzzy Rules
  FuzzyRuleAntecedent *errornegativeAndderrornegative = new FuzzyRuleAntecedent();
  errornegativeAndderrornegative->joinWithAND(negative, dnegative);
  FuzzyRuleAntecedent *errornegativeAndderrorzero = new FuzzyRuleAntecedent();
  errornegativeAndderrorzero->joinWithAND(negative, dzero);
  FuzzyRuleAntecedent *errorzeroAndderrornegative = new FuzzyRuleAntecedent();
  errorzeroAndderrornegative->joinWithAND(zero, dnegative);
  FuzzyRuleAntecedent *ifOutputEqualtoFastpartial = new FuzzyRuleAntecedent();
  ifOutputEqualtoFastpartial->joinWithOR( errornegativeAndderrornegative, errornegativeAndderrorzero);
  FuzzyRuleAntecedent *ifOutputEqualtoFast = new FuzzyRuleAntecedent();
  ifOutputEqualtoFast->joinWithOR( ifOutputEqualtoFastpartial, errorzeroAndderrornegative); 
  FuzzyRuleConsequent *thenpwmfast = new FuzzyRuleConsequent();
  thenpwmfast->addOutput(fast);
  FuzzyRule *fuzzyRule01 = new FuzzyRule(1, ifOutputEqualtoFast, thenpwmfast);
  fuzzy->addFuzzyRule(fuzzyRule01);


  FuzzyRuleAntecedent *errorpositiveAndderrornegative = new FuzzyRuleAntecedent();
  errorpositiveAndderrornegative->joinWithAND(positive, dnegative);
  FuzzyRuleAntecedent *errorzeroAndderrorzero = new FuzzyRuleAntecedent();
  errorzeroAndderrorzero->joinWithAND(zero, dzero);
  FuzzyRuleAntecedent *errornegativeAndderrorpositive = new FuzzyRuleAntecedent();
  errornegativeAndderrorpositive->joinWithAND(negative, dpositive);
  FuzzyRuleAntecedent *ifOutputEqualtoAveragePartial = new FuzzyRuleAntecedent();
  ifOutputEqualtoAveragePartial ->joinWithOR( errorpositiveAndderrornegative, errorzeroAndderrorzero);
  FuzzyRuleAntecedent *ifOutputEqualtoAverage = new FuzzyRuleAntecedent();
  ifOutputEqualtoAverage ->joinWithOR( ifOutputEqualtoAveragePartial, errornegativeAndderrorpositive); 
  FuzzyRuleConsequent *thenpwmAverage = new FuzzyRuleConsequent();
  thenpwmAverage->addOutput(average);
  FuzzyRule *fuzzyRule02 = new FuzzyRule(2, ifOutputEqualtoAverage, thenpwmAverage);
  fuzzy->addFuzzyRule(fuzzyRule02);


  FuzzyRuleAntecedent *errorpositiveAndderrorzero = new FuzzyRuleAntecedent();
  errorpositiveAndderrorzero->joinWithAND(positive, dzero);
  FuzzyRuleAntecedent *errorzeroAndderrorpositive = new FuzzyRuleAntecedent();
  errorzeroAndderrorpositive->joinWithAND(zero, dpositive);
  FuzzyRuleAntecedent *errorpositiveAndderrorpositive = new FuzzyRuleAntecedent();
  errorpositiveAndderrorpositive->joinWithAND(positive, dpositive);
  FuzzyRuleAntecedent *ifOutputEqualtoSlowPartial = new FuzzyRuleAntecedent();
  ifOutputEqualtoSlowPartial ->joinWithOR( errorpositiveAndderrorzero, errorzeroAndderrorpositive); 
  FuzzyRuleAntecedent *ifOutputEqualtoSlow = new FuzzyRuleAntecedent();
  ifOutputEqualtoSlow ->joinWithOR(ifOutputEqualtoSlowPartial, errorpositiveAndderrorpositive);
  FuzzyRuleConsequent *thenpwmslow = new FuzzyRuleConsequent();
  // Including a FuzzySet to this fuzzyRuleConsequent
  thenpwmslow->addOutput(slow);
  // Instantiating a fuzzyRule objects
  FuzzyRule *fuzzyRule03 = new FuzzyRule(3, ifOutputEqualtoSlow, thenpwmslow);
  // Including the fuzzyRule into fuzzy
  fuzzy->addFuzzyRule(fuzzyRule03);

}
#endif

#if defined(STRATEGY_PID)
void setupPID() {
    //* Init PID controller
    moonPID.SetMode(AUTOMATIC);
    moonPID.SetOutputLimits(0, 255);
}
#endif

void upCallBack(byte buttonEvent);
void dnCallBack(byte buttonEvent);
void slCallBack(byte buttonEvent);


void setupButtons() {
  upButton.set(BUTTON_UP,upCallBack,INT_PULL_UP);
  dnButton.set(BUTTON_DN,dnCallBack,INT_PULL_UP);
  slButton.set(BUTTON_SL,slCallBack,INT_PULL_UP);

#define DEBOUNCE_TIME_MS 200
  upButton.setDebounceTime(DEBOUNCE_TIME_MS);
  dnButton.setDebounceTime(DEBOUNCE_TIME_MS);
  slButton.setDebounceTime(DEBOUNCE_TIME_MS);
}

void setup() {
  tempMax = 0;
  seconds = 0;
  minutes = 0;
  hours = 0;

  incubationON();

  float storedTargetTemp = getTargetTemp();
  if (storedTargetTemp > MAX_TEMPERATURE_C) {
    setTargetTemp(MAX_TEMPERATURE_C);
    storedTargetTemp = MAX_TEMPERATURE_C;
  }
  if (storedTargetTemp < 25.0) {
    setTargetTemp(35.0);
    Serial.println(F("target temp to low, setting to 35!"));
    storedTargetTemp = 35.0;
  }
  targetTemperatureC = storedTargetTemp;


  // This is insufficient to read the DS18B20 temperature sensor
  pinMode(A0, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);
  analogWrite(BUZZER_PIN, 50);
  delay(500);
  analogWrite(BUZZER_PIN,0);
  delay(100);
  Serial.begin(BAUD_RATE);

  sensor.begin(); //* Init sensor

#if defined(R4)
 analogReadResolution(10);
 analogReference(AR_DEFAULT);
#else
  analogReference(DEFAULT);
#endif

  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  analogWrite(BUZZER_PIN, 100); //Put to 100 after tests
  delay(500);
  analogWrite(BUZZER_PIN,0);
  delay(100);
  Serial.begin(BAUD_RATE);
  delay(1000);
  while (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error while initializing OLED"));
  }
  delay(100);
  Serial.println("successfully started OLED");
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  // Display static text
  display.drawBitmap(0, 0, image_data_Saraarray, 128, 64, 1);
  display.display();
  delay(1500);
  display.clearDisplay();
 // pinMode(BUTTON_SL, INPUT_PULLUP);
 //  pinMode(BUTTON_UP, INPUT_PULLUP);
 // pinMode(BUTTON_DN, INPUT_PULLUP);


#if defined(STRATEGY_FUZZY)
  setupFuzzy();
#endif

#if defined(STRATEGY_PID)
  setupPID();
#endif
  serialSplash();

  setupButtons(); 

  showMenu();


  Serial.println("Setup Done");
  delay(3000);
}
int renderDisplay_bool = 0;


#if defined(STRATEGY_PID)
float pidPWM_fraction(float curC) {
      //* PID
      setPoint = targetTemperatureC;
      controlInput = curC;
      Serial.print("setPoint, controlInput: ");
      Serial.println(setPoint);
      Serial.println(controlInput);
      moonPID.Compute();
      Serial.print("PID controlOuput: ");
      Serial.println(controlOutput);
   return controlOutput / 255.0;
}
#endif

#if defined(STRATEGY_FUZZY)
float fuzzyPWM_fraction(float curC) {
      //* fuzzy
      // get entrances
      float ErrorInput = curC-targetTemperatureC;
      float DiffErrorInput = ErrorInput - OldErrorInput;
      OldErrorInput = ErrorInput;
      fuzzy->setInput(1, ErrorInput);
      fuzzy->setInput(2, DiffErrorInput);
      fuzzy->fuzzify();
      return fuzzy->defuzzify(1) / 255.0;
}
#endif

#if defined(STRATEGY_THERMOSTAT)
float thermostatPWM_fraction(curC) {
  if (curC < targetTemperatureC) {
    return 1.0;
  } else {
    return 0.0;
  }
}
#endif


int exit_flag = 0;
void checkTimeExpired() {
  if ((hours >= timeMax) && exit_flag == 0)
  {
    displayExitScreen();
    analogWrite(BUZZER_PIN, 100); //Put to 100 after tests
    delay(100);
    analogWrite(BUZZER_PIN,0);
    delay(100);
    if ((digitalRead(BUTTON_UP) == HIGH) && (digitalRead(BUTTON_DN) == HIGH))
    {
      exit_flag = 1;
      // Now we will dump the data and end this state...
      dumpData();

    }
  }
}

/* Displays the menu.

   Menu Options:
   - Show Temperature : switches to temperature display mode
   - Show Graph : switches to graph dispay mode
   - Start/Stop : starts or stops data collection
   - Set Target : sets the target incubation temperature
   - Export : send temperature data through serial port to a pc
   - Reset : resets temperature data
*/
void showMenu() {
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  //print title
  display.setCursor(0, 0);
  display.setTextSize(2);
  display.print(F("Menu"));
  display.setTextSize(1);
  //displays time since incubation started
  display.setCursor(56, 7);

  // It's really useful to see the current temp at all times...
  double curTempC = read_tempC();
  display.print(curTempC);
  display.print(" ");
  char currentTime[80];
  // String currentTime =
  getTimeString(currentTime);
  // chop off the seconds...
  currentTime[5] = '\0';
  display.println(currentTime);
  //menu option for current temperature
  display.setTextSize(1);
  display.setCursor(LEFT_MARGIN, SPLIT);
  //menu option for current temperature
  display.println(F("Temperature"));
  display.setCursor(LEFT_MARGIN, SPLIT + LINE_HEIGHT);
  //menu option for detailed graph
  display.println(F("Graph 1"));
  display.setCursor(LEFT_MARGIN, SPLIT + 2 * LINE_HEIGHT);

  //menu option for graph over 48 hours
  //  display.println(F("Graph 2"));
  //  display.setCursor(LEFT_MARGIN, SPLIT + 3 * LINE_HEIGHT);
  //menu option for entering the target temparature. Entered by clicking the up/down buttons
  display.println(F("Set Temperature"));
  display.setCursor(LEFT_MARGIN, SPLIT + 3 * LINE_HEIGHT);
  if (incubating) {
    display.println(F("Stop"));
  } else {
    display.println(F("Start"));
  }

  //print cursor
  display.setCursor(0, SPLIT + menuSelection * LINE_HEIGHT);
  display.fillCircle(3, SPLIT + menuSelection * LINE_HEIGHT + 3, 3, SSD1306_WHITE);

  display.display();
  inMainMenu = true;
}


#define BUTTON_POLL_PERIOD 0




void upCallBack(byte buttonEvent) {
  switch (buttonEvent){
		case onPress:
      Serial.println("UP onpress");
			break;
		case onRelease:
      Serial.println("UP onRelease");
        if (inMainMenu) {
          if (menuSelection > 0) {  
            menuSelection--;
            fastBeep();
            showMenu();

          } else {
            // We should make a beep here
          }
        } else {
        switch (menuSelection) {
          case TEMPERATURE_M:
            showCurStatus(CurrentTempC);
            inMainMenu = false;
          break;
          case GRAPH_1_M:
          {
            uint16_t index = getIndex();
            Serial.println("showing graph");
            showGraph(index);
            inMainMenu = false;
            showingGraph = true;
            Serial.println("returned to Graph");
          }
          break;
          case SET_TEMP_M:
          {
            inMainMenu = false;
            showingGraph = false;
              if (targetTemperatureC > MAX_TEMPERATURE_C) {
                targetTemperatureC = MAX_TEMPERATURE_C;
              }
              targetTemperatureC += 0.5;
              setTargetTemp(targetTemperatureC);
            // This is wrong; we whould be showing instructions
            showSetTempMenu(targetTemperatureC);
          }
          break;
        }
			break;
	}
  }
}
void dnCallBack(byte buttonEvent) {
  switch (buttonEvent){
		case onPress:
      Serial.println("DN onpress");
			break;
		case onRelease:
      Serial.println("DN onRelease");
      if (inMainMenu) {
        if (menuSelection < (NUM_MENU_SELECTIONS - 1)) {  
          menuSelection++;
          fastBeep();
          showMenu();
          Serial.println("Menu Selection Increased!");
          Serial.println(menuSelection);
        } else {
          // We should make a beep here.
          Serial.println("INTERNAL ERROR");
        }
      } else {
       switch (menuSelection) {
          case TEMPERATURE_M:
            showCurStatus(CurrentTempC);
            inMainMenu = false;
          break;
          case GRAPH_1_M:
          {
            uint16_t index = getIndex();
            Serial.println("showing graph");
            showGraph(index);
            inMainMenu = false;
            showingGraph = true;
            Serial.println("returned to Graph");
          }
          break;
          case SET_TEMP_M:
          {
            inMainMenu = false;
            showingGraph = false;
            targetTemperatureC -= 0.5;
            setTargetTemp(targetTemperatureC);
            // This is wrong; we whould be showing instructions
            showSetTempMenu(targetTemperatureC);
          }
          break;
        }
      }
			break;
	}
}
void slCallBack(byte buttonEvent) {
  switch (buttonEvent){
		case onPress:
      Serial.println("SL onpress");
			break;
		case onRelease:
      Serial.println("SL onRelease");
      fastBeep();
      Serial.println(inMainMenu);
      if (!inMainMenu) {
        switch (menuSelection) {
          case TEMPERATURE_M:
            showMenu();
          break;
          case GRAPH_1_M:
          {
            showMenu();
          }
          break;
          case SET_TEMP_M:
          {
            showMenu();
          }
          break;
          case STOP_M:
          {
            Serial.println(F("Toggling Incubation!"));
            rom_reset();
            Serial.println(F("EEPROM RESET!"));
            showMenu();
            incubating = !incubating;
            showingGraph = false;
          }
          break;
        }
      } else {
        switch (menuSelection) {
          case TEMPERATURE_M:
            showCurStatus(CurrentTempC);
            inMainMenu = false;
          break;
          case GRAPH_1_M:
          {
            uint16_t index = getIndex();
            Serial.println("showing graph");
            showGraph(index);
            inMainMenu = false;
            showingGraph = true;
            Serial.println("returned to Graph");
            showMenu();
          }
          break;
          case SET_TEMP_M:
          {
            // This is wrong; we whould be showing instructions
            showSetTempMenu(targetTemperatureC);
            inMainMenu = false;
            showingGraph = false;
          }
          break;
          case STOP_M:
          {
            Serial.println(F("Toggling Incubation!"));
            rom_reset();
            Serial.println(F("EEPROM RESET!"));
            showMenu();
            incubating = !incubating;
            inMainMenu = true;
            showingGraph = false;
          }
          break;
        }
      }
			break;
	}
}

uint32_t last_temp_check_ms = 0;
uint32_t time_since_last_report_ms = 0;
#define PERIOD_TO_CHECK_TEMP_MS 5000
#define REPORT_PERIOD 5000
void loop() {

  upButton.poll();
  dnButton.poll();
  slButton.poll();
//  processButtons();

  uint32_t loop_start = millis(); 
  if (loop_start < (last_temp_check_ms + PERIOD_TO_CHECK_TEMP_MS))
    return;

  boolean dumpdata = false;
  boolean reset = false;
  //read keyboard entries from the serial monitor
  char T;
  if (Serial.available()) {
    T = Serial.read();  //getting string input in varaible "T"
    Serial.print(F("T ="));
    Serial.println(T);
    dumpdata = (T == 'x');
    reset = (T == 'r');
  } 
  if (dumpdata) {
    Serial.println(F("dump Data"));
    dumpData();
  }
  if (reset) {
    Serial.println(F("reset ROM INDEX"));
    rom_reset();
  }
 
  if (exit_flag) {
    return;
  }

  selectedOption = 0;

  // if ((digitalRead(BUTTON_UP) == HIGH) && (digitalRead(BUTTON_DN) == HIGH) && (digitalRead(BUTTON_SL) == HIGH))
  // {
  //     exit_flag = 1;
  //     Serial.println("Because 3 buttons were put at once, this incubation is cancelled.");
  //     return;
  // }

  checkTimeExpired();
  timeNow = (millis()/1000); // the number of milliseconds that have passed since boot
  seconds = timeNow;
  uint32_t time_now_ms = millis();
  uint32_t time_since_last_entry = time_now_ms - time_of_last_entry;
    // assert(secondsToUpdateDisplay > secondsToUpdateTemp);
  if((seconds - secondsSinceTempUpdate) >= secondsToUpdateTemp) // This occurs one per second....
  {
    sensor.requestTemperatures();
    CurrentTempC = read_tempC();
#if defined(KALMAN)
    FilteredTemp = kalmanFilter(CurrentTempC,FilteredTemp);
#else
    FilteredTemp = CurrentTempC;
#endif
    renderDisplay_bool = (seconds - secondsLastDisplay) > secondsToUpdateDisplay;
    if (renderDisplay_bool) {
      float outputPWM_fraction;
#if defined(STRATEGY_FUZZY)
      outputPWM_fraction = fuzzyPWM_fraction(FilteredTemp);

      Serial.print("fuzzyPWM Output: ");
      Serial.println(outputPWM_fraction);
#endif
#if defined(STRATEGY_PID)
      outputPWM_fraction = pidPWM_fraction(FilteredTemp);
#endif
#if defined(STRATEGY_THERMOSTAT)
      outputPWM_fraction = thermostatPWM_fraction(FilteredTemp);
#endif

      setHeatPWM_fraction(outputPWM_fraction);  
      // Serial.print("Heater PWM: ");
      // Serial. print((float) outputPWM * 100.0 / 256.0);
      // Serial.println("%");
      // display.setCursor(0, SCREEN_HEIGHT-8); // Position adjustment for Min legend
      // display.print("PWM:");
      // display.print(int(round(outputPWM)));
      // display.display();
      // Serial.print("renderDisplay: ");
      if (showingGraph) {
        uint16_t index  = getIndex();
        showGraph(index);
      }

      if ((time_now_ms - time_since_last_report_ms) > REPORT_PERIOD) {
        showReport(CurrentTempC);
          time_since_last_report_ms = time_now_ms;
        }
      if (time_since_last_entry > DATA_RECORD_PERIOD) {
          //  entryFlag = false;
          Serial.println(F("Writing New Entry"));
          writeNewEntry(CurrentTempC);
          time_of_last_entry = time_now_ms;
        }
      secondsLastDisplay = seconds;
    } 
    secondsSinceTempUpdate = seconds; 
  }
  minutes = seconds / 60;
  hours = minutes / 60;
}