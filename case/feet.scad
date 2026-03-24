e=.001;
$fn=48;
padding=5;

dim = [ 357, 125, 10 ] ;

holes = [ [31, dim.y-12], [dim.x-64, dim.y-12], [ 5, 50 ], [dim.x-5, 50 ] ] ;

r = 5;

bottom = 1.2;
screw_hole_d = 2.8;

angle = 5;

module do_center(dim, center)
{
    if (center)
        translate(-dim/2)
            children();
    else
        children();
}
 
module rounded_box(dim, r, center=false)
{
    do_center(dim, center)
    hull()
    {
        for (x=[r,dim.x-r])
        for (y=[r,dim.y-r])
        translate([x,y,0])
        cylinder(h=dim.z,r=r);
    }
}

module preview()
{
	if ($preview) children();
}


module at_holes()
{
	for (pos=holes)
        translate([pos.x, pos.y, 0])
            children();
}

module reflect_x(x)
{
	translate([x,0,0])
	scale([-1,1,1])
	translate([-x,0,0])
	children();
}

module mirror_x(x)
{
	children();
	reflect_x(x)
	children();
}


module feet()
{

	difference()
	{
	union()
	{
		mirror_x(dim.x/2)
		hull()
		{
		rotate([-angle,0,0])
		translate([2,dim.y-16,0])
		rounded_box([67, 10, bottom], 5);
		translate([0,dim.y-20,-bottom])
		rounded_box([76, 20, bottom], r);
		}

		mirror_x(dim.x/2)
		hull()
		{
		rotate([-angle,0,0])
		translate([2,dim.y-81,0])
		rounded_box([8, 75, bottom], 4);
		translate([0,dim.y-81,-bottom])
		rounded_box([10, 81, bottom], r);
		}
	}

	at_holes() translate([0,0,-20+e]) cylinder(d=screw_hole_d, h=20);
	}
}

module left_foot()
{
	intersection()
	{
	translate([-5,-5,-25])
	cube([dim.x/2+5,dim.y+10, 30]);
	feet();
	}
}


module right_foot()
{
	intersection()
	{
	reflect_x(dim.x/2)
	translate([-5,-5,-25])
	cube([dim.x/2+5,dim.y+10, 30]);
	feet();
	}
}

preview()
{
left_foot();
right_foot();
}
