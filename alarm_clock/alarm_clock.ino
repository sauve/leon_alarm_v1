#include <TM1637Display.h>
#include <DS3231.h>
#include <Wire.h>
#include <inttypes.h>

// Todo
// - Autre etat pour voir l'alarme
// - test musique sur speaker
// - input sur alarm trigger

// Module connection pins (Digital Pins)
#define CLK 2
#define DIO 3

// I2C = A5 et A4

#define BTN_CONF_PIN  A0
#define BTN_PLUS_PIN  A1
#define BTN_MOINS_PIN  A2
#define BTN_SNOOZE_PIN  A3
#define BTN_STOP_PIN  6

#define SPEAKER_PIN 9

// LED PIN
#define LED_PM_PIN 5
#define LED_ALARM_PIN 6
#define LED_ALARM2_PIN 10


// button bit constant
#define BTN_CONF    0b00000001
#define BTN_PLUS    0b00000010
#define BTN_MOINS   0b00000100
#define BTN_STOP    0b00001000
#define BTN_SNOOZE  0b00010000



// enumeration des etats
enum ETAT_PRINCIPAL {
  MODE_HEURE = 1,
  MODE_SECONDE,
  MODE_DATE,
  MODE_ALARM1,
  MODE_ALARM2,
  MODE_CHRONO,
};

enum ETAT_HEURE {
  AFFICHE_HEURE = 0,
  CONFIG_HEURE,
  CONFIG_MINUTE,
  CONFIG_CONFIRM_HEURE
};

enum ETAT_CRONO {
  CHRONO_DEPART = 0,
  CHRONO_CHRONO,
  CHRONO_PAUSE,
  CHRONO_STOP,
  CHRONO_RESET
};

TM1637Display display(CLK, DIO);
DS3231 myRTC;
bool century = false;
bool h12Flag;
bool pmFlag;
byte alarmDay, alarmHour, alarmMinute, alarmSecond, alarmBits;
bool alarmDy, alarmH12Flag, alarmPmFlag;

byte sethour, setminute, setseconde;
byte setalarmhour, setalarmminute;
byte setday, setmonth, setyear;


// Variables pour 
int etat = MODE_HEURE;
int sousetat = AFFICHE_HEURE;
int actionaffichage = 0;
int actionsonore = 0;

// delais pour la gestion des etats
int gestionEtatDelais = 0;
int gestionEtatAffichage = 0;
int gestionEtatSon = 0;

// timer pour la gestion des etats


// Variables pouir la gestion des boutons
uint8_t btnlastpressed;
uint8_t btnpressed;
uint8_t btnchanged;


// timer interne arduino


// variable pour parametre usager
int ledbrightness;
int alarmsound;


void setup() {
  // button pin setup
  pinMode(BTN_CONF_PIN, INPUT_PULLUP);
  pinMode(BTN_PLUS_PIN, INPUT_PULLUP);
  pinMode(BTN_MOINS_PIN, INPUT_PULLUP);
  pinMode(BTN_SNOOZE_PIN, INPUT_PULLUP);
  pinMode(BTN_STOP_PIN, INPUT_PULLUP);
  
  //led pin setup
  pinMode(LED_PM_PIN, OUTPUT);
  pinMode(LED_ALARM_PIN, OUTPUT);
  pinMode(LED_ALARM2_PIN, OUTPUT);
  analogWrite(LED_PM_PIN, 0);
  analogWrite(LED_ALARM_PIN, 0);
  analogWrite(LED_ALARM2_PIN, 0);

  pinMode(SPEAKER_PIN, OUTPUT);
  analogWrite(SPEAKER_PIN, 0);


  // Start the I2C interface
	Wire.begin();
 
	// Start the serial interface
	Serial.begin(57600);

  display.setBrightness(0x0f);
  int8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  display.setSegments(data);
}

void updateButtons()
{
  // Update 
  btnlastpressed = btnpressed;
  // btnpressed
  btnpressed = 0;
  if ( digitalRead(BTN_CONF_PIN) == 0 )
  {
    btnpressed += BTN_CONF;
  }
  if ( digitalRead(BTN_PLUS_PIN) == 0 )
  {
    btnpressed += BTN_PLUS;
  }
  if ( digitalRead(BTN_MOINS_PIN) == 0 )
  {
    btnpressed += BTN_MOINS;
  }
  // update les inputs
  btnchanged = btnpressed ^ btnlastpressed;
}

bool justPressed( uint8_t button)
{
  return ((btnpressed & btnchanged) & button ) != 0;
}

bool stillPressed( uint8_t button)
{
  return ((btnpressed & ~btnchanged) & button ) != 0;
}

void AfficherHeure()
{
  // aller lire l'heure sur le RTC
  byte heure = myRTC.getHour(h12Flag, pmFlag);
  byte minute = myRTC.getMinute();
  byte seconde = myRTC.getSecond();

  Serial.print(heure, DEC);
  Serial.print(":");
  Serial.print(minute, DEC);
  Serial.print(":");
  Serial.print(seconde, DEC);
  Serial.print("\n");

  // affiche sur le 7 segment l'heure et le minutes
  //int heurelcd = ((int)heure * 100) + (int)minute;
  //display.showNumberDec(heurelcd, true);
  if ( (seconde % 2) == 0 )
  {
      display.showNumberDecEx(heure, 0b00000000, true, 2, 0);
      //analogWrite(9, 0);
  }
  else
  {
      display.showNumberDecEx(heure, 0b01000000, true, 2, 0);
      //analogWrite(9, 250);
  }
  display.showNumberDec(minute, true, 2, 2);
}


