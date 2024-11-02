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
// #define STRATEGY_PID 2
#define STRATEGY_FUZZY 3
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

// // Set controller type here
// #define R3 //Uncomment for Arduino UNO R3
#define R4 //Uncomment for Arduino UNO R4
#ifdef R3 //Configuration for Arduino UNO R3
//  #include <progmemAssert.h>
#endif

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
#define BUTTON_SELECT 5 //Button = SELECT pin
#define BUTTON_UP 7 //Button =  UP pin
#define BUTTON_DOWN 6 //Button = DOWN pin
#define TRIGGER_PIN 11 // used to set a time signal for triggering oscilloscope

int LOG_VERBOSE = 5;
int LOG_DEBUG   = 4;
int LOG_WARNING = 3;
int LOG_MAJOR   = 2;
int LOG_ERROR   = 1;
int LOG_PANIC   = 0;

int LOG_LEVEL   = 5;

const int DEBUG_TEMP = 1;


#define MAX_TEMPERATURE_C 42.0


// #define USE_LOW_TEMP 1
#ifdef USE_LOW_TEMP
float targetTemperatureC = 30.0;  // Celcius
#else
float targetTemperatureC = 35.0;  // Celcius
#endif

float CurrentTemp;
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
int timeMax = 0;
int totalOptions = 4; // Change this to the total number of options in the menu

// KALMAN FILTER VARIABLES
float Q = 1E-9;       // Process variance
float R = 1.12E-5;     // Reading variance
float Pc = 0.0;
float G = 0.0;
float P = 1.0;
float Xp = 0.0;
float Zp = 0.0;
float FilteredTemp = 0.0;

float computeKalmanFilter(float currentTemp,float filteredTemp) {
      // Kalman Filter
      Pc = P + Q;
      G = Pc/(Pc + R);
      P = (1-G) * Pc;
      Xp = filteredTemp;
      Zp = Xp;
      return G*(currentTemp-Zp)+Xp;
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
  double contolInput; // Sensor's information in voltage
  double controlOutput; // Control's output signal

  PID moonPID(&contolInput, &controlOutput, &setPoint, Kp, Ki, Kd, DIRECT);

#endif

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
    float currentTemp = readIndex(i);
    minTempToGraph = min(minTempToGraph, currentTemp);
    maxTempToGraph = max(maxTempToGraph, currentTemp);
  }

  int graphMaxTemp = ceil(maxTempToGraph);
  int graphMinTemp = floor(minTempToGraph);

  Serial.print(F("graph midpoint ")); 
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

  Serial.println(F("start, eeindex"));
  Serial.println(startIndex);
  Serial.println(eeindex);
  for (int i = startIndex; i < eeindex; i++) {
    float currentTemp = readIndex(i);
    float ypos = (currentTemp - graphMinTemp);

    int x = LEFT_MARGIN + j;
    int y = (64 - (scale * ypos));
    int midy = (64 - (scale * middle));
    display.drawPixel(x, y, SSD1306_WHITE);  //plots temparature
    display.drawPixel(x, midy, SSD1306_WHITE);
    j++;
  }  

  display.display();
  Serial.println(F("display done"));
}

// void renderDisplay() {
//     Serial.println("begin render-display");
//     display.clearDisplay();
//         display.display();
//     Serial.println("begin render-display");
//       display.setTextSize(1);
//       display.setTextColor(SSD1306_WHITE);
//       display.setCursor(0, 2);
//       display.print("Temp:");
//       display.print(FilteredTemp);
//       display.println("C ");

//     // Show the horizontal lines for maximum and minimum temperatures.
//     int yMax = map(tempMax, 0, 45, 0, 14);
//     int yMin = map(tempMin, 0, 45, 50, 0);
//     //display.drawFastHLine(0, yMax, SCREEN_WIDTH, SSD1306_WHITE);
//     display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
//     //display.drawFastHLine(0, yMin, SCREEN_WIDTH, SSD1306_WHITE);
//     display.drawLine(0, 54, SCREEN_WIDTH, 54, SSD1306_WHITE);

