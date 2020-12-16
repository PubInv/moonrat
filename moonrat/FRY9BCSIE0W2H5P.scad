

length = 86;
width= 46;
height = 19;
length2=30;
lengtho2=44;
width2=40;
lengthtwo=80;

height2=16;


height20 = 19;

height22=16;
lengthhole=25;
length2hole=40;
widthhole=15;
width2hole= 30;
heighthole=3;
dhole=5;
depthhole=3;

difference() {
    difference() {
                
                translate([0, 0, 0]){
                    cube(size=[length, width, height],center=false); 
                }
                translate([3,3,3]) {
                    cube(size=[length2, width2, height2], center=false); 
                }
        
    }
    //translate([83,22.5,3]){
       // cylinder(depthhole,dhole/2,dhole/2,center=false);
    //}
    translate([39,3,3]) {
                    cube(size=[lengtho2, width2, height2], center=false); 
                }
}
difference(){
    difference(){
        difference() {
                    
                    translate([150, 0, 0]){
                        cube(size=[length, width, height20],center=false); 
                    }
                    translate([153,3,0]) {
                        cube(size=[lengthtwo, width2, height22], center=false); 
                    }
            
        }
        translate([157,10,16]){
            cube(size=[lengthhole,widthhole,heighthole],center=false);
        }
         translate([157,30,16]){
            cylinder(depthhole,dhole/2,dhole/2,center=false);
        }
        translate([167,30,16]){
            cylinder(depthhole,dhole/2,dhole/2,center=false);
        }
        translate([177,30,16]){
            cylinder(depthhole,dhole/2,dhole/2,center=false);
        }
        translate([190,10,16]){
            cube(size=[length2hole,width2hole,heighthole],center=false);
        }
    
   
    }
    translate([190,10,15]){
            cube(size=[length2hole,width2hole,heighthole],center=false);
        }
}

translate([250,0,0]){
            cube(size=[length2hole,width2hole,heighthole],center=false);
        }