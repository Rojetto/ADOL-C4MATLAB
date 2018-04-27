﻿function TapeId = madTapeCreate2(n, m, keep, FuncFileName, varargin);

% TapeId = madTapeCreate(n, m, keep, FuncFileName.m)
%
% Creates an Adol-C tape of the function y = f(x)
% where f is a mapping f : R^n -> R^m, i.e.
% y is a m x 1 column vector and x is a n x 1 column
% vector. The function must be specified in a file
% with the name FuncFileName.m using syntax of
% programming language c. The n elements of x must
% be referred to by x[0]...x[n-1] and the m elements
% y must be referred to by y[0]...y[n-1] within
% this function.
%
% TapeFactory generates a intermediate c++ source code
% file named FuncFileName.cpp which is than automatically
% compiled to TapeFactory_FuncFileName.exe which is than
% executed so the tapes are generated. The generated tapes
% do have the names
% M-Locations_FuncFileName.tap
% M-Operations_FuncFileName.tap
% ADOLC-Values_FuncFileName.tap
%
% The function returns the TapeId which can be used in
% the mad-functions madForwad, madReverse, madJacobian,
% madClose, ... to refer to the tape.
%
% See also: madTapeOpen, madTapeClose

% (c) 2010-2012 
% Carsten Friede, Jan Winkler
% Institut für Regelungs- und Steuerungstheorie
% TU Dresden
% Jan.Winkler@tu-dresden.de



% Some global settings for this file
% ==================================
OptionFileName    = 'madTapingSettings';
NumTapeFiles      = 3;

TapePraefix{1}    = 'ADOLC-Locations_';
TapePraefix{2}    = 'ADOLC-Operations_';
TapePraefix{3}    = 'ADOLC-Values_';

StaticTapePraefix{1}    = 'M-Locations_';
StaticTapePraefix{2}    = 'M-Operations_';
StaticTapePraefix{3}    = 'M-Values_';

if (isunix)
    CopyCommand = 'cp -f';
else
    CopyCommand = 'copy /Y';
end

% Zu vergebende Tape-Nummer
% =========================
DefaultTapeNumber = madTapeOpen() + 1;
if (isempty(DefaultTapeNumber))
   DefaultTapeNumber = 1;
end

% cpp-, lib- und exe-Dateinamen bauen
% ===================================
BaseFileName = FuncFileName;

indM = strfind(BaseFileName, '.m');
if (isempty(indM))
    error(sprintf('File %s is not a m-file!', FuncFileName));
    return;
end

BaseFileName = BaseFileName(1:indM-1);
FuncFileName   = [BaseFileName, '.m'];
SourceFileName = [BaseFileName, '.cpp'];
ObjectFileName = [BaseFileName, '.o'];
ExeFileName    = ['TapeFactory_', BaseFileName, '.exe'];


% Funktion, von der ein Tape erzeugt werden soll, einlesen
% ========================================================
fid            = fopen(FuncFileName, 'rt');
if (fid < 3)
    error(sprintf('Function file with name %s could not be opened!', FuncFileName));
    return;
end

% FuncToBeTaped= fscanf(fid,'%s');  % einfachste Möglichkeit schreibt aber
                                    % alles in eine Zeile
FuncToBeTaped = '';
k=1;
while 1
    currentLine{k} = fgetl(fid);
    if ~ischar(currentLine{k})
        break;
    end
    
    FuncToBeTaped = strcat(FuncToBeTaped, currentLine{k}, '\n\t');
    k = k + 1;
end
fclose(fid);

% Parameter für das Taping konvertieren und speichern
% ==================================================
TapeParameter = {num2str(DefaultTapeNumber),
                 num2str(n),
                 num2str(m),
                 num2str(keep)}; 

             
% cpp-Datei anlegen
% =================
fid_out = fopen(SourceFileName, 'wt');
if (fid_out < 3)
    error(sprintf('A source file with the name %s could not be created!', SourceFileName));
    return;
