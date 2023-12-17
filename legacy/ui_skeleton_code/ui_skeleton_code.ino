//Sara Barker, modified July 6, 2021
//Team Petri-FI UI Code
  //This is the very rough and unfinished draft of the UI functionality code. 


//For right now, the variables are hardcoded into here so we don't have to set up the whole system to test.
int startingTemp = 30; //this is only used at the starting screen to show the inside temp of the container
int temp; //keeps track of temp inside container - will eventually be the thermistor reading
int runTime; //the time that is set at the beginning of the run
int elapsedTime; //the time that the device has been running for

#define MS_PREPARING 0
#define MS_INCUBATING 1
#define MS_DONE 2
int machine_state = MS_PREPARING; // Hold machine state

#define UI_STARTUP 0
#define UI_HEATING 1
#define UI_SUMMARY 2
#define UI_SET_TEMP_AND_TIME 3 

int ui_state = UI_STARTUP; // hold GUI_State




/*Each function represents a screen. The switch statement above will read the button and electrical state
 * and use the switch statement to determine which function to call.
 */

int startupScreen(int startingTemp) {
/*The first screen - automatically starts up once the battery is connected.
 * Enter with battery connection, exit with OK
 * Inputs: currentTemp, outputs: OKbuttonState
 */

// Note: Sara had used some more powerful syntax which is legal C++ but not available on an Arduino,
// So I have broken this into 3 statements -- rlr
Serial.print("Current inside temperature: ");
Serial.print(startingTemp);
Serial.println("C");
Serial.print("Press OK to set new incubation");

//read button press
//return OK button value = 1


}

int setTempBTN(bool up, bool dn, bool ok) {
  if (up) {
    temp++;
  }
  if (dn) {
    temp--;
  }
}

int setTemp(){
/*Setting temperature before beginning the incubation.
 *Enter with OK, exit with OK
 *Inputs: none, outputs: temp
 *
 *Note: setTemp goes directly into setTime
 */  

// Note: we currently don't have "currentTemp" defined---I'm commenting this out for now -- rlr
// temp = currentTemp;
/*logic for buttons:
 * if up = 1 && down = 0
 *  temp = temp+1
 * if down = 1 && up = 0
 *  temp = temp-1
 *  loop until OK pressed
 */
Serial.print("Set temp: ");
Serial.print(temp); //put highlight on the number
Serial.println("C");
Serial.println("Set time: 48h");
Serial.print("Press OK to proceed");
//if OK button pressed, go to setTime(temp)
}

int setTime(int temp){
  /*Sets time on the same screen as setTemp
   * Enters with OK, exits with OK
   * Inputs: temp, outputs: runTime
   */
runTime = 48; //default = 48 hours

Serial.print("Set temp: " + String(temp) + "C");
Serial.print("Set time: ");
Serial.print(runTime);//put highlight on the number
Serial.println("h");
Serial.print("Press OK to proceed");
//return runTime
}


int summary(int temp, int runTime){
/*Show summary screen with set temp/time, can start run or go back to setting screen
 * Enters with OK from setTime, exits with either OK or down/OK
 * Inputs: temp, runTime, outputs: temp, runTime (doesn't modify, just passes on)
 */
Serial.println("SUMMARY");
Serial.println("Temp: " + String(temp) + "C");
Serial.println("Time: " + String(runTime) + "h");
Serial.println("START");
Serial.println("Edit incubation settings");
}


void heating(int temp){
/*Screen shows only when heating up to the set temperature; will switch off when the temperature
 *remains above the set temp for a set amount of time
 *Enters with OK from summary, exits when temperature has not dropped below temp for a set amount of time
 *  or with OK
 *Inputs: temp, outputs: none
 */
Serial.println("Current temp: " + String(temp) + "C");
Serial.println("HEATING...");
Serial.println("Press OK to stop incubation");
}

void runningScreen(){

}

void statsScreen(){

}

void warningScreenCurrent(){
//if temperature is currently below set temp


}

void warningScreenPast(){
//if temperature went below set temp previously but device has not been checked since
}

void runStopped(){

}

void setup() {
//will fill this in later once we integrate the UI code into the main code
}

void loop() {
  /* This code works by checking the electrical state (i.e. time, temperature) and the button state (i.e. the value
   *  (0 or 1) of the three buttons).
  */

//********run startup screen first before entering into the switch statement

//*****check current temp, display, output OK button press
//startingTemp = read temp
//buttonState = startupScreen(startingTemp)

//now start the loop

  //check electrical state - gives temp, time

  //check button state


  //switch(elec state)
      //switch(button state)
          //use the minicubator code for detecting button presses

 // BUTTON PRESS
 bool up = false; // Digital read  of a particular pin
 bool dn = false;
 bool ok = false;

  switch(ui_state) {
    case UI_STARTUP:
    startupScreen(startingTemp);
    break;
     case UI_HEATING:
     break;
    case UI_SUMMARY:
    break;
    case UI_SET_TEMP_AND_TIME:
      setTempBTN(up,dn,ok);
    break;
    default:
    Serial.println("Internal Error! Unimplemented State!");
  }
 // EXTERNAL EVENT

 // PID/HEAT

 // UISTATE

  switch(ui_state) {
    case UI_STARTUP:
    startupScreen(startingTemp);
    break;
     case UI_HEATING:
     break;
    case UI_SUMMARY:
    break;
    case UI_SET_TEMP_AND_TIME:
      setTemp();
      setTime(temp);
    break;
    default:
    Serial.println("Internal Error! Unimplemented State!");
  }
          
 delay(100);
}
