# Factory Test Plan, MoonRat V1, Printed Wiring Assembly
Journal of tests for assemblies of December 2023.  
PCB assembly by JLCPCB with some finish work in Maraland.  
Five Assemblies assigned SN 1-5.  


## Resources
12V power supply with current monitor. Color the plug Red. 
Multi meter to measure voltage and resistance.  (Actual Multimeter (EMCO DMR-3800) )  
PC with Arduino IDE to connect to USB programing port of Due  
Firmware: https://github.com/PubInv/NASA-MCOG/tree/develop/elec/Control%20Board/Control_1v1/FactoryTest  
Arduino Sketches: 
1. DueWithThreeSSRs
2. GraphicsTest

## Test Procedure
### Before Test, Finish Assembly
Photos of received assemblies at: ??????
Install on these units the feed through headers for the UNO connections which make this assembly become a shield for a UNO.

Color the connector J11, +12VIn red with a sharpie.


### Unpowered Test of PWA.
These tests make sure we can plug the device into power with out letting out magic smoke.

**Summary and data capture table**

Tester Name ____ F. Lee Erickson ________________
Date of Test _____ 20231026 _____________________

| Test ID 	| Test Name       	| Setup                                          	| Test Location 	| Requirements    	| Results SN 9 	| Results SN 10 	| Results SN 11 	| Results SN 12 	| Results SN 13 	|
|--------:	|-----------------	|------------------------------------------------	|---------------	|-----------------	|--------------	|---------------	|---------------	|---------------	|---------------	|
|       1 	| Inspection      	| Inspect correct orientation on polarized parts 	|               	|                 	|      D1 and D2 Probably backwards. |               	|               	|               	|               	|
|       2 	| +24In In No Short 	| Set ohm meter 20M. Com to GND                  	| TP20 (J12 Pin 1)   	|  > 10K 	|  34.6K  	|  34.6K        	|     34.5K  	|  34.6K       	|               	|
|       3 	| +12In In No Short 	| Set ohm meter 20M. Com to GND                  	| TP 2 (J11 Pin 1) |   > 10K 	 |    39.8K |  39.9K      	|   39.8K         |   39.8k      	|               	|
|       4 	| 5VHEAD net      	| Set ohm meter 20M. Com to GND                  	| J8 Pin 1 | Open or > 1 Meg 	|     Open |       Open   	|      Open     	|      Open   	|               	|
|       5 	| +5V net         	| Set ohm meter 20M. Com to GND                  	| J3 Pin 4 | Open or > 1 Meg 	|     12.1 Meg |      Open   	|        Open 	| 2.4K (Issue #126 https://github.com/PubInv/NASA-MCOG/issues/126) |       	|
|       6 	| +3.3V net       	| Set ohm meter 20M. Com to GND                  	| J3 Pin 5 | Open or > 1 Meg 	|   11.7 Meg |         Open    	|         Open     	|         Open     	|               	|



### Program UNO with Firmware
Before connecting to the MoonRatII V1 Assembly, program the Uno with the factory test firmware "?????.ino" 

## Finish Assembly

#### Build Up MAX31850 Thermocouple Amplifier Sub Assemblies
Note that the side of the board to be up has the pads for configuring the addresses of the assemblies. 
Solder the header and screw terminal to the MAX31850 on the correct side.

1. Install the MAC13850 thermocouple amplifier assemblies to the Control V1.1 at JP9, JP10 and JP15.
Add a ?2.5mm? x 10 mm screw with nut as mechanice support and retention.
Observing polarity, connect thermocouples to the MAX31850 amplifire assemblies.
Cut the four on board shunt on each MAX31850 PCB (See image below for where to cut on one).  Measure with multimeter that the shunt to GND is open.
![image](https://github.com/PubInv/NASA-MCOG/assets/5836181/df5c26bd-828b-47e3-a31b-94f947e3e6c9)

3. Add screws for legs such as 8-32 x 2") with nuts.  These legs are necessary because the long stacking connectors to be installed must not be bent which would happen if the legs were not installed.
4. Solder in the stakcing Due connectors, seven locations.  The long tails must go in the top and through to the bottom
5. Connect the Due to the Controller assembly.  The Due is under the Controller V1.1 assembly as a "shield".
6. Connect the Ethernet W5200 (or similar) sheild on the top of the Controller V1.1

### Apply power. 
Note the +12VIn, the +24VIn and the Stack programable power supply are connected to a switchable outlet switch.  
With the power outlet off, 
Connect the +12VIn power at J11, the RED connector.  
Connect the  +24VIn power at J12.  
Connect SSR simulator LEDS at  SSR1, SSR2, SSR3.  
Connect the Stack power supply at J10 ( or J27) at this time.

Apply power by switching on the AC power strip.
**Test ID7:** Note current in table on the +12VIn supply current monitor for excessive, above 100 mA.  THIS NUMBER WILL NEED TO BE CHECKED WITH DUE's THAT HAVE NEVER BEEN PROGRAMED.
Check that the BUILDIN_LED ner the LAN Shield and SSR2 and SSR3 connectors is blinking rapidly as an idication that the firmware is running.
Open the Arduino Serial Plotter (<Ctrl> <Shift> <L>) and touch some thermocouples.

### Temperature Measurements.
**Test ID8:** Typical Serial Plotter results. Touching some of the thermocouples to ensure they respond.
Note resutls in table
![image](https://github.com/PubInv/NASA-MCOG/assets/5836181/e4f5bb6a-2a47-4039-8720-b6846f36b7f7)

**Results continued...**
| Test ID 	| Test Name  | Setup | Test Location | Requirements  | Results SN 9 | Results SN 10 | Results SN 11 | Results SN 12 | Results SN 13 	|
|--------:	|------------ | -------- |---------- | ------------	|--------------	|----------	|----------- |--------------- |--------------- |
|      7 | Current, Temperature Only  | In line DC current meter |    Due input |  85 mA |             |   270mA   |  200mA     |         |
|      8 | Check thermocouples  | Connect three |   Serial Plolter |  Three traces |   OK   |      OK   |      OK     |             |         |
|      9 | future use  |  |      |               |             |             |             |         |            |


Load and run factory test firmware DueWithThreeSSRs.ino
Open the Arduino Serial Monitor. Observe the data.
**Test ID10 ** Observe SSR1, SSR2, SSR3 LED Blinking.
**Test ID11 ** Observe Mini12864 Display back lit and displaying splash message and four measurements.
**Test ID12 ** Encoder Rotation Knob is back lit. Rotate the know and observe the serial port reporting changes.
**Test ID13 ** Encoder knob switch. Press the knob and observer serial port message about shutdown.
**Test ID14 ** PCB mount SHUT DOWN button. Press the button and observer serial port message about shutdown.


**Results continued...**
| Test ID 	 | Test Name  | Setup | Test Location | Requirements  | Results SN 9 | Results SN 10 | Results SN 11 | Results SN 12 | Results SN 13 	|
|--------:	|------------ | -------- |---------- | ------------	|--------------	|----------	|----------- |--------------- |--------------- |
|      10 | SSR1, SSR2, SSR3 LED Blink  |  |      |               |    OK       |          OK    |       OK      |               |                |
|      11 | Mini12864 Display           |  |      |               |             |          OK    |       OK      |               |                |
|      12 |Encoder Rotation Knob        |  |      |               |             |          OK    |       OK      |               |                |
|      13 |Encoder knob switch          |  |      |               |             |          OK    |       OK      |               |                |
|      14 | PCB button SHUT DOWN        |  |      |               |             |          OK    |       OK      |               |                |
|      15 | LAN CONNECTION              |  |      |               |             |          OK    |       OK      |               |                |
|      16 | Stack voltage at reset 1V   |  |      |               |             |          OK    |       OK      |               |                |
|      17 | Stack power to load         |  |      |               |             |          OK    |       OK      |               |                |
|      18 | Cumulocity reporting        |  |      |               |             |          OK    |       OK      |               |                |
|      19 | Reset to safe               |  |      |               |             |          OK    |       OK      |               |                |
|      20 | PxxxxxxxxxxxxxxxxxN         |  |      |               |             |             |             |         |            |

### Stack Power Supply Control 
With AC power on.

#### PS1_EN Jumper

| Nets to Jumper 	| Description  |
|--------	|------------ |
| ![image](https://github.com/PubInv/NASA-MCOG/assets/5836181/106d175e-e942-4e01-a3b8-b7b6f470ea7f) | Untill the OEDCS firmware is modified to support the PS1_EN and PS2_EN control lines place a jumper wire from J14 Pin 5 to J9 pin 9   |

Load the OEDCS firmware into the unit under test.  
Open the serial monitor. 
Observe the traffice and note that a Power supply  is reported.  
![image](https://github.com/PubInv/NASA-MCOG/assets/5836181/f4dca5d2-eb4a-4583-9b26-30fd3afd920a)  
Observe the traffice and note that an IP address is reported.  
![image](https://github.com/PubInv/NASA-MCOG/assets/5836181/607dcabb-5c68-43cf-8c0f-e29e81acb0e4)  
On serial port monitor confirm device connects to LAN **ID16**.  

#### Set Stack power supply and +24VIn.  
Connect a 1.5K 1/2 wat resistor load on the Stack Power Supply ouput.
With multi meeter observe the voltage on the stack power supply load.  
After observe and record voltage **ID16** after reset it should be 1V the power on voltage.
Send OEDCS serial port commands to set Stack power to load manual.
* s:1
* a:0.1
* w:2.5
Observe and record **ID17** Stack power to load should now be about 12V

### Observe data on Cumulocity  
https://mcogs.us.cumulocity.com/apps/app-builder/index.html#/application/34000/dashboard/71202350/device/45175917
![image](https://github.com/PubInv/NASA-MCOG/assets/5836181/2a72fef9-9d39-459b-a6e8-70a8755b9307)
Observe and report **ID19**. 

### Reset to Safe
Press and hold the reset button.
Observe that the Stack power supply voltage goes to zero.  (IT DID NOT as of 20231218 with TF800A12K)
Observe that the blower stops.
Observe that the SSR LEDs go off.
Recorde results in test **ID19**  

### End of Test
Power off the unit under test.
Remove all connections including the jumper wire at J14 and J25.
Very carfuly so as to not bend pins, remove the LAN Sheild.
Very carfuly so as to not bend pins, remove the Due controller. A modified pop cycle stick helps.

## Rejoice another great OEDCS has been born!


