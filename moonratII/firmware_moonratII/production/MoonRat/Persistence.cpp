//  Moonrat Control Code - Persistence.cpp
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

#include <stdio.h>
#include <EEPROM.h>
#include "utility.h"

extern int LOG_DEBUG;
extern int LOG_LEVEL;
extern int HEATER_PIN;


extern int LOG_VERBOSE;
extern int LOG_DEBUG;
extern int LOG_WARNING;
extern int LOG_MAJOR;
extern int LOG_ERROR;
extern int LOG_PANIC;

extern int LOG_LEVEL;

// Persistent Memory Issues
bool heating = false;

// This is switched ONLY when turning heater on or off.
bool currently_heating = false;
uint32_t time_incubation_started_ms = 0;
uint32_t time_heater_turned_on_ms = 0;
uint32_t time_spent_incubating_ms = 0;
uint32_t time_of_last_entry = 0;


// This is ten minutes, this should give us more
// than 48 hours. We record data in the eprom at this rate
// once the begin is done.
#define WORDS_IN_EEPROM 8192/2
#define MAX_SAMPLES (WORDS_IN_EEPROM -1)
unsigned long BASE_DATA_RECORD_PERIOD_S = 48 * 60 * 60 / (MAX_SAMPLES -1);
unsigned long BASE_DATA_RECORD_PERIOD_MS = BASE_DATA_RECORD_PERIOD_S * 1000;

// Don't USE_DEBUGGING in production!
// #define USE_DEBUGGING_SPEE 0
// For DEBUGGING, we may use a faster rate, of every 10 seconds
#ifdef USE_DEBUGGING_SPEED
unsigned long DATA_RECORD_PERIOD = (BASE_DATA_RECORD_PERIOD_MS / 60);
#else
unsigned long DATA_RECORD_PERIOD = BASE_DATA_RECORD_PERIOD_MS;
#endif


//time variables
#define FIVE_MINUTES 300000

//graph variables
int graphTimeLength = 24;  //2 hours long bexause plotting every 5 mins
/*graphTimeLength2=511;*/

//eeprom variables
// NOTE: I treat the EEPROM as 16-bit words.
#define TARGET_TEMP_ADDRESS 4095
// Because we keep the "INDEX" at location, we chave to be careful
// about our accounting and our meaning.
// TODO: This is not implementing a full ring buffer!
// INDEX is the number of samples we have at any point in time.
// Our functions to read and write must remove the 0-indexed element
// in which a sample is not stored.
#define INDEX_ADDRESS 0  //location of index tracker in EEPROM
// #define MAX_SAMPLES 509  //maximum number of samples that we can store in EEPROM

bool incubating = true;

int b = 1;  //initialize serial print index



// EEPROM FUNCTIONS --------------------------------------------------

//creates a whole number
uint16_t floatToSixteen(float flt) {
  uint16_t out = round(flt * 100);
  return out;
}
//creates a decimal
float sixteenToFloat(uint16_t sixteen) {
  float flt = sixteen / 100.0;
  return flt;
}

/*
   Writes 16 bits into EEPROM using big-endian respresentation. Will be called to enter temp at a specific address every 5 mibutes
*/
void rom_write16(uint16_t address, uint16_t data) {
  EEPROM.update(address, (data & 0xFF00) >> 8);
  EEPROM.update(address + 1, data & 0x00FF);
}

/*
   Reads 16 bits from EEPROM using big-endian respresentation. Will be called in readindex to get temp at certain index in order to plot graph
*/
uint16_t rom_read16(uint16_t address) {
  uint16_t data = EEPROM.read(address) << 8;
  data += EEPROM.read(address + 1);
  return data;
}

uint16_t getIndex() {
  return rom_read16(INDEX_ADDRESS);
}
/*
   Marks the EEPROM as empty by clearing the first address
   Note: Data is not actually cleared from other addresses
*/
void rom_reset() {
  rom_write16(INDEX_ADDRESS * 2, 0);
}



//gets the contents of the EEPROM at each indec
float readIndex(int index) {
  if (index > MAX_SAMPLES) {
    return -1;
  }
  uint16_t bitData = rom_read16((index + 1) * 2);
  return sixteenToFloat(bitData);
}

