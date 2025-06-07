#
# ugly get the work done code
#
# to be replaced by calls to kicad API once those work for schematics
#

import re

def str_at(t, a=0):
    x, y = t
    x, y, a = float(x), float(y), float(a)
    return f"(at {x:#.5f} {y:#.5f} {a:#.5f})"


def str_point(t):
    x, y = t
    x, y = float(x), float(y)
    return f"(at {x:#.5f} {y:#.5f})"


def str_xy(t):
    x, y = t
    x, y = float(x), float(y)
    return f"(xy {x:#.5f} {y:#.5f})"

def yflip(p):
    x, y = p
    return (x, -y)

def sub_pos(a, b):
    ax, ay = a
    bx, by = b
    return (ax-bx, ay-by)

def add_pos(a, b):
    ax, ay = a
    bx, by = b
    return (ax+bx, ay+by)

class Symbol:

    def __init__(self, name="", anchors={}):
        self._name = name
        self.anchors = anchors

    def pos(self, ref=None):
        if ref is None:
            return (0,0)
        return self.anchors[ref]

    def name(self):
        return self._name

    def definition(self):
        raise("meh.")

    def instance(self, instance):
        raise("meh.")


class SwitchSymbol(Symbol):
    def __init__(self, name):
        anchors = {
            "1":           ( -5.08, 0     ),
            "2":           (  5.08, 0     ),
            "Reference":   (  1.27, 2.54  ),
            "Value":       (  0,   -5.08  ),
            "Footprint":   (  0,    5.08  ),
            "Datasheet":   (  0,    5.08  ),
            "Description": (  0,    0     ),
        }
        super().__init__(name, anchors)

    def definition(self):
        def at(s, a=0):
            return str_at(yflip(self.pos(s)), a)
        return f"""
		(symbol "{self.name()}"
			(pin_numbers (hide yes))
			(pin_names (offset 1.016) (hide yes))
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(property "Reference" "SW" {at("Reference")} (effects (font (size 1.27 1.27)) (justify left)))
			(property "Value" "SW_Push" {at("Value")} (effects (font (size 1.27 1.27))))
			(property "Footprint" "" {at("Datasheet")} (effects (font (size 1.27 1.27)) (hide yes)))
			(property "Datasheet" "~" {at("Datasheet")} (effects (font (size 1.27 1.27)) (hide yes)))
			(property "Description" "Push button switch, generic, two pins" {at("Description")} (effects (font (size 1.27 1.27)) (hide yes)))
			(symbol "SW_Push_0_1"
				(circle (center -2.032 0) (radius 0.508) (stroke (width 0) (type default)) (fill (type none)))
				(polyline (pts (xy 0 1.27) (xy 0 3.048)) (stroke (width 0) (type default)) (fill (type none)))
				(circle (center 2.032 0) (radius 0.508) (stroke (width 0) (type default)) (fill (type none)))
				(polyline (pts (xy 2.54 1.27) (xy -2.54 1.27)) (stroke (width 0) (type default)) (fill (type none)))
				(pin passive line {at("1", 0)} (length 2.54) (name "1" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))
				(pin passive line {at("2", 180)} (length 2.54) (name "2" (effects (font (size 1.27 1.27)))) (number "2" (effects (font (size 1.27 1.27)))))
			)
			(embedded_fonts no)
		)
"""

    def instance(self, instance):
        ref = instance.ref()
        if ref is None:
            ref = "SW"
        value = instance.value()
        if value is None:
            value = "SW_Push"

        footprint = instance.footprint()
        if footprint is None:
            footprint = ""

        def at(s=None, a=0):
            return str_at( instance.pos(s), a)
        return f"""	
	(symbol
		(lib_id "{self.name()}")
		{at()}
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(fields_autoplaced yes)
		(property "Reference" "{ref}" {at('Reference')} (effects (font (size 1.27 1.27))))
		(property "Value" "{value}" {at('Value')} (effects (font (size 1.27 1.27))))
		(property "Footprint" "{footprint}" {at('Footprint')} (effects (font (size 1.27 1.27)) (hide yes)))
		(property "Datasheet" "~" {at('Datasheet')} (effects (font (size 1.27 1.27)) (hide yes)))
		(property "Description" "Push button switch, generic, two pins" {at('Description')} (effects (font (size 1.27 1.27)) (hide yes)))
		(pin "2")
		(pin "1")
	)
"""

