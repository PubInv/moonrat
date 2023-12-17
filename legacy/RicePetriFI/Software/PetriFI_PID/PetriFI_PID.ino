/*
  Team Petri-FI | SEED Internship & Rice360 Institute

  Contributors:
  Kenton Roberts
  Nancy Lindsey
  Teja Paturu
  Sara Barker

  Code Adapted From Previous Student Teams:
  Moonrats
  Minicubator
  and Contributions from the Public Invention Moonrat Team.

  This sketch allows the running of an incubation, with options
  to adjust the duration and temperature of the incubator. This
  is accomplished by using a digital thermometer and a heating pad.
  This PID version attempts to implement a PID controller instead
  of temperature thresholds for controlling the heating pad, for
  more consistent temperature control.
*/

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1351.h>
#include <SPI.h>
#include <SD.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <PID_v1.h>

// Screen Dimensions
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 128

// Screen Pins (OLED Display)
// SCLK, MOSI, and MISO are SPI pins (11-13) with CS being pin 10 (hardware SS pin)
#define SCLK_PIN          13
#define MOSI_PIN          11
#define MISO_PIN          12
#define DC_PIN            4
#define SD_CHIP_SELECT    10 // must be pin 10 (hardware SS pin on Uno)
#define OLED_CHIP_SELECT  5
#define RST_PIN           6

// Defines Button Pins
#define selectButton  8 // 10 originally
#define Up            3 // 11 originally
#define Down          9

// Define Pins for Control Devices
#define transistor    2 // transistor controls the heating pad (on/off)
#define led           0
#define piezo_alarm   1 // piezo alarm generates a sound, analog pin A1
#define one_wire_bus  7 // one-wire bus for digital thermometer

// Color Definitions with Hex Codes
#define BLACK           0x0000
#define BLUE            0x001F
#define RED             0xF800
#define GREEN           0x07E0
#define CYAN            0x07FF
#define MAGENTA         0xF81F
#define YELLOW          0xFFE0
#define WHITE           0xFFFF

// Incubation Defaults
#define DEFAULT_TEMP 30 // degrees Celsius
#define MAX_TEMP_SET 42 // degrees Celsius; max temperature that the user can set during incubation
#define MIN_TEMP_SET 20 // degrees Celsius; minimum temperature that the user can set during incubation
#define DEFAULT_TIME 48 // hours

// Assigns parameters and pins associated with the Adafruit OLED TFT screen with SD card
Adafruit_SSD1351 tft_screen = Adafruit_SSD1351(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, OLED_CHIP_SELECT, DC_PIN, RST_PIN);

bool firstTempSet = true;
bool firstTimeSet = true;
float p = 3.1415927;
float T;
int finalStatus = 0;
int menuButtonPreviousState = LOW;
int tempUpPreviousState = LOW;
int tempDownPreviousState = LOW;
int out_range_counter = 0; // counts minutes that the temperature is out of range.
int menuOpt = 0;
float runningAvg[] = {0, 0, 0, 0, 0};
float T_average;

// FIXME these were originally all integers, not doubles
// floats used for better compatibility with the PID controller
double temp_setting = DEFAULT_TEMP; // use button to increment or decrement as necessary
double temp_setting_prev = DEFAULT_TEMP;
int time_setting = DEFAULT_TIME; // default time setting is 48 hours, use button to increment or decrement as necessary
int time_setting_prev = DEFAULT_TIME; // default time setting is 48 hours, use button to increment or decrement as necessary

int DownState = 0;
int UpState = 0;
int selectButtonState = 0;

int onStatus = 0; // keeps track of heating pad status

OneWire oneWire(one_wire_bus); // sets up one-wire bus for digital thermometer
DallasTemperature sensors(&oneWire); // passes oneWire reference to Dallas Temperature

File temperatureLog; // SD card file


// PID Controller Parameters
double Kp = 4, Ki = 1, Kd = 1; // tune these parameters for optimal temperature control.
double* PID_input = (double*)&T; // temperature is input // FIXME is this the right syntax?
double* PID_output; // must also be double*
double* PID_setpoint = (double*)&temp_setting; // setpoint is given by the desired temperature
float heatingPadThreshold = 0.5; // Can be between 0 and 1. Values closer to 1 make the heating pad slower to turn on.

// PID Controller Definition for Temperature - DIRECT because output (heating) causes temperature to rise.
PID therm_PID(PID_input, PID_output, PID_setpoint, Kp, Ki, Kd, DIRECT);


// Duty Cycle Recording (for main incubation duration)
long elapsedMillisHeating = 0L;
float overallDutyCycle = 0.0;


