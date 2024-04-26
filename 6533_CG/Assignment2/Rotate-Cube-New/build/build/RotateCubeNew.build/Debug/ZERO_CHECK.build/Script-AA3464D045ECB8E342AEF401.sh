#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd /Users/ahhyunmoon/NYU_Class/Rotate-Cube-New/build
  make -f /Users/ahhyunmoon/NYU_Class/Rotate-Cube-New/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd /Users/ahhyunmoon/NYU_Class/Rotate-Cube-New/build
  make -f /Users/ahhyunmoon/NYU_Class/Rotate-Cube-New/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd /Users/ahhyunmoon/NYU_Class/Rotate-Cube-New/build
  make -f /Users/ahhyunmoon/NYU_Class/Rotate-Cube-New/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd /Users/ahhyunmoon/NYU_Class/Rotate-Cube-New/build
  make -f /Users/ahhyunmoon/NYU_Class/Rotate-Cube-New/build/CMakeScripts/ReRunCMake.make
fi

