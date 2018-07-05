thisFilePath = fileparts(mfilename('fullpath'));

% ====================================================
% List of paths to the Adol-C header files
%
% Use IncDir{1} to specify the path for VC compiler
% Use IncDir{2} to specify the path for gnu compiler
% Use IncDir{3} to specify the path for gnu compiler
%
% Do NOT use quotes (")!
% Paths may contain spaces!
% =====================================================
IncDir{1} = [thisFilePath, '\..\..\Adol-C\ADOL-C\include'];
IncDir{2} = [thisFilePath, '\..\..\Adol-C\ADOL-C\include'];
IncDir{3} = '/home/mirko/adolc_base/include';


% ====================================================
% List of paths to the Adol-C library directories
%
% Use LibDir{1} to specify the path for VC compiler
% Use LibDir{2} to specify the path for gnu compiler
%
% Use absolute paths only!
%
% Do NOT use quotes (")!
% Path may contain spaces!
% =====================================================
LibDir{1} = [thisFilePath, '\..\..\Adol-C\MSVisualStudio\v14\nosparse'];
LibDir{2} = [thisFilePath, '\..\..\Adol-C\ADOL-C\.libs'];
LibDir{3} = '/home/mirko/adolc_base/lib64';


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


clear thisFilePath;