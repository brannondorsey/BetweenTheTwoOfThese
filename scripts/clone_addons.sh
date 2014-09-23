#!/bin/bash
# All scripts must be run from inside the scripts/ dir

if [ -z $1 ]; then
    PREFIX="https://github.com/"
else
    PREFIX="git@github.com:"
fi

PWD=`pwd`

OF_ROOT=../../..

ADDONS_PATH=${OF_ROOT}/addons

cd $ADDONS_PATH

# ofxMostPixelsEver
echo "Cloning ofxMostPixelsEver..."
git clone ${PREFIX}brannondorsey/ofxMostPixelsEver.git &> /dev/null
cd ofxMostPixelsEver
git pull
cd -

#ofxDOF
echo "Cloning ofxDOF..."
git clone ${PREFIX}obviousjim/ofxDOF.git &> /dev/null
cd ofxDOF
git pull
cd -

#ofxUI
echo "Cloning ofxUI..."
git clone ${PREFIX}rezaali/ofxUI &> /dev/null
cd ofxUI
git pull
cd -

#ofxCv
echo "Cloning ofxCv..."
git clone ${PREFIX}kylemcdonald/ofxCv &> /dev/null
cd ofxCv
git pull
cd -

