# moonrat
Moonrat: A second-generation portable incubator



## Motivation

By building a small, portable, intelligent incubator that can maintain constant elevated temperature, a variety of biological experiments and assays can be 
performed "in the field" without having access to a electric grid power.

## Origin

This is an offshoot of the [Rapid E. coli project](https://github.com/PubInv/rapid-e-coli). It is an attempt to build a better, smaller, more intelligent 
portable incubator that the Armadillo, described [elsewhere](http://jhe.ewb.org.au/index.php/jhe/article/view/127) and buildable from an [instructable](https://www.instructables.com/id/Portable-Petrifilm-Incubator-for-Inexpensive-In-Fi/).

## Current Design

Our current design use a "casette" which is bolted to the inside of the lid of a thermos bottle, shown below. Our circuit is on a breadboard, and we have not yet designed the enclosure for the electronics integrated into the cap or as a holder for the bottle.

Our analysis is preliminary, but our current tests show we at 25C ambient temperature we can hold the needed 35C incubation temperature with only 
(12V * 12V / 8 Ohms * 6% duty cycle * 48 hours) = 51.84 Watt Hours.  This will have to be supplied by a battery pack, which will be about the same size as the Thermos bottle.

![MoonratWithBreadBoard](https://user-images.githubusercontent.com/5296671/110479740-580de280-80ab-11eb-90c4-a859c0519b3d.png)


## Initial Design Approach

![Basic Structure of Portable Incubator (1)](https://user-images.githubusercontent.com/5296671/86680945-b470ad80-bfc4-11ea-9ef6-19c4f70aceba.png)

## References (Incomplete)

1. Petrifilms: [https://www.3m.com/3M/en_US/company-us/all-3m-products/~/ECOLICT-3M-Petrifilm-E-coli-Coliform-Count-Plates/?N=5002385+3293785155&rt=rud](https://www.3m.com/3M/en_US/company-us/all-3m-products/~/ECOLICT-3M-Petrifilm-E-coli-Coliform-Count-Plates/?N=5002385+3293785155&rt=rud)
1. EPA Guidelines: [https://19january2017snapshot.epa.gov/sites/production/files/2015-11/documents/drinking_water_sample_collection.pdf](https://19january2017snapshot.epa.gov/sites/production/files/2015-11/documents/drinking_water_sample_collection.pdf)
1. Original Incubator Paper - [http://jhe.ewb.org.au/index.php/jhe/article/view/127](http://jhe.ewb.org.au/index.php/jhe/article/view/127)
1. Construction of a Low-cost Mobile Incubator for Field and Laboratory Use [https://www.jove.com/t/58443/construction-low-cost-mobile-incubator-for-field-laboratory](https://www.jove.com/t/58443/construction-low-cost-mobile-incubator-for-field-laboratory)
1. Solar Powered Portable Culture
Incubator [https://www.jscimedcentral.com/Pediatrics/pediatrics-3-1063.pdf](https://www.jscimedcentral.com/Pediatrics/pediatrics-3-1063.pdf)


## Acknowledgements

Thanks to Dr. Sabia Abidi of Rice University for input and references.
