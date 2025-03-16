# Factory Test Plan, MoonRat V1, Printed Wiring Assembly
Journal of tests for assemblies of silk screen 20231120.  
PCB assembly by JLCPCB with some finish work in Connecticut.  

Five Assemblies assigned SN 1-5.  Lable with sharpie.  
![image](https://github.com/PubInv/moonrat/assets/5836181/55aeb627-2a82-43cc-8342-174519147171)


## Update 20230123 -> Link to the Moonrat II Wokwi simulator: 
https://wokwi.com/projects/384727010846638081  
_TODO: get up to date from Horacio_

## Resources
12V @ 1.5 A power supply. Color the plug Red. 
Multi meter to measure voltage and resistance.  (Actual Multimeter: AstroiAI  DM6000AR )  
PC with Arduino IDE to connect to USB programing port of UNO.
Firmware: https://github.com/PubInv/NASA-MCOG/tree/develop/elec/Control%20Board/Control_1v1/FactoryTest  
Arduino Sketches: 
1. Foo
2. Bar

## Test Procedure
### Before Test, Finish Assembly
Photos of received assemblies at in folder at:
https://github.com/PubInv/moonrat/tree/main/moonratII/pcb_moonratII/manufacturing_test/Assembly_photos_Rev1
See also testing notes and rework against short of USB to SW1 at https://github.com/PubInv/moonrat/issues/209


These tests make sure we can plug the device into power with out letting out magic smoke.
**Check polarity on:**
1. Buzzer
2. D1  Look for the laser mark across the part. This is the Cathode.
![image](https://github.com/PubInv/moonrat/assets/5836181/e7f7f59b-3af6-4b98-8371-8dd43a3b12aa)


3. Connectors: J5, J6, J9, J10, J11
4. U1

Install M2x6 screws at the four corners of the OLED. Manufacturier:  ________,  Partnumber: ____________, Steel Pan Head Phillips Screw M2 x 0.4 mm Thread, 6 mm Long

De-panalize the Heater from the Controller.  Place the Controller over a hard surface. Hold it. Hold it down against the hard surface. Press the Heater side down to break it clear.

Install (Solder) the feed through headers for the UNO connections which make this assembly become a shield for a UNO.

### Unpowered Test of PWA.


**Summary and data capture table**

Tester Name ____ Melanie Laporte ________________
Date of Test _____ 20220126 _____________________

| Test ID 	| Test Name       	| Setup                                          	| Test Location 	| Requirements    	| Results SN 1 	| Results SN 2 	| Results SN 3 	| Results SN 4 	| Results SN  	|
|--------:	|-----------------	|------------------------------------------------	|---------------	|-----------------	|--------------	|---------------	|---------------	|---------------	|---------------	|
|       1 	| Inspection      	| Inspect correct orientation on polarized parts 	|               	|                 	|      D1 and D2 Probably backwards. |               	|               	|               	|               	|
|       3 	| +12In In No Short 	| Set ohm meter 20M. Com to GND                  	| J1 pin 1 to GND |   > 100K 	 |   Open  |  Open      	|   Open         |   Open      	|    Open           	|
|       4 	| NA     	| Set ohm meter 20M. Com to GND                  	| J8 Pin 1 | Open or > 1 Meg 	|     Open |       Open   	|      Open     	|      Open   	|               	|
|       5 	| NA     	| Set ohm meter 20M. Com to GND                  	| J3 Pin 4 | Open or > 1 Meg 	|     12.1 Meg |      Open   	|        Open 	|  foo |       	|
|       6 	| NA     	| Set ohm meter 20M. Com to GND                  	| J3 Pin 5 | Open or > 1 Meg 	|   11.7 Meg |         Open    	|         Open     	|         Open     	|               	|



### Program UNO with Firmware
Before connecting to the MoonRatII V1 Assembly, program the Uno with the factory test firmware "?????.ino" 

## Finish Assembly

#### Build Up MAX31850 Thermocouple Amplifier Sub Assemblies
.....

### Apply power. 
....

### Temperature Measurements.
....

**Results continued...**
A table of results here...


### End of Test
Power off the unit under test.
Remove all connections 
Box up unit.

## Rejoice another great MoonRatII has been born!


