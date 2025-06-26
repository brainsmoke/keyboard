
def get_layout():

    h_spacing = [

        [ 1, 1 ] + ( [ 1 ]*4 + [.5] ) *3 + [ 1,1,1 ],
        [],
                     [ 1 ] * 13 + [2, .5, 1,1,1],
        [ 1.5 ]    + [ 1 ] * 12 + [1.5, .5, 1,1,1],
        [ 1.75 ]   + [ 1 ] * 11 + [2.25],
        [ 2.25 ]   + [ 1 ] * 10 + [2.75]    + [ 1.5,  1],
    	[ 1.25 ]*3 + [6.25] +     [1.25] *4 + [  .5,  1, 1, 1],
    ]

    v_spacing = [1, 1/3, 1, 1, 1, 1, 1]

    key_names = [

        [ 'esc', None, 'f1', 'f2', 'f3', 'f4', None, 'f5', 'f6', 'f7', 'f8', None, 'f9', 'f10', 'f11', 'f12', None, 'printscreen', 'scrollock', 'pause'],
        [],
        [ 'tilde', '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', 'minus', 'equals', 'backspace', None, 'insert', 'home', 'pageup' ],
        [ 'tab', 'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p', 'openbracket', 'closebracket', 'backslash', None, 'delete', 'end', 'pagedown' ],
        [ 'capslock', 'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'semicolon', 'quote', 'enter' ],
        [ 'leftshift', 'z', 'x', 'c', 'v', 'b', 'n', 'm', 'comma', 'dot', 'slash', 'rightshift', None, 'up' ],
        [ 'leftcontrol', 'fn', 'alt', 'space', 'altgr', 'win', 'menu', 'rightcontrol', None, 'left', 'down', 'right' ],
    ]

    assert(len(h_spacing) == len(key_names))
    for a, b in zip(h_spacing, key_names):
        assert(len(a) == len(b))

    keys = []
    y = 0
    for row, dy in enumerate(v_spacing):

        if len(h_spacing[row]) > 0:
            x = 0
            key_row = []
            for dx, name in zip(h_spacing[row], key_names[row]):
                if name is not None:
                    key_row.append( {'box': (x, y, dx, dy), 'key': name})

                x += dx
            keys.append(key_row)

        y += dy

    for row, r in enumerate(keys[:-1]):
        for col, key in zip(range(16), r):
            key['matrix'] = (col, row)

    for row, r in [ [5, keys[-1]] ]:
        for col, key in zip([0,1,2,5,8,9,11,12,13,14,15], r):
            key['matrix'] = (col, row)

    keys[4][11]['matrix'] = (12, 4)
    keys[4][12]['matrix'] = (14, 4)
    keys[1][16]['matrix'] = (15, 3)
    keys[2][16]['matrix'] = (15, 4)

    return keys

def get_led_mapping():

    mapped = [ [ f"drv_{i}_{c}" for c in 'abcdefgh' ] for i in range(11) ]

    def do_map(i, keys):
        for m in mapped:
             for j, key in enumerate(keys):
                 if key is not None:
                     assert key not in [ k for r in mapped for k in r ]
                     mapped[i][j] = f'led_{key}'

    do_map(6, ( 'esc', 'tilde', 'z', 'fn', 'tab', 'capslock', 'leftshift', 'leftcontrol' ) )
    do_map(5, ( 'alt', 'x', 's', 'w', '1', 'q', 'a', 'f1' ) )
    do_map(4, ( 'd', 'e', '3', 'v', 'c', '2', 'f3', 'f2' ) )
    do_map(3, ( 'space', 'b', 'g', 't', '4', 'r', 'f', 'f4' ) )
    do_map(2, ( 'y', '6', 'j', 'm', 'h', 'n', '5', 'f5' ) )
    do_map(1, ( 'f7', 'comma', 'i', 'k', 'altgr', '7', 'u', 'f6' ) )
    do_map(0, ( None, 'l', 'o', '9', 'dot', 'win', '8', 'f8' ) )
    do_map(7, ( 'minus', 'openbracket', 'quote', 'menu', '0', 'p', 'semicolon', 'slash' ) )
    do_map(8, ( 'f11', 'f10', 'closebracket', 'equals', 'enter', 'rightshift', 'rightcontrol', 'f9' ) )
    do_map(9, ( 'printscreen', 'delete', 'up', 'down', 'left', 'backspace', 'backslash', 'f12' ) )
    do_map(10, ( 'pause', 'pageup', 'pagedown', 'right', 'home', 'end', 'insert', 'scrollock' ) )

    flat_mapped = [ k for r in mapped for k in r ]
    keynames = [ f'led_{x["key"]}' for r in get_layout() for x in r ]

    for k in keynames:
        if k not in flat_mapped:
             print(k)
             assert k in flat_mapped

    return mapped

