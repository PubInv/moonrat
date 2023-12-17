
//case bottom dimensions
arduino_width=80;
arduino_length=60;
case_height=30;

slope_angle=10;

box_thickness=3;
port_width=30;
port_length=15;
port_height=60;
cap_diameter=30;
mount_pos=(cap_diameter/2)-3;

//case top dimensions
top_height=10;

top_length= arduino_length/cos(slope_angle);


lengthOLED=25;

widthOLED=15;
depthholes=top_height+10;

dbuttons=5;
buttonspace=15;

dOLED_to_butt=-18;

length2hole=40;

height22=16;
width2hole= 30;
heighthole=8;
//screws
dscrews=2;



//Creates screw mounts
module screw_mount(mount_height){
    difference(){
        cube(size=[dscrews+2,dscrews+2,mount_height], center=true);
        cylinder(mount_height+1,dscrews/2,dscrews/2,center=true);
    }
}

//creates cube to cut the box bottom at an angle
module rotated_cube(angle){
    rotate([angle, 0, 0]){
        cube(size=[arduino_width+10,arduino_length+50,case_height+1], center=true);
    }
}

//create bottom
module  createCaseBottom() {
    difference(){
    union(){
        difference(){
            cube(size=[arduino_width,arduino_length,case_height], center=true);
        
            
       
                cube(size=[arduino_width-(2*box_thickness),arduino_length-(2*box_thickness),case_height-(2*box_thickness)], center=true);
        
                cube(size=[port_width,port_length,port_height],center=true);
        
                //mounting screws
                translate([0,mount_pos,0]){
                    cylinder(port_height,dscrews/2,dscrews/2,center=true);
        
                    }
                translate([0,-mount_pos,0]){
                    cylinder(port_height,dscrews/2,dscrews/2,center=true);
        
                }
            }
            //screw mounts
            translate([((arduino_width/2)-(dscrews+2)/2)-box_thickness,((arduino_length/2)-(dscrews+2)/2)-box_thickness,0]){
            screw_mount(case_height);
            }
            translate([-(((arduino_width/2)-(dscrews+2)/2)-box_thickness),((arduino_length/2)-(dscrews+2)/2)-box_thickness,0]){
            screw_mount(case_height);
            }
            translate([-(((arduino_width/2)-(dscrews+2)/2)-box_thickness),-(((arduino_length/2)-(dscrews+2)/2)-box_thickness),0]){
            screw_mount(case_height);
            }
            translate([(((arduino_width/2)-(dscrews+2)/2)-box_thickness),-(((arduino_length/2)-(dscrews+2)/2)-box_thickness),0]){
            screw_mount(case_height);
            }
        }
        translate([0,0,20])
             rotated_cube(slope_angle);
    }
        
}


        
//creates hole for the screw       
module screw_hole(d_of_screw){
     cylinder(depthholes,(d_of_screw)/2,(d_of_screw)/2,center=true);
        
     translate([0,0,2]){
           cylinder(1.5,(d_of_screw+2)/2,(d_of_screw+2)/2,center=true);
     }
     //Todo, Add screw thread
}


module Case_top(){
    difference(){
        cube(size=[arduino_width,top_length,box_thickness+2], center=true);
        
        translate([0,0,0])
                cube(size=[lengthOLED,widthOLED,depthholes], center=true);
     
    //buttons
        translate([0,dOLED_to_butt,0]){
          cylinder(depthholes,dbuttons/2,dbuttons/2,center=true);
        }
        translate([buttonspace,dOLED_to_butt,0]){
          cylinder(depthholes,dbuttons/2,dbuttons/2,center=true);
        }
        
        translate([-buttonspace,dOLED_to_butt,0]){
          cylinder(depthholes,dbuttons/2,dbuttons/2,center=true);
        }
     //OLED screws
         translate([(lengthOLED/2)+2,(widthOLED/2)+2,0]){
          screw_hole(dscrews);
        
        }
        translate([-((lengthOLED/2)+2),(widthOLED/2)+2,0]){
          screw_hole(dscrews);
        
        }
        translate([-((lengthOLED/2)+2),-((widthOLED/2)+2),0]){
          screw_hole(dscrews);
        
        }
        translate([((lengthOLED/2)+2),-((widthOLED/2)+2),0]){
          screw_hole(dscrews);
        
        }
         translate([((arduino_width/2)-(dscrews+2)/2)-box_thickness,((arduino_length/2)-(dscrews+2)/2)-box_thickness,0]){
          screw_hole(dscrews);
        }
       
         translate([-(((arduino_width/2)-(dscrews+2)/2)-box_thickness),((arduino_length/2)-(dscrews+2)/2)-box_thickness,0]){
          screw_hole(dscrews);
        }
        
         translate([-(((arduino_width/2)-(dscrews+2)/2)-box_thickness),-(((arduino_length/2)-(dscrews+2)/2)-box_thickness),0]){
          screw_hole(dscrews);
        }
        
         translate([((arduino_width/2)-(dscrews+2)/2)-box_thickness,-(((arduino_length/2)-(dscrews+2)/2)-box_thickness),0]){
          screw_hole(dscrews);
        }
        
    }
        

}



translate([150,0,0])
//rotate([slope_angle, 0, 0]){
 Case_top();
//}
translate([0,0,0])
 createCaseBottom();
