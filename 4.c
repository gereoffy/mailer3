
/*   GyikSoft Mailer for UNIX v2.0   (C) 1997-99 by GyikSoft / ESP-team   */

   /*            Compile:   cc 4.c -lcurses -o mailer               */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curses.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "config.h"

#define MIMEFLAG_PQ 1
#define MIMEFLAG_ISO 2
#define MIMEFLAG_B64 4

int auto_mimeflags=0;
int skip_header=1;
char search_str[256];
int case_insensitive=1;

struct rek_tip {
        char _from[80];
        char _to[64];
        char _subject[70];
        char _date[40];
        long int _pos;
        long int _size;
        long int _msize;
        short int _flags;
};

/* kirajzolando sorok szama: */
#define YS (term_ys-2)

char spck[]="                                                                      ";
char tmpsor[256];

struct rek_tip level[maxlevel];
char puffer[puffsize];
long int puffer_pos=0;
long int puffer_size=0;
long int puffer_mut=0;
unsigned long int sor_pos,eol_pos;
int eof_jel;
int eol_jel;

char temp1[sormaxsize];
char folder_nev[128];
int folder_size=0;
char new_mails=0;

int from_mod=2;

int y0=0,xx=0,yy=0,mail_db=0;
int last_yy,last_y0=-1;
int auto_refresh=0;

char sor[sormaxsize];
char sor1[sormaxsize];
char sor2[sormaxsize];
char sor3[sormaxsize];
char kitol[80];
int i=0;
FILE *f1,*f2,*f3,*f4;

char _from[80];
char _to[80];
char _subject[80];
char _date[80];

/*******************************************************************************
	                    ****  ISO->ASCII  ****
*******************************************************************************/

#include "iso.inc"

/*******************************************************************************
                                ****  MFS  ****
*******************************************************************************/
int mfs;            /* "mail filesystem" */
#define MFS_CNM 0
#define MFS_101 1
#define MFS_PMM 2
#define MFS_INBOX 3
#define MFS_HIX 5
#define MFS_DIGEST 7

/*******************************************************************************
          ****  TERMINAL I/O driver v1.1  (C) 1998. by GyikSoft  ***       
*******************************************************************************/

#include "string.inc"
#include "term.inc"
#include "menu.inc"

/******************************************************************************/
/******************************************************************************/

void addressbook_get(char* hova){
FILE *fa;
int i;
char sor[80];
char fnev[256];
  strcpy(fnev,ADDRESS_BOOK_FILE);
ujra:
  i=0;
  if(!(fa=fopen(fnev,"rt"))) return;
  while(!feof(fa)){
    fgets(sor,80,fa);
/*    printf("%d %d %d\n",sor[0],sor[1],sor[2]);refresh();waitkey();  */
    sor[strlen(sor)-1]=0;
    if(sor[0]!=0){
      strncpy2(menuitems1[i],sor,80);
      menuitems2[i][0]=0;
      i++;
    }
  }
  fclose(fa);
  if(i==0)return;
  i=draw_menu(i-1,menu_addr_name,0);
  if(i>=0){
    strcpy(hova,menuitems1[i]);
    if(!strchr(menuitems1[i],'@')){
      strcpy(fnev,menuitems1[i]);
      goto ujra;
    }
  }
}

void addressbook_add(char* ujcim){
FILE *fa;
  if(!(fa=fopen(ADDRESS_BOOK_FILE,"a")))return;
  fprintf(fa,"%s\n",ujcim);
  fclose(fa);
}

/******************************************************************************/
/******************************************************************************/

int redrawcnt=0;

