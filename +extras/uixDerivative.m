classdef (HandleCompatible) uixDerivative
% Abstract helper class for determining if the GUI Layout Toolbox has been
% installed.
% Classes which use the uix toolbox sould derive from this class
% See: https://www.mathworks.com/matlabcentral/fileexchange/47982-gui-layout-toolbox
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.   

    methods
        function this = uixDerivative()
            if ~exist('uix.Box','class')
                
                fprintf(['This object relies on the GUI Layout Toolbox.\n',...
                    '\tEither install the Toolbox or make sure the +uix directory is on the command path.\n',...
                    '\tSee https://www.mathworks.com/matlabcentral/fileexchange/47982-gui-layout-toolbox for details.\n']);
                
                answer = questdlg(...
                                    sprintf('The GUI Layout Toolbox is required for this class.\nWould you like to download and install it?'),...
                                    'Widgets Toolbox Required',...
                                    'Yes','No','Yes');
                
                if strcmpi(answer,'Yes')

                    try
                        [p,~,~] = fileparts(mfilename('fullpath'));
                        [fn,pth] = uiputfile('*.mltbx','Save GUI Layout Toolbox',fullfile(p,'GUILayoutToolbox.mltbx'));
                        if fn==0
                            error('Download canceled');
                        end
                        outfilename = websave(fullfile(pth,fn),'https://www.mathworks.com/matlabcentral/mlc-downloads/downloads/e5af5a78-4a80-11e4-9553-005056977bd0/5d8c4f7b-ac21-403e-861e-e6c7158a660f/packages/mltbx');

                        % install toolbox
                        installedToolbox = matlab.addons.toolbox.installToolbox(outfilename);
                        fprintf('Installed Toolbox:\n')
                        disp(installedToolbox);

                        delete(outfilename);
                    catch ME
                        warning('Error occured while installing GUI Layout Toolbox');
                        disp(ME.getReport)
                        error('GUI Layout Toolbox is not installed');
                    end
                else
                    error('GUI Layout Toolbox could not be found');
                end
                
            end
        end
    end

end