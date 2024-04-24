# MooonRatII, A Tote-Size Portable Incubator for Rapid Field Work

Aliases used by other developers include Moonrat, Rice MiniCubator, and Rice Petri FI.  
**MoonratII** is the development effort of 2023.  
Visit the Legacy folder (above) for the work of volunteers prior to 2023 and the hand built prototype.  

# Status, April 2024  
The MoonRatII team has produced five Printed Circuit Assemblies (Control and Heater) of the Rev 1 for use by the USA team.  
The MoonRatII team has produced five Printed Circuit Assemblies (Control and Heater) of the Rev 2 for use by  Mexico team. 
Enclosure design for the Controller Assembly is underway.  

# Current Design Summary
In one schematic and PCB, we have developed a "Control Assembly" and a "Heater Assembly".  
The Control Assembly will be in an enclosure detachable from the incubation chamber.  
The Heater Assembly will be mounted internal to the incubation assembly and in addition to providing current to a heating pad or pads has a PCB mounted temperature sensor.  
A third component, a rechargeable battery with its recharging station, will complete a MoonRatII incubator system.

## 2023 MoonRatII Design
### Fundamental Modularity: The 6-wire appliance 
The connections from the controller to the heater needs only 6 wires:
1. GND (ground, nominally voltage 0.)
2. +12V PWR (the heating elements requires approximately 12V power.)
3. HEAT ON (Vin)
4. +5V (for the thermometer).
5. SIG -- data from the digital "one-wire" format
6. UNK - a final wire is included for future functionality that is unknown right now.  It is connected as a redundant ground in Rev 1 and Rev 2.

These (esp. the power) should probably be rated for 2 amps, but in practice 1 amp is probably good enough.

## Initial Design Approach Block Diagram
From the 2020 design.  
<a
href="https://user-images.githubusercontent.com/5296671/86680945-b470ad80-bfc4-11ea-9ef6-19c4f70aceba.png"><img style="border:5px double black"
src="https://user-images.githubusercontent.com/5296671/86680945-b470ad80-bfc4-11ea-9ef6-19c4f70aceba.png"
alt="https://user-images.githubusercontent.com/5296671/86680945-b470ad80-bfc4-11ea-9ef6-19c4f70aceba.png"
width="400">  
Basic Structure of Portable Incubator (1)
</a> 

## Motivation
By building a small, portable, intelligent incubator that can maintain constant elevated temperature, a variety of biological experiments and assays can be
performed "in the field" without having access to a electric grid power.

## Origin

This is an offshoot of the [Rapid E. coli project](https://github.com/PubInv/rapid-e-coli). It is an attempt to build a better, smaller, more intelligent
portable incubator that the Armadillo, described [elsewhere](http://jhe.ewb.org.au/index.php/jhe/article/view/127) and buildable from an [instructable](https://www.instructables.com/id/Portable-Petrifilm-Incubator-for-Inexpensive-In-Fi/).

## References (Incomplete)

1. Petrifilms: [https://www.3m.com/3M/en_US/company-us/all-3m-products/~/ECOLICT-3M-Petrifilm-E-coli-Coliform-Count-Plates/?N=5002385+3293785155&rt=rud](https://www.3m.com/3M/en_US/company-us/all-3m-products/~/ECOLICT-3M-Petrifilm-E-coli-Coliform-Count-Plates/?N=5002385+3293785155&rt=rud)
1. EPA Guidelines: [https://19january2017snapshot.epa.gov/sites/production/files/2015-11/documents/drinking_water_sample_collection.pdf](https://19january2017snapshot.epa.gov/sites/production/files/2015-11/documents/drinking_water_sample_collection.pdf)
1. Original Incubator Paper - [http://jhe.ewb.org.au/index.php/jhe/article/view/127](http://jhe.ewb.org.au/index.php/jhe/article/view/127)
1. Construction of a Low-cost Mobile Incubator for Field and Laboratory Use [https://www.jove.com/t/58443/construction-low-cost-mobile-incubator-for-field-laboratory](https://www.jove.com/t/58443/construction-low-cost-mobile-incubator-for-field-laboratory)
1. Solar Powered Portable Culture
Incubator [https://www.jscimedcentral.com/Pediatrics/pediatrics-3-1063.pdf](https://www.jscimedcentral.com/Pediatrics/pediatrics-3-1063.pdf)


## Acknowledgements
Thanks to Dr. Sabia Abidi of Rice University for input and references.

## MoonRatII Team
Harshit Kumar  
Enrique Ruiz   
Horacio Garcia   
Silvia Castillas    
[Melanie Laporte ](https://github.com/MelanieLaporte)  
[F. Lee Erickson](https://github.com/ForrestErickson)  
[Robert L. Read of Public Invention](https://github.com/PubInv/)  

<hr>
<h6><i> Updated by F. Lee Erickson, 20240424</i></h6>