void redraw(){
  /* printf("\x1B[2J"); */  /* clrscr */
  int xs1=term_xs/2-12;
  int xs2=term_xs-xs1-12;
  for(i=0;i<YS;i++){
   if((last_y0!=y0)||((i+y0)==yy)||((i+y0)==last_yy)){ 
    gotoxy(0,i+2);
 /*   if(yy==(i+y0))
       {backgcolor(7);textcolor(0);}
    else
       {backgcolor(0);textcolor(7);}
*/
    if(yy==(i+y0)) set_color(7); else set_color(0);
 
    if((y0+i)<mail_db)
/*      printf("%-28.28s %8ld  %-.40s\x1B[K\n",  */
      printf("%-*.*s %8ld  %-*.*s\n",
                xs1,xs1,strofs2(cim_ertelmezo(level[y0+i]._from,from_mod),xx),
		level[y0+i]._msize,
		xs2,xs2,strofs2(level[y0+i]._subject,xx)
	    );
    else
      printf("\x1B[K\n");
   }   
  }
  last_y0=y0;last_yy=yy;

  /* a STATUSZ-sor kiirasa legfelulre: */
  set_color(0);
  gotoxy(0,0);
  printf("%d/%d  Y0=%d  Term: %dx%d  Pos=%ld Size=%ld  %d\x1B[K",
    yy+1,mail_db,y0,tigetnum("cols"),tigetnum("lines"),level[yy]._pos,
    level[yy]._size,redrawcnt++);
  if(new_mails){gotoxy(term_xs-4,0);printf("!%d!",new_mails);}
  
 gotoxy(0,yy-y0+2);
 refresh();
 
}

char str_add_tmp[256];

char* str_add(char* s1,char* s2){
  strcpy(str_add_tmp,s1);
  strcat(str_add_tmp,s2);
  return(str_add_tmp);
}


void exec2(char* s1){
  /* curses uninit */
  
  curses_ki();
  clrscr();

  /*EXEC*/
  system(s1);

  curses_be();
  clrscr();refresh();
}

void write_signature(){
FILE *fs;
int c;
  if((fs=fopen(SIGNATURE_FILE,"rb"))){
    while((c=fgetc(fs))!=-1)fputc(c,f2);
    fclose(fs);
  }
}


/* egyelore nem hasznalt. */
int save_mail(struct rek_tip* lev){
long int hossz;
  fseek(f1,lev->_pos,0);
 /* printf("size=%d\n  From:%s\n\n",lev->_msize,lev->_from);refresh(); */
  f2=fopen(temp_nev,"wb");
  if(f2==NULL){printf("Cannot create tempfile\n");return(-1);};
  hossz=lev->_size;
  while(hossz>puffsize){
    fread(&puffer,1,puffsize,f1);
    fwrite(&puffer,1,puffsize,f2);
    hossz-=puffsize;
  }
    fread(&puffer,1,hossz,f1);
    fwrite(&puffer,1,hossz,f2);
  fclose(f2);
  exec2(str_add(EDITOR_CMD,temp_nev));
  /* getch(); */
  return(0);
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
char xxx[256];
FILE *f_cim;
 do{
  clrscr();refresh();
  compose_redraw();box_input(10,60,"From:",_from);
  if(gomb==SCAN_ESC)return;
uj_to:;
  compose_redraw();box_input(10,60,"To:",_to);
  if(gomb==SCAN_ESC)return;
  if(gomb==SCAN_F+3){
    addressbook_get(_to);
    clrscr();
    goto uj_to;
  }
  compose_redraw();box_input(10,60,"Subject:",_subject);
  if(gomb==SCAN_ESC)return;
  exec2(str_add(EDITOR_CMD,temp_nev));
  /* redraw(); */
  clrscr();box_message("Are you sure?",7);waitkey();
  if(gomb=='y'){
    f_cim=fopen(cim_temp_nev,"wb");
    fprintf(f_cim,"From: %s\nTo: %s\nSubject: %s\n",_from,_to,_subject);
    fprintf(f_cim,"X-Mailer: GyikSoft Mailer for UNIX v2.0\n\n");
    fclose(f_cim);
    sprintf(xxx,"cat %s %s | " SENDMAIL " %s",cim_temp_nev,temp_nev,cim_ertelmezo(_to,1));
    exec2(xxx);
    return;
  }
 }while(1);
}


#define max_mime max_menuitems
long int mime_start[max_mime];
long int mime_end[max_mime];
unsigned char mime_flags[max_mime];
int mime_db;
char replystr[16];
int include_orig=0;

/*******************************************************************************
      View_MAIL v3.0   (C) 1997-98 by GyikSoft, C version by GyikSoft 1998.
*******************************************************************************/

   /*  az i-edik Base64 kodolasu darab kikodolasa */
   void save_decoded(int i,char* nev){
   FILE *ff;
   char header_ok;
     ff=fopen(nev,"wb");
     if(!ff){
       box_message("Cannot create file",7);waitkey();
       return;
     }
     fseek(f1,mime_start[i],0);puffer_update();
     eol_jel=0;eol_pos=mime_end[i];
     header_ok=1;
     do{
       readln_sor();
       if(header_ok){
         if(sor[0]==0)header_ok=0;
       }else{
         if(sor[0]==0)goto kesz;
	 decode_b64(sor);
	 fwrite(decoded,1,decoded_size,ff);
       }
     }while(!eol_jel);
     box_message("Unexpected end of base64-encoded file!",7);waitkey();
   kesz:;
     fclose(ff);
   }

