/******************************************************************************/
/* **  GyikSoft Mailer LIBRARY v3.0 * (C) 1997-99. by GyikSoft & PiluSoft  ** */
/******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>

#include "libmail.h"

#define PUFFSIZE 0x4000
#define cim_temp_nev ".tmp.libMail~"

#define STRINGS_MALLOC 0x4000
#define STRINGS_HASH 1024

char sor[sormaxsize];
char sor2[sormaxsize];

int default_mimeflags=0;

char addr_list_t[MAX_ADDR][10];
char addr_list_v[MAX_ADDR][128];
int addr_count=0;
char message_id[256];

#include "string.inc.c"
#include "iso.inc.c"
#include "readln.inc.c"

/******************************************************************************/

int read_rek(folder_st *folder,int i,rek_st *mail){
  if(fseek(folder->file_index,i*sizeof(rek_st),SEEK_SET)) return 0;
  if(fread(mail,sizeof(rek_st),1,folder->file_index)!=1) return 0;
  return 1;
}

int write_rek(folder_st *folder,int i,rek_st *mail){
  if(fseek(folder->file_index,i*sizeof(rek_st),SEEK_SET)) return 0;
  if(fwrite(mail,sizeof(rek_st),1,folder->file_index)!=1) return 0;
  return 1;
}

static int re_malloc(char **ptr,int oldsize,int newsize){
  void *newp;
  newsize&=(~15);
  printf("Reallocating STRINGS memory  %d -> %d\n",oldsize,newsize);
  newp=realloc(*ptr,newsize);
  *ptr=newp;
  return newsize;
}

//  folder_seek(folder,mail->pos);
static int folder_seek(folder_st* folder,int i){
  if(fseek(folder->file_folder,i,SEEK_SET)) return -1;
  file_readln=folder->file_folder;
  eof_jel=0; puffer_update();
  return 0;
}

/******************************************************************************/

static char *strings;
static char *strings_end;
static int  *strings_hash;
static int strings_pos;

static int write_strings(folder_st* folder,char *str){
  int tmp=strings_pos;
  int i;
  int len=strlen(str)+1;
  int hash=0;
#ifdef STRINGS_MALLOC
    /* Try to optimize */
  if(strings){
#ifdef STRINGS_HASH
    if(strings_hash){
      for(i=0;i<len;i++){ hash=(hash<<3)^(hash>>29); hash^=str[i]; }
      hash&=STRINGS_HASH-1;
      if((i=strings_hash[hash])) 
        if(strcmp(&strings[i],str)==0)
          return i; /* Megvan!!! */
    }    
#endif
{   char *p=strings;
    char *e=&strings[strings_pos];
#if 1
    while(p<e){
      char *q=p;
      while(*p++){};
      if(len==(p-q)) if(*q==*str) if(strcmp(q,str)==0){
        if(strings_hash) strings_hash[hash]=q-strings;
        return (q-strings); /* Megvan!!! */
      }
    }
#endif
    if((e+len)>=strings_end){
      int oldsize=strings_end-strings;
      int newsize=re_malloc(&strings,oldsize,oldsize+STRINGS_MALLOC);
      strings_end=&strings[newsize];
    }
}
    if(strings) memcpy(&strings[strings_pos],str,len);
  }
#endif
/*  if(folder->mail_db<5) printf("ftell=%d  strings_pos=%d\n",ftell(folder->file_strings),strings_pos); */
  i=fwrite(str,1,len,folder->file_strings);/* fflush(folder->file_strings); */
  if(i!=len) return 0; /* !!! */
  strings_pos+=len;
  if(strings_hash) strings_hash[hash]=tmp;
  return tmp;
}

