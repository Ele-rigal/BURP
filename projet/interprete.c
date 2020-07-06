#include "interprete.h"
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


/*
EXPLICATION
Les robots et les missiles sont géres par des GArray pour pouvoir les supprimer et ajouter rapidement
Cette structure permet d'avoir la taille du tableau et l'element à un indexe rapidement

Les missiles et les robots ont chacun une structure.
Les positions des objets sur l'arene ont une structure de données Positon pour que ce soit plus facile à manipuler

Il y a aussi une structure Mouvement qui permet d'enregister le mouvement de l'objet,
le mouvement n'est pas interrompu tant qu'il n'y  a pas eu un ordre ENGINE ou une collision

Le robot a une sous structure RobotAction
c'est par cette structure que les ordres du script sont transmis
Cela permet d'enregistrer les infos jusqu'au prochain ordre
Et aussi de transmettre les infos aux differentes fonction uniquement en transmettant les robots



to do:
- Créer une structure Environnement qui regroupe les Robot et les missiles pour avoir qu'un parametre par fonction
- Améliorer le nom des fonctions
- Ecrire en anglais les fonctions
*/


//COMPILATION
// gcc `pkg-config --libs --cflags glib-2.0` -Wall -o test interprete.c -lncursesw -lm
// gcc `pkg-config --cflags glib-2.0` interprete.c `pkg-config --libs glib-2.0` -o interprete -lm -lncursesw



//////////////////      POSITION      //////////////////
//Créer une paire de position en double (valeur réelle)
Position createPos(double posX,double posY){
  Position pos;
  pos.posX = posX;
  pos.posY = posY;
  return pos;
}

//Copie une position
Position copiePos(Position pos){
  Position ret;
  ret.posX = pos.posX;
  ret.posY = pos.posY;
  return ret;
}


//calcul la position d'arrivé
Position destPos(Position pos,int angleInt,int distance){
  double angle = (double) (angleInt % 360); //l'angle ne doit pas dépasser 360
  Position ret;
  if (angle >= 0.0){
    if(angle <= 90.0){
      ret.posX = pos.posX +  cos(degToRad(angle)) * distance;
      ret.posY = pos.posY +  sin(degToRad(angle)) * distance;
      return ret;
    }
    if(angle <= 180.0){
      ret.posX = pos.posX - sin(degToRad(angle)) * distance;
      ret.posY = pos.posY + cos(degToRad(angle)) * distance;
      return ret;
    }
    if(angle <= 270.0 ){
      ret.posX = pos.posX - cos(degToRad(angle)) * distance ;
      ret.posY = pos.posY - sin(degToRad(angle)) * distance ;
      return ret;
    }
    if(angle <= 360.0 ){
      ret.posX = pos.posX + sin(degToRad(angle)) * distance ;
      ret.posY = pos.posY - cos(degToRad(angle)) * distance;
      return ret;
    }
  }
  //Erreur
  //retourne la position actuelle
  return pos;

}

//Calcul distance entre deux positions
double distance(Position a, Position b){
  return sqrt(pow((b.posX-a.posX),2) + pow((b.posY-a.posY),2));
}

//test si les positions sont à moins de 10 unités d'ecart
// cela signifie collision entre deux robots
bool equalPos(Position pos1, Position pos2){
  return (distance(pos1,pos2) <= ROBOTSIZE);
}

//la taille du missile est ponctuelle
//test si les positions sont à moins de 10 unités d'ecart
//cela signifie collision entre un robot et un missile
bool robotToMissilePos(Position posRobot, Position posMissile){
  return (distance(posRobot,posMissile) <= ROBOTSIZE / 2);
}

//Test égalité excate des positions
bool samePos(Position pos1, Position pos2){
  return (round(pos1.posX) == round(pos2.posX) && round(pos1.posY) == round(pos2.posY));
}
///////////MOUVEMENT
/*
Lorque l'engine ou un missile est lancé il se crée un mouvment
qui continue jusqu'a ca qu'il soir arreté
*/

//créer un mouvement
Mouvement createMouv(int angle,int speed){
  Mouvement ret ;
  ret.angle = angle;
  ret.speed = speed;
  return ret;
}

//Mouvemement nul
Mouvement noMouvement(){
  return createMouv(0,0);
}


//////////////////      ROBOT      //////////////////
//Initialise et met l'action à zero
void auxAction( RobotAction * robot){
  robot->action = A_NONE;
  robot->angleEngine = 0;
  robot->speedEngine = 0;
  robot->angleShoot = 0;
  robot->distanceShoot = 0;
}