// here we dump to the serial port for debugging and checking
void dumpData() {
  Serial.println(F("DATA DUMP:"));
  uint16_t num_samples = rom_read16(INDEX_ADDRESS);
  Serial.print(F("# samples: "));
  Serial.println(num_samples);
  if (num_samples > MAX_SAMPLES) num_samples = MAX_SAMPLES;
  for (int i = 0; i < num_samples; i++) {
    float currenttemp = readIndex(i);
    Serial.print(i);
    Serial.print(F(" "));
    Serial.println(currenttemp);
  }
  Serial.println(F("Done with output!"));
}

// UTILITY FUNCTIONS --------------------------------------------------
void incubationON() {
  if (incubating) return;
  time_incubation_started_ms = millis();
}
void incubationOFF() {
  if (!incubating) return;
  uint32_t time_now = millis();
  time_spent_incubating_ms = time_now - time_incubation_started_ms;
}
uint32_t time_incubating() {
  return (incubating) ? millis() - time_incubation_started_ms : time_spent_incubating_ms;
}

uint32_t time_of_last_PWM;
int current_PWM_255;
double weighted_voltage_ms = 0.0;


void setHeatPWM_fraction(double df_fraction) {
  float intended_df_255 = 255.0 * df_fraction;
  if (intended_df_255 > 255.0) {
    Serial.println(F("excessive duty_factor: "));
    Serial.println(intended_df_255);
    intended_df_255 = 255.0;
  }
  if ( intended_df_255 < 0.0) {
    Serial.println(F("negative duty_factor: "));
    Serial.println(intended_df_255);
    intended_df_255 = 0.0;
  }
  uint32_t tm = millis();
  // _voltage_ms += ((double) current_PWM / 255.0)* (tm - time_of_last_PWM);
  const int pwm_255 = (int)intended_df_255;
// Now we must compute the fraction of the time of the last period that it 
// the heater was on.
  float time_of_last_period_ms = (tm - time_of_last_PWM);
  time_heater_turned_on_ms += (unsigned long) (time_of_last_period_ms *  ((float) current_PWM_255 / 255.0));

  analogWrite(HEATER_PIN, pwm_255);
  time_of_last_PWM = tm;
  current_PWM_255 = pwm_255;
  if (LOG_LEVEL >= LOG_MAJOR) {
    Serial.print(F("Set Heat PWM :"));
    Serial.println(pwm_255);
  }
}

void getTimeString(char* buff) {
  uint32_t mils = time_incubating();
  uint8_t hours = mils / 3600000;
  mils = mils % 3600000;
  uint8_t mins = mils / 60000;
  mils = mils % 60000;
  uint8_t secs = mils / 1000;
  mils = mils % 1000;

  sprintf(buff, "%02d:%02d:%02d", hours, mins, secs);
}

/*
   Writes a new entry into the next spot in EEPROM
   returns false if the list is full
*/
bool writeNewEntry(float data) {
  uint16_t bitData = floatToSixteen(data);
  uint16_t index = rom_read16(INDEX_ADDRESS * 2);
  // TODO: this needs to be turned into a ring buffer
  if (index >= MAX_SAMPLES) {
    return false;
  }
  rom_write16(2 * (index + 1), bitData);
  index = index + 1;
  rom_write16(INDEX_ADDRESS * 2, index);
  return true;
}

// Note: Since we desire to allow the target temperature to be half values,
// yet are using an integer, we will double the value on the way in and
// halve it on the way out.
float getTargetTemp() {
  uint16_t targetTempC = rom_read16(TARGET_TEMP_ADDRESS * 2);
  return targetTempC / 2.0;
}
void setTargetTemp(float temp) {
  uint16_t temp_i = (uint16_t)temp * 2;
  rom_write16(TARGET_TEMP_ADDRESS * 2, temp_i);
}
// return the number of watt hours used in the current incubation
float wattHours(float& average_watts) {
  float time_on_hours =  MS_TO_HOURS * time_heater_turned_on_ms ;
  float time_incubating_hours = MS_TO_HOURS * time_incubating();
  float watt_hours = POWER_WATTS * time_on_hours;
  average_watts = watt_hours / time_incubating_hours;
  return watt_hours;
}