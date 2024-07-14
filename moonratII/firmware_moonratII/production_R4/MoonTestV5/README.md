IMPORTANT NOTE: It is necessary to include within the Arduino IDE the eFLL library for the fuzzy controller and the necessary libraries for the Arduino UNO R4 version to be used.

Improvements of this version:
1.- Fixed the temperature conversion needed for the TMP37 sensor embedded in the T6G IC, the ADC reading problems for the Arduino UNO R4 have been fixed.
1.- Added a reset command during incubation by simultaneously pressing the three buttons on the board.
2.- Modified the sample selection screen to directly display the desired incubation temperature (modify if necessary).
3.- A menu was added to select the required incubation hours (modify if necessary).
4.- An acoustic alarm was added to notify that the incubation time has been completed. 
5.- A command was added to exit the completed incubation time alert by simultaneously pressing the up/down buttons.
