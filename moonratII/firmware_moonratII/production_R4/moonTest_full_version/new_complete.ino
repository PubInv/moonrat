// REQUIRED LIBRARIES
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fuzzy.h>
#include <PID_v1.h>
#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// #define KALMAN // Uncomment to use the Kalman Filter

//* Define control technique 
//#define FUZZY
#define PID_

//OLED VARIABLES
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// I/0 VARIABLES
// const int sensorPin = A0; //TMP37 SOC pin
OneWire ourWire(A0);
#define HEATER_PIN  10 //Heater (termopad) pin
#define BUZZER_PIN  9 // Buzzer pin
#define BUTTON_SELECT 5 //Button = SELECT pin
#define BUTTON_UP 7 //Button =  UP pin
#define BUTTON_DOWN 6 //Button = DOWN pin
float CurrentTemp;
float OldErrorInput = 0.0;
float calibration;
float FilteredTemp = 0.0;

// GRAPH VARIABLES
const int numPoints = 60;
float TempHistory[numPoints] = {0};
const int barWidth = SCREEN_WIDTH / numPoints;

// MENU VARIABLES
int selectedOption = 0;
int tempMaxOptions[] = {29, 35, 37, 41.5};
int timeMaxOptions[] = {12, 24, 36, 48};
int tempMin = 0;
int tempMax = 0;
int timeMin = 0;
int timeMax = 0;
int totalOptions = 4; // Change this to the total number of options in the menu

#ifdef KALMAN
  // KALMAN FILTER VARIABLES
  float Q = 1E-9;       // Process variance
  float R = 1.12E-5;     // Reading variance
  float Pc = 0.0;
  float G = 0.0;
  float P = 1.0;
  float Xp = 0.0;
  float Zp = 0.0;
  float FilteredTemp = 0.0;
#endif

// TIMER VARIABLES
unsigned long timeNow = 0;
unsigned long timeLast = 0;
int startingHour = 0;
int seconds = 0;
int secondOld = 0;
int minutes = 0;
int hours = startingHour;

#ifdef FUZZY
  // Fuzzy
  Fuzzy *fuzzy = new Fuzzy();

  // FuzzyInput(Error)
  FuzzySet *negative = new FuzzySet(-50, -50, -5,0);
  FuzzySet *zero = new FuzzySet(-3, 0, 0, 3);
  FuzzySet *positive = new FuzzySet(0, 5, 50, 50);

  // FuzzyInput(DiffError)
  FuzzySet *dnegative = new FuzzySet(-50, -50, -2, 0);
  FuzzySet *dzero = new FuzzySet(-1, 0, 0, 1);
  FuzzySet *dpositive = new FuzzySet(0, 2, 50, 50);

  // FuzzyOutput(Frequency)
  FuzzySet *fast = new FuzzySet(10, 200, 255, 255);
  FuzzySet *average = new FuzzySet(0, 5, 5, 20);
  FuzzySet *slow = new FuzzySet(0, 0, 1, 5);

#endif

#ifdef PID_
  //* PID controller
  float Kp = 312.7907;
  float Ki = 520.0;
  float Kd = 230.0;
  double setPoint; // Desired reference for the controller
  double contolInput; // Sensor's information in voltage
  double controlOutput; // Control's output signal

  PID moonPID(&contolInput, &controlOutput, &setPoint, Kp, Ki, Kd, DIRECT);

#endif

//* Digital sensor
DallasTemperature sensor(&ourWire);

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

void setup() {

  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  analogWrite(BUZZER_PIN, 100); //Put to 100 after tests
  delay(500);
  analogWrite(BUZZER_PIN,0);
  delay(100);
  Serial.begin(9600);
  sensor.begin(); //* Init sensor
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error while initializing OLED"));
    for (;;)
      ;
  }
  delay(100);
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

  #ifdef FUZZY
    // FuzzyInput (Error)
    FuzzyInput *Error = new FuzzyInput(1);

    Error->addFuzzySet(negative);
    Error->addFuzzySet(zero);
    Error->addFuzzySet(positive);
    fuzzy->addFuzzyInput(Error);

    // FuzzyInput (DiffError)
    FuzzyInput *DiffError = new FuzzyInput(2);

    DiffError->addFuzzySet(dnegative);
    DiffError->addFuzzySet(dzero);
    DiffError->addFuzzySet(dpositive);
    fuzzy->addFuzzyInput(DiffError);

    // FuzzyOutput (Frequency)
    FuzzyOutput *Frequency = new FuzzyOutput(1);

    Frequency->addFuzzySet(fast);
    Frequency->addFuzzySet(average);
    Frequency->addFuzzySet(slow);
    fuzzy->addFuzzyOutput(Frequency);

      // Building Fuzzy Rules
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
    // Including a FuzzySet to this FuzzyRuleConsequent
    thenpwmslow->addOutput(slow);
    // Instantiating a FuzzyRule objects
    FuzzyRule *fuzzyRule03 = new FuzzyRule(3, ifOutputEqualtoSlow, thenpwmslow);
    // Including the FuzzyRule into Fuzzy
    fuzzy->addFuzzyRule(fuzzyRule03);

  #endif

  #ifdef PID_
    //* Init PID controller
    moonPID.SetMode(AUTOMATIC);
    moonPID.SetOutputLimits(0, 255);

  #endif
}

