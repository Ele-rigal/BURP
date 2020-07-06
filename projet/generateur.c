#include <glib.h>
#include <glib/gprintf.h>
#include <gmodule.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include "Graphics.h"
#include <locale.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
// #include "parser.h"
// #include "interprete.h"

#define G_CSET_DIGITS "0123456789"
#define SIZE 1024


//////////////////      ACTION      //////////////////
/* Traduit les nouvelles actions transmit par le script */
//Action envoyé par le script

//Initialise et met l'action à zero
void setActionZero( RobotAction * action){
  action->action = A_NONE;
  action->angleEngine = 0;
  action->speedEngine = 0;
  action->angleShoot = 0;
  action->distanceShoot = 0;
  // action->cycle = 0; //provoque des bug avec wait
}

//Met l'action pour le case CONTINU
void setActionContinu(RobotAction * action){
  action->action = A_CONTINU;
  action->angleEngine = 0;
  action->speedEngine = 0;
  action->angleShoot = 0;
  action->distanceShoot = 0;
  //fprintf(stderr,"cycle ? %d\n",action->cycle);
}

//Met l'action pour le case SHOOT
void setActionShoot(RobotAction * action,int angle, int distance){
  action->action = A_SHOOT;
  action->angleEngine = 0;
  action->speedEngine = 0;
  action->angleShoot = angle;
  action->distanceShoot = distance;
}

//Met l'action pour l'ENGINE
void setActionEngine(RobotAction * action,int angle, int speed){
  action->action = A_ENGINE;
  action->angleEngine = angle;
  action->speedEngine = speed;
  action->angleShoot = 0;
  action->distanceShoot = 0;
}

//incremente le cycle pour le décompte du wait
int incCycle(RobotAction * action){
  int inc = action->cycle+1;
  action->cycle = inc;
  return action->cycle;
}

//Initialisation des actions
void initActions(  RobotAction actions[] ){
  for(int i=0;i<NUMPLAYERS;i++){
    RobotAction action ;
    setActionZero(&action);
    actions[i]= action;
  }
}

//Donne le numero de ligne
int getNumber(token * tok,  int index, GArray * arrayRobot, GHashTable * env)
{
  //fprintf(stderr,"number type : %d, val : %d\n",tok->type, tok->val);
  if(tok->val == VOID && tok->type == NUMBER){
    return tok->data;
  }else{
    fprintf(stderr, " Number : mauvais type ou mauvaise val\n ");
    exit(0);
  }
}

//effectue l'operation des valeurs transmise
int getOperator(token * tok,int val1,int val2,  int index, GArray * arrayRobot, GHashTable * env)
{
  //fprintf(stderr,"Operator type : %d, val : %d\n",tok->type, tok->val);
  if(tok->type == OPERATOR){
    switch (tok->val) {
      case PLUS:
      return val1 + val2;
      break;
      case MINUS:
      return val1 - val2;
      break;
      case TIMES:
      return val1 * val2;
      break;
      case DIV:
      return val1 / val2;
      break;
      case MOD:
      return val1 % val2;
      break;
      default:
      fprintf(stderr, " Operator : pas la bonne valeur de case\n ");
      exit(0);
    }
  }else {
    fprintf(stderr, " Operator : pas le bon type\n ");
    exit(0);
  }
}

