#include "Arene.h"
#include "Graphics.h"
#include <ncurses.h>
#include <stdlib.h>
#include <locale.h>
#include <unistd.h>
#include <time.h>

/**** pour compiler : gcc -Wall  -o Arene Arene.c -lncursesw ***/
#define DELAY 30000

#define t_x 6
#define t_y 10
#define pos_x LINES*0.6+2

#define ROBOT1 1
#define ROBOT2 2
#define ROBOT3 3
#define ROBOT4 4
#define NOIR 5

#define max_x 20
#define max_y LINES*0.60-1

WINDOW *haut, *bas,*coeur1,*coeur2,*coeur3,*coeur4,*robot1,*robot2,*robot3,*robot4;

//initialise les fenêtre de l'état de vie du robot
void heart_init(WINDOW **coeur,int pos_y){
	*coeur= subwin(stdscr,t_x,t_y,pos_x,pos_y);
	mvwprintw(*coeur,0,0,HEART1);
	wrefresh(*coeur);

}

//met à jour l"tat de vie du robot
void maj_coeur(int color,WINDOW **coeur,int pos_y,int life){
	wclear(*coeur);
	if(life>0){
		*coeur= subwin(stdscr,t_x,t_y,pos_x,pos_y);
		mvwprintw(*coeur,0,0,HEART1);
		wrefresh(*coeur);
		mvwprintw(bas,t_x,pos_y,"%d",life);
		//wrefresh(bas);

		if(life>=90){
			wattron(*coeur,COLOR_PAIR(color));
			mvwprintw(*coeur,1,1,"%ls",L"\u2592\u2592\u2592");
			mvwprintw(*coeur,1,5,"%ls",L"\u2592\u2592\u2592");
			mvwprintw(*coeur,2,2,"%ls",L"\u2592\u2592\u2592\u2592\u2592");
			mvwprintw(*coeur,3,4,"%ls",L"\u2592");
			wattroff(*coeur,COLOR_PAIR(color));
		}
		else if(life==90){
			wattron(*coeur,COLOR_PAIR(color));
			mvwprintw(*coeur,1,2,"%ls",L"\u2592\u2592");
			mvwprintw(*coeur,1,5,"%ls",L"\u2592\u2592\u2592");
			mvwprintw(*coeur,2,2,"%ls",L"\u2592\u2592\u2592\u2592\u2592");
			mvwprintw(*coeur,3,4,"%ls",L"\u2592");
			wattroff(*coeur,COLOR_PAIR(color));
		}
		else if(life>=80){
			wattron(*coeur,COLOR_PAIR(color));
			mvwprintw(*coeur,1,3,"%ls",L"\u2592");
			mvwprintw(*coeur,1,5,"%ls",L"\u2592\u2592\u2592");
			mvwprintw(*coeur,2,2,"%ls",L"\u2592\u2592\u2592\u2592\u2592");
			mvwprintw(*coeur,3,4,"%ls",L"\u2592");
			wattroff(*coeur,COLOR_PAIR(color));
		}
		else if(life>=70){
			wattron(*coeur,COLOR_PAIR(color));
			mvwprintw(*coeur,1,3,"%ls",L"\u2592");
			mvwprintw(*coeur,1,5,"%ls",L"\u2592\u2592\u2592");
			mvwprintw(*coeur,2,2,"%ls",L"\u2592\u2592\u2592\u2592\u2592");
			mvwprintw(*coeur,3,4,"%ls",L"\u2592");
			wattroff(*coeur,COLOR_PAIR(color));
		}
		else if(life>=60){
			wattron(*coeur,COLOR_PAIR(color));
			mvwprintw(*coeur,1,6,"%ls",L"\u2592\u2592");
			mvwprintw(*coeur,2,2,"%ls",L"\u2592\u2592\u2592\u2592\u2592");
			mvwprintw(*coeur,3,4,"%ls",L"\u2592");
			wattroff(*coeur,COLOR_PAIR(color));
		}
		else if(life>=50){
			wattron(*coeur,COLOR_PAIR(color));
			mvwprintw(*coeur,2,4,"%ls",L"\u2592\u2592\u2592");
			mvwprintw(*coeur,3,4,"%ls",L"\u2592");
			wattroff(*coeur,COLOR_PAIR(color));
		}
		else if(life>=30){
			wattron(*coeur,COLOR_PAIR(color));
			mvwprintw(*coeur,2,5,"%ls",L"\u2592\u2592");
			mvwprintw(*coeur,3,4,"%ls",L"\u2592");
			wattroff(*coeur,COLOR_PAIR(color));
		}else if(life>=20){
			wattron(*coeur,COLOR_PAIR(color));
			mvwprintw(*coeur,2,6,"%ls",L"\u2592");
			mvwprintw(*coeur,3,4,"%ls",L"\u2592");
			wattroff(*coeur,COLOR_PAIR(color));
		}else{
			wattron(*coeur,COLOR_PAIR(color));
			mvwprintw(*coeur,3,4,"%ls",L"\u2592");
			wattroff(*coeur,COLOR_PAIR(color));
		}
		wrefresh(*coeur);
	}
	else{
		WINDOW *death= subwin(stdscr,t_x,t_y,pos_x+1,pos_y);
		//box(death, ACS_VLINE, ACS_HLINE);
		mvwprintw(death,1,2,"DEAD");
		wrefresh(death);
	}


}

