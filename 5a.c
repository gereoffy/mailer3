#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "libmail.h"
#include "config.h"
#include "term1.c"

#define VERSION "GyikSoft Mailer for UNIX v3.99pre2 by Arpi/ESP-team (http://esp-team.scene.hu)\n\n"

/******************************************************************************/

/* kirajzolando sorok szama: */
#define YS (term_ys-2)
static int from_mod=2;
static int y0=0,xx=0,yy=0;
static int last_yy,last_y0=-1;
static int auto_refresh=0;
static int skip_header=1;
static int linewrap=1;
static char new_mails=0;
static char last_new_mails;

// search:
static char search_str_input[256];
static char search_str[256];
static char search_ustr[256];
static int search_flags=0;
static int case_insensitive=1;

// compose:
static char _from[1024];
static char _to[1024];
static char _subject[1024];
static char _date[1024];

/******************************************************************************/
static int menu_tipus=-1;
static int menu_yy=0;
static int menu_y0=0;
static int refresh_time;

static folder_st default_folder={MFS_INBOX,0};
static folder_st *folder=&default_folder;

#define MENUTYPE_ADDRBOOK 1
#define MENUTYPE_ADDRLIST 2
#define MENUTYPE_SELFOLDER 3
#define MENUTYPE_FILESELECTOR 4
#define MENUTYPE_ENCODING 5
#define MENUTYPE_MULTIPART 6

#include "menu1.c"
#include "menu2.c"

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

static int filter_attach=0;
static int filter_deleted=0;
static int filter_selected=0;
static int filter_extra=0;
static int filter_list=0;

static int check_match(int i);

static int m_step(int old,int dist){
  int dir=(dist<0)?-1:1;
  dist*=dir;
  while(dist>0){
//    if(old+dir<0 || old+dir>=MAIL_DB) break;
    old+=dir;
    if(old<0 || old>=MAIL_DB) break; //return -1;
    //if(!filter_selected || check_match(old))
    {
      if(filter_deleted==1 || !(M_FLAGS(old)&MAILFLAG_DEL) ||
        (filter_deleted==2 && (M_FLAGS(old)&MAILFLAG_DEL)) )
      if(!filter_extra || M_FLAGS(old)&MAILFLAG_EXTRA)
      if(!filter_attach || M_FLAGS(old)&MAILFLAG_ATTACH)
      if(!filter_list || (filter_list==1 && M_FLAGS(old)&MAILFLAG_LIST) ||
         (filter_list==2 && !(M_FLAGS(old)&MAILFLAG_LIST)))
      if(!filter_selected || (filter_selected==1 && M_FLAGS(old)&MAILFLAG_SELECTED) ||
         (filter_selected==2 && !(M_FLAGS(old)&MAILFLAG_SELECTED)))
	--dist;
    }
  }
  return old;
}

static void redraw(){
int i;
  /* printf("\x1B[2J"); */  /* clrscr */
  int xs1=term_xs/2-20;
  int xs2=term_xs-xs1-13;
  int y=y0;
  int g_y=1;
  for(i=0;i<YS;i++){
   if((last_y0!=y0)||(y==yy)||(y==last_yy)){ 
    gotoxy(0,i+2);
    if(yy==y){ set_color(7); g_y=i+2; } else set_color(0);

    if(y<MAIL_DB){
      if(M_FLAGS(y)&MAILFLAG_SELECTED) set_color(1);
/*      printf("%-28.28s %8ld  %-.40s\x1B[K\n",  */
      printf("%c%c%-*.*s %8d %-*.*s\n",
        (M_FLAGS(y)&MAILFLAG_DEL) ? 'D' :
        (M_FLAGS(y)&MAILFLAG_REPLY) ? 'R' :
          ((M_FLAGS(y)&MAILFLAG_READ) ? 'r' :
            ((M_FLAGS(y)&MAILFLAG_NEW) ? '!' : ' ')
          ),
	  ((M_FLAGS(y)&MAILFLAG_EXTRA) ? '*' : 
	   ((M_FLAGS(y)&MAILFLAG_ATTACH) ? '+' : ' ')),
        xs1,xs1,strofs2(cim_ertelmezo(M_FROM(y),from_mod),xx),
		    M_MSIZE(y),
//		    (y+1),
		    xs2,xs2,strofs2(M_SUBJ(y),xx)
	    );
    } else
      printf("\x1B[K\n");
   }
   y=m_step(y,1);
  }
  last_y0=y0;last_yy=yy;

  /* a STATUSZ-sor kiirasa legfelulre: */
  set_color(0); gotoxy(0,0);
  printf("[%c%c%c%s%c%s%c%s] %d/%d [%dx%d]  P:%d S:%d  [%c%c%c%c%c]\x1B[K",
    filter_extra ? 'E' : 'e',
    filter_attach ? 'A' : 'a',
    filter_deleted ? 'D' : 'd',
    (filter_deleted==1) ? "+" : "",
    (filter_selected<2) ? 'S' : 's',
    filter_selected ? "" : "+",
    (filter_list<2) ? 'L' : 'l',
    filter_list ? "" : "+",
    yy+1,MAIL_DB,term_xs,term_ys,M_POS(yy),M_SIZE(yy),
    '0'+from_mod,(default_mimeflags&MIMEFLAG_PQ)?'P':'p',
    skip_header?'h':'H', linewrap?'W':'w',
    case_insensitive?'I':'i'/*, redrawcnt++*/);
  if(new_mails){gotoxy(term_xs-4,0);printf("!%d!",new_mails);}
  
  gotoxy(0,g_y);
  refresh();
 
}

