//    Copyright 2021, Robert L. Read and Halimat Farayola
//    This program is free software: you can redistribute it and/or modify
//    it under the terms of the GNU General Public License as published by
//    the Free Software Foundation, either version 3 of the License, or
//    (at your option) any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program.  If not, see <https://www.gnu.org/licenses/>.




// This is a quick customizable way to make an array of holes of any size in a cylindrical plate - specifically for use in a Buchner funnel.



// TODO:
// A) The module for the electronics needs to be designed so that it 
// can be fitted onto or mates with the heating chamber design in such 
// a way that the electronics can be inserted. - 2 hours
// B) We need to choose a Bill of Materials for standoffs and design the
// holes for that - 2 hours
// C) We need to choose the hinge and get the precise dimensions. - 2 hours
// D) We need to decide on latching hardware and make sure it is mountable. - 2 hours
// E) We need to design a platform to support the materials - 2 hours
// F) We need to add a pipette holder - 2 hours
// G) We need to design some sort of battery attachment underneath - 2 hours
// H) We need to decide the dimensions of the inner chamber carefull - 1 hour

// I think we can define a basic "interface" the electronics module which will
// be a standard physical mount. This will allow us to use a standard inteface
// with a variety of boxes. Possibly this can be done with pillow mounts.

// MEDIA DIMENSIONS
// Petrifilms and other media vary in size.
// https://www.3m.com/3M/en_US/company-us/all-3m-products/~/ECOLICT-3M-Petrifilm-E-coli-Coliform-Count-Plates/?N=5002385+3293785155&preselect=3293786499&rt=rud
// However, E. coli Petrifilms are precisely:
ecoli_pf_width_mm = 74;
ecoli_pf_length_mm = 100;
ecoli_pf_height_mm = 1; // This is not a precise measumrent, but close.
// Note that Petfilims are allows to be stacked no more than 20 high.
ecoli_stack_number = 20;
// I suggest that we base the interior dimentions on this.

heatingClothGap=5;
plateWidth=3;
cassetteHeight=heatingClothGap+2*plateWidth;// 
throatDiameter=60;
backWallHeight=20;

include <Perforated_plate.scad>

// Note this site:
// https://www.shoppopdisplays.com/CS004/clear-acrylic-box-with-hasp-lock-hinged-lid-custom-size.html
// sells custom boxes with hasps and hinges which could be acceptable for the
// out box if we bolt something on; the other parts we can 3D print

include <rcube.scad>

// These are for the inner heat chanber dimensions
// I'm going to add some padding so the chamber won't be to cramped
hc_margin_mm = 10;
hc_len = ecoli_pf_length_mm + hc_margin_mm;
hc_wid = ecoli_pf_width_mm + hc_margin_mm;
// NOTE : This currently does not take account of the tray and heatin pad height.
hc_hgt = ecoli_stack_number*ecoli_pf_height_mm+hc_margin_mm;

// These are the outer dimensions; they could be 
// computed from the hc dimensions
hc_ins_width = 50; // an empty insulation width

hc_olen = hc_len+hc_ins_width;
hc_owid = hc_wid+hc_ins_width;
hc_ohgt = hc_hgt+hc_ins_width;
inner_shell_width = 3;
outer_shell_width = 3;
outer_shell_radius = 3;

// hinge specifcation (assume two holes)
hinge_hole_distance = 10;
hinge_hole_radius = 1.5; // for the screw shaft, not the head!
hinge_hole_distance_from_pin = 5;
hinge_leaf_width = 5; // for "recessing"
hinge_leaf_length = hinge_hole_distance*3; // length along hinge axis
hinge_leaf_thickness = 2;
hinge_screw_length = 15;
hinge_screw_barrel_radius = hinge_hole_radius+3;


// flange specification

flange_thickness = 10;
flange_length = hinge_leaf_width*3;

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

// Here we create two open boxes; these should be printed as two distinct parts!
module  createHeatChamber(ilen,iwid,ihgt,olen,owid,ohgt,ish,osh,fl,ft) {
    // outer box, which has "lips" cut off for lid
    difference() {
        union() {
        open_box(olen,owid,ohgt,osh,outer_shell_radius,0);
        createHingeFlange(olen,owid,ohgt,fl,ft);
        }
        // here I need to cut out the edges
        translate([-osh,0,ohgt/2])
        cube(size=[olen+osh*2, owid-osh*2, osh*2],center=true);
    }

}
// This is printed separately!!!
module innerChamber(ilen,iwid,ihgt,ish) {
    color([1,0,0])
    open_box(ilen,iwid,ihgt,ish,0,0);
}

module hingeleaf(olen,owid,ohgt,fsh) {
        color([1,0.5,0.5])
        scale([2,2,2])
        rcube([hinge_leaf_width*2,hinge_leaf_length,hinge_leaf_thickness],1);
}
// LID -- This needs to have a place for the latch hardware
module lid(olen,owid,ohgt,osh,fsh) {
    difference() {
        union() {
        difference() {
            union() {
                translate([0,0,ohgt/2-osh/2])
                cube(size=[olen,owid-osh*2,osh],center=true);
            } 
            translate([olen/2+-hinge_hole_distance_from_pin,0,ohgt/2 - hinge_screw_length/2]) 
             { 
                translate([0,hinge_hole_distance/2,0])     
                cylinder(hinge_screw_length*2,hinge_screw_barrel_radius,hinge_screw_barrel_radius,true);
                translate([0,-hinge_hole_distance/2,0])
                cylinder(hinge_screw_length*2,hinge_screw_barrel_radius,hinge_screw_barrel_radius,true);
             }                   
    
        }
        translate([olen/2+-hinge_hole_distance_from_pin,0,ohgt/2 - hinge_screw_length/2]) 
        dualBarrels();
        }
        // now we cut away the hinge_leaf....
        // we first translate it an then rotate it....
        translate([olen/2,0,ohgt/2 - hinge_leaf_thickness/2]) 
        rotate([0,180,0])
        hingeleaf(olen,owid,ohgt,fsh);
    }
}

