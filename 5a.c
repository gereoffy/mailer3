#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "libmail.h"
#include "config.h"
#include "term1.c"

/******************************************************************************/

/* kirajzolando sorok szama: */
#define YS (term_ys-2)
int from_mod=2;
int y0=0,xx=0,yy=0;
int last_yy,last_y0=-1;
int auto_refresh=0;
int skip_header=1;
char search_str[256];
int case_insensitive=1;
char new_mails=0;
char last_new_mails;

char _from[80];
char _to[80];
char _subject[80];
char _date[80];

/******************************************************************************/
int menu_tipus=-1;
int menu_yy=0;
int menu_y0=0;
int refresh_time;

#define MENUTYPE_ADDRBOOK 1
#define MENUTYPE_SELFOLDER 3
#define MENUTYPE_FILESELECTOR 4
#define MENUTYPE_ENCODING 5
#define MENUTYPE_MULTIPART 6

#include "menu1.c"

/*******************************************************************************
                		--==>    R E D R A W   <==--
*******************************************************************************/

/* int redrawcnt=0; */

#define M_FROM(x) (&folder->f_strings[folder->f_mails[x].from])
#define M_TO(x) (&folder->f_strings[folder->f_mails[x].to])
#define M_SUBJ(x) (&folder->f_strings[folder->f_mails[x].subject])
#define M_POS(x) (folder->f_mails[x].pos)
#define M_SIZE(x) (folder->f_mails[x].size)
#define M_MSIZE(x) (folder->f_mails[x].msize)
#define M_FLAGS(x) (folder->f_mails[x].flags)
#define MAIL_DB (folder->mail_db)
#define UPDATE_REK(i) write_rek(folder,i,&folder->f_mails[i])

void redraw(){
int i;
  /* printf("\x1B[2J"); */  /* clrscr */
  int xs1=term_xs/2-20;
  int xs2=term_xs-xs1-13;
  for(i=0;i<YS;i++){
   if((last_y0!=y0)||((i+y0)==yy)||((i+y0)==last_yy)){ 
    gotoxy(0,i+2);
    if(yy==(i+y0)) set_color(7); else set_color(0);

    if((y0+i)<MAIL_DB)
/*      printf("%-28.28s %8ld  %-.40s\x1B[K\n",  */
      printf("%c %-*.*s %8d %-*.*s\n",
        (M_FLAGS(y0+i)&MAILFLAG_REPLY) ? 'R' :
          ((M_FLAGS(y0+i)&MAILFLAG_READ) ? 'r' :
            ((M_FLAGS(y0+i)&MAILFLAG_NEW) ? '!' : ' ')
          ),
        xs1,xs1,strofs2(cim_ertelmezo(M_FROM(y0+i),from_mod),xx),
		    M_MSIZE(y0+i),
		    xs2,xs2,strofs2(M_SUBJ(y0+i),xx)
	    );
    else
      printf("\x1B[K\n");
   }   
  }
  last_y0=y0;last_yy=yy;

  /* a STATUSZ-sor kiirasa legfelulre: */
  set_color(0); gotoxy(0,0);
  printf("%d/%d  Term: %dx%d  Pos=%d  Size=%d  Flags=%c%c%c%c\x1B[K",
    yy+1,MAIL_DB,term_xs,term_ys,M_POS(yy),M_SIZE(yy),
    '0'+from_mod,(default_mimeflags&MIMEFLAG_PQ)?'P':'p',
    skip_header?'h':'H',case_insensitive?'I':'i'/*, redrawcnt++*/);
  if(new_mails){gotoxy(term_xs-4,0);printf("!%d!",new_mails);}
  
  gotoxy(0,yy-y0+2);
  refresh();
 
}

/*******************************************************************************
                		--==>    V I E W   <==--
*******************************************************************************/

void exec2(char* s1,char *s2){
  char cmd[sormaxsize];
  strcpy(cmd,s1);strcat(cmd,s2);
/*  printf("Executing '%s'\n",cmd);waitkey(); */
  clrscr();refresh();
  getch2_disable();
  system(cmd); /*EXEC*/
  getch2_enable();
  clrscr();refresh();
}