void setup(void) {

  //battery
  pinMode(transistor, OUTPUT);
  pinMode(selectButton, INPUT);
  pinMode(Up, INPUT);
  pinMode(Down, INPUT);
  digitalWrite(transistor, LOW);

  Serial.begin(9600);
  Serial.println("Starting Up...");
  tft_screen.begin();
  tft_screen.fillScreen(BLACK);

  Serial.println("Initializing...");

  uint16_t time = millis();
  tft_screen.fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
  time = millis() - time;

  sensors.begin(); // starts up the library (for reading digital thermometer)

  pinMode(SD_CHIP_SELECT, OUTPUT);
  digitalWrite(SD_CHIP_SELECT, HIGH);

  SPI.begin();

  // Attempts to initialize the SD card.
  // Make sure that the file is already created
  // in the main directory of the SD card.
  if (!SD.begin(SD_CHIP_SELECT)) {
    Serial.println("Initialization of SD card failed...");
    while (true);
  }
  Serial.println("SD card initialized.");

  // Sets the mode of the PID controller
  therm_PID.SetMode(AUTOMATIC);
}

void loop() {
  /*
    Main loop function of the Arduino.

    Each loop, this functions reads if the select button has been pressed.
    If not, it continues to look for Up/Down input for setTemp or setTime,
    until the temperature and time have been selected.

    When the select button is pressed, we increment menuOpt to specify the
    next menu option (temp, time, and so on).
    Finally, when all menu options have been selected, startUp() is called,
    starting the incubation and heating the chamber.

    Returns: none
  */

  // Reads if the select button has been pressed.
  selectButtonState = digitalRead(selectButton);

  // Prints banner info about the current temperature and time settings.
  printBanner(temp_setting, temp_setting_prev, time_setting, time_setting_prev);

  if (selectButtonState == HIGH) { // pressing Down will increment menuopt by 1
    menuOpt = (menuOpt + 1) % 3; // incrsement the menu setting by 1
    //tft_screen.fillScreen(BLACK);
    tft_screen.fillRect(0, 40, SCREEN_WIDTH, 60, BLACK);
    delay(200); // delays so that select button won't register keypresses so quickly
  }
  if (menuOpt == 0) {
    setTemp(menuOpt);
  }
  else if (menuOpt == 1) {
    setTime(menuOpt);
  }
  else { // only entered if menuOpt is 2
    runningAvg[0] = temp_setting;
    runningAvg[1] = temp_setting;
    runningAvg[2] = temp_setting;
    runningAvg[3] = temp_setting;
    runningAvg[4] = temp_setting;

    startUp(); // Starts heating the incubation chamber
  }
}

void printBanner(int temp_setting, int temp_setting_prev, int time_setting, int time_setting_prev) {
  /*
    Prints the yellow text at the top of the screen. Shows the selected setting while powered on.
    Includes the temperature and duration settings for the incubation period.

    Returns: none
  */
  tft_screen.setCursor(0, 10);
  tft_screen.setTextColor(YELLOW);
  tft_screen.print("Curr Temp Set (C): "); // Temperature set prompt
  tft_screen.print(temp_setting);

  tft_screen.setCursor(0, 20);
  tft_screen.setTextColor(YELLOW);
  tft_screen.print("Curr Time Set (h): "); // Time set prompt
  tft_screen.print(time_setting);

}

void setTemp(int m) { // menu option to set temperature
  /*
    Sets the temperature for the next incubation period. Applied as a loop
    that reads Up/Down input at the beginning of each run.

    Returns: none
  */

  DownState = digitalRead(Down);
  UpState = digitalRead(Up);

  int temp_setting_prev = temp_setting;

  if ((DownState == HIGH) && (temp_setting <= MAX_TEMP_SET) && (temp_setting > MIN_TEMP_SET)) {
    temp_setting--; // decrement the temperature setting by 1, given within the acceptable range

  }
  if ((UpState == HIGH) && (temp_setting < MAX_TEMP_SET) && (temp_setting >= MIN_TEMP_SET)) {
    temp_setting++; // increment the temperature setting by 1, given within the acceptable range

  }

  if ((temp_setting != temp_setting_prev) || (firstTempSet == true)) {
    tft_screen.setCursor(0, 10); // this code blots out the yellow print at the top while the temperature selection is occurring
    tft_screen.setTextColor(YELLOW);
    tft_screen.print("Curr Temp Set (C): ");
    tft_screen.setTextColor(BLACK);
    tft_screen.print(temp_setting_prev);
    tft_screen.setCursor(0, 50); // display the selection in progress to the user.
    tft_screen.setTextColor(WHITE);
    tft_screen.println("Set temperature (C): ");
    tft_screen.setCursor(0, 60);
    tft_screen.setTextColor(BLACK);
    tft_screen.print(temp_setting_prev);
    tft_screen.setCursor(0, 60);
    tft_screen.setTextColor(MAGENTA);
    tft_screen.print(temp_setting);
  }

  //delay(250);
  firstTempSet = false;
}

