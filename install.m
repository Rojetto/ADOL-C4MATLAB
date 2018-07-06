function install()

% install()
%
% 

% (c) 2010-2018 
% Mirko Franke, Jan Winkler, Carsten Friede
% Institute of Control Theory
% Technische Universität Dresden
% {Mirko.Franke, Jan.Winkler}@tu-dresden.de


    % specify settings file
    SettingsFile    = 'madSettings';
% 	SettingsFile    = 'madSettings_Octave_Win';
% 	SettingsFile    = 'madSettings_Octave_Linux';
    
    
    % load settings
    if (~exist(sprintf('%s.m', SettingsFile), 'file'))
        error('Settings-file %s.m not found!', OptionFile);
    else
        Settings = eval(SettingsFile);
    end
    
    
    % detect whether Matlab or Octave is running
    isOctave = exist('OCTAVE_VERSION', 'builtin') ~= 0;

    
    % compiler and linker settings
    iPath1   = ['-I', Settings.IncDir];
    iPath2   = ['-I', 'madDrivers/src/madHelpers'];
    iPath3   = ['-I', 'madDrivers/src/MatrixLib'];
    lPath    = ['-L', Settings.LibDir];
    lName    = '-ladolc';
    buildDir = './madDrivers/build/';
    oPath    = {'-outdir' buildDir};
    defDebug = '-D__DEBUG__';
    enDebug  = '-g';

    
    % list of MEX functions to build
    madFiles = {'madCompTorqueControl'; 
                'madCompTorqueLagrange';
                'madExtLuenObs';
                'madFeedbackLin';
                'madForward';
                'madFunction'; 
                'madGradient';
                'madHessian';
                'madHighGainObs';
                'madJacobian';
                'madLagrange';
                'madLagrangePartLin';
                'madLieBracket';
                'madLieCovector';
                'madLieDerivative';
                'madLieGradient';
                'madLieMixedDerivative';
                'madLieScalar';
                'madReverse';
                };

            
    % build MEX files
    for i=1:size(madFiles,1)
        madFile = madFiles{i};
        if isOctave
            warning('off', 'Octave:mixed-string-concat');
            oPath = {'-o' [buildDir, madFile, '.', mexext]};
        end
        fName = ['./madDrivers/src/', madFile, '/', madFile, '.cpp'];
        fprintf(['Building ', madFile, ' ...\n']);
        if Settings.Debug
            mex(defDebug, enDebug, iPath1, iPath2, iPath3, lPath, lName, oPath{:}, '-O', fName, './madDrivers/src/madHelpers/madHelpers.cpp', './madDrivers/src/MatrixLib/matrixlib.c');
        else
            mex(iPath1, iPath2, iPath3, lPath, lName, oPath{:}, '-O', fName, './madDrivers/src/madHelpers/madHelpers.cpp', './madDrivers/src/MatrixLib/matrixlib.c');
        end
        fprintf('Done!\n\n');
        if exist([madFile, '.o'], 'file')
            delete([madFile, '.o']);
        end
    end
    
    
    % delete remaning object files
    if exist('madHelpers.o', 'file')
        delete('madHelpers.o');
    end
    if exist('matrixlib.o', 'file')
        delete('matrixlib.o');
    end
end