module screwBarrel(len,od,id) {
    difference() {
        cylinder(len,od,od,true, $fs = 0.01);
        cylinder(len*2,id,id,true,  $fs = 0.01);
    }
}
module dualBarrels() {
    // now we create two "barrels" to receive the screws (with holes in those!)
    color([0,0,1])
    translate([0,hinge_hole_distance/2,0])
    screwBarrel(hinge_screw_length,hinge_screw_barrel_radius,hinge_hole_radius);
    color([0,0,1])
    translate([0,-hinge_hole_distance/2,0])
    screwBarrel(hinge_screw_length,hinge_screw_barrel_radius,hinge_hole_radius);
}

// Now we create a "flange" which is a place to bolt one half of the hinge.
// fsh is the "flange thickness"
module createHingeFlange(olen,owid,ohgt,flen,fsh) {
    
    // We want to add two "barrels" for the screws, but first we use that 
    // tool to cut out the holes!!
    difference() {
        union() {
            difference() {
                    translate([olen/2+flen/2,0,ohgt/2 - fsh/2])
                    cube(size=[flen, owid, fsh],center=true);
                    translate([olen/2+hinge_hole_distance_from_pin,0,ohgt/2 - hinge_screw_length/2]) 
                // This cuts the screw holes
                    {    
                        translate([0,hinge_hole_distance/2,0])     
                        cylinder(hinge_screw_length*2,hinge_screw_barrel_radius,hinge_screw_barrel_radius,true);
                        translate([0,-hinge_hole_distance/2,0])
                        cylinder(hinge_screw_length*2,hinge_screw_barrel_radius,hinge_screw_barrel_radius,true);
                    }      
                }

        translate([olen/2+hinge_hole_distance_from_pin,0,ohgt/2 - hinge_screw_length/2]) 
        dualBarrels();
            }
               // now we cut away a "recession" for the hinge...
            // this is mostly to make it easier to see how pieces fit together!!!
            // this is mostly to make it easier to see how pieces fit together!!!
        translate([olen/2,0,ohgt/2 - hinge_leaf_thickness/2]) 
        hingeleaf(olen,owid,ohgt,flen,fsh);
    }

}


module  open_box(olen,owid,ohgt,sh, or=4,ir =2) {   
    // inside
    difference() {
        difference() {             
                rcube([olen, owid, ohgt],or, center=true); 
                rcube([olen-2*sh, owid-2*sh, ohgt-2*sh],ir,center=true); 
        }
        // This makes the box "open"
        translate([0,0,sh])
        cube(size=[olen-2*sh, owid-2*sh, sh + ohgt-2*sh],center=true); 
    }
// Now we will cut the top out, which makes this slightly assymetric...    
}

module  createCaseBottom(width,length,height,shellWidth,dftp) {
    inner_box_width = width - (2*shellWidth);
    inner_box_length = length - (2*shellWidth);
    
    // inside
    difference() {             
                cube(size=[length, width, height],center=true); 
                translate([0,0,shellWidth])
                cube(size=[inner_box_length, inner_box_width, height], center=true); 
    } 
    translate([length/2 - dftp,0,0])
    cube(size=[shellWidth,width,height], center=true);
        
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
        #translate([40,10,14]){
            cube(size=[length2hole,width2hole,heighthole],center=false);
        }   

}
}

//translate([250,100,0]){
//            cube(size=[length2hole,width2hole,heighthole],center=false);
//        }
        
//createCaseBottom(width,length,height,shellWidth,dividerFromTop);
//createHeatChamber(hc_len,hc_wid,hc_hgt,hc_olen,hc_owid,hc_ohgt,inner_shell_width,outer_shell_width,flange_length,flange_thickness);
//
//translate([-120,0,0]) 
//color([0,0,1,0.7])       
//lid(hc_olen,hc_owid,hc_ohgt,outer_shell_width);

//translate([150,0,0])
// createCaseTop();
      
// translate([150,0,0])
// innerChamber(hc_len,hc_wid,hc_hgt,inner_shell_width);

module cassettePlate() {
    difference() {
    cube(size=[throatDiameter,ecoli_pf_length_mm,plateWidth],center=true);
        arrayOfCylinders(5,14,10); // TODO: Make these constants
    }
}

module cassetteTop() {
    translate([0,0,cassetteHeight/2])
    union() {
        translate([0,ecoli_pf_length_mm/2-plateWidth/2,backWallHeight/2])
        cube(size=[throatDiameter/2,plateWidth,backWallHeight],center=true);
        translate([0,ecoli_pf_length_mm/2-backWallHeight/2,backWallHeight])
        cube(size=[throatDiameter/2,backWallHeight,plateWidth],center=true);
        cassettePlate();
    }
}
module cassetteBottom() {
    translate([0,0,-cassetteHeight/2])
    cassettePlate();
}
module cassetteMounts() {
    translate([throatDiameter/2-plateWidth/2,0,0])
    cube(size=[plateWidth,ecoli_pf_length_mm,cassetteHeight],center=true);
        translate(-[throatDiameter/2-plateWidth/2,0,0])
    cube(size=[plateWidth,ecoli_pf_length_mm,cassetteHeight],center=true);
}

module thermosCassette() {
    union () {
        cassetteTop();
        cassetteBottom();
        cassetteMounts();
    }
}

thermosCassette();