int open_folder(folder_st* folder,char *folder_name,char *index_name,char *strings_name){
  rek_st mail;
  if(!folder) return 1;
  if(!folder_name) return 1;
//  if(folder->file_folder){ printf("\n\nfile_folder leak!  \n");abort();}
  if(!(folder->file_folder=fopen(folder_name,"rb"))) return 1;/* no such folder*/

//  if(folder->file_index){ printf("\n\nfile_index leak!  \n");abort();}
  if(!(folder->file_index=fopen(index_name,"ab+"))) return 2;/* create index*/
  fclose(folder->file_index);
  if(!(folder->file_index=fopen(index_name,"rb+"))) return 3;/* write to index*/
  fseek(folder->file_index,0,SEEK_END);
  folder->mail_db=ftell(folder->file_index);
  printf("Index size = %d\n",folder->mail_db);
  if(folder->mail_db % sizeof(rek_st)) return 4; /* invalid filesize */
  folder->mail_db/=sizeof(rek_st);
  printf("mail_db=%d\n",folder->mail_db);
  if(!folder->mail_db)
    sor_pos=(folder->mfs==MFS_PMM) ? 128 : 0;
  else {
    if(!read_rek(folder,folder->mail_db-1,&mail)) return 5;
    sor_pos=mail.pos+mail.size;
  }
  if(folder_seek(folder,sor_pos)){ printf("Cannot seek folder to %d\n",sor_pos); return 6;}

//  if(folder->file_strings){ printf("\n\nfile_strings leak!  \n");abort();}
  if(!(folder->file_strings=fopen(strings_name,"ab+"))) return 3;
  fclose(folder->file_strings);
  if(!(folder->file_strings=fopen(strings_name,"rb+"))) return 3;
  fseek(folder->file_strings,0,SEEK_END);
  strings=NULL;
  if((strings_pos=ftell(folder->file_strings))==0){
    printf("Creating new STRINGS file\n");
    write_strings(folder,"");
    printf("ftell=%ld  strings_pos=%d\n",ftell(folder->file_strings),strings_pos);
  }

  /* Read folder, build index */
//  sor_pos=puffer_pos+puffer_mut;

if(!eof_jel){ /* have new mail */

#ifdef STRINGS_MALLOC
  strings=malloc((strings_pos+STRINGS_MALLOC)&(~15));
  if(strings){
    strings_end=&strings[(strings_pos+STRINGS_MALLOC)&(~15)];
    rewind(folder->file_strings);
    printf("after rewind:  ftell=%ld  strings_pos=%d\n",ftell(folder->file_strings),strings_pos);
    fread(strings,1,strings_pos,folder->file_strings);
    fseek(folder->file_strings,strings_pos,SEEK_SET);
    /*fflush(folder->file_strings);*/
    printf("after fread:  ftell=%ld  strings_pos=%d\n",ftell(folder->file_strings),strings_pos);
  }
#ifdef STRINGS_HASH
  strings_hash=malloc(sizeof(int)*STRINGS_HASH);
  if(strings_hash) memset(strings_hash,0,sizeof(int)*STRINGS_HASH);
#endif
#endif

  mail.date=0;
  do{
    // index new mail:
    mail.pos=sor_pos;
    mail.from=mail.to=mail.subject=0;
    mail.flags=MAILFLAG_NEW;
    eol_jel=0;eol_pos=-1;
    // parse header:
    do{
      readln_sor2(folder->mfs,1); if(eof_jel) break;
      if(strncmp(sor,"From:",5)==0)mail.from=write_strings(folder,iso(sor+5));
      if(strncmp(sor,"To:",3)==0)mail.to=write_strings(folder,iso(sor+3));
      if(strncmp(sor,"Subject:",8)==0)mail.subject=write_strings(folder,iso(sor+8));
    }while(!eol_jel && sor[0]);
    mail.msize=puffer_pos+puffer_mut;
    // parse body:
    do{
      if(eof_jel || eol_jel) break;
      readln_sor2(folder->mfs,0);
    }while(folder->mfs!=MFS_INBOX || strncmp(sor,"From ",5) );
    mail.msize=sor_pos-mail.msize;
    // store index (only if size!=0)
    if((mail.size=sor_pos-mail.pos)){
      fflush(folder->file_index);
      if(1!=fwrite(&mail,sizeof(rek_st),1,folder->file_index)){
        printf("error writting to index file!\n");
        perror("hiba");
      }
      ++folder->mail_db;
    }
    if(eol_jel) ++sor_pos; // skip mail separator char (PMM folders)
  }while(!eof_jel);

#ifdef STRINGS_MALLOC
  if(strings){ free(strings);strings=NULL; }
#ifdef STRINGS_HASH
  if(strings_hash){ free(strings_hash);strings_hash=NULL;}
#endif
#endif
}

  printf("mail_db=%d (after reading new)\n",folder->mail_db);

  folder->strings_size=strings_pos;
  folder->folder_size=sor_pos;
  folder->f_mails=NULL;
  folder->f_strings=NULL;

/* Allocate memory and load index+strings files */

  folder->f_mails=malloc(sizeof(rek_st) * folder->mail_db);
  folder->f_strings=malloc(folder->strings_size);
  if(!folder->f_strings || !folder->f_mails) return 7;

  fflush(folder->file_index);
  fflush(folder->file_strings);

  rewind(folder->file_index);
  if(folder->mail_db!=fread(folder->f_mails,sizeof(rek_st),folder->mail_db,folder->file_index))
    { printf("load_folder: index read error");return 8;}
  rewind(folder->file_strings);
  if(folder->strings_size!=fread(folder->f_strings,1,folder->strings_size,folder->file_strings))
    { printf("load_folder: strings read error");return 9;}

  return 0;
}

