function view_NS_img(filename,varargin)
% View Nanoscope file
%
% Input:
%   filename: string specifying path to file
%             if not specified, or filename=[] then the user is prompted to
%             select a file using uigetfile().
%
% Parameters:
%   'ImageType',val (default=[]) if specified the viewer will only load images
%                                with the field Type=val.
%                                Use this option to only load a specific
%                                measurement dataset (like 'Height')

import DIreader.*;

%% Input Parser
p = inputParser;
p.CaseSensitive = false;
addParameter(p,'ImageType',[],@(x) isempty(x)||ischar(x));

parse(p,varargin{:});


%% get file
persistent LastDir;
if nargin<1 || isempty(filename)
    [FileName,PathName] = uigetfile({'*.spm;*.001;*.003','Nanoscope Files';'*.*','All Files (*.*)'},'Select Nanoscope File',fullfile(LastDir,'*.spm'));
    if FileName == 0
        return;
    end
    LastDir = PathName;
    filename = fullfile(PathName,FileName);
end

[~,F_name,~] = fileparts(filename);

NS_data = DIreader.get_NS_file_info(filename);

if isempty(NS_data)
    error('The specified file did not contain any images');
end

%% Filter by image type
if ~isempty(p.Results.ImageType)
    NS_data = NS_data(strcmpi(p.Results.ImageType),{NS_data.type});
end

%% Default colormap
try
    cmap = load('ZV_cmap.mat','cmap_zsuzsi');
catch
    cmap = 'pink';
end

%% Display each image
for n=1:numel(NS_data)
    data = get_NS_img_data(NS_data(n), 1); %read the data
    
    %create figure
    figure('Name',[F_name,':',NS_data(n).type]);
    imagesc([0,NS_data(n).width],[0,NS_data(n).height],data);
    axis('image');
    
    %set colormap
    colormap(cmap);
    %colorbar
    h = colorbar();
    if strcmpi(NS_data(n).type, 'Height')
        h.Label.String = 'Height (nm)';
    else
        h.Label.String = [NS_data(n).type,' (uncalibrated)'];
    end
    imcontrast(gca); % provide a contrast adjustment tool
    
end
