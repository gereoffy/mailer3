
#gcc -g -O3 -mcpu=i686 -march=i686 -Wall 5a.c libmail1f.c getch2.c -ltermcap -o a
#gcc -g -O3 -Wall 5a.c libmail1f.c getch2.c -o a -ltermcap # -liconv

#gcc -g -march=i586 -mtune=i686 -O3 -Wall -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE 5a.c libmail1f.c getch2.c -ltermcap -o a

#define USE_TERMCAP

#gcc -g3 -march=i586 -mtune=i686 -O -Wall -DUSE_TERMCAP -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE 5a.c libmail1f.c getch2.c -ltermcap -o a

gcc -g3 -O2 -Wall -DUSE_TERMCAP -D_LARGEFILE_SOURCE -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE 5a.c libmail1f.c getch2.c -lcurses -o a

