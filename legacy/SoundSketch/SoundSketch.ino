void setup() {
  // put your setup code here, to run once:

}

boolean FirstTone = false;
boolean SecondTone = false;
long startFirst;
long startSecond;

void loop() {

  boolean alarm = true;

  if (alarm) {
    FirstTone = true;
    startFirst = millis();
    tone(SPEAKER_PIN,440,500);
    alarm = false;
  }

  if (FirstTone && millis() > (startFirst + 500)) {
    SecondTone = true;
    tone(SPEAKER_PIN,220,500);
    startSecond = millis();
    FirstTone = false;
  }

  if (SecondTone && millis() > (startSecond + 500)) {
    noTone(SPEAKER_PIN);
    FirstTone = false;
    SecondTone = false;
  }
}
