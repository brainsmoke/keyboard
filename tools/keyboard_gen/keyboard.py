u=19.05
schema_u = u

import sys

from layout import get_layout, get_led_mapping
from schematic import Schematic, Switch, Diode, LED, Resistor, Capacitor, Wire, Junction, HierarchicalLabel, Label, TLC5916, add_pos, sub_pos

_designators = {}
def designator(base):
   i =  _designators.get(base, 0)
   i += 1
   _designators[base] = i
   return base+str(i)

def footprint(key):
    x, y, w, h = key['box']
    return f"footprints:SW_Cherry_MX_{float(w):#.2f}u_PCB"

def schema_pos(x, y):
    return round(x, 2), round(y, 2)

def row_label(key):
    c, r = key['matrix']
    return "r"+str(r)

def col_label(key):
    c, r = key['matrix']
    return "c"+str(c)

def get_configuration():
    conf = get_layout()

    x_start, y_start = 25.4, 25.4
    for r_ix, row in enumerate(conf):
        for key in row:
            x, y, w, h = key['box']
            pos = schema_pos( x_start+schema_u*(x+w/2), y_start+schema_u*(y+h/2) )
            key['switch'] = Switch(pos, designator("SW"), "key_"+key['key'], footprint(key))
            diode_pos = schema_pos( pos[0], pos[1]+2.54*3 )
            key['diode'] = Diode(diode_pos, designator("D"), "d_"+key['key'], "Diode_SMD:D_SOD-123")
            key['LED'] = LED(pos, designator("LED"), "LED_"+key['key'], "footprints:LED3mm")

    return conf

def draw_wire(s, pos, path):
    for (p, x) in path:
        last_pos = pos
        pos = add_pos(pos, {
            'up' : (0, -x),
            'down' : (0, x),
            'left' : (-x, 0),
            'right' : (x, 0),
        }[p])
        s.add(Wire(last_pos, pos))
    return pos

def add_label(s, pos, name, label_type, pin_type, path):
    pos = draw_wire(s, pos, path)
    dir = "right" 
    if len(path) > 0:
        dir=path[-1][0]

    rot = { 'up': 90, 'down': 270, 'left': 180, 'right': 0 }[dir]
    s.add({ 'h' : HierarchicalLabel, 'l' : Label}[label_type](pos, name, pin_type, rot=rot))

def get_keys_schematic(conf):

    s = Schematic(paper="A3")

    for r_ix, row in enumerate(conf):

        for key in row[:1]:
            cathode_pos = key['diode'].pos("K")
            high_pos = key['switch'].pos("1")
            row_connect_pos = (cathode_pos[0]-2.54, high_pos[1]-6.35)
            last_label_pos = (15*1.27, row_connect_pos[1])
            last_label = row_label(key)
            s.add(HierarchicalLabel(last_label_pos, last_label, "input", rot=180))
            s.add(Wire(last_label_pos, row_connect_pos))

        for key in row:
            s.add(key['switch'])
            s.add(key['diode'])
            anode_pos = key['diode'].pos("A")
            cathode_pos = key['diode'].pos("K")

            low_pos = key['switch'].pos("2")
            high_pos = key['switch'].pos("1")
            row_connect_pos = (cathode_pos[0]-2.54, high_pos[1]-6.35)
            col_label_pos = add_pos(cathode_pos, (-2.54, 0))

            cur_row_label = row_label(key)
            if last_label != cur_row_label:
                last_label_pos = add_pos(row_connect_pos, (-2.54, 0))
                last_label = cur_row_label
                s.add(HierarchicalLabel(last_label_pos, last_label, "input", rot=180))

            s.add(Wire(last_label_pos, row_connect_pos))
            s.add(Junction(row_connect_pos))

            last_label_pos = row_connect_pos

            s.add(HierarchicalLabel(col_label_pos, col_label(key), "output", rot=180))

            s.add(Wire(low_pos, (anode_pos[0]+2.54, low_pos[1])))
            s.add(Wire(high_pos, (cathode_pos[0]-2.54, high_pos[1])))
            s.add(Wire((cathode_pos[0]-2.54, high_pos[1]), row_connect_pos))
            s.add(Wire(anode_pos, add_pos(anode_pos, (2.54, 0))))
            s.add(Wire(cathode_pos, add_pos(cathode_pos, (-2.54, 0))))
            s.add(Wire((anode_pos[0]+2.54, low_pos[1]), add_pos(anode_pos, (2.54, 0))))

    return s

def LED_driver_symbol(s, pos, n, pin_labels):
    d = TLC5916(schema_pos(*pos), designator("U"), f"TLC5916_{n}", footprint="Package_SO:TSSOP-16_4.4x5mm_P0.65mm")
    s.add(d)
    r = Resistor(draw_wire(s, d.pos(f"REXT"), [ ('left', 10.16), ("down", 2.54) ]), designator("R"), f'REXT_{n}', footprint='Resistor_SMD:R_0402_1005Metric', relative_to="1")
    s.add(r)
    c = Capacitor(add_pos(d.pos(), (-31.75, 0)), designator("C"), f'1uF', footprint='Capacitor_SMD:C_0402_1005Metric')
    s.add(c)

    for p in ( d.pos("VDD"), c.pos("1")):
        add_label(s, p, "3V3", "h", "input", [ ("up", 2.54), ("right", 2.54) ])

    for p in ( d.pos("GND"), r.pos("2"), c.pos("2") ):
        add_label(s, p, "gnd", "h", "input", [ ("down", 2.54), ("right", 2.54) ])


    add_label(s, d.pos("CLK"), "clk", "h", "input", [ ("left", 2.54) ])
    add_label(s, d.pos("OE"), "oe", "h", "input", [ ("left", 2.54) ])
    add_label(s, d.pos("LE"), "latch", "h", "input", [ ("left", 2.54) ])

    add_label(s, d.pos("SDO"), f"data_out{n}", "l", None, [ ("right", 2.54) ])

    if (n == 0):
        add_label(s, d.pos("SDI"), "data_left", "h", "input", [ ("left", 2.54) ])
    elif (n == 7):
        add_label(s, d.pos("SDI"), "data_right", "h", "input", [ ("left", 2.54) ])
    else:
        add_label(s, d.pos("SDI"), f"data_out{n-1}", "l", None, [ ("left", 2.54) ])

    for i, label in enumerate(pin_labels):
        add_label(s, d.pos(f"OUT{i}"), label, "l", None, [ ('right', 2.54) ])

def get_LEDs_schematic(conf):
    s = Schematic(paper="A3")
    mapping = get_led_mapping()

    for r_ix, row in enumerate(conf):
        for key in row:
            s.add(key['LED'])
            anode_pos = key['LED'].pos("A")
            add_label(s, key['LED'].pos("A"), "V_in", "h", "input", [ ("right", 1.27), ('up', 5.08), ('right', 1.27) ])
            add_label(s, key['LED'].pos("K"), "led_"+key['key'], "l", None, [ ("left", 1.27), ('down', 7.62), ('left', 1.27) ])

    for i in range(11):
        LED_driver_symbol(s, (50.8+i*63.5, 177.8), i, mapping[i])

    return s


conf = get_configuration()

if sys.argv[1] == 'schematic':
    s = get_keys_schematic(conf)
    print(s)
if sys.argv[1] == 'leds':
    s = get_LEDs_schematic(conf)
    print(s)
