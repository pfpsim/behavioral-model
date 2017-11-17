#!/bin/sh
set -e
#wget http://download.nanomsg.org/nanomsg-0.5-beta.tar.gz
 wget -O nanomsg-0.5-beta.tar.gz https://github.com/nanomsg/nanomsg/archive/0.5-beta.tar.gz
tar -xzvf nanomsg-0.5-beta.tar.gz
cd nanomsg-0.5-beta
./autogen.sh
./configure
# ./configure --prefix=$HOME/nanomsg
make && sudo make install
cd ..