void view_mail(rek_st *mail){
int i;
  open_mail(folder,mail);
  if(mime_db<1) return;
  do{
      clrscr();
      if(mime_db>1) i=draw_menu_mimeparts(mime_db,MENUTYPE_MULTIPART,0); 
               else {i=0;menu_yy=0;}
      if(gomb==KEY_ENTER){
        FILE* f2=fopen(temp_nev,"wt");
        if(f2){
          save_part(folder,menu_yy,f2,"",skip_header);
          fclose(f2);
          exec2(EDITOR_CMD,temp_nev);
        }
      }
      if(gomb=='s' || gomb==KEY_F+2){
        char nev[256];
        char *n=mime_parts[menu_yy].name;
        strcpy(nev,"savemail.txt");
        if(*n==34){
          copy(nev,n,1,strposc(34,n+1)-1);
        } else if(*n) strcpy(nev,n);
        /* gotoxy(1,1);printf(" i=%d ",menu_yy); */
        box_input(10,60,"Save as:",nev);
        if(gomb==KEY_ENTER){
          FILE *f=fopen(nev,"wb");
          if(f){ save_attachment(folder,menu_yy,f); fclose(f); }
        }
        gomb=-1;
      }
  }while(gomb!=KEY_ESC && mime_db>1);
  gomb=-1;
  return;
}

/*******************************************************************************
                		--==>    C O M P O S E   <==--
*******************************************************************************/

void write_signature(FILE *f2){
FILE *fs;
int c;
  if((fs=fopen(SIGNATURE_FILE,"rb"))){
    while((c=fgetc(fs))!=-1)fputc(c,f2);
    fclose(fs);
  }
}

void compose_redraw(){
  draw_box(5,3,70,5);
  gotoxy(7,4);printf("From:");
  gotoxy(7,5);printf("To:");
  gotoxy(7,6);printf("Subject:");
  gotoxy(7,7);printf("Date:");
  set_color(0);
  gotoxy(16,4);printf("%s",_from);
  gotoxy(16,5);printf("%s",_to);
  gotoxy(16,6);printf("%s",_subject);
  gotoxy(16,7);printf("%s",_date);
  refresh();
}

void compose(){
 do{
  clrscr();refresh();
  compose_redraw();box_input(10,60,"From:",_from); if(gomb==KEY_ESC)return;
/* uj_to:; */
  compose_redraw();box_input(10,60,"To:",_to); if(gomb==KEY_ESC)return;
/*  if(gomb==KEY_F+3){ addressbook_get(_to); clrscr(); goto uj_to; } */
  compose_redraw();box_input(10,60,"Subject:",_subject); if(gomb==KEY_ESC)return;
  exec2(EDITOR_CMD,temp_nev);
  /* redraw(); */
  clrscr();box_message("Are you sure?");
  do{ waitkey(); }while(gomb!='y' && gomb!='n');
  if(gomb=='y'){
    FILE *f_cim=fopen(cim_temp_nev,"wb");
    fprintf(f_cim,"From: %s\nTo: %s\nSubject: %s\n",_from,_to,_subject);
    fprintf(f_cim,"X-Mailer: GyikSoft Mailer for UNIX v3.0 by www.esp-team.org\n\n");
    fclose(f_cim);
    { char xxx[256];
      sprintf(xxx,"cat %s %s | " SENDMAIL " %s",cim_temp_nev,temp_nev,cim_ertelmezo(_to,1));
      exec2(xxx,"");
    }
    return;
  }
 }while(1);
}


/*******************************************************************************
                		--==>    M A I N   <==--
*******************************************************************************/

void save_mail_source(char *fnev,rek_st *mail){
FILE *f=fopen(fnev,"wb");
void *p=malloc(mail->size);
if(!f || !p) return;
folder_seek(folder,mail->pos);
fread(p,1,mail->size,folder->file_folder);
fwrite(p,1,mail->size,f);
fclose(f);
free(p);
}

void fatal(int n,char *s){
  fprintf(stderr,"mail: FATAL ERROR - %s\n",s);
  exit(n);
}

