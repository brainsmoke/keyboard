
import sys
from os.path import basename

from symmetric_matrix import table_size, table_index

from layout import get_layout, get_led_mapping

from keycodes import get_keycodes

from hid_descriptor import parse_hid_descriptor

hid_input_report_size = 18
hid_output_report_size = 1

hid_keyboard_descriptor = """
Usage Page (Generic Desktop)
Usage (Keyboard)
Collection (Application)
  Report Size (1)
  Report Count (8)
  Usage Page (Keyboard)
  Usage Minimum (224)
  Usage Maximum (231)
  Logical Minimum (0)
  Logical Maximum (1)
  Input (Data, Variable, Absolute)
  Report Count (1)
  Report Size (8)
  Input (Constant)
  Report Count (5)
  Report Size (1)
  Usage Page (LED)
  Usage Minimum (1)
  Usage Maximum (5)
  Output (Data, Variable, Absolute)
  Report Count (1)
  Report Size (3)
  Output (Constant)
  Report Count (6)
  Report Size (8)
#  Logical Minimum (0)
#  Logical Maximum(255)
#  Usage Page (Keyboard)
#  Usage Minimum (0)
#  Usage Maximum (255)
#  Input (Data, Array)
  Input (Constant)
  Usage Page (Keyboard)
  Report Size (1)
  Report Count (79)
  Usage Minimum (4)
  Usage Maximum (82)
  Input (Data, Variable, Absolute)
  Report Size (1)
  Report Count (1)
  Input (Constant)
End Collection
"""


# labels

def get_matrix_defs(layout):
    num_rows = 0
    for row in layout:
        for key in row:
            r = key['matrix'][1]
            num_rows = max(num_rows, r+1)

    matrix_defs = [None] * 16 * num_rows

    def matrix_id(x, y):
        return 16*y + x

    num_rows = len(layout)
    for row in layout:
        for key in row:
            matrix_defs[matrix_id(*key['matrix'])] = key['key']

    return matrix_defs


def get_led_indices():
    led_indices = []

    m=get_led_mapping()
    for i in (6,5,4,3,2,1,0,10,9,8,7):
        led_indices.extend( [ s[4:] for s in  m[i] ])

    return led_indices

# tables

def get_matrix_to_report_bit(matrix_defs, keycodes):
    
    report_bit_scheme = [
    #     report_off  start   end
        ( 0,          0xe0,   0xe7 ),
    #      1*8 bits reserved
    #      6*8 bits keycode array
        ( 64,         0x04,   0x52 ),
    ]

    def map_keycode_to_report(code):
        for off, start, end in report_bit_scheme:
            if start <= code <= end:
                index = code-start+off
                byte_index = index//8
                bit_mask = 1<<(index&7)
                return f'{{ .byte = {byte_index:2d}, .mask = 0x{bit_mask:02x} }}'
        return "{             .mask = 0x00 } /* unmapped */"

    matrix_to_report_bit = [None]*len(matrix_defs)

    for i, key in enumerate(matrix_defs):
        val = -1

        if key is None:
            ix = f'0x{i:x}'
        else:
            ix = f'MATRIX_{key}'

            if key in keycodes:
                val = keycodes.index(key)

        matrix_to_report_bit[i] = (ix, map_keycode_to_report(val))

    return matrix_to_report_bit


def get_matrix_to_keycode(matrix_defs, keycodes):
    
    matrix_to_keycode = [None]*len(matrix_defs)

    for i, key in enumerate(matrix_defs):
        val = '0xff'

        if key is None:
            ix = f'0x{i:x}'
        else:
            ix = f'MATRIX_{key}'
            if key in keycodes:
                 val = f'KEYCODE_{key}'

        matrix_to_keycode[i] = (ix, val)

    return matrix_to_keycode


def get_matrix_to_led(matrix_defs, led_indices):
    
    for m in matrix_defs:
        if m is not None:
            assert m in led_indices

    for l in led_indices:
        if l != 'unused':
            assert l in matrix_defs

    matrix_to_led = [None]*len(matrix_defs)

    for i, key in enumerate(matrix_defs):
        val = 'LED_unused'

        if key is None:
            ix = f'0x{i:x}'
        else:
            ix = f'MATRIX_{key}'
            val = f'LED_{key}'

        matrix_to_led[i] = (ix, val)

    return matrix_to_led


def get_led_locations(led_indices, layout):
    u=19.05
    led_locations = [None] * len(led_indices)

    for row in layout:
        for key in row:
            ix = led_indices.index(key['key'])
            x, y, w, h = key['box']
            led_locations[ix] = ((x+w/2)*u, (y+h/2)*u)

    ix=led_indices.index('unused')
    led_locations[ix] = (0,0)

    return led_locations

