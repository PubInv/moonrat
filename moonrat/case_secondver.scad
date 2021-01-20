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
ecoli_stack_thickness=ecoli_pf_height_mm*ecoli_stack_number;

heatingClothGap=5;
plateWidth=3;
cassetteHeight=heatingClothGap+2*plateWidth;// 
throatDiameter=60;
backWallHeight=20;
perfdepth=5;
perfdiameter=10;
perfspacing=14;

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









module cassettePlate() {
    difference() {
    cube(size=[throatDiameter,ecoli_pf_length_mm,plateWidth],center=true);
        arrayOfCylinders(perfdepth,perfspacing,perfdiameter); 
    }
}

module cassetteTop() {
    translate([0,0,cassetteHeight/2])
    union() {//shouldn't the cassette width be a little smaller than throat diameter
        translate([0,(ecoli_pf_length_mm-1)/2-plateWidth/2,backWallHeight/2])
        cube(size=[throatDiameter/2,plateWidth,ecoli_stack_thickness],center=true);
        translate([0,ecoli_pf_length_mm/2-backWallHeight/2,ecoli_stack_thickness])
        cube(size=[throatDiameter/2,backWallHeight,plateWidth],center=true);
        cassettePlate();
        translate([0,-(ecoli_pf_length_mm-1)/2+plateWidth/2,backWallHeight/2])
        cube(size=[throatDiameter/2,plateWidth,ecoli_stack_thickness],center=true);
        translate([0,-ecoli_pf_length_mm/2+backWallHeight/2,ecoli_stack_thickness])
        cube(size=[throatDiameter/2,backWallHeight,plateWidth],center=true);
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
module screwMounts() {
    translate([throatDiameter/2-plateWidth/2,0,0])
    difference(){
    cylinder(h=ecoli_pf_length_mm,r=(cassetteHeight+4)/2,center=true);
        cylinder(h=ecoli_pf_length_mm+1,r=(cassetteHeight)/2,center=true);
    }
        translate(-[throatDiameter/2-plateWidth/2,0,0])
    difference(){
    cylinder(h=ecoli_pf_length_mm,r=(cassetteHeight+4)/2,center=true);
        cylinder(h=ecoli_pf_length_mm+1,r=(cassetteHeight)/2,center=true);
    }
}
module thermosCassette() {
    union () {
        cassetteTop();
        cassetteBottom();
        rotate([90, 0, 0]) {
            screwMounts();
        }
    }
}

thermosCassette();