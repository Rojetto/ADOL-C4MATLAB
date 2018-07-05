% Beispielscript f�r ADOL-C unter Matlab
%
% Vorher bitte Readme lesen


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Schritt 1: Pfade hinzuf�gen und DLL kopieren
% 
% Ben�tigt wird Datei Settings.m, in der der die Pfade richtig spezifiziert
% werden m�ssen
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
SettingsFile    = '../madSettings';
%SettingsFile    = '../madSettings_Octave_Win';
%SettingsFile    = '../madSettings_Octave_Linux';
    
% load settings
[filepath, name] = fileparts([SettingsFile, '.m']);
addpath(filepath);
Settings = eval(name);
rmpath(filepath);



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Schritt 2: Tape erzeugen
% muss nur 1x durchgef�hrt werden
% 
% Ben�tigt wird Datei XSinXY.m, in der der reine Code der Funktion steht
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% n = 2, d = 1, keep = 0
TapeId = madTapeCreate(2, 1, 1, 'XSinXY.m', Settings);

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Schritt 3: Arbeiten mit den Treibern
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Funktionswert an der Stelle (3,4)
X = [3 4]';
disp('Funktionswert: ');
%madFunction(TapeId, X)

% Gradient an der Stelle (3,4)
X = [3 4]';
disp('Gradient: ');
madGradient(TapeId, X)

% Jacobimatrix an der Stelle (3,4)
X = [3 4]';
disp('Jacobi-Matrix: ');
madJacobian(TapeId, X)

% Hessematrix an der Stelle (3,4)
X = [3 4]';
disp('Hessematrix: ');
madHessian(TapeId, X)

% forward bis d=2
X = [3 4; 4 -2; 4 -2]';
disp('forward: ');
madForward(TapeId, 2, 0, X)

% Ende
madTapeClose(TapeId);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Schritt 4: Pfade l�schen und aufr�umen
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

%clear madTapingPath madDriversPath
clear all;
%clear mex;  % mex-Funktionen entladen
warning off;
%delete *.cpp *.obj *.exe;
warning on;