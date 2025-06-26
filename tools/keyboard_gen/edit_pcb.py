
from kipy import KiCad
from kipy.geometry import Vector2, Angle
from kipy.board_types import BoardLayer

from keyboard import get_configuration

u=19.05
diode_offset = (4.3, -2.2)
switch_offset = (-2.54 , 5.08)
board_offset = (25.4,25.4)
LED_offset = (0,-5.08)

def get_footprint_pos(key, u, offset=(0,0), board=(0,0)):
    x, y, w, h = key['box']
    dx, dy = offset
    bdx, bdy = board
    return Vector2.from_xy_mm(u*(x+w/2.)+dx+bdx, u*(y+h/2.)+dy+bdy)

conf = get_configuration()

keys_by_switch_value = { k['switch'].value() : k for row in conf for k in row }
keys_by_diode_value = { k['diode'].value() : k for row in conf for k in row }
keys_by_LED_value = { k['LED'].value() : k for row in conf for k in row }
kicad = KiCad()
board = kicad.get_board()
commit = board.begin_commit()
footprints = board.get_footprints()
for footprint in footprints:
    value = footprint.value_field.text.value
    if value in keys_by_switch_value:
        key = keys_by_switch_value[value]
        footprint.orientation = Angle.from_degrees(180)
        footprint.position = get_footprint_pos(key, u=u, offset=switch_offset, board=board_offset)
        footprint.reference_field.visible = False
    elif value in keys_by_diode_value:
        key = keys_by_diode_value[value]
        footprint.position = get_footprint_pos(key, u=u, offset=diode_offset, board=board_offset)
        footprint.orientation = Angle.from_degrees(180)
        if footprint.layer != BoardLayer.BL_B_Cu:
            footprint.definition.id.library = "invalidate"
            footprint.layer = BoardLayer.BL_B_Cu
        footprint.reference_field.visible = False
    elif value in keys_by_LED_value:
        key = keys_by_LED_value[value]
        footprint.position = get_footprint_pos(key, u=u, offset=LED_offset, board=board_offset)
        if footprint.layer != BoardLayer.BL_B_Cu:
            footprint.definition.id.library = "invalidate"
            footprint.layer = BoardLayer.BL_B_Cu


board.update_items(footprints)
board.push_commit(commit)
