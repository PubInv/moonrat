//  Persistence.h
//  Copyright (C) 2024 Robert L. Read
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

#ifndef PERSISTENCE_H
#define PERSISTENCE_H 1

extern unsigned long BASE_DATA_RECORD_PERIOD;
extern unsigned long DATA_RECORD_PERIOD;
extern bool incubating;

uint16_t getIndex();

// EEPROM FUNCTIONS --------------------------------------------------
// creates a whole number
uint16_t floatToSixteen(float flt);
// creates a decimal
float sixteenToFloat(uint16_t sixteen);

/*
  Writes 16 bits into EEPROM using big-endian respresentation. Will be called to enter temp at a specific address every 5 mibutes
*/
void rom_write16(uint16_t address, uint16_t data);

/*
  Reads 16 bits from EEPROM using big-endian respresentation. Will be called in readindex to get temp at certain index in order to plot graph
*/
uint16_t rom_read16(uint16_t address);
/*
  Marks the EEPROM as empty by clearing the first address
  Note: Data is not actually cleared from other addresses
*/
void rom_reset();

//gets the contents of the EEPROM at each indec
float readIndex(int index);

uint16_t getIndex();

// here we dump to the serial port for debugging and checking
void dumpData();

// UTILITY FUNCTIONS --------------------------------------------------
void incubationON();
void incubationOFF();
uint32_t time_incubating();
uint32_t time_heating();
float duty_factor();
//turn the heating pad on
// void heatOFF();
// void heatON();
void setHeatPWM_fraction(double intended_df);
void getTimeString(char* buff);
bool writeNewEntry(float data);
float getTargetTemp();
void setTargetTemp(float temp);

float wattHours(float& average_watts);

#endif