int main(int argc,char *argv[]){
int folder_size=0;
char *foldername;

  if(argc>1) foldername=argv[1]; else foldername=getenv("MAIL");

restart:
{ int i;
  printf("Reading folder...\n");
  i=open_folder(folder,foldername,"mail.idx","mail.str");
  printf("open_folder return value=%d\n",i);
  if(i) fatal(1,"Cannot open folder/index");
  if(load_folder(folder)) fatal(2,"Cannot load index");
  folder_size=folder->folder_size;
}

printf("mail_db=%d  size=%d\n",folder->mail_db,folder->folder_size);
last_new_mails=new_mails;

/***************** BEGIN ************************/
load_termcap(NULL);
getch2_enable();
/* waitkey(); */
clrscr();

yy=MAIL_DB-1;

ujra:   clrscr();last_y0=-1;
ujra2:
do{
  if(auto_refresh) last_y0=-1;
  if(yy<0)yy=0;
  if(yy>=MAIL_DB)yy=MAIL_DB-1;
  if(yy<y0)y0=yy;
  if(yy>=(y0+YS))y0=yy-YS+1;

  redraw();

  /* 0 kell a HALFDELAY tesztelesehez */
#if 1
  refresh_time=0;
  do{
    if(folder_size<filesize(foldername)){
      ++new_mails;
      folder_size=filesize(foldername);
      gomb=0;break;
    }
    if(new_mails>last_new_mails && (++refresh_time)>5){ gomb='R';break; }
  } while((gomb=getch2(HALFDELAY_TIME))<=0);
#else
  gomb=getch2(HALFDELAY_TIME);
#endif

  if(gomb==KEY_UP)--yy;
  if(gomb==KEY_DOWN)++yy;
  if(gomb==KEY_LEFT && xx>0){--xx;goto ujra;}
  if(gomb==KEY_RIGHT){++xx;goto ujra;}
  if(gomb==KEY_PGUP)yy-=YS-1;
  if(gomb==KEY_PGDWN)yy+=YS-1;
  if(gomb==KEY_END)yy=MAIL_DB-1;
  if(gomb==KEY_HOME)yy=0;
 
  /* VIEW MIME-DECODED MAIL */
  if(gomb==KEY_ENTER){
    view_mail(&folder->f_mails[yy]);
    M_FLAGS(yy)|=MAILFLAG_READ; 
    M_FLAGS(yy)&=(~MAILFLAG_NEW);
    UPDATE_REK(yy);
    goto ujra;
  }

  /* COMPOSE */ 
  if(gomb=='c'){
    strcpy(_from,__from);
    /* 
      strcpy(_to,level[yy]._from);
      strcpy(_subject,"Re: ");strcat(_subject,level[yy]._subject);
    */
    get_date(_date);
    { FILE *f2=fopen(temp_nev,"wb");
      if(!f2){printf("Cannot create tempfile\n");return(-1);}
      fprintf(f2,"Hi,\n\n");
      write_signature(f2);
      fclose(f2);
    }
    compose();
    goto ujra;
  }
  
  /* REPLY & FORWARD */
  if(gomb=='r' || gomb=='f'){
    int reply=(gomb=='r');
    strcpy(_from,__from);
    strcpy(_to,reply? M_FROM(yy) : M_TO(yy));
    strcpy(_subject,reply?"Re: ":"Fwd: ");strcat(_subject,M_SUBJ(yy));
    get_date(_date);
    { FILE *f2=fopen(temp_nev,"wb");
      if(!f2){printf("Cannot create tempfile\n");return(-1);}
      if(reply)
        fprintf(f2,"Hi,\n\n");
      else {
        fprintf(f2,"--------- Forwarded message ---------\n");
        fprintf(f2,"From: %s\nTo: %s\nSubject: %s\n\n",M_FROM(yy),M_TO(yy),M_SUBJ(yy));
      }  
      open_mail(folder,&folder->f_mails[yy]);
      if(mime_db>0){
        int i;
        for(i=0;i<mime_db;i++){
          if(mime_parts[i].flags&MIMEFLAG_B64){
	          box_message2("Include this part? (Y/N)",mime_parts[i].name);
            waitkey(); if(gomb!='y') continue;
          }
          save_part(folder,i,f2,reply?"> ":"",skip_header);
        }
      }
      if(reply) write_signature(f2);
      fclose(f2);
    }
    compose();
    if(reply){ 
      M_FLAGS(yy)|=MAILFLAG_REPLY; 
      M_FLAGS(yy)&=(~MAILFLAG_NEW);
      UPDATE_REK(yy);}
    goto ujra;
  }
  
  /* F = TOGGLE From: MODE */
  if(gomb=='F'){ if(++from_mod==3)from_mod=0; goto ujra; }
  /* P = TOGGLE quoted-printable MODE */
  if(gomb=='P'){ default_mimeflags^=MIMEFLAG_PQ; goto ujra2; }
  /* H = TOGGLE skip-header MODE */
  if(gomb=='H'){ skip_header^=1; goto ujra2; }
  /* I = TOGGLE case-insensitive MODE */
  if(gomb=='I'){ case_insensitive^=1; goto ujra2; }

  /* ADD TO ADDRESS_BOOK */
  if(gomb=='a'){
    strcpy(sor,M_FROM(yy));
    box_input(10,70,"Save to address book?",sor);
    /* if(gomb!=KEY_ESC)addressbook_add(sor); */
    goto ujra;
  }

  /* s = SEARCH FIRST */
  if(gomb=='s' || gomb=='n'){
    int xxx=gomb;
    box_input(10,60,"Search:",search_str); if(gomb==KEY_ESC) goto ujra;
    clrscr();last_y0=-1;  /* goto ujra; helyett */
    gomb=xxx-32;
  }
  /* S = SEARCH BACKWARD NEXT */
  if(gomb=='S'){
    int i;
    if(case_insensitive){
      upcstr(search_str,search_str);
      for(i=yy-1;i>=0;i--){
        if( strstr(upcstr(sor2,M_FROM(i)),search_str)
         || strstr(upcstr(sor2,M_TO(i)),search_str)
         || strstr(upcstr(sor2,M_SUBJ(i)),search_str) ) {
          yy=i; goto ujra2;
        }
      }
    } else
      for(i=yy-1;i>=0;i--){
        if( strstr(M_FROM(i),search_str)
         || strstr(M_TO(i),search_str)
         || strstr(M_SUBJ(i),search_str) ) {
          yy=i; goto ujra2;
        }
      }
    goto ujra2;
  }  

  /* N = SEARCH FORWARD NEXT */
  if(gomb=='N'){
    int i;
    if(case_insensitive){
      upcstr(search_str,search_str);
      for(i=yy+1;i<MAIL_DB;i++){
        if( strstr(upcstr(sor2,M_FROM(i)),search_str)
         || strstr(upcstr(sor2,M_TO(i)),search_str)
         || strstr(upcstr(sor2,M_SUBJ(i)),search_str) ) {
          yy=i;
  	  goto ujra2;
        }
      }
    } else
      for(i=yy+1;i<MAIL_DB;i++){
        if( strstr(M_FROM(i),search_str)
         || strstr(M_TO(i),search_str)
         || strstr(M_SUBJ(i),search_str) ) {
          yy=i;
  	  goto ujra2;
        }
      }
    goto ujra2;
  }  


  /* p = SEARCH FORWARD NEXT SAME From: */
  if(gomb=='p'){
    int i;
      strcpy(sor2,cim_ertelmezo(M_FROM(yy),1));
      for(i=yy+1;i<MAIL_DB;i++){
        if( strcmp(sor2,cim_ertelmezo(M_FROM(i),1))==0 ) {
          yy=i;
  	  goto ujra2;
        }
      }
    goto ujra2;
  }  

  /* P = SEARCH BACKWARD NEXT SAME From: */
  if(gomb=='P'-64){
    int i;
      strcpy(sor2,cim_ertelmezo(M_FROM(yy),1));
      for(i=yy-1;i>=0;i--){
        if( strcmp(sor2,cim_ertelmezo(M_FROM(i),1))==0 ) {
          yy=i;
  	  goto ujra2;
        }
      }
    goto ujra2;
  }  

}while(gomb!=KEY_ESC && gomb!='R');

#if 1
if(gomb!='R'){
  int i;
  for(i=0;i<MAIL_DB;i++) if(M_FLAGS(i)&MAILFLAG_NEW){
    M_FLAGS(i)&=(~MAILFLAG_NEW);
    UPDATE_REK(i);
  }
}
#endif

clrscr();
getch2_disable();
/***************** END ************************/

/* the end */
free_folder(folder);
close_folder(folder);

unlink(temp_nev);
unlink(cim_temp_nev);

if(gomb=='R') goto restart;
return 0;
}

/*******************************************************************************
				    E N D
*******************************************************************************/
