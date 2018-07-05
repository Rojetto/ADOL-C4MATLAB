function TapeId = madTapeCreate(varargin)

% TapeId = madTapeCreate(n, m, keep, FuncFileName, buildSettings, runtimeSettings)
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
% executed so the tapes are generated. 
%
% The function returns the TapeId which can be used in
% the mad-functions madForwad, madReverse, madJacobian,
% madClose, ... to refer to the tape.
%
% The function needs a file madTapingSettings.m in which settings
% for several compilers and targets are defined. Open this file
% and adapt it to your local environment.
%
% See also: madTapeOpen, madTapeClose, madSettings

% (c) 2010-2018 
% Mirko Franke, Jan Winkler, Carsten Friede
% Institute of Control Theory
% Technische Universität Dresden
% {Mirko.Franke, Jan.Winkler}@tu-dresden.de


if (nargin == 5)
    n               = varargin{1};
    m               = varargin{2};
    keep            = varargin{3};
    FuncFileName    = varargin{4};
    Settings        = varargin{5};
else
    error('Incorrect number of input arguments!');
end

isOctave = exist('OCTAVE_VERSION', 'builtin') ~= 0;
if isOctave
     warning('off', 'Octave:mixed-string-concat');
     warning('off', 'Octave:language-extension');
end


% Aktueller Pfad
% ==============
% p = mfilename('fullpath');
% fileDirectory = fileparts(p);

% cpp-, lib- und exe-Dateinamen bauen
% ===================================
BaseFileName = FuncFileName;

indM = strfind(BaseFileName, '.m');
if (isempty(indM))
    error('File %s is not a m-file!', FuncFileName);
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
    error('Function file with name %s could not be opened!', FuncFileName);
end

% FuncToBeTaped= fscanf(fid,'%s');  % einfachste Mï¿½glichkeit schreibt aber
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


             
% cpp-Datei anlegen
% =================
fid_out = fopen(SourceFileName, 'wt');
if (fid_out < 3)
    error('A source file with the name %s could not be created!', SourceFileName);
end


% cpp-Datei aufbauen
% ===================
fid_in = fopen([fileparts(mfilename('fullpath')), '/TapingTemplate.cpp'],'rt');    % Vorlage lesend ï¿½ffnen

k=1;    % Zeilenindex
while 1
    lines{k} = fgets(fid_in);   % einlesen mit Zeilenumbruch
    if ~ischar(lines{k})        % Prï¿½fung auf Dateiende
        break;
    end
    
    % Suchmuster vergleichen, um Eingabeparameter zu setzen
    lines{k} = regexprep(lines{k}, '//%n%',          num2str(n));
    lines{k} = regexprep(lines{k}, '//%m%',          num2str(m));
    lines{k} = regexprep(lines{k}, '//%keep%',       num2str(keep));
    lines{k} = regexprep(lines{k}, '// Insertion of function to be taped', FuncToBeTaped);
    k = k+1;
end
fclose(fid_in);     % Vorlage schlieï¿½en


% parametrisierte cpp-Datei schreiben
% ===================================
for n=1:length(lines)-1
    fprintf(fid_out,'%s',lines{n});
end


% geschriebene cpp-Datei schlieï¿½en
% ================================
res = fclose(fid_out);
if (res < 0)
    error('Source file %s could not be written to disk!', SourceFileName);
end




% compiler and linker settings
    iPath1 = ['-I', Settings.IncDir];
    iPath2 = ['-I', [fileparts(mfilename('fullpath')), '/../madDrivers/src/madHelpers']];
    iPath3 = ['-I', [fileparts(mfilename('fullpath')), '/../madDrivers/src/MatrixLib']];
    lPath = ['-L', Settings.LibDir];
    helperFile = [fileparts(mfilename('fullpath')), '/../madDrivers/src/madHelpers/madHelpers.cpp'];
    matrixFile = [fileparts(mfilename('fullpath')), '/../madDrivers/src/MatrixLib/matrixlib.c'];
    lName = '-ladolc';
    outName = {'-output', MexFileName};
    

% build MEX files
mex(iPath1, iPath2, iPath3, lPath, lName, '-O', outName{:}, SourceFileName, helperFile, matrixFile);



% Löschen von nicht benötigten Dateien
delete(SourceFileName);
if exist(ObjectFileName, 'file')
    delete(ObjectFileName);
end
if exist('madHelpers.o', 'file')
    delete('madHelpers.o');
end
if exist('matrixlib.o', 'file')
    delete('matrixlib.o');
end



% Öffnen des Tapes
TapeId = madTapeOpen(BaseFileName);