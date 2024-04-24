This folder is legacy files and folder for Moonrat prior to January 2023.  
See each folder for fuller explination.
# The Legacy README.md 
<hr>  

# Tote-Size Portable Incubator for Rapid Field Work
Aliases used by other developers include Moonrat, Rice MiniCubator, and Rice Petri FI. 


# 2020 Status
A single device was hand fabricated by Public Invention Volunteers in about 2020.
This device was field tested in Tanzania.
This 2020 device became the starting point for further development of the Moonrat II starting in 2023

## Motivation

By building a small, portable, intelligent incubator that can maintain constant elevated temperature, a variety of biological experiments and assays can be
performed "in the field" without having access to a electric grid power.

## Origin

This is an offshoot of the [Rapid E. coli project](https://github.com/PubInv/rapid-e-coli). It is an attempt to build a better, smaller, more intelligent
portable incubator that the Armadillo, described [elsewhere](http://jhe.ewb.org.au/index.php/jhe/article/view/127) and buildable from an [instructable](https://www.instructables.com/id/Portable-Petrifilm-Incubator-for-Inexpensive-In-Fi/).

## Hand Built 2020 Design

Our current (2020) design uses a soup thermos to have good insulation. 3D printed parts hold the Petrifilms inside the thermos and way from the metal edges.
Additional parts hold the heating cloth and thermometer, which our powered and sensed by an external Arduino.

<a
href="https://user-images.githubusercontent.com/5296671/130527922-66cce437-a89f-4815-a980-bc79295f1a5a.JPG"><img
src="https://user-images.githubusercontent.com/5296671/130527922-66cce437-a89f-4815-a980-bc79295f1a5a.JPG"
alt="https://user-images.githubusercontent.com/5296671/130527922-66cce437-a89f-4815-a980-bc79295f1a5a.JPG"
width="400">
IMG-2511
</a> 

Our previous design use a "casette" which is bolted to the inside of the lid of a thermos bottle, shown below. Our circuit is on a breadboard, and we have not yet designed the enclosure for the electronics integrated into the cap or as a holder for the bottle. We use a 3D printed bracket to hold the Thermos bottle. We believe this is the most compact design which will be thermally efficient.

### 2020 Design Thermos Bottle in Bracekt With Battery in Place
<a
href="./moonrat/3D_printed_part_pictures/6C0C779D-602B-4875-BB12-1951AA8B21C5.jpg"><img
src="./moonrat/3D_printed_part_pictures/6C0C779D-602B-4875-BB12-1951AA8B21C5.jpg"
alt="./moonrat/3D_printed_part_pictures/6C0C779D-602B-4875-BB12-1951AA8B21C5.jpg"
width="400">
Thermos Bottle In Bracket With Battery
</a> 

### Bracket by Itself
<a
href="./moonrat/3D_printed_part_pictures/9DBF69BE-EAB6-436A-977B-694C6931730C.jpg"><img
src="./moonrat/3D_printed_part_pictures/9DBF69BE-EAB6-436A-977B-694C6931730C.jpg"
alt="./moonrat/3D_printed_part_pictures/9DBF69BE-EAB6-436A-977B-694C6931730C.jpg"
width="400">
Bracket by itself
</a> 


Our analysis is preliminary, but our current tests show we at 25C ambient temperature we can hold the needed 35C incubation temperature with only
(12V * 12V / 8 Ohms * 6% duty cycle * 48 hours) = 51.84 Watt Hours.  This will have to be supplied by a battery pack, which will be about the same size as the Thermos bottle.

<a
href="https://user-images.githubusercontent.com/5296671/110479740-580de280-80ab-11eb-90c4-a859c0519b3d.png"><img 
src="https://user-images.githubusercontent.com/5296671/110479740-580de280-80ab-11eb-90c4-a859c0519b3d.png"
alt="https://user-images.githubusercontent.com/5296671/110479740-580de280-80ab-11eb-90c4-a859c0519b3d.png"
width="400">
MoonratWithBreadBoard
</a> 

## Initial Design Approach

<a
href="https://user-images.githubusercontent.com/5296671/86680945-b470ad80-bfc4-11ea-9ef6-19c4f70aceba.png"><img style="border:5px double black"
src="https://user-images.githubusercontent.com/5296671/86680945-b470ad80-bfc4-11ea-9ef6-19c4f70aceba.png"
alt="https://user-images.githubusercontent.com/5296671/86680945-b470ad80-bfc4-11ea-9ef6-19c4f70aceba.png"
width="400">
Basic Structure of Portable Incubator (1)
</a> 


## References (Incomplete)

1. Petrifilms: [https://www.3m.com/3M/en_US/company-us/all-3m-products/~/ECOLICT-3M-Petrifilm-E-coli-Coliform-Count-Plates/?N=5002385+3293785155&rt=rud](https://www.3m.com/3M/en_US/company-us/all-3m-products/~/ECOLICT-3M-Petrifilm-E-coli-Coliform-Count-Plates/?N=5002385+3293785155&rt=rud)
1. EPA Guidelines: [https://19january2017snapshot.epa.gov/sites/production/files/2015-11/documents/drinking_water_sample_collection.pdf](https://19january2017snapshot.epa.gov/sites/production/files/2015-11/documents/drinking_water_sample_collection.pdf)
1. Original Incubator Paper - [http://jhe.ewb.org.au/index.php/jhe/article/view/127](http://jhe.ewb.org.au/index.php/jhe/article/view/127)
1. Construction of a Low-cost Mobile Incubator for Field and Laboratory Use [https://www.jove.com/t/58443/construction-low-cost-mobile-incubator-for-field-laboratory](https://www.jove.com/t/58443/construction-low-cost-mobile-incubator-for-field-laboratory)
1. Solar Powered Portable Culture
Incubator [https://www.jscimedcentral.com/Pediatrics/pediatrics-3-1063.pdf](https://www.jscimedcentral.com/Pediatrics/pediatrics-3-1063.pdf)


## Acknowledgements
Thanks to Dr. Sabia Abidi of Rice University for input and references.

<hr>
<h6><i> This file created by Melanie Laporte and F. Lee Erickson, 20231217</i></h6>
<h6><i> Udated by copying much of the original landing page README.md into this document the by F. Lee Erickson, 20240424</i></h6>