//créer un Robot
Robot createRobot(int id,double posX,double posY){
  Robot ret;
  ret.pv = INITPV;
  ret.id = id;
  ret.pos = createPos(posX,posY);
  RobotAction action;
  auxAction(&action);
  ret.action = action;
  ret.compteurMis = 0;
  ret.mouv = noMouvement();
  return ret;
}



//accesseurs
Robot * getRobotById(GArray * arrayRobot,int robot_id){
  for(int i = 0;i<arrayRobot->len;i++){
    Robot * ret = &g_array_index(arrayRobot,Robot,i);
    if(ret->id == robot_id){
      return ret;
    }
  }
  return NULL;
}

int getPv(Robot robot){
    return robot.pv;
}

int getAction(Robot robot){
  return robot.action.action;
}

int getAngleEngine(Robot robot){
  return robot.action.angleEngine;
}

int getSpeedEngine(Robot robot){
  return robot.action.speedEngine;
}

int getAngleShoot(Robot robot){
  return robot.action.angleShoot;
}

int getAngleMouv(Robot robot){
  return robot.mouv.angle;
}

int getSpeedMouv(Robot robot){
  return robot.mouv.speed;
}

Position getPosRobot(Robot robot){
  return robot.pos;
}

//setteurs

void stopEngine(Robot * robot){
  //fprintf(stderr,"robot à l'arret\n");
  robot->mouv.angle = 0;
  robot->mouv.speed = 0;
}

//supprime robot de la partie si vie inferieur à 0
void checkForRemove(GArray * robots){
  int i = 0;
  int end = robots->len;
  while(i < end){
    Robot r = g_array_index(robots,Robot,i);
    if(getPv(r) >= MAXPV){
      g_array_remove_index(robots,i);
      end --;
    }else{
      i++;
    }
  }
}

//////////////////      MISSILE      //////////////////
//créer un missile
Missile * createMissile(Robot robot, Missile * mis){
  mis->etat = false;
  mis->id = robot.compteurMis;
  mis->robot_id = robot.id;
  mis->pos = copiePos(robot.pos);
  int distance = robot.action.distanceShoot;
  if(distance > MAXDISTANCE){distance = MAXDISTANCE;}
  mis->posEnd = destPos(mis->pos,robot.action.angleShoot,distance);
  mis->mouv = createMouv(getAngleShoot(robot),MISSILESPEED);
  return mis;
}

//créer un faux missile interdit
Missile noMissile(){
  Missile mis;
  mis.etat = true;
  mis.id = -1;
  mis.robot_id = -1;
  mis.pos = createPos(0.0,0.0);
  mis.posEnd = createPos(0.0,0.0);
  mis.mouv = noMouvement();
  return mis;
}

//accesseurs
Position getPosMissile(Missile missile){
  return missile.pos;
}

/////////////                TODO                /////////////
////////////                                     /////////////
//                                                          //
//             AFFICHE LES ROBOTS DANS L'ARÊNE              //
//                                                          //
////////////                                     /////////////

//////////////////      AFFICHAGE      //////////////////
void affichePos(  Position pos,char objet[],int id){
 fprintf(stderr,"%s %d à la position %f,%f\n",objet,id,pos.posX,pos.posY);
}

void afficheRobot( Robot robot){
  affichePos(robot.pos,"robot",robot.id);
}
void afficheMissile(  Missile mis){
  // if(mis.id == 1) fprintf(stderr,"%der ",mis.id);
  // elsefprintf(stderr,"%deme ",mis.id);
  affichePos(mis.pos,"missile de",mis.robot_id);
}

void afficheRobots(  GArray * robots){
  for(int i =0;i<robots->len;i++){
    afficheRobot(g_array_index(robots,Robot,i));
  }
}
void afficheMissiles(  GArray * missiles){
  for(int i =0;i<missiles->len;i++){
    afficheMissile(g_array_index(missiles,Missile,i));
  }
}

void affichePv(GArray * robots){
  for(int i = 0;i<robots->len;i++){
    Robot robot = g_array_index(robots,Robot,i);
    int pv = robot.pv;
   fprintf(stderr,"pv de %d\n",robot.id);
    int l = 0;
    for(int j=0;j<pv;j = j +5){
     fprintf(stderr,"█");
    }
    for(l=pv;l<100;l=l+5){
      if(l == 0){fprintf(stderr,"|");}
      else{
     fprintf(stderr,"_");}
    }
   fprintf(stderr,"|\n");
  }
}

