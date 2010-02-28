/******************************************************************************/
/*  **  Buffered TEXTFILE READLN routine v2.0   (C) 1998-99. by GyikSoft  **  */
/******************************************************************************/

static char puffer[PUFFSIZE];
static off_t puffer_pos=0;
static int puffer_size=0;
static int puffer_mut=0;
static off_t sor_pos,eol_pos;
static int eof_jel=0;
static int eol_jel=0;
static FILE *file_readln=NULL;   /* used for readln_sor(); function */

static int puffer_update(){
  puffer_pos=ftello(file_readln);
  puffer_size=fread(&puffer,1,PUFFSIZE,file_readln);
  puffer_mut=0;
  if(puffer_size>0) return 1;
  eof_jel=1;return 0;
}

static char sor_next=0;

static int readln_sor2(int mfs, int header) {
int i=0;
register char c;
  sor_pos=puffer_pos+puffer_mut;
  if(sor_pos>=eol_pos){eol_jel=1;sor[0]=0;return(-1);}

again:
  sor_next=0;
  if(1 && mfs!=MFS_PMM && puffer_mut+(sormaxsize-i)+2<puffer_size){
    /* Optimized version! */
    int i0=i;
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
    }while(i<sormaxsize-18);
    puffer_mut+=i-i0+1;
    if(i && sor[i-1]==13) --i;
    sor[i]=0;
  } else {
  
  do{
    if(puffer_size<=puffer_mut){
        if(!puffer_update()){sor[i]=0;return(-2);}
    }
    c=puffer[puffer_mut++];
    if(c==26 && mfs==MFS_PMM){eol_jel=1;return(-1);}
    if(c==10) break;
    if(c!=13) sor[i++]=c;
  }while(i<sormaxsize);
  sor[i]=0;
  if(puffer_size<=puffer_mut)
    if(!puffer_update()){ // no more data...
      eof_jel=0; // clear eof flag, as we're not at EOF yet
      return 0;
    }

  }
  sor_next=puffer[puffer_mut];
  if(header && (sor_next==9 || sor_next==32) && i){ // multi-line header
//    printf("[%d] sor_next=%d '%c' sor='%s'\n",i,sor_next,(sor_next<32) ? ' ' : sor_next,sor);
    goto again;
  }
//  if(header) printf("sor(%d)='%s'\n",i,sor);
  return(0);
}

static int readln_sor2_cont(int mfs) {
int i=strlen(sor);
while(1){
  if(sor_next!=9 && sor_next!=32) return 0;
  sor_next=0;
  do{
    register char c;
    if(puffer_size<=puffer_mut){
        if(!puffer_update()){sor[i]=0;return(-2);}
    }
    c=puffer[puffer_mut++];
    if(c==26 && mfs==MFS_PMM){eol_jel=1;return(-1);}
    if(c==10) break;
    if(c!=13) sor[i++]=c;
  }while(i<sormaxsize);
  sor[i]=0;
  if(puffer_size<=puffer_mut)
    if(!puffer_update()){ // no more data...
      eof_jel=0; // clear eof flag, as we're not at EOF yet
      return 0;
    }
  sor_next=puffer[puffer_mut];
} // while(1)
}

