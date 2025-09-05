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

#ifndef DISPLAY_H
#define DISPLAY_H 1

#include <Adafruit_SSD1306.h>


extern Adafruit_SSD1306 display;
extern float targetTemperatureC;
extern bool inMainMenu;
extern bool showingGraph;

void showCurStatus(float temp,int timeMax,int hours,int minutes);
void showReport(float temp);
void showSetTempMenu(float target);
void showSetTimeMenu(float incubationTime);

void showGraph(int eeindex);
void displayTempMenu(); 
void displayTimeMenu();


void showGraph(int eeindex);
void displayTempMenu(); 
void displayTimeMenu();
void setMaxTemp();
void displayExitScreen();

void showMenu(float CurrentTempC);

void setupDisplay();

#endif