/*  az i-edik leveldarab kimentese (f2)-be  */
void save_part(int i){
  int b64_jel=(mime_flags[i] & MIMEFLAG_B64);
  int header_ok=1;
  int b64_ok;
  FILE *ft2;
  char sor2[sormaxsize];
  char sor3[sormaxsize];
  char* p;
  
  fseek(f1,mime_start[i],0);puffer_update();
  eol_jel=0;eol_pos=mime_end[i];
  sor3[0]=0;
  do{
     readln_sor();
     if(header_ok){
	/* a headerben vagyunk? */
       if(sor[0]==0) header_ok=0;
/*       upcstr(sor2,sor); if(strstr(sor2,"BASE64")) b64_jel=1; */
     }else{
       if(b64_jel){
         b64_ok=1;
         if(include_orig){
	   box_message2("Include this part? (Y/N)",menuitems1[i],7);
	   waitkey(); b64_ok=(gomb=='y');   
	 } 
	 /* Ez itt a base64 file dekodolo: */
         if((ft2=fopen(cim_temp_nev,"w+"))){
	   do{
	     decode_b64(sor);
	     fwrite(decoded,1,decoded_size,ft2);
	     readln_sor();
	   }while(!eol_jel && sor[0]);
	   rewind(ft2);
	     if(b64_ok)
	     while(!feof(ft2)){
	       fgets(sor,LINEWRAP,ft2);
	       sor[strlen(sor)-1]=0;
	       while((p=strchr(sor,13))) (*p)=' ';
               fprintf(f2,"%s%s\n",replystr,sor);
	     }
	   fclose(ft2);
	 }
       }
       b64_jel=0;
     }

  if( !skip_header 
   || !header_ok
   || strncmp(sor2,"SUBJECT: ",9)==0
   || strncmp(sor2,"FROM: ",6)==0 )
  if(!header_ok && (mime_flags[i]|auto_mimeflags) & MIMEFLAG_PQ ){

#if 0
     strcat(sor3,hexa2ascii(sor));
     if(sor3[strlen(sor3)-1]=='='){
       sor3[strlen(sor3)-1]=0;
     } else {
       fprintf(f2,"%s%s\n",replystr,sor3);
       sor3[0]=0;
     }
#else
     if(sor[strlen(sor)-1]=='='){
       sor[strlen(sor)-1]=0;
       strcat(sor3,hexa2ascii(sor));
     } else {
       strcat(sor3,hexa2ascii(sor));
       fprintf(f2,"%s%s\n",replystr,sor3);
       sor3[0]=0;
     }
#endif

     if(strlen(sor3)>LINEWRAP){
       fprintf(f2,"%s%s\n",replystr,sor3);
       sor3[0]=0;
     }

  } else {

       fprintf(f2,"%s%s\n",replystr,sor);
  
  }

  }while(!eol_jel);
  fprintf(f2,"%s%s\n",replystr,sor3);
}

