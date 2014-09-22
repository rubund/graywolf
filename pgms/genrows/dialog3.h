static TWDIALOGBOX macro_dialogS[29] = {
    1,5,8," ACCEPT ",2,6,0,
    1,25,8," CANCEL ",2,3,0,
    2,15,9,"Macro Edit",0,3,0,
    4,5,11,"Coordinates",0,4,0,
    5,5,11,"-----------",0,2,0,
    6,5,9,"X       :",0,4,0,
    6,25,8," ",1,4,0,
    7,5,9,"Y       :",0,4,0,
    7,25,8," ",1,4,0,
    8,5,9,"Delta X :",0,4,0,
    8,25,8," ",1,4,0,
    9,5,9,"Delta Y :",0,4,0,
    9,25,8," ",1,4,0,
    10,5,7,"Orient:",3,4,14,
#ifndef TI
    10,25,1,"0",2,4,1,
    10,26,1,"1",2,4,1,
    10,27,1,"2",2,4,1,
    10,28,1,"3",2,4,1,
    10,29,1,"4",2,4,1,
    10,30,1,"5",2,4,1,
    10,31,1,"6",2,4,1,
    10,32,1,"7",2,4,1,
#else /* the TI case */
    10,25,1,"1",2,4,1,
    10,26,1,"2",2,4,1,
    10,27,1,"3",2,4,1,
    10,28,1,"4",2,4,1,
    10,29,1,"5",2,4,1,
    10,30,1,"6",2,4,1,
    10,31,1,"7",2,4,1,
    10,32,1,"8",2,4,1,
#endif /* TI */
    13,1,35,"To change case fields put pointer",0,4,0,
    14,1,35,"in window and click.  To change",0,4,0,
    15,1,35,"input fields put pointer in window",0,4,0,
    16,1,35,"and back up over current contents",0,4,0,
    17,1,35,"using the delete key.  After modifying",0,4,0,
    18,1,35,"the field, hit the return key.",0,4,0,
    0,0,0,0,0,0,0
} ;

#ifdef TI
/* timberwolf to ice view translations */
INT tw2ice(orient)
INT orient;
{
   switch (orient){
      case 0:
         return(1);
      case 1:
         return(7);
      case 2:
         return(5);
      case 3:
         return(3);
      case 4:
         return(8);
      case 5:
         return(6);
      case 6:
         return(2);
      case 7:
         return(4);
   }

}


INT ice2tw(orient)
INT orient;
{
   switch (orient){
      case 1:
         return(0);
      case 2:
         return(6);
      case 3:
         return(3);
      case 4:
         return(7);
      case 5:
         return(2);
      case 6:
         return(5);
      case 7:
         return(1);
      case 8:
         return(4);
   }
} /* end ice2tw */

#endif /* TI */
