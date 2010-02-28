/*  GyikSoft Mailer for UNIX v3.x  Compile-time Configuration File
    (C) 1999 A'rpi/ESP-team
*/

/* >>> default for From: field <<< */
char __from[]="Arpi <arpi@thot.banki.hu>";

/* >>> Editor/Viewer program name: <<< */
#define EDITOR_CMD "joe -asis "

/* >>> Select 'mail' for SGI-IRIX, 'sendmail' for AIX and LINUX: <<< */
/* #define SENDMAIL "mail" */
#define SENDMAIL "/usr/sbin/sendmail"

/* >>> Filenames: <<< */
#define ADDRESS_BOOK_FILE "address_book.txt"
#define SIGNATURE_FILE "signature.txt"

#define COPYSELF "Copyself"

char temp_nev[]="tempfile1.tmp";
char cim_temp_nev[]="tempfile2.tmp";

/* Menu system limits: max lines and line lengths */
#define max_menuitems 64
#define menuitem1_max 400
#define menuitem2_max 400

/* Time between new-mail checks ( seconds ) */
#define HALFDELAY_TIME 2




