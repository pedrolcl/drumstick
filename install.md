## Basic build commands

Using a Linux terminal...

~~~
$ cd drumstick-x.x.x
$ mkdir -p build
$ cd build
$ cmake ..
    (or ccmake ..)
    (or cmake-gui ..)
    (or cmake .. -options, see below...)
~~~

For simple test/development scenarios you can also use Qmake, but the Qmake-based build system provides only minimal functionality.

~~~
$ make
    (or make VERBOSE=1)

$ sudo make install

$ sudo ldconfig
    (not needed if you only want STATIC_DRUMSTICK)
~~~

## Requirements

Minimum supported versions:

* CMake 3.14
* Qt5 >= 5.9, Qt6 >= 6.2 
  When using Qt6, Qt6Core5Compat is required for Drumstick::File
* For Linux: ALSA 1.x
* RT backends for
    * Linux: ALSA
    * macOS: CoreMIDI
    * Windows: WinMM
    * Unix: OSS
    * Synthesizers (output backends): Sonivox, FluidSynth, Apple DLS Synth
    * All: Network - ipMIDI (IPv4, IPv6): QtNetwork
* shared-mime-info 0.30
See http://freedesktop.org/wiki/Software/shared-mime-info
The utility "update-mime-database" must be executed after installing the library "drumstick-file" and the "drumstick.xml" file. This is automatically done by the cmake build system unless you defined DESTDIR. In this case, your package manager should perform it as a post-install step. 
* Doxygen 1.5
See http://www.doxygen.org
If you want to generate the HTML documentation for the libraries

If you want to generate and install the man pages, the build system can do it if you have installed in your system the following packages:

* xsltproc program.
* docbook XSLT stylesheets.

The package names depend on the Linux distribution. For Debian they are:
xsltproc, docbook-xsl and docbook-xml. For openSUSE: libxslt, docbook_4, and
docbook-xsl-stylesheets.

RealtimeKit actions are called through DBus, so it is not a direct dependency.

## Optional CMake parameters

-DSTATIC_DRUMSTICK=YES|ON|1
Build static libraries instead of a shared object

-DSTATIC_DRUMSTICK=NO|OFF|0
Build dynamic libraries (default)

-DCMAKE_BUILD_TYPE=Debug
Compile with debug flags enabled

-DCMAKE_BUILD_TYPE=Release
Compile without debug flags, and optimization enabled

-DCMAKE_CXX_FLAGS="-W -Wall"
Specify custom compilation flags

-DCMAKE_INSTALL_PREFIX=/usr/local
Specify the desired install prefix

-DUSE_DBUS=YES|ON|1  
Build DBus support, required to use RealtimeKit 

-DUSE_DBUS=NO|OFF|0  
Don't include DBus support (default)

-DBUILD_DOCS=YES|ON|1  
Build Doxygen documentation and man pages (default in Unix)

-DBUILD_DOCS=NO|OFF|0  
Don't build Doxygen documentation nor man pages

Note: the last option only creates a "doxygen" target. You still need to call the
target build if you want to generate the actual documentation.

-DBUILD_UTILS=YES|ON|1
Build utilities and example programs (default)

-DBUILD_UTILS=NO|OFF|0 
Don't build utilities and example programs

-DBUILD_TESTING=YES|ON|1
Build unit tests (default)

-DBUILD_TESTING=NO|OFF|0
Don't build unit tests

-DBUILD_FRAMEWORKS=YES|ON|1
Build macOS style frameworks (default on macOS)

-DBUILD_FRAMEWORKS=NO|OFF|0
Don't build macOS style frameworks, build Unix style dynamic libraries (.dylib)

-DBUILD_ALSA=YES|ON|1
Build Drumstick::ALSA (default on Linux)

-DBUILD_ALSA=NO|OFF|0
Don't build Drumstick::ALSA (default on non Linux Operating Systems)

-DBUILD_FILE=YES|ON|1
Build Drumstick::File (default)

-DBUILD_FILE=NO|OFF|0
Don't build Drumstick::File

-DBUILD_RT=YES|ON|1
Build Drumstick::RT (default)

-DBUILD_RT=NO|OFF|0
Don't build Drumstick::RT

-DBUILD_WIDGETS=YES|ON|1
Build Drumstick::Widgets (default)

-DBUILD_WIDGETS=NO|OFF|0
Don't build Drumstick::Widgets

-DUSE_NETWORK=YES|ON|1
Build the ipMIDI Network RT backend (default)
Triggers a fatal error at configuration time if libQtNetwork is not available.

-DUSE_NETWORK=NO|OFF|0
Don't build the ipMIDI Network RT backend

-DUSE_PULSEAUDIO=YES|ON|1
Build the SonivoxEAS RT output backend (default)
Triggers a fatal error at configuration time if libpulse-simple is not available.

-DUSE_PULSEAUDIO=NO|OFF|0
Don't build the SonivoxEAS RT output backend

Note: FluidSynth has also a PulseAudio driver, which is independent of the last option

-DUSE_FLUIDSYNTH=YES|ON|1
Build the FluidSynth RT output backend (default)
Triggers a fatal error at configuration time if libfluidsynth is not available.

-DUSE_FLUIDSYNTH=NO|OFF|0
Don't build the FluidSynth RT output backend

-DUSE_QT=5|6
Choose which Qt major version (5 or 6) to prefer. By default (if not set) uses whatever is found.
notes: Qt6 support is still experimental.
When using Qt6, the Core5Compat additional library is required for Drumstick::File

-DUSE_SONIVOX=YES|ON|1
Build Sonivox RT backend (default)
Triggers a fatal error at configuration time if libsonivox is not available.

-DUSE_SONIVOX==NO|OFF|0
Don't build SonivoxEAS RT output backend
