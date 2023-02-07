//This is the code for the Arduino connected to the pilot wastewater treatment system in Carmey-Avdat. It creates a data logging unit for gathering information from a different sensors (ORP, pressure and temperature). The ORP and pressure sensors were calibrated in the lab, while the temp sensor doesn't need to be calibrated (it comes with the DallasTemperature library). The data is then saved on a SD card in the form of a .csv file, which can then be explored by Excel or Pandas.

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>
#include <OneWire.h>
#include <DallasTemperature.h>

/* what arduino pins to connect to each of the LCD pins.
const int rs = 12, en = 11, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7); */
LiquidCrystal_I2C lcd(0x27,20,4);

int X = 0; // This is used to count loops

// how many milliseconds between grabbing data and logging it. 1000 ms is once a second
#define LOG_INTERVAL  1000 // mills between entries (reduce to take more/faster data)

// how many milliseconds before writing the logged data permanently to disk
// set it to the LOG_INTERVAL to write each time (safest)
// set it to 10*LOG_INTERVAL to write all data every 10 datareads, you could lose up to 
// the last 10 reads if power is lost but it uses less power and is much faster!
#define SYNC_INTERVAL 1000 // mills between calls to flush() - to write data to the card 
uint32_t syncTime = 0; // time of last sync()

#define ECHO_TO_SERIAL   1 // echo data to serial port
#define WAIT_TO_START    0 // Wait for serial input in setup()

// the digital pins that connect to the LEDs
#define redLEDpin 2
#define greenLEDpin 3

RTC_PCF8523 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;

// Temp sensor part:
#define ONE_WIRE_BUS 2 // Temperature sensor connected to Digital pin 2

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

 float Temperature = 0;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while(1);
  
}

void setup(void)
{
  Serial.begin(9600);
  sensors.begin();
  
  lcd.init();
  lcd.backlight();
  Serial.println();
  
  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);
  
#if WAIT_TO_START
  Serial.println("Type any character to start");
  while (!Serial.available());
#endif //WAIT_TO_START

  // initialize the SD card
  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
  Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LOGGER00.CSV"; // you can change the name here, keep in mind that the names generate a +1 extension every time (eg., LOGGER1, LOGGER2,LOGGER3 etc)
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = i/10 + '0';
    filename[7] = i%10 + '0';
    if (! SD.exists(filename)) {
      // only open a new file if it doesn't exist
      logfile = SD.open(filename, FILE_WRITE); 
      break;  // leave the loop!
    }
  }
  
  if (! logfile) {
    error("couldnt create file");
  }
  
  Serial.print("Logging to: ");
  Serial.println(filename);

  // connect to RTC
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
    Serial.println("RTC failed");
#endif  //ECHO_TO_SERIAL
  }
  

  logfile.println("minutes,Date & Time,ORP (mV), Pressure (mBar), Temperature (C)"); //Column names in the .csv file.
#if ECHO_TO_SERIAL
  Serial.println("seconds, Date & Time, ORP (mV), Pressure (mBar), Temperature (C)"); 
#endif //ECHO_TO_SERIAL
 
}

void loop(void)
{
  DateTime now;

  // delay for the amount of time we want between readings
  delay(LOG_INTERVAL);
  
  digitalWrite(greenLEDpin, HIGH);
  
  // log milliseconds since starting
  uint32_t m = millis();

#if ECHO_TO_SERIAL
  Serial.print(m/1000);         // seconds since start
  Serial.print(", ");  
#endif

  // fetch the time
  now = RTC.now();
  // log time

// Sensor setup:
// ORP analog read is set to analog pin 8
float ORPread = analogRead(A8);
float ORP = (((ORPread)-207.3)/0.4414); // this is according to the ORP controller monitor, NOT the ORP calibration solutions! make a new calibration curve if you prefer.

sensors.requestTemperatures(); // set to digital pin 2
Temperature = sensors.getTempCByIndex(0);


float Pressureread = analogRead(A9); // Pressure analog read is set to analog pin 9
float Pressure = (((Pressureread)-617.25)/0.3932); //this is according to the excel sheet for calibration i made with amit. make a new calibration curve for every new pressure sensor being used.

// LCD code:
  lcd.setCursor(0,0); //first line
  lcd.print(ORP);
  lcd.print(" mV");

  lcd.setCursor(0, 1); //second line 
  lcd.print(Temperature);
  lcd.print("C, ");

  lcd.print(Pressure);
  lcd.print("mB");

if (X%60 == 0) {
  logfile.print(m/60000);           // minutes since start
  logfile.print(",");    
  
  if (now.day()< 10)          //the next paragraph is for the timestamp
      logfile.print('0');
      logfile.print(now.day());
  logfile.print("-");
  if (now.month()< 10)
      logfile.print('0');
      logfile.print(now.month());
  logfile.print("-");
  logfile.print(now.year(), DEC);
  logfile.print(" ");
  if (now.hour()< 10)
      logfile.print('0');
      logfile.print(now.hour());
  logfile.print(":");
  if (now.minute()< 10)
      logfile.print('0');
      logfile.print(now.minute());
  logfile.print(":");
  if (now.second()< 10)
      logfile.print('0');
      logfile.print(now.second());


  logfile.print(", ");    // sensor data
  logfile.print(ORP);

  logfile.print(", ");    
  logfile.print(Pressure);

  if (X%600 == 0) {    //this is a count for ~10 minutes
    logfile.print(", ");
    logfile.print(Temperature);
    logfile.println();  
} else {
  logfile.println(); }
}


#if ECHO_TO_SERIAL
// it is not important to log data to the serial, but it helps testing the functionality while in the field.
  if (now.day()< 10)
      Serial.print('0');
      Serial.print(now.day());
  Serial.print("/");
  if (now.month()< 10)
      Serial.print('0');
      Serial.print(now.month());
  Serial.print("/");
  Serial.print(now.year(), DEC);
  Serial.print(" ");
  if (now.hour()< 10)
      Serial.print('0');
      Serial.print(now.hour());
  Serial.print(":");
  if (now.minute()< 10)
      Serial.print('0');
      Serial.print(now.minute());
  Serial.print(":");
  if (now.second()< 10)
      Serial.print('0');
      Serial.print(now.second());

#endif //ECHO_TO_SERIAL

  
#if ECHO_TO_SERIAL
  Serial.print(", ");   
  Serial.print(ORP);
  Serial.print(" mV");
#endif //ECHO_TO_SERIAL
  
  
#if ECHO_TO_SERIAL
  Serial.print(", ");   
  Serial.print(Pressure);
  Serial.print(" mBar");
#endif //ECHO_TO_SERIAL

#if ECHO_TO_SERIAL
  Serial.print(", ");   
  Serial.print(Temperature);
  Serial.print(" C");
  Serial.println();    
#endif //ECHO_TO_SERIAL

  digitalWrite(greenLEDpin, LOW);

/*
  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  */
  
  // blink LED to show we are syncing data to the card & updating FAT!
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW);

X=X+1;

}
