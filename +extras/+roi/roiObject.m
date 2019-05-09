classdef roiObject < handle & matlab.mixin.Heterogeneous & matlab.mixin.SetGet
% roi = ImageTracker.roiObject(Window)
% 
% ROI window object, with unique identifier
%
% Construction:
%============================
% ROI(1:n) = ImageTracker.roiObject( [n x 4])
% Windows must be n x 4 array
%   number of rows corresponds to the number of roi objects created
%
% Window: 1x4 array specifying [x0,y0,w,h] of rectangle
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    properties (SetAccess=immutable)
        UUID
    end
    properties (SetObservable=true,AbortSet=true)
        Window = NaN(1,4); %[x,y,w,h]
    end
    
    events
        PropertyChanged
    end
    
    %% Dependent Dimensions
    properties(Dependent)
        Window_X1
        Window_X2
        Window_Y1
        Window_Y2
        Window_Width
        Window_Height
    end
    methods
        
        function s = toStruct(this)
            s = struct('Window',{this.Window},'UUID',{this.UUID});
        end
        
        function v = get.Window_X1(this)
            v = this.Window(1);
            
        end
        function set.Window_X1(this,v)
           this.Window(1) = v; 
        end
        
        function v = get.Window_X2(this)
            v = this.Window(1)+this.Window(3);
        end
        function set.Window_X2(this,v)
           this.Window(3) = v-this.Window(1);
        end
        
        function v = get.Window_Y1(this)
            v = this.Window(2);
        end
        function set.Window_Y1(this,v)
            this.Window(2) = v;
           
        end
        
        function v = get.Window_Y2(this)
            v = this.Window(2)+this.Window(4);
        end
        function set.Window_Y2(this,v)
           this.Window(4) = v-this.Window(2);
        end
        
        function v = get.Window_Width(this)
            v = this.Window(3);
        end
        function set.Window_Width(this,v)
           this.Window(3) = v;
        end
        
        function v = get.Window_Height(this)
            v = this.Window(4);
        end
        function set.Window_Height(this,v)
           this.Window(4) = v;
        end
    end
    
    %% create/delete
    methods
        function this = roiObject(Window)
            %% set window
            if ~exist('Window','var')
                Window = NaN(1,4);
            end
            assert(size(Window,2)==4, 'Window must be n x 4 array');
            
            for n=1:size(Window,1)
                this(n).Window = Window(n,:);
                this(n).UUID = char(java.util.UUID.randomUUID);
            end
            
            addlistener(this,'Window','PostSet',@(~,~) notify(this,'PropertyChanged'));
        end
    end
    
    %% Get/Set
    methods
        function set.Window(this,val)
            assert(numel(val)==4);
            this.Window = reshape(val,1,4);
        end
    end
    
    %% Overloads
    methods
%         function sROI = struct(ROI)
%             sROI(numel(ROI)) = struct('UUID','','Window',zeros(1,4));
%             [sROI.UUID] = deal(ROI.UUID);
%             [sROI.Window] = deal(ROI.Window);
%             sROI = reshape(sROI,size(ROI));
%         end
        function tf = eq(ROI,B) %roi==...
            if ischar(B) %ROI=='...' -> test UUID
                tf = false(size(ROI));
                for n=1:numel(ROI)
                    tf(n) = isequal(ROI(n).UUID,B);
                end
            elseif iscellstr(B) % ROI=={'...','...'} -> test UUID
                if numel(ROI)==1
                    tf =false(size(B));
                    for n=1:numel(B)
                        tf(n) = isequal(ROI.UUID,B{n});
                    end
                elseif numel(B)==1
                    tf = false(size(ROI));
                    for n=1:numel(ROI)
                        tf(n) = isequal(ROI(n).UUID,B{1});
                    end
                else
                    assert(all(size(ROI)==size(B)),'dimension of LHS and RHS do not match');
                    tf = false(size(ROI));
                    for n=1:numel(ROI)
                        tf(n) = isequal(ROI(n).UUID,B{n});
                    end
                end
            elseif isnumeric(B) && (numel(B)==4||size(B,2)==4) % ROI == [n x 4] -> test window
                if numel(B)==4
                    B=reshape(B,1,4);
                end
                
                if numel(ROI)==1
                    tf =false(size(B,1),1);
                    for n=1:size(B,1)
                        tf(n) = isequal(ROI.Window,B(n,:));
                    end
                elseif size(B,1)==1
                    tf = false(size(ROI));
                    for n=1:numel(ROI)
                        tf(n) = isequal(ROI(n).Window,B);
                    end
                else
                    assert(numel(ROI)==size(B,1),'numel of LHS and number of rows in RHS do not match');
                    tf = false(size(ROI));
                    for n=1:numel(ROI)
                        tf(n) = isequal(ROI(n).Window,B(n,:));
                    end
                end
            else %default
                %tf = builtin('eq',ROI,B);
                tf = eq@handle(ROI,B);
            end
        end
        function tf = ne(ROI,B) %roi==...
            %if isa(B,class(ROI))
            if isa(B,mfilename('class'))
                tf = ne@handle(ROI,B);
            else
                tf = ~eq(ROI,B);
            end
        end
    end
end