# chores

Chores is a very simple and also very lightweight to-do application.

HOW TO INSTALL:

- WINDOWS:

Run the latest `.exe` file.
If after the installation the program doesn't start, install `vc_redist.exe` from the installation folder.

- MAC:

Download and open the latest `.dmg` image file.

- LINUX:

a. from source:

1. Donwload the code or clone the repo
2. Open extracted directory in terminal and run:
   `qmake nefchef.pro CONFIG+=release && make && sudo make install && make clean`
   
You might need to install `libqt5gui5`, `g++` and `make` packages to your system before compiling.

b. Debian & Debian-based systems:

Download the latest `.deb` file from releases and run it with your package manager.

c. RPM-based systems:

Download the latest `.rpm` file and run it with your package manager.

You might need to have `libqt5gui5` installed before running the binary.
