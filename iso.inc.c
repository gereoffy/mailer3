/*
      ISO -> ASCII converter
      (C) 1997-98  original (pascal) by GyikSoft
      (C) 1998  converted to Borland C++ by PilaSoft
      (C) 1998  rewritten in ANSI C by GyikSoft
*/

int decoded_size;
unsigned char decoded[256];  
char kodlap_tabl[256];
char table_b64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
char table_uue[]="`!\"#$%&\'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_";
char table_xxe[]="+-0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

/*********************/
/** K¢dlap geneg l¢ **/
void kodlap_generalo(){
 int i;
 for (i=0;i<=255;i++) kodlap_tabl[i]=i;
  /*Windoz 3.1, iso-8859-1,-2*/
 /*  kisbetuk:                nagybetuk:       */
#if 0
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


int hexjegy(int a){
  if((a>='0')&&(a<='9'))return(a-'0');
  if((a>='A')&&(a<='F'))return(a-'A'+10);
  if((a>='a')&&(a<='f'))return(a-'a'+10);
  return(-1);
}

char* kodlap_dekodolo(char* sor2){
int i;
  for (i=0;i<strlen(sor2);i++) sor2[i]=kodlap_tabl[(unsigned char)sor2[i]];
  return(sor2);
}

long decode(char x){
  long i;
  for(i=0;i<=63;i++)
    if(x==table_b64[i]) return(i);
  return(0);
}


unsigned char* decode_b64(char* sor){
 long adat;
 int  i,j,k,l;

  if (sor[0]==0){decoded[(decoded_size=0)]=0;return(decoded);}
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


char* hexa2ascii(char* sor)
{
  int i=0,j=0;
  int l=strlen(sor);
  int c;

  while (i<=l){
    c=sor[i];
    if (c=='_') c=' ';
    if (c=='='){
      if((i<=l-2)&&(hexjegy(sor[i+1])!=-1)&&(hexjegy(sor[i+2])!=-1)){
      	c=hexjegy(sor[i+1])*16+hexjegy(sor[i+2]);
	i+=2;
      }
    }
    if(c==10 || c==13) c=' ';
    sor2[j++]=c;
    i++;
  }
  sor2[j]=0;
/*  printf("pq='%s'\n",sor2); */
  return(sor2);
}

char* iso(char* sor){
char sor1[256],sor3[256],kodlap[256],temp[256];
  char isoflag;
  int  i,j,l;
  char  c,iso_kodolas;
  strcpy(sor1,sor); strcpy(temp,sor);
  iso_kodolas='Q';
  /*-- ISO-kodok ertelmezese --*/
  i=0; temp[0]=0; /*i=1*/
  isoflag=0;
  strcpy(kodlap,"?");
  while (i<=strlen(sor1))
  {
    c=sor1[i++];
    if ((isoflag) && (c=='?') && (sor1[i]=='='))
      {
	if (iso_kodolas=='B') 
	  {strcat(temp,(char*)decode_b64(sor3));}
	else 
	  {strcat(temp,hexa2ascii(sor3));}
	isoflag=0;i++;
	goto ok;
      }
    if ((!isoflag) && (c=='=') && (sor1[i]=='?'))
    {
      isoflag=1;
      sor3[0]=0;
      j=i+1;
      while ((j<=strlen(sor1)) && (sor1[j]!='?'))j++;
      strncpy(kodlap,sor1+i+5,j-i-5);
      i=j+3;
      iso_kodolas=sor1[j+1];
      goto ok;
    }
    if (isoflag)
      {l=strlen(sor3);sor3[l]=c;sor3[l+1]=0;}
    else 
      {l=strlen(temp);temp[l]=c;temp[l+1]=0;}
  ok:;
  }
  if (isoflag){
      if(iso_kodolas=='B')
        strcat(temp,(char*)decode_b64(sor3));
      else
	strcat(temp,hexa2ascii(sor3));
  }
  i=0;while(temp[i]==' ')++i;if(i)strcpy(temp,temp+i);  /* nyir_elol */
/*return(kodlap_dekodolo(temp));*/
strcpy(sor,temp);
return(sor);
}


