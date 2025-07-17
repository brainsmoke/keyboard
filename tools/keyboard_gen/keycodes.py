
def get_keycodes():
    keycodes = [None] * 4
    for c in 'abcdefghijklmnopqrstuvwxyz1234567890':
        keycodes.append( c )
    for k in ('enter', 'esc', 'backspace', 'tab', 'space', 'minus', 'equals', 'openbracket', 'closebracket', 'backslash', 'ISO',
              'semicolon', 'quote', 'tilde', 'comma', 'dot', 'slash', 'capslock'):
        keycodes.append( k )
    for i in range(1,12+1):
        keycodes.append( f'f{i}' )
    for k in ('printscreen', 'scrollock', 'pause', 'insert', 'home', 'pageup', 'delete', 'end', 'pagedown', 'right', 'left', 'down', 'up'):
        keycodes.append( k )
    keycodes += [None]*(0xE0-len(keycodes))
    for k in ('leftcontrol', 'leftshift', 'alt', 'win', 'rightcontrol', 'rightshift', 'altgr', 'menu'):
        keycodes.append( k )
    return keycodes

