#include <LiquidCrystal.h>
#include<Wire.h>
#define USE_ARDUINO_INTERRUPTS true

LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
const int PulseWire = A1;
int threshold;
float sensorValue = 0;
int count = 0;
unsigned long starttime = 0;
int bpm = 0;
boolean counted = false;
int prevMax = 0;
int prevMin = 1000;
int mini;
int maxi;
int prevBPM;
int steps;
const int MPU=0x68;
const float thresholdA = 6000;
float xavg, yavg, zavg;
boolean flag = true;

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPU);
  Wire.write(0x6B);
  Wire.write(0);
  Wire.endTransmission(true);
  lcd.begin(16, 2);
  Serial.begin(9600);
  analogWrite(6, 100);
  calibrateA();
}

void calibrateA() {
  float xval = 0;
  float yval = 0;
  float zval = 0;
  float sumX = 0;
  float sumY = 0;
  float sumZ = 0;
  for (int i = 0; i < 100; i++) {
    Wire.beginTransmission(MPU);
    Wire.write(0x3B);
    Wire.endTransmission(false);
    Wire.requestFrom(MPU, 14 ,true);
    xval = Wire.read()<<8|Wire.read();
    yval = Wire.read()<<8|Wire.read();
    zval = Wire.read()<<8|Wire.read();
    Wire.read()<<8|Wire.read();
    Wire.read()<<8|Wire.read();
    Wire.read()<<8|Wire.read();
    Wire.read()<<8|Wire.read();
    sumX = xval + sumX;
    sumY = yval + sumY;
    sumZ = zval + sumZ;
  }
  delay(100);
  xavg = sumX / 100;
  yavg = sumY / 100;
  zavg = sumZ / 100;
}

void calibrate () {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrating");
  starttime = millis();
  while(millis() < starttime + 10000){
  sensorValue = analogRead(PulseWire);
  Serial.println(sensorValue);
  if (sensorValue > 20 && millis() > starttime + 5000) {
  maxi = max(sensorValue, prevMax);
  mini = min(sensorValue, prevMin);
  prevMax = maxi;
  prevMin = mini;
  }
  }
  threshold = (prevMax + prevMin) / 2;
  prevMax = 0;
  prevMin = 1000;
  }
  int getData(){
  float totVect = 0;
  float aX, aY, aZ;
  float sumVect = 0;
  starttime = millis();
  while (millis() < starttime + 10000) {
  sensorValue = analogRead(PulseWire);
  if (sensorValue < 20 || prevBPM > 200){
  calibrate();
  starttime = millis();
  }
  Serial.println(sensorValue);
  if (sensorValue > threshold && !counted) {
  count++;
  counted = true;
  } else if (sensorValue < threshold && counted){
  counted = false;
  }
  maxi = max(sensorValue, prevMax);
  mini = min(sensorValue, prevMin);
  prevMax = maxi;
  prevMin = mini;
  Serial.println(count);
  Wire.beginTransmission(MPU);
  Wire.write(0x3B);
  Wire.endTransmission(false);
  Wire.requestFrom(MPU, 14 ,true);
  aX = Wire.read()<<8|Wire.read();
  aY = Wire.read()<<8|Wire.read();
  aZ = Wire.read()<<8|Wire.read();
  Wire.read()<<8|Wire.read();
  Wire.read()<<8|Wire.read();
  Wire.read()<<8|Wire.read();
  Wire.read()<<8|Wire.read();
  totVect = sqrt(((sq(aX - xavg) + sq(aY - yavg) + sq(aZ - zavg))));
  if (sumVect == 0) {
  sumVect = totVect;
  } else {
  sumVect = (totVect + sumVect) / 2;
  }
  if (sumVect > thresholdA && flag) {
  steps++;
  flag = false;
  } else if (sumVect < thresholdA && !flag){
  flag = true;
  }
  if (steps < 0) {
  steps = 0;
  }
  lcd.setCursor(0, 1);
  lcd.print("Steps: ");
  lcd.print(steps);
  }
  calibrateA();
  threshold = (prevMax + prevMin) / 2;
  prevMax = 0;
  prevMin = 1000;
  return count * 6;
}

void loop() {
  lcd.setCursor(0, 0);
  lcd.print("BPM: ");
  if (prevBPM == 0) {
  lcd.print(bpm);
  } else {
  lcd.print(prevBPM);
  }
  prevBPM = (bpm + prevBPM) / 2;
  lcd.setCursor(0, 1);
  lcd.print("Steps: ");
  lcd.print(steps);
  sensorValue = analogRead(PulseWire);
  bpm = getData();
  lcd.setCursor(0, 0);
  lcd.print("BPM: ");
  lcd.print(bpm);
  delay(100);
  lcd.clear();
  count = 0;
}
