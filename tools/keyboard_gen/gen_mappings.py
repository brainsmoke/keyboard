
import sys

from layout import get_layout

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
 
layout = get_layout()

matrix_keys = [None] * 256

keycodes = [None] * 4
for c in 'abcdefghijklmnopqrstuvwxyz1234567890':
    keycodes.append( c )
for k in ('enter', 'escape', 'delete', 'tab', 'space', 'minus', 'plus', 'openbracket', 'closebracket', 'backslash', 'ISO',
          'semicolon', 'quote', 'tilde', 'comma', 'dot', 'slash', 'capslock'):
    keycodes.append( k )
for i in range(1,12+1):
    keycodes.append( f'f{i}' )
for k in ('printscreen', 'scrollock', 'pause', 'insert', 'home', 'pageup', 'delete', 'end', 'pagedown', 'right', 'left', 'down', 'up'):
    keycodes.append( k )

def matrix_id(x, y):
    return 16*y + x

max_row = -1
for row in layout:
    for key in row:
        matrix_keys[matrix_id(*key['matrix'])] = key['key']
        max_row = max(max_row, key['matrix'][1])
for x in range(16):
    matrix_keys[matrix_id(x, max_row+1)] = matrix_keys[x]+'_alt'


def print_enum_defs(arr, prefix='', name = None):
    with CEnum(name):
        for i, s in enumerate(arr):
            if s is not None:
                print (f'\t{prefix}{s} = 0x{i:x},')

if __name__ == '__main__':
    if sys.argv[1] == 'header':
        with CHeader("KEYBOARD_DEFS_H"):
            print('#include "config.h"\n')
            print_enum_defs(matrix_keys, 'MATRIX_', 'matrix_keys')
            print_enum_defs(keycodes, 'KEYCODE_', 'keycodes')
    elif sys.argv[1] == 'tables':
            print('#include "keyboard_defs.h"\n')