class DiodeSymbol(Symbol):
    def __init__(self, name):
        anchors = {
            "1":           ( -3.81, 0     ),
            "2":           (  3.81, 0     ),
            "K":           ( -3.81, 0     ),
            "A":           (  3.81, 0     ),
            "Reference":   (  0,    2.54  ),
            "Value":       (  0,   -2.54  ),
            "Footprint":   (  0,    0     ),
            "Datasheet":   (  0,    0     ),
            "Description": (  0,    0     ),
        }
        super().__init__(name, anchors)

    def definition(self):
        def at(s, a=0):
            return str_at(yflip(self.pos(s)), a)
        return f"""
		(symbol "{self.name()}"
			(pin_numbers (hide yes))
			(pin_names (offset 1.016) (hide yes))
			(exclude_from_sim no)
			(in_bom yes)
			(on_board yes)
			(property "Reference" "D" {at("Reference")} (effects (font (size 1.27 1.27))))
			(property "Value" "D_Schottky" {at("Value")} (effects (font (size 1.27 1.27))))
			(property "Footprint" "" {at("Footprint")} (effects (font (size 1.27 1.27)) (hide yes)))
			(property "Datasheet" "~" (at 0 0 0) (effects (font (size 1.27 1.27)) (hide yes)))
			(property "Description" "Schottky diode" {at("Description")} (effects (font (size 1.27 1.27)) (hide yes)))
			(symbol "D_Schottky_0_1"
				(polyline (pts (xy -1.905 0.635) (xy -1.905 1.27) (xy -1.27 1.27) (xy -1.27 -1.27) (xy -0.635 -1.27) (xy -0.635 -0.635))
					(stroke (width 0.254) (type default))
					(fill (type none))
				)
				(polyline (pts (xy 1.27 1.27) (xy 1.27 -1.27) (xy -1.27 0) (xy 1.27 1.27)) (stroke (width 0.254) (type default)) (fill (type none)))
				(polyline (pts (xy 1.27 0) (xy -1.27 0)) (stroke (width 0) (type default)) (fill (type none)))
			)
			(symbol "D_Schottky_1_1"
				(pin passive line {at("K")} (length 2.54) (name "K" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))
				(pin passive line {at("A", 180)} (length 2.54) (name "A" (effects (font (size 1.27 1.27)))) (number "2" (effects (font (size 1.27 1.27)))))
			)
			(embedded_fonts no)
		)
"""

    def instance(self, instance):
        ref = instance.ref()
        if ref is None:
            ref = "D"
        value = instance.value()
        if value is None:
            value = "D_Schottky"

        footprint = instance.footprint()
        if footprint is None:
            footprint = ""

        def at(s=None, a=0):
            return str_at( instance.pos(s), a)
        return f"""
	(symbol
		(lib_id "Device:D_Schottky")
		{at()}
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(fields_autoplaced yes)
		(property "Reference" "{ref}" {at('Reference')} (effects (font (size 1.27 1.27))))
		(property "Value" "{value}" {at('Value')} (effects (font (size 1.27 1.27))))
		(property "Footprint" "{footprint}" {at('Footprint')} (effects (font (size 1.27 1.27)) (hide yes)))
		(property "Datasheet" "~" {at('Datasheet')} (effects (font (size 1.27 1.27)) (hide yes)))
		(property "Description" "Schottky diode" {at('Description')} (effects (font (size 1.27 1.27)) (hide yes)))
		(pin "2")
		(pin "1")
	)
"""

