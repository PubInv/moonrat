

// Comment these
length = 86;//outer length of box
width= 46;//outer width of box

// outside box dimension
height = 19;//height of box
length2=30;
lengtho2=44;
width2=40;//inner width of box
lengthtwo=80;//inner length of box

// height of cutting tool
height2=18;

//height of lid
height20 = 19;
//divider between heating chamber and circuit
dividerFromTop = 55;
//height of cutting tool for lid
height22=16; 
lengthhole=25;//length of OLED hole
length2hole=40;//length of heating chamber door
widthhole=15;//width of OLED hole
width2hole= 30;//width of heating chamber door
heighthole=8;//depth of oled and chamber door holes
dhole=5;//diameters of button holes
depthhole=8;//depth of button hole

// Create basic shell width
shellWidth = 3;


module  createCaseBottom(width,length,height,shellWidth,dftp) {
    inner_box_width = width - (2*shellWidth);
    inner_box_length = length - (2*shellWidth);
    
    // inside
    difference() {             
                cube(size=[length, width, height],center=true); 
                translate([0,0,shellWidth])
                cube(size=[inner_box_length, inner_box_width, height], center=true); 
    } 
    translate([length/2 - dftp,0,12])
    cube(size=[shellWidth,width,height+height22], center=true);
        
}
module createCaseTop() {
difference(){
        difference() {
                    
                    translate([0, 0, 0]){
                        cube(size=[length, width, height20],center=false); 
                    }
                    translate([3,3,-1]) {
                        cube(size=[lengthtwo, width2, height22], center=false); 
                    }
            
        }
        translate([7,10,14]){
            cube(size=[lengthhole,widthhole,heighthole],center=false);
        }
         translate([7,30,15]){
            cylinder(depthhole,dhole/2,dhole/2,center=false);
        }
        translate([17,30,15]){
            cylinder(depthhole,dhole/2,dhole/2,center=false);
        }
        translate([27,30,15]){
            cylinder(depthhole,dhole/2,dhole/2,center=false);
        }
        translate([40,10,14]){
            cube(size=[length2hole,width2hole,heighthole],center=false);
        }   

}
}

translate([250,100,0]){
            cube(size=[length2hole,width2hole,heighthole],center=false);
        }
        
createCaseBottom(width,length,height,shellWidth,dividerFromTop);

translate([150,0,0])
createCaseTop();