//retourne la valeur demander par l'expression
int getExpression(token* tok,  int index, GArray * arrayRobot, GHashTable * env)
{
  //fprintf(stderr,"expression type : %d, val : %d\n",tok->type, tok->val);
  if(tok->type == EXPRESSION){
    Robot *robot = &g_array_index(arrayRobot,Robot,index);
    Robot * target;
    int * hashval = NULL;
    // int * hashadress = NULL;
    int ret = 0;
    int val1, val2, val3, val4, max, num;
    switch (tok->val) {
      case INT:
      return tok->data;
      break;
      case OP:
      val1 = getExpression(tok->arg[0],index,arrayRobot, env);
      val2 = getExpression(tok->arg[2],index,arrayRobot, env);
      ret = getOperator(tok->arg[1],val1,val2,index,arrayRobot, env);
      return ret;
      break;

      case PEEK :
      // * hashadress = getExpression(tok->arg[0],index,arrayRobot, env);
      // hashval = g_hash_table_lookup(env, hashadress);
      val1 = getExpression(tok->arg[0],index,arrayRobot, env);
      hashval = g_hash_table_lookup(env, &val1);
      if(hashval == NULL){
        fprintf(stderr, " clé %d n'existe pas \n ",val1);
        exit(0);
      }else{
       // fprintf(stderr,"valeur de %d is %d\n",val1,*hashval);
        return *hashval;
      }
      break;

      case RAND :
      max = getExpression(tok->arg[0],index,arrayRobot, env);
      ret = rand() % max ;
      return ret;
      break;

      case CARDINAL :
      return NUMPLAYERS;
      break;

      case SELF :
      return robot->id;
      break;

      case SPEED :
      return getSpeedMouv(*robot);
      break;

      case STATE :
      num = getExpression(tok->arg[0],index,arrayRobot, env);
      target = getRobotById(arrayRobot,num);
      if(target == NULL){
        return MAXPV;
      }else{
        return getPv(*target);
      }
      break;

      case GPSX :
      num = getExpression(tok->arg[0],index,arrayRobot, env);
      target = getRobotById(arrayRobot,num);
      if(target == NULL){
        return 0;
      }else{
        return getPosRobot(*target).posX;
      }
      break;

      case GPSY :
      num = getExpression(tok->arg[0],index,arrayRobot, env);
      target = getRobotById(arrayRobot,num);
      if(target == NULL){
        return 0;
      }else{
        return getPosRobot(*target).posY;
      }
      break;

      case ANGLE:
      //TO DO
      val1 = getExpression(tok->arg[0],index,arrayRobot, env);
      val2 = getExpression(tok->arg[1],index,arrayRobot, env);
      val3 = getExpression(tok->arg[2],index,arrayRobot, env);
      val4 = getExpression(tok->arg[3],index,arrayRobot, env);
      ret = round(radToDeg(acos((val1*val3+val2*val4)/(sqrt(val1*val1+val2*val2)*sqrt(val3*val3+val4*val4)))));
      return ret;
      break;

      case TARGETX:
      //TO DO
      val1 = getExpression(tok->arg[0],index,arrayRobot, env);
      val2 = getExpression(tok->arg[1],index,arrayRobot, env);
      val3 = getExpression(tok->arg[2],index,arrayRobot, env);
      ret = val1+val2+val3;
      return ret;
      break;

      case TARGETY:
      //TO DO
      val1 = getExpression(tok->arg[0],index,arrayRobot, env);
      val2 = getExpression(tok->arg[1],index,arrayRobot, env);
      val3 = getExpression(tok->arg[2],index,arrayRobot, env);
      ret = val1+val2+val3;
      return ret;
      break;

      case DISTANCE:
      //TO DO
      val1 = getExpression(tok->arg[0],index,arrayRobot, env);
      val2 = getExpression(tok->arg[1],index,arrayRobot, env);
      val3 = getExpression(tok->arg[2],index,arrayRobot, env);
      val4 = getExpression(tok->arg[3],index,arrayRobot, env);
      ret = val1+val2+val3+val4;
ret = sqrt(pow((val3-val1),2) + pow((val4-val2),2));

      return ret;
      break;

      default:
      fprintf(stderr, " Expression : pas la bonne valeur de case\n ");
      exit(0);
    }
  }else{
    fprintf(stderr, " Expression :mauvais type\n ");
    exit(0);
  }
}

//retourne le resultat du test entre les valeurs en parametre
bool getComparaison(token * tok,int val1 ,int val2,  int index, GArray * arrayRobot,GHashTable * env)
{
  //fprintf(stderr,"comparaison type : %d, val : %d\n",tok->type, tok->val);
  if(tok->type == COMPARAISON){
    switch (tok->val) {
      case INF: return val1 < val2 ;
      break;
      case SUPP: return val1 > val2 ;
      break;
      case DIFF: return val1 != val2 ;
      break;
      case INF_EQ: return val1 <= val2 ;
      break;
      case SUPP_EQ: return val1 >= val2 ;
      break;
      case EQ: return val1 == val2 ;
      break;
      default:
      fprintf(stderr, " Comparaison : pas la bonne valeur de case\n ");
      exit(0);
    }
  }else{
    fprintf(stderr, " Comparaison : mauvais type\n ");
    exit(0);
  }
}

//transmet a getComparaison les valeurs à tester et le test de comparaison
bool getCondition(token * tok,  int index, GArray * arrayRobot,GHashTable * env)
{
  //fprintf(stderr,"condition type : %d, val : %d\n",tok->type, tok->val);
  int val1, val2, val3, val4;
  if(tok->type == CONDITION && tok->val == VOID){
    val1 = getExpression(tok->arg[0],index,arrayRobot, env);
    val2 = getExpression(tok->arg[2],index,arrayRobot, env);
    return getComparaison(tok->arg[1],val1,val2,index,arrayRobot, env);
  }
  fprintf(stderr, " Condition : mauvais type ou mauvaise val");
  exit(0);
}

