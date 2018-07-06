Installation
============

Requirements
------------

The ADOL-C4MATLAB toolbox has the following requirements

-  for **Linux**

   -  GNU Toolchain
   -  ADOL-C
   -  Octave (Matlab not tested yet)

-  for **Windows**

   -  MSYS and MinGW (required to build ADOL-C). Using the MinGW
      Installation Manager, the following packages have to be chosen:

      -  mingw-developer-toolkit
      -  mingw<32|64>-base
      -  mingw<32|64>-gcc-g++
      -  mingw<32|64>-gcc-objc
      -  msys-base

   -  ColPack (required for ADOL-C)
   -  ADOL-C
   -  either

      -  Matlab >= 2015b and
      
         -  an suitable compiler supporting C++11
         -  Microsoft Visual Studio (Visual C++) 2015 or newer `must be
            supported by the specific Matlab version`_, be aware that the
            64-bit compiler only can be used with a 64-bit version of
            Matlab)
         -  MinGW-w64

   -  or

      -  Octave (already comes with the MinGW compiler)

Install ADOL-C
--------------

ADOL-C is available from the COIN-OR initiative, see
https://projects.coin-or.org/ADOL-C. Alternatively ADOL-C can be cloned
from `GitLab`_, e.g.:

::

   git clone https://gitlab.com/adol-c/adol-c

It is recommended to read the installation instructions provided by the
ADOL-C package first. A short summary on how to install ADOL-C is given
in the following.

Linux
~~~~~

Provided that you are in the ADOL-C base directory, the following
commands has to be executed:

::

   autoreconf -fi
   ./configure
   make
   make install

Windows
~~~~~~~

As the installation instructions of ADOL-C for Windows do not seem to be
up to date, the build instructions will be given here.

1. Run MSYS and go to the ADOL-C base directory.
2. Execute the following commands:

   ::

      autoreconf -fi
      ./configure
      make
      make install

   where ``make`` and ``make install`` only have to be executed when
   Octave should be used later.
3. In case Matlab will be used later follow these instructions

   -  Open the Visual Studio Solution ``adolc.sln``.
   -  Chose th right configuration (sparse|nosparse) and platform
      (Win32|x64).
   -  ‘Build Solution’ (F7)

Install the toolbox
-------------------

1. At first in ``madSettings.m`` the paths to the ADOL-C include and library directory have to be set properly. Use absolute paths only.
2. **Linux only:** make sure that the ADOL-C library path can be found at
   runtime. By default this is not the case when the installation
   directory of ADOL-C (``/home/<user name>/adolc_base``) was not
   changed. Therefore the library path has to be added to the
   ``LD_LIBRARY_PATH`` variable. This can be done by adding

   ::

      export LD_LIBRARY_PATH=/home/<user name>/adolc_base:$LD_LIBRARY_PATH

   to ``~/.bashrc``. Afterwards you have to log off and on again or type

   ::

      source ~/.bashrc

3. **Matlab only:** set the the compiler to build the mex functions by

   ::

      mex -setup

   and chose an appropriate compiler that supports C++11, e.g.,
   ``Microsoft Visual C++ 2015``

4. Run ``install.m``.

.. _must be supported by the specific Matlab version: https://de.mathworks.com/support/sysreq/previous_releases.html
.. _GitLab: https://gitlab.com/adol-c/adol-c