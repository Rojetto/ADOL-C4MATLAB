function TapeId = madTapeCreate(varargin)

% TapeId = madTapeCreate(n, m, keep, FuncFileName)
% TapeId = madTapeCreate(n, m, keep, FuncFileName, ToolChain)
%
% Creates an Adol-C tape of the function y = f(x)
% where f is a mapping f : R^n -> R^m, i.e.
% y is a m x 1 column vector and x is a n x 1 column
% vector. The function must be specified in a file
% with the name FuncFileName.m using syntax of
% programming language C. The n elements of x must
% be referred to by x[0]...x[n-1] and the m elements
% y must be referred to by y[0]...y[n-1] within
% this file.
%
% madTapeCreate generates an intermediate c++ source code
% file named FuncFileName.cpp which is than automatically
% compiled to TapeFactory_FuncFileName.(mexext) which is than
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
% The function needs a file madTapingSettings.m in which settings
% for several compilers and targets are defined. Open this file
% and adapt it to your local environment.
%
% The second call syntax of the command is equal to the first one
% except that the toolchain is explicitely determined in the 
% parameter ToolChain, i.e. it needs not to be selected interactively
% during run of the function. ToolChain is an integer determing
% the compiler as defined in the file madTapingSettings.m
%
% See also: madTapeOpen, madTapeClose, madTapingSettings.m

% (c) 2010-2018 
% Carsten Friede, Jan Winkler, Mirko Franke
% Institut f�r Regelungs- und Steuerungstheorie
% TU Dresden
% {Jan.Winkler, Mirko.Franke}@tu-dresden.de

% TODO:
% Auch erm�glichen, das Tape an einer expliziten Stelle zu erstellen


if (nargin == 4)
    n             = varargin{1};
    m             = varargin{2};
    keep          = varargin{3};
    FuncFileName  = varargin{4};
elseif (nargin == 5)
    n                 = varargin{1};
    m                 = varargin{2};
    keep              = varargin{3};
    FuncFileName      = varargin{4};
    selectedToolchain = varargin{5};
else
    error('Incorrect number of input arguments!');
end


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

% Aktueller Pfad
% ==============
p = mfilename('fullpath');
fileDirectory = fileparts(p);

% cpp-, lib- und exe-Dateinamen bauen
% ===================================
BaseFileName = FuncFileName;

indM = strfind(BaseFileName, '.m');
if (isempty(indM))
    error(sprintf('File %s is not a m-file!', FuncFileName));
end

BaseFileName = BaseFileName(1:indM-1);
FuncFileName   = [BaseFileName, '.m'];
SourceFileName = [BaseFileName, '.cpp'];
ObjectFileName = [BaseFileName, '.o'];
MexFileName    = ['TapeFactory_', BaseFileName];


% Funktion, von der ein Tape erzeugt werden soll, einlesen
% ========================================================
fid            = fopen(FuncFileName, 'rt');
if (fid < 3)
    error(sprintf('Function file with name %s could not be opened!', FuncFileName));
end

% FuncToBeTaped= fscanf(fid,'%s');  % einfachste M�glichkeit schreibt aber
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


