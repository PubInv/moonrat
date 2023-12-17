# Team Petri-FI Software Documentation

## Introduction

Team Petri-FI made progress in designing software to perform the incubation of Petrifilms inside of a small heating chamber equipped with a digital thermometer and heating pad(s). The code itself includes contributions from previous student teams as well as the Public Invention Moonrat team. The software’s goal is to provide an intuitive user experience for setting up an incubation and performing that incubation over the desired time period (generally, 48 hours). Team Petri-FI has developed two sketches for the Arduino Uno; these files can be uploaded to the Arduino, and the incubator can be powered with a battery. This documentation will describe the functionality of the existing code and the differences between the two files.

## "PetriFI.ino"

The main Arduino sketch that Team Petri-FI has used is called “PetriFI.ino.” This sketch was the original Team Petri-FI file, and it has undergone the most testing. This sketch makes greater use of Minicubator’s heating algorithm, which makes use of temperature thresholds to control the temperature inside the incubator. For example, this software will wait for the temperature to drop below an appropriate temperature before turning on the heating pad, and waiting to reach the target temperature again. In our own incubation testing, we have observed large temperature oscillations that result from the use of this algorithm (1 to 2 degrees Celsius below the target temperature), in which the temperature swings past the thresholds set by the software. In testing, this software was able to keep the temperature near the target temperature for a full 48-hour incubation, and it was still able to successfully incubate E. coli colonies.

Upon startup, this sketch allows users to set the duration and temperature of the incubation, and start the incubation. Once the incubator reaches the target temperature, it will enter the main incubation phase, and will proceed for the full duration (in hours) set at the beginning. The user interface is still limited, and does not allow interaction during the main incubation phase.

For data logging, this code utilizes a microSD card connected to the OLED screen. Our team struggled to get the SD card working consistently, so further testing is still required. We have written a function to write a single temperature to the SD card (into a .csv file), along with the time at which the temperature was recorded. For testing purposes, this sketch will record a temperature value occasionally, about every minute, but in the future the code should be able to record data at more consistent intervals. It may also be valuable to keep a running average of data points and write the average to the SD card every couple of minutes. Additionally, for testing, we chose to also write some values during the pre-heating phase. Since the “clock” hasn’t started during this phase, and to prevent confusion with legitimate values recorded during the incubation, we have assigned these a time value of -1. Later, these temperature values can be omitted, since they are not as important in evaluating the success of a full incubation.

When adding the capabilities for the SD card, we found that it was important which pins were connected at the Arduino Uno. In particular, the “SPI Bus” (pins 10-13) of the Uno were important for writing to the SD card. Here, we have included a list of pins that we have used with the Uno. Below, the first seven connections are to the OLED screen, which contains both the display and the SD card. The SD Chip Select pin must be the hardware SS pin (pin 10 on the Uno) so that the SD card can use the SPI bus. If possible, all of pins 10-13 should be connected as described below in order to utilize the SD card. The screen itself will work even if the SPI bus is not used, but the SD card requires the SPI bus (SDCS, MOSI, MISO, and SCLK). 

### Arduino Uno Pin Setup

| Connection                         | Pin Number        |
|------------------------------------|-------------------|
| SD Chip Select (SDCS)              | 10                |
| MOSI                               | 11                |
| MISO                               | 12                |
| SCLK                               | 13                |
| DC                                 | 4                 |
| OLED Chip Select (OLED CS)         | 5                 |
| RST                                | 6                 |
| Up Button                          | 3                 |
| Select Button                      | 8                 |
| Down Button                        | 9                 |
| Transistor (Heating Pad)           | 2                 |
| LED                                | 0                 |
| One-Wire Bus (Digital Thermometer) | 7                 |
| Piezo Alarm                        | A1 (Analog Pin 1) |

## "PetriFI_PID.ino"

The most recent sketch is called “PetriFI_PID.ino,” and it is an attempt to implement a PID controller for temperature. This separate sketch was made to isolate the many changes that would need to be made for the PID controller. The goal of the PID controller is to hold the temperature perfectly steady by controlling the heating pad. This would alleviate the problems that we have observed with temperature oscillations of more than one degree Celsius. This sketch is designed to be almost identical in functionality to the original sketch, but to better control the temperature during the incubation period.

Currently, this PID controller code is not working. This sketch makes use of the “PID_v1.h” Arduino library. In our understanding, the PID controller will update its output as a value between 0 and 255. This value should describe how much the heating pad needs to be turned on after each loop. Then, the sketch can use that value to control the heating pad in a couple of ways. The most fluid way to control the heating pad is to use pulse-width modulation with the 0-255 PID output. However, the time constant of the chamber is fairly high (it takes multiple minutes for the temperature to change by even just one degree Celsius), and pulse-width modulation would mean that duty cycle calculations (what percentage of the time the heating pad is ON) might be less accurate. Additionally, we were somewhat concerned about switching losses associated with switching the heating pad state multiple times each second. Instead, we attempted to convert the PID output (0-255) into either a “heating pad on” or “heating pad off” command, using a threshold. Thus, when the output reaches the threshold, then the heating pad would be turned on until the output fell below the threshold again.

We have not yet been able to use the PID controller, but we have used print statements to perform some troubleshooting. Most recently, the output of the PID controller remained at 0 despite adjustments to various parameters. We believe that once the PID controller is implemented into this sketch, it may require some “tuning” of the threshold and the PID parameters, but that it will be able to control the temperature better than “PetriFI.ino.” Below we have provided some of the resources related to the PID library that were useful to us.

https://playground.arduino.cc/Code/PIDLibrary/

https://github.com/br3ttb/Arduino-PID-Library

## Areas for Further Testing or Improvement

We hope that a few improvements can be made to our software in the future. First, we believe that the PID controller would significantly improve the consistency of incubation. Some debugging is still needed for this, but “PetriFI_PID.ino” should be nearly ready to use if the PID controller is correctly implemented.

We have also worked hard on a new user interface that would help make the device “smarter” and more intuitive to work with. It would also allow users to interact with it during the incubation cycle. We have provided diagrams and many more details within our user interface documentation, but we believe that it would be a great improvement to the user experience. Based on the lifespan of the LEDs inside the OLED screen, it may also be important to create a timer for the screen to turn off during the main incubation period to reduce power consumption and increase the lifespan of the screen.

While the SD card did not work consistently with our system, it was occasionally able to write a temperature value to the SD card. On another system, or with a few adjustments, the SD card may be able to work very consistently. As part of the data display, it may also be useful to provide functions to read desired values from the SD card and display them onto the user interface. Therefore, we have a created part of a function, “readTempFromSD(int seconds),” that could read a single temperature value from the SD card. This could be used to improve data logging and retrieval capabilities, so that the user would not need to remove the SD card from its slot in order to evaluate the success of an incubation.