//  // Display temperature graph on the rest of the screen
//     // for (int i = 0; i < numPoints; i++) {
//     //   int x = i * barWidth;
//     //   int barHeight = map(TempHistory[i], 0, 50, 0, 44);
//     //   //display.fillRect(x, SCREEN_HEIGHT - barHeight, barWidth, barHeight-14, SSD1306_WHITE);
//     //   display.fillRect(x, 54 - barHeight, barWidth, barHeight, SSD1306_WHITE);
//     //   //display.fillRect(x, 32 , 1, 6, SSD1306_WHITE);
//     // }

//     // Show the maximum and minimum temperature label
//     display.setTextSize(1);
//     display.setTextColor(SSD1306_WHITE);
//     display.setCursor(74, 2); // Position adjustment for Max legend
//     display.print("Goal:");
//     display.print(tempMax);
//     display.print("C");
//     display.setCursor(45, SCREEN_HEIGHT-8);
//     display.print("Timer:");
//     display.print(hours);
//     display.print(":");
//     display.print(minutes);
//     display.print(":");
//     display.print(seconds);
   

//     // get entrances
//     float ErrorInput = FilteredTemp-tempMax;
//     float DiffErrorInput = ErrorInput - OldErrorInput;
//     OldErrorInput = ErrorInput;

//     display.setCursor(0, SCREEN_HEIGHT-8); // Position adjustment for Min legend
//     display.print("PWM:");
//  //   display.print(int(round(OutputFreq)));
//     Serial.println("done with render-display");
//     display.display();
// }


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
}