//afficher le nom des robots
void name(int color,WINDOW* win,int pos,char* nom){
	wattron(win,COLOR_PAIR(color));
	mvwprintw(win,1,pos+2,"%s",nom);
	wattroff(win,COLOR_PAIR(color));
	wrefresh(win);
}

//initialise les robots
void robot_init(int color,WINDOW **robot,int po_x,int pos_y){
	*robot= subwin(haut,t_x-3,t_y,po_x,pos_y);
	wrefresh(*robot);
	wattron(*robot,COLOR_PAIR(color));
	mvwprintw(*robot,0,0,ROBOT);
	wattroff(*robot,COLOR_PAIR(color));
	wrefresh(*robot);

}
//met à jour l'écran, id==-1 signifie que l'état des robots n'a pas changé
int  maj_arene(Robot r1,Robot r2,Robot r3,Robot r4,int id){
	//	clear(); // Clear the screen of all
	haut = subwin(stdscr, LINES*0.60, COLS, 0,0);
	bas= subwin(stdscr,LINES*0.4, COLS,LINES*0.6,0 );
	heart_init(&coeur1,2);
	heart_init(&coeur2,(COLS/4)+2);
	heart_init(&coeur3,(COLS/2)+2);
	heart_init(&coeur4,COLS-(COLS/4)+2);
	maj_coeur(ROBOT1,&coeur1,2,100-r1.pv);
	maj_coeur(ROBOT2,&coeur2,(COLS/4)+2,100-r2.pv);
	maj_coeur(ROBOT3,&coeur3,(COLS/2)+2,100-r3.pv);
	maj_coeur(ROBOT4,&coeur4,COLS-(COLS/4)+2,100-r4.pv);

	switch(id){

		case -1:
			if(r1.pv<100) robot_init(ROBOT1,&robot1,r1.pos.posY,r1.pos.posX);
			if(r2.pv<100) robot_init(ROBOT2,&robot2,r2.pos.posY,r2.pos.posX);
			if(r3.pv<100) robot_init(ROBOT3,&robot3,r3.pos.posY,r3.pos.posX);
			if(r4.pv<100) robot_init(ROBOT4,&robot4,r4.pos.posY,r4.pos.posX);
			break;

		case 0:
			if(r2.pv<100) robot_init(ROBOT2,&robot2,r2.pos.posY,r2.pos.posX);
			if(r3.pv<100) robot_init(ROBOT3,&robot3,r3.pos.posY,r3.pos.posX);
			if(r4.pv<100) robot_init(ROBOT4,&robot4,r4.pos.posY,r4.pos.posX);
			break;

		case 1:
			if(r1.pv<100) robot_init(ROBOT1,&robot1,r1.pos.posY,r1.pos.posX);
			if(r3.pv<100) robot_init(ROBOT3,&robot3,r3.pos.posY,r3.pos.posX);
			if(r4.pv<100) robot_init(ROBOT4,&robot4,r4.pos.posY,r4.pos.posX);
			break;

		case 2:
			if(r1.pv<100) robot_init(ROBOT1,&robot1,r1.pos.posY,r1.pos.posX);
			if(r2.pv<100) robot_init(ROBOT2,&robot2,r2.pos.posY,r2.pos.posX);
			if(r4.pv<100) robot_init(ROBOT4,&robot4,r4.pos.posY,r4.pos.posX);
			break;

		case 3:
			if(r1.pv<100) robot_init(ROBOT1,&robot1,r1.pos.posY,r1.pos.posX);
			if(r2.pv<100) robot_init(ROBOT2,&robot2,r2.pos.posY,r2.pos.posX);
			if(r3.pv<100) robot_init(ROBOT3,&robot3,r3.pos.posY,r3.pos.posX);
			break;

		default: printf("bad id\n"); exit(1);
	}

	/*maj_robot(ROBOT2,&robot2,1,COLS-10);
	maj_robot(ROBOT3,&robot3,max_y-4,0);
	maj_robot(ROBOT4,&robot4,max_y-4,COLS-10);*/
	box(haut, ACS_VLINE, ACS_HLINE);
	box(bas, ACS_VLINE, ACS_HLINE);

	mvwprintw(haut,0,2, "Arene");
	mvwprintw(bas,0,2, "Etat Robot");

	name(ROBOT1,bas,2,"ROBOT1");
	name(ROBOT2,bas,(COLS/4)+2, "ROBOT2");
	name(ROBOT3,bas,(COLS/2)+2, "ROBOT3");
	name(ROBOT4,bas,COLS-(COLS/4)+2, "ROBOT4");

	// wrefresh(haut);
	wrefresh(bas);
	return 0;
}