//////////////////      FONCTIONS PRINCIPALES      //////////////////
//MISE A JOUR DES ÉLÉMENTS

//Initialisation des robots
void initRobots( GArray* robots){
  int marge = 100.0;
  Robot robot0 = createRobot(0,marge,marge);
  Robot robot1 = createRobot(1,MUR - marge,marge);
  Robot robot2 = createRobot(2,MUR - marge,MUR - marge);
  Robot robot3 = createRobot(3,marge,MUR - marge);
  robots = g_array_append_val(robots,robot0);
  robots = g_array_append_val(robots,robot1);
  robots = g_array_append_val(robots,robot2);
  robots = g_array_append_val(robots,robot3);
}


//mise à jour des Mouvement
void majMouvementRobot( Robot * robot){
  robot->mouv.angle = getAngleEngine(*robot);
  robot->mouv.speed = getSpeedEngine(*robot);
  //fprintf(stderr,"angle %d et vitesse %d\n",robot->mouv.angle,robot->mouv.speed );
}

//Lance missile
void shoot(Robot* robot,GArray* arrayMissiles ){
  if(robot->compteurMis < 2){
    robot->compteurMis = robot->compteurMis + 1;
    Missile missile;
    createMissile(*robot,&missile);
    arrayMissiles = g_array_append_val(arrayMissiles,missile);
  }
}

//Active l'action
void startActions(  GArray* arrayRobot, GArray* arrayMissile){
  for(int i =0;i<arrayRobot->len;i++){
    Robot * robot = &g_array_index(arrayRobot,Robot,i);
    switch (getAction(*robot)) {
      case A_ENGINE:
     fprintf(stderr,"Engine %d\n",i);
      majMouvementRobot(robot);
      break;
      case A_SHOOT:
     fprintf(stderr,"Shoot %d\n",i);
      shoot(robot,arrayMissile);
      break;
      case A_CONTINU:
     fprintf(stderr,"Continu\n");
      break;
      default:
      break;
    }

  }
}

//donne des degats à un robot par une collision
void degatCollision(Robot * robot){
  robot->pv = robot->pv  + COLLISIONDAMAGE;
  stopEngine(robot);
}

//donne des degats à un robot par un missile
void degatMissile(Robot * robot,double distance){
  if(distance <= CLOSEDISTANCE){
    robot->pv = robot->pv + CLOSEDAMAGE;
    // return;
  }
  else if(distance <= MEDIUMDISTANCE){
    robot->pv = robot->pv + MEDIUMDAMAGE;
    // return;
  }
  else if(distance <= FARDISTANCE){
    robot->pv = robot->pv + FARDAMAGE;
    // return-;
  }
}

//explosion du missile
void explosion(GArray * arrayRobot, GArray* arrayMissiles,int index){
  /*
  diminue le compteur du robot mère
  provoque des degats
  supprime le missile
  */
  //compteur
  Missile m = g_array_index(arrayMissiles,Missile,index);
  Robot * r = getRobotById(arrayRobot,m.robot_id);
  if(r != NULL){
    if(r->compteurMis > 0)
      r->compteurMis = r->compteurMis - 1;
  }

  //Selon la position les degats sont importants
  for(int i=0;i<arrayRobot->len;i++){
    Robot * robot = &g_array_index(arrayRobot,Robot,i);
    double d = distance(getPosRobot(*robot),getPosMissile(m));
    degatMissile(robot,d);
  }

  //suppression
  //fprintf(stderr,"explosion du %d eme ",m.id);
 fprintf(stderr,"explosion du ");
  affichePos(m.pos,"missile du robot",m.robot_id);
  arrayMissiles = g_array_remove_index(arrayMissiles,index);
}

