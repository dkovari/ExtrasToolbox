function [NS_data,filepath] = NanoscopeLoader(varargin)
%% Load Nanoscope Image data
% [NS_info,NS_data,filepath] = NanoscopeLoader(Name,Value,...)
%
% Input Name,Value Options:
%   'FilePath',str: specify filepath of the nanoscope image to load
%                   if not specified, user is prompeted to select file
%   
%   'scale_nm',true/false (default=true):
%               specify if image should be scale to nanometer height
%
%
% Output
%   NS_data: Struct with information about nanoscope file and image data

%% Persistent and Globals
persistent LastDir;

%% Parse Inputs
p = inputParser;

addParameter(p,'Filepath',[]);
addParameter(p,'scale_nm',true,@(x) isscalar(x));
%addParameter(p,'Interactive',true,@isscalar);

parse(p,varargin{:});

%% Set Processing Parameters

%% Choose File
filepath = p.Results.Filepath;
if isempty(filepath)
    num_ext = sprintf('*.%03d;',1:999); %create extension list: '*.001;*.002;...'
    num_ext(end) = []; %get rid of dangling ';'
    [FileName,PathName] = uigetfile({'*.spm','Nanoscope SPM (*.spm)';...
                                     num_ext,'Nanoscope Files (*.###)';...
                                    '*.*','All Files (*.*)'},...
                                    'Select Nanoscope Image File',...
                                    fullfile(LastDir,'*.spm'));
    %do nothing if canceled
    if FileName == 0
        NS_data = [];
        filepath = [];
        return;
    end
    filepath = fullfile(PathName,FileName);
    LastDir = PathName;
end

%% Load File
NS_data = extras.DIreader.get_NS_file_info(filepath);
scale_nm = logical(p.Results.scale_nm);
for n=1:numel(NS_data)
    NS_data(n).ImageData = extras.DIreader.get_NS_img_data(NS_data(n),scale_nm);
end

