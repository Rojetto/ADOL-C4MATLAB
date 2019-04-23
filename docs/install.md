Installation
============


- [Requirements](#requirements)
- [Install ADOL-C](#install-adol-c)
  - [Linux](#linux)
  - [Windows](#windows)
- [Install the toolbox](#install-the-toolbox)


## Requirements

The ADOL-C4MAT toolbox has the following requirements

- for Linux
  - GNU toolchain
  - ADOL-C
  - Octave (Matlab not tested yet)
- for Windows
  - MSYS and MinGW (required to build ADOL-C).
    - 32bit Matlab or Octave:
      It is recommended to use the [Graphical User Interface Installer](https://sourceforge.net/projects/mingw/).
        There the following packages have to be chosen:
      - mingw-developer-toolkit
      - mingw32-base
      - mingw32-gcc-g++
      - mingw32-gcc-objc
      - msys-base
    - 64bit Matlab or Octave:
      You should use [MSYS2](https://www.msys2.org/).
      Run 'MSYS2 MinGW 64-bit' and execute the following commands to set up the build environment:
      ```
      pacman -Syu
      pacman -Su
      pacman -S mingw-w64-x86_64-toolchain autoconf make libtool automake
      ```
  - ColPack (optional)
  - ADOL-C
  - either
    - Matlab >= 2015b and
    - a suitable compiler supporting C++11, e.g., Microsoft Visual Studio (Visual C++) 2015 or newer [must be supported by the specific Matlab version](https://de.mathworks.com/support/sysreq/previous_releases.html), be aware that the 64-bit compiler only can be used with a 64-bit version of Matlab) - the MinGW-w64 Compiler is not supported yet
  - or
    - Octave (already comes with the MinGW compiler)


## Install ADOL-C

ADOL-C is available from the COIN-OR initiative, see https://projects.coin-or.org/ADOL-C. Alternatively ADOL-C can be cloned from [GitLab](https://gitlab.com/adol-c/adol-c), e.g.:

	git clone https://gitlab.com/adol-c/adol-c

It is recommended to read the installation instructions provided by the ADOL-C package first. A short summary on how to install ADOL-C is given in the following.


### Linux

Provided that you are in the ADOL-C base directory, the following commands has to be executed:

    autoreconf -fi
    ./configure
    make
    make install


### Windows

As the installation instructions of ADOL-C for Windows do not seem to be up to date, the build instructions will be given here.

1. Run MSYS and go to the ADOL-C base directory.
2. Execute the following commands:
   ```
   autoreconf -fi
   ./configure
   ```
3. Build ADOL-C *either*
    - for **GNU Octave** using **`make` in MSYS**
        - In the same shell as before, execute
            ```
            make
            make install
            ```
    - or for **MATLAB** using **Microsoft Visual Studio**
        - Setup the required libraries for building ADOL-C:
            1. __boost 1.70.0__: [Download here](https://www.boost.org/users/history/version_1_70_0.html), then copy the archive contents into `{ADOL-C-DIR}\MSVisualStudio\v14\boost\`
            2. __ColPack__ (Optional, needed when building `sparse` configuration):
                * [Download here](https://github.com/CSCsw/ColPack/tree/10b780b52a25c6ccef3d2abc12055ee85b7e8667), newer versions have a directory structure that is incompatible with the VS solution shipped with ADOL-C 2.6.3
                * Copy archive contents into `{ADOL-C-DIR}\MSVisualStudio\v14\ColPack\`
                * Move `{ADOL-C-DIR}\MSVisualStudio\v14\ColPack.vcxproj` to `{ADOL-C-DIR}\MSVisualStudio\v14\ColPack\ColPack.vcxproj`
        - Open `{ADOL-C-DIR}\MSVisualStudio\v14\adolc.sln` in Visual Studio 2015 or newer, VS 2017+ needs to have the 'Desktop Development with C++' workload installed
        - When using VS 2017+, either agree to retargeting the projects to the current build platform when prompted, or manually retarget by right clicking on the 'adolc' and 'ColPack' projects in the solution explorer, and selecting 'Retarget solution'
        - Choose the right configuration (sparse|nosparse) and platform (Win32|x64)
        - Select 'Build', 'Build Solution' (Ctrl+Shift+B)
        - Generated binaries are in `{ADOL-C-DIR}\MSVisualStudio\v14\sparse|nosparse\` and called `adolc.dll` and `adolc.lib`
 

## Install the toolbox

1. At first in \c madSettings.m the paths to the ADOL-C include and library directory have to be set properly. Use absolute paths only. 
2. Linux only: make sure that the ADOL-C library path can be found at runtime. By default this is not the case when the installation directory of ADOL-C (`/home/<user name>/adolc_base`) was not changed. Therefore the library path has to be added to the `LD_LIBRARY_PATH` variable. This can be done by adding

   ```
   export LD_LIBRARY_PATH=/home/<user name>/adolc_base:$LD_LIBRARY_PATH
   ```
   to `~/.bashrc`. Afterwards you have to log off and on again or type
   
   ```
   source ~/.bashrc
   ```
3. Matlab only: set the the compiler to build the mex functions by

   ```
   mex -setup
   ```
   and choose an appropriate compiler that supports C++11, e.g., `Microsoft Visual C++ 2015`
4. Run `install.m`.
