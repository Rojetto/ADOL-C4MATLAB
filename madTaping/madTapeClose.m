function result = mexAD_CloseTape(TapeId)

% result = mexAD_CloseTape(TapeId)
%
% Closes the tape with the id TapeId. The command leads to the deletion
% of the temporary files 
%
% ADOLC-Locations_X.tap
% ADOLC-Operations_X.tap
% ADOLC-Values_X.tap
%
% where X is the TapeId.
%
% Call madTapeClose('all') to close all opened tapes.
%
% The original tapes stored as
% ADOLC-Locations_FileBaseName.tap
% ADOLC-Operations_FileBaseName.tap
% ADOLC-Values_FileBaseName.tap
%
% from which 
% ADOLC-Locations_X.tap
% ADOLC-Operations_X.tap
% ADOLC-Values_X.tap
%
% have been created are not deleted.
% See also: madTapeCreate, madTapeOpen

% (c) 2010-2012 
% Carsten Friede, Jan Winkler
% Institut für Regelungs- und Steuerungstheorie
% TU Dresden
% Jan.Winkler@tu-dresden.de

% Erst mal nur in den Papierkorb damit
OldState = recycle('on');

result = 0;
NumTapeFiles       = 3;

TapePraefix{1}    = 'ADOLC-Locations_';
TapePraefix{2}    = 'ADOLC-Operations_';
TapePraefix{3}    = 'ADOLC-Values_';


% Löschen aller nummerierten Tapes
if (strcmp(TapeId, 'all'))
    files = dir(pwd);
    for i=1:1:length(files)
        if ( ~isempty(findstr(files(i).name, TapePraefix{1})) | ...
                ~isempty(findstr(files(i).name, TapePraefix{2})) | ...
                ~isempty(findstr(files(i).name, TapePraefix{3})) )
            delete(files(i).name);
        end
    end

% Löschen des Tapes mit der Nummer TapeId
else
    for i = 1:1:NumTapeFiles
        TapeFile{i} = strcat(TapePraefix{i}, num2str(TapeId),'.tap');
        if (exist(TapeFile{i}))
            delete(TapeFile{i});
        else    
            result = -1;
        end
    end

    if (result == 0)
        disp(sprintf('Tape \n %s \n %s \n %s \nsuccessfully unloaded!', TapeFile{1}, TapeFile{2}, TapeFile{3}));
        disp(' ');
    end


end

recycle(OldState);