class LEDSymbol(Symbol):
    def __init__(self, name):
        anchors = {
            "1":           ( -3.81, 0     ),
            "2":           (  3.81, 0     ),
            "K":           ( -3.81, 0     ),
            "A":           (  3.81, 0     ),
            "Reference":   (  0,    2.54  ),
            "Value":       (  0,   -2.54  ),
            "Footprint":   (  0,    0     ),
            "Datasheet":   (  0,    0     ),
            "Description": (  0,    0     ),
        }
        super().__init__(name, anchors)

    def definition(self):
        def at(s, a=0):
            return str_at( yflip(self.pos(s)), a)
        return f"""
        (symbol "{self.name()}"
            (pin_numbers (hide yes))
            (pin_names (offset 1.016) (hide yes))
            (exclude_from_sim no)
            (in_bom yes)
            (on_board yes)
			(property "Reference" "D" {at("Reference")} (effects (font (size 1.27 1.27))))
			(property "Value" "LED" {at("Value")} (effects (font (size 1.27 1.27))))
			(property "Footprint" "" {at("Footprint")} (effects (font (size 1.27 1.27)) (hide yes)))
			(property "Datasheet" "~" (at 0 0 0) (effects (font (size 1.27 1.27)) (hide yes)))
			(property "Description" "Light emitting diode" {at("Description")} (effects (font (size 1.27 1.27)) (hide yes)))

            (symbol "LED_0_1" 
                (polyline (pts (xy -3.048 -0.762) (xy -4.572 -2.286) (xy -3.81 -2.286) (xy -4.572 -2.286) (xy -4.572 -1.524)) (stroke (width 0) (type default)) (fill (type none)))
                (polyline (pts (xy -1.778 -0.762) (xy -3.302 -2.286) (xy -2.54 -2.286) (xy -3.302 -2.286) (xy -3.302 -1.524)) (stroke (width 0) (type default)) (fill (type none)))
                (polyline (pts (xy -1.27 0) (xy 1.27 0)) (stroke (width 0) (type default)) (fill (type none)))
                (polyline (pts (xy -1.27 -1.27) (xy -1.27 1.27)) (stroke (width 0.254) (type default)) (fill (type none)))
                (polyline (pts (xy 1.27 -1.27) (xy 1.27 1.27) (xy -1.27 0) (xy 1.27 -1.27)) (stroke (width 0.254) (type default)) (fill (type none)))
            )
            (symbol "LED_1_1" 
				(pin passive line {at("K")} (length 2.54) (name "K" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))
				(pin passive line {at("A", 180)} (length 2.54) (name "A" (effects (font (size 1.27 1.27)))) (number "2" (effects (font (size 1.27 1.27)))))
            )
            (embedded_fonts no)
        )
"""

    def instance(self, instance):
        ref = instance.ref()
        if ref is None:
            ref = "D"
        value = instance.value()
        if value is None:
            value = "D_Schottky"

        footprint = instance.footprint()
        if footprint is None:
            footprint = ""

        def at(s=None, a=0):
            return str_at( instance.pos(s), a)
        return f"""
	(symbol
		(lib_id "{self.name()}")
		{at()}
		(unit 1)
		(exclude_from_sim no)
		(in_bom yes)
		(on_board yes)
		(dnp no)
		(fields_autoplaced yes)
		(property "Reference" "{ref}" {at('Reference')} (effects (font (size 1.27 1.27))))
		(property "Value" "{value}" {at('Value')} (effects (font (size 1.27 1.27))))
		(property "Footprint" "{footprint}" {at('Footprint')} (effects (font (size 1.27 1.27)) (hide yes)))
		(property "Datasheet" "~" {at('Datasheet')} (effects (font (size 1.27 1.27)) (hide yes)))
		(property "Description" "Light emitting diode" {at('Description')} (effects (font (size 1.27 1.27)) (hide yes)))
		(pin "2")
		(pin "1")
	)
"""


