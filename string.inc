/******************************************************************************/

/* ua. mint az strncpy, de mindenkepp lesz lezaro #0 a string vegen. */
char* strncpy2(char* d,char* s,int n){
  strncpy(d,s,n-1);
  d[n-1]=0;
  return d;
}

/* visszater az 's' string 'x'-edik karakterenek poziciojaval.
   majdnem ugyanazaz mint az s[x] de ha x>strlen(s) akkor is mukodik. */
char* strofs2(char* s,int x){
  if(x>strlen(s))return("");
  return(s+x);
}

/* visszater az 'fnev' file meretevel */
int filesize(char* fnev){
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

void set_date(char* _date){

   time_t timer;
   struct tm *tblock;

   /* gets time of day */
   timer = time(NULL);

   /* converts date/time to a structure */
   tblock = localtime(&timer);

   strcpy(_date,asctime(tblock));

   return;

}

/******************************************************************************/
/*  **  Buffered TEXTFILE READLN routine v2.0   (C) 1998-99. by GyikSoft  **  */
/******************************************************************************/

void puffer_update() {
  puffer_pos=ftell(f1);
  puffer_size=fread(&puffer,1,puffsize,f1);
  puffer_mut=0;
  if(puffer_size<=0)eof_jel=1;
}

int readln_sor() {
int i=0;
register char c;
  sor_pos=puffer_pos+puffer_mut;
  if(sor_pos>=eol_pos){eol_jel=1;sor[0]=0;return(-1);}
  
  if(mfs!=MFS_PMM && puffer_mut+sormaxsize<puffer_size){
    /* Optimized version! */
    register char *p=&puffer[puffer_mut];
    do{
      c=p[0]; if(c==10) break; sor[i++]=c;
      c=p[1]; if(c==10) break; sor[i++]=c;
      c=p[2]; if(c==10) break; sor[i++]=c;
      c=p[3]; if(c==10) break; sor[i++]=c;
      c=p[4]; if(c==10) break; sor[i++]=c;
      c=p[5]; if(c==10) break; sor[i++]=c;
      c=p[6]; if(c==10) break; sor[i++]=c;
      c=p[7]; if(c==10) break; sor[i++]=c;
#if 1
      p+=8;
#else
      c=p[8];  if(c==10) break; sor[i++]=c;
      c=p[9];  if(c==10) break; sor[i++]=c;
      c=p[10]; if(c==10) break; sor[i++]=c;
      c=p[11]; if(c==10) break; sor[i++]=c;
      c=p[12]; if(c==10) break; sor[i++]=c;
      c=p[13]; if(c==10) break; sor[i++]=c;
      c=p[14]; if(c==10) break; sor[i++]=c;
      c=p[15]; if(c==10) break; sor[i++]=c;
      p+=16;
#endif      
    }while(i<sormaxsize);
    puffer_mut+=i+1;
    if(i && sor[i-1]==13) --i;
    sor[i]=0;
    return 0;
  }
  
  do{
    if(puffer_size<=puffer_mut){
        puffer_update();
        if(eof_jel){sor[i]=0;eol_jel=1;return(-2);}
    }
    c=puffer[puffer_mut++];
    if(c==26){eol_jel=1;return(-1);}
    if(c==10) break;
    if(c!=13) sor[i++]=c;
  }while(i<sormaxsize);
  sor[i]=0;
  return(0);
}

/******************************************************************************/
/*     ****  Cim-ertelmezo v2.0   (C) 1997-98. by GyikSoft & PiluSoft ***     */
/******************************************************************************/

char from_nev[255];
char from_email[255];

char * nyir(char * from_nev)
{ char * i=NULL;
  char temp[256];
  short int hossz;
  
/* return(from_nev); */

  hossz=strlen(from_nev);
  if (hossz>254)return(from_nev);
  strcpy(&temp[0], from_nev);
  temp[254]=0;
    while ((i=strchr(&temp[0],'"'))!=NULL)
    {
      strncpy(temp,temp,i-&temp[0]);i[0]=0;
      strcat(&temp[0],(i+1));strcat(&temp[0],"\0");
    }
/*
  while ((i=strstr(temp,"\'"))!=NULL)
    {  strncpy(temp,temp,i-&temp[0]);i[0]=0;
       strcat(temp,i+1);strcat(temp,"\n");
    }
*/
  while ((from_nev[0]!=0) && (from_nev[1]==' ')) strcpy(from_nev,from_nev+1);
  while ((i=strstr(temp,"  "))!=NULL)
    {  strncpy(temp,temp,i-&temp[0]);i[0]=0;
       strcat(temp,i+1);strcat(temp,"\0");
    }
  if ((temp[0]!=0) && (temp[0]=='('))
  { i=strchr(temp,')');
    if (i)
     {  strncpy(temp,temp,i-&temp[0]);i[0]=0;
        strcat(temp,i+1);strcat(temp,"\0");
     }
    strcpy(temp,temp+1);
  }
  strcpy(from_nev,temp);
  return(from_nev);
}


/* ertelmezi a "From:" mezot, es visszater a 'mod' szerint:
  mod=0  -> cim+nev
  mod=1  -> csak E-Mail cim
  mod=2  -> csak nev                                      */  

char* cim_ertelmezo(char * sor,char from_mod)
{ char sor1[255]; /*,sor2[255],sor3[255];*/
  char * i, * j ;

  if(from_mod==0)return(sor);
/* From: "Andrasi Aron" <AA@sparta.banki.hu>  */
/* From: jeffg@nbnet.nb.ca (Jeff Gilchrist)   */
  strcpy(sor1,nyir(sor));
  i=strchr(sor1,'<');
  j=strchr(sor1,'>');
  if ((j>i) && (i!=NULL) && (j!=NULL))
   {
    strncpy(from_email,i+1,j-i-1); /* from_email:=copy(sor1,i+1,j-i-1);*/
    from_email[j-i-1]=0;i[0]=0;/*   strncpy(sor1,sor1,(i-&sor1[0]));*/
    strcat(sor1,j+1);strcat(sor1,"\0"); /* delete(sor1,i,j-i+1);*/
   }
  else {
    i=strchr(sor1,' ');   /* i=0 ha nem tal lta meg a stringet! */
/*    writeln('"',sor1,'"  ',i);*/
    if ((strchr(sor1,'@')!=NULL) && (i!=NULL))
     {
      strncpy(from_email,&sor1[0],i-&sor1[0]);/*from_email:=copy(sor1,1,i-1);*/
      from_email[i-&sor1[0]]=0;
      strcpy(sor1,i+1);/*delete(sor1,1,i);*/
     }
    else strcpy(from_email,sor1);
  }
/*printf("step2:");*/

  if (from_email[0]==0) strcpy(from_email,sor1);
  strcpy(from_nev,sor1);
  if (from_nev[0]==0) strcpy(from_nev,from_email);

/* printf("from_nev:'%s' from_email:'%s'\n",from_nev,from_email); */

  if (from_mod==2) return(nyir(from_nev));
  return(nyir(from_email));

}

/*******************************************************************************
				    E N D
*******************************************************************************/

