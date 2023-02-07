//This is the code for the Arduino connected to the filtration experiment system in Lab 25. It creates a data logging unit for gathering information from a pressure sensor and a scale. The data is then saved on a SD card in the form of a .csv file, which can then be explored by Excel or Pandas.
// Additionally, this contains a code to prevent overflowing of the permeate cup on the scale, by engaging a second peristaltic pump whenever the scale shows >700g.

#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include "RTClib.h"
#include <LiquidCrystal_I2C.h>

 /* what arduino pins to connect to each of the LCD pins.
const int rs = 12, en = 11, d4 = 6, d5 = 5, d6 = 4, d7 = 3;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);*/
LiquidCrystal_I2C lcd(0x27,20,4);

int Relaypin = 8; // Relay pin for peristaltic pump

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

RTC_DS1307 RTC; // define the Real Time Clock object

// for the data logging shield, we use digital pin 10 for the SD cs line
const int chipSelect = 10;

// the logging file
File logfile;

void error(char *str)
{
  Serial.print("error: ");
  Serial.println(str);
  
  // red LED indicates error
  digitalWrite(redLEDpin, HIGH);

  while(1);
  
}

//String WeightString;

void setup(void)
{
  //Serial.begin(9600);
  Serial.begin(4800,SERIAL_7E1); // this is for the scale for weight measurements (can be changed within the scale itself)
  pinMode(Relaypin,OUTPUT);
  digitalWrite(Relaypin, HIGH);


  //sensors.begin();
  
  lcd.init();
  lcd.backlight();
  //Serial.println();
  
  // use debugging LEDs
  pinMode(redLEDpin, OUTPUT);
  pinMode(greenLEDpin, OUTPUT);
  
#if WAIT_TO_START
 /* Serial.println("Type any character to start");
  while (!Serial.available());*/
#endif //WAIT_TO_START 

  // initialize the SD card
//  Serial.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);
  
  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    error("Card failed, or not present");
  }
 // Serial.println("card initialized.");
  
  // create a new file
  char filename[] = "LABLog00.CSV"; // you can change the name here, keep in mind that the names generate a +1 extension every time (eg., LOGGER1, LOGGER2,LOGGER3 etc)
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
  
 /* Serial.print("Logging to: ");
  Serial.println(filename); */

  // connect to RTC
  Wire.begin();  
  if (!RTC.begin()) {
    logfile.println("RTC failed");
#if ECHO_TO_SERIAL
/*    Serial.println("RTC failed"); */
#endif  //ECHO_TO_SERIAL 
  }
  

  logfile.println("seconds,Date & Time, Pressure (mBar), Weight (g)"); //Column names in the .csv file.
#if ECHO_TO_SERIAL
//  Serial.println("seconds, Date & Time, Pressure (mBar), Weight (g)"); 
#endif //ECHO_TO_SERIAL 


}

void loop(void)
{
  DateTime now;

  // delay for the amount of time we want between readings
  //delay((LOG_INTERVAL -1) - (millis() % LOG_INTERVAL));
  delay(LOG_INTERVAL);
  
  digitalWrite(greenLEDpin, HIGH);
  
  // log milliseconds since starting
  uint32_t m = millis();


  // fetch the time
  now = RTC.now();
  // log time

// Sensor setup:

float Pressureread = analogRead(A0); // Pressure analog read is set to analog pin 0
float Pressure = (((Pressureread)-616.68)/805.68)*1000; //this is according to the excel sheet for calibration i made with Gal*/

float Weight = Serial.parseFloat(); //This parses/extracts a float from the string received from the scale (in ASCII). use .parseInt instead if you don't want the numbers after the decimal point.

// LCD code:
  lcd.setCursor(0,0); //first line
  /*lcd.print("    "); //If you need to show the current time on screen. This is not important, because time is saved onto the SD card anyway.
  if (now.hour()< 10)
      lcd.print('0');
      lcd.print(now.hour());
  lcd.print(":");
  if (now.minute()< 10)
      lcd.print('0');
      lcd.print(now.minute());
  lcd.print(":");
  if (now.second()< 10)
      lcd.print('0');
      lcd.print(now.second()); */
if ((Weight) > 0) { 
  lcd.print(Weight);
  lcd.print(" g"); 
  lcd.setCursor(0,1);
  lcd.print(Pressure);
  lcd.print(" mBar");
  
//if (X%10 == 0) { // interval of logging to SD card. currently set to every 10 seconds. Either use this, or set the scale to send information every few seconds (if it has that option)

  logfile.print(m/1000);           // seconds since start
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
//}
  
  logfile.print(", ");    
  logfile.print(Pressure);
  logfile.print(", ");
  logfile.println(Weight); 
  

#if ECHO_TO_SERIAL
//if ((Weight) > 0) { // This section is here if, for some reason, you want to log the information into the Arduino Serial. Notice that if some pin is connected to the RX pin, it will read that information instead. disable this to prevent discomunication with the scale (connected to the RX/TX).
/*  Serial.print(m/1000);         // seconds since start
  Serial.print(", ");  
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

  Serial.print(", ");   
  Serial.print(Pressure);
  Serial.print(" mBar,");
  Serial.print(Weight); 
  Serial.println(" g"); }
}
  */
#endif //ECHO_TO_SERIAL 


  digitalWrite(greenLEDpin, LOW);


  // Now we write data to disk! Don't sync too often - requires 2048 bytes of I/O to SD card
  // which uses a bunch of power and takes time
  if ((millis() - syncTime) < SYNC_INTERVAL) return;
  syncTime = millis();
  
  
  // blink LED to show we are syncing data to the card & updating FAT!
  digitalWrite(redLEDpin, HIGH);
  logfile.flush();
  digitalWrite(redLEDpin, LOW); 
  }
  
  if ((Weight) > 700) { // This code activates the peristaltic pump connected to the relay, which prevents overflowing of the permeate.
   digitalWrite(Relaypin, LOW); }
  if ((Weight) < 200 && (Weight) > 0) {
   digitalWrite(Relaypin, HIGH); }
   
}
