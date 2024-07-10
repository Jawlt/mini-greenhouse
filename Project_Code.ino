//Soil Moisture Section (the sensor when connected messes with the lcd and other parts of the cirucit causes nothing to work when it is plugged in, so I have commented the code for the soil out.)
//const int dry = 595; // value for dry sensor
//const int wet = 239; // value for wet sensor

//Water Section
#define sensorPower 8
#define sensorPin A0

int waterpumpPin = 2;
int value = 0; // Storing value for water level

//Heating Section
int heatingPin = 4;

//Fan Section
int fanPin = 5;

//Temperature and Humidity Section
#include "DHT.h"
#define DHTPIN 3     
#define DHTTYPE DHT11   

DHT dht(DHTPIN, DHTTYPE);

//LED Section
#include <FastLED.h>

#define LED_PIN     6
#define NUM_LEDS    60

CRGB leds[NUM_LEDS];

//RTC and LCD Section
#include <DS1302.h>
#include <Wire.h>  
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16, 2);
DS1302 rtc(10, 11, 12);

int TIME = 0;
const int hs = 0;
const int ms = 1;
int state1 = 0;
int state2 = 0;

Time t;
int OnHour = 6;
int OffHour = 20;

void setup()
{
  //LED Section
  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);

  //DHT11 Section
  dht.begin();
  
  //RTC and LCD Section
  // The following lines can be commented out to use 
  // the values already stored in the DS1302
  rtc.setDOW(FRIDAY);      // Set Day-of-Week to FRIDAY
  rtc.setTime(10, 0, 30);   // Set the time to 12:00:00 (24hr format)
  rtc.setDate(18, 3, 2022); // Set the date to August 6th, 2019 
  
  lcd.begin();
  Serial.begin(9600);
  pinMode(hs, INPUT_PULLUP);
  pinMode(ms, INPUT_PULLUP);

  //Fan Section
  pinMode(5, OUTPUT);
  
  //Heating Section
  pinMode(4, OUTPUT);

  //Water Section
  pinMode(sensorPower, OUTPUT);
  digitalWrite(sensorPower, LOW);
  pinMode(waterpumpPin, OUTPUT);
}

void loop()
{
  //RTC and LCD Section
  t = rtc.getTime();
  Serial.print(t.hour);
  Serial.print(":");
  Serial.print(t.min);
  Serial.print(":");
  Serial.println(t.sec);
  Serial.println("*********");

  int h = t.hour; //hour 24hr format
  int m = t.min; //minute
  int s = t.sec; //second
  int flag = h; //same as hour
  
  Serial.print("Flag:");
  Serial.println(flag);
  Serial.println("*********");
  lcd.setCursor(0,0);
  s = s + 1;
  lcd.print("TIME:" );
  lcd.print(h);
  lcd.print(":");
  lcd.print(m);
  lcd.print(":");
  lcd.print(s);
  if (flag < 12) lcd.print(" AM");
  if (flag == 12) lcd.print(" PM");
  if (flag > 12) lcd.print(" PM");
  if (flag == 24) flag = 0;
  if (s == 60)
  {
    s = 0;
    m = m + 1;
  }
  if (m == 60)
  {
    m = 0;
    h = h + 1;
    flag = flag + 1;
  }
  if (h == 13)
  {
    h = 1;
  }
  lcd.setCursor(0, 0);
  
  state1 = digitalRead(hs);
  if (state1 == 0)
  {
    h = h + 1;
    flag = flag + 1;
    if (flag < 12) lcd.print(" AM");
    if (flag == 12) lcd.print(" PM");
    if (flag > 12) lcd.print(" PM");
    if (flag == 24) flag = 0;
    if (h == 13) h = 1;
  }
  state2 = digitalRead(ms);
  if (state2 == 0)
  {
    s = 0;
    m = m + 1;
  }
  
  //LED Section
  lights();
  delay(2000);
  lcd.clear();
  
  //Temperature and Humidity Section
  dht11();
  delay(2000);
  lcd.clear();
  
  //Water Section
  waterlevel();
  delay(2000);

  //Soil Moisture Section
  //soil();
  //delay(2000);
}

/*
  void soil() {
  int soilVal = analogRead(A5);
  int perHumididy = map(soilVal, wet, dry, 100, 0);
  Serial.print(perHumididy);
  Serial.println("%"); 
} */

void lights() {
  if (t.hour >= OnHour && t.hour <= 20 ) {
    for(int i = 0; i <= 59; i++) {
    leds[i] = CRGB(25,25,25);
    FastLED.show();
    }
    lcd.setCursor(0,1);
    lcd.print("LIGHTS ON       "); 
  }
  else {
    for(int i = 59; i >= 0; i--) {
    leds[i] = CRGB(0,0,0);
    FastLED.show();
    }
    lcd.setCursor(0,1);
    lcd.print("LIGHTS OFF      "); 
  }
}

void dht11() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.println(temp);
  
  lcd.setCursor(0,0); 
  lcd.print("Temp: ");
  lcd.setCursor(6,0);
  lcd.print(temp);
  lcd.setCursor(11,0);
  lcd.print(" C   ");
  lcd.setCursor(0,1); 
  lcd.print("Hum: ");
  lcd.setCursor(5,1);
  lcd.print(hum);
  lcd.setCursor(10,1);
  lcd.print(" %    ");

  if(temp > 25 || hum > 70){
    analogWrite(fanPin, 200);
  }
  else {
    analogWrite(fanPin, 0);
  }
  if(temp < 25){
    analogWrite(heatingPin, 200);
  }
  else {
    analogWrite(heatingPin, 0);
  }
}

void waterlevel() {
  float hum = dht.readHumidity();
  float temp = dht.readTemperature();
  int level = readSensor();
  String wl = "";
  
  Serial.print("Water level: ");
  if(level < 20) {
    wl = "LOW";
  }
  else if (level <= 40 && level >= 20) {
    wl = "MEDIUM";
  }
  else {
    wl = "HIGH";
  }
  if(temp > 20 || hum > 70) {
     if(level >= 40) {
     analogWrite(waterpumpPin, 150);
     delay(1000);
     analogWrite(waterpumpPin, 0);
    }
  }
  else {
  analogWrite(waterpumpPin, 0);
  }
 
  Serial.print(level);
  Serial.println(" " + wl);
  
  lcd.setCursor(0,0); 
  lcd.print("Water Level: ");
  lcd.setCursor(0,1);
  lcd.print(wl);
  lcd.print(" ");
  lcd.print(level);
}

int readSensor() {
  digitalWrite(sensorPower, HIGH);
  delay(10);
  value = analogRead(sensorPin);
  digitalWrite(sensorPower, LOW);
  return value;
}
