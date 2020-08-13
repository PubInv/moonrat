#include <SPI.h>
#include <Wire.h>
#include <EEPROM.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

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
#define BTN_UP 6
#define BTN_DOWN 7
#define BTN_SELECT 5
#define BZR_PIN 9

//temperature variables
#define TEMP_PIN A0  //This is the Arduino Pin that will read the sensor output
int sensorInput;    //The variable we will use to store the sensor input
int targetTemperature = 85;
int rawTemp = 0;
float temperature;        //The variable we will use to store temperature in degrees.
bool heating = false;

//graph variables
int graphTimeLength = 24;
int graphMaxTemp = 100;
int graphMinTemp = 60;

//menu variables
int menuSelection = 0;
bool inMenu = true;
bool up = false;
bool down = false;
bool select = false;

//eeprom variables
#define INDEX_ADDRESS 0 //location of index tracker in EEPROM
#define MAX_ADDRESS 1023 //highest possible address in arduino UNO EEPROM
#define MAX_SAMPLES 511 //maximum number of samples that we can store in EEPROM

//other variables
uint32_t milliTime = 0;
bool incubating = false;

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
  for(int i = 0; i < 128-LEFT_MARGIN; i++){
    int ypos = (int)(readIndex(i) - 50);
    if(ypos < 0)
      ypos = 0;
    display.drawPixel(LEFT_MARGIN + i, 64-ypos, SSD1306_WHITE);
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
  double temp = (double)raw / 1024;       //find percentage of input reading
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

  pinMode(BTN_UP, INPUT);
  pinMode(BTN_DOWN, INPUT);
  pinMode(BTN_SELECT, INPUT);
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

// LOOP --------------------------------------------------

//main
void loop() {
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
  }

  delay(250);
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
  }

  if(incubating){
    if(milliTime % 300000 == 0){ // 5 minutes
      writeNewEntry(temperature);
    }
    milliTime += 250;
    if(milliTime > 172800000){ //48 hours
      buzz();
    }
  }
}

// ISRs --------------------------------------------------

//runs at 8 Hz
ISR(TIMER1_COMPA_vect){
  rawTemp = analogRead(TEMP_PIN);    //read the analog sensor and store it
  //read buttons
  up = digitalRead(BTN_UP);
  down = digitalRead(BTN_DOWN);
  select = digitalRead(BTN_SELECT);
}
