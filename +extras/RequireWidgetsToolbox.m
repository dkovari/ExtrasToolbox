classdef (HandleCompatible) RequireWidgetsToolbox
% Abstract helper class for determining if the Widgets Toolbox has been
% installed.
%   see: https://www.mathworks.com/matlabcentral/fileexchange/66235-widgets-toolbox
% Classes which use the uiw toolbox sould derive from this class

    methods
        function this = RequireWidgetsToolbox()
            if ~exist('uiw.abstract.WidgetContainer','class')
                fprintf('This object relies on the Widgets Toolbox.\n\tEither install the Toolbox or make sure the +uiw directory is on the command path.\n\tSee https://www.mathworks.com/matlabcentral/fileexchange/66235-widgets-toolbox for details.\n');
                
                answer = questdlg(...
                                    sprintf('The Widgets Toolbox is required for this class.\nWould you like to download and install it?'),...
                                    'Widgets Toolbox Required',...
                                    'Yes','No','Yes');
                
                if strcmpi(answer,'Yes')

                    try
                        [p,~,~] = fileparts(mfilename('fullpath'));
                        [fn,pth] = uiputfile('*.mltbx','Save Widgets Toolbox',fullfile(p,'WidgetsToolbox.mltbx'));
                        if fn==0
                            error('Download canceled');
                        end
                        outfilename = websave(fullfile(pth,fn),'https://www.mathworks.com/matlabcentral/mlc-downloads/downloads/b0bebf59-856a-4068-9d9c-0ed8968ac9e6/a43556a9-3eb1-4d86-9284-3ca86b8f22c7/packages/mltbx');

                        % install toolbox
                        installedToolbox = matlab.addons.toolbox.installToolbox(outfilename);
                        fprintf('Installed Toolbox:\n')
                        disp(installedToolbox);

                        delete(outfilename);
                    catch ME
                        warning('Error occured while installing Widgets Toolbox');
                        disp(ME.getReport)
                        error('Widgets Toolbox is not installed');
                    end
                else
                    error('Widgets Toolbox could not be found');
                end
            end
        end
    end
end