

#include <AFMotor.h>
#include <Wire.h>    // I2C-Bibliothek einbinden
#include "RTClib.h"  // RTC-Bibliothek einbinden
 
RTC_DS1307 RTC;         // RTC Modul
AF_Stepper motor1(180,1);
AF_Stepper motor2(200,2);

void setup()
{
  // Initialisiere I2C  
  Wire.begin();
  // Initialisiere RTC
  RTC.begin();
  // Serielle Ausgabe starten 
  Serial.begin(9600);
  
  // Prüfen ob RTC läuft  
  if (! RTC.isrunning()) {
    
    // Aktuelles Datum und Zeit setzen, falls die Uhr noch nicht läuft
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  motor1.setSpeed(10);
  motor2.setSpeed(25);
}

void fuettern()  // Funktion Zellenradschleuse

{
  Serial.println ("Zellenrad");
  motor1.step(900,BACKWARD,INTERLEAVE); 
  motor1.release();
  delay (2000);
}


void werfen()

{
  Serial.println ("Werfen");
  motor2.step(1200,FORWARD,SINGLE);
  motor2.release();
  delay (15000);// 15 sec warten, bis nächsterFütterunszyclus
}

int anzahl_auslesen()

{
  // DIP-Schalter auslesen
  
  return 5;
  
}

void show_time (DateTime datetime) {
  // Stunde:Minute:Sekunde
  if(datetime.hour()<10)Serial.print(0);
  Serial.print(datetime.hour(),DEC);
  Serial.print(":");
  if(datetime.minute()<10)Serial.print(0);
  Serial.print(datetime.minute(),DEC);
  Serial.print(":");
  if(datetime.second()<10)Serial.print(0);
  Serial.println(datetime.second(),DEC);
}

void wait_until_time (int hour, int minute, int second) {
  DateTime now;
  
  do {
    now = RTC.now();
    show_time (now);
    delay (500);
    
    Serial.print ("Wait for ");
    if(hour<10)Serial.print(0);
    Serial.print(hour,DEC);
    Serial.print(":");
    if(minute<10)Serial.print(0);
    Serial.print(minute,DEC);
    Serial.print(":");
    if(second<10)Serial.print(0);
    Serial.println(second,DEC);
    
  } while (  (now.hour() != hour)
          || (now.minute() != minute)
          || (now.second() != second)
          );
  delay(1000);
}

struct FeedingTime {
  int hour;
  int minute; 
  int second;
};

#define FEED_NUM 5                               // insgesamt ein Array mit 5 Einträgen

int find_next (struct FeedingTime *times)       // * bedeutet "pointer at times" in dem Array
{
  DateTime now = RTC.now();
  int j;
  for (j = 0; j < FEED_NUM; j++)
  {  
    if (times[j].hour > now.hour()) {                        // wenn die gewünschte Fütterungsstunde größer ist, als die Realzeit, dann 
      return j;                                              // gehe zu j und erhöhe um 1
      
    } else if (times[j].hour == now.hour()) {               // sonst, wenn die gewünschte Fütterungsstunde exakt der Realzeit entspricht,                                    
        if (times[j].minute > now.minute()) {               // und die Gew. Fütterungsminute größer als die reale Minute ist, 
        return j;                                          // gehe zu j und erhöhe um 1
      } else if (times[j].minute == now.minute()) {        // sonst, wenn die gewünschte Fütterungsminute exakt der Realzeit entspricht,
        if (times[j].second > now.second()) {              // und wenn die gewünschte Fütterungssekunde größer als die Realzei ist,
          return j;                                        // gehe zu j und erhöhe um 1
        }
      }
    } 
  }
  return 0;
}

void loop()
{
  int i,j;
  int anzahl;
  struct FeedingTime times[FEED_NUM] = {{6,30,0}, {7,30,0}, {8,40,0}, {9,00,0}, {13,29,0}}; //Array der Fütterungszeiten
  
  
  for (j = find_next (times); j < FEED_NUM; j++)
  {
    Serial.print ("Next time: ");
    Serial.println (j, DEC);
    wait_until_time (times[j].hour, times[j].minute, times[j].second);
    anzahl=anzahl_auslesen();
    for (i=0; i < anzahl; i++)
    {
      fuettern();
      werfen();
    }
  } 
}
 


