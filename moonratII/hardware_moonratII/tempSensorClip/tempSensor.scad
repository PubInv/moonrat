

// Simple torus in OpenSCAD

minor_radius = 3;    // Radius of the tube
major_radius = 84/2 - minor_radius;   // Distance from center to tube center
chamber_height = 6.5*25.4; // approximately 6.25" high
shaft_radius = 3;
chamber_half_height = chamber_height/2;
harp_major_radius_interior = 5/2;

module chamberFit() {
    rotate_extrude($fn=100)
    translate([major_radius, 0, 0])
    circle(r = minor_radius, $fn=60);
}

module harp() {
    harp_minor_radius = 5/2;
    harp_major_radius = harp_minor_radius+harp_major_radius_interior;
    gap_angle = 110;
    translate([major_radius-shaft_radius*2,0,chamber_half_height])
    rotate([0,0,180+gap_angle/2])
    rotate_extrude($fn=100,angle = 360-gap_angle)
    translate([harp_major_radius, 0, 0])
    circle(r = harp_minor_radius, $fn=60);
}

module tempSensorHolder() {
    translate([0,0,chamber_half_height])
        chamberFit();
    chamberFit();
    // connecting shaft
    translate([major_radius,0,0])
        cylinder(r=shaft_radius,h=chamber_half_height, $fn=60);
    translate([-major_radius,0,0])
        cylinder(r=shaft_radius,h=chamber_half_height, $fn=60);
    // add holding clip
    harp();
    // add additional clip more toward the center
    clip_shaft_length = major_radius/2;
    rotate([0,0,180])
    translate([-clip_shaft_length,0,0])
        harp();
    // clip holding shaft
    translate([-major_radius+clip_shaft_length/2 + minor_radius,0,chamber_half_height])
    rotate([0,90,0])
    cylinder(h = clip_shaft_length,r = harp_major_radius_interior, center = true,$fn=100); 
}

tempSensorHolder();