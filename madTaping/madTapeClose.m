function result = mexAD_CloseTape(TapeId)

% result = mexAD_CloseTape(TapeId)
%
% Closes the tape with the id TapeId. The command leads to the deletion
% of the temporary files 
%
% ADOLC-Locations_X.tap
% ADOLC-Operations_X.tap
% ADOLC-Values_X.tap
% TapeFactory_X.(mexw32|mexw64)
%
% where X is the TapeId.
%
% Call madTapeClose('all') to close all opened tapes.
%
% The original tapes stored as
% TapeFactory_FileBaseName.(mexext)
% are not deleted.
% See also: madTapeCreate, madTapeOpen

% (c) 2010-2018
% Carsten Friede, Jan Winkler, Mirko Franke
% Institut für Regelungs- und Steuerungstheorie
% TU Dresden
% {Jan.Winkler, Mirko.Franke}

% Erst mal nur in den Papierkorb damit
OldState = recycle('on');

result = 0;
NumTapeFiles       = 3;

TapePraefix{1}    = 'ADOLC-Locations_';
TapePraefix{2}    = 'ADOLC-Operations_';
TapePraefix{3}    = 'ADOLC-Values_';

TapeFactoryPraefix = 'TapeFactory_';


% Löschen aller nummerierten Tapes
if (strcmp(TapeId, 'all'))
    files = dir(pwd);
    for i=1:1:length(files)
        if ( ~isempty(findstr(files(i).name, TapePraefix{1})) | ...
                ~isempty(findstr(files(i).name, TapePraefix{2})) | ...
                ~isempty(findstr(files(i).name, TapePraefix{3})) )
            delete(files(i).name);
        end
		if ( regexp(files(i).name, ['TapeFactory_[0-9]+\.', mexext]) )
			[pn,fn,en]=fileparts(files(3).name);
			eval(['clear ', fn]);
            delete(files(i).name);
        end
    end

% Löschen des Tapes mit der Nummer TapeId
else
    for i = 1:1:NumTapeFiles
        TapeFile{i} = strcat(TapePraefix{i}, num2str(TapeId),'.tap');
        if (exist(TapeFile{i}))
            delete(TapeFile{i});
        end
    end
	
	TapeFactoryFile = strcat(TapeFactoryPraefix, num2str(TapeId),'.', mexext);
	if (exist(TapeFactoryFile))
		eval(['clear ', TapeFactoryPraefix, num2str(TapeId)]);
		delete(TapeFactoryFile);
	else    
        result = -1;
    end
	
    if (result == 0)
		disp(sprintf('Tape # %d successfully unloaded!\n', TapeId));
    end
end

recycle(OldState);