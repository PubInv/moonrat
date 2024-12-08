#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Arduino.h>

// Set controller type here. Use R3 xor R4
#define R3
//#define R4

//OLED VARIABLES
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define BUZZER_PIN 9
#define HEATER_PIN 10

const byte tempPin = A0; // connect TPM36 to 3.3volt A0 and (not-shared) ground
//float calibration = 0.1039; // calibrate temp by changing the last digit(s) of "0.1039"
//float calibration = 0.0045703125; // For ~4.68 voltage supply
float calibration;// = 0.00486328125; // For ~4.98 voltage supply

float tempC;

long readVcc() {

  /*  The code must be improved by addign the "#ifdef at the beggining"  */
  //ARDUINO_AVR_UNO
#ifdef R3
  // ------------------------------------------------ Use this part to the R3 --------------------------------------
  // Configure the ADC to use the reference channel -> 1.1V
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2);  // Espera a que la referencia de voltaje se estabilice
  ADCSRA |= _BV(ADSC);  // Inicia la conversión de ADC
  while (bit_is_set(ADCSRA, ADSC));

  // Read ADC value
  uint8_t low = ADCL;
  uint8_t high = ADCH;
  long result = (high << 8) | low;

  // // Vcc in milivoltios
  long vcc = 1126400L / ADC;  // Utiliza 1.1 * 1024 * 1000
  //long vcc = (1.1 * 1024 * 1000) / result;  // Utiliza 1.1 * 1024 * 1000

  //#elif(defined(ARDUINO_AVR_UNOR4))
#elif defined(R4)
  //Assume R4
  // ------------------------------------------------ Use this part to the R4 --------------------------------------
  analogReference(AR_DEFAULT);  // Usa la referencia interna
  delay(10);  // Espera a que la referencia se estabilice
  float sensorValue = map(analogReference(), 0, 5.0, 0, 255.0);  // Usa la referencia interna
  long vcc = (1000 * 1.1 * 1023) / sensorValue;

#else
#error Serial.println("Unsupported board selection.");
#endif
  return vcc;
}

void setup() {
  // Set heater and buzzer off, required for version 1 PWAs.
  pinMode(HEATER_PIN, OUTPUT);
  digitalWrite(HEATER_PIN, LOW);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  //#if defined(ARDUINO_AVR_UNOR4)
#if defined(R4)
  //Assume R4
  analogReadResolution(10); // R4
#endif
  Serial.begin(115200);

#if defined(R4)
  //Assume R4
  analogReference(AR_DEFAULT); // use internal 1.1volt Aref ------ R4
#else
  analogReference(DEFAULT); // use internal 1.1volt Aref --------- R3
#endif
 
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("Error while initializing OLED"));
    for (;;)
      ;
  }
}

void loop() {
  float vcc = readVcc() / 1000.0;
  Serial.print("\nVcc: ");
  Serial.print(vcc);
  Serial.println(" V");
  int Volt = analogRead(tempPin);
  //float ref_voltage = analogReference(DEFAULT);
  //tempC = (Volt * (500.0/ 1024.0)-50) ; //T6G Sensor
  calibration = vcc / 1024.0;
  tempC = (analogRead(tempPin) * calibration * 100) - 50.0;
  Serial.print("Temperature:  ");
  Serial.print(tempC, 1); // one correct decimal place is all you get
  Serial.println(" C");
  Serial.print("Voltage: ");
  Serial.print(Volt * calibration);
  //Serial.print("\nReference Voltage: ");
  //Serial.print(ref_voltage);
  Serial.println(); //Make space at end of serial report

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 2);
  display.print("Temp:");
  display.print(tempC);
  display.println("C ");
  display.display();

  delay(1000); // use a non-blocking delay when combined with other code
}