end


% cpp-Datei aufbauen
% ===================
fid_in = fopen('TapingTemplate2.cpp','rt');    % Vorlage lesend öffnen

optargin = size(varargin,2);
optarginstr = num2str(optargin);

if (optargin ~= 0)
if (optargin<n||optargin>n)
    error(sprintf('The number of independent variable must be the same as the number of the initial values!'));
    return;
end
end

if(optargin ~= 0)
    x0Str = sprintf('[%d] = {', n);
    for i=1:optargin-1        
        x0Str = strcat(x0Str, sprintf('%f, ', varargin{i}));  
    end
    x0Str = strcat(x0Str, sprintf('%f}', varargin{end}));
end
if(optargin == 0)
    x0Str = sprintf('[%d] = {', n);
    for i=1:n-1        
        x0Str = strcat(x0Str, sprintf('%f, ', 0.0));  
    end
    x0Str = strcat(x0Str, sprintf('%f}', 0.0));
end

k=1;    % Zeilenindex
while 1
    lines{k} = fgets(fid_in);   % einlesen mit Zeilenumbruch
    if ~ischar(lines{k})        % Prüfung auf Dateiende
        break;
    end
    
    % Suchmuster vergleichen, um Eingabeparameter zu setzen
    lines{k} = regexprep(lines{k}, '//%TapeNumber%', TapeParameter{1});
    lines{k} = regexprep(lines{k}, '//%n%',          TapeParameter{2});
    lines{k} = regexprep(lines{k}, '//%m%',          TapeParameter{3});
    lines{k} = regexprep(lines{k}, '//%keep%',       TapeParameter{4});
    lines{k} = regexprep(lines{k}, '//%x0%', x0Str);  
    lines{k} = regexprep(lines{k}, '// Insertion of function to be taped', FuncToBeTaped);
    k = k+1;
end
fclose(fid_in);     % Vorlage schließen


% parametrisierte cpp-Datei schreiben
% ===================================
for n=1:length(lines)-1
    fprintf(fid_out,'%s',lines{n});
end


% geschriebene cpp-Datei schließen
% ================================
res = fclose(fid_out);
if (res < 0)
    error('Source file %s could not be written to disk!', SourceFileName);
end


% Auswahl des zu verwendenden Compilers
% =====================================
clc
fprintf('==============================================================\n')
fprintf('Please select a compiler to generate a tape building EXE file!\n\n')
fprintf('\t (1) Visual C++ - cl.exe\n\n')
fprintf('\t (2) MinGW - gcc (NOT SUPPORTED YET) \n\n\n')
selectedToolchain = 1;
selectedToolchain = input('Your Choice? [1]: ');


% Befehlsstring für Compiler bauen
% ================================
clear IncDir;
clear LibDir;

% Einstellungen laden
if (~exist(sprintf('%s.m', OptionFileName)))
    error('Option-file %s.m not found!', OptionFileName);
else
    run(OptionFileName);
end

% Compiler-Datei
CC = Compiler{selectedToolchain};

switch(selectedToolchain)
    case 1
        % Auswahl angeben
        fprintf('\n\n\t +--- Visual Studio C++ compiler had been selected ---+\n');
        
        % Quelltext
        CC = [CC, ' "/Od /EHsc /Tp ', SourceFileName];
        % Include-Pfad
        CC = [CC, ' /I ', IncDir{selectedToolchain}];
        
        % Angabe der Objektreferenzen für die DLL
        CC = [CC, ' /link /LIBPATH ', LibDir{selectedToolchain}, '\\', LibName{1},'"'];
                
    case 2
        % Auswahl angeben
        fprintf('\n\n\t +--- MinGW GCC compiler had been selected ---+\n');
        
        % Quelldatei
        CC = [CC, ' -c ', SourceFileName];

        % Objekt-Datei
        CC = [CC, ' -o ', ObjectFileName];

        % Include-Pfade
        for i=2:1:length(IncDir)
            CC = [CC, ' -I', IncDir{i}];
        end

        % Definitionen
        if (exist('Def'))
            for i=1:1:length(Def)
                CC = [CC, ' -D', Def{i}];
            end
        end

        % Debug-Flag
        if (Debug == 1)
            CC = [CC, ' -D__DEBUG__ -g3'];
        end
        
    otherwise
        disp('kein Compiler verfügbar');
