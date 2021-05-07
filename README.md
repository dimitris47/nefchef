# NefChef

This is a cookbook creator with embedded calories calculator.
As of this time, the menus and items are in Greek. Translators welcome!

HOW TO INSTALL:

- WINDOWS SYSTEMS:

Run the latest <code>.exe</code> file.
If after the installation the program doesn't start, install <code>vc_redist.exe</code> from the installation folder.


- UNIX SYSTEMS:

a. from source:

1. Donwload the code or clone the repo
2. Open extracted directory in terminal and run:
   <code>qmake nefchef.pro CONFIG+=release && make && sudo make install && make clean</code>
   
You might need to install <code>libqt5gui5</code>, <code>g++</code> and <code>make</code> packages to your system before compiling.


b. Debian & Debian-based systems:

Download the latest <code>.deb</code> file from releases and run it with your package manager.

c. RPM-based systems:

Download the latest <code>.rmp</code> file and run it with your package manager.

You might need to install the <code>libqt5gui5</code> package before running the <code>.deb</code> file.