void setTime(int m) {
  /*
    Activates the menu option to set the duration of incubation. Applied as a loop
    that reads Up/Down input at the beginning of each loop. Modifies

    Returns: none
  */

  DownState = digitalRead(Down);
  UpState = digitalRead(Up);

  int time_setting_prev = time_setting;

  if (DownState == HIGH) {
    time_setting--; // decrement the time setting by 1 hour
  }
  if (UpState == HIGH) {
    time_setting++; // increment the time setting by 1 hour
  }

  if ((time_setting != time_setting_prev) || (firstTimeSet == true)) {
    tft_screen.setCursor(0, 20); // this code blots out the yellow print at the top while the time selection is occurring
    tft_screen.setTextColor(YELLOW);
    tft_screen.print("Curr Time Set (h): ");
    tft_screen.setTextColor(BLACK);
    tft_screen.print(time_setting_prev);
    tft_screen.setCursor(0, 50); // display the selection in progress to the user.
    tft_screen.setTextColor(WHITE);
    tft_screen.println("Set time (hrs): ");
    tft_screen.setCursor(0, 60);
    tft_screen.setTextColor(BLACK);
    tft_screen.print(time_setting_prev);
    tft_screen.setCursor(0, 60);
    tft_screen.setTextColor(MAGENTA);
    tft_screen.print(time_setting);
  }

  firstTimeSet = false;
}

float readTemp() {
  /*
    Reads temperature value from a digital thermometer
    using a one-wire bus, and the DallasTemperature
    sensors library.

    Example Usage: https://github.com/milesburton/Arduino-Temperature-Control-Library/blob/master/examples/Simple/Simple.ino

    Returns: temp - floating point number representing temperature (Celsius)
  */

  // Serial.println("Reading digital thermometer...");
  sensors.requestTemperatures();
  // Serial.println("Got temperature(s)");
  float temp = sensors.getTempCByIndex(0);
  Serial.print("Temp: ");
  Serial.println(temp);
  return temp;
}

void writeTempToSD(int seconds, float temperature) {
  /*
    Writes a temperature to the SD card. Used
    to store many time and temperature values.
    Written to CSV in the format "seconds,temperature",
    where "seconds" is an integer and "temperature" is
    a float.

    Returns: none
  */

  temperatureLog = SD.open("temp_log.csv", FILE_WRITE);

  // if the temperatureLog file is available, write to it.
  if (temperatureLog) {
    temperatureLog.print(seconds);
    temperatureLog.print(",");
    temperatureLog.println(temperature);
    Serial.print("Value successfully written to SD card: ");
    Serial.print(seconds);
    Serial.print(", ");
    Serial.println(temperature);
  }
  else {
    Serial.println("Error opening temp_log.csv (SD card file)");
  }

  temperatureLog.close();

}

float readTempFromSD(int seconds) {
  /*
    Reads a temperature from termperature_log.csv, the SD
    card temperature log for incubation. Takes a parameter
    "seconds" that represents the time in seconds from the
    start of the incubation period (after pre-heating).

    Returns: T - temperature (in Celsius) of chamber at time "seconds"
  */

  temperatureLog = SD.open("temperature_log.csv", FILE_READ);

  if (temperatureLog) {
    // TODO find desired seconds value, then output temperature at that time.
  }
}

void startUp() {
  /*
    This is the function that is called immediately after all incubation settings
    have been selected. This function manages the initial heating of the chamber
    to the desired temperature, at which point startRun() is called.

    Returns: none
  */
  
  T = readTemp();

  while (T < temp_setting - 0.5) {
    tftPrintStartUp();
    dutyCycle(20000, 0.7); // sets duty cycle just for pre-heating phase. 
    tftEraseStartUp();
    T = readTemp();

    // Test Write to SD card for pre-heating phase
    writeTempToSD(-1, T); // writes temperature readings to SD card.
  }
  tftEraseStartUp();
  startRun();
}