end


% Compilierung der Quellcode-Datei
% ================================
disp('============================================');
disp('Trying to compile with command: ');
disp(CC);
[res, msg] = system(['cmd.exe /k BuildRunVC.bat c ', CC]);
disp(sprintf('\nCompilation exited with Code %d \n\n', res));
disp(msg);

if (res ~= 0)
    error('Compilation failed! Refer to the error message displayed above for more details!');
    return;
end

disp('============================================');

% Linkage-Prozess (nur bei Verwendung von GCC)
if (selectedToolchain == 2)

    % Befehlsstring für Linker bauen
    % ==============================
    
    % Compiler-Datei
    LC = Compiler;

    % Objektdatei für Linker
    LC = [LC, ' ', ObjectFileName];

    % Zu bauenende Datei
    LC = [LC, ' -o ', ExeFileName];

    % Suchpfad für Bibliotheken
    for i=1:1:length(LibDir)
        LC = [LC, ' -L', LibDir{i}];
    end

    % Bibliotheken, gegen die gelinkt werden soll
    for i=1:1:length(LibName)
        LC = [LC, ' ', LibName{i}];
    end

    % Definitionen
    if (exist('Def'))
        for i=1:1:length(Def)
            LC = [LC, ' -D', Def{i}];
        end
    end

    % Debug
    if (Debug == 1)
        LC = [LC, ' -D__DEBUG__ -g3'];
    end



    % Durchführung der Linkage
    % ========================
    disp('Trying to link with command: ');
    disp(LC);
    [res, msg] = system(LC);
    disp(sprintf('\nLinkage exited with Code %d\n\n', res));
    disp(msg);

    if (res ~= 0)
        error('Linkage failed! Refer to the error message displayed above for more details!');
        return;
    end

    disp('============================================');

end


% Ausführung der erzeugten Datei zur Erzeugung der Tapes
% ======================================================

%pause(5)
%if exist(ExeFileName,'file') == 0
    movefile([BaseFileName,'.exe'],ExeFileName)
%end
disp(sprintf('Executing tape factory %s', ExeFileName));
[res, msg] = system(['cmd.exe /k BuildRunVC.bat r ',LibDir{1},' ',ExeFileName]);
disp(sprintf('\nCreation of tapes exited with Code %d\n\n', res));
disp(msg);

if (res ~= 0)
    error('Creation of tapes failed! Refer to the error message displayed above for more details!');
    return;
end



% Umbenennen der erzeugten Tapes
% ==============================
for i = 1:1:NumTapeFiles
    TapeFile      = strcat(TapePraefix{i}, num2str(DefaultTapeNumber), '.tap');
    TapeFileNamed{i} = strcat(StaticTapePraefix{i}, BaseFileName, '.tap');
    [res, msg] = system(sprintf('%s %s %s', CopyCommand, TapeFile, TapeFileNamed{i}));

    if (res ~= 0)
        disp(msg);
        error('Rename of Tape %s to %s failed! Refer to the error message displayed above for more details!', TapeFile, TapeFileNamed{i});
        return;
        %else
        %    disp(sprintf('Tape %s successfully renamed to %s', TapeFile, TapeFileNamed));
    end
end

disp(sprintf('%s successfully taped to \n %s \n %s \n %s', FuncFileName, TapeFileNamed{1}, TapeFileNamed{2}, TapeFileNamed{3}));

disp(' ');


TapeId = madTapeOpen(BaseFileName);