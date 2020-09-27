#include <Adafruit_I2CDevice.h>
#include <Adafruit_I2CRegister.h>
#include <Adafruit_SPIDevice.h>
#include <Adafruit_BusIO_Register.h>

#include <Adafruit_GrayOLED.h>
#include <gfxfont.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SPITFT.h>
#include <Adafruit_SPITFT_Macros.h>

#include <Adafruit_SSD1306.h>
#include <splash.h>

// EEPROM for arc32 - Version: Latest 
#include <EEPROM.h>

//Why not use the SRAM for the zoomed in part and EEPROM FROM ZOOMED OUT PART
 /*
moonrat
*/
#include <SPI.h>
#include <Wire.h>

int a=0;//initialize index for EEPROM
int b=1;//initialize serial print values
uint16_t value1;//first two bits of eeprom storage
uint16_t value2;//last bits of EEprom 

//display variables
#define WIDTH 128
#define HEIGHT 64
#define SPLIT 16
#define LEFT_MARGIN 9
#define LINE_HEIGHT 9

#define OLED_RESET 4
Adafruit_SSD1306 display(WIDTH, HEIGHT, &Wire, OLED_RESET);

//pin variables
#define HEAT_PIN 8
//#define BTN_UP 6
//#define BTN_DOWN 7
//#define BTN_SELECT 5
#define BZR_PIN 9

//temperature variables
#define TEMP_PIN A0  //This is the Arduino Pin that will read the sensor output
int sensorInput;    //The variable we will use to store the sensor input
int targetTemperature = 85;//im fahrenheit
int rawTemp = 0;
float temperature;        //The variable we will use to store temperature in degrees.
bool heating = false;


//graph variables

int graphTimeLength = 24;//2 hours long
/*graphTimeLength2=511;*/
int graphMaxTemp = 90;
int graphMinTemp = 80;

//menu variables
int menuSelection = 0;
bool inMenu = true;
bool up ;
bool down ;
bool select ;
/* bool ZOOMIN= false;
 *  bool ZOOMOUT=true;
 */

//eeprom variables
#define INDEX_ADDRESS 0 //location of index tracker in EEPROM
#define MAX_ADDRESS 1023 //highest possible address in arduino UNO EEPROM
#define MAX_SAMPLES 511 //maximum number of samples that we can store in EEPROM

//other variables
uint32_t milliTime = 0;
bool incubating = false;
bool entryFlag = false;
int ticksSinceHeat = 0;

#define FIVE_MINUTES 300000
//#define TWO_HOURS 7200000
#define TICK_LENGTH 125

// DISPLAY FUNCTIONS --------------------------------------------------

void showNumber(float number){
  display.clearDisplay();
  display.setCursor(28,24);
  display.setTextSize(2);
  display.println(number);
  display.display();
}

/*
 * Shows a graph of the temperature data over a period of time
 */
 /*drawing graph from zero to 128. TODO : DATA IS COLLECTED EVERYFIVE MINUTES IT CAN COLLECT 511. FOR 48 HOURS.
 IDEALLY BE ABLE TO VIEW SPECIFIC SECTIONS. THE DATA IS STORED IN EEPROM. USE BUTTONS AND CREATE A MENU TO ZOOM IN TO THE GRAPH)*/