void close_folder(folder_st *folder){
  if(folder->f_strings) {free(folder->f_strings);folder->f_strings=NULL;}
  if(folder->f_mails)   {free(folder->f_mails);  folder->f_mails=NULL;}
  fclose(folder->file_strings);folder->file_strings=NULL;
  fclose(folder->file_index);folder->file_index=NULL;
  fclose(folder->file_folder);folder->file_folder=NULL;
}

/*******************************************************************************
      View_MAIL v3.0   (C) 1997-98 by GyikSoft, C version by GyikSoft 1998-99.
*******************************************************************************/

#define bound_maxdb 10
#define bound_maxsize 80
static char boundary[bound_maxdb][bound_maxsize];
static int boundary_db;

int mime_db;
mime_st mime_parts[MAX_MIMEPARTS];

void open_mail(folder_st *folder,rek_st *mail){
int i,j;
char usor[sormaxsize];
char not_empty,header_ok;
char field[256];   /* pl: "CONTENT_ENCODING:" */
char *data;

  folder_seek(folder,mail->pos);
  eol_jel=0; eol_pos=mail->pos+mail->size;

  boundary_db=0; boundary[0][0]=0;
  mime_db=0;
  mime_parts[0].start=puffer_pos+puffer_mut;
  mime_parts[0].flags=default_mimeflags;
  strcpy(mime_parts[0].name,"header");
  strcpy(mime_parts[0].encoding,"text");

  header_ok=1; field[0]=0;
  not_empty=0;
  addr_count=0;
  message_id[0]=0;

  do{
    readln_sor2(folder->mfs,header_ok);
    if(header_ok){
    /*--------------- We're in the Mail HEADER ------------*/
  
      if(sor[0]==0){ header_ok=0; continue; }   /* End of the header */
      
      upcstr(usor,sor);
      
      data=sor+1;
      if(usor[0]!=' ' && usor[0]!=9){
        char *p=strchr(usor,':');
	    if(!p){
              field[0]=0;
              data=sor;
            } else {
	      int l=p-usor;
              if(l>255) l=255;
	      strncpy2n(field,usor,l);
              data=sor+l+1;
	    }
      }
      
      if(strcmp(field,"FROM")==0 ||
         strcmp(field,"REPLY-TO")==0 ||
         strcmp(field,"SENDER")==0 ||
         strcmp(field,"X-BEENTHERE")==0 ||
         strcmp(field,"LIST-ID")==0 ||
         strcmp(field,"CC")==0 ||
         strcmp(field,"X-SENDER")==0) if(addr_count<MAX_ADDR){
         strcpy(addr_list_t[addr_count],field);
         strcpy(addr_list_v[addr_count],iso(data));
         ++addr_count;
      }

      if(strcmp(field,"MESSAGE-ID")==0){
          strcpy(message_id,data);
      }

      if(strcmp(field,"CONTENT-TYPE")==0){
        // MIME boundary?
        if((i=strpos("BOUNDARY",usor))>=0){
  	      i+=9;i+=strposc(34,sor+i);
	      j=strposc(34,sor+i);
          if(j==0)j=255;
          if(j>1){
            strcpy(boundary[boundary_db],"--");
	        copy(&boundary[boundary_db][2],sor,i,j-1);
            boundary_db++;
          }
	      continue;
        }
	// Encoding?
	if(strstr(usor,"ISO-8859")) mime_parts[mime_db].flags|=MIMEFLAG_ISO;
	// Filename?
        if((i=strpos("NAME=",usor))>=0){
          char *p=strchr(&sor[i+6],34);
          if(p) *(p+1)=0; else if((p=strchr(&sor[i+5],';'))) *p=0;
          strncpy2(mime_parts[mime_db].name,&sor[i+5],MIME_MAXLEN);
          continue;
	}
	// MIME Type?
        if(strncmp(usor,"CONTENT-TYPE:",13)==0){
          char *p=strchr(&sor[13],';');
          if(p) *p=0;
          strncpy2(mime_parts[mime_db].name,nyir(&sor[13]),MIME_MAXLEN);
        }
	continue;
      }

      if(strcmp(field,"CONTENT-DISPOSITION")==0){
	// Filename?
        if((i=strpos("FILENAME=",usor))>=0){
          char *p=strchr(&sor[i+10],34);
          if(p) *(p+1)=0; else if((p=strchr(&sor[i+9],';'))) *p=0;
          strncpy2(mime_parts[mime_db].name,&sor[i+9],MIME_MAXLEN);
          continue;
	}
      }
      
      if(strcmp(field,"CONTENT-TRANSFER-ENCODING")==0){
	if(strstr(usor,"QUOTED-PRINTABLE")) mime_parts[mime_db].flags|=MIMEFLAG_PQ;
	if(strstr(usor,"BASE64")) mime_parts[mime_db].flags|=MIMEFLAG_B64;
        if(strncmp(field,usor,25)==0)
          strncpy2(mime_parts[mime_db].encoding,nyir(&sor[26]),MIME_MAXLEN);
      }
    
    } else {
      /*--------------- We're in the Mail BODY -----------*/

      /* Searching for MIME boundaries... (multipart separator strings) */
      i=-1;
      for(j=0;j<boundary_db;j++){
        if((i=strpos(boundary[j],sor))>=0) break;
      }
      if(i!=-1){
        mime_parts[mime_db].end=sor_pos+i;
	    if(not_empty){++mime_db;}
        /* Begin new part: */
	    mime_parts[mime_db].start=sor_pos+i;
	    mime_parts[mime_db].flags=default_mimeflags;
	    mime_parts[mime_db].name[0]=0;
	    mime_parts[mime_db].encoding[0]=0;
	    not_empty=0;
	    header_ok=1;
      }else{
        if(sor[0]!=0) not_empty=1;
      }     
    }
    
  }while(!eol_jel);
  mime_parts[mime_db].end=puffer_pos+puffer_mut;
  if(not_empty) ++mime_db;

#if 0
if(mime_db>2){
  printf("---------------------------------------------------\n");
  printf("%-25s|%8d|%-40s\n",
      cim_ertelmezo(&folder->f_strings[mail->from],2),
      mail->msize, &folder->f_strings[mail->subject]);
  printf("Mime-db: %d   Boundary_db: %d\n",mime_db,boundary_db);
  for(i=0;i<mime_db;i++){
    printf("%1d|%10d%8d  %-28.28s  %-28.28s\n",
      mime_parts[i].flags,
      mime_parts[i].start,
      mime_parts[i].end-mime_parts[i].start,
      mime_parts[i].name,
      mime_parts[i].encoding);
  }
}
#endif

}

