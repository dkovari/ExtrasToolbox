classdef TargetValueDevice < matlab.mixin.SetGet & matlab.mixin.Heterogeneous & extras.widgets.mixin.HasDeviceName
% Generic Class for devices which have a "Target" set point & actual "Value"
%
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.

    %% Abstract Properties
    properties (Abstract=true,SetAccess=protected,SetObservable=true,AbortSet=true)
        Value;
    end
    properties (Abstract=true,SetObservable=true) %allow setting TargetValue to same TargetValue, that way  message gets sent again
        Target;
    end
    
    %% Non-Abstract Props
    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        UpdatedAfterTargetChange;
    end
    properties (SetAccess=protected, SetObservable=true, AbortSet = true)
        Units = '';
        Limits = [-Inf,Inf];
        ValueSize = [1,1];
        ValueLabels = '';
    end
    methods
        function set.Limits(this,val)
            
            %validate
            ok = false;
            if isnumeric(val)&&numel(val)==2
                val = reshape(val,1,2);
                ok = true;
            elseif iscell(val) && all(size(val)==this.ValueSize)
                ok = true;
                for n=1:numel(val)
                    if ~isnumeric(val{n})||numel(val{n})~=2
                        ok=false;
                        break;
                    end
                    val{n} = reshape(val{n},1,2);
                end
            end
            assert(ok, 'Limits must be 1x2 numeric or cell array containing 1x2 numerics that is the same size as ValueSize');
            
            this.Limits = val;
        end
    end
    
    %% UI Factory, optionally overloadable
    methods
        function GUI_Obj = createUI(this,varargin)
        % Factory method which creates UI for controlling device
        % Syntax:
        %   this: calling class
        %   varargin: optionally forward arguments to UI constructor
        %
        % Unless you overload this method, it will return a
        % extras.hardware.TargetValueDeviceUI object.
            GUI_Obj = extras.hardware.TargetValueDeviceUI(varargin{:},'Device',this);
        end
    end
    
    %% MISC Public Methods
    methods
        function S = struct(this)
        %convert public properties inherited from TargetValueDevice Class into struct
            S = struct('Value',{this.Value},'Target',{this.Target},'Units',{this.Units},'Limits',{this.Limits},'ValueLabels',{this.ValueLabels});
        end
    end
end