void AfficherSeconde()
{
  byte seconde = myRTC.getSecond();
  display.showNumberDec(seconde, true);
}

void AfficherDate()
{
  byte mois = myRTC.getMonth(century);
  byte jour = myRTC.getDate();
  display.showNumberDec(mois, false, 2, 0);
  display.showNumberDec(jour, false, 2, 2);
}

void AfficherAnnee()
{
  int annee = myRTC.getYear();
  display.showNumberDec(annee, false);
}

void AfficherAlarm1()
{
  myRTC.getA1Time(alarmDay, alarmHour, alarmMinute, alarmSecond, alarmBits, alarmDy, alarmH12Flag, alarmPmFlag);
  display.showNumberDec(alarmHour, false, 2, 0);
  display.showNumberDec(alarmMinute, false, 2, 2);
}

void AfficherAlarm2()
{
  myRTC.getA2Time(alarmDay, alarmHour, alarmMinute, alarmBits, alarmDy, alarmH12Flag, alarmPmFlag);
  display.showNumberDec(alarmHour, false, 2, 0);
  display.showNumberDec(alarmMinute, false, 2, 2);
}


// Code gestion des etats
void HandleHorloge()
{
  if ( justPressed(BTN_PLUS))
  {
    actionaffichage = actionaffichage + 1;
    if (actionaffichage > 5)
    {
      actionaffichage = 0;
    }
  }
  else if ( justPressed(BTN_MOINS))
  {
    actionaffichage = actionaffichage - 1;
    if (actionaffichage < 0)
    {
      actionaffichage = 5;
    }
  }
  else if ( justPressed(BTN_CONF))
  {
    actionaffichage = 0;
    etat = 1;
    actionsonore = 1;
  }
}

void HandleAlarme()
{
  if ( justPressed(BTN_PLUS))
  {
    etat = 0;
    actionaffichage = 0;
    actionsonore = 0;
  }
}

void GestionModeHeure()
  {
  switch(sousetat)
  {
    case AFFICHE_HEURE:
      if ( justPressed(BTN_CONF))
      {
        sousetat = CONFIG_HEURE;
        // set les variable set pour l'heure et minute
      }
      else if (justPressed(BTN_PLUS))
      {
        etat = AFFICHE_HEURE;
      }
      else if (justPressed(BTN_MOINS))
      {
        etat = AFFICHE_HEURE;
      }
      AfficherHeure();
      break;
    case CONFIG_HEURE:
      // affiche heure set en flash
      // plus et moins selon le AMPM ou 24h
      if ( justPressed(BTN_CONF))
      {
        sousetat = CONFIG_MINUTE;
        // set les variable set pour l'heure et minute
      }
      else if (justPressed(BTN_PLUS))
      {
        etat = AFFICHE_HEURE;
      }
      else if (justPressed(BTN_MOINS))
      {
        etat = AFFICHE_HEURE;
      }
      else if (justPressed(BTN_STOP))
      {
        etat = AFFICHE_HEURE;
      }
      AfficherSeconde();
      break;
    case CONFIG_MINUTE:
      // affiche minute set en flash
      // plus et moins roll 60
      if ( justPressed(BTN_CONF))
      {
        sousetat = CONFIG_CONFIRM_HEURE;
        // set les variable set pour l'heure et minute
      }
      else if (justPressed(BTN_PLUS))
      {
        etat = AFFICHE_HEURE;
      }
      else if (justPressed(BTN_MOINS))
      {
        etat = AFFICHE_HEURE;
      }
      else if (justPressed(BTN_STOP))
      {
        etat = AFFICHE_HEURE;
      }
      AfficherDate();
      break;
    case CONFIG_CONFIRM_HEURE:
      if ( justPressed(BTN_CONF))
      {
        sousetat = AFFICHE_HEURE;
        // set les variable set pour l'heure et minute
      }
      else if (justPressed(BTN_STOP))
      {
        etat = AFFICHE_HEURE;
      }
      AfficherAnnee();
      break;
  }
}

// Gestion des sons

void StopSound()
{
  analogWrite(SPEAKER_PIN, 0);
}

void PlayAlarmSound()
{
  analogWrite(SPEAKER_PIN, 250);
}


//// Affichage sur ecran oled


void oled_Affiche_heure()
{}

void oled_affiche_date()
{}

void oled_affiche_horloge()
{}

void oled_affiche_etat()
{
  // si alarm a on
  // si sur snooze
  // si ecoute radio
  //
}


// methode appeler constamment
void loop() {
  
  updateButtons();

  switch(etat)
  {
    case MODE_HEURE:
      GestionModeHeure();
      break;
  }
/*
  switch(etat)
  {
    case ETAT_PRINCIPAL.AFFICHE_HEURE:
      HandleHorloge();
      break;
    case ETAT_PRINCIPAL.:
      HandleAlarme();
      break;
  }

  switch(actionaffichage)
  {
    case 0:
      AfficherHeure();
      break;
    case 1:
      AfficherSeconde();
      break;
    case 2:
      AfficherDate();
      break;
    case 3:
      AfficherAnnee();
      break;
    case 4:
      AfficherAlarm1();
      break;
    case 5:
      AfficherAlarm2();
      break;
  }

  switch(actionsonore)
  {
    case 0:
      StopSound();
      break;
    case 1:
      PlayAlarmSound();
      break;
  }
  */
  delay(75);
}
