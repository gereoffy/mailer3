/******************************************************************************/
/*                TERMINAL routines v2.0   (curses & ANSI)                    */
/******************************************************************************/

WINDOW *curs_w;   /* main window pointer */
int term_xs=80,term_ys=24;

void curses_ki(){  endwin(); }

void curses_be(){ 
  initscr(); cbreak(); noecho();
  halfdelay(20);
  term_xs=tigetnum("cols");
  term_ys=tigetnum("lines");
}

/* Hany masodpercenkent nezze meg, hogy jott-e ujabb level:  */
#define HALFDELAY_TIME 10

/******************************************************************************/
/*      ****  TERMINAL/KEYBOARD driver v1.3  (C) 1998. by GyikSoft  ***       */
/******************************************************************************/

int gomb;
int c,c1,c2,c3,c4;

#define SCAN2 512
#define SCAN3 SCAN2+256
#define SCAN4 SCAN3+256
#define SCAN_ESCENTER SCAN2+10

#define SCAN_UP SCAN3+65
#define SCAN_DOWN SCAN3+66
#define SCAN_RIGHT SCAN3+67
#define SCAN_LEFT SCAN3+68

#define SCAN_HOME SCAN3+49
#define SCAN_END SCAN3+52
#define SCAN_PGUP SCAN3+53
#define SCAN_PGDWN SCAN3+54

/* funkciobillentyuk.  pl: F3-hoz: SCAN_F+3  */
#define SCAN_F SCAN4+64
#define SCAN_ESC SCAN2+27
#define SCAN_BS 8
#define SCAN_DEL SCAN3+51

int waitkey2(){
  c1=getch();
  if(c1==27){
    c2=getch();
    if(c2==-1) return(gomb=SCAN_ESC);  /* getch() timeout */
    if(c2>='0' && c2<='9') return(gomb=SCAN_F+c2-'0');
    if((c2==91)||(c2==79)){
       c3=getch();
       if((c3==91)||(c3==79)){
         c4=getch();
	 gomb=SCAN4+c4;
       }else{
         gomb=SCAN3+c3;
         if(gomb==51)gomb=SCAN_DEL;
       } 
    } else {gomb=SCAN2+c2;}
  } else {gomb=c1;}
  if(gomb==10)gomb=13;
  if((gomb==127)||(gomb==4))gomb=SCAN_DEL;
  if(gomb=='A'-64)gomb=SCAN_HOME;
  if(gomb=='E'-64)gomb=SCAN_END;
  if(gomb=='U'-64)gomb=SCAN_PGUP;
  if(gomb=='V'-64)gomb=SCAN_PGDWN;
  if(gomb=='B'-64)gomb=SCAN_LEFT;
  if(gomb=='F'-64)gomb=SCAN_RIGHT;
return(gomb);
}

int waitkey(){
  do{}while(waitkey2()==-1);
  return(gomb);
}

/******************************************************************************/
/*      ****  ANSI VIDEO routines v1.0   (C) 1997-98. by GyikSoft        ***  */
/******************************************************************************/


/*  *  *  *  ANSI rutinok  *  *  *  *  */
void clrscr(void){printf("\x1B[2J\x1B[1;1f");}          /* CLS + HOME       */
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

void mvprintc(int y,int x,char c){
printf("\x1B[%d;%df%c",y,x,c);
}

void draw_box(char x1,char y1,char xs,char ys)
{ int i,j;
  char ktip=2;/*0:egyszeru;1:kettos;2:Unix*/
  char keret_tomb[3][8]={{'Ú','Ä','¿','³','³','À','Ä','Ù'},
                         {'É','Í','»','º','º','È','Í','¼'},
                         {'+','-','+','|','|','+','-','+'}
                        }
;

 set_color(7);

 gotoxy(x1,y1);
 printf("%c",keret_tomb[ktip][0]);
 for(i=x1+1;i<x1+xs;i++) printf("%c",keret_tomb[ktip][1]);
 printf("%c",keret_tomb[ktip][2]);
 for(i=y1+1;i<y1+ys;i++){
    gotoxy(x1,i);
    printf("%c",keret_tomb[ktip][3]);
    for(j=x1+1;j<x1+xs;j++)printf(" ");
    printf("%c",keret_tomb[ktip][4]);
 }  
 gotoxy(x1,y1+ys);
 printf("%c",keret_tomb[ktip][5]);
 for(i=x1+1;i<x1+xs;i++) printf("%c",keret_tomb[ktip][6]);
 printf("%c",keret_tomb[ktip][7]);
/* move(yy-y0+1,1); */
/* refresh(); */
}


void input(int x0,int y0,int xs,char* hova){
int x=0;
char fl1=0,fl2;
char sor[160];
  strcpy(sor,hova);
  do{
/*    gotoxy(1,1);printf("%d ",x);  */
    gotoxy(x0,y0);printf("%-*s",xs,sor);
    gotoxy(x0+x,y0);
    refresh();

    fl2=0;
    waitkey();
        if(gomb==SCAN_LEFT){
	  if(x>0)--x;
	  ++fl2;
	}
	if(gomb==SCAN_RIGHT){
	  if((x<(xs-1))&&(x<strlen(sor)))++x;
	  ++fl2;
	}
      if((c1>=32)&&(c1<126)){
        /* insert char */
        if(strlen(sor)<xs){
	  if((x==0)&&(fl1==0))
	    {sor[1]=0;}
	  else
	    {memmove(sor+x+1,sor+x,strlen(sor)-x+1);}
          sor[x]=c1; ++x; ++fl2;
        };
      }	
      if((gomb==SCAN_BS)&&(x>0)){  /* backspace */
        memmove(sor+x-1,sor+x,strlen(sor)-x+1);
	--x;
      }
      if((gomb==SCAN_DEL)&&(x<strlen(sor))){  /* DEL */
        memmove(sor+x,sor+x+1,strlen(sor)-x+1);
      }
      if(gomb==SCAN_F+3)return;
     
    fl1|=fl2;
    if(gomb==13){strcpy(hova,sor);return;}
  }while(gomb!=SCAN_ESC);
}

void box_input(int y1,int xs,char* t1,char* sor){
int x1=(term_xs-xs-10)/2;
if(y1==0)y1=(term_ys-10)/2;
  draw_box(x1,y1,xs+10,5);
  gotoxy(x1+5,y1+1);printf(t1);refresh();
  set_color(0);input(x1+5,y1+3,xs,sor);
}

void box_message(char* t1,char co){
int x1=(term_xs-strlen(t1)-10)/2;
int y1=(term_ys-10)/2;
  draw_box(x1,y1,strlen(t1)+10,4);
  gotoxy(x1+5,y1+2);set_color(co);printf(t1);
  refresh();
}

void box_message2(char* t1,char* t2,char co){
int xs,x1,y1;
xs=strlen(t1);if(strlen(t2)>xs)xs=strlen(t2);
x1=(term_xs-xs-10)/2;
y1=(term_ys-10)/2;
  draw_box(x1,y1,xs+10,6);
  set_color(co);
  gotoxy(x1+5,y1+2);printf(t1);
  gotoxy(x1+5,y1+4);printf(t2);
  refresh();
}


/*******************************************************************************
				    E N D
*******************************************************************************/

