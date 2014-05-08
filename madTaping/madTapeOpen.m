function TapeId = madTapeOpen(varargin)

% TapeId = mad_OpenTape(FileBaseName)
%
% Opens a tape with the name FileBaseName, i.e. the tape which saved on
% disk in the files
% ADOLC-Locations_FileBaseName.tap
% ADOLC-Operations_FileBaseName.tap
% ADOLC-Values_FileBaseName.tap
%
% The function returns the id under which the tape can be accessed using
% the mad-functions madClose, madForward, madReverse, etc. 
%
% Call this function without any argument to get the maximum tape number
% used at the moment.
%
% On disk this function renames the files mentioned above to
%
% ADOLC-Locations_X.tap
% ADOLC-Operations_X.tap
% ADOLC-Values_X.tap
%
% where X is a unique integer number so the built-in ADOL-C functions
% can access the tapes.

% (c) 2010-2012 
% Carsten Friede, Jan Winkler
% Institut für Regelungs- und Steuerungstheorie
% TU Dresden
% Jan.Winkler@tu-dresden.de

persistent MaxAssignedTapeId;

if (nargin == 0)
    TapeId = MaxAssignedTapeId;
    return;
elseif (nargin == 1)
    BaseFileName = varargin{1};
end

if (isempty(MaxAssignedTapeId))
    MaxAssignedTapeId = 1;
else
    MaxAssignedTapeId = MaxAssignedTapeId + 1;
end

NumTapeFiles       = 3;

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

% Kopieren der erzeugten Tapes
% ==============================
for i = 1:1:NumTapeFiles
   TapeFile      = strcat(TapePraefix{i}, num2str(MaxAssignedTapeId),'.tap');
   TapeFileNamed{i} = strcat(StaticTapePraefix{i}, BaseFileName, '.tap');
   [res, msg] = system(sprintf('%s %s %s', CopyCommand, TapeFileNamed{i}, TapeFile));

   if (res ~= 0)
       disp(msg);
       error('Copy of Tape %s to %s failed! Refer to the error message displayed above for more details!', TapeFile, TapeFileNamed{i});
       TapeId = -1;
       return;
   end
end

TapeId = MaxAssignedTapeId;

disp(sprintf('Tape \n %s \n %s \n %s \nsuccessfully opened under id %d', TapeFileNamed{1}, TapeFileNamed{2}, TapeFileNamed{3}, TapeId));
disp(' ');

