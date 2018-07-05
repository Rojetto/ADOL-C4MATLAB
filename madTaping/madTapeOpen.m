function TapeId = madTapeOpen(varargin)

% TapeId = mad_OpenTape(MexFileName)
%
% Opens a tape with the name MexFileName, i.e. the tape which saved on
% disk in the file
% TapeFactory_MexFileName.(mexw32|mexw64)
%
% The function returns the id under which the tape can be accessed using
% the mad-functions madClose, madForward, madReverse, etc. 
%
% Call this function without any argument to get the maximum tape number
% used at the moment.
%
% On disk this function renames the files mentioned above to
%
% TapeFactory_X.(mexext)
%
% where X is a unique integer number so the built-in ADOL-C functions
% can access the tapes.

% (c) 2010-2018 
% Mirko Franke, Jan Winkler, Carsten Friede
% Institute of Control Theory
% Technische Universität Dresden
% {Mirko.Franke, Jan.Winkler}@tu-dresden.de

persistent MaxAssignedTapeId;

isOctave = exist('OCTAVE_VERSION', 'builtin') ~= 0;
if isOctave
     warning('off', 'Octave:mixed-string-concat');
     warning('off', 'Octave:language-extension');
end

if (nargin == 0)
    TapeId = MaxAssignedTapeId;
    return;
elseif (nargin == 2)
    MexFileName = varargin{1};
    Settings    = varargin{2};
else
    error('Incorrect number of input arguments!');
end

if (isempty(MaxAssignedTapeId))
    MaxAssignedTapeId = 1;
else
    MaxAssignedTapeId = MaxAssignedTapeId + 1;
end

TapeId = MaxAssignedTapeId;

if (isunix)
    CopyCommand = 'cp -f';
else
    CopyCommand = 'copy /Y';
end

% Kopieren des erzeugten Tapes
% ==============================
TapeFile         = ['TapeFactory_', MexFileName];
TapeFileNumbered = ['TapeFactory_', int2str(TapeId)];
[res, msg] = system(sprintf('%s %s %s', CopyCommand, [TapeFile, '.', mexext], [TapeFileNumbered, '.', mexext]));

if (res ~= 0)
    disp(msg);
    error('Copy of Tape %s to %s failed! Refer to the error message displayed above for more details!', [TapeFile, '.', mexext], [TapeFileNumbered, '.', mexext]);
    TapeId = -1;
	return;
end


eval([TapeFileNumbered, '(', int2str(TapeId), ')']);

disp(sprintf('Tape successfully opened under id %d', TapeId));
disp(' ');