void view_mail(struct rek_tip* lev){

#define bound_maxdb 10
#define bound_maxsize 80

char boundary[bound_maxdb][bound_maxsize];
int boundary_db;

int i,j;
char usor[sormaxsize];
char last_jel,header_ok;
char nev[256];     /* save-hoz */
char field[256];   /* pl: "CONTENT_ENCODING:" */

  fseek(f1,lev->_pos,0);
  puffer_update();
  eof_jel=0;
  eol_jel=0;eol_pos=lev->_pos+lev->_size;

  mime_db=0;
  boundary_db=0;
  boundary[0][0]=0;
  mime_start[0]=puffer_pos+puffer_mut;
  mime_flags[0]=0;
  strcpy(menuitems1[0],"header");
  strcpy(menuitems2[0],"text");

  header_ok=1; field[0]=0;
  last_jel=0;

  clrscr();

  do{
    readln_sor();upcstr(usor,sor);

    if(header_ok){
      if(sor[0]==0){ header_ok=0; goto nextline; }
      if(usor[0]!=' ' && usor[0]!=9){
        char *p=strchr(usor,':');
	if(!p) field[0]=0; else {
	  int l=p-usor;
	  strncpy(field,usor,l); field[l]=0;
	  /* upcstr(usor,sor+l+1); */
	}
      }

      if(strcmp(field,"CONTENT-TYPE")==0){
        if((i=strpos("BOUNDARY",usor))>=0){
  	  i+=9;i+=strposc(34,sor+i);
	  j=strposc(34,sor+i);if(j==0)j=255;
	  copy(boundary[boundary_db++],sor,i,j-1);
	  goto nextline;
        }
	if(strstr(usor,"ISO-8859")) mime_flags[mime_db]|=MIMEFLAG_ISO;
        if((i=strpos("NAME=",usor))>=0){
	  copy(menuitems1[mime_db],sor,i+5,strposc(34,&sor[i+6])+1);
	} else if(strncmp(usor,"CONTENT-TYPE:",13)==0)
	  copy(menuitems1[mime_db],sor,14,menuitem1_max-1);
      }

      if(strcmp(field,"CONTENT-TRANSFER-ENCODING")==0){
	if(strstr(usor,"QUOTED-PRINTABLE")) mime_flags[mime_db]|=MIMEFLAG_PQ;
	if(strstr(usor,"BASE64")) mime_flags[mime_db]|=MIMEFLAG_B64;
        copy(menuitems2[mime_db],sor,27,menuitem2_max-1);
      }
    }

    if(!header_ok){
      /* mime boundary-k keresese a szovegben... */
#if 0
      i=j=0;
      do{
	if(boundary[j][0]) if(strstr(sor,str_add("--",boundary[j]))) i=1;
	++j;
      }while((j<boundary_db)&&(!i));
      if(i){
#else
      j=0;
      do{
        i=-1;if(boundary[j][0]!=0)i=strpos(str_add("--",boundary[j]),sor);
	++j;
      }while((j<=boundary_db)&&(i==-1));
      if(i!=-1){
#endif
        mime_end[mime_db]=sor_pos+i;
	if(last_jel){ ++mime_db; }
	  mime_start[mime_db]=sor_pos+i;
	  mime_flags[mime_db]=0;
	  menuitems1[mime_db][0]=0;
	  menuitems2[mime_db][0]=0;
	last_jel=0;
	header_ok=1;
      }else{
        if(sor[0]!=0)last_jel=1;
	/*  UUEncoded rutin helye */
      }     
    }
    
  nextline:;
  }while(!eol_jel);
  mime_end[mime_db]=puffer_pos+puffer_mut;
  if(!last_jel)--mime_db;

#if 0
  clrscr();
  printf("Mime-db: %d   Boundary_db: %d\n\r",mime_db,boundary_db);
  for(i=0;i<=mime_db;i++){
    printf("%10d%8d  %-28.28s  %-28.28s\n\r",mime_start[i],mime_end[i]-mime_start[i],menuitems1[i],menuitems2[i]);
  }
  refresh();
  waitkey();
  clrscr();
#endif

  menu_tipus=-1;

  if(include_orig){
    if((f2=fopen(temp_nev,"wb"))){
      for(i=0;i<=mime_db;i++) save_part(i);
      fclose(f2);
    }  
    return;
  }  
  
  do{
      clrscr();
      if(mime_db) i=draw_menu(mime_db+1,menu_mime,0); else {i=0;menu_yy=0;}
      if(gomb==13){
        f2=fopen(temp_nev,"wb");
	if(f2){
	  save_part(menu_yy);
	  fclose(f2);
	  exec2(str_add(EDITOR_CMD,temp_nev));
	}
      }
      if(gomb=='r'){
      }
      if(gomb=='s'){
        strcpy(nev,"savemail.txt");
	if(menuitems1[menu_yy][0]==34){
	  copy(nev,menuitems1[menu_yy],1,strposc(34,menuitems1[menu_yy]+1)-1);
	}
	gotoxy(1,1);printf(" i=%d ",menu_yy);
        box_input(10,60,"Save to:",nev);
	if(gomb!=SCAN_ESC)save_decoded(menu_yy,nev);
	gomb=-1;
      }  
  }while(gomb!=SCAN_ESC && mime_db!=0);
  gomb=-1;
  return;
}




int main(int argc,char* argv[]) {

  if(argc>1)
    strcpy(folder_nev,argv[1]);
  else
    strcpy(folder_nev,getenv("MAIL"));

  mfs=MFS_INBOX;if(strpos(".pmm",folder_nev)!=-1)mfs=MFS_PMM;

  /* CURSES init */
  curses_be();
  clear();refresh();
  clrscr();
  box_message("Reading folder...",7);

  /*  restart  */
  f1=fopen(folder_nev,"rb");if(f1==NULL){printf("File not found\n");endwin();return(-1);};
  if(mfs==MFS_PMM)fseek(f1,128,0);
  puffer_update();
  eof_jel=0;mail_db=0;
  sor_pos=puffer_pos+puffer_mut;
  do{
    level[mail_db]._pos=sor_pos;
    eol_jel=0;eol_pos=-1;
    do{
      readln_sor();
      if(eof_jel) break;
#if 1
      if(strncmp(sor,"From:",5)==0)strcpy(level[mail_db]._from,iso(sor+6));
      if(strncmp(sor,"To:",3)==0)strcpy(level[mail_db]._to,sor+4);
      if(strncmp(sor,"Subject:",8)==0)strcpy(level[mail_db]._subject,iso(sor+9));
      if(strncmp(sor,"Date:",5)==0)strcpy(level[mail_db]._date,sor+6);
#endif
    }while(!eol_jel && sor[0]);
    level[mail_db]._msize=puffer_pos+puffer_mut;
    do{
      if(eol_jel) break;
      readln_sor();
    }while(mfs!=MFS_INBOX || strncmp(sor,"From ",5) );
    level[mail_db]._msize=sor_pos-level[mail_db]._msize;
    if((level[mail_db]._size=sor_pos-level[mail_db]._pos)) ++mail_db;
    if(eol_jel) ++sor_pos;
  }while(!eof_jel);

 /* printf("%d mails found.\n",mail_db); */
   folder_size=filesize(folder_nev);

refresh();
yy=mail_db-1;


ujra:;
clrscr();last_y0=-1;
ujra2:;
do{
if(auto_refresh)last_y0=-1;

  if(yy<0)yy=0;
  if(yy>=mail_db)yy=mail_db-1;
  if(yy<y0)y0=yy;
  if(yy>=(y0+YS))y0=yy-YS+1;

  redraw();

  /* 0 kell a HALFDELAY tesztelesehez */
#if 1 
  do{
    if(folder_size!=filesize(folder_nev)){
      ++new_mails;
      folder_size=filesize(folder_nev);
      break;
    }
  } while(waitkey2()==255);
#else
  waitkey2();  
#endif

  if(gomb==SCAN_UP)--yy;
  if(gomb==SCAN_DOWN)++yy;
  if(gomb==SCAN_LEFT && xx>0){--xx;goto ujra;}
  if(gomb==SCAN_RIGHT){++xx;goto ujra;}
  if(gomb==SCAN_PGUP)yy-=YS-1;
  if(gomb==SCAN_PGDWN)yy+=YS-1;
  if(gomb==SCAN_END)yy=mail_db-1;
  if(gomb==SCAN_HOME)yy=0;
 
  /* VIEW MIME-DECODED MAIL */
  if(gomb==13){
    include_orig=0;
    view_mail(&level[yy]);
    goto ujra;
  }

  /* VIEW RAW MAIL SOURCE */
  if(gomb==SCAN_ESCENTER){
    include_orig=0;
    save_mail(&level[yy]);
    goto ujra;
  }

  /* F = TOGGLE From: MODE */
  if(gomb=='F'){
    if(++from_mod==3)from_mod=0;
    goto ujra;
  }

  /* P = TOGGLE quoted-printable MODE */
  if(gomb=='P'){
    auto_mimeflags^=MIMEFLAG_PQ;
    goto ujra2;
  }

  /* H = TOGGLE skip-header MODE */
  if(gomb=='H'){
    skip_header^=1;
    goto ujra2;
  }

  /* I = TOGGLE case-insensitive MODE */
  if(gomb=='I'){
    case_insensitive^=1;
    goto ujra2;
  }

  /* ADD TO ADDRESS_BOOK */
  if(gomb=='a'){
    strcpy(sor,level[yy]._from);
    box_input(10,70,"Save to address book?",sor);
    if(gomb!=SCAN_ESC)addressbook_add(sor);
    goto ujra;
  }

  /* REPLY */
  if(gomb=='r'){
    strcpy(_from,__from);
    strcpy(_to,level[yy]._from);
    strcpy(_subject,"Re: ");strcat(_subject,level[yy]._subject);
    set_date(_date);
    include_orig=1; strcpy(replystr,"> ");
    view_mail(&level[yy]);
    strcpy(replystr,"");
    compose();
    goto ujra;
  }

  /* FORWARD */
  if(gomb=='f'){
    strcpy(_from,__from);
    strcpy(_to,level[yy]._from);
    strcpy(_subject,"Fwd: ");strcat(_subject,level[yy]._subject);
    set_date(_date);
    include_orig=1;
    view_mail(&level[yy]);
    compose();
    goto ujra;
  }

  /* s = SEARCH FIRST */
  if(gomb=='s' || gomb=='n'){
    int xxx=gomb;
    box_input(10,60,"Search:",search_str);
    if(gomb==SCAN_ESC) goto ujra;
    clrscr();last_y0=-1;  /* goto ujra; helyett */
    gomb=xxx-32;
  }
  
  /* S = SEARCH BACKWARD NEXT */
  if(gomb=='S'){
    int i;
    if(case_insensitive){
      upcstr(search_str,search_str);
      for(i=yy-1;i>=0;i--){
        if( strstr(upcstr(sor2,level[i]._from),search_str)
         || strstr(upcstr(sor2,level[i]._to),search_str)
         || strstr(upcstr(sor2,level[i]._subject),search_str) ) {
          yy=i;
  	  goto ujra2;
        }
      }
    } else
      for(i=yy-1;i>=0;i--){
        if( strstr(level[i]._from,search_str)
         || strstr(level[i]._to,search_str)
         || strstr(level[i]._subject,search_str) ) {
          yy=i;
  	  goto ujra2;
        }
      }
    goto ujra2;
  }  

  /* N = SEARCH FORWARD NEXT */
  if(gomb=='N'){
    int i;
    if(case_insensitive){
      upcstr(search_str,search_str);
      for(i=yy+1;i<mail_db;i++){
        if( strstr(upcstr(sor2,level[i]._from),search_str)
         || strstr(upcstr(sor2,level[i]._to),search_str)
         || strstr(upcstr(sor2,level[i]._subject),search_str) ) {
          yy=i;
  	  goto ujra2;
        }
      }
    } else
      for(i=yy+1;i<mail_db;i++){
        if( strstr(level[i]._from,search_str)
         || strstr(level[i]._to,search_str)
         || strstr(level[i]._subject,search_str) ) {
          yy=i;
  	  goto ujra2;
        }
      }
    goto ujra2;
  }  


  /* p = SEARCH FORWARD NEXT SAME From: */
  if(gomb=='p'){
    int i;
      strcpy(sor2,cim_ertelmezo(level[yy]._from,1));
      for(i=yy+1;i<mail_db;i++){
        if( strcmp(sor2,cim_ertelmezo(level[i]._from,1))==0 ) {
          yy=i;
  	  goto ujra2;
        }
      }
    goto ujra2;
  }  

  /* P = SEARCH BACKWARD NEXT SAME From: */
  if(gomb=='P'-64){
    int i;
      strcpy(sor2,cim_ertelmezo(level[yy]._from,1));
      for(i=yy-1;i>=0;i--){
        if( strcmp(sor2,cim_ertelmezo(level[i]._from,1))==0 ) {
          yy=i;
  	  goto ujra2;
        }
      }
    goto ujra2;
  }  


  /* COMPOSE */ 
  if(gomb=='c'){
    strcpy(_from,__from);
    /* 
      strcpy(_to,level[yy]._from);
      strcpy(_subject,"Re: ");strcat(_subject,level[yy]._subject);
    */
    set_date(_date);
    f2=fopen(temp_nev,"wb");
    if(f2==NULL){printf("Cannot create tempfile\n");return(-1);}
    fprintf(f2,"Hi,\n");
    write_signature();
    fclose(f2);
    compose();
    goto ujra;
  }

}while(gomb!=SCAN_ESC);

  fclose(f1);
  remove(temp_nev);
  remove(cim_temp_nev);
  
set_color(0);endwin();
return(0);
}


