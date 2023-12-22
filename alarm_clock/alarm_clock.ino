#include <TM1637Display.h>
#include <DS3231.h>
#include <Wire.h>
#include <ssd1306.h>
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
  MODE_CHRONO
};

enum ETAT_HEURE {
  AFFICHE_HEURE = 0,
  CONFIG_HEURE,
  CONFIG_MINUTE,
  CONFIG_CONFIRM_HEURE
};

enum ETAT_DATE {
  DATE_AFFICHE_DATE = 0,
  DATE_CONFIG_DATE,
  DATE_CONFIG_MOIS,
  DATE_CONFIG_ANNEE,
  DATE_CONFIG_CONFIRM_DATE
};

enum ETAT_ALARM {
  ALARM_AFFICHE_ALARM = 0,
  ALARM_CONFIG_ALARM,
  ALARM_CONFIG_MINUTE,
  ALARM_CONFIG_CONFIRM_ALARM
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

// etat actuel des senseur, alarm, horloge et radio
byte curheure, curminute, curseconde;
byte curday, curmonth, curyear, curdow;
bool curampm, cur24h, curalarmon, curradio, cursnooeze;
byte curtemp;

uint16_t cur_fmfrequency;

// Variables de statut courant
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
int param_ledbrightness;
int param_alarmsound;
int param_fmvolume;
int param_snooze_delais;

// hum, combien de poste max en mémoire ?


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


  // Initialise l'interface I2C
	Wire.begin();
 
	// Initialise la communication série
	Serial.begin(57600);


  display.setBrightness(0x0f);
  int8_t data[] = { 0xff, 0xff, 0xff, 0xff };
  display.setSegments(data);

  // initialise l'écran oled
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_128x64_i2c_init();
  ssd1306_clearScreen();
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
  if ( digitalRead(BTN_SNOOZE_PIN) == 0 )
  {
    btnpressed += BTN_SNOOZE;
  }
  if ( digitalRead(BTN_STOP_PIN) == 0 )
  {
    btnpressed += BTN_STOP;
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


// methode pour la gestion du module RTC
void rtc_lire_heure()
{
  // aller lire l'heure sur le RTC
  curheure = myRTC.getHour(h12Flag, pmFlag);
  curminute = myRTC.getMinute();
  curseconde = myRTC.getSecond();
}

void rtc_lire_date()
{
  // aller lire l'heure sur le RTC
  curmonth = myRTC.getMonth(century);
  curday = myRTC.getDate();
  curyear = myRTC.getYear();
}

void rtc_lire_alarm1()
{
  myRTC.getA1Time(alarmDay, alarmHour, alarmMinute, alarmSecond, alarmBits, alarmDy, alarmH12Flag, alarmPmFlag);
}

void rtc_lire_temp()
{
  curtemp = myRTC.getTemperature();
}



void led_AfficherHeure()
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


void led_AfficherSeconde()
{
  byte seconde = myRTC.getSecond();
  display.showNumberDec(seconde, true);
}

void led_AfficherDate()
{
  byte mois = myRTC.getMonth(century);
  byte jour = myRTC.getDate();
  display.showNumberDec(mois, false, 2, 0);
  display.showNumberDec(jour, false, 2, 2);
}

void led_AfficherAnnee()
{
  int annee = myRTC.getYear();
  display.showNumberDec(annee, false);
}

void led_AfficherAlarm1()
{
  myRTC.getA1Time(alarmDay, alarmHour, alarmMinute, alarmSecond, alarmBits, alarmDy, alarmH12Flag, alarmPmFlag);
  display.showNumberDec(alarmHour, false, 2, 0);
  display.showNumberDec(alarmMinute, false, 2, 2);
}

void led_AfficherAlarm2()
{
  myRTC.getA2Time(alarmDay, alarmHour, alarmMinute, alarmBits, alarmDy, alarmH12Flag, alarmPmFlag);
  display.showNumberDec(alarmHour, false, 2, 0);
  display.showNumberDec(alarmMinute, false, 2, 2);
}


//// Affichage sur ecran oled
void oled_affiche_boot()
{}


void oled_Affiche_heure()
{
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_printFixed(0,  32, "HH:MM ss", STYLE_NORMAL);
}

void oled_affiche_date()
{
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_printFixed(0,  32, "dow, dd mmmYYYY", STYLE_NORMAL);
}

void oled_affiche_radio()
{
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_printFixed(0,  32, "dow, dd mmm YYYY", STYLE_NORMAL);
}

void oled_affiche_alarme()
{
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_printFixed(0,  32, "on, HH:MM", STYLE_NORMAL);
}

void oled_affiche_horloge()
{
  ssd1306_drawLine(64, 32, 64, 24);
  ssd1306_drawLine(64, 32, 92, 32);
  ssd1306_drawLine(64, 32, 32, 44);
}


void oled_affiche_statut()
{
  ssd1306_drawLine(0,64 - 9, ssd1306_displayWidth() -1, 64 - 9);
  ssd1306_printFixed(0,  64 - 8, "Debug curent status", STYLE_NORMAL);
}

// Affiche l'etat courant
// Resposnsable d'efeacer l'ecran et d'afficher l'etat courant
// Doit etre appeler avant tout autre affichage a moin de vouloir controler l'ensemble de l'ecran
void oled_affiche_etat()
{
  ssd1306_setFixedFont(ssd1306xled_font6x8);
  ssd1306_clearScreen();
  // si alarm a on
  ssd1306_printFixed(0,  8, "A", STYLE_NORMAL);
  // si sur snooze
  ssd1306_printFixed(8,  8, "S", STYLE_NORMAL);
  // si ecoute radio
  ssd1306_printFixed(16,  8, "FM", STYLE_NORMAL);
  // volume radio
  ssd1306_printFixed(32,  8, "V", STYLE_NORMAL);
  // am ou pm
  ssd1306_printFixed(48,  8, "AM", STYLE_NORMAL);
  // affiche temperature
  ssd1306_printFixed(112,  8, "V", STYLE_NORMAL);
  ssd1306_drawLine(0,16, ssd1306_displayWidth() -1, 16);
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
  rtc_lire_heure();
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
        etat = MODE_DATE;
      }
      else if (justPressed(BTN_MOINS))
      {
        etat = MODE_ALARM1;
      }
      led_AfficherHeure();
      oled_affiche_etat();
      oled_Affiche_heure();

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
      led_AfficherSeconde();
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
      led_AfficherDate();
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
      led_AfficherAnnee();
      break;
  }
}

