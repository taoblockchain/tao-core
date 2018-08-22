# Welcome to the simple unix build guide.
Don't be intimidated, this is really easy!\
You don't need to understand how to use linux programs, copying and pasting the commands into your terminal should do the job.\
I recommend you to download this document onto your computer so you can copy-paste the commands into your terminal rather than typing them by hand, don't want to make any mistakes!\
Linux distributions this guide has been tested with: (as of 22/08/2018)

    Ubuntu 18.04.1 (Ubuntu Bionic)
    Debian 7 (Debian Stretch)
    Raspbian Stretch (raspberry pi)
Feeling adventurous? Want to install this on another distro or a different ubuntu version?\
The instructions should more or less work for other versions as well, but there's no guarantee, if you need any help doing this, ask someone on the XTO/AltMarket discord!
## Your terminal
The terminal is an application that can be found on all linux distros listed below which allows you to run commands.\
The user interface for these distributions are easy to use, so it won't be much trouble finding the search bar and typing in "terminal" or opening the terminal from a menu.\
The icon of a terminal generally looks something like [this](https://upload.wikimedia.org/wikipedia/commons/thumb/b/b3/Terminalicon2.png/240px-Terminalicon2.png)
## If you ARE using a Raspberry Pi, read this:
This will change your swap file (on-disk RAM) to be 4GB in size, useful for devices with a low amount of RAM.\
Run these commands:

    sudo sed -i 's/100/4096/' /etc/dphys-swapfile
    sudo /etc/init.d/dphys-swapfile stop
    sudo /etc/init.d/dphys-swapfile start
Also, I would advise you to make sure you are using a class 10 microSD card, slow SD cards can make your pi very slow, especially while performing the commands in this guide.
# Let's begin
## Update packages
First we update our packages by running these two commands:

    sudo apt-get update
    sudo apt-get upgrade
## Install stuff we need to compile
Install the toolchain (compiling tools & libs):

    sudo apt-get install build-essential libtool automake autotools-dev autoconf pkg-config libgmp3-dev libevent-dev bsdmainutils
Install Boost:

    sudo apt-get install libboost-system-dev libboost-filesystem-dev libboost-chrono-dev libboost-program-options-dev libboost-test-dev libboost-thread-dev
Install QT (gui stuff)

    sudo apt-get install libqt5gui5 libqt5core5a libqt5dbus5 qttools5-dev qttools5-dev-tools libprotobuf-dev protobuf-compiler qt5-default
Remove a package that may conflict with another we are about to install:\
(if it says the package is not installed, that's ok, continue regardless of the result)
    
    sudo apt remove libssl-dev
Install a few extra packages:

    sudo apt-get install software-properties-common libssl1.0-dev libminiupnpc-dev git
## Download the Tao github repo
Simple, just run this command:

    git clone https://github.com/taoblockchain/tao-core.git
Now if you run the `ls` command, you'll see a `tao-core` directory, don't do anything yet though. 

## Building Berkeley DB
Now just run these commands and Berkeley DB will be installed.

    cd tao-core/
    TRANSFER_ROOT=$(pwd)
    BDB_PREFIX="${TRANSFER_ROOT}/db4"
    mkdir -p $BDB_PREFIX
    wget 'http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz'
    tar -xzf db-4.8.30.NC.tar.gz
    cd db-4.8.30.NC/build_unix/
    ../dist/configure --enable-cxx --disable-shared --with-pic --prefix=$BDB_PREFIX
    make install
After running `make install` you'll see a lot of text fly past your screen, that's perfectly normal.
## Compiling the wallet
Run these commands to compile the wallet:

    cd $TRANSFER_ROOT
    qmake "BDB_LIB_PATH+=db4/lib" "BDB_INCLUDE_PATH+=db4/include"
    make
Compiling can take quite a while...
## Done!
Run the wallet by running the command:

    ./tao-qt
Easy peasy!\
Guide written by nullarmo.