/*******************************************************************************
                		--==>    V I E W   <==--
*******************************************************************************/

static void exec2(char* s1,char *s2){
  char cmd[sormaxsize];
  strcpy(cmd,s1);strcat(cmd,s2);
/*  printf("Executing '%s'\n",cmd);waitkey(); */
  clrscr();refresh();
  getch2_disable();
  system(cmd); /*EXEC*/
  getch2_enable();
  clrscr();refresh();
}

static void view_mail(rek_st *mail){
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
          save_part(folder,menu_yy,f2,"",skip_header,linewrap);
          fclose(f2);
          exec2(EDITOR_CMD,temp_nev);
        }
      } else
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

static void write_signature(FILE *f2){
FILE *fs;
int c;
  if((fs=fopen(SIGNATURE_FILE,"rb"))){
    while((c=fgetc(fs))!=-1)fputc(c,f2);
    fclose(fs);
  }
}

static void compose_redraw(){
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

static int compose(){
 while(1){
  clrscr();refresh();
  compose_redraw();box_input(10,60,"From:",_from); if(gomb==KEY_ESC)return 0;
/* uj_to:; */
  compose_redraw();box_input(10,60,"To:",_to); if(gomb==KEY_ESC)return 0;
/*  if(gomb==KEY_F+3){ addressbook_get(_to); clrscr(); goto uj_to; } */
  compose_redraw();box_input(10,60,"Subject:",_subject); if(gomb==KEY_ESC)return 0;
  exec2(EDITOR_CMD,temp_nev);
  /* redraw(); */
  clrscr();box_message("Are you sure?");
  do{ waitkey(); }while(gomb!='y' && gomb!='n');
  if(gomb=='y'){
    FILE *f_cim=fopen(cim_temp_nev,"wb");
    fprintf(f_cim,"From: %s\nTo: %s\nSubject: %s\n",_from,_to,_subject);
    if(message_id[0]) 
      fprintf(f_cim,"In-Reply-To: %s\n",message_id);
    fprintf(f_cim,"X-Mailer: " VERSION);
    fclose(f_cim);
    clrscr();refresh();
    { char xxx[256];
#ifdef COPYSELF
      printf("Copyself...");fflush(stdout);
      sprintf(xxx,"echo \"From %s\" >>" COPYSELF,cim_ertelmezo(_to,1)); exec2(xxx,"");
      sprintf(xxx,"echo \"Date: %s\" >>" COPYSELF,_date); exec2(xxx,"");
      sprintf(xxx,"cat %s %s >>" COPYSELF,cim_temp_nev,temp_nev); exec2(xxx,"");
      exec2("echo >>" COPYSELF,"");
      printf("OK\n");
#endif
      printf("Sending mail...");fflush(stdout);
      sprintf(xxx,"cat %s %s | " SENDMAIL " %s",cim_temp_nev,temp_nev,cim_ertelmezo(_to,1));
      exec2(xxx,"");
      printf("OK\n");
    }
    return 1;
  }
 }
 return 0;
}

/*******************************************************************************
        		--==>    S E A R C H   <==--
*******************************************************************************/

#define SEARCHF_FROM 1
#define SEARCHF_TO 2
#define SEARCHF_SUBJ 4

static void init_search(char *s){
char *p=s;
 if(*p==92){
   ++p;search_flags=0xFF;
 } else {
  search_flags=0;
  folyt:
  switch(*p){
    case 'F': search_flags|=SEARCHF_FROM<<8;
    case 'f': search_flags|=SEARCHF_FROM;
      ++p;goto folyt;
    case 'T': search_flags|=SEARCHF_TO<<8;
    case 't': search_flags|=SEARCHF_TO;
      ++p;goto folyt;
    case 'S': search_flags|=SEARCHF_SUBJ<<8;
    case 's': search_flags|=SEARCHF_SUBJ;
      ++p;goto folyt;
    case ':': ++p; break;
    default: p=s;search_flags=0xFF; /* default: search in all !!!!! */
  }
 }
 if(case_insensitive) search_flags|=(search_flags&0xFF)<<8;
 strcpy(search_str,p);
 upcstr(search_ustr,search_str);
}

static int check_match(int i){
  if(search_flags&(SEARCHF_FROM)){
    if(search_flags&(SEARCHF_FROM<<8)){
      if(strstr(upcstr(sor2,M_FROM(i)),search_ustr)) return 1;
    } else {
      if(strstr(M_FROM(i),search_str)) return 1;
    }
  }
  if(search_flags&(SEARCHF_TO)){
    if(search_flags&(SEARCHF_TO<<8)){
      if(strstr(upcstr(sor2,M_TO(i)),search_ustr)) return 1;
    } else {
      if(strstr(M_TO(i),search_str)) return 1;
    }
  }
  if(search_flags&(SEARCHF_SUBJ)){
    if(search_flags&(SEARCHF_SUBJ<<8)){
      if(strstr(upcstr(sor2,M_SUBJ(i)),search_ustr)) return 1;
    } else {
      if(strstr(M_SUBJ(i),search_str)) return 1;
    }
  }
  return 0;
}

/*******************************************************************************
            		--==>    D E L   <==--
*******************************************************************************/

#if 0
static void delete_mails(){
int i=0,j;
  while(i<MAIL_DB && !(M_FLAGS(i)&MAILFLAG_DEL)) i++;
  if(i>=MAIL_DB) return; // nincs mit torolni
  j=i;
  printf("Deleting mails...\n");
  while(1){
    while(i<MAIL_DB && (M_FLAGS(i)&MAILFLAG_DEL)) i++;
    if(i>=MAIL_DB) break;
    printf("Copy mail %d -> %d\n",i,j);
    folder->f_mails[j]=folder->f_mails[i]; UPDATE_REK(j);
    ++i; ++j;
  }
  printf("Deleted total %d mails\n",MAIL_DB-j);
  MAIL_DB=j;

#if 0
  while(j<i){
    printf("Clearing mail %d\n",j);
//    M_SIZE(j)=0;
    M_MSIZE(j)=0;
    UPDATE_REK(j);
    ++j;
  }
#endif

}
#endif

/*******************************************************************************
                		--==>    M A I N   <==--
*******************************************************************************/

static void fatal(int n,char *s){
  fprintf(stderr,"mail: FATAL ERROR - %s\n",s);
  exit(n);
}

int main(int argc,char *argv[]){
int folder_size=0;
char *foldername;
char *foldername_idx;
char *foldername_str;

  if(argc>1){
      foldername=argv[1];
      foldername_idx=malloc(strlen(foldername)+8);
      foldername_str=malloc(strlen(foldername)+8);
      sprintf(foldername_idx,"%s.idx",foldername);
      sprintf(foldername_str,"%s.str",foldername);
  } else {
      foldername=getenv("MAIL");
      if(!foldername){
          printf("$MAIL not defined - you must specify folder file\n");
          exit(1);
      }
      foldername_idx="MAIL.idx";
      foldername_str="MAIL.str";
  }

load_termcap(NULL);

{ int i;
  printf("Reading folder...\n");
  i=open_folder(folder,foldername,foldername_idx,foldername_str);
  printf("open_folder return value=%d\n",i);
  if(i) fatal(1,"Cannot open folder/index");
}

restart:
{ int i;
  printf("Updating folder...\n");
  i=update_folder(folder);
  printf("update_folder return value=%d\n",i);
  if(i) fatal(1,"Cannot open folder/index");
  folder_size=filesize(foldername); //folder->folder_size;
  if(folder_size!=folder->folder_size){
    printf("foldersize=%d != filesize=%d\n",folder->folder_size,folder_size);
    abort();
  }
}

printf("mail_db=%d  size=%d\n",folder->mail_db,folder->folder_size);
last_new_mails=new_mails;

/***************** BEGIN ************************/
getch2_enable();

//printf("key1=%d\n",waitkey());
//printf("key2=%d\n",waitkey());
//printf("key3=%d\n",waitkey());
//waitkey();


//clrscr();

yy=MAIL_DB-1;

ujra:   clrscr();last_y0=-1;
ujra2:
do{
  if(auto_refresh) last_y0=-1;
  if(yy<m_step(-1,1))yy=m_step(-1,1);
  if(yy>m_step(MAIL_DB,-1))yy=m_step(MAIL_DB,-1);
  if(yy<y0)y0=yy;
  if(yy>=m_step(y0,YS)) y0=m_step(yy,-(YS-1));
  if(y0<m_step(-1,1))y0=m_step(-1,1);
//  if(y0<0)y0=m_step(-1,1);

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

  if(gomb==KEY_UP) yy=m_step(yy,-1);
  if(gomb==KEY_DOWN) yy=m_step(yy,1);
  if(gomb==KEY_PGUP) yy=m_step(yy,-(YS-1));
  if(gomb==KEY_PGDWN)yy=m_step(yy,YS-1);
  if(gomb==KEY_END)yy=MAIL_DB;//m_step(MAIL_DB,-1);
  if(gomb==KEY_HOME)yy=0; //m_step(-1,1);
  if(gomb==KEY_LEFT && xx>0){xx-=5;goto ujra;}
  if(gomb==KEY_RIGHT){xx+=5;goto ujra;}
 
  /* VIEW MIME-DECODED MAIL */
  if(gomb==KEY_ENTER){
    view_mail(&folder->f_mails[yy]);
    M_FLAGS(yy)|=MAILFLAG_READ; 
    M_FLAGS(yy)&=(~MAILFLAG_NEW);
    UPDATE_REK(yy);
    goto ujra;
  }

  if(gomb==KEY_ESC_ENTER){
    save_mail_source(folder,&folder->f_mails[yy],temp_nev);
    exec2(EDITOR_CMD,temp_nev);
    goto ujra;
  }

  /* Select filtering */
  if(gomb=='D'){
    filter_deleted=(filter_deleted+1)%3;
    yy=0;
    goto ujra;
  }
  if(gomb=='L'){
    filter_list=(filter_list+1)%3;
    yy=0;
    goto ujra;
  }
  if(gomb=='A'){
    filter_attach^=1;
    yy=0;
    goto ujra;
  }
  if(gomb=='?'){
    filter_selected=(filter_selected+1)%3;
    yy=0;
    goto ujra;
  }
  if(gomb=='E'){
    filter_extra^=1;
    yy=0;
    goto ujra;
  }

  /* Select for deletion */
  if(gomb==KEY_DEL /* || gomb=='d'*/ ){
    M_FLAGS(yy)^=MAILFLAG_DEL;
    UPDATE_REK(yy);
    if(m_step(yy,1)<MAIL_DB) yy=m_step(yy,1); else
    if(m_step(yy,-1)>=0) yy=m_step(yy,-1); else yy=0;
    goto ujra;
  }

  /* Select for extra */
  if(gomb=='e'){
    M_FLAGS(yy)^=MAILFLAG_EXTRA;
    UPDATE_REK(yy);
    if(m_step(yy,1)<MAIL_DB) yy=m_step(yy,1); else
    if(m_step(yy,-1)>=0) yy=m_step(yy,-1); else yy=0;
    goto ujra;
  }

  /* Upgrade record */
  if(gomb=='u'){
    if(upgrade_rek(folder,&folder->f_mails[yy])>0){
      UPDATE_REK(yy);
      if(m_step(yy,1)<MAIL_DB) yy=m_step(yy,1);
    }
    goto ujra;
  }

  /* Upgrade folder */
  if(gomb=='U'){
    int i;
    int cnt=0;
    clrscr();refresh();
    full_hash_strings(folder);
    for(i=0;i<MAIL_DB;i++){
      rek_st old=folder->f_mails[i];
      if(upgrade_rek(folder,&old)>0 && 
         memcmp(&old,&folder->f_mails[i],sizeof(rek_st))){
	folder->f_mails[i]=old;
        UPDATE_REK(i);
        ++cnt;
	printf("[Updated: %d/%d]  \r",cnt,i);
	//gotoxy(1,1);printf("[Updated: %d/%d]  ",cnt,i);refresh();
      }
    }
    sprintf(sor,"Records changed: %d",cnt);
    box_message(sor); waitkey();
    gomb='R'; //goto ujra;
  }

  /* Dump folder data */
  if(gomb=='}'){
    char nev[256];
    strcpy(nev,"mail-data.txt");
    box_input(10,60,"Save as:",nev);
    if(gomb==KEY_ENTER){
      FILE* f=fopen(nev,"wb");
      if(f){
        int i=m_step(-1,1);
	while(i<MAIL_DB){
	  fprintf(f,"%s|%s\n",cim_ertelmezo(M_FROM(i),from_mod),M_SUBJ(i));
          i=m_step(i,1);
	}
	fclose(f);
      }
    }
    goto ujra;
  }

  /* Copy folder data */
  if(gomb=='{'){
    char nev[256];
    strcpy(nev,"selected-mail-folder");
    box_input(10,60,"Save as:",nev);
    if(gomb==KEY_ENTER){
      FILE* f=fopen(nev,"wb");
      if(f){
        int i=m_step(-1,1);
	while(i<MAIL_DB){
	  save_mail_source_file(folder,&folder->f_mails[i],f);
          i=m_step(i,1);
	}
	fclose(f);
      }
    }
    goto ujra;
  }

  /* Select for deletion */
  if(gomb==KEY_INSERT){
    M_FLAGS(yy)^=MAILFLAG_SELECTED;
    UPDATE_REK(yy);
    if(m_step(yy,1)<MAIL_DB) yy=m_step(yy,1); else
    if(m_step(yy,-1)>=0) yy=m_step(yy,-1); else yy=0;
    goto ujra;
  }
  
  /* (un)select messages */
  if(gomb=='-' || gomb=='+' || gomb=='*'){
    int i=-1;
    int mode=gomb;
    if(filter_selected==1 && mode=='*'){
      // special case! 'S' mode + * => unselect all & go to 'S+' mode
      while((i=m_step(i,1))<MAIL_DB){
	M_FLAGS(i)&=~MAILFLAG_SELECTED;
      }
      filter_selected=0;
      goto ujra;
    }
    box_input(10,60,"Search:",search_str_input); if(gomb==KEY_ESC) goto ujra;
    init_search(search_str_input);
    while((i=m_step(i,1))<MAIL_DB){
      int ret=(check_match(i)) ? MAILFLAG_SELECTED : 0;
      switch(mode){
      case '-': M_FLAGS(i)&=~ret; break;
      case '+': 
        if(filter_selected==1){
	  // show selected items only: do AND
	  M_FLAGS(i)&=(~MAILFLAG_SELECTED) | ret;
	} else {
	  // show nonselected items too: do OR
	  M_FLAGS(i)|=ret;
	}
        break;
      case '*': M_FLAGS(i)^=ret; break;
      }
    }
    goto ujra;
  }

  /* Delete selected */
  if(gomb=='d'){
    int i=-1;
    int n=0;
    char ize[200];
    while((i=m_step(i,1))<MAIL_DB) ++n;
    sprintf(ize,"Are you sure to (un)delete %d mails?",n);
    box_message(ize);waitkey();
    if(gomb=='Y'){
      int i=-1;
      while((i=m_step(i,1))<MAIL_DB){
	M_FLAGS(i)^=MAILFLAG_DEL;
	UPDATE_REK(i);
      }
    }
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
    addr_count=0;
    message_id[0]=0;
    compose();
    goto ujra;
  }
  
  /* REPLY & FORWARD */
  if(gomb=='r' || gomb=='f'){
    int reply=(gomb=='r');
    const char repre[]="Re: ";
    strcpy(_from,__from);
    strcpy(_to,reply? M_FROM(yy) : M_TO(yy));
    if(reply) {
      if(strncmp(repre,M_SUBJ(yy),sizeof(repre)-1))
       strcpy(_subject,repre);
      else
       _subject[0]='\0';
    } else {
      strcpy(_subject,"Fwd: ");
    }
    strcat(_subject,M_SUBJ(yy));
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
          save_part(folder,i,f2,reply?"> ":"",skip_header,linewrap);
        }
      }
      if(reply) write_signature(f2);
      fclose(f2);
    }
    if(reply && addr_count>1){
        // select reply-to or from address:
        int ret=draw_menu_addrlist(addr_count,MENUTYPE_ADDRLIST,0);
        if(ret>=0) strcpy(_to,addr_list_v[ret]);
        if(ret==-1) goto ujra; // ESC
    }
    if(compose()){
      if(reply) M_FLAGS(yy)|=MAILFLAG_REPLY;
    }
    M_FLAGS(yy)&=(~MAILFLAG_NEW);
    UPDATE_REK(yy);
    goto ujra;
  }
  
  /* F = TOGGLE From: MODE */
  if(gomb=='F'){ if(++from_mod==3)from_mod=0; goto ujra; }
  /* P = TOGGLE quoted-printable MODE */
  if(gomb=='P'){ default_mimeflags^=MIMEFLAG_PQ; goto ujra2; }
  /* H = TOGGLE skip-header MODE */
  if(gomb=='H'){ skip_header^=1; goto ujra2; }
  /* W = TOGGLE line wrapping MODE */
  if(gomb=='W'){ linewrap^=1; goto ujra2; }
  /* I = TOGGLE case-insensitive MODE */
  if(gomb=='I'){ case_insensitive^=1; goto ujra2; }

  /* ADD TO ADDRESS_BOOK */
  if(gomb=='a'){
    strcpy(sor,M_FROM(yy));
    box_input(10,70,"Save to address book?",sor);
    /* if(gomb!=KEY_ESC)addressbook_add(sor); */
    goto ujra;
  }

  /* s/n = SEARCH FIRST */
  if(gomb=='s' || gomb=='n'){
    int xxx=gomb;
    box_input(10,60,"Search:",search_str_input); if(gomb==KEY_ESC) goto ujra;
    clrscr();last_y0=-1;  /* goto ujra; helyett */
    gomb=xxx-32;
    init_search(search_str_input);
  }
  /* S = SEARCH BACKWARD NEXT */
  if(gomb=='S'){
    int i=yy;
    while((i=m_step(i,-1))>=0)
      if(check_match(i)){
        yy=i; goto ujra2;
      }
    goto ujra2;
  }  
  /* N = SEARCH FORWARD NEXT */
  if(gomb=='N'){
    int i=yy;
    while((i=m_step(i,1))<MAIL_DB)
      if(check_match(i)){
        yy=i; goto ujra2;
      }
    goto ujra2;
  }  

  /* ctrl+F = SELECT MAILS WITH SAME From: */
  if(gomb=='F'-64){
    int i=-1;
      strcpy(sor2,cim_ertelmezo(M_FROM(yy),1));
      while((i=m_step(i,1))<MAIL_DB){
        if( strcmp(sor2,cim_ertelmezo(M_FROM(i),1))==0 ) {
          //yy=i;
	  M_FLAGS(i)|=MAILFLAG_SELECTED;
        } else {
	  //if(filter_selected==1) 
	  M_FLAGS(i)&=~MAILFLAG_SELECTED;
	}
      }
    filter_selected=1;
    goto ujra2;
  }

  /* p = SEARCH FORWARD NEXT SAME From: */
  if(gomb=='p'){
    int i=yy;
      strcpy(sor2,cim_ertelmezo(M_FROM(yy),1));
      while((i=m_step(i,1))<MAIL_DB){
        if( strcmp(sor2,cim_ertelmezo(M_FROM(i),1))==0 ) {
          yy=i;
  	  goto ujra2;
        }
      }
    goto ujra2;
  }  

  /* P = SEARCH BACKWARD NEXT SAME From: */
  if(gomb=='P'-64){
    int i=yy;
      strcpy(sor2,cim_ertelmezo(M_FROM(yy),1));
      while((i=m_step(i,-1))>=0){
        if( strcmp(sor2,cim_ertelmezo(M_FROM(i),1))==0 ) {
          yy=i;
  	  goto ujra2;
        }
      }
    goto ujra2;
  }  

}while(gomb!=KEY_ESC && gomb!='R');

clrscr();
getch2_disable();
/***************** END ************************/

#if 1
if(gomb!='R'){
  int i;
  for(i=0;i<MAIL_DB;i++) if(M_FLAGS(i)&MAILFLAG_NEW){
    M_FLAGS(i)&=(~MAILFLAG_NEW);
    UPDATE_REK(i);
  }
}
#endif

//delete_mails();
//truncate(foldername_idx,sizeof(rek_st)*MAIL_DB);

if(gomb=='R') goto restart;

/* the end */
close_folder(folder);

unlink(temp_nev);
unlink(cim_temp_nev);
return 0;
}

/*******************************************************************************
				    E N D
*******************************************************************************/
