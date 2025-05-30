u=19.05
schema_u = 20.32


from layout import get_layout
from schematic import Schematic, Switch, Diode, Wire, Junction, HierarchicalLabel, add_pos, sub_pos

_designators = {}
def designator(base):
   i =  _designators.get(base, 0)
   i += 1
   _designators[base] = i
   return base+str(i)

def footprint(key):
    x, y, w, h = key['box']
    return f"SW_Cherry_MX_{float(w):#.2f}_PCB"

def row_label(key):
    c, r = key['matrix']
    return "r"+str(r)

def col_label(key):
    c, r = key['matrix']
    return "c"+str(c)

layout = get_layout()

s = Schematic(paper="A3")

for row, r in enumerate(layout):
    for col, key in zip(range(16), r):
        key['matrix'] = (col, row)
layout[1][16]['matrix'] = (15, 5)
layout[2][16]['matrix'] = (14, 5)


x_start, y_start = 25.4, 25.4
for r_ix, row in enumerate(layout):

    for key in row:
        x, y, w, h = key['box']
        pos = x_start+ schema_u*(x+w/2), y_start+schema_u*(y+h/2)
        key['switch'] = Switch(pos, designator("SW"), "key_"+key['key'], footprint(key))
        s.add(key['switch'])
        diode_pos = pos[0], pos[1]+2.54*3
        key['diode'] = Diode(diode_pos, designator("D"), "", "Diode_SMD:D_SOD-123")
        s.add(key['diode'])



for r_ix, row in enumerate(layout):

    for key in row[:1]:
        cathode_pos = key['diode'].pos("K")
        high_pos = key['switch'].pos("1")
        row_connect_pos = (cathode_pos[0]-2.54, high_pos[1]-6.35)
        last_label_pos = (15*1.27, row_connect_pos[1])
        last_label = row_label(key)
        s.add(HierarchicalLabel(last_label_pos, last_label, "input"))
        s.add(Wire(last_label_pos, row_connect_pos))


    for key in row:
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
            s.add(HierarchicalLabel(last_label_pos, last_label, "input"))

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



print(s)
