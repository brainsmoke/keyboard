use <feet.scad>;

translate([0,150,0])
rotate([180,0,0])
{
left_foot();

translate([-267,25,0])
right_foot();
}
