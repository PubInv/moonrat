#define NCHAN_SHIELD 1
// #define RICE_PETRI_FI 2
// #define HALIMAT_VERSION 3
 
#define LOG_VERBOSE 5
#define LOG_DEBUG 4
#define LOG_WARNING 3
#define LOG_MAJOR 2
#define LOG_ERROR 1
#define LOG_PANIC 0
#define LOG_LEVEL 4

#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_BusIO_Register.h>
#include <stdio.h>
#include <math.h>
// Note: This is not currently in use!!!
// #include <Adafruit_GrayOLED.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>
#include <Adafruit_SSD1306.h>
#include <splash.h>
// EEPROM for arc32 - Version: Latest 
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
// For our temperature sensor

// This hardware uses a "onewire" digital temperature sensor
#ifdef NCHAN_SHIELD

#define INPUT_PIN A0
#include <OneWire.h>
#include <DallasTemperature.h>
OneWire oneWire(INPUT_PIN);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
#endif






//display variables
#define WIDTH 128
#define HEIGHT 64
#define SPLIT 16 //location of x axis
#define LEFT_MARGIN 9
#define LINE_HEIGHT 9
#define OLED_RESET 4
Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, OLED_RESET);

//pin variables
#define HEAT_PIN 8  
#define BTN_SELECT 5
#define BTN_UP 6
#define BTN_DOWN 7

#define BZR_PIN 9

//temperature variables
#define TEMP_PIN A0  //This is the Arduino Pin that will read the sensor output
int sensorInput;    //The variable we will use to store the sensor input

#define USE_DEBUGGING_TARGET 
#ifdef USE_DEBUGGING_TARGET
int targetTemperatureC = 30;// Celcius
#else
int targetTemperatureC = 35;// Celcius
#endif


int rawTemp = 0;
float temperatureC;        //The variable we will use to store temperature in degrees.
bool heating = false;

// This is switched ONLY when turning heater on or off.
bool currently_heating = false;
uint32_t time_heater_turned_on_ms = 0;
uint32_t time_spent_heating_ms = 0;
uint32_t time_spent_incubating = 0;
uint32_t time_of_last_entry = 0;

#define DATA_RECORD_PERIOD 5*60*1000
//time variables
#define FIVE_MINUTES 300000

#define FORTYEIGHT_HOURS 172860000 //forty eight hours
#define TICK_LENGTH 125 
//graph variables
int graphTimeLength = 24;//2 hours long bexause plotting every 5 mins
/*graphTimeLength2=511;*/
int graphMaxTemp = targetTemperatureC+2; //initializing max temp
int graphMinTemp = targetTemperatureC-2; //initializing min temp
int graphMaxTemp1 = targetTemperatureC+20; //initializing max temp
int graphMinTemp1 = targetTemperatureC-20; //initializing min temp
int maxtarget= targetTemperatureC+2; //max range for target temp
int mintarget= targetTemperatureC-2; //min range for target temp
int center= SPLIT+ ((64-SPLIT)/2);
int graphwidth= 128-LEFT_MARGIN;
int xaxisleft=LEFT_MARGIN*2;
int xaxismid=LEFT_MARGIN+(graphwidth/2)-4;
int xaxisright=128-24;
float timeleft=0;//(xaxisright-xaxisleft)*FIVE_MINUTES;
float timemid=(((float)(graphwidth)/2)*FIVE_MINUTES)/3600000;//(xaxisright-xaxismid)*FIVE_MINUTES;
float timeright=((float)(graphwidth)*FIVE_MINUTES)/3600000;//xaxisright*FIVE_MINUTES;
//int countery=0;
//int timerpulse=0;

//menu variables
int menuSelection = 0;
bool inMenu = true;
bool up ;
bool down ;
bool select ;

const int BAUD_RATE = 9600;

//eeprom variables
#define INDEX_ADDRESS 0 //location of index tracker in EEPROM
#define MAX_ADDRESS 1023 //highest possible address in arduino UNO EEPROM
#define MAX_SAMPLES 511 //maximum number of samples that we can store in EEPROM
uint16_t value1;//first two bits of eeprom storage
uint16_t value2;//last bits of EEprom 

//other variables
uint32_t milliTime = 0;
uint32_t heatTime = 0;
bool incubating = true;
bool entryFlag = false; //set to true when data should be put in EEPROM ie every 5 minutes
int ticksSinceHeat = 0;
int b=1;//initialize serial print index


//gets the contents of the EEPROM at each indec
float readIndex(int index){
  if(index > MAX_SAMPLES){
    return -1;
  }
  uint16_t bitData = rom_read16(index*2);
  return sixteenToFloat(bitData);
}

