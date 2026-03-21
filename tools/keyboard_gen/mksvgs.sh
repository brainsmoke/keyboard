#!/bin/bash

for i in {top,bottom}{1,2}; do
	python3 lasercut_cover.py "$i" > "$i".svg
done