class TLC5916Symbol(Symbol):
    def __init__(self, name):
        anchors = {
            "Reference": (-7.62, 13.97),
            "Value": (-7.62, -16.51),
            "Footprint": (0, 0),
            "Datasheet": (1.27, 0),
            "Description": (0, 0),
 
            "SDI": ( -10.16, -10.16 ),
            "CLK": ( -10.16, -7.62 ),
            "LE": ( -10.16, -5.08 ),
            "OE": ( -10.16, 0 ),
            "REXT": ( -10.16, 7.62 ),
            "VDD": ( 0, -15.24 ),
            "GND": ( 0, 17.78 ),
            "OUT0": ( 10.16, -10.16 ),
            "OUT1": ( 10.16, -7.62 ),
            "OUT2": ( 10.16, -5.08 ),
            "OUT3": ( 10.16, -2.54 ),
            "OUT4": ( 10.16, 0 ),
            "OUT5": ( 10.16, 2.54 ),
            "OUT6": ( 10.16, 5.08 ),
            "OUT7": ( 10.16, 7.62 ),
            "SDO": ( 10.16, 12.7 ),
        }
        super().__init__(name, anchors)


    def definition(self):
        def at(s, a=0):
            return str_at(yflip(self.pos(s)), a)
        return f"""(symbol "{self.name()}"
            (exclude_from_sim no)
            (in_bom yes)
            (on_board yes)
            (property "Reference" "U" (at -7.62 13.97 0) (effects (font (size 1.27 1.27))))
            (property "Value" "TLC5916" (at -7.62 -16.51 0) (effects (font (size 1.27 1.27))))
            (property "Footprint" "" (at 0 0 0) (effects (font (size 1.27 1.27)) (hide yes)))
            (property "Datasheet" "https://www.ti.com/lit/ds/symlink/tlc5916.pdf" (at 1.27 0 0) (effects (font (size 1.27 1.27)) (hide yes)))
            (property "Description" "8-Channel Constant-Current LED Sink Driver" (at 0 0 0) (effects (font (size 1.27 1.27)) (hide yes)))
            (property "ki_keywords" "LED Constant-Current Driver" (at 0 0 0) (effects (font (size 1.27 1.27)) (hide yes)))
            (property "ki_fp_filters" "DIP*W7.62mm* SOIC*3.9x9.9mm*P1.27mm* TSSOP*4.4x5mm*P0.65mm*" (at 0 0 0) (effects (font (size 1.27 1.27)) (hide yes)))
            (symbol "TLC5916_1_0"
        (pin input line (at -10.16 10.16 0) (length 2.54) (name "SDI" (effects (font (size 1.27 1.27)))) (number "2" (effects (font (size 1.27 1.27)))))
        (pin input line (at -10.16 7.62 0) (length 2.54) (name "CLK" (effects (font (size 1.27 1.27)))) (number "3" (effects (font (size 1.27 1.27)))))
        (pin passive line (at -10.16 5.08 0) (length 2.54) (name "LE(ED1)" (effects (font (size 1.27 1.27)))) (number "4" (effects (font (size 1.27 1.27)))))
        (pin passive line (at -10.16 0 0) (length 2.54) (name "~{{OE}}(ED2)" (effects (font (size 1.27 1.27)))) (number "13" (effects (font (size 1.27 1.27)))))
        (pin input line (at -10.16 -7.62 0) (length 2.54) (name "R-EXT" (effects (font (size 1.27 1.27)))) (number "15" (effects (font (size 1.27 1.27)))))
        (pin power_in line (at 0 15.24 270) (length 2.54) (name "VDD" (effects (font (size 1.27 1.27)))) (number "16" (effects (font (size 1.27 1.27)))))
        (pin power_in line (at 0 -17.78 90) (length 2.54) (name "GND" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))
        (pin open_collector line (at 10.16 10.16 180) (length 2.54) (name "~{{OUT0}}" (effects (font (size 1.27 1.27)))) (number "5" (effects (font (size 1.27 1.27)))))
        (pin open_collector line (at 10.16 7.62 180) (length 2.54) (name "~{{OUT1}}" (effects (font (size 1.27 1.27)))) (number "6" (effects (font (size 1.27 1.27)))))
        (pin open_collector line (at 10.16 5.08 180) (length 2.54) (name "~{{OUT2}}" (effects (font (size 1.27 1.27)))) (number "7" (effects (font (size 1.27 1.27)))))
        (pin open_collector line (at 10.16 2.54 180) (length 2.54) (name "~{{OUT3}}" (effects (font (size 1.27 1.27)))) (number "8" (effects (font (size 1.27 1.27)))))
        (pin open_collector line (at 10.16 0 180) (length 2.54) (name "~{{OUT4}}" (effects (font (size 1.27 1.27)))) (number "9" (effects (font (size 1.27 1.27)))))
        (pin open_collector line (at 10.16 -2.54 180) (length 2.54) (name "~{{OUT5}}" (effects (font (size 1.27 1.27)))) (number "10" (effects (font (size 1.27 1.27)))))
        (pin open_collector line (at 10.16 -5.08 180) (length 2.54) (name "~{{OUT6}}" (effects (font (size 1.27 1.27)))) (number "11" (effects (font (size 1.27 1.27)))))
        (pin open_collector line (at 10.16 -7.62 180) (length 2.54) (name "~{{OUT7}}" (effects (font (size 1.27 1.27)))) (number "12" (effects (font (size 1.27 1.27)))))
        (pin output line (at 10.16 -12.7 180) (length 2.54) (name "SDO" (effects (font (size 1.27 1.27)))) (number "14" (effects (font (size 1.27 1.27)))))
            )
            (symbol "TLC5916_1_1"
                (rectangle (start -7.62 12.7) (end 7.62 -15.24) (stroke (width 0.254) (type default)) (fill (type background)))
            )
            (embedded_fonts no)
        )
"""

    def instance(self, instance):
        ref = instance.ref()
        if ref is None:
            ref = "U"
        value = instance.value()
        if value is None:
            value = "TLC5916"

        footprint = instance.footprint()
        if footprint is None:
            footprint = ""

        def at(s=None, a=0):
            return str_at(instance.pos(s), a)
        return f"""
    (symbol
        (lib_id "{self.name()}")
        {at()}
        (unit 1)
        (exclude_from_sim no)
        (in_bom yes)
        (on_board yes)
        (dnp no)
        (fields_autoplaced yes)
        (property "Reference" "{ref}" {at("Reference")} (effects (font (size 1.27 1.27)) (justify left)))
        (property "Value" "{value}" {at("Value")} (effects (font (size 1.27 1.27)) (justify left)))
        (property "Footprint" "{footprint}" {at("Footprint")} (effects (font (size 1.27 1.27)) (hide yes)))
        (property "Datasheet" "https://www.ti.com/lit/ds/symlink/tlc5916.pdf" {at("Datasheet")} (effects (font (size 1.27 1.27)) (hide yes)))
        (property "Description" "8-Channel Constant-Current LED Sink Driver" {at("Description")} (effects (font (size 1.27 1.27)) (hide yes)))
	)
"""


