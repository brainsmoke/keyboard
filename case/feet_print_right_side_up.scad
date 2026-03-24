use <feet.scad>;


translate([0,-43,0])
{
rotate( [5,0,0] )
left_foot();

translate([-268,25,0])
rotate( [5,0,0] )
right_foot();
}