//met à jour la position d'un missile
void maj_missile(Robot r1,Robot r2,Robot r3,Robot r4,Missile m){
	int x=m.pos.posX;
	int y=m.pos.posY;
	int delay=time(NULL);
	while(x!=m.posEnd.posX || y!=m.posEnd.posY) {
		delay=time(NULL);
		if(x<m.posEnd.posX)
		x++;
		if(x>m.posEnd.posX)
		x--;
		if(y<m.posEnd.posY)
		y++;
		if(y>m.posEnd.posY)
		y--;
		wclear(stdscr);
		// previously-printed characters
		mvwprintw(stdscr,y,x,"o"); // Print our "ball" at the current xy position
		maj_arene(r1,r2,r3,r4,-1);
		wrefresh(stdscr);
		// Shorter delay between movements // Advance the ball to the right
		usleep(DELAY*5);
	}

}
//met à jour la position des robots
void maj_robot(Robot r,Position end,Robot r1,Robot r2,Robot r3,Robot r4){
	int i,j;
	i=r.pos.posX;
	j=r.pos.posY;
	WINDOW **robot;
	int color;
	int delay=time(NULL);
	switch(r.id){
		case 0: robot=&robot1;
		color=ROBOT1;
		break;
		case 1: robot=&robot2;
		color=ROBOT2;
		break;
		case 2: robot=&robot3;
		color=ROBOT3;
		break;
		case 3: robot=&robot4;
		color=ROBOT4;
		break;
		default: printf("bad id\n"); exit(1);
	}
	while(i!=end.posX || j!=end.posY){
		//if(difftime(time(NULL),delay)>(float)(1/600)){
		delay=time(NULL);
		clear();
		wclear(*robot);
		if(i<end.posX)
		i++;
		if(i>end.posX)
		i--;
		if(j<end.posY)
		j++;
		if(j>end.posY)
		j--;
		*robot= subwin(haut,t_x,t_y,j,i);
		wattron(*robot,COLOR_PAIR(color));
		mvwprintw(*robot,0,0,ROBOT);
		wattroff(*robot,COLOR_PAIR(color));
		wrefresh(*robot);
		maj_arene(r1,r2,r3,r4,r.id);
		wrefresh(haut);

		usleep(DELAY*5);
	}


}