class ResistorSymbol(Symbol):
    def __init__(self, name):
        anchors = {
            "1": ( 0, -3.81 ),
            "2": ( 0, 3.81 ),
            "Reference": ( 2.032, 0 ),
            "Value": ( 0, -0 ),
            "Footprint": ( -1.778, 0 ),
            "Datasheet": ( 0, 0 ),
            "Description": ( 0, 0 ),
 
        }
        super().__init__(name, anchors)


    def definition(self):
        def at(s, a=0):
            return str_at(yflip(self.pos(s)), a)
        return f"""
        (symbol "{self.name()}"
            (pin_numbers (hide yes))
            (pin_names (offset 0))
            (exclude_from_sim no)
            (in_bom yes)
            (on_board yes)
            (property "Reference" "R" {at("Reference", 90)} (effects (font (size 1.27 1.27))))
            (property "Value" "R" {at("Value", 90)} (effects (font (size 1.27 1.27))))
            (property "Footprint" "" {at("Footprint", 90)} (effects (font (size 1.27 1.27)) (hide yes)))
            (property "Datasheet" "~" {at("Datasheet", 0)} (effects (font (size 1.27 1.27)) (hide yes)))
            (property "Description" "Resistor" {at("Description", 0)} (effects (font (size 1.27 1.27)) (hide yes)))
            (property "ki_keywords" "R res resistor" (at 0 0 0) (effects (font (size 1.27 1.27)) (hide yes)))
            (property "ki_fp_filters" "R_*" (at 0 0 0) (effects (font (size 1.27 1.27)) (hide yes)))
            (symbol "R_0_1"
                (rectangle (start -1.016 -2.54) (end 1.016 2.54) (stroke (width 0.254) (type default)) (fill (type none)))
            )
            (symbol "R_1_1"
                (pin passive line (at 0 3.81 270) (length 1.27) (name "~" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))
                (pin passive line (at 0 -3.81 90) (length 1.27) (name "~" (effects (font (size 1.27 1.27)))) (number "2" (effects (font (size 1.27 1.27)))))
            )
            (embedded_fonts no)
        )
"""

    def instance(self, instance):
        ref = instance.ref()
        if ref is None:
            ref = "R"
        value = instance.value()
        if value is None:
            value = "R"

        footprint = instance.footprint()
        if footprint is None:
            footprint = ""

        def at(s=None, a=0):
            return str_at(instance.pos(s), a)
        return f"""
   (symbol
        (lib_id "{self.name()}")
        {at()}
        (unit 1)
        (exclude_from_sim no)
        (in_bom yes)
        (on_board yes)
        (dnp no)
        (property "Reference" "{ref}" {at("Reference")} (effects (font (size 1.27 1.27))))
        (property "Value" "{value}" {at("Value")} (effects (font (size 1.27 1.27))))
        (property "Footprint" "{footprint}" {at("Footprint")} (effects (font (size 1.27 1.27)) (hide yes)))
        (property "Datasheet" "" {at("Datasheet")} (effects (font (size 1.27 1.27)) (hide yes)))
        (property "Description" "" {at("Description")} (effects (font (size 1.27 1.27))))
        (pin "1")
        (pin "2")
    )
"""

