classdef (HandleCompatible) uixDerivative
% Abstract helper class for determining if the GUI Layout Toolbox has been
% installed.
% Classes which use the uix toolbox sould derive from this class

    methods
        function this = uixDerivative()
            if ~exist('uix.Box','class')
                fprintf('This object relies on the GUI Layout Toolbox.\nEither install the Toolbox or make sure the \n+uix directory is on the command path.\See https://www.mathworks.com/matlabcentral/fileexchange/47982-gui-layout-toolbox for details');
                error('GUI Layout Toolbox could not be found');
            end
        end
    end

end