// DISPLAY FUNCTIONS --------------------------------------------------
//dispays the current temperature i.e. when menu option 'temparature' is selected
void showNumber(float number){
  display.clearDisplay(); //removes current plots
  display.setCursor(28,24);
  display.setTextSize(2);
  display.println(number);
  display.display();
}

/*
 * Shows a graph of the temperature data over a period of time
 */

void showGraph(int eeindex){
  //TODO find out have to shift x axis values
  //Graph 
  if (eeindex>graphwidth){
 
    timeleft=timeleft+0.08333333333333;
    timemid=timemid+0.0833333333333;
    timeright=timeright+0.0833333333333;
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("GRAPH");
  display.setTextSize(1);
  display.setCursor(1, SPLIT);
  display.println(graphMaxTemp);
  display.setCursor(1, 56);
  display.println(graphMinTemp);
  display.setCursor(LEFT_MARGIN*2, 56);
  display.println(timeleft);
  display.setCursor(xaxismid, 56);
  display.println(timemid);
  display.setCursor(xaxisright, 56);
  display.println(timeright);
  int diff1;
  int diff2;
  int scale;
  int i=0;
  int j=0;
  // index begins at the end minus graph width
  if (eeindex>graphwidth){
    i=eeindex-graphwidth;
  }
  //plots 119 points fron the EEPROM Contents at a period of 5 minutes
  for(; i < eeindex; i++){
    float currenttemp=readIndex(i);
    int oldmaxtemp= graphMaxTemp;
    int oldmintemp= graphMinTemp;
    //changes max/min temp depending on current temp
    if (currenttemp>50){ //so that empty eeprom vals are not entered
      if (currenttemp>graphMaxTemp){  
        graphMaxTemp=ceil(currenttemp);
        display.setCursor(1, SPLIT);
        display.println(graphMaxTemp);
      }
      else if (currenttemp<graphMinTemp){
        graphMinTemp=floor(currenttemp)-1;
        display.setTextSize(1);
        display.setCursor(1, 56);
        display.println(graphMinTemp);
        //Serial.println(readIndex(i));
        
      }
    }
    diff1=graphMaxTemp-oldmaxtemp;  
    diff2= oldmintemp-graphMinTemp;
   
    
    int ypos = (int)(currenttemp - graphMinTemp);//subtract y coordinate from value of x axis
    scale= (int)((64-SPLIT)/(graphMaxTemp-graphMinTemp)); //creates scale y axis pixels
    //Serial.println(int(readIndex(i)));
    int middle= graphMinTemp +((graphMaxTemp-graphMinTemp)/2);
    //display.setCursor(1, center);
    //display.println(middle);
    if(ypos < 0)
      ypos = 0;
    
    display.drawPixel(LEFT_MARGIN + j,(64-(scale*ypos)), SSD1306_WHITE);//plots temparature
    //display.drawPixel(LEFT_MARGIN + i,(16+(diff1*scale)), SSD1306_WHITE); //draws horizontal line at max target, 2 degrees below target temp
    //display.drawLine(LEFT_MARGIN + i,(64-(diff2*scale)), SSD1306_WHITE);//draws horizontal line at min target, 2 degrees above target temp
    //display.drawFastHLine(LEFT_MARGIN,(64-(diff2*scale)),128-LEFT_MARGIN, SSD1306_WHITE);
    j+=1;
  }
  
  display.display();
}


//create another graph function for all EEPROM contents
void showGraph2(int b, int multiple){
  
  if (b>graphwidth){
 
    timeleft=timeleft+(multiple*0.08333333333333);
    timemid=timemid+(multiple*0.0833333333333);
    timeright=timeright+(multiple*0.0833333333333);
  }
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("GRAPH");
  display.setTextSize(1);
  display.setCursor(1, SPLIT);
  display.println(graphMaxTemp1);
  display.setCursor(1, 56);
  display.println(graphMinTemp1);
  display.setCursor(1, center);
  display.println(targetTemperatureC);
  display.setCursor(LEFT_MARGIN*2, 56);
  display.println(timeleft);
  display.setCursor(xaxismid, 56);
  display.println(timemid);
  display.setCursor(xaxisright, 56);
  display.println(timeright);
 
  //plots 119 points fron the EEPROM Contents at a period of ceil(b/119)*5 minutes
  for(int i = 0; i <(graphwidth) ; i++){
    float currenttemp=readIndex(i);
    if ((b<(graphwidth))&&(floor(currenttemp)>0)){
     
      //plot one value
      int ypos = (int)(currenttemp - graphMinTemp1);//subtract y coordinate from value of x axis
     

      if(ypos < 0){
        ypos = 0;
      }
      //make a loop from center to the y value
    
      display.drawLine(LEFT_MARGIN + i,center,LEFT_MARGIN + i,64-(ypos+8), SSD1306_WHITE);//plots temparature
      
    }
    else if ((b>=(graphwidth))&&(floor(currenttemp)>0)){
      
      //loop through the values in width multiple and find the min and max
      int maxval;
      int minval;
      int newi=multiple*i;
      
      for(int j=0;j<multiple-1;j++){
        maxval=max(readIndex(newi+j),readIndex(newi+j+1));
        minval=min(readIndex(newi+j),readIndex(newi+j+1));
      }
     
      int ypos = (int)(maxval - graphMinTemp1);//subtract max y coordinate from value of x axis
      int ypos1 = (int)(minval - graphMinTemp1);//subtract min y coordinate from value of x axis
      
      
      if(ypos < 0)
        ypos = 0;
      if(ypos1 < 0)
        ypos1 = 0;
      if ((maxval>0)&&(minval>0)){
        display.drawLine(LEFT_MARGIN + i,64-(ypos1+8),LEFT_MARGIN + i,64-(ypos+8), SSD1306_WHITE);//plots temparature
      }
    }
    
    
  }
    
  
  display.display();
}
/* Displays the menu.
 *  
 * Menu Options:
 * - Show Temperature : switches to temperature display mode
 * - Show Graph : switches to graph dispay mode
 * - Start/Stop : starts or stops data collection
 * - Set Target : sets the target incubation temperature
 * - Export : send temperature data through serial port to a pc
 * - Reset : resets temperature data
 */
void showMenu(){
  display.setTextColor(SSD1306_WHITE);
  display.clearDisplay();
  //print title
  display.setCursor(0,0);
  display.setTextSize(2);
  display.print("Menu");
  display.setTextSize(1);
  //displays time since incubation started
  display.setCursor(64, 7);
  String currentTime = getTimeString();
  display.println(currentTime);
  //menu option for current temperature
  
  display.setTextSize(1);
  display.setCursor(LEFT_MARGIN,SPLIT);
  //menu option for current temperature
  display.println("Temperature");
  display.setCursor(LEFT_MARGIN,SPLIT + LINE_HEIGHT);
  //menu option for detailed graph
  display.println("Graph 1");
  display.setCursor(LEFT_MARGIN,SPLIT + 2*LINE_HEIGHT);
  //menu option for graph over 48 hours
  display.println("Graph 2");
  display.setCursor(LEFT_MARGIN,SPLIT + 3*LINE_HEIGHT);
  //menu option for entering the target temparature. Entered by clicking the up/down buttons
  display.println("Set Temperature");
  display.setCursor(LEFT_MARGIN,SPLIT + 4*LINE_HEIGHT);
 
  if(incubating){
    display.println("Stop");
  }else{
    display.println("Start");   
  }
  //print cursor
  display.setCursor(0,SPLIT + menuSelection*LINE_HEIGHT);
  display.fillCircle(3, SPLIT + menuSelection*LINE_HEIGHT + 3, 3, SSD1306_WHITE);
  
  display.display();
}

// UTILITY FUNCTIONS --------------------------------------------------
//turn the heating pad on
void heatsOFF(){
  if (currently_heating) {
//  digitalWrite(HEAT_PIN, HIGH);
#ifdef NCHAN_SHIELD
  digitalWrite(HEAT_PIN, LOW);
#else
  Serial.println(F("not implemented heatsOFF")); 
#endif
  if (LOG_LEVEL >= LOG_DEBUG) {
    Serial.println(F("HEAT OFF!"));
  }
  currently_heating = false;
  uint32_t time_now = millis();
  time_spent_heating_ms = time_now - time_heater_turned_on_ms;
  
  }
}
void heatsON(){
  if (!currently_heating) {
//  digitalWrite(HEAT_PIN, LOW);
#ifdef NCHAN_SHIELD
  digitalWrite(HEAT_PIN, HIGH); 
#else
  Serial.println(F("not implemented heatsOFF")); 
#endif
    if (LOG_LEVEL >= LOG_DEBUG) {
      Serial.println(F("HEAT ON!"));
    }
    time_heater_turned_on_ms = millis();
    currently_heating = true; 
  }
}

//// convert temparature sensor data to fareinheit
//// WARNING: I don't know what sensor this is for! It is not the TMP36!
//double convertTemp(int raw){
//  double temp = (double)raw / 1024;       //find percentage of input reading 
//  temp = temp * 5;                 //multiply by 5V to get voltage
//  temp = temp - 0.5;               //Subtract the offset 
//  temp = temp * 180 + 32 ;          //Convert to degrees
//  temperature = (3*temperature + temp)/4;
//  return temperature;
//}
//function to display current time
String getTimeString(){
  uint32_t mils = milliTime;
  uint8_t hours = mils / 3600000;
  mils = mils % 3600000;
  uint8_t mins = mils / 60000;
  mils = mils % 60000;
  uint8_t secs = mils / 1000;
  mils = mils % 1000;
  String timeString = "";
  timeString.concat(hours);
  timeString.concat(":");
  timeString.concat(mins);
  timeString.concat(":");
  timeString.concat(secs);
  timeString.concat(".");
  timeString.concat(mils);
  return timeString;
}
//Turn on buzzer at an interval of 20 seconds
void buzz(){
  digitalWrite(BZR_PIN, HIGH);
  delay(20);
}
//creates a whole number
uint16_t floatToSixteen(float flt){
  uint16_t out = round(flt*100);
  return out;
}
//creates a decimal
float sixteenToFloat(uint16_t sixteen){
  float flt = sixteen / 100.0;
  return flt;
}

// EEPROM FUNCTIONS --------------------------------------------------

/*
 * Writes 16 bits into EEPROM using big-endian respresentation. Will be called to enter temp at a specific address every 5 mibutes
 */
void rom_write16(uint16_t address, uint16_t data){
  EEPROM.write(address, (data & 0xFF00) >> 8);
  EEPROM.write(address + 1, data & 0x00FF);
}

/*
 * Reads 16 bits from EEPROM using big-endian respresentation. Will be called in readindex to get temp at certain index in order to plot graph
 */
uint16_t rom_read16(uint16_t address){
  uint16_t data = EEPROM.read(address) << 8;
  data += EEPROM.read(address + 1);
  return data;
}
/*
 * Marks the EEPROM as empty by clearing the first address
 * Note: Data is not actually cleared from other addresses
 */
void rom_reset(){
  rom_write16(INDEX_ADDRESS,0);
}

/*
 * Writes a new entry into the next spot in EEPROM
 * returns false if the list is full
 */
bool writeNewEntry(float data){
  uint16_t bitData = floatToSixteen(data);
  uint16_t index = rom_read16(INDEX_ADDRESS);
  if(index >= MAX_SAMPLES){
    return false;
  }
  index = index + 1;
  rom_write16(2*index, bitData);
  rom_write16(INDEX_ADDRESS, index);
  return true;
}

/*
 * Reads the data at a given index, first index is 1
 * Returns -1 if the index is invalid
 */
/*#define ARRAY_LENGTH 5
uint16_t arra[ARRAY_LENGTH];
//void createarray(){
    for(int i=0; i<ARRAY_LENGTH; i++){
      arra[i]=i;
    }
}*/
/*float readIndex2(int i){
  return ((i%50)+50);
 
}*/
  

// SETUP FUNCTIONS --------------------------------------------------

/*
 * Starts interrupts
 * Timer1 runs at 8 Hz
 */

//void startInterrupts(){
//  noInterrupts();
//  
//  //set timer1 interrupt at 8Hz
//  TCCR1A = 0;// set entire TCCR1A register to 0
//  TCCR1B = 0;// same for TCCR1B
//  TCNT1  = 0;//initialize counter value to 0
//  // set compare match register for 8hz increments
//  OCR1A = 1952;// = (16*10^6) / (8*1024) - 1
//  // turn on CTC mode
//  TCCR1B |= (1 << WGM12);
//  // Set CS12 and CS10 bits for 1024 prescaler
//  TCCR1B |= (1 << CS12) | (1 << CS10);  
//  // enable timer compare interrupt
//  TIMSK1 |= (1 << OCIE1A);
//
//  interrupts();
//}

//setup

void setup() {
     // put your setup code here, to run once:
  Serial.begin(BAUD_RATE); //Start the Serial Port at 9600 baud (default)
  delay(500);
//  while (! Serial); // Wait untilSerial is ready
  Serial.println(F("Enter u, d, s"));

  //createarray();

  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_SELECT, INPUT);
  
  pinMode(BZR_PIN, OUTPUT);
  digitalWrite(BZR_PIN, LOW);
  pinMode(HEAT_PIN, OUTPUT); 
 // heatsON();

  rom_reset();

//  startInterrupts();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);

#ifdef NCHAN_SHIELD
  pinMode(INPUT_PIN, INPUT);
  pinMode(A0, INPUT_PULLUP);
  
#endif
 
}
// Loop


