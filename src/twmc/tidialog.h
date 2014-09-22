static TWDIALOGBOX dialogS[53] = {
    1,5,8," ACCEPT ",2,3,0,
    1,25,8," CANCEL ",2,2,0,
    2,15,9,"Edit Cell",0,3,0,
    3,5,13,"Name",0,4,0,
    4,5,12,"Number",0,4,0,
    5,5,8,"Xcenter:",0,4,0,
    5,14,8,"10",1,4,0,
    6,5,8,"Ycenter:",0,4,0,
    6,14,8,"10",1,4,0,
    7,5,7,"Orient:",3,4,10,
    7,14,2,"1 ",2,4,1,
    7,16,2,"2 ",2,4,1,
    7,18,2,"3 ",2,4,1,
    7,20,2,"4 ",2,4,1,
    7,22,2,"5 ",2,4,1,
    7,24,2,"6 ",2,4,1,
    7,26,2,"7 ",2,4,1,
    7,28,2,"8 ",2,4,1,
    9,5,19,"Valid orientations",0,4,0,
    10,7,1,"1",3,4,20,
    10,14,5,"valid",2,6,2,
    10,25,7,"invalid",2,3,2,
    11,7,1,"2",3,4,23,
    11,14,5,"valid",2,6,3,
    11,25,7,"invalid",2,3,3,
    12,7,1,"3",3,4,26,
    12,14,5,"valid",2,6,4,
    12,25,7,"invalid",2,3,4,
    13,7,1,"4",3,4,29,
    13,14,5,"valid",2,6,5,
    13,25,7,"invalid",2,3,5,
    14,7,1,"5",3,4,32,
    14,14,5,"valid",2,6,6,
    14,25,7,"invalid",2,3,6,
    15,7,1,"6",3,4,35,
    15,14,5,"valid",2,6,7,
    15,25,7,"invalid",2,3,7,
    16,7,1,"7",3,4,38,
    16,14,5,"valid",2,6,8,
    16,25,7,"invalid",2,3,8,
    17,7,1,"8",3,4,41,
    17,14,5,"valid",2,6,9,
    17,25,7,"invalid",2,3,9,
    19,5,5,"Fixed",3,4,44,
    19,14,3,"on ",2,6,10,
    19,25,3,"off",2,3,10,
    21,1,35,"To change case fields put pointer",0,4,0,
    22,1,35,"in window and click.  To change",0,4,0,
    23,1,35,"input fields put pointer in window",0,4,0,
    24,1,35,"and back up over current contents",0,4,0,
    25,1,35,"using the delete key.  After modifying",0,4,0,
    26,1,35,"the field, hit the return key.",0,4,0,
    0,0,0,0,0,0,0
} ;


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