/*Calcul les differents degats
Test en premier les collisions entre robots
-> deduit directement les dégats
Test en suite les collisions entre robot et missile
-> missile passe en etat explosion (true)
Puis active l'explosion des missiles
(ceux qui sont en collision avec un mur on etait initilialisé lors des deplacments)
*/
void calculDegats(GArray * arrayRobot, GArray* arrayMissile){
  //Impact entre robots
  for(int i=0;i<arrayRobot->len;i++){
    for(int j=i+1;j<arrayRobot->len;j++){
      Robot * r1 = &g_array_index(arrayRobot,Robot,i);
      Robot * r2 = &g_array_index(arrayRobot,Robot,j);
      if(equalPos(getPosRobot(*r1),getPosRobot(*r2))){
       fprintf(stderr,"collision entre robot %d et robot %d\n",i,j);
        degatCollision(r1);
        degatCollision(r2);
      }
    }
  }

  //Missile touche robot sur trajectoire
  for(int i=0;i<arrayRobot->len;i++){
    for(int j=0;j<arrayMissile->len;j++){
      Robot * r = &g_array_index(arrayRobot,Robot,i);
      Missile * m = &g_array_index(arrayMissile,Missile,j);
      if(robotToMissilePos(getPosRobot(*r),getPosMissile(*m))){
       fprintf(stderr,"collision entre robot %d et missile %d\n",i,j);
        m->etat = true;
      }
    }
  }

  //explosion des missiles
  for(int i=0;i<arrayMissile->len;i++){
    Missile * m = &g_array_index(arrayMissile,Missile,i);
    // if(m->etat != 0){
    // if(m->etat == true){
    if(m->etat){
      explosion(arrayRobot,arrayMissile,i);
    }
  }
}

//Sortie d'Arene
void horsArene(Position * pos, Robot * robot){

  if(pos->posX >= MURX - ROBOTSIZE){
    pos->posX = MURX- ROBOTSIZE - 1;
    stopEngine(robot);
    degatCollision(robot);
  }
  if(pos->posY >= MURY - ROBOTSIZE){
    pos->posY = MURY- ROBOTSIZE - 1;
    stopEngine(robot);
    degatCollision(robot);
  }
  if(pos->posX <= 0 + ROBOTSIZE){
    pos->posX = 0 + ROBOTSIZE + 1;
    stopEngine(robot);
    degatCollision(robot);
  }
  if(pos->posY <= 0 + ROBOTSIZE){
    pos->posY = 0 + ROBOTSIZE + 1;
    stopEngine(robot);
    degatCollision(robot);
  }

}

//déplacement d'un robot
void deplacementRobot(Robot *robot){
  Position endPos = destPos(robot->pos,getAngleMouv(*robot),(getSpeedMouv(*robot)*MAXSPEED)/100);
  //si la position est en dehors de l'arene met à jour et fais des degats
  horsArene(&endPos,robot);
  robot->pos = copiePos(endPos);
  // Position endPos = destPos(robot->pos,getAngleMouv(*robot),(getSpeedMouv(*robot)*MAXSPEED)/100);
  // //si la position est en dehors de l'arene met à jour et fais des degats
  // horsArene(&endPos,robot);
  // robot->pos = copiePos(endPos);
}

//deplacement missiles
void deplacementMissile(Missile * mis){
  Position newPos = destPos(mis->pos,mis->mouv.angle,mis->mouv.speed);

  //verifie si la distance maximale a été dépassé
  if(distance(mis->pos,mis->posEnd) < distance(mis->pos,newPos)){
    mis->pos = copiePos(mis->posEnd);
    mis->etat = 1;
   fprintf(stderr,"DESTINATION ATTEINTE\n");
    return;
   fprintf(stderr,"NE DOIT PAS S'AFFICHER\n");
  }
  //si sort de l'arene explose
  if(newPos.posX > MURX){
    newPos.posX = MURX;
    mis->etat = 1;
  }
  if(newPos.posY > MURY){
    newPos.posY = MURY;
    mis->etat = 1;
  }
  if(newPos.posX < 0){
    newPos.posX = 0;
    mis->etat = 1;
  }
  if(newPos.posY < 0){
    newPos.posY = 0;
    mis->etat = 1;
  }

  mis->pos = copiePos(newPos);
}

//déplacement d'un missile
//vérifie que le prochain mouvement est possible et calcul la position de destination
//vérifie les murs
void majPositions(  GArray * arrayRobot,  GArray* arrayMissile){
  for(int i =0;i<arrayRobot->len;i++){
    Robot *robot = &g_array_index(arrayRobot,Robot,i);
    deplacementRobot(robot);
  }

  for(int i=0;i<arrayMissile->len;i++){
    Missile * m = &g_array_index(arrayMissile,Missile,i);
    deplacementMissile(m);
  }

  calculDegats(arrayRobot,arrayMissile);
  checkForRemove(arrayRobot);
}