void dutyCycle(float period, float onPercentage) {
  /*
    Controls the heating pad using a duty cycle. Takes inputs period and onPercentage.
    For each period (in seconds), the heating pad will be turned on for the percent
    fraction of that period. Note: onPercentage must be between 0 and 1.

    Returns: none
  */
  digitalWrite(transistor, HIGH);
  delay(int(period * onPercentage));
  digitalWrite(transistor, LOW);
  delay(int(period * (1 - onPercentage)));
}

void startRun() {
  /*
    Manages the incubation period after the startUp() function heats the
    chamber to the desired temperature. This function continues the heating
    of the incubation chamber for the full duration, and manages transistor control,
    the alarm system, and the display of some information.

    Returns: none
  */

  bool currentLoopHeating = false;
  int currentMillis;
  int currentSeconds;
  int currentHours = -1;
  int previousHours;
  int loopDuration = 0;
  uint32_t period = time_setting * 60 * 60000L; // 5 minutes

  for (uint32_t tStart = millis();  (millis() - tStart) < period;  ) { //https://arduino.stackexchange.com/questions/22272/how-do-i-run-a-loop-for-a-specific-amount-of-time

    previousHours = currentHours;
    currentMillis = millis() - tStart; // current time in milliseconds
    currentSeconds = currentMillis / 1000; // get current time in seconds
    currentHours = currentMillis / 3600000; // get current full hours elapsed


    // record temperature every hour to serial log for testing summary (if computer is connected during incubation)
    if (currentHours != previousHours) {
      //Serial.print("Time (hours): ");
      Serial.print("Time (hours): ");
      Serial.println(currentHours); // time in hours
      Serial.print("Current Temperature: ");
      Serial.println(T);
      Serial.print("Running Average: ");
      Serial.println(T_average);
      Serial.print("");
      Serial.println("˚C");
    }

    delay(500);
    // resets screen words to black (clears characters)
    // FIXME very unclear which print statements are writing and which are "erasing"
    printBanner(temp_setting, temp_setting_prev, time_setting, time_setting_prev);
    tft_screen.setCursor(0, 43);
    tft_screen.setTextColor(BLACK);
    tft_screen.print(T);
    tft_screen.setCursor(30, 51);
    tft_screen.setTextColor(BLACK); //blacks out the screen from the previous temperature measurement
    tft_screen.print("IN RANGE");
    tft_screen.setCursor(30, 51);
    tft_screen.setTextColor(BLACK);
    tft_screen.print("OUT OF RANGE");
    tft_screen.setCursor(0, 59);
    tft_screen.setTextColor(WHITE);
    tft_screen.println("Current Test Status:"); // information on whether the internal temperature is at the correct temp over a trend of time

    if (finalStatus == 1) { // build in the audio and visual alarms data here
      tft_screen.setCursor(0, 67);
      tft_screen.setTextColor(BLACK);
      tft_screen.print("Good");
      tft_screen.setCursor(0, 67);
      tft_screen.setTextColor(WHITE);
      tft_screen.print("Some Deviations");

      // trigger alarm by activating the LED and piezo
      digitalWrite(led, HIGH);
      //tone(piezo_alarm, 1000, 1000);
    }
    else {
      tft_screen.setCursor(0, 67);
      tft_screen.setTextColor(WHITE);
      tft_screen.print("Good");
    }

    T = readTemp(); // reads current temperature
    // Serial.print("Current Seconds of Incubation: ");
    // Serial.println(currentSeconds); // current seconds jump about 2 seconds each loop
    // Writes temperature to SD card occasionally
    if (currentSeconds % 30 == 0) {
      writeTempToSD(currentSeconds, T); // it will NOT write every 30 seconds, because the loop doesn't complete every second.
    }

    // tft_screen print function
    tftPrintTest();

    currentLoopHeating = onStatus; // recorded to calculate duty cycle (to add elapsed time of heating)

    // PID Control of Heating Pad
    therm_PID.Compute();
    Serial.print("PID Input (Temp):   ");
    Serial.println(*PID_input);
    Serial.print("PID Setpoint:       ");
    Serial.println(*PID_setpoint);
    Serial.print("PID Output (0-255): ");
    Serial.println(*PID_output); // FIXME output is always zero, regardless of parameters, input, and setpoint. I don't know why.
    transistorControlPID(heatingPadThreshold, *PID_output); // TODO measure time at each loop to calculate duty cycle.
    // FIXME as I've updated the PID controller stuff, I've started encountering instability,
    // such as the screen turning off or Serial information being incomplete. Likely using
    // too much storage space and/or dynamic memory of the Arduino.

    delay(1000); // originally 500

    loopDuration = (millis() - tStart) - currentMillis; // actual current time minus time at the start of loop.
    
    // Duty Cycle Recording
    if (currentLoopHeating) {
      elapsedMillisHeating += loopDuration; // adds length of previous loop
      overallDutyCycle = elapsedMillisHeating / currentMillis; // Uses currentMillis for convenience, though it will be a little imprecise.
    }

  }

  // Turns off transistor at the end of incubation.
  digitalWrite(transistor, LOW);
}

