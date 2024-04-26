#!/bin/sh
set -e
if test "$CONFIGURATION" = "Debug"; then :
  cd "/Users/ahhyunmoon/NYU_Spring2023/6533_Interactive CG/Example/build"
  make -f /Users/ahhyunmoon/NYU_Spring2023/6533_Interactive\ CG/Example/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "Release"; then :
  cd "/Users/ahhyunmoon/NYU_Spring2023/6533_Interactive CG/Example/build"
  make -f /Users/ahhyunmoon/NYU_Spring2023/6533_Interactive\ CG/Example/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "MinSizeRel"; then :
  cd "/Users/ahhyunmoon/NYU_Spring2023/6533_Interactive CG/Example/build"
  make -f /Users/ahhyunmoon/NYU_Spring2023/6533_Interactive\ CG/Example/build/CMakeScripts/ReRunCMake.make
fi
if test "$CONFIGURATION" = "RelWithDebInfo"; then :
  cd "/Users/ahhyunmoon/NYU_Spring2023/6533_Interactive CG/Example/build"
  make -f /Users/ahhyunmoon/NYU_Spring2023/6533_Interactive\ CG/Example/build/CMakeScripts/ReRunCMake.make
fi

