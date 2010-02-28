/******************************************************************************/
/*                TERMINAL routines v3.0                                      */
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "getch2.h"
#include "keycodes.h"

int term_xs=80,term_ys=24;
int gomb=0;

int waitkey(){
  do{}while((gomb=getch2(1))==-1);
  return(gomb);
}

/******************************************************************************/
/*      ****  ANSI VIDEO routines v1.0   (C) 1997-98. by GyikSoft        ***  */
/******************************************************************************/

void clrscr(void){                              /* CLS + HOME       */
  get_screen_size();
  term_xs=screen_width;
  term_ys=screen_height-1;
  printf("\x1B[2J\x1B[1;1f");
}

void gotoxy(int x,int y){printf("\x1B[%d;%df",y,x);}    /* GotoXY(x,y)      */
void clreoln(void){printf("\x1B[K");}                   /* Sor vegeig torol */

void set_color(int x){printf("\x1B[%dm",x);}    /* Beallitja az attributumot */
void textcolor(int x){set_color(x+30);}         /* Beallitja a betuszint     */
void backgcolor(int x){set_color(x+40);}        /* Beallitja a hatterszint   */

void crsr_left (int x){printf("\x1B[%dD",x);}   /* Balra megy x-szer */
void crsr_right(int x){printf("\x1B[%dC",x);}   /* Jobbra megy x-szer */
void crsr_up   (int x){printf("\x1B[%dA",x);}   /* Fel megy x-szer */
void crsr_down (int x){printf("\x1B[%dB",x);}   /* Le megy x-szer */

void crsr_push(void){printf("\x1B[s");}   /* Lementi a CRSR poziciojat */
void crsr_pop (void){printf("\x1B[u");}   /* Visszatolti  -||-         */

void mvprintc(int y,int x,char c){printf("\x1B[%d;%df%c",y,x,c);}
void refresh(){ fflush(stdout);}

/******************************************************************************/

void draw_box(int x1,int y1,int xs,int ys){
 int i;
 char keret_tomb[3][8]={{'Ú','Ä','¿','³','³','À','Ä','Ù'},
                         {'É','Í','»','º','º','È','Í','¼'},
                         {'+','-','+','|','|','+','-','+'}};
 char *keret=keret_tomb[2];  /*0:egyszeru;1:kettos;2:Unix*/
 set_color(7);
 gotoxy(x1,y1);
 putchar(keret[0]);for(i=0;i<xs;i++) putchar(keret[1]);putchar(keret[2]);
 for(i=y1+1;i<y1+ys;i++){
    gotoxy(x1,i); printf("%c%*s%c",keret[3],xs,"",keret[4]);
 }
 gotoxy(x1,y1+ys);
 putchar(keret[5]);for(i=0;i<xs;i++) putchar(keret[6]);putchar(keret[7]);
}

void box_message(char* t1){
int x1=(term_xs-strlen(t1)-10)/2;
int y1=(term_ys-10)/2;
  draw_box(x1,y1,strlen(t1)+10,4);
  gotoxy(x1+5,y1+2);printf(t1);
  refresh();
}

void box_message2(char* t1,char* t2){
int x1,y1;
int xs=strlen(t1);if(strlen(t2)>xs)xs=strlen(t2);
  x1=(term_xs-xs-10)/2;
  y1=(term_ys-10)/2;
  draw_box(x1,y1,xs+10,6);
  gotoxy(x1+5,y1+2);printf(t1);
  gotoxy(x1+5,y1+4);printf(t2);
  refresh();
}

/******************************************************************************/

void input(int x0,int y0,int xs,char* hova){
int xbase=0;
int x=0;
char fl1=0;
char sor[1024];
  strcpy(sor,hova);
  do{
//    gotoxy(1,1);printf("%d/%d (%d) ",x,xs,strlen(sor)); // debug
    gotoxy(x0,y0);printf("%-*.*s",xs,xs,sor+xbase);gotoxy(x0+x,y0);refresh();
    waitkey();
    if((gomb>=32)&&(gomb<255)){     /* insert char */
//      if(strlen(sor)<xs){
	      if(!fl1) sor[1]=0; else memmove(sor+x+xbase+1,sor+x+xbase,strlen(sor)-x-xbase+1);
        sor[xbase+x]=gomb;
        gomb=KEY_RIGHT; // trick
//      }
    }
    fl1=1;
    if((gomb==KEY_BS)&&(x+xbase>0)){  /* backspace */
      memmove(sor+x+xbase-1,sor+x+xbase,strlen(sor)-x-xbase+1);
      gomb=KEY_LEFT; //	--x;
    }
    if(gomb==KEY_LEFT){ if(x>0) --x; else if(xbase>0) --xbase;}
    if(gomb==KEY_HOME) x=xbase=0;
    if(gomb==KEY_END){
        x=strlen(sor);
        if(x>=xs-1){ x=xs-1; xbase=strlen(sor)-x;}
    }
    if(gomb==KEY_RIGHT && x+xbase<strlen(sor)){
        if(x>=xs-1) ++xbase; else ++x;
    }
    if((gomb==KEY_DEL)&&(x+xbase<strlen(sor))){  /* DEL */
      memmove(sor+x+xbase,sor+x+xbase+1,strlen(sor)-x-xbase+1);
    }
    if(gomb==KEY_F+3) return;
    if(gomb==KEY_ENTER){strcpy(hova,sor);return;}
  }while(gomb!=KEY_ESC);
}

void box_input(int y1,int xs,char* t1,char* sor){
int x1=(term_xs-xs-10)/2;
if(y1==0)y1=(term_ys-10)/2;
  draw_box(x1,y1,xs+6,5);
  gotoxy(x1+4,y1+1);printf(t1);
  set_color(0);
  input(x1+4,y1+3,xs,sor);
}

/******************************************************************************/

/*
int main(){
char sor[256];

load_termcap(NULL);
getch2_enable();

strcpy(sor,"ez van");
clrscr();
box_input(5,60,"Hello world",sor);

getch2_disable();
return 0;
}
*/

/*******************************************************************************
				    E N D
*******************************************************************************/

