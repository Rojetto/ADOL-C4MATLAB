% Example script that demonstrates how to use the toolbox.

% (c) 2010-2018 
% Mirko Franke, Jan Winkler, Carsten Friede
% Institute of Control Theory
% Technische Universität Dresden
% {Mirko.Franke, Jan.Winkler}@tu-dresden.de


%% 
% Step 1: load the settings
% requires the file Settings.m that defines reqired path variables

%SettingsFile    = '../madSettings';
SettingsFile    = '../madSettings_Octave_Win';
%SettingsFile    = '../madSettings_Octave_Linux';
    
% load settings
[filepath, name] = fileparts([SettingsFile, '.m']);
addpath(filepath);
Settings = eval(name);
rmpath(filepath);


%%
% Step 2: generate the tapes
% this step has to be done once for each function that has to be taped
% 
% in this example the function is implemented within the file XSinXY.m

n = 2;      % number of independent variables
d = 1;      % number of dependent variables
keep = 1;   % prepare for an immediate call of the reverse mode (see the ADOL-C manual)

% generate the tape
TapeId = madTapeCreate(n, m, keep, 'XSinXY.m', Settings);


%%
% Step 3: using the toolbox drivers

% function evaluation at (3, 4)
X = [3 4]';
disp('Funktionswert: ');
madFunction(TapeId, X)

% gradient evaluation at (3, 4)
X = [3 4]';
disp('Gradient: ');
madGradient(TapeId, X)

% Jacobian evaluation at (3, 4)
X = [3 4]';
disp('Jacobi-Matrix: ');
madJacobian(TapeId, X)

% Hessian evaluation at (3, 4)
X = [3 4]';
disp('Hessematrix: ');
madHessian(TapeId, X)

% forward mode
X = [3 4; 4 -2; 4 -2]';
disp('forward: ');
madForward(TapeId, 2, 0, X)

% close the tape
madTapeClose(TapeId);


%%
% Step 4: clean up

clear all;
clear mex;