void transistorControlPID(float threshold, int output) {
  /*
    Controls the heating pad transistor by turning it either on
    or off, depending on the PID controller output. The threshold
    represents the value at which the heating pad will turn on, between
    0 and 1. Converts output (0-255) to a value between 0 and 1. Values
    closer to 255 indicate more heating

    Inputs: output - output of PID controller computation. Between 0 and 255
            threshold - float between 0 and 1.

    Returns: none
  */

  // checks if the output meets the threshold to turn on the heating pad.
  if (((float)output / 255.0) > threshold) {
    //turn on transistor, start power flow from battery to the heating pad if in range or below range
    digitalWrite(transistor, HIGH);
    if (onStatus == 0) {
      tft_screen.fillRect(67, 80, 128, 87, BLACK);
    }
    tft_screen.setCursor(0, 80);
    tft_screen.setTextColor(WHITE);
    tft_screen.setTextSize(0);
    tft_screen.println("HEATING PAD ON");
    onStatus = 1;
  }
  else {
    // turn off transistor, stop power flow from battery to the heating pad
    digitalWrite(transistor, LOW);
    if (onStatus == 1) {
      tft_screen.fillRect(67, 80, 128, 87, BLACK);
    }
    tft_screen.setCursor(0, 80);
    tft_screen.setTextColor(WHITE);
    tft_screen.setTextSize(0);
    tft_screen.println("HEATING PAD OFF");
    onStatus = 0;
  }
}

void tftPrintStartUp() {
  /*
    This method starts the tft_screen and prints the startup text.

    Returns: none
  */

  tft_screen.setCursor(0, 35);
  tft_screen.setTextColor(WHITE);
  tft_screen.setTextSize(0);
  tft_screen.println("Current Temperature:");
  tft_screen.setTextColor(RED);
  tft_screen.println(T);
  tft_screen.setCursor(0, 51);
  tft_screen.setTextColor(WHITE);
  tft_screen.setCursor(0, 51);
  tft_screen.print("PRE-HEATING");
  tft_screen.setCursor(0, 61);
  tft_screen.print("Please wait");
}

void tftEraseStartUp() {
  /*
    Erases the start-up text by overwriting it with BLACK text.
    
    Returns: none
  */
  tft_screen.setCursor(0, 35);
  tft_screen.setTextColor(BLACK);
  tft_screen.setTextSize(0);
  tft_screen.println("Current Temperature:");
  tft_screen.println(T);
  tft_screen.setCursor(0, 51);
  tft_screen.setCursor(0, 51);
  tft_screen.print("PRE-HEATING");
  tft_screen.setCursor(0, 61);
  tft_screen.print("Please wait");
}

void tftPrintTest() {
  /*
    Prints the test at the end of each startRun() loop. Describes the
    current status of temperatures of the incubator, and can also
    trigger the alarm system if the system has been out of range
    for too long.

    Returns: none
  */

  for (int x = 4; x > 0; x--) // max number is number of secs between each measurement
  {
    runningAvg[x] = runningAvg[x - 1];
  }
  runningAvg[0] = T;

  float sum = 0;
  for (int i = 0; i < 5; i++)
  {
    sum = sum + runningAvg[i];
  }
  T_average = sum / 5;

  tft_screen.setCursor(0, 35);
  tft_screen.setTextColor(WHITE);
  tft_screen.setTextSize(0);
  tft_screen.println("Current Temperature:");
  tft_screen.setTextColor(RED);
  tft_screen.println(T);
  tft_screen.setCursor(0, 51);
  //reads stored temperature and determines if the temperature falls within range
  tft_screen.setTextColor(WHITE);
  tft_screen.print("Test:");
  if (T_average > temp_setting - 2 && T_average < temp_setting + 2) { // this is if the temperature is in range
    out_range_counter = 0;
    tft_screen.setCursor(30, 51);
    tft_screen.print("IN RANGE");
    // finalStatus = 0;
  }
  else { // this is if the temperature is out of range
    out_range_counter++;
    tft_screen.setCursor(30, 51);
    tft_screen.print("OUT OF RANGE");
    if (out_range_counter >= 5) { // within 5 minutes, with a 15-second interval between measurements
      finalStatus = 1; // in startRun(), this is meant to trigger the alarm/notification system
    }
  }
}
