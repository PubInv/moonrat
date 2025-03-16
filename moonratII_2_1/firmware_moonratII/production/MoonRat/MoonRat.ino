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

// Define the control strategy in utility.h!

#include "utility.h"

#if defined(STRATEGY_FUZZY)
#include <Fuzzy.h>
#include "FuzzyLogic.h"
extern Fuzzy *fuzzy;
#endif
#if defined(STRATEGY_PID)
#include <PID_v1.h>
#endif
// The thermostat function requires no code!

#include <stdio.h>
#include <math.h>
#include <EEPROM.h>
#include <Wire.h>
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "Persistence.h"
#include "Display.h"

#include <JC_Button.h>          // https://github.com/JChristensen/JC_Button

// // Set controller type here
// #define R3 //Uncomment for Arduino UNO R3
#define R4 //Uncomment for Arduino UNO R4
#ifdef R3 //Configuration for Arduino UNO R3
//  #include <progmemAssert.h>
#endif

const int DEBUG = 2;

const int DEBUG_BUTTONS = 0;

#define BAUD_RATE 115200

extern Adafruit_SSD1306 display;

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

const int DEBOUNCE_TIME_MS = 150;
Button upBtn(BUTTON_UP,DEBOUNCE_TIME_MS);
Button dnBtn(BUTTON_DN,DEBOUNCE_TIME_MS);
Button slBtn(BUTTON_SL,DEBOUNCE_TIME_MS);

// replace this with an enum
bool inMainMenu = true;
bool showingGraph = false;

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

// GRAPH VARIABLES
const int numPoints = 60;

// MENU VARIABLES
int selectedOption = 0;

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

// TIMER VARIABLES
unsigned long timeNow = 0;
// unsigned long timeLast = 0;
int startingHour = 0;
int seconds = 0;
int secondsSinceTempUpdate = 0;
int minutes = 0;
int hours = startingHour;

int secondsLastDisplay = 0;

int secondsToUpdateTemp = 10;
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
  upBtn.begin();
  dnBtn.begin();
  slBtn.begin();
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

  setupDisplay();

  // pinMode(BUTTON_SL, INPUT_PULLUP);
  // pinMode(BUTTON_UP, INPUT_PULLUP);
  // pinMode(BUTTON_DN, INPUT_PULLUP);


#if defined(STRATEGY_FUZZY)
  setupFuzzy();
#endif

#if defined(STRATEGY_PID)
  setupPID();
#endif
  serialSplash();
  setupButtons();
  showMenu(CurrentTempC);
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

#define BUTTON_POLL_PERIOD 0

const int onPress = 1;
const int onRelease = 2;