//initialise l'arene
void init_arene(Robot r1,Robot r2,Robot r3,Robot r4){
	setlocale(LC_ALL, ""); //permet de mettre de l'unicode dans le terminal
	initscr();
	noecho();
	curs_set(FALSE);

	r2.pos.posX=COLS-10;
	r2.pos.posY=1;

	r3.pos.posX=0;
	r3.pos.posY=LINES*0.6-t_x+1;

	r4.pos.posX=COLS-10;
	r4.pos.posY=LINES*0.6-t_x+1;


	if(has_colors() == FALSE)
	{	endwin();
		printf("Your terminal does not support color\n");
		exit(1);
	}
	start_color();
	init_pair(ROBOT1,COLOR_RED,COLOR_BLACK);
	init_pair(ROBOT2,COLOR_YELLOW,COLOR_BLACK);
	init_pair(ROBOT3,COLOR_GREEN,COLOR_BLACK);
	init_pair(ROBOT4,COLOR_BLUE,COLOR_BLACK);

	haut = subwin(stdscr, LINES*0.60, COLS, 0,0);
	bas= subwin(stdscr,LINES*0.4, COLS,LINES*0.6,0 );
	heart_init(&coeur1,2);
	heart_init(&coeur2,(COLS/4)+2);
	heart_init(&coeur3,(COLS/2)+2);
	heart_init(&coeur4,COLS-(COLS/4)+2);
	maj_coeur(ROBOT1,&coeur1,2,100-r1.pv);
	maj_coeur(ROBOT2,&coeur2,(COLS/4)+2,100-r2.pv);
	maj_coeur(ROBOT3,&coeur3,(COLS/2)+2,100-r3.pv);
	maj_coeur(ROBOT4,&coeur4,COLS-(COLS/4)+2,100-r4.pv);



	box(haut, ACS_VLINE, ACS_HLINE);
	box(bas, ACS_VLINE, ACS_HLINE);

	mvwprintw(haut,0,2, "Arene");
	mvwprintw(bas,0,2, "Etat Robot");

	robot_init(ROBOT1,&robot1,r1.pos.posY,r1.pos.posX);
	robot_init(ROBOT2,&robot2,r2.pos.posY,r2.pos.posX);
	robot_init(ROBOT3,&robot3,r3.pos.posY,r3.pos.posX);
	robot_init(ROBOT4,&robot4,r4.pos.posY,r4.pos.posX);
	name(ROBOT1,bas,2,"ROBOT1");
	name(ROBOT2,bas,(COLS/4)+2, "ROBOT2");
	name(ROBOT3,bas,(COLS/2)+2, "ROBOT3");
	name(ROBOT4,bas,COLS-(COLS/4)+2, "ROBOT4");


	wrefresh(haut);
	wrefresh(bas);
}

void end_screen(){
	getch();
	endwin();
}
/*int main(void){
Robot r1,r2,r3,r4;
r1.pv=100;
r1.id=1;
r1.pos.posX=0;
r1.pos.posY=1;

r2.pv=100;
r2.id=2;


r3.pv=100;
r3.id=3;


r4.pv=100;
r4.id=4;


int delay=time(NULL);
init_arene(r1,r2,r3,r4);


Position end;
end.posX=6;
end.posY=3;
maj_robot(r1,end,r1,r2,r3,r4);
r2.pv=80;
end.posX=10;
end.posY=3;
maj_robot(r2,end,r1,r2,r3,r4);
end.posX=5;
end.posY=3;
maj_robot(r2,end,r1,r2,r3,r4);
end.posX=6;
end.posY=10;
maj_robot(r3,end,r1,r2,r3,r4);
end.posX=0;
end.posY=5;
r3.pv=50;
maj_robot(r4,end,r1,r2,r3,r4);

Missile m;
m.pos.posX=2;
m.pos.posY=5;
m.posEnd.posX=10;
m.posEnd.posY=7;

maj_missile(r1,r2,r3,r4,m);
//test des fonctions en attendant l'interpréteur
//maj_robot(ROBOT1,&robot1,1,0,10,20,40);
//usleep(DELAY);
//usleep(DELAY);
//sleep(1);
//	maj_robot(ROBOT3,&robot3,2,3,3,6,40);
//sleep(1);
//maj_robot(ROBOT3,&robot3,7,9,5,6,40);
//sleep(1);
//maj_robot(ROBOT3,&robot3,13,15,0,0,40);
///sleep(1);
//maj_robot(ROBOT1,&robot1,11,20,6,5,40);
//maj_robot(ROBOT3,&robot3,1,0,10,20,40);
//maj_robot(ROBOT4,&robot4,1,0,10,20,40);


end_screen();
return 0;
}*/

/*** l'arene fait 10 000 * 10 000 unités donc :
-***/
