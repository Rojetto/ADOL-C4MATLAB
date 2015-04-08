% ====================================================
% 32 Bit or 64 Bit Matlab
% ====================================================
MatlabBit = '32bit';
%MatlabBit = '64bit';


% ====================================================
% Path and filename of the compiler to be used
%
% For Visual-Studio users: Please do only enter
% the base path, e.g.
% c:\Program Files (x86)\Microsoft Visual Studio 9.0
%
% Example:
% Compiler = C:/Programme/Dev-Cpp/bin/g++.exe
%
% Do NOT use quotes (")!
% Path may contain spaces!
% ====================================================
Compiler{1} = 'C:\Program Files (x86)\Microsoft Visual Studio 9.0';
Compiler{2} = 'C:/Programme/MinGW/MinGW/bin/g++.exe';


% ====================================================
% List of paths to header files
%
% Use IncDir{1} to specify the path for the first compiler
% Use IncDir{2} to specify the path for the second compiler
% 
% and so on
%
% At least the path to the AdolC header files is 
% required here
% Use more than one directory separating them by ';'
% Example:
% IncDir{1} = '..\..\AdolC4Win\src\ADOL-C\include;..\..\AdolC4Win\windows\vc2008\stdint';
%
% Do NOT use quotes (")!
% Paths may contain spaces!
% =====================================================
IncDir{1} = '..\..\AdolC4Windows\src\ADOL-C\include;..\..\AdolC4Windows\windows\vc2008\stdint';
IncDir{2} = '../../AdolC4Windows/ADOL-C';


% ====================================================
% List of paths to library directories
%
% Use LibDir{1} to specify the path for the first compiler
% Use LibDir{2} to specify the path for the second compiler
%
% At least the path to the standard libraries is
% required here, e.g.
% LibDir{0} = C:/Programme/MinGW/MinGW/lib
%
% Do NOT use quotes (")!
% Path may contain spaces!
% =====================================================
LibDir{1} = '..\..\AdolC4Windows\build32\Debug NoSparse';
LibDir{2} = '../../../AdolC4Win/windows/build C:/Programme/MinGW/MinGW/lib';


% ====================================================
% The name of the AdolC library
%
% Here the name of the AdolC library 
% the TapeFactory is going to be linked against
% must be specified, e.g.
% LibName = libadolc.lib
%
% LibName{1} Library name generated by the first compiler
% LibName{2} Library name generated by the second compiler
% =====================================================
LibName{1} = 'adolc.lib';
LibName{2} = 'libadolc.lib';


% ====================================================
% List of definitions
%
% Add as many definitions as you need by setting Def{0},
% Def{1}, Def{2}, Def{3}, ...
%
% E.g. Def{0} = __TEST__;
% =====================================================
%Def{1} = 'C:/Programme/MinGW/MinGW/lib';
%Def{1} =
%Def{2} =


% =====================================================
% Create tape factory with debug information
%
% Set this to 1 if you want the tape factory to be
% created with debug information. (For the gnu-compiler
% it sets the debug level to "-g3" and the definition
% -D__DEBUG__.
%
% Set this to 0 if you do not wish to include debug
% information into the factory.
%
% Debug informations are only useful if you are going
% to debug the tape factory during execution using
% e.g. the GNU debugger gdb
% =====================================================
Debug = 0;