<h1 align="center">MoonRatII, A Tote-Size Portable Incubator for Rapid Field Work</h1>

Aliases used by other developers include [Moonrat](https://github.com/PubInv/moonrat/tree/main/legacy/moonrat), [Rice MiniCubator](https://github.com/PubInv/moonrat/tree/main/legacy/RiceMiniCubator), and [Rice Petri FI](https://github.com/PubInv/moonrat/tree/main/legacy/RicePetriFI). Visit the [Legacy](https://github.com/PubInv/moonrat/tree/main/legacy) folder for the work of volunteers prior to December 2022 and the hand built prototype.

**MoonratII** development effort began January 2023.   

---

# Status, April 28, 2025  
Created a stand alone schematic for heaterboard PCB.

# Status, January 2025  
#### Project presentation on YouTube (Click image to watch)

[![Watch the video](https://github.com/user-attachments/assets/b2fde032-a1f6-4cba-a1fe-d6f2304b3a2d)](https://www.youtube.com/watch?v=yTW_BcTsaGw&t=22s)
#### Updated Experiment.com succesful project funding round
In January 2023, 13 generous patrons funded the production of five complete MoonRatII prototypes. we updated them with the [project results](https://experiment.com/projects/tote-size-portable-incubator-for-rapid-field-work
).
#### 2024 field testing resulted in future unit changes:
- Heater board redesign with a Dallas One wire digital temperature thermometer and a 4.7 ohm pull up resistor.
- Design a universal heater board cage to secure the heater board beneath the container lid in any external enclosure.
- Adjust top button spacing in KiCad.
- Add bottom mounting holes to the enclosure.

# Status, November 2024 
A scientific research paper is tentatively scheduled for March 2025 in the HardwareX journal.

# Status, September 2024  
Prototypes of the MoonratII were field tested in two locations - the Galapagos Islands by researcher Auja Bywater and in Tanzania by Dr. Robert Read. The heater features the TMP36 low voltage temperature sensor. A HardwareX scientific research paper is slated for October 2024 publication.

# Status, April 2024  
The MoonRatII team has produced five Printed Circuit Assemblies (Control and Heater) of the Rev 1 PWB for use by the USA team and five Printed Circuit Assemblies (Control and Heater) of the Rev 2 PWB for use by  the Mexico team. Enclosure design for the Controller Assembly of Rev 3 PWB is underway TBA. 

# Status January 2024
[Dr. Robert Read](https://www.pubinv.org) and [Melanie Laporte](https://github.com/melanielaporte) presented the MoonRatII project. Watch the [presentation](https://www.youtube.com/watch?v=yTW_BcTsaGw&t=22s). 

# Current Design Summary
In one schematic and PCB, we have developed a "Control Assembly" and a "Heater Assembly".  
- The Control Assembly will be in an enclosure detachable from the incubation chamber.  
- The Heater Assembly will mounted internal to the incubation assembly under the unit top. Current to a heating pad or pads has changed from the previously used TMP36 low voltage temperature sensor to a digital temperature sensor. 
- A rechargable battery with its recharging station will complete a MoonRatII incubator system.
  
## Image Gallery - Current FreeCAD Assembly Designs
|A    |B    |C    |
| --- | --- | --- |
| <img width="300" atl="image1" src="https://github.com/user-attachments/assets/e7c12fc4-8284-4690-aac5-48fd973704af"> | <img width="300" atl="image2" src="https://github.com/user-attachments/assets/09263421-4ce9-420e-a656-ec62c1cdc5f2"> | <img width="300" atl="image4" src="https://github.com/user-attachments/assets/596ca510-e76f-4bca-a64a-ca0ef15ba3e8">|
| <img width="300" atl="image5" src="https://github.com/user-attachments/assets/1494b485-7472-4db9-9431-564c4f7dbedf"> | <img width="300" atl="image6" src="https://github.com/user-attachments/assets/4dbd4b99-946e-4f79-9589-6b527d1ca0e3"> | <img width="300" atl="image7" src="https://github.com/user-attachments/assets/b570462b-f9e0-4349-9fd8-33f91ea28a3b"> |
| <img width="300" atl="image8" src="https://github.com/user-attachments/assets/a76b752d-e472-4d29-aec0-d65051a0d5a1"> | <img width="200" atl="image9" src="https://github.com/user-attachments/assets/1ebdbb64-3320-4e3b-8af4-2a12f6a69f7d"> | <img width="300" atl="image10" src="https://github.com/user-attachments/assets/ffd1c814-bdfc-4d9a-a9ed-c0cb051c3e43"> |  

# 2023 MoonRatII Design

### Fundamental Modularity: The 6-wire appliance 
The connections from the controller to the heater needs only 6 wires:
1. GND (ground, nominally voltage 0).
2. +12V PWR (the heating elements requires approximately 12V power).
3. HEAT ON (Vin).
4. +5V (for the thermometer).
5. SIG - data from the digital "one-wire" format.
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
- Petrifilms [https://www.3m.com/3M/en_US/company-us/all-3m-products/~/ECOLICT-3M-Petrifilm-E-coli-Coliform-Count-Plates/?N=5002385+3293785155&rt=rud](https://www.3m.com/3M/en_US/company-us/all-3m-products/~/ECOLICT-3M-Petrifilm-E-coli-Coliform-Count-Plates/?N=5002385+3293785155&rt=rud)
- EPA Guidelines [https://19january2017snapshot.epa.gov/sites/production/files/2015-11/documents/drinking_water_sample_collection.pdf](https://19january2017snapshot.epa.gov/sites/production/files/2015-11/documents/drinking_water_sample_collection.pdf)
- Original Incubator Paper [http://jhe.ewb.org.au/index.php/jhe/article/view/127](http://jhe.ewb.org.au/index.php/jhe/article/view/127)
- Construction of a Low-cost Mobile Incubator for Field and Laboratory Use [https://www.jove.com/t/58443/construction-low-cost-mobile-incubator-for-field-laboratory](https://www.jove.com/t/58443/construction-low-cost-mobile-incubator-for-field-laboratory)
- Solar Powered Portable Culture Incubator [https://www.jscimedcentral.com/Pediatrics/pediatrics-3-1063.pdf](https://www.jscimedcentral.com/Pediatrics/pediatrics-3-1063.pdf)
- World Health Organization
- Fuzzy Logic - 

## Acknowledgements
Thanks to [Dr. Sabia Abidi](https://profiles.rice.edu/faculty/sabia-abidi) of Rice University for input and references.

## MoonratII Team
[Robert L. Read - Founder of Public Invention](https://github.com/PubInv/) 

[F. Lee Erickson](https://github.com/ForrestErickson)

[Melanie Laporte](https://github.com/MelanieLaporte)  

[Silvia Castillas](https://github.com/SilviaKarbid)  

[Harshit Kumar](https://github.com/harshit9602)

[Enrique Ruiz](https://github.com/kurama79) 

[Horacio Garcia](https://github.com/HJGV05) 

<hr>
<h6><i> Updated by Melanie Laporte, 20240512</i></h6>

