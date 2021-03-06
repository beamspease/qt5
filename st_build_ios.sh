#!/bin/bash

set -e
version=`$TEAMCITY_GIT_PATH rev-parse HEAD`
install_dir=$PWD/$version

./configure -prefix $install_dir -debug-and-release -opensource -confirm-license -xplatform macx-ios-clang -nomake tests -nomake examples -skip qttranslations -skip qtwebkit -no-warnings-are-errors

make -j8

# Static build, don't need symbols until after link
# python ./st_gen_and_upload_symbols.py --os ios

make install

tar cvzf qt-$version-ios.tar.gz ./$version
# Delete the version folder, since the way teamcity cleans things having a folder that's
# also a revision is bad
rm -rf ./$version
