
///////////////////////////////////////////////////////////////////
// Foosball score counter with infrared sensors and TM1638 display
// Copyright Vincent Gerris
// This code is available under the GPL V3 license
// have fun!

#include "TM1638.h"
#include "EEPROM.h"
#include "AnythingEEPROM.h"
#include <inttypes.h>

#define EEPROM_OFFSET  100

#define KEY_P1         1 << 0
#define KEY_P2         1 << 1
#define KEY_MAX_SCORE  1 << 2
#define KEY_VIER       1 << 3
#define KEY_UNDO       1 << 4
#define KEY_RESET      1 << 5
#define KEY_DECR       1 << 6
#define KEY_INCR       1 << 7

struct SettingsStruct {
  unsigned char  max_score;
  //unsigned char  upper_threshold;
  //unsigned short sensorAtrigger;

} settings;

unsigned long debounce_time = 1500;

void read_settings() {
  EEPROM_readAnything(EEPROM_OFFSET, settings);
  if (settings.max_score == 0xff) settings.max_score = 10;
}

TM1638 display(/*dio*/ 4, /*clk*/ 5, /*stb0*/ 3);

char idletext[9] = "--------";

void display_text (char* text, boolean keep = true) {
  display.setDisplayToString(text);
  if (keep) strcpy(idletext, text);
}

void display_numtext (unsigned short num, char* text, boolean keep = true) {
  char numstr[9] = "";
  itoa(num, numstr, 10);
  char str[9] = "        ";
  byte width = 4; //set a value
  strcpy(&str[width - strlen(numstr)], numstr);
  strcpy(&str[width], "    ");
  strcpy(&str[8 - strlen(text)], text);
  display_text(str, keep);
}

void restore_display () {
  display_text(idletext, false);
}

void save_settings() {
  EEPROM_writeAnything(EEPROM_OFFSET, settings);
}

int scoreA = 0;
int scoreB = 0;
int cycles = 0;

unsigned long restore_time = 0;
boolean settingschanged = false;
unsigned long key_debounce = 0;
char txt_buffer[10] = "";


void setup () {
  Serial.begin(57600);
  Serial.println("Program Starting"); 
  pinMode(A1, INPUT);
  pinMode(A2, INPUT);
  pinMode(13, OUTPUT);
  pinMode(2, INPUT);
  digitalWrite(2, HIGH);
  read_settings();
}

void loop () {
  
  unsigned short sum = 0;
  unsigned short ratio = 0;
 if (cycles == 0) {
   display_text(" ready  ");
   delay(1000);
   display_text("        ");
   delay(500);
   display_text(" ready  ");
   delay(1000);
   display_text("        ");
   delay(500);
   display_text(" start  ");
   delay(500);
   sprintf(txt_buffer, "  %02d  %02d", scoreA, scoreB);
   display_text(txt_buffer);
 }
 cycles = 1;
   unsigned short sumA = 0;
   unsigned short sumB = 0;
   int number_of_polls = 5; //how many times the analog input is read
       for (byte i = 0; i < number_of_polls; i++) {
            sumA += analogRead(A1);
                //Serial.println(sumA);
            sumB += analogRead(A2);
                //Serial.println(sumB);
        }
       
       int sensorA1 = sumA / number_of_polls;
       int sensorA2 = sumB / number_of_polls;

       if (sensorA1 > 1007 && scoreA < settings.max_score && scoreB < settings.max_score) {
         scoreA++;
         //Serial.print("Sensor 1 Goallllll!");
         //Serial.println(sensorA1);
         display_text("gOAL P1 ");
         delay(1000);
         display_text("        ");
         delay(500);
         display_text("gOAL P1 ");
         delay(1000);
         display_text("        ");
         if (scoreA == settings.max_score) {
           display_text("P1 rUlEZ");
           delay(3000);
         }
         sprintf(txt_buffer, "  %02d  %02d", scoreA, scoreB);
         display_text(txt_buffer);
         
       }
       if (sensorA2 > 1011 && scoreA < settings.max_score && scoreB < settings.max_score) {
         scoreB++;
         //Serial.print("Sensor 2 Goallllll! ");
         //Serial.println(sensorA2);
         display_text("gOAL P2 ");
         delay(1000);
         display_text("        ");
         delay(500);
         display_text("gOAL P2 ");
         delay(1000);
         display_text("        ");
         if (scoreB == settings.max_score) {
           display_text("P2 rUlEZ");
           delay(3000);
         }
         sprintf(txt_buffer, "  %02d  %02d", scoreA, scoreB);
         display_text(txt_buffer);

       }
       //sprintf(txt_buffer, "  %02d  %02d", scoreA, scoreB);
       //display_text(txt_buffer);  
       //Serial.print("Sensor 2 value: ");
       //Serial.println(sensorA2);
       
       
    byte keys = display.getButtons();
       
    if (keys) {
    restore_time = millis() + 2000;
    if (!key_debounce) {
      if (keys == (KEY_P1  | KEY_DECR)) scoreA--;
      if (keys == (KEY_P1  | KEY_INCR)) scoreA++;
      if (keys == (KEY_P2  | KEY_DECR)) scoreB--;
      if (keys == (KEY_P2  | KEY_INCR)) scoreB++;
      if (keys == (KEY_MAX_SCORE   | KEY_DECR)) --settings.max_score;
      if (keys == (KEY_MAX_SCORE   | KEY_INCR)) ++settings.max_score;
      if (keys & KEY_INCR || keys & KEY_DECR) {
        key_debounce = millis() + 200;
        settingschanged = true;
      }
    } else if (millis() >= key_debounce ) {
      key_debounce = 0;
    }
    if (keys & KEY_P1)   display_numtext(scoreA, " P1 ", false);
    if (keys & KEY_P2)  display_numtext(scoreB, " P2 ", false);
    if (keys & KEY_MAX_SCORE)   display_numtext(settings.max_score, " SC ", false);
    if (keys & KEY_RESET) {
        display_numtext(0 , "RST", false);
           display_text(" reset  ");
           delay(1000);
           display_text("        ");
           delay(500);
           display_text(" ready  ");
           delay(1000);
           display_text("        ");
           delay(500);
           display_text(" start  ");
            scoreA=0;
            scoreB=0;
           delay(500);
           sprintf(txt_buffer, "  %02d  %02d", scoreA, scoreB);
           display_text(txt_buffer);
    };
    // more if keys
  }
  if (restore_time && millis() >= restore_time) {
    restore_time = 0;
    if (settingschanged) {
      //Serial.println("Saving settings");
      save_settings();
      settingschanged = false;
    }
    //restore_display();
   sprintf(txt_buffer, "  %02d  %02d", scoreA, scoreB);
   display_text(txt_buffer);
  }
       
}


