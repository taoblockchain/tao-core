#!/bin/bash
# create multiresolution windows icon
ICON_DST=../../src/qt/res/icons/tao.ico

convert ../../src/qt/res/icons/tao-16.png ../../src/qt/res/icons/tao-32.png ../../src/qt/res/icons/tao-48.png ${ICON_DST}
