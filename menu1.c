/******************************************************************************/
/*                   MIME/Multipart MENU                                      */
/******************************************************************************/

#define MENUITEMS1(x) mime_parts[x].name
#define MENUITEMS2(x) mime_parts[x].encoding

/*  Ki:
  0..n  valasztott menupont
  -1    ESC
  -2    mas billentyu (gomb,skod)
*/
int draw_menu_mimeparts(int ys,int tipus,char keyflag){
  int i,x,y,xs,xs1,xs2,ys2;

  if((tipus!=menu_tipus)||(menu_yy>=ys)){menu_yy=0;menu_y0=0;}
  menu_tipus=tipus;
  xs1=10;xs2=0;
  if(ys==0){ MENUITEMS1(0)[0]=0;MENUITEMS2(0)[0]=0;++ys; }
  for(i=0;i<ys;i++){
    if(strlen(MENUITEMS1(i))>xs1)xs1=strlen(MENUITEMS1(i));
    if(strlen(MENUITEMS2(i))>xs2)xs2=strlen(MENUITEMS2(i));
  }
  if((ys2=ys)>(term_ys-2))ys2=(term_ys-2);
  xs=(++xs1)+xs2;
  x=(term_xs-4-xs)/2; y=(term_ys+3-ys2)/2;
  draw_box(x-1,y-1,xs+3,ys2+1);
  do{
    /* redraw} */
/*  gotoxy(1,1);printf("yy=%d  y0=%d  ys=%d  ys2=%d",menu_yy,menu_y0,ys,ys2);*/
    for(i=0;i<ys2;i++){
      if((menu_y0+i)==menu_yy)set_color(0);else set_color(7);
      gotoxy(x,y+i);
      printf(" %-*.*s%-*.*s ",xs1,xs1,MENUITEMS1(menu_y0+i),
                              xs2,xs2,MENUITEMS2(menu_y0+i));
    }
    gotoxy(x,y+menu_yy-menu_y0);
    refresh();
    waitkey();
    if((gomb==KEY_UP)&&menu_yy)--menu_yy;
    if((gomb==KEY_DOWN)&&(menu_yy<(ys-1)))++menu_yy;
    if(gomb==KEY_PGUP)menu_yy=0;
    if(gomb==KEY_PGDWN)menu_yy=ys-1;
    if(menu_yy>=(menu_y0+ys2-1)) menu_y0=menu_yy-ys2+1;
    if(menu_yy<menu_y0) menu_y0=menu_yy;
    if(gomb<256){ return(menu_yy); }
  }while(gomb!=KEY_ESC);
  return(-1);
}

#undef MENUITEMS2
#undef MENUITEMS1