def get_distance_matrix(locations):
    N = len(locations)
    m_sym = [None]*table_size(N)

    def d( a, b ):
        ax, ay = a
        bx, by = b
        dx, dy = ax-bx, ay-by
        return ( dx*dx + dy*dy ) **.5

    for i,pi in enumerate(locations):
        for j,pj in enumerate(locations):
            m_sym[table_index(i, j, N)] = int(.5+100*d(pi, pj))

    return m_sym

layout = get_layout()
matrix_defs = get_matrix_defs(layout)
keycodes = get_keycodes()
led_indices = get_led_indices()
matrix_to_report_bit = get_matrix_to_report_bit(matrix_defs, keycodes)
matrix_to_keycode = get_matrix_to_keycode(matrix_defs, keycodes)
matrix_to_led = get_matrix_to_led(matrix_defs, led_indices)

led_locations = get_led_locations(led_indices, layout)
led_distance_matrix = get_distance_matrix(led_locations)

# code

class CHeader:
    def __init__(self, name):
        self.name = name
    def __enter__(self):
        print(f"#ifndef {self.name}\n#define {self.name}\n")
    def __exit__(self, type, value, traceback):
        print(f"#endif // {self.name}")
        return False
 
class CEnum:
    def __init__(self, name=None):
        self.name = name
    def __enter__(self):
        if self.name:
            print(f"enum {self.name}\n{{")
        else:
            print("enum\n{")
    def __exit__(self, type, value, traceback):
        print("};\n")
        return False

def codegen_comment(arg):
    print(f"/* Auto-generated by {basename(__file__)} {arg} */")
    print()

def print_enum_defs(arr, prefix='', name = None):
    with CEnum(name):
        for i, s in enumerate(arr):
            if s is not None:
                print (f'\t{prefix}{s} = 0x{i:x},')

def print_enum_defs_swap_fn(arr, prefix, name):
    fn_index = arr.index("fn")
    leftcontrol_index = arr.index("leftcontrol")
    with CEnum(name):
        for i, s in enumerate(arr):
            if s is not None and s not in ('leftcontrol', 'fn'):
                print (f'\t{prefix}{s} = 0x{i:x},')
            elif s == 'leftcontrol':
                print (
f"""/* define for ThinkPad style layout: */
#ifdef SWAP_CONTROL_AND_FN
	{prefix}leftcontrol = 0x{fn_index:x},
	{prefix}fn = 0x{leftcontrol_index:x},
#elif
	{prefix}leftcontrol = 0x{leftcontrol_index:x},
	{prefix}fn = 0x{fn_index:x},
#endif""")

def print_table(arr, decl):
    maxlen = max( len(i) for i, v in arr )
    print (f'{decl} = {{')
    for i, v in arr:
        print (f'\t[{i}]{" "*(maxlen-len(i))} = {v},')
    print ('};\n')

def print_num_array(arr, decl):
    print (f'{decl} = {{')
    arr = [f'{n},' for n in arr]
    maxlen = max( len(s) for s in arr )
    arr = [' '*(maxlen-len(s)) + s for s in arr]
    n_cols = (80-4)//(maxlen+1)
    for i in range(0, len(arr), n_cols):
        print ('\t' + ' '.join(arr[i:i+n_cols]))
    print ('};\n')


def print_hid_descriptor():
    print('static const uint8_t hid_keyboard_descriptor[] = {')
    print(parse_hid_descriptor(hid_keyboard_descriptor))
    print ('};\n')

if __name__ == '__main__':
    if sys.argv[1] == 'header':
        with CHeader("CODEGEN_HEADER_H"):
            codegen_comment('header')
            print('#include "config.h"\n')
            print_enum_defs_swap_fn(matrix_defs, 'MATRIX_', 'matrix_defs')
            print_enum_defs(keycodes, 'KEYCODE_', 'keycodes')
            print_enum_defs_swap_fn(led_indices, 'LED_', 'led_index')
    elif sys.argv[1] == 'tables':
            print('#include "codegen_header.h"\n')
            codegen_comment('tables')
            print_hid_descriptor()
            print_table(matrix_to_report_bit, 'static const struct { uint8_t byte, mask; } matrix_to_report_bit[]')
            print_table(matrix_to_keycode, 'static const uint8_t matrix_to_keycode[]')
            print_table(matrix_to_led, 'static const uint8_t matrix_to_led[]')
    elif sys.argv[1] == 'led_distances':
            codegen_comment('led_distances')
            print_num_array(led_distance_matrix, 'static const uint16_t led_distance_matrix[]')