/*
 * Switch test program
 */
#ifndef NCHAN_SHIELD
double read_temp() {
  double sensorInput = analogRead(A0);        //read the analog sensor and store it
  double temp = (double)sensorInput / (double) 1024.0;   //find percentage of input reading
  double voltage = temp * 5.0;  //multiply by 5V to get voltage
  Serial.print(F("voltage: "));
  Serial.println(voltage);
  double offsetVoltage = voltage - 0.5;                   //Subtract the offset 
  double degreesC = offsetVoltage * 100.0;                   //Convert to degrees C
  return degreesC;
}
#endif

#ifdef NCHAN_SHIELD
double read_temp() {
  sensors.requestTemperatures(); // Send the command to get temperatures
  // After we got the temperatures, we can print them here.
  // We use the function ByIndex, and as an example get the temperature from the first sensor only.
  float tempC = sensors.getTempCByIndex(0);

  // Check if reading was successful
  if(tempC != DEVICE_DISCONNECTED_C) 
  {
    Serial.print(F("Temperature for the device 1 (index 0) is: "));
    Serial.println(tempC);
  } 
  else
  {
    Serial.println(F("Error: Could not read temperature data"));
  }
  return tempC;
}
#endif

//main
// NOTE: I now 

void loop() {

// Test reading the buttons...
  if (LOG_LEVEL >= LOG_DEBUG) {
    Serial.print(F("Incubating: "));   
    Serial.println(incubating);
    Serial.print(F("Currently Heating: "));   
    Serial.println(currently_heating);
  }
  
  //read keyboard entries from the serial monitor
  char T;
  if (Serial.available()){
    T=Serial.read(); //getting string input in varaible "T" 
      
    Serial.print(F("T ="));
    Serial.println(T);  
    up= (T=='u');
    down=(T=='d');
    select=(T=='s');
  }  

  select = digitalRead(BTN_SELECT);
  up = digitalRead(BTN_UP);
  down = digitalRead(BTN_DOWN);

  if (LOG_LEVEL >= LOG_DEBUG) {
    Serial.print(select);   
    Serial.print(up);
    Serial.print(down);
    Serial.println();
  }

  temperatureC = read_temp();
  if (LOG_LEVEL >= LOG_DEBUG) {
    Serial.print(F("Temp (C): "));
    Serial.println(temperatureC);
    Serial.print(F("Time spent heating (ms):"));
    Serial.println(time_spent_heating_ms);
   }
  
  int numEEPROM=0;
  int multiple=0;
  //store temperature when flag is set by ISR every five minutes
  uint32_t time_now = millis();
  uint32_t time_since_last_entry = time_now - time_of_last_entry;
  if(time_since_last_entry < DATA_RECORD_PERIOD){
  //  entryFlag = false;
    writeNewEntry(temperatureC);
    time_of_last_entry = time_now;
  }

//  delay(1000);
//  convertTemp(rawTemp);
  //if temp below target turn heat on
  //if temp above target + gap turn heat off
  //if the incubating has started then start heating if the temperature is too low. 
  if(incubating){
    if(temperatureC > targetTemperatureC + 0.5){
      heatsOFF();
    }
    else if(temperatureC < targetTemperatureC - 0.5){
      heatsON();
    } else { // no change
      
    }
  }
  else{
    heatsOFF();
  }

  
 //controls menu selection
  if(inMenu){
    //read buttons and menu
    if(up && menuSelection > 0){
      menuSelection--;
    }
    else if(down && menuSelection < 4){
      menuSelection++;
    }
    else if(select){
      inMenu = false;
    }
    showMenu();
  }
  else{
    //return to menu on select
    if(select){
      inMenu = true;
    }
    //show temperature option
    switch(menuSelection){
      case 0 :
        showNumber(temperatureC);
        break;
      
      //show graph1
      case 1 :
        showGraph(b);
        break;

      //show graph 2
      case 2 :
        showGraph2(b,multiple);
        break;
      
      //set target temperature option
      case 3 :
        showNumber(targetTemperatureC);
        if(up){
          targetTemperatureC++;
        }
        if(down){
          targetTemperatureC--;
        }
        break;
      
      //toggle incubation
      case 4 :
        incubating = !incubating;
        milliTime = 0;
  
        inMenu = true;
        break;
      
    }
 }
 //countery=countery+1;
 //timerpulse=timerpulse+digitalRead(HEAT_PIN);
 //start buzzing after 48 hours until the user stops incubating
 int timern= ceil(milliTime/1000);
 if (incubating && (timern% 60==0)){
  //buzz();
  //Serial.println(float(timerpulse/countery));
  Serial.println(heatTime);
  
 }
 delay(100);
}