class CapacitorSymbol(Symbol):
    def __init__(self, name):
        anchors = {
            "1": ( 0, -3.81 ),
            "2": ( 0, 3.81 ),
            "Reference": ( 0.635, -2.54 ),
            "Value": ( 0.635, 2.54 ),
            "Footprint": ( 0.9652, 3.81 ),
            "Datasheet": ( 0, 0 ),
            "Description": ( 0, 0 ),
 
        }
        super().__init__(name, anchors)


    def definition(self):
        def at(s, a=0):
            return str_at(yflip(self.pos(s)), a)
        return f"""
        (symbol "{self.name()}"
            (pin_numbers (hide yes))
            (pin_names (offset 0))
            (exclude_from_sim no)
            (in_bom yes)
            (on_board yes)
            (property "Reference" "C" {at("Reference")} (effects (font (size 1.27 1.27))))
            (property "Value" "C" {at("Value")} (effects (font (size 1.27 1.27))))
            (property "Footprint" "" {at("Footprint")} (effects (font (size 1.27 1.27)) (hide yes)))
            (property "Datasheet" "~" {at("Datasheet")} (effects (font (size 1.27 1.27)) (hide yes)))
            (property "Description" "Resistor" {at("Description", 0)} (effects (font (size 1.27 1.27)) (hide yes)))
            (property "ki_keywords" "C cap capacitor" (at 0 0 0) (effects (font (size 1.27 1.27)) (hide yes)))
            (property "ki_fp_filters" "C_*" (at 0 0 0) (effects (font (size 1.27 1.27)) (hide yes)))
            (symbol "C_0_1" 
                (polyline (pts (xy -2.032 0.762) (xy 2.032 0.762)) (stroke (width 0.508) (type default)) (fill (type none)))
                (polyline (pts (xy -2.032 -0.762) (xy 2.032 -0.762)) (stroke (width 0.508) (type default)) (fill (type none)))
            )
            (symbol "C_1_1"
                (pin passive line (at 0 3.81 270) (length 2.794) (name "~" (effects (font (size 1.27 1.27)))) (number "1" (effects (font (size 1.27 1.27)))))
                (pin passive line (at 0 -3.81 90) (length 2.794) (name "~" (effects (font (size 1.27 1.27)))) (number "2" (effects (font (size 1.27 1.27)))))
            )
            (embedded_fonts no)
        )
"""

    def instance(self, instance):
        ref = instance.ref()
        if ref is None:
            ref = "C"
        value = instance.value()
        if value is None:
            value = "C"

        footprint = instance.footprint()
        if footprint is None:
            footprint = ""

        def at(s=None, a=0):
            return str_at(instance.pos(s), a)
        return f"""
   (symbol
        (lib_id "{self.name()}")
        {at()}
        (unit 1)
        (exclude_from_sim no)
        (in_bom yes)
        (on_board yes)
        (dnp no)
        (property "Reference" "{ref}" {at("Reference")} (effects (font (size 1.27 1.27))))
        (property "Value" "{value}" {at("Value")} (effects (font (size 1.27 1.27))))
        (property "Footprint" "{footprint}" {at("Footprint")} (effects (font (size 1.27 1.27)) (hide yes)))
        (property "Datasheet" "" {at("Datasheet")} (effects (font (size 1.27 1.27)) (hide yes)))
        (property "Description" "" {at("Description")} (effects (font (size 1.27 1.27))))
        (pin "1")
        (pin "2")
    )
"""



_symbols = {
    'Switch:SW_Push': SwitchSymbol('Switch:SW_Push'),
    'Device:D_Schottky': DiodeSymbol('Device:D_Schottky'),
    'Device:LED': LEDSymbol('Device:LED'),
    'Driver_LED:TLC5916': TLC5916Symbol('Driver_LED:TLC5916'),
    'Device:R': ResistorSymbol('Device:R'),
    'Device:C': CapacitorSymbol('Device:C'),
}