% Parameter f�r das Taping konvertieren und speichern
% ==================================================
TapeParameter = {num2str(n),
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
fid_in = fopen('TapingTemplate.cpp','rt');    % Vorlage lesend �ffnen

k=1;    % Zeilenindex
while 1
    lines{k} = fgets(fid_in);   % einlesen mit Zeilenumbruch
    if ~ischar(lines{k})        % Pr�fung auf Dateiende
        break;
    end
    
    % Suchmuster vergleichen, um Eingabeparameter zu setzen
    lines{k} = regexprep(lines{k}, '//%n%',          TapeParameter{1});
    lines{k} = regexprep(lines{k}, '//%m%',          TapeParameter{2});
    lines{k} = regexprep(lines{k}, '//%keep%',       TapeParameter{3});
    lines{k} = regexprep(lines{k}, '// Insertion of function to be taped', FuncToBeTaped);
    k = k+1;
end
fclose(fid_in);     % Vorlage schlie�en


% parametrisierte cpp-Datei schreiben
% ===================================
for n=1:length(lines)-1
    fprintf(fid_out,'%s',lines{n});
end


% geschriebene cpp-Datei schlie�en
% ================================
res = fclose(fid_out);
if (res < 0)
    error('Source file %s could not be written to disk!', SourceFileName);
end


% Auswahl des zu verwendenden Compilers
% =====================================
if ~exist('selectedToolchain','var')
    clc
    fprintf('==============================================================\n')
    fprintf('Please select a compiler to generate a tape building EXE file!\n\n')
    fprintf('\t (1) Visual C++ - cl.exe\n\n')
    fprintf('\t (2) MinGW - gcc (NOT SUPPORTED YET) \n\n\n')
    selectedToolchain = input('Your Choice? [1]: ');
end


% Befehlsstring f�r Compiler bauen
% ================================
clear IncDir;
clear LibDir;

% Einstellungen laden
if (~exist(sprintf('%s.m', OptionFileName)))
    error('Option-file %s.m not found!', OptionFileName);
else
    feval(OptionFileName)
    %run(OptionFileName);
end

% Compiler-Datei
CC = Compiler{selectedToolchain};

switch(selectedToolchain)
    case 1
        % Auswahl angeben
        fprintf('\n\n\t +--- Visual Studio C++ compiler had been selected ---+\n');
        
        % Quelltext
        CC = ['"', CC, '"', ' "/Od /EHsc /D "MATLAB_MEX_FILE" /Tp ', SourceFileName, ' "', fileDirectory, '\..\madDrivers\src\madHelpers\madHelpers.cpp"'];
        
        % Include-Pfad (pr�fen, ob mehrere Pfade angegeben wurden, dann
        % m�ssen diese einzeln mit dem fileDirectory zusammengef�gt
        % und dem Compiler �bergeben werden
        ind = strfind(IncDir{selectedToolchain}, ';');
        if isempty(ind) % 1 include Verzeichnis
            CC = [CC, ' /I "', fileDirectory, '\', IncDir{selectedToolchain}, '"'];
        else % mehrere include-Verzeichnisse
            IncDirs = IncDir{selectedToolchain};
            ind = [0 ind length(IncDirs)+1];
            NumIncDirs = length(ind)-1;
            for i = 1:1:NumIncDirs                
                CC = [CC, ' /I "', fileDirectory, '\', IncDirs(ind(i)+1:ind(i+1)-1), '"'];
            end
        end
		% Matlab include-Verzeichnis zum Bauen von MEX-Funktionen
		CC = [CC, ' /I "', matlabroot, '\extern\include', '"'];
		% madHelpers
		CC = [CC, ' /I "', fileDirectory, '\..\madDrivers\src\madHelpers', '"'];
		
		CC = [CC, ' /link /OUT:"', MexFileName, '.', mexext, '" /DLL /MACHINE:X86 /EXPORT:"mexFunction"'];
		
		% Angabe der notwendigen Bibliotheken
		CC = [CC, ' /DYNAMICBASE "', LibName{1},'"'];	% ADOL-C
		CC = [CC, ' "libmex.lib" "libmx.lib" "libmat.lib"']; % Matlab MEX
		
		
		
		% Angabe der Objektreferenzen f�r die libs
        CC = [CC, ' /LIBPATH:"', fileDirectory, '\', LibDir{selectedToolchain},'"'];	% ADOL-C
		CC = [CC, ' /LIBPATH:"', matlabroot, '\extern\lib\win32\microsoft', '""'];
                
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
        disp('kein Compiler verf�gbar');
end


% Compilierung der Quellcode-Datei
% ================================
disp('============================================');
disp('Trying to compile with command: ');
disp(CC);
[tempstr,maxArraySize]=computer; 
is64bitComputer=maxArraySize> 2^31;
if (is64bitComputer)
	[res, msg] = system(['"', fileDirectory ,'\BuildRunVC.bat" c ', CC, ' 64']);
else
	[res, msg] = system(['"', fileDirectory ,'\BuildRunVC.bat" c ', CC, ' 32']);
end
disp(sprintf('\nCompilation exited with Code %d \n\n', res));
disp(msg);

if (res ~= 0)
    error('Compilation failed! Refer to the error message displayed above for more details!');
    return;
end

disp('============================================');

% Linkage-Prozess (nur bei Verwendung von GCC)
if (selectedToolchain == 2)

    % Befehlsstring f�r Linker bauen
    % ==============================
    
    % Compiler-Datei
    LC = Compiler;

    % Objektdatei f�r Linker
    LC = [LC, ' ', ObjectFileName];

    % Zu bauenende Datei
    LC = [LC, ' -o ', ExeFileName];

    % Suchpfad f�r Bibliotheken
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



    % Durchf�hrung der Linkage
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

% L�schen von nicht ben�tigten Dateien
if (selectedToolchain == 1)
    delete *.cpp
    delete *.obj
    delete *.lib
    delete *.exp
end


% Ausf�hrung der erzeugten Datei zur Erzeugung der Tapes
% ======================================================

% zun�chst muss die DLL ins aktuelle Arbeitsverzeichnis kopiert werden
try
    copyfile([fileDirectory, '\' , LibDir{1}, '\adolc.dll'], 'adolc.dll');
catch
    warning('Could not copy DLL: adolc.dll');
end

% �ffnen des Tapes
TapeId = madTapeOpen(BaseFileName);