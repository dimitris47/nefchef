# NefChef

This is a cookbook creator with embedded calories calculator.
As of this time, the menus and items are in Greek. Translators welcome!

HOW TO INSTALL:

- WINDOWS SYSTEMS:

Run the latest <code>.exe</code> file.
If after the installation the program doesn't start, install <code>vc_redist.exe</code> from the installation folder.


- LINUX SYSTEMS:

a. Compile from source:

1. Donwload and extract archive from releases
2. Open extracted directory with Terminal and run:
   <code>qmake nefchef.pro CONFIG+=release && make && sudo make install && make clean</code>
   
You might need to install <code>libqt5gui5</code>, <code>g++</code> and <code>make</code> packages to your Linux system before compiling.


b. Debian systems:

1. Download the latest <code>.deb</code> file from releases.
2. Open downloads folder with Terminal and run:
<code>sudo dpkg -i nefchef-$VERSION_NUMBER.deb</code>

You might need to install the <code>libqt5gui5</code> package before running the <code>.deb</code> file.


- MAC SYSTEMS:

Download and run the <code>nefchef.app.zip</code> file. You need to have XCode and Qt installed on your system beforehand.
