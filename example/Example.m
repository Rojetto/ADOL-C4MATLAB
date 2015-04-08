% Beispielscript für ADOL-C unter Matlab
%
% Vorher bitte Readme lesen


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Schritt 1: Pfade hinzufügen und DLL kopieren
% 
% Benötigt wird Datei Settings.m, in der der die Pfade richtig spezifiziert
% werden müssen
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
madTapingPath = '..\madTaping';
madDriversPath = '..\madDrivers\build32';
addpath(madTapingPath);
addpath(madDriversPath);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Schritt 2: Tape erzeugen
% muss nur 1x durchgeführt werden
% 
% Benötigt wird Datei XSinXY.m, in der der reine Code der Funktion steht
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% n = 2, d = 1, keep = 0
TapeId = madTapeCreate(2, 1, 0, 'XSinXY.m',1);


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Schritt 3: Arbeiten mit den Treibern
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

% Funktionswert an der Stelle (3,4)
X = [3 4]';
disp('Funktionswert: ');
madFunction(TapeId, X)

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
% Schritt 4: Pfade löschen und aufräumen
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
rmpath(madTapingPath);
rmpath(madDriversPath);
%clear madTapingPath madDriversPath
clear all;
%clear mex;  % mex-Funktionen entladen
warning off;
delete *.tap *.cpp *.obj *.exe;
warning on;