void showGraph(){
  display.clearDisplay();
  display.setTextSize(2);
  display.setCursor(0,0);
  display.println("GRAPH");
  display.setTextSize(1);
  display.setCursor(1, SPLIT);
  display.println("100");
  display.setCursor(1, 56);
  display.println("50");
  /*this is where the zoomed in part works. create a conditional statement that makes this run if if zoom in is clicked
  this should not be the default*/
  /*if (ZOOMIN){*/
  for(int i = 0; i < 128-LEFT_MARGIN; i++){
    int ypos = (int)(readIndex(i) - 50);
    //if graph min temp is 60, how come 50 is subtracted from the index
    if(ypos < 0)
      ypos = 0;
    display.drawPixel(LEFT_MARGIN + i, 64-ypos, SSD1306_WHITE);
  /*elseif (ZOOMOUT){
   * for(int i = 0; i < 128-LEFT_MARGIN; i++){
    int ypos = (int)(readIndex(i) - 50);//Maybe readindex2, read index after a longer while but using a di55
    if(ypos < 0)
      ypos = 0;
    display.drawPixel(LEFT_MARGIN + i, 64-ypos, SSD1306_WHITE);
  CREATE A WIDER RANGE FOR THE GRAPH */
  }
  //ask how the graph determines the number of plots to make, i.e what is LEFT_MARGIN so have a less detailed graph with a longer period 
  // 
  
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
  display.setCursor(64, 7);
  String currentTime = getTimeString();
  display.println(currentTime);
  //print options
  display.setTextSize(1);
  display.setCursor(LEFT_MARGIN,SPLIT);
  display.println("Temperature");
  display.setCursor(LEFT_MARGIN,SPLIT + LINE_HEIGHT);
  display.println("Graph");
  display.setCursor(LEFT_MARGIN,SPLIT + 2*LINE_HEIGHT);
  display.println("Set Temperature");
  display.setCursor(LEFT_MARGIN,SPLIT + 3*LINE_HEIGHT);
  /*Zoom into current time*/
  //display.println("Zoom in");
  //display.setCursor(LEFT_MARGIN,SPLIT + 4*LINE_HEIGHT);
   /* zoom out*/
 // display.println("Zoom out");
  //display.setCursor(LEFT_MARGIN,SPLIT + 5*LINE_HEIGHT);
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

void heatOn(){
  digitalWrite(HEAT_PIN, HIGH);
}

void heatOff(){
  digitalWrite(HEAT_PIN, LOW);
}

double convertTemp(int raw){
  double temp = (double)raw / 1024;       //find percentage of input reading ASK ABOUT THIS PART
  temp = temp * 5;                 //multiply by 5V to get voltage
  temp = temp - 0.5;               //Subtract the offset 
  temp = temp * 180 + 32 ;          //Convert to degrees
  temperature = (3*temperature + temp)/4;
  return temperature;
}

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

void buzz(){
  digitalWrite(BZR_PIN, HIGH);
}

uint16_t floatToSixteen(float flt){
  uint16_t out = round(flt*100);
  return out;
}

float sixteenToFloat(uint16_t sixteen){
  float flt = sixteen / 100.0;
  return flt;
}

// EEPROM FUNCTIONS --------------------------------------------------

/*
 * Writes 16 bits into EEPROM using big-endian respresentation
 */
void rom_write16(uint16_t address, uint16_t data){
  EEPROM.write(address, (data & 0xFF00) >> 8);
  EEPROM.write(address + 1, data & 0x00FF);
}

/*
 * Reads 16 bits from EEPROM using big-endian respresentation
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
float readIndex(int index){
  if(index > MAX_SAMPLES){
    return -1;
  }
  uint16_t bitData = rom_read16(index*2);
  return sixteenToFloat(bitData);
}

// SETUP FUNCTIONS --------------------------------------------------

/*
 * Starts interrupts
 * Timer1 runs at 8 Hz
 */
void startInterrupts(){
  noInterrupts();
  
  //set timer1 interrupt at 8Hz
  TCCR1A = 0;// set entire TCCR1A register to 0
  TCCR1B = 0;// same for TCCR1B
  TCNT1  = 0;//initialize counter value to 0
  // set compare match register for 8hz increments
  OCR1A = 1952;// = (16*10^6) / (8*1024) - 1
  // turn on CTC mode
  TCCR1B |= (1 << WGM12);
  // Set CS12 and CS10 bits for 1024 prescaler
  TCCR1B |= (1 << CS12) | (1 << CS10);  
  // enable timer compare interrupt
  TIMSK1 |= (1 << OCIE1A);

  interrupts();
}

//setup

void setup() {
     // put your setup code here, to run once:
  Serial.begin(9600); //Start the Serial Port at 9600 baud (default)
  while (! Serial); // Wait untilSerial is ready - Leonardo
  Serial.println("Enter u, d, s");

//buttons defined earlier
//read serial port
  //pinMode(BTN_UP, INPUT);
  //pinMode(BTN_DOWN, INPUT);
  //pinMode(BTN_SELECT, INPUT);
  pinMode(BZR_PIN, OUTPUT);
  digitalWrite(BZR_PIN, LOW);
  pinMode(HEAT_PIN, OUTPUT); 
  heatOff();

  rom_reset();

  startInterrupts();

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)){
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}
// Loop
  
