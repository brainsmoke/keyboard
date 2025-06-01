
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

    return keys