void loop() {

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
  timeLast = millis()/1000;
  while (flag == 0)
  {
    if ((digitalRead(BUTTON_UP) == HIGH) && (digitalRead(BUTTON_DOWN) == HIGH) && (digitalRead(BUTTON_SELECT) == HIGH))
      {
        flag = 1;
      }
    Serial.println(timeMax);
    while((hours >= timeMax) && exit == 0)
      {
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
        analogWrite(BUZZER_PIN, 100); //Put to 100 after tests
        delay(100);
        analogWrite(BUZZER_PIN,0);
        delay(100);
        flag = 1;
        if ((digitalRead(BUTTON_UP) == HIGH) && (digitalRead(BUTTON_DOWN) == HIGH))
        {
          exit = 1;
        }
      }
    timeNow = millis()/1000; // the number of milliseconds that have passed since boot
    seconds = timeNow - timeLast;

    if(seconds - secondOld != 0)
    {
      // CurrentTemp = (analogRead(sensorPin) *4.98*50.0)/1024.0; //T6G SOC based on TMP37 Sensor
      //* New sensor...
      sensor.requestTemperatures();
      CurrentTemp = sensor.getTempCByIndex(0);

      #if defined(KALMAN)
      
        // Kalman Filter
        Pc = P + Q;
        G = Pc/(Pc + R);
        P = (1-G) * Pc;
        Xp = FilteredTemp;
        Zp = Xp;
        FilteredTemp = G*(CurrentTemp-Zp)+Xp;
      
      #else
      
        FilteredTemp = CurrentTemp;
      
      #endif

      for (int i = 0; i < numPoints - 1; i++)
      {
        TempHistory[i] = TempHistory[i + 1];
      }

      TempHistory[numPoints - 1] = FilteredTemp;

      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(SSD1306_WHITE);
      display.setCursor(0, 2);
      display.print("Temp:");
      display.print(FilteredTemp);
      display.println("C ");
      
  // Display temperature graph on the rest of the screen
    for (int i = 0; i < numPoints; i++) {
      int x = i * barWidth;
      int barHeight = map(TempHistory[i], 0, 50, 0, 44);
      //display.fillRect(x, SCREEN_HEIGHT - barHeight, barWidth, barHeight-14, SSD1306_WHITE);
      display.fillRect(x, 54 - barHeight, barWidth, barHeight, SSD1306_WHITE);
      //display.fillRect(x, 32 , 1, 6, SSD1306_WHITE);
    }

    // Show the horizontal lines for maximum and minimum temperatures.
    int yMax = map(tempMax, 0, 45, 0, 14);
    int yMin = map(tempMin, 0, 45, 50, 0);
    //display.drawFastHLine(0, yMax, SCREEN_WIDTH, SSD1306_WHITE);
    display.drawLine(0, 10, SCREEN_WIDTH, 10, SSD1306_WHITE);
    //display.drawFastHLine(0, yMin, SCREEN_WIDTH, SSD1306_WHITE);
    display.drawLine(0, 54, SCREEN_WIDTH, 54, SSD1306_WHITE);

    // Show the maximum and minimum temperature label
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(74, 2); // Position adjustment for Max legend
    display.print("Goal:");
    display.print(tempMax);
    display.print("C");
    display.setCursor(45, SCREEN_HEIGHT-8);
    display.print("Timer:");
    display.print(hours);
    display.print(":");
    display.print(minutes);
    display.print(":");
    display.print(seconds);
   
    #ifdef FUZZY
      //* FUZZY
      // get entrances
      float ErrorInput = FilteredTemp-tempMax;
      float DiffErrorInput = ErrorInput - OldErrorInput;
      OldErrorInput = ErrorInput;
      fuzzy->setInput(1, ErrorInput);
      fuzzy->setInput(2, DiffErrorInput);
      fuzzy->fuzzify();
      float OutputFreq = fuzzy->defuzzify(1);

      Serial.println("");
      Serial.print("Frequency Output: ");
      Serial.println(round(OutputFreq));
      Serial.println("");
      analogWrite(HEATER_PIN, int(round(OutputFreq)));
      display.setCursor(0, SCREEN_HEIGHT-8); // Position adjustment for Min legend
      display.print("PWM:");
      display.print(int(round(OutputFreq)));
      display.display();

    #endif

    #ifdef PID_
      //* PID
      setPoint = tempMax;
      contolInput = FilteredTemp;
      moonPID.Compute();

      Serial.println("");
      analogWrite(HEATER_PIN, int(round(controlOutput)));
      // analogWrite(HEATER_PIN, 52); // PID Tunning mode
      display.setCursor(0, SCREEN_HEIGHT-8); // Position adjustment for Min legend
      display.print("PWM:");
      display.print(int(round(controlOutput)));
      // display.print(52); // PID Tunning mode
      display.display();

    #endif
  }
  if (seconds == 60)
{
  timeLast = timeNow;
  minutes = minutes + 1; 
}
if (minutes == 60)
{
  minutes = 0;
  hours = hours + 1; 
}
secondOld = seconds;
}
  //displayTempMenu();
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
  display.println(F("41.5 C"));
  
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
