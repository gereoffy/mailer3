/*
      ISO -> ASCII converter
      (C) 1997-98  original (pascal) by GyikSoft
      (C) 1998  translated to Borland C++ by PilaSoft
      (C) 1998  rewritten in ANSI C by GyikSoft
      (C) 1999  clean-up & optimized by GyikSoft
*/

/* #define CODEPAGES */

int decoded_size;
unsigned char decoded[256];  
#ifdef CODEPAGES
char kodlap_tabl[256];
#endif  
char table_b64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char table_uue[]="`!\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";
char table_xxe[]="+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

/******************************************************************************/
/*     ****  Cim-ertelmezo v2.0   (C) 1997-98. by GyikSoft & PiluSoft ***     */
/******************************************************************************/

#define nyir(a) nyir2((a),(a))

char* nyir2(char *dest,char *s){
char *d=dest;
int c=32;
int spc;
  do{
    spc=c;
    c=*s++;
    if(c==9) c=32;
    if(spc==32 && c==32) continue;
    *d++=c;
  }while(c);
  --d;
  if(spc==32 && d>dest){ --d; *d=0;}
  while(d>dest){
    if(*dest==34 && d[-1]==34){ ++dest;--d;*d=0;continue; }
    if(*dest=='(' && d[-1]==')'){ ++dest;--d;*d=0;continue; }
    break;
  }
  return dest;
}

static char from_nev[sormaxsize];
static char from_email[sormaxsize];

/* ertelmezi a "From:" mezot, es visszater a 'from_mod' szerint:
  mod=0  -> cim+nev
  mod=1  -> csak E-Mail cim
  mod=2  -> csak nev                                      */  

char* cim_ertelmezo(char *sor_in,int from_mod)
{ char *sor1;
  char *i, *j;
  char tmpsor[sormaxsize];

  if(from_mod==0) return(sor_in);
/* From: "Andrasi Aron" <AA@sparta.banki.hu>  */
/* From: jeffg@nbnet.nb.ca (Jeff Gilchrist)   */
  sor1=nyir2(tmpsor,sor_in);
  i=strchr(sor1,'<');
  j=strchr(sor1,'>');
  if(i && j && j>i){
    strncpy(from_email,i+1,j-i-1); from_email[j-i-1]=0;
    i[0]=0; strcat(sor1,j+1);  /* delete substring[i..j] */
  } else {
    i=strchr(sor1,' ');   /* i=0 ha nem talalta meg a stringet! */
    if(i && strchr(sor1,'@')){
      strncpy(from_email,sor1,i-sor1); from_email[i-sor1]=0;
      strcpy(sor1,i+1);
    } else strcpy(from_email,sor1);
  }

  if (from_email[0]==0) strcpy(from_email,sor1);
  strcpy(from_nev,sor1);
  if (from_nev[0]==0) strcpy(from_nev,from_email);
#if 0
  if(from_mod==2) return(from_nev);
  return(from_email);
#else
  if(from_mod==2) return(nyir(from_nev));
  return(nyir(from_email));
#endif
}

/******************************************************************************/

/******************************/
/** Codepage table generator **/
void kodlap_generalo(){
#ifdef CODEPAGES
 int i;
 for (i=0;i<=255;i++) kodlap_tabl[i]=i;
  /*Windoz 3.1, iso-8859-1,-2*/
 /*  kisbetuk:                nagybetuk:       */
 kodlap_tabl[0xE1]=' ';  kodlap_tabl[0xC1]='';
 kodlap_tabl[0xE9]='‚';  kodlap_tabl[0xC9]='';
 kodlap_tabl[0xED]='¡';  kodlap_tabl[0xCD]='Œ';
 kodlap_tabl[0xF3]='¢';  kodlap_tabl[0xD3]='•';
 kodlap_tabl[0xF6]='”';  kodlap_tabl[0xD6]='™';
 kodlap_tabl[0xF4]='“';  kodlap_tabl[0xD4]='§';
 kodlap_tabl[0xFA]='£';  kodlap_tabl[0xDA]='—';
 kodlap_tabl[0xFC]='';  kodlap_tabl[0xDC]='š';
 kodlap_tabl[0xFB]='–';  kodlap_tabl[0xDB]='˜';
#endif
 return;
}

