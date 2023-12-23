$ Roadmap de développement




## Version 0.8

Version support avec menu

+ Bontuon conf ouvre les menu
  + + et - permettre de naviguer
  + Permet un scroll de la liste
  + Inverse le texte de l'élément choisi



## Version 0.7

Version support OLED

+ Afficher l'heure comme une ancienne horloge
+ Affiche l'heure comme une horloge binaire
+ Sauvegarde en EEPROM l'affichage de l'heure
+ Icone pour la barre d'état
+ Limiter les zone de refresj si possible

## Version 0.6

Version gestion de la date

+ Ajouter mode configuation de la date
  + Configurer le jour
  + Configurer le mois
  + Configurer l'année
  + Configurer le jour de la semaine
  + Confirmation changement
+ Afficher la date sur le OLED
  + nom de la journée
  + nom du mois

## Version 0.5

Version avec alarme

+ Ajouter mode configure alarme
  + Configurer heure alarme
  + Configurer minute alarme
  + Configurer durée snooze alarme
  + Configurer son d'alarme
  + Accepter alarme
+ Sauvegarder et lire durée du snooze pour alarme en EEPROM
+ Sauvegarder et lire le son d'alarme en EEPROM
+ Ajouter gestion LED pour alarm à on
+ Ajouter support pour le snooze
+ Ajouter mode alarme
  + Snooze ajoute un timer pour revenir dans alarme
  + Stop reset l'alarme
+ Oled état affiche 
  + durée restant du snooze
  + Si alarm a on
+ Afficher l'alarme sur le OLED
  + Inverse à chaque seconde l'heure
+ Genère le son et chage à chaque seconde
  + selon la config en paramter

## Version 0.4

Version suvegarde paramètres

+ Ajouter mode luminosité
  + Ajouter set 7 segment luminosité
  + Mode confirm luminosité
+ Sauvegarde et lecture au boot de la luminosité du 7 segmnt en EEPROM
+ Afficher sur OLED une barre pour voir le min et max


## Version 0.3

Version de la gedtion des états

+ Ajouter le support des boutons conf, snooze et stop
+ Ajouter les état de base heure, date et alarme
+ Ajouter les sous états pour la configuration et la sauvegarde
+ Afficher l'état courant sur le OLED
+ Implémenter la configuration de l'heure
  + Configurer 24h ou 12h
  + Configurer l'heure
  + Confifurer les minutes
  + Accepter la config et save
+ Ajouter le support de la del pour affiche am ou pm


## Verison 0.2

Ajout test écran OLED

+ Interfacer l'écran OLED
+ Afficher la barre d'état dans le haut de l'écran
+ Afficher l'heure sur l'écran OLED

## Version 0.1

Version de test des modules

+ Accède au module RTC 
  + lire l'heure
  + lire la date
  + lire l'alarme 1
+ Afficher l'heure sur le module 7 segments
+ Interfacer les boutons + et moins pour changer d'état
+ Afficher la date la date sur le 7 segments
+ Afficher l'alarme 1 sur le 7 segment

