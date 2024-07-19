# Can21Oz "Incubator"
An experiment in measuring temperature characteristics of a heated system.  
![image](https://github.com/PubInv/moonrat/assets/5836181/f278499d-bf72-4868-a73a-109042c12807)

## The incubator, Can21Oz

### Construction
It is fabricated out of a 21oz can which held fruit.  
The can empty weight is 62gm assumed to be steel.  
The can when full of water is 600 to 649 gm.  
The water mass is therefor about 587 gm  
The sides of the can are insulated with a layer of bubble wrap about 3/8 inch think.  
The top is open.  
The can is sitting on a 3 1/2" thick 89 mm polystyrine foam block.  

#### Can Dimensions
Radius = (3 3/8)/2 = 1.6875" = 42.9mm  
Height = 4 1/2 " =  114.3mm  
Volume = Pi * R^2 * H = Pi* 42.9^2 * 114.3 = 331,000 mm^3  
Surface Area = (2 * Pi * R^2) + (2 * pi * R * H) = 11,560 + 30,810 = 42,370 mm^2  

#### System Heater
A relay switched, 120VAC, Immersion heater.   
https://www.amazon.com/dp/B01M0Q84BR?psc=1&ref=ppx_yo2ov_dt_b_product_details  
120 volts and 300 watts. UL approved.  

### The System
| This "incubator" is more like a hot water bath.                                                 	| The 120V AC Heating Element                                                                                                        	|   	|
|-------------------------------------------------------------------------------------------------	|----------------------------------------------------------------------------------------------------------------------------	|---	|
| ![image](https://github.com/PubInv/moonrat/assets/5836181/0e5a92aa-5a8e-4679-949f-f9b827926b7d) 	| ![image](https://github.com/ForrestErickson/moonrat_atForrestErickson/assets/5836181/e68eafa2-4cbd-4304-98c3-a4ee8701be33) 	|   	|

## Instruments, Temperature Measurement
Three MAX31850 thermocouple amplifiers with K type thermocouples immersed into the water in the 21Oz can.  
They are held to the side of the can with a wooden cloths pin.  
The exact location of the thermo couple junctions was not well constrained.  
Measurement displayed by a Cumulocity dash board.  

## Experiments
### Near Boiling, Warm Up / Cool Down
**Heat Up**  
At 4:02 PM EST or so plug in heater.  
![image](https://github.com/ForrestErickson/moonrat_atForrestErickson/assets/5836181/30312920-d633-4b0e-8ee3-0cbe05e94e5c)
At about 4:04 the temperature had risen about 23 degrees or about 1/e of the way from 19 to 95C.  
Time rise would be about 120 seconds **but this curve is not a full exponential** so the rise time is longer.

**Cool Down**  
Unplug at about 4:12 PM EST.  
From a peak of about 98 to 19 is 79. About 1/3 of that is 24 so at 98-24 about 75 C which happnes at about 4:25 PM.  
![image](https://github.com/ForrestErickson/moonrat_atForrestErickson/assets/5836181/2fd799e3-9bc2-4bc7-afba-f694d80a94c1)  
4:12 from 4:25 is 13 minutes or 780 seconds for about one time constant.



### One Minute Rectangular Heat / Cool
Heat with a rectantular profile at the full power of the immersion heater.
At 10:34💯 AM EST connect heater to 120V AC.  
At 10:35:00 AM EST disconnect heater from 120V AC.  

**Heat Up Detail**
Just before or at turn on  
![image](https://github.com/ForrestErickson/moonrat_atForrestErickson/assets/5836181/5e4af454-11af-4da9-91c8-9da8ee0efb04)
About one minute later  
![image](https://github.com/ForrestErickson/moonrat_atForrestErickson/assets/5836181/7736c2d5-e13f-4237-b1b6-ff9e35a6f72f)

**Cool Down Detail**  
![image](https://github.com/ForrestErickson/moonrat_atForrestErickson/assets/5836181/a4e5357f-6fe7-475f-ab2e-df8bfcb7b5ca)

The tempaterature at the start was the ambient of about 18 C.  
The peak temperature on the thermocouple labeled Getter reached 56.25 C.  
![image](https://github.com/ForrestErickson/moonrat_atForrestErickson/assets/5836181/cfd0712b-b470-4f93-a224-f88abcc84011)

On the cool down, the 1/e (One time constant) temperature would be at about 42 degrees which happened at about 10:38. From the 10:35 turn off this is three minutes or 180 seconds.  
![image](https://github.com/ForrestErickson/moonrat_atForrestErickson/assets/5836181/d9d8957a-cc0b-47fa-bfa0-bbf31a04c958)

## Conclusion.
The system has significantly faster rise time than fall time.  
The time constant for cooling down from near boiling temprature was 780 seconds while the time constant from a peak of 52.25 was only 180 seconds.  
I hypothesize that, for the one minute heating experiment, the heat was not fully distributed through the water in the sample (it was not in equilibrium).











