/******************************************************************************/
/*  **  Buffered TEXTFILE READLN routine v2.0   (C) 1998-99. by GyikSoft  **  */
/******************************************************************************/

char puffer[PUFFSIZE];
int puffer_pos=0;
int puffer_size=0;
int puffer_mut=0;
unsigned int sor_pos,eol_pos;
int eof_jel=0;
int eol_jel=0;
FILE *file_readln=NULL;   /* used for readln_sor(); function */

int puffer_update(){
  puffer_pos=ftell(file_readln);
  puffer_size=fread(&puffer,1,PUFFSIZE,file_readln);
  puffer_mut=0;
  if(puffer_size>0) return 1;
  eof_jel=1;return 0;
}

int readln_sor() {
int i=0;
register char c;
  sor_pos=puffer_pos+puffer_mut;
  if(sor_pos>=eol_pos){eol_jel=1;sor[0]=0;return(-1);}
  
  if(folder->mfs!=MFS_PMM && puffer_mut+sormaxsize<puffer_size){
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
        if(!puffer_update()){sor[i]=0;return(-2);}
    }
    c=puffer[puffer_mut++];
    if(c==26){eol_jel=1;return(-1);}
    if(c==10) break;
    if(c!=13) sor[i++]=c;
  }while(i<sormaxsize);
  sor[i]=0;
  return(0);
}

