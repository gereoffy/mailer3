/*  GyikSoft Mailer for UNIX v2.0  Compile-time Configuration File
    (C) 1999 A'rpi/ESP-team
*/

/* >>> Select 'mail' for SGI-IRIX, 'sendmail' for AIX and LINUX: <<< */
/* #define SENDMAIL "mail" */
#define SENDMAIL "/usr/sbin/sendmail"

/* >>> default for From: field <<< */
char __from[]="Arpi <arpi@banki1.banki.hu>";

/* >>> Editor/Viewer program name: <<< */
#define EDITOR_CMD "joe -asis "

/* >>> maximum line length, longer lines are splitted <<< */
#define LINEWRAP 90

/* >>> Filenames: <<< */
#define ADDRESS_BOOK_FILE "address_book.txt"
#define SIGNATURE_FILE "signature.txt"

char temp_nev[]="tempfile1.tmp";
char cim_temp_nev[]="tempfile2.tmp";

/* >>> Maximum number of mails (sorry, no dynamic allocation yet...) <<< */
#define maxlevel 4000

/* >>> Buffer size for folder reader: <<< */
#define puffsize 65536

/* >>> Maximum length of a line (not eq. line-wrap!) <<< */
#define sormaxsize 8192