/************************** SAVE_PART *************************************/

static void wrap_print(FILE *f2,char *replystr,char* sor,int linewrap){
  unsigned char* p=sor;
  char* q;
  int maxlen,wraplen;
// filter
// TODO: UTF8 decoder
       while(*p){
           if((*p!=9 && *p<32) || (*p>=127 && *p<224) || *p>254) *p='.';
           ++p;
       }
// wrap
  do{
    maxlen=strlen(sor);
    q=strchr(sor,10); if(q) maxlen=q-sor;
    q=strchr(sor,13); if(q) if(q-sor<maxlen) maxlen=q-sor;
    if(linewrap){
	// IntelliWrap (C) :)
	wraplen=maxlen;
	if(sor[0]=='>' || (sor[0]==' ' && sor[1]=='>')){
	    if(maxlen>100) wraplen=74;
	} else {
	    if(maxlen>79) wraplen=74;
	}
	if(wraplen<maxlen){
	    maxlen=wraplen;
	    while(maxlen>0 && maxlen+12>=wraplen){
		int c=sor[maxlen];
		if(c==32 || c==9 || c==',') break;
		--maxlen;
	    }
	}
    }

    fprintf(f2,"%s%.*s\n",replystr,maxlen,sor);
    sor+=maxlen;
    while(sor[0]==10 || sor[0]==13 || sor[0]==' ' || sor[0]==9) ++sor;
  } while(sor[0]);

}

