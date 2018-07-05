function [Settings] = madSettings()

% Settings = madSettings()
%
% (c) 2010-2018 
% Mirko Franke, Jan Winkler, Carsten Friede
% Institute of Control Theory
% Technische Universität Dresden
% {Mirko.Franke, Jan.Winkler}@tu-dresden.de
   
    thisFilePath = fileparts(mfilename('fullpath'));

    % ====================================================
    % List of paths to the Adol-C header files
    %
    % Use absolute paths only! 
    % Do NOT use quotes (")!
    % Paths may contain spaces!
    % =====================================================
    Settings.IncDir = [thisFilePath, '\..\Adol-C\ADOL-C\include'];


    % ====================================================
    % List of paths to the Adol-C library directories
    %
    % Use absolute paths only!
    % Do NOT use quotes (")!
    % Path may contain spaces!
    % =====================================================
    Settings.LibDir = [thisFilePath, '\..\Adol-C\MSVisualStudio\v14\nosparse'];


    % =====================================================
    % Create tape factory with debug information
    %
    % Set this to true if you want the tape factory to be
    % created with debug information. (For the gnu-compiler
    % it sets the debug level to "-g" and the definition
    % -D__DEBUG__.
    %
    % Set this to false if you do not wish to include debug
    % information into the factory.
    %
    % Debug informations are only useful if you are going
    % to debug the tape factory during execution using
    % e.g. the GNU debugger gdb
    % =====================================================
    Settings.Debug = false;
    
    
    % =====================================================
    % Runtime Settings
    %
    % Do not edit!
    % =====================================================
    addpath([thisFilePath, '/madTaping']);
	addpath([thisFilePath, '/madDrivers/build']);
    
    if all(size(strfind(getenv('PATH'), Settings.LibDir)) == [0, 0])
        setenv('PATH', [getenv('PATH') pathsep() Settings.LibDir]);
    end
    
end