void upCallBack(byte buttonEvent) {
  if (inMainMenu) {
    if (menuSelection > 0) {
      menuSelection--;
      fastBeep();
      showMenu(CurrentTempC);

    } else {
      // Does two sound like it is not an error?
      fastBeep();
      delay(100);
      fastBeep();
      delay(100);
      fastBeep();
    }
  } else {
    switch (menuSelection) {
    case TEMPERATURE_M:
      showCurStatus(CurrentTempC);
      showingGraph = false;
      inMainMenu = false;
      break;
    case GRAPH_1_M:
      {
        uint16_t index = getIndex();
        showGraph(index);
        inMainMenu = false;
        showingGraph = true;
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
  }
}
void dnCallBack(byte buttonEvent) {
  if (inMainMenu) {
    if (menuSelection < (NUM_MENU_SELECTIONS - 1)) {
      menuSelection++;
      fastBeep();
      showMenu(CurrentTempC);
    } else {
      // Does two sound like it is not an error?
      fastBeep();
      delay(100);
      fastBeep();
      delay(100);
      fastBeep();
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
        showGraph(index);
        inMainMenu = false;
        showingGraph = true;
      }
      break;
    case SET_TEMP_M:
      {
        inMainMenu = false;
        showingGraph = false;
        targetTemperatureC -= 0.5;
        setTargetTemp(targetTemperatureC);
        showSetTempMenu(targetTemperatureC);
      }
      break;
    }
  }
}
void slCallBack(byte buttonEvent) {
  Serial.println("SL onRelease");
  fastBeep();
  Serial.println(inMainMenu);
  if (!inMainMenu) {
    switch (menuSelection) {
    case TEMPERATURE_M:
      showMenu(CurrentTempC);
      break;
    case GRAPH_1_M:
      {
        showMenu(CurrentTempC);
      }
      break;
    case SET_TEMP_M:
      {
        showMenu(CurrentTempC);
      }
      break;
    case STOP_M:
      {
        Serial.println(F("Toggling Incubation!"));
        rom_reset();
        Serial.println(F("EEPROM RESET!"));
        showMenu(CurrentTempC);
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
        showGraph(index);
        inMainMenu = false;
        showingGraph = true;
      }
      break;
    case SET_TEMP_M:
      {
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
        showMenu(CurrentTempC);
        incubating = !incubating;
        inMainMenu = true;
        showingGraph = false;
      }
      break;
    }
  }
}

uint32_t last_temp_check_ms = 0;
uint32_t time_since_last_report_ms = 0;
#define PERIOD_TO_CHECK_TEMP_MS 5000
#define REPORT_PERIOD 5000
int n = 0;
void loop() {

  upBtn.read();
  if (upBtn.wasPressed())
      upCallBack(onRelease);

  dnBtn.read();
  if (dnBtn.wasPressed())
      dnCallBack(onRelease);

  slBtn.read();
  if (slBtn.wasPressed())
      slCallBack(onRelease);

  if ((digitalRead(BUTTON_UP) == HIGH) && (digitalRead(BUTTON_DN) == HIGH) && (digitalRead(BUTTON_SL) == HIGH))
    {
      exit_flag = 1;
      Serial.println("Because 3 buttons were put at once, this incubation is cancelled.");
      return;
    }

  uint32_t loop_start = millis();
  if (loop_start < (last_temp_check_ms + PERIOD_TO_CHECK_TEMP_MS))
    return;

  //read keyboard entries from the serial monitor
  char T;
  if (Serial.available()) {
    T = Serial.read();  //getting string input in varaible "T"
    Serial.print(F("T ="));
    Serial.println(T);
    if (T == 'x') {
      Serial.println(F("dump Data"));
      dumpData();
    }
    if (T == 'r') {
      Serial.println(F("reset ROM INDEX"));
      rom_reset();
    }
  }

  if (exit_flag) {
    // What to do in this case is not tested or obvious, but we consider the incubation over.
    setHeatPWM_fraction(0.0);
    return;
  }

  selectedOption = 0;

  checkTimeExpired();
  timeNow = (millis()/1000); // the number of milliseconds that have passed since boot
  seconds = timeNow;
  uint32_t time_now_ms = millis();
  uint32_t time_since_last_entry = time_now_ms - time_of_last_entry;

  // assert(secondsToUpdateDisplay > secondsToUpdateTemp);
  if((seconds - secondsSinceTempUpdate) >= secondsToUpdateTemp) // This occurs one per second....
    {
      CurrentTempC = read_tempC();

      renderDisplay_bool = (seconds - secondsLastDisplay) > secondsToUpdateDisplay;
      if (renderDisplay_bool) {
        float outputPWM_fraction;
#if defined(STRATEGY_FUZZY)
        outputPWM_fraction = fuzzyPWM_fraction(CurrentTempC);
#endif
#if defined(STRATEGY_PID)
        outputPWM_fraction = pidPWM_fraction(CurrentTempC);
#endif
#if defined(STRATEGY_THERMOSTAT)
        outputPWM_fraction = thermostatPWM_fraction(CurrentTempC);
#endif

        setHeatPWM_fraction(outputPWM_fraction);
        if (showingGraph) {
          uint16_t index  = getIndex();
          showGraph(index);
        }

        if ((time_now_ms - time_since_last_report_ms) > REPORT_PERIOD) {
          showReport(CurrentTempC);
          time_since_last_report_ms = time_now_ms;
        }
        if (time_since_last_entry > DATA_RECORD_PERIOD) {
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