class Component:

    def __init__(self, at=(0,0), ref=None, value=None, footprint=None, relative_to=None, symbol_name=None):
        self._ref = ref
        self._value = value
        self._symbol_name = symbol_name
        self._footprint = footprint
        if symbol_name is None:
            self._symbol = None
            self.at = at
        else:
            self._symbol = _symbols[symbol_name]
            self.at = sub_pos(at, self._symbol.pos(relative_to))

    def footprint(self):
        return self._footprint

    def ref(self):
        return self._ref

    def value(self):
        return self._value

    def pos(self, element=None):
        if element is None:
            return self.at
        else:
            return add_pos(self.at, self._symbol.pos(element))

    def symbol(self):
        return self._symbol

    def symbol_name(self):
        return self._symbol_name

    def __str__(self):
        return self._symbol.instance(self)

class Wire(Component):
    def __init__(self, start=(0,0), end=(0,0)):
        super().__init__(start, None, None, None, None)
        self.to = end

    def __str__(self):
        return f"(wire (pts {str_xy(self.at)} {str_xy(self.to)}) (stroke (width 0) (type default)))"

class HierarchicalLabel(Component):
    def __init__(self, at=(0,0), ref=None, shape=None, rot=0):
        super().__init__(at, ref)
        if shape not in ("input", "output", "bidirectional"):
            shape = "input"
        self._shape = shape
        self._rot = rot

    def __str__(self):
        justify = "left"
        if 90 < (self._rot % 360) <= 270:
            justify = "right"
        return f"""(hierarchical_label "{self.ref()}" (shape {self._shape}) {str_at(self.at, self._rot)} (effects (font (size 1.27 1.27)) (justify {justify}) ) )"""


class Label(Component):
    def __init__(self, at=(0,0), ref=None, shape=None, rot=0):
        super().__init__(at, ref)
        self._rot = rot

    def __str__(self):
        justify = "left"
        if 90 < (self._rot % 360) <= 270:
             justify = "right"
        return f"""(label "{self.ref()}" {str_at(self.at, self._rot)} (effects (font (size 1.27 1.27)) (justify {justify} bottom) ) )"""

class Junction(Component):
    def __init__(self, at=(0,0)):
        super().__init__(at)

    def __str__(self):
        return f"(junction {str_point(self.at)} (diameter 0) (color 0 0 0 0))"

class Switch(Component):
    def __init__(self, at=(0,0), ref=None, value=None, footprint=None, relative_to=None):
        super().__init__(at, ref, value, footprint, relative_to, 'Switch:SW_Push')

class Diode(Component):
    def __init__(self, at=(0,0), ref=None, value=None, footprint=None, relative_to=None):
        super().__init__(at, ref, value, footprint, relative_to, 'Device:D_Schottky')

class LED(Component):
    def __init__(self, at=(0,0), ref=None, value=None, footprint=None, relative_to=None):
        super().__init__(at, ref, value, footprint, relative_to, 'Device:LED')

class Resistor(Component):
    def __init__(self, at=(0,0), ref=None, value=None, footprint=None, relative_to=None):
        super().__init__(at, ref, value, footprint, relative_to, 'Device:R')

class Capacitor(Component):
    def __init__(self, at=(0,0), ref=None, value=None, footprint=None, relative_to=None):
        super().__init__(at, ref, value, footprint, relative_to, 'Device:C')

class TLC5916(Component):
    def __init__(self, at=(0,0), ref=None, value=None, footprint=None, relative_to=None):
        super().__init__(at, ref, value, footprint, relative_to, 'Driver_LED:TLC5916')

class Schematic:

    def __init__(self, paper="A4"):
        self.components = []
        self.paper = paper

    def add(self, component):
        self.components.append(component)

    def str_lib_symbols(self):
        lib = {}
        for c in self.components:
            if c.symbol_name() is not None:
                lib[c.symbol_name()] = c.symbol().definition()

        return '\n'.join(str(lib[s]) for s in sorted(lib))

    def str_components(self):
        return '\n'.join(str(c) for c in self.components)

    def __str__(self):
        return f"""(kicad_sch
	(version 20250114)
	(generator "eeschema")
	(generator_version "9.0")
	(paper "{self.paper}")
	(lib_symbols
		{self.str_lib_symbols()}
	)
	{self.str_components()}
)
"""

