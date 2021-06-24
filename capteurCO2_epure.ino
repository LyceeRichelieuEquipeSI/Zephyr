#include "Adafruit_CCS811.h"

#include <NDIR_I2C.h>
#include <SoftwareSerial.h>
#include <NDIR_SoftwareSerial.h>
#include <Wire.h>

#include <Servo.h>

//MOX
//initialiser CCS
bool initialisationCCS (Adafruit_CCS811 &ccs)
{
  if (!ccs.begin())
  {
    return false;
  }
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);
  return true;
}

//recevoir valeur TVOC
float valeurTVOC (Adafruit_CCS811 &ccs)
{
  if(ccs.available())
  {
    if(!ccs.readData())
    {
      return ccs.geteCO2();
    }
    else
    {
      return -1.0;
    }
  }
  else
  {
    return -2.0;
  }
}

//NDIR
//initialiser
bool initialisationNDIR (NDIR_SoftwareSerial &ndir)
{
  if (!ndir.begin()) 
  {
      return false;
  }
  return true;
}

//recevoir valeur TVOC
float valeurTVOC (NDIR_SoftwareSerial &ndir)
{
  if (ndir.measure()) 
  {
    return ndir.ppm;
  }
  else
  {
    return -1.0;
  }
}

//inititaliser RGB
void initialisationRGB (int const& brocheR, int const& brocheG, int const& brocheB)
{
  pinMode(brocheR, OUTPUT);
  pinMode(brocheG, OUTPUT);
  pinMode(brocheB, OUTPUT);
  analogWrite(brocheR, 0);
  analogWrite(brocheG, 0);
  analogWrite(brocheB, 0);
}

//eclairer RGB
void eclairageRGB(int const& brocheR, int const& brocheG, int const& brocheB, int const& r, int const& g, int const& b)
{
  analogWrite(brocheR, r);
  analogWrite(brocheG, g);
  analogWrite(brocheB, b);
}

//attendre temps
void attendreTemps(int const& mlls)
{
  unsigned long tempsDebut = millis();
  while(millis()-tempsDebut < mlls);
}


//Servomoteur et CCS
Servo monServomoteur;
Adafruit_CCS811 ccs;
NDIR_SoftwareSerial ndir(2,3);

//VARIABLES GLOBALES
//Utlisateur
const float nombreDePositions=3.0;
const float anglesReelsDemiTour=220;
const float vitesseServoTemps=20;
const int angleDOrigine=0;
const int brocheR=11;
const int brocheG=10;
const int brocheB=9;
const int brocheServomoteur=4;
//Programme
float TVOC;
int angleServo=angleDOrigine;
int pasIncrementation;

void setup() {
  Serial.begin(9600);
  Serial.println("NDIR test ");

  while(!initialisationNDIR(ndir));
  initialisationRGB(brocheR,brocheG,brocheB);

  monServomoteur.attach(brocheServomoteur);
  while(!monServomoteur.attached());
  monServomoteur.write(angleDOrigine);
  attendreTemps(1000);
  monServomoteur.detach();
  eclairageRGB(brocheR,brocheG,brocheB,0,255,0);
  Serial.println("Initialisation réussie...");
}

void loop() {
  attendreTemps(1000);
  TVOC = valeurTVOC(ndir);
  if (0<TVOC){
    Serial.print("tvoc : ");
    Serial.print(TVOC);
    Serial.print("     degres var : ");
    Serial.print(angleServo);
    Serial.print("     degres reels: ");
    Serial.println(monServomoteur.read());
    for (int i(0);i<nombreDePositions;i++)
    {
      if (((i*(1800/nombreDePositions)<= TVOC)&(TVOC <= (i+1)*(1800/nombreDePositions)))|(i==nombreDePositions-1))
      {
        if ((i*(180*180/anglesReelsDemiTour)/(nombreDePositions-1)-angleServo)>0)
        {
          pasIncrementation=1;
        }
        else if((i*(180*180/anglesReelsDemiTour)/(nombreDePositions-1)-angleServo)==0)
        {
          break;
        }
          else
        {
          pasIncrementation=-1;
        }
        monServomoteur.attach(brocheServomoteur);
        while(!monServomoteur.attached());
        for (int o(0);o<abs(i*(180*180/anglesReelsDemiTour)/(nombreDePositions-1)-angleServo)+1;o++)
        {
          monServomoteur.write(angleServo+pasIncrementation*o);
          attendreTemps(vitesseServoTemps);
        }
        monServomoteur.detach();
        angleServo=i*(180*180/anglesReelsDemiTour)/(nombreDePositions-1);
        //LED
        if (angleServo<60){
          eclairageRGB(brocheR,brocheG,brocheB,0,255,0);
        }else if (120<angleServo) {
          eclairageRGB(brocheR,brocheG,brocheB,255,0,0);
        }else{
          eclairageRGB(brocheR,brocheG,brocheB,255,255,0);
        }
        break;
      }
    }
  }
}
