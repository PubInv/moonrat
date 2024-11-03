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
uint32_t time_spent_heating_ms = 0;
uint32_t time_spent_incubating_ms = 0;
uint32_t time_of_last_entry = 0;


// This is ten minutes, this should give us more
// than 48 hours. We record data in the eprom at this rate
// once the begin is done.
unsigned long BASE_DATA_RECORD_PERIOD = 600000;

// Don't USE_DEBUGGING in production!
#define USE_DEBUGGING 1
// For DEBUGGING, we may use a faster rate, of every 10 seconds
#ifdef USE_DEBUGGING
unsigned long DATA_RECORD_PERIOD = (BASE_DATA_RECORD_PERIOD / 60);
#else
unsigned long DATA_RECORD_PERIOD = BASE_DATA_RECORD_PERIOD;
#endif


//time variables
#define FIVE_MINUTES 300000

#define FORTYEIGHT_HOURS 172860000  //forty eight hours
#define TICK_LENGTH 125
//graph variables
int graphTimeLength = 24;  //2 hours long bexause plotting every 5 mins
/*graphTimeLength2=511;*/

//eeprom variables
// NOTE: I treat the EEPROM as 16-bit words.
#define TARGET_TEMP_ADDRESS 511
// #define TARGET_TEMP_ADDRESS 211
// Because we keep the "INDEX" at location, we chave to be careful
// about our accounting and our meaning.
// TODO: This is not implementing a full ring buffer!
// INDEX is the number of samples we have at any point in time.
// Our functions to read and write must remove the 0-indexed element
// in which a sample is not stored.
#define INDEX_ADDRESS 0  //location of index tracker in EEPROM
#define MAX_SAMPLES 509  //maximum number of samples that we can store in EEPROM

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
  rom_read16(INDEX_ADDRESS);
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
int current_PWM;
double weighted_voltage_ms = 0.0;

uint32_t time_heating() {
  return (currently_heating) ? time_spent_heating_ms + (millis() - time_heater_turned_on_ms) : time_spent_heating_ms;
}


float duty_factor() {
#ifdef USE_PID_AND_PWM
  return (double)(((millis() - time_of_last_PWM) * current_PWM / 255.0) + weighted_voltage_ms) / (double)time_incubating();
#else
  return (float)time_heating() / (float)time_incubating();
#endif
}


//turn the heating pad on
void heatOFF() {
  if (currently_heating) {
    //  digitalWrite(HEATER_PIN, HIGH);
#ifdef NCHAN_SHIELD
    analogWrite(HEATER_PIN, 0);
    // digitalWrite(HEATER_PIN, LOW);
#else
    Serial.println(F("not implemented heatsOFF"));
#endif
    if (LOG_LEVEL >= LOG_DEBUG) {
      Serial.println(F("HEAT OFF!"));
    }
    currently_heating = false;
    uint32_t time_now = millis();
    time_spent_heating_ms += time_now - time_heater_turned_on_ms;
  }
}
void heatON() {
  if (!currently_heating) {
    //  digitalWrite(HEATER_PIN, LOW);
    analogWrite(HEATER_PIN, 255);

    if (LOG_LEVEL >= LOG_DEBUG) {
      Serial.println(F("HEAT ON!"));
    }
    time_heater_turned_on_ms = millis();
    currently_heating = true;
  }
}
// TODO: I would like to have a total measure of Joules here...
// that requires us to keep track of the last time we changed
// the pwn, track the pwm, and assume linearity.

void setHeatPWM(double intended_df) {
  if (intended_df > 255.0) {
    Serial.println(F("excessive duty_factor: "));
    Serial.println(intended_df);
    intended_df = 255.0;
  }
  if (intended_df < 0.0) {
    Serial.println(F("negative duty_factor: "));
    Serial.println(intended_df);
    intended_df = 0.0;
  }
  uint32_t tm = millis();
  // _voltage_ms += ((double) current_PWM / 255.0)* (tm - time_of_last_PWM);
  const int pwm = (int)intended_df;
  analogWrite(HEATER_PIN, pwm);

  time_of_last_PWM = tm;
  current_PWM = pwm;
  if (LOG_LEVEL >= LOG_MAJOR) {
    Serial.print(F("Set Heat PWM :"));
    Serial.println(pwm);
    Serial.print(F("Weighted Duty Factor :"));
    Serial.println(duty_factor());
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
