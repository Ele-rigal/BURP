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
#include "parser.c"
#include "interprete.c"
#include "generateur.c"
#include "Arene.c"

#define G_CSET_DIGITS "0123456789"
#define SIZE 1024

/*

Pour compiler à la main le projet

gcc projet/main.c -o main -g `pkg-config glib-2.0 --cflags --libs` -lm

Pour enregistrer un historique de la partie
./main  2> txt
*/

//Fonctions auxiliaire pour
Robot * dead(int id){
  Robot *dead= malloc(sizeof(Robot));
  *dead = createRobot(id,0,0);
  dead->pv = MAXPV;
  return dead;
}

Robot * alive(GArray * arrayRobot,int id){
  Robot * ret;
  ret = getRobotById(arrayRobot,id);
  if(ret == NULL) return dead(id);
  return ret;
}

int main(int argc, char const *argv[]) {
  srand(time(NULL));
  bool continuer = true;
  int i = 0;

  if (argc != 5)
  {
   fprintf(stderr,"Nombre d’argument incorrect. La fonction a besoin d’un argument : le chemin du script à parser.\n");
    exit(0);
  }

  //Initialisation des scripts
  token scripts[NUMPLAYERS];
  for(int i = 0;i < NUMPLAYERS;i++){
    scripts[i] = *start(argv[i+1]);
  }
  infoChat();

  for(int i=0;i< NUMPLAYERS;i++){
    //fprintf(stderr,"\n\nScript %d \n Nombre de lignes %d\n",i, scripts[i].nbArg);
    printTokenRec(&scripts[i], 0);
  }

  //Initialisation de l'environnement du script et du compteurs de ligne
  int line[NUMPLAYERS];
  for(int i = 0;i<NUMPLAYERS;i++) {line[i]=0;}
  GHashTable * env[NUMPLAYERS];
  for(int i = 0;i<NUMPLAYERS;i++)
  {
    env[i] = g_hash_table_new (g_int_hash, g_int_equal);
  }

  //Initialisation des instances Robot et missiles
  RobotAction actions[NUMPLAYERS];
  GArray* arrayRobot = NULL;
  arrayRobot = g_array_new(FALSE,FALSE,sizeof(Robot));
  GArray* arrayMissile = NULL;
  arrayMissile = g_array_new(FALSE,FALSE,sizeof(Missile));

  initRobots(arrayRobot);
  initActions(actions);

  // Initialisation Arene
  init_arene(
  g_array_index(arrayRobot,Robot,0),
  g_array_index(arrayRobot,Robot,1),
  g_array_index(arrayRobot,Robot,2),
  g_array_index(arrayRobot,Robot,3));

  //Lancement du programme
  while(continuer){
   fprintf(stderr,"[cycle %d]\n",i);
    /*
    - En premier effectue l'action donner lors du tour précedant par le script
    avec `startActions`.
    - Recupere la prochaine action avec `interpreteScript` et la traduit.
    - Met à jour les coordonnées et calcule les desgats avec `majPositions`.
    - Affiche la position des objets.
    - Affiche les points de vie!;
    - Arrete si un robot ou moins encore en vie sinon Recommence.
    */

    //Activation des actions
    startActions(arrayRobot,arrayMissile);

    //lecture du script et interpretation en actions
   fprintf(stderr,"= Next action =\n");
    for(int i =0;i<arrayRobot->len;i++){
      Robot * robot = &g_array_index(arrayRobot,Robot,i);
      int id = robot->id;
      line[id] = interpreteScript(&scripts[id],i,arrayRobot,line[id],env[id]);
    }

    //affichage
   fprintf(stderr,"== Mouvement du robot ==\n");

    //Fonctions principale "majPositions"
    majPositions(arrayRobot,arrayMissile);

    // TO DO AFFICHER DANS L'arêne
    afficheRobots(arrayRobot);
    afficheMissiles(arrayMissile);
   fprintf(stderr,"=\n");
    affichePv(arrayRobot);
   fprintf(stderr,"|=====|\n");




    for(int i = 0;i<NUMPLAYERS;i++){

      Robot rArene [NUMPLAYERS];
      for(int i=0;i<NUMPLAYERS;i++){
        rArene[i] = *alive(arrayRobot,i);
      }
      for(int i=0;i<NUMPLAYERS;i++){
        maj_robot(rArene[i],getPosRobot(rArene[i]),rArene[0],rArene[1],rArene[2],rArene[3]);
      }

      for(int i =0;i<arrayMissile->len;i++){
        maj_missile(rArene[0],rArene[1],rArene[2],rArene[3],g_array_index(arrayMissile,Missile,i));
      }
    }




    //nouveau cycle
    i++;
    if(arrayRobot->len <= 1 ){
      continuer = false;
    }
    if(i >= 2000 ){ //limiter le nombre de tour max
      continuer = false;
    }
    end_screen();
  }

  fprintf(stderr,"VAINQUEUR %d\n",g_array_index(arrayRobot,Robot,0).id);

  return 0;
}
