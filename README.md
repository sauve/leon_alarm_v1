# leon_alarm_v1
Code arduino pour reveil matin version 1

## modules electroniques

Liste des composants connectes

+ 4 digit 7 segments LED TM1637
+ RTC DS3231
+ 5 buttons
+ 1 speaker
+ Recepteur FM 
+ Ecran OLED I2C 
+ Arduino Micro 
+ LED ( Alarm, PM )

## Libraries utilisées

+ PUC2CHOSE pour RDA5807
+ ssd1306
+ TM1637
+ DS3231

## Fonctionnalites

+ Afffiche et configurer l'heure
    + Affiche en digital ou avec des aiguille style horloge
+ Afficher et configurer la date
    + ecrire le nom du mois et jour
+ Afficher et configuer une alarme
+ Ecouter la radio FM
  + Seek pour trouver le prochain poste
  + Mettre en mémoire le poste
+ Chronometre
+ 

## Elements configurable

+ Luminosite de l'affichage 7 segment
+ Delais lors d'un snooze
+ Affichage 24h ou AM/PM
+ Station FM prefere
+ son pour l'alarme
