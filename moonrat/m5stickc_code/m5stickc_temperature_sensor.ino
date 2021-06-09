#include "M5StickC.h"
int sensorPin = 36;
void setup() {
  M5.begin(115200);
  M5.Lcd.setRotation(3);
  pinMode(sensorPin,INPUT);
  //pinMode(25, INPUT_PULLDOWN);
  // put your setup code here, to run once:

}

void loop() {
 M5.Lcd.fillScreen(BLACK);
 M5.Lcd.setCursor(0, 0, 2);
 M5.Lcd.setTextColor(GREEN);
 
 //getting the voltage reading from the temperature sensor
 int reading = analogRead(sensorPin);  
 Serial.println(reading);
 
 // converting that reading to voltage, for 3.3v arduino use 3.3
 float voltage = reading * 3.3;
 voltage /= 4096.0; 
 Serial.println(voltage);
 
 // print out the voltage
 //Serial.print(voltage); Serial.println(" volts");
 //M5.Lcd.print(voltage); M5.Lcd.println(" volts");
 
  
 // now print out the temperature
 float temperatureC = (voltage - 0.5) * 100 ;  //converting from 10 mv per degree wit 500 mV offset
                                               //to degrees ((voltage - 500mV) times 100)
 Serial.print(temperatureC); Serial.println(" degrees C");
 M5.Lcd.print(temperatureC); M5.Lcd.println(" degrees C");
 
 // now convert to Fahrenheit
 float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
 Serial.print(temperatureF); Serial.println(" degrees F");
 M5.Lcd.print(temperatureF); M5.Lcd.println(" degrees F");

 M5.update();
 
 delay(2000);                                     //waiting a second
}
