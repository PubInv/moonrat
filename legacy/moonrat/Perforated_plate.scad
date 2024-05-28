// From jpearce at thingiverse:
// https://www.thingiverse.com/thing:105723/files
// Licensed under Creative Commons Attribution Share Alike
// Modifcations Copyright Robert L. Read, licensed under
// CC SA

//Defines the diameter of filter paper for your funnel
d_paper = 90;

//Defines the thickness of the perforated plate
t_plate=2;


//Defines the area of the  array
a=100;

//Defines the radius of the holes
r=1; //size

//Defines the spacing of the holes
hole_radius = 3;
// s should be at least twice the hole_radius!
s=10; //space

t=t_plate*3; //thickness or depth of the holes



module arrayOfCylinders(r=1,s=6,t=2) {
  q = floor(a/2/s);
    for (x=[-q:q])
      for (y=[-q:q])
        translate([x*s,y*s,0])
          cylinder(h=t, r=r, ,center=true);
}

r=(d_paper)/2;
t=4;  
difference(){
 cylinder(h=t_plate, r=(d_paper)/2, center=true);
 arrayOfCylinders(hole_radius,s,t);
 }
