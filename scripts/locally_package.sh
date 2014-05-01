#!/bin/bash

set -u
make clean
rm -rf node_modules/
export CXX11=true
source ~/projects/mapnik-packaging/osx/MacOSX.sh
npm install --build-from-source
make test
npm install aws-sdk
export PATH=$(pwd)/lib/binding:${PATH}
npm test
./node_modules/.bin/node-pre-gyp package publish
# node v0.8.x
npm install --build-from-source --target=0.8.26
nvm use 0.8
npm test
./node_modules/.bin/node-pre-gyp package publish --target=0.8.26

