//    Copyright 2021, Robert L. Read and Avinash Baskaran
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


//A simple Box with Rounded Edges


module rcube(size,fR) {
    bL = size[0];
    bW = size[1];
    bH = size[2];

    hull() union() {
        // first, we add the central object....
        union(){
            cube(size=[bL-2*fR,bW-2*fR,bH],center=true);
            cube(size=[bL,bW-2*fR,bH-2*fR],center=true);
            cube(size=[bL-2*fR,bW,bH-2*fR],center=true);
        }
        // next we add the corner spheres
        union() {
            for(xdim = [-1:2:1])
                for(ydim = [-1:2:1])
                    for(zdim = [-1:2:1])
            translate([xdim*(bL/2 -fR),ydim*(bW/2 -fR),zdim*(bH/2 -fR)])
            sphere(fR);
         }
     }
}

// boxLength = 25;
// boxWidth = 35;
// boxHeight = 15;
// fR  = 4;
// rcube([boxLength,boxWidth,boxHeight],fR);