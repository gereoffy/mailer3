/******************************************************************************/

/* ua. mint az strncpy, de mindenkepp lesz lezaro #0 a string vegen. */
char* strncpy2(char* d,char* s,int n){
  strncpy(d,s,n-1);
  d[n-1]=0;
  return d;
}

/* ua. mint az strncpy, de mindenkepp lesz lezaro #0 a string vegen. */
char* strncpy2n(char* d,char* s,int n){
  strncpy(d,s,n);
  d[n]=0;
  return d;
}

/* visszater az 's' string 'x'-edik karakterenek poziciojaval.
   majdnem ugyanazaz mint az s[x] de ha x>strlen(s) akkor is mukodik. */
char* strofs2(char* s,int x){
  if(x>strlen(s))return("");
  return(s+x);
}

/* visszater az 'fnev' file meretevel */
off_t filesize(char* fnev){
  struct stat stb;
  stat(fnev,&stb);
  return(stb.st_size);
}

/* 'sor'-ban keresi 'c' karaktert, es visszater a poziciojaval,
   ugyanugy mint a PASCAL-os pos().  0->nincs benne,  1->elso karakter... */
int strposc(char c,char* sor){
  char* p=strchr(sor,c);
  if(!p) return 0;
  return (p-sor+1);
}

/* kb. az mint a PASCAL-ban a copy() */
char* copy(char* hova,char* mibol,int p,int l){
  if(p>strlen(mibol)){
    hova[0]=0;
  }else{
    strncpy(hova,mibol+p,l);
    hova[l]=0;
  }
  return(hova);
}

/* string nagybetusitese. */
char* upcstr(char* s2,char* s1){
  int c,i=0;
  do{
    c=s1[i];
    if((c>='a')&&(c<='z'))c-=32;
    s2[i++]=c;
  } while(c);
  return s2;
}

/******************************************************************************/

int strpos(char* s1,char* s2){
char *p=strstr(s2,s1);
  if(!p) return -1;
  return (p-s2);
}

/******************************************************************************/

#include <time.h>

void get_date(char* _date){
 time_t timer;
 struct tm *tblock;
   timer = time(NULL);           /* gets time of day */
   tblock = localtime(&timer);   /* converts date/time to a structure */
   strcpy(_date,asctime(tblock));
   while(strlen(_date) && _date[strlen(_date)-1]==10) _date[strlen(_date)-1]=0;
   return;
}

/*******************************************************************************
				    E N D
*******************************************************************************/