/*  Writes 'i'th part to file 'f2', using reply prefix 'replystr' */
void save_part(folder_st *folder,int i,FILE *f2,char *replystr,int skip_header, int linewrap){
  int b64_jel=(mime_parts[i].flags & MIMEFLAG_B64);
  int header_ok=1;
  FILE *ft2;
  char sor3[sormaxsize];
//  unsigned char* p;
  
  if(i<0 || i>=mime_db) return;
  
  folder_seek(folder,mime_parts[i].start);
  eol_jel=0;eol_pos=mime_parts[i].end;
  sor3[0]=0;
  do{
    readln_sor2(folder->mfs,header_ok);
    if(header_ok){
      if(sor[0]==0) header_ok=0;
      if(!skip_header) fprintf(f2,"%s%s\n",replystr,sor);
      continue;
    }

    if(b64_jel){
	 /* Decoding Base64 to binary TEMP file, and including after as text */
       if((ft2=fopen(cim_temp_nev,"w+"))){
	     do{
	       decode_b64(sor);
	       fwrite(decoded,1,decoded_size,ft2);
	       readln_sor2(folder->mfs,0);
	     }while(!eol_jel && sor[0]);
	     rewind(ft2);
	     while(!feof(ft2)){
	       fgets(sor,sormaxsize-16,ft2);sor[strlen(sor)-1]=0;
               wrap_print(f2,replystr,sor,linewrap);
	     }
	     fclose(ft2);
         unlink(cim_temp_nev);
       }
       b64_jel=0;
    }

    if(mime_parts[i].flags&MIMEFLAG_PQ){
      if(sor[strlen(sor)-1]=='='){
        char *p;
        sor[strlen(sor)-1]=0;
        strcat(sor3,hexa2ascii(sor,0));
	p=strchr(sor3,10);
	if(!p) p=strchr(sor3,13);
//	printf("\n****** sor3=%d p=%p  *******\n",strlen(sor3),p);
//	printf("sor3='%s'\n",sor3);
	if(p){
	    // line has pq coded CR/LF - split it!
	    p[0]=0;++p;
	    wrap_print(f2,replystr,sor3,linewrap);
	    memmove(sor3,p,strlen(p)+1);
	}
      } else {
        strcat(sor3,hexa2ascii(sor,0));
        wrap_print(f2,replystr,sor3,linewrap);
        sor3[0]=0;
      }
    } else {
        wrap_print(f2,replystr,sor,linewrap);
    }

  }while(!eol_jel);
  if(sor3[0]) wrap_print(f2,replystr,sor3,linewrap);
}


/*  Save Base64-encoded attachment to file 'ff' */
int save_attachment(folder_st *folder,int i,FILE *ff){
int total=0;
  if(i<0 || i>=mime_db) return 0;
  
  folder_seek(folder,mime_parts[i].start);
  eol_jel=0;eol_pos=mime_parts[i].end;
  /* Skipping Mail HEADER */
  do{
    readln_sor2(folder->mfs,0);
    if(sor[0]==0) break;
  }while(!eol_jel);
  /* Decoding Base64 to binary file */
  do{
	readln_sor2(folder->mfs,0);
	decode_b64(sor);
	total+=fwrite(decoded,1,decoded_size,ff);
  }while(!eol_jel && sor[0]);
  return total;
}

void save_mail_source(folder_st *folder,rek_st *mail, char *fnev){
FILE *f=fopen(fnev,"wb");
void *p=malloc(mail->size);
  if(!f || !p) return;
  fseek(folder->file_folder,mail->pos,SEEK_SET);
  fread(p,1,mail->size,folder->file_folder);
  fwrite(p,1,mail->size,f);
  fclose(f);
  free(p);
}


/*******************************************************************************
                		--==>    M A I N   <==--
*******************************************************************************/

#if 0

void fatal(int n,char *s){
  fprintf(stderr,"mail: FATAL ERROR - %s\n",s);
  exit(n);
}

int main(int argc,char *argv[]){
char *foldername;

if(argc>1) foldername=argv[1]; else foldername=getenv("MAIL");
if(open_folder(folder,foldername,"mail.idx","mail.str")) fatal(1,"Cannot open folder/index");
if(load_folder(folder)) fatal(2,"Cannot load index");

printf("mail_db=%d  size=%d\n",folder->mail_db,folder->folder_size);

#if 0
read_rek(folder,40,&mail);
save_mail_source("bugzik.mail",&mail);
#endif

#if 1
{ int i;
  for(i=0;i<folder->mail_db;i++) open_mail(folder,&folder->f_mails[i]);
}
#endif

/* the end */
free_folder(folder);
close_folder(folder);
return 0;
}

#endif
/*******************************************************************************
				    E N D
*******************************************************************************/

