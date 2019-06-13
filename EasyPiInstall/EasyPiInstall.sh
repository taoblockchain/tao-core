#!/bin/bash
cd /home/pi
sudo sed -i 's/100/4096/' /etc/dphys-swapfile
sudo /etc/init.d/dphys-swapfile stop
sudo /etc/init.d/dphys-swapfile start
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install build-essential libtool automake autotools-dev autoconf pkg-config libgmp3-dev libevent-dev bsdmainutils  <<-EOF
yes
EOF
sudo apt-get install libboost-system-dev libboost-filesystem-dev libboost-chrono-dev libboost-program-options-dev libboost-test-dev libboost-thread-dev  <<-EOF
yes
EOF
sudo apt-get install libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler qt5-default  <<-EOF
yes
EOF
sudo apt remove libssl-dev
sudo apt-get install software-properties-common libssl1.0-dev libminiupnpc-dev git  <<-EOF
yes
EOF
git clone https://github.com/taoblockchain/tao-core.git
cd tao-core/
TRANSFER_ROOT=$(pwd)
BDB_PREFIX="${TRANSFER_ROOT}/db4"
mkdir -p $BDB_PREFIX
wget 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
tar -xzf db-4.8.30.NC.tar.gz
cd db-4.8.30.NC/build_unix/
../dist/configure --enable-cxx --disable-shared --with-pic --prefix=$BDB_PREFIX
make install
cd $TRANSFER_ROOT
qmake "BDB_LIB_PATH+=db4/lib" "BDB_INCLUDE_PATH+=db4/include"
make
./tao-qt