/*
Fonction qui interprete une commande en une action SHOOT,CONTINU,ENGINE
mais egalement les if, poke, goto
*/
void getCommand(token* tok, int index, GArray * arrayRobot, int * nextline,GHashTable * env)
{
  Robot *robot = &g_array_index(arrayRobot,Robot,index);
  int angle,speed,distance,line,max,test,val1,val2;
  int * hashval = NULL;
  int * hashadress = NULL;
  // int * val2;
  bool cond;
  //fprintf(stderr,"command type : %d, val : %d, %d\n",tok->type, tok->val,SHOOT);
  if(tok->type == COMMAND){
    setActionZero(&robot->action);
    switch (tok->val) {
      case POKE:
     fprintf(stderr,"POKE\n");
      hashval = malloc(sizeof(int));
      hashadress = malloc(sizeof(int));
      *hashadress = getExpression(tok->arg[0],index,arrayRobot, env);
      *hashval= getExpression(tok->arg[1],index,arrayRobot, env);
      cond = g_hash_table_insert(env, hashadress, hashval);
      // if(cond == FALSE){ //décommenter si interdiction de réecriture des variables
      //   fprintf(stderr, " POKE :  valeur deja utilisé\n ");
      //   exit(0);
      // }
      hashval = g_hash_table_lookup(env, hashadress);

      break;

      case GOTO:
     fprintf(stderr,"GOTO\n");
      * nextline = tok->data;
      break;

      case WAIT:
     fprintf(stderr,"WAIT\n");
      max = getExpression(tok->arg[0],index,arrayRobot, env);
      setActionContinu(&robot->action);
      test = incCycle(&robot->action) % max;
      if(test == 0){
        (robot->action).cycle = 0;
      }else{
        * nextline = (*nextline)-1;
      }
      break;

      case IF:
     fprintf(stderr,"IF\n");
      cond = getCondition(tok->arg[0],index,arrayRobot, env);
      if(cond){ * nextline = getNumber(tok->arg[1],index,arrayRobot, env);}
      break;

      case ENGINE:
     fprintf(stderr,"ENGINE\n");
      angle = getExpression(tok->arg[0],index,arrayRobot, env);
      speed = getExpression(tok->arg[1],index,arrayRobot, env);
      setActionEngine(&robot->action,angle,speed);
      break;

      case SHOOT:
     fprintf(stderr,"SHOOT\n");
      angle = getExpression(tok->arg[0],index,arrayRobot, env);
      distance = getExpression(tok->arg[1],index,arrayRobot, env);
      setActionShoot(&robot->action,angle,distance);
      break;

      default:
     fprintf(stderr,"commande à implementer\n");
      setActionContinu(&robot->action);
      break;
    }
  }else{
    fprintf(stderr, " Command : pas le bon type ");
    exit(0);
  }
}


//Execute tout le programme à l'origine de cette ligne
//retourne le numero de la prochaine ligne à executer
//celle ci peut être modifie par des go to et des if
int getLine(token *tok, int index, GArray * arrayRobot,int line,GHashTable * env)
{
  //fprintf(stderr,"LINE type : %d, val : %d\n",tok->type, tok->val);
  if(tok->val == VOID && tok -> type == LINE){
    int nextline = getNumber(tok->arg[0],index,arrayRobot,env) + 1;
    getCommand(tok->arg[1],index,arrayRobot,&nextline,env);
    return nextline-1;
  }else {
    fprintf(stderr, " Line : pas le bon type ou pas la bonne val");
    exit(0);
  }
}

//fonction de test
void iterator(gpointer key, gpointer value, gpointer user_data) {
 fprintf(stderr,user_data, *(gint*)key,*(gint*) value);
}

//Récupère l'action du Script
//Lance le programme à la ligne en parametre
//si on arrive à la fin du script on recommence
int interpreteScript(token * tok, int index, GArray * arrayRobot,int line,GHashTable * env)
{
  //fprintf(stderr,"Program type : %d, val : %d\n",tok->type, tok->val);
  // fprintf(stderr,"all key of %d\n",index);
  // g_hash_table_foreach(env, (GHFunc)iterator, "key %d val %d\n");

  line = line % tok->nbArg;
  if(tok->val == VOID && tok->type == PROGRAM){
    return getLine(tok->arg[line],index,arrayRobot,line,env);
  }else{
    fprintf(stderr, " Program : wrong type or wrong val");
    exit(0);
  }
}
