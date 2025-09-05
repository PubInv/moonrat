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

#include "Display.h"

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "Persistence.h"
#include "utility.h"

extern int selectedOption;
extern int tempMax;
extern int timeMax;
extern int menuSelection;

extern uint32_t time_heater_turned_on_ms;


//display variables
#define WIDTH 128
#define HEIGHT 64
#define SPLIT 16  // location of the graphing area (vertially, down from the top)
#define LEFT_MARGIN 9
#define LINE_HEIGHT 9
// #define OLED_RESET 4

//OLED VARIABLES
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// graph stuff
int center = SPLIT + ((64 - SPLIT) / 2);  // (of ex axis?)
int graphwidth = 128 - LEFT_MARGIN;
int xaxisleft = LEFT_MARGIN * 2;
int xaxismid = LEFT_MARGIN + (graphwidth / 2) - 4;
int xaxisright = 128 - 24;



void showCurStatus(float temp,int timeMax,int hours,int minutes) {
  float time_remaining_hrs = (float) timeMax - ((float)hours + minutes/ 60.0);
  display.clearDisplay();  //removes current plots
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print(F("T (C):"));
  display.println(temp);
  display.print(F("Watt Hrs:"));
  float avg_watts = 0.0;
  float wh = wattHours(avg_watts);
  display.println(wh);
  display.print(F("Amp  Hrs:"));
  display.println(ampHours(wh));
  display.println(F("Hours Remaining:"));
  display.println(time_remaining_hrs);
  display.display();
}

void showReport(float temp) {
  Serial.print(F("Target (C) : "));
  Serial.print(targetTemperatureC);
  Serial.print(F(" "));
  Serial.print(F("Temp : "));
  Serial.println(temp);
  Serial.print("Hours Incubated:");
  Serial.println(MS_TO_HOURS * time_heater_turned_on_ms);
  Serial.print(F("Watt Hours: "));
  float avg_watts = 0.0;
  float watt_hrs = wattHours(avg_watts);
  Serial.println(watt_hrs);
  Serial.print(F("Average Watts: "));
  Serial.println(avg_watts);
  Serial.print(F("Amp  Hours: "));
  Serial.println(ampHours(watt_hrs));
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


void showSetTimeMenu(float incubationTime) {
  display.clearDisplay();  //removes current plots
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.println(F("Up (U) to increase,"));
  display.println(F("Dn (D) to decrease,"));
  display.println(F("Incubation Time"));
  display.print(F("(Hrs): "));
  display.println(incubationTime);
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
  float time_graph_start_hrs = 0;  
  // This is the time of the midpoint measure in hours....                                                               //(xaxisright-xaxisleft)*FIVE_MINUTES;
  float timemid_hrs = (((float)(num_samples_drawn) / 2) * DATA_RECORD_PERIOD) / 3600000;  //(xaxisright-xaxismid)*FIVE_MINUTES;

  // Redoing this math...
  float time_shown_hrs =  ((float)(num_samples_drawn)*DATA_RECORD_PERIOD) / 3600000;

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
  display.println(time_graph_start_hrs);
  display.setCursor(xaxismid, 56);
  display.println(timemid_hrs);
  display.setCursor(xaxisright, 56);
  display.println(time_shown_hrs);

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

/* Displays the menu.

   Menu Options:
   - Show Temperature : switches to temperature display mode
   - Show Graph : switches to graph dispay mode
   - Start/Stop : starts or stops data collection
   - Set Target : sets the target incubation temperature
   - Export : send temperature data through serial port to a pc
   - Reset : resets temperature data
*/
void showMenu(float CurrentTempC) {
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
  display.print(CurrentTempC);
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
  display.println(F("Status"));
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
  // Set the Time
  display.println(F("Set Time"));
  display.setCursor(LEFT_MARGIN, SPLIT + 4 * LINE_HEIGHT);
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
  showingGraph = false;
}

void setupDisplay() {
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
}
