/*****************************************************************************/

#define MFS_CNM 0
#define MFS_101 1
#define MFS_PMM 2
#define MFS_INBOX 3
#define MFS_HIX 5
#define MFS_DIGEST 7

#define MAILFLAG_READ 1
#define MAILFLAG_REPLY 2
#define MAILFLAG_NEW 8
#define MAILFLAG_ATTACH 16
#define MAILFLAG_DEL 32
#define MAILFLAG_EXTRA 64
#define MAILFLAG_LIST 128

#define MAILFLAG_SELECTED 32768

#define MIMEFLAG_PQ 1
#define MIMEFLAG_ISO 2
#define MIMEFLAG_B64 4

typedef struct rek_tip {
        int from;
        int to;
        int subject;
        int date;
        int pos;
        int size;
        int msize;
        int flags;
} rek_st;

typedef struct {
  int mfs;                  /* "mail filesystem" */
  int mail_db;
  int folder_size;
  int strings_pos;
  int f_mails_size;
  rek_st *f_mails;
  char *f_strings;
  char *strings_end;
  int  *strings_hash;
  int  *strings_hash_full;
  FILE *file_folder;
  FILE *file_index;
  FILE *file_strings;
} folder_st;

/*****************************************************************************/
#define MAX_MIMEPARTS 32
#define MIME_MAXLEN 40

typedef struct {
  int start;
  int end;
  int flags;
  char name[MIME_MAXLEN];
  char encoding[MIME_MAXLEN];
} mime_st;

extern int mime_db;
extern mime_st mime_parts[MAX_MIMEPARTS];
extern int default_mimeflags;

/*****************************************************************************/
#define MAX_ADDR 8

extern int addr_count;
extern char addr_list_t[MAX_ADDR][10];
extern char addr_list_v[MAX_ADDR][128];

extern char message_id[];

/*****************************************************************************/

#define sormaxsize 4096

extern char sor[sormaxsize];
extern char sor2[sormaxsize];

extern int open_folder(folder_st* folder,char *folder_name,char *index_name,char *strings_name);
extern void full_hash_strings(folder_st* folder);
extern int update_folder(folder_st* folder);
extern void close_folder(folder_st *folder);

//extern int read_rek(folder_st *folder,int i,rek_st *mail);
extern int write_rek(folder_st *folder,int i,rek_st *mail);
extern int upgrade_rek(folder_st* folder,rek_st* mail);

extern void open_mail(folder_st *folder,rek_st *mail);
extern void save_part(folder_st *folder,int i,FILE *f2,char *replystr,int skip_header,int linewrap);
extern int save_attachment(folder_st *folder,int i,FILE *ff);
extern void save_mail_source(folder_st *folder,rek_st *mail, char *fnev);
extern void save_mail_source_file(folder_st *folder,rek_st *mail, FILE* f);

extern char* cim_ertelmezo(char *sor,int from_mod);
extern void kodlap_generalo();

/*****************************************************************************/
extern char* strncpy2(char* d,char* s,int n);
extern char* strncpy2n(char* d,char* s,int n);
extern char* strofs2(char* s,int x);
extern int filesize(char* fnev);
extern int strposc(char c,char* sor);
extern char* copy(char* hova,char* mibol,int p,int l);
extern char* upcstr(char* s2,char* s1);
extern int strpos(char* s1,char* s2);
extern void get_date(char* _date);
/*****************************************************************************/