#ifdef CODEPAGES
char* kodlap_dekodolo(char* sor2){
int i;
  for (i=0;i<strlen(sor2);i++) sor2[i]=kodlap_tabl[(unsigned char)sor2[i]];
  return(sor2);
}
#else
#define kodlap_dekodolo(x) (x)
#endif

/*****************************************************************************/

static int hexjegy(int a){
  if((a>='0')&&(a<='9'))return(a-'0');
  if((a>='A')&&(a<='F'))return(a-'A'+10);
  if((a>='a')&&(a<='f'))return(a-'a'+10);
  return(-1);
}

static long decode(char x){
  long i;
  for(i=0;i<=63;i++)
    if(x==table_b64[i]) return(i);
  return(0);
}


unsigned char* decode_b64(char* sor){
 long adat;
 int  i,j,k,l;

  if(sor[0]==0){decoded[(decoded_size=0)]=0;return(decoded);}
  k=0;
  l=(strlen(sor)-1) / 4;
  strcat(sor,"==");
  for (i=0;i<=l;i++){ 
     adat=0;
     for (j=0;j<4;j++) adat=adat*64+decode(sor[i*4+j]);
     decoded[k+2]=(adat & 0xff); adat>>=8;
     decoded[k+1]=(adat & 0xff); adat>>=8;
     decoded[k+0]=(adat & 0xff);
     k+=3;
  }
  k=(strchr(sor,'=')-&sor[0])*3/4;
  decoded[k]=0;decoded_size=k;

/*  printf("Base64 in:  '%s'\n",sor);
  printf("Base64 out: '%s'\n",decoded); */

return(decoded);
}


char* hexa2ascii(char* sor,int ulflag)
{
  int i=0,j=0;
  int l=strlen(sor);
  int c;

  while((c=sor[i++])){
    if(c=='_' && ulflag) c=' ';
    if(c=='=' && i<=l-1){
      int a=hexjegy(sor[i]);
      int b=hexjegy(sor[i+1]);
      if(a!=-1 && b!=-1){ c=(a<<4)+b; i+=2;}
    }
    if(c==10 || c==13) c=' ';
    sor2[j++]=c;
  }
  sor2[j]=0;
/*  printf("pq='%s'\n",sor2); */
  return(sor2);
}


char* iso(char* sor){
char sor1[4*256],sor3[4*256],kodlap[4*256],temp[4*256];
char isoflag=0;
char iso_kodolas='Q';
int  i;
char c;
char *t=temp;
char *s=0;

  /*-- ISO-kodok ertelmezese --*/
  i=0;
//  strcpy(sor1,sor);
  strncpy(sor1,sor,255); sor1[255]=0;
  strcpy(kodlap,"?");
  while (i<strlen(sor1)){
    c=sor1[i++];
    if((isoflag) && (c=='?') && (sor1[i]=='=')){
      *s=0;
      if(iso_kodolas=='B') s=(char*)decode_b64(sor3); else s=hexa2ascii(sor3,1);
      while((c=*s++)) *t++=c;
	  isoflag=0;
      i++; continue;
    }
    if((!isoflag) && (c=='=') && (sor1[i]=='?')){
      int j=i+1;
      while ((j<=strlen(sor1)) && (sor1[j]!='?'))j++;
      printf("len(sor1)=%d i=%d j=%d\n",strlen(sor1),i,j);
      //len(sor1)=76 i=2 j=3
      if(j-i-5>0) strncpy(kodlap,sor1+i+5,j-i-5);
      isoflag=1; s=sor3;
      iso_kodolas=sor1[j+1];
      i=j+3; continue;
    }
    if(isoflag) *s++=c; else *t++=c;
  }
  if(isoflag){
    *s=0;
    if(iso_kodolas=='B') s=(char*)decode_b64(sor3); else s=hexa2ascii(sor3,1);
    while((c=*s++)) *t++=c;
  }
  while(t>temp && t[-1]==32) --t;  /* nyir_hatul */
  *t=0;
  t=temp; while(*t==' ') ++t;      /* nyir_elol */

#ifdef CODEPAGES
return kodlap_dekodolo(temp);
#else
return strcpy(sor,t);
#endif
}