void checkTempButtons() {
  while (tempMax == 0){
  if (digitalRead(BUTTON_SELECT) == HIGH) {
    Serial.println(F("BUTTON SELECT PUSHED!"));
    tempMax = tempMaxOptions[selectedOption];
    delay(200); // Debouncing
  }

  if (digitalRead(BUTTON_UP) == HIGH) {
    Serial.println(F("BUTTON UP PUSHED!"));
    selectedOption = (selectedOption - 1 + totalOptions) % totalOptions;
    delay(200); // Debouncing
    displayTempMenu();
  }

  if (digitalRead(BUTTON_DOWN) == HIGH) {
    Serial.println(F("BUTTON DOWN PUSHED!"));
    selectedOption = (selectedOption + 1) % totalOptions;
    delay(200); // Debouncing
    displayTempMenu();
  }
  }
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

void checkTimeButtons() {
  while (timeMax == 0){
  if (digitalRead(BUTTON_SELECT) == HIGH) {
    Serial.println(F("BUTTON SELECT PUSHED!"));
    timeMax = int(timeMaxOptions[selectedOption]);
    delay(200); // Debouncing
  }

  if (digitalRead(BUTTON_UP) == HIGH) {
    Serial.println(F("BUTTON UP PUSHED!"));
    selectedOption = (selectedOption - 1 + totalOptions) % totalOptions;
    delay(200); // Debouncing
    displayTimeMenu();
  }

  if (digitalRead(BUTTON_DOWN) == HIGH) {
    Serial.println(F("BUTTON DOWN PUSHED!"));
    selectedOption = (selectedOption + 1) % totalOptions;
    delay(200); // Debouncing
    displayTimeMenu();
  }
  }
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
  delay(1000);
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

void setup() {
  tempMax = 0;
  timeMax = 0;
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
  display.setCursor(20, 10);
  display.println(F("NOTE: TO RESET"));
  display.setCursor(12, 20);
  display.println(F("DURING INCUBATION,"));
  display.setCursor(22, 30);
  display.println(F("PRESS ALL THREE"));
  display.setCursor(40, 40);
  display.println(F("BUTTONS"));
  display.setCursor(20, 50);
  display.println(F("SIMULTANEOUSLY"));
  display.display();
  delay(3500);
  display.clearDisplay();
  pinMode(BUTTON_SELECT, INPUT_PULLUP);
  pinMode(BUTTON_UP, INPUT_PULLUP);
  pinMode(BUTTON_DOWN, INPUT_PULLUP);


#if defined(STRATEGY_FUZZY)
  setupFuzzy();
#endif

#if defined(STRATEGY_PID)
  setupPID();
#endif

  tempMax = 0;
  timeMax = 0;
  seconds = 0;
  minutes = 0;
  hours = 0;
  int flag = 0;
  int exit = 0;
  delay(1000);
  displayTempMenu();
  checkTempButtons();
  selectedOption = 0;
  displayTimeMenu();
  checkTimeButtons();
  setMaxTemp();


  Serial.println("Setup Done");
}
int renderDisplay_bool = 0;


#if defined(STRATEGY_PID)
int pidPWM() {
      //* PID
      setPoint = tempMax;
      contolInput = FilteredTemp;
      moonPID.Compute();
      Serial.print("PID controlOuput: ");
      Serial.println(int(round(controlOutput)));
   return controlOutput;
}
#endif

#if defined(STRATEGY_FUZZY)
int fuzzyPWM() {
      //* fuzzy
      // get entrances
      float ErrorInput = FilteredTemp-tempMax;
      float DiffErrorInput = ErrorInput - OldErrorInput;
      OldErrorInput = ErrorInput;
      fuzzy->setInput(1, ErrorInput);
      fuzzy->setInput(2, DiffErrorInput);
      fuzzy->fuzzify();
      return fuzzy->defuzzify(1);
}
#endif

#if defined(STRATEGY_TERMOSTAT)
int thermostatPWM() {
  if (curtemp > targetTemp) {
    return 255;
  } else {
    return 0;
  }
}
#endif


int exit_flag = 0;
void checkTimeExpired() {

  while((hours >= timeMax) && exit_flag == 0)
  {
    displayExitScreen();
    analogWrite(BUZZER_PIN, 100); //Put to 100 after tests
    delay(100);
    analogWrite(BUZZER_PIN,0);
    delay(100);
    if ((digitalRead(BUTTON_UP) == HIGH) && (digitalRead(BUTTON_DOWN) == HIGH))
    {
      exit_flag = 1;
    }
  }
}
void loop() {
  sensor.requestTemperatures();
  CurrentTemp = sensor.getTempCByIndex(0);
  Serial.println("CurrentTemp: ");
  Serial.println(CurrentTemp);

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

#if defined(KALMAN)
  FilteredTemp = kalmanFilter(CurrentTemp,FilteredTemp);
#else
  FilteredTemp = CurrentTemp;
#endif

  if (exit_flag) {
    Serial.println("INCUBATION DONE.");
    return;
  }

  selectedOption = 0;

  if ((digitalRead(BUTTON_UP) == HIGH) && (digitalRead(BUTTON_DOWN) == HIGH) && (digitalRead(BUTTON_SELECT) == HIGH))
  {
      exit_flag = 1;
      Serial.println("Because 3 buttons were put at once, this incubation is cancelled.");
      return;
  }

  checkTimeExpired();
  timeNow = (millis()/1000); // the number of milliseconds that have passed since boot
  seconds = timeNow;

    // assert(secondsToUpdateDisplay > secondsToUpdateTemp);
  if((seconds - secondsSinceTempUpdate) >= secondsToUpdateTemp) // This occurs one per second....
  {
      Serial.println("Updating Temp");
// // This should be replaced with a RingBuffer style implementation....
//     for (int i = 0; i < numPoints - 1; i++)
//     {
//       TempHistory[i] = TempHistory[i + 1];
//     }
//     TempHistory[numPoints - 1] = FilteredTemp;
    renderDisplay_bool = (seconds - secondsLastDisplay) > secondsToUpdateDisplay;
    if (renderDisplay_bool) {

      Serial.println("Updating Display");
      float outputPWM;
#if defined(STRATEGY_FUZZY)
      outputPWM = fuzzyPWM();
#endif
#if defined(STRATEGY_PID)
      outputPWM = pidPWM();
#endif
#if defined(STRATEGY_THERMOSTAT)
      outputPWM = thermostatPWM();
#endif

    setHeatPWM(int(round(outputPWM)));  
//     analogWrite(HEATER_PIN, int(round(outputPWM)));
      Serial.print("Heater PWM: ");
      Serial.print((float) outputPWM * 100.0 / 256.0);
      Serial.println("%");
      display.setCursor(0, SCREEN_HEIGHT-8); // Position adjustment for Min legend
      display.print("PWM:");
      display.print(int(round(outputPWM)));
      display.display();
      Serial.print("renderDisplay: ");

      uint16_t index = getIndex();
      showGraph(index);

 //     renderDisplay();
      secondsLastDisplay = seconds;
    } else {
    }
    secondsSinceTempUpdate = seconds; // This is actually the number of times this can
  }
  minutes = seconds / 60;
  hours = minutes / 60;
}