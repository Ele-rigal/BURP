#include <glib.h>
#include <glib/glist.h>
#include <gmodule.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "Graphics.h"
#include <locale.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

typedef enum
{
  A_WAIT,
  A_POKE,
  A_SHOOT,
  A_ENGINE,
  A_NONE,
  A_CONTINU
} actions;

#define DELAY 30000

//Constante robot
// #define NONE 0
// #define ENGINE 1
// #define SHOOT 2
// #define CONTINU 3

//Constante VALEURS
#define NUMPLAYERS 4
#define MAXSPEED 10
#define MAXDISTANCE 7000
#define MISSILESPEED 500
#define INITPV 0
#define MAXPV 100
#define COLLISIONDAMAGE 2
#define FARDAMAGE 3
#define FARDISTANCE 400
#define MEDIUMDAMAGE 5
#define MEDIUMDISTANCE 200
#define CLOSEDAMAGE 10
#define CLOSEDISTANCE 50
#define ROBOTSIZE 10

//Constante de l'Arene
#define MUR 10000
#define MURX 10000
#define MURY 10000

#define radToDeg(angleInRadians) ((angleInRadians) * 180.0 / M_PI)
#define degToRad(angleInDegrees) ((angleInDegrees) * M_PI / 180.0)

typedef struct{
  int angle;
  int speed;
} Mouvement;

typedef struct Position{
  double posX;
  double posY;
} Position;

typedef struct RobotAction  {
  int action;
  int angleEngine;
  int speedEngine; /*between 0 and 100, pourcentage du maxspeed*/
  int angleShoot;
  int distanceShoot;
  int cycle;
}RobotAction;

typedef struct Robot {
  float pv;
  int id;
  Position pos;
  RobotAction action;
  int compteurMis;
  Mouvement mouv;
}Robot;

typedef struct Missile {
  bool etat;
  int id;
  int robot_id;
  Position pos;
  Position posEnd;
  Mouvement mouv;
}Missile;
