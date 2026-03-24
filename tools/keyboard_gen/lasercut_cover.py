
import sys

import svg, path_manual
from layout import get_layout

def rect_path( mid_pos, dim ):
    x, y = mid_pos
    dx, dy = dim
    bounds = [ (-.5,-.5), (.5,-.5), (.5,.5), (-.5,.5) ]
    return [ (x+dx*bx, y+dy*by) for bx, by in bounds ]

def vec_sum( *v ):
   x, y = 0., 0.
   for dx, dy in v:
       x, y = (x+dx, y+dy)
   return x, y

u=19.05

edges_origin = 23.113, 23.225
board_offset = 25.4, 25.4

diode_path = rect_path( (4.3, -2.2), (5,2) )
switch_path = rect_path( (0,0), (17,17) )
LED_path = rect_path( (0,-5.08), (5.08,2.54) )
mid_hole_path = rect_path( (0,0), (4.5,4.5) )
side_holes_path = rect_path( (0,0), (10.16+2, 2) )
pad_a_path = rect_path( (-2.54, 5.08), (3,3) )
pad_b_path = rect_path( (3.81,2.54), (3,3) )
stabilizer_path = rect_path( (0,0), (7.2,20.5) )
stabilizer_l2_path = rect_path( (0,0), (7.2,11.5) )

hull_path = LED_path[0:2] + diode_path[1:3] + [ side_holes_path[2], pad_b_path[2] ] + pad_a_path[2:4] + [side_holes_path[3], side_holes_path[0] ]

hole_d = 3.2
holes = [ vec_sum( edges_origin, pos ) for pos in (
    (31,12),
    (357-64,12),
    (5,125-50),
    (357-5,125-50),
) ]

def stabilizer_rod(w):
    return rect_path( (0,-9), (w,2.5) )

l = get_layout()
def all_keys():
    for row in l:
        for key in row:
            yield key

def footprint_pos(key):
    x, y, w, h = key['box']
    return vec_sum( (u*(x+w/2.), u*(y+h/2.)), board_offset )

def get_footprints_path(shape):
    paths = []
    for key in all_keys():
        paths.append( [ vec_sum(footprint_pos(key), p) for p in shape ] )
    return paths

def stabilizer_locations():
    for key in all_keys():
        if key['key'] in ('leftshift','rightshift','enter','backspace'):
            for dx in (-5/8*u, 5/8*u):
                yield vec_sum(footprint_pos(key), (dx,0))
        if key['key'] in ('space',):
            for dx in (-50, 50):
                yield vec_sum(footprint_pos(key), (dx,0))

def get_stabilizers_path(shape):
    paths = []
    for loc in stabilizer_locations():
        paths.append( [ vec_sum(loc, p) for p in shape ] )

    return paths

def get_stabilizer_rod():
    paths = []
    for key in all_keys():
        if key['key'] in ('leftshift','rightshift','enter','backspace'):
            paths.append( [ vec_sum(footprint_pos(key), p) for p in stabilizer_rod(5/4*u) ] )
        if key['key'] in ('space',):
            paths.append( [ vec_sum(footprint_pos(key), p) for p in stabilizer_rod(100) ] )
    return paths

def common(stroke_outside, stroke_inside):
    path_manual.edges(stroke=stroke_outside, fill='none')
    for pos in holes:
        svg.circle(pos, hole_d/2, stroke=stroke_inside)

if __name__ == '__main__':

    outside = '#000000'
    inside = '#ff0000'
    engrave = '#0000ff'

    svg.header(500,250)

    if sys.argv[1] in ('top1', 'all'):
        svg.start_group()

        common(outside, inside)

        svg.path(get_footprints_path(switch_path), stroke=inside, fill='none')
        svg.path(get_stabilizers_path(stabilizer_path), stroke=inside, fill='none')
        svg.path(get_stabilizer_rod(), stroke=inside, fill='none')

        path_manual.usb_c_connector_solder_blob(fill=engrave)

        svg.end_group()

    if sys.argv[1] in ('top2', 'all'):
        svg.start_group()

        common(outside, inside)

        svg.path(get_footprints_path(switch_path), stroke=inside, fill='none')
        svg.path(get_stabilizers_path(stabilizer_l2_path), stroke=inside, fill='none')

        svg.end_group()

    if sys.argv[1] in ('bottom1', 'all'):
        svg.start_group()

        common(outside, inside)

        for pos in stabilizer_locations():
            svg.circle(vec_sum(pos, (0,7)), 4.5/2, stroke=inside)

        for pos in stabilizer_locations():
            svg.circle(vec_sum(pos, (0,-10.24)), 3.2/2, stroke=inside)

        #svg.path(get_footprints_path(diode_path), stroke=inside, fill='none')
        #svg.path(get_footprints_path(LED_path), stroke=inside, fill='none')
        #svg.path(get_footprints_path(mid_hole_path), stroke=inside, fill='none')
        #svg.path(get_footprints_path(side_holes_path), stroke=inside, fill='none')
        #svg.path(get_footprints_path(pad_a_path), stroke=inside, fill='none')
        #svg.path(get_footprints_path(pad_b_path), stroke=inside, fill='none')
        svg.path(get_footprints_path(hull_path), stroke=inside, fill='none')
        path_manual.bottom_components_keepout(stroke=inside, fill='none')

        svg.end_group()

    if sys.argv[1] in ('bottom2', 'all'):
        svg.start_group()
        common(outside, inside)

        path_manual.usb_c_connector_engrave_bump(fill=engrave)
        path_manual.swirl(stroke=outside)

        svg.end_group()

    svg.footer()