void GestionModeDate()
{
  rtc_lire_date();
  switch(sousetat)
  {
    case DATE_AFFICHE_DATE:
      if ( justPressed(BTN_CONF))
      {
        //sousetat = CONFIG_DATE;
        // set les variable set pour l'heure et minute
      }
      else if (justPressed(BTN_PLUS))
      {
        etat = MODE_HEURE;
      }
      else if (justPressed(BTN_MOINS))
      {
        etat = MODE_ALARM1;
      }
      led_AfficherDate();
      oled_affiche_etat();
      oled_affiche_date();
      break;
    /*
    case CONFIG_DATE:
      // affiche heure set en flash
      // plus et moins selon le AMPM ou 24h
      if ( justPressed(BTN_CONF))
      {
        sousetat = CONFIG_CONFIRM_DATE;
        // set les variable set pour l'heure et minute
      }
      else if (justPressed(BTN_PLUS))
      {
        etat = AFFICHE_DATE;
      }
      else if (justPressed(BTN_MOINS))
      {
        etat = AFFICHE_DATE;
      }
      else if (justPressed(BTN_STOP))
      {
        etat = AFFICHE_DATE;
      }
      led_AfficherDate();
      break;
    case CONFIG_CONFIRM_DATE:
      if ( justPressed(BTN_CONF))
      {
        sousetat = AFFICHE_DATE;
        // set les variable set pour l'heure et minute
      }
      else if (justPressed(BTN_STOP))
      {
        etat = AFFICHE_DATE;
      }
      led_AfficherAnnee();
      break;
      */
  }
}

void GestionModeAlarme()
{
  switch(sousetat)
  {
    case ALARM_AFFICHE_ALARM:
      if ( justPressed(BTN_CONF))
      {
        //sousetat = CONFIG_ALARM;
        // set les variable set pour l'heure et minute
      }
      else if (justPressed(BTN_PLUS))
      {
        etat = MODE_DATE;
      }
      else if (justPressed(BTN_MOINS))
      {
        etat = MODE_HEURE;
      }
      led_AfficherAlarm1();
      break;
    /*
    case CONFIG_ALARM:
      // affiche heure set en flash
      // plus et moins selon le AMPM ou 24h
      if ( justPressed(BTN_CONF))
      {
        sousetat = CONFIG_CONFIRM_ALARM;
        // set les variable set pour l'heure et minute
      }
      else if (justPressed(BTN_PLUS))
      {
        etat = AFFICHE_ALARM;
      }
      else if (justPressed(BTN_MOINS))
      {
        etat = AFFICHE_ALARM;
      }
      else if (justPressed(BTN_STOP))
      {
        etat = AFFICHE_ALARM;
      }
      led_AfficherAlarm1();
      break;
    case CONFIG_CONFIRM_ALARM:
      if ( justPressed(BTN_CONF))
      {
        sousetat = AFFICHE_ALARM;
        // set les variable set pour l'heure et minute
      }
      else if (justPressed(BTN_STOP))
      {
        etat = AFFICHE_ALARM;
      }
      led_AfficherAnnee();
      break;
      */
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




// methode appeler constamment
void loop() {
  
  updateButtons();

  switch(etat)
  {
    case MODE_HEURE:
      GestionModeHeure();
      break;
    case MODE_DATE:
      GestionModeDate();
      break;
    case MODE_ALARM1:
      GestionModeAlarme();
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
