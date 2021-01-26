

arduino_width=80;
arduino_length=60;
case_height=30;
box_thickness=3;
port_width=30;
port_length=15;
port_height=60;
top_height=10;


// Comment these
length = 86;
width= 46;

// outside box dimension
height = 19;
length2=30;
lengtho2=44;
width2=40;
lengthtwo=80;

// Possibly height of cutting tool
height2=18;


height20 = 19;


height22=16;
lengthhole=25;
length2hole=40;
widthhole=15;
width2hole= 30;
heighthole=8;
dhole=5;
depthhole=8;

// Create basic shell width
shellWidth = 3;
dividerFromTop = 30;

slope_angle=10;

module rotated_cube(angle){
    rotate([angle, 0, 0]){
        cube(size=[arduino_width+10,arduino_length+50,case_height+1], center=true);
    }
}


module  createCaseBottom() {
    difference(){
        cube(size=[arduino_width,arduino_length,case_height], center=true);
        
       translate([0,0,20])
        rotated_cube(slope_angle);
       
        cube(size=[arduino_width-(2*box_thickness),arduino_length-(2*box_thickness),case_height-(2*box_thickness)], center=true);
        
        cube(size=[port_width,port_length,port_height],center=true);
        
    }
        
}
module createCaseTop() {
        difference(){
            cube(size=[arduino_width,arduino_length,top_height], center=true);
            
            translate([0,0,box_thickness])
                cube(size=[arduino_width-(2*box_thickness),arduino_length-(2*box_thickness),top_height], center=true);
        }
        
       
}
module Case_top(){
    rotate([180, 0, 0]){
    createCaseTop();
    }
     //translate([7,30,15]){
          //  cylinder(depthhole,dhole/2,dhole/2,center=false);
        
       // translate([17,30,15]){
        //    cylinder(depthhole,dhole/2,dhole/2,center=false);
        
        //translate([27,30,15]){
         //   cylinder(depthhole,dhole/2,dhole/2,center=false);
        
       // #translate([40,10,14]){
          //  cube(size=[length2hole,width2hole,heighthole],center=false);
        

}

translate([150,0,0])
 Case_top();
translate([0,0,0])
 createCaseBottom();
