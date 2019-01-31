function I_PTH = IncludePath()
% Return path to c++ include directory

[pth,~,~] = fileparts(mfilename('fullpath'));

I_PTH = fullfile(pth,'include');