//main
void loop() {
  //read buttons
  char T;

    if (Serial.available()){
      T=Serial.read(); //getting string input in varaible "T" 
      
      
      up= (T=='u');
      down=(T=='d');
      select=(T=='s');
    }  
  

  /*
  convertTemp(rawTemp);
  //toggle heat if incubator is running
  if(incubating){
    if(!heating && temperature < targetTemperature - .25){
      heating = true;
    }
    else if(heating && temperature > targetTemperature + .25){
      heating = false;
    }
  }
  else{
    heating = false;
  }
  if(heating){
    heatOn();
  }else{
    heatOff();
  }*/

  delay(1000);
  convertTemp(rawTemp);
  //if temp below target turn heat on
  //if temp above target + gap turn heat off
  if(incubating){
    if(temperature > targetTemperature + 0.5){
      heatOn();
      ticksSinceHeat = 0;
    }
    else if(temperature < targetTemperature){
      heatOff();
    }
  }
  else{
    heatOff();
  }

  //prevents battery from turning off
  if(ticksSinceHeat*TICK_LENGTH > 1000){
    heatOn();
    delay(100);
    heatOff();
    ticksSinceHeat = 0;
  }
  
 
  if(inMenu){
    //read buttons and menu
    if(up && menuSelection > 0){
      menuSelection--;
    }
    else if(down && menuSelection < 3){
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
    if(menuSelection == 0){
      showNumber(temperature);
    }
    //show graph
    else if(menuSelection == 1){
      showGraph();
    }
    //set target temperature option
    else if(menuSelection == 2){
      showNumber(targetTemperature);
      if(up){
        targetTemperature++;
      }
      if(down){
        targetTemperature--;
      }
    }
    //toggle incubation
    else if(menuSelection == 3){
      incubating = !incubating;
      milliTime = 0;

      inMenu = true;
    }
    /*else if(menuSelection == 4{
       ZOOMIN=true;
       ZOOMOUT=false}*/
    /*else if(menuSelection == 5{
       ZOOMOUT=true;
       ZOOMIN=false;}*/
    
 }

  //store temperature when flag is set by ISR
  if(entryFlag){
    entryFlag = false;
    writeNewEntry(temperature);
    
    value1= EEPROM.read(a);
    value2 = EEPROM.read(a+1);
    float value22= value1+(sixteenToFloat(value2));
    //float totalvalue= sixteenToFloat(value22);
      if (b<513){

        Serial.print(b);
        Serial.print("\t");
        Serial.print(value22);
        Serial.println();
        b=b+1;
      }
      

      a = a + 2;
  }
}

    
    /*else if(menuSelection == 4{
       ZOOMIN=true;
       ZOOMOUT=false*/
    /*else if(menuSelection == 5{
       ZOOMOUT=true;
       ZOOMIN=false;*/


// ISRs --------------------------------------------------

//runs at 8 Hz
ISR(TIMER1_COMPA_vect){
  rawTemp = analogRead(TEMP_PIN);    //read the analog sensor and store it
  
  //up = digitalRead(BTN_UP);
  //down = digitalRead(BTN_DOWN);
  //select = digitalRead(BTN_SELECT);
  if(incubating/*&& ZOOMIN*/){
    milliTime += TICK_LENGTH;
    if(milliTime % FIVE_MINUTES == 0){ // 5 minutes
      entryFlag = true;
    }
    
    ticksSinceHeat += 1;
  }
  /*else if(incubating && ZOOMOUT){
   * CHANGE TIME FOR STORING ANALOG SENSOR VALUES TO 2 HOURS
    milliTime += TICK_LENGTH;
    if(milliTime % TWO_HOURS == 0){ // 5 minutes
      entryFlag = true;*/
}
