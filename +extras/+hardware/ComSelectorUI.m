classdef ComSelectorUI < extras.GraphicsChild & extras.RequireGuiLayoutToolbox
%% UI for managing the com port for extras.hardware.SerialDevice objects
%
% extras.hardware.ComSelectorUI(...)
%==========================================================================
%   obj = ComSelectorUI(serialdevice)
%   obj = ComSelectorUI(parent,serialdevice)
%           parent is the graphics parent in which ui is created
%   obj = ComSelectorUI(serialdevice,'Parent',parent)
%   obj =ComSelectorUI('SerialDevice',serialdevice,'Parent',parent)
%==========================================================================
%% Copyright 2019 Daniel T. Kovari, Emory University
%   All rights reserved.   
    
    properties (SetAccess=protected)
        serialdevice
    end
    
    properties (Access=protected)
        OuterContainer
        com_popmenu;
        com_string = [];
        
        con_discon_button;
        
        ComListTimer = timer();
        
        SerialDeleteListener
        ConnectedListener
        
        SerialDeviceNameListener;
    end
    
    %% Create
    methods
        function this = ComSelectorUI(varargin)
        % Create ComSelectorUI
        %   obj = ComSelectorUI(serialdevice)
        %   obj = ComSelectorUI(parent,serialdevice)
        %           parent is the graphics parent in which ui is created
        %   obj = ComSelectorUI(serialdevice,'Parent',parent)
        %   obj =ComSelectorUI('SerialDevice',serialdevice,'Parent',parent)
        
            
            
            %% Look for serialdevice
            if numel(varargin)<1
                error('serialdevice not specified');
            end
            
            found_serialdevice = false;
            %no parent, sd as first arg
            if isa(varargin{1},'extras.hardware.SerialDevice')
                serialdevice = varargin{1};
                varargin(1)=[];
                found_serialdevice = true;
            end
            
            %name,value pair
            if numel(varargin)>1
                ind = find(strcmpi('SerialDevice',varargin));
                if found_serialdevice && numel(ind)>0
                    error('First argument was a serial device and ''SerialDevice'' name-value pair was also specified');
                end

                if numel(ind)>1
                    error('''SerialDevice'' name-value pair was specified multiple times');
                end

                if numel(ind)==1
                    serialdevice = varargin{ind+1};
                    found_serialdevice = true;
                    varargin(ind:ind+1) = [];
                end
            end
            
            %second arg, assume first arg is parent
            if numel(varargin)>1
                if isa(varargin{2},'extras.hardware.SerialDevice')
                    serialdevice = varargin{2};
                    varargin(2)=[];
                    found_serialdevice = true;
                end
            end
            
            %didn't find
            if ~found_serialdevice
                error('serial device was not specified in the arguments');
            end
            
            %% initiate graphics parent related variables
            this@extras.GraphicsChild(@() figure('menubar','none','Name',[serialdevice.DeviceName,' COM Selector'],'NumberTitle','off'));
            %look for parent specified in arguments
            varargin = this.CheckParentInput(varargin{:});
            
            %% if created parent, set listener to change title when device title changes
            if this.CreatedParent
                this.SerialDeviceNameListener = addlistener(serialdevice,'DeviceName','PostSet',@(~,~) set(this.Parent,'Name',[serialdevice.DeviceName,' COM Selector']));
            end
            
            %% Set Serial Device
            this.serialdevice = serialdevice;
            %% Create GUI Elements
            this.OuterContainer = uix.VBox('Parent',this.Parent);
            
            hb = uix.HBox('Parent',this.OuterContainer);
            this.OuterContainer.Heights = 40;
            
            uicontrol('Parent',hb,...
                'style','text',...
                'string','COM Port:');
            
            list = extras.hardware.SerialDevice.serialportlist();
            if isempty(list)
                list = ' ';
            end
            
            this.com_popmenu = uicontrol('Parent',hb,...
                'style','popupmenu',...
                'String',list,...
                'Enable','on',...
                'Callback',@(~,~) this.ChangePopMenu());
            
            this.con_discon_button = uicontrol('Parent',hb,...
                'style','pushbutton',...
                'String','Connect',...
                'Callback',@(~,~) this.ToggleConnect());
            
            hb.Widths = [70,-1,110];
            
            %change figure size if created parent
            if this.CreatedParent
                this.Parent.Units = 'pixels';
                this.Parent.Position(4) = 45;
                this.Parent.Position(3) = 400;
            end
            
            %% Listeners
            %for serial device delete
            this.SerialDeleteListener = addlistener(this.serialdevice,'ObjectBeingDestroyed',@(~,~) delete(this));
            
            %listener for connection change
            this.ConnectedListener = addlistener(this.serialdevice,'connected','PostSet',@(~,~) this.ConnectedChanged());
                
            
            %% Timer for com list
            this.ComListTimer = timer(...
                'BusyMode','drop',...
                'Period',2,...
                'ObjectVisibility','off',...
                'TimerFcn',@(~,~) this.UpdateComList());
            
            start(this.ComListTimer);

            
        end
        
        function delete(this)
            delete(this.SerialDeviceNameListener);
            delete(this.SerialDeleteListener);
            delete(this.ConnectedListener);
            
            stop(this.ComListTimer);
            delete(this.ComListTimer);
            
            delete(this.OuterContainer);
            
        end
    end
    
    %% Callbacks
    methods (Hidden)
        function ToggleConnect(this)
            if ~this.serialdevice.connected %not connected, try to connect
                if isempty(this.com_string)
                    warndlg('Select COM Port before connecting','Select COM');
                    return;
                end
                
                this.serialdevice.ConnectCOM(this.com_string);
            else
                this.serialdevice.DisconnectCOM();
            end
        end
        
        function ConnectedChanged(this)
            if this.serialdevice.connected
                this.con_discon_button.String = 'Disconnect';
                this.com_popmenu.Enable = 'off';
            else
                this.con_discon_button.String = 'Connect';
                this.com_popmenu.Enable = 'on';
            end
        end
        
        function ChangePopMenu(this)
            this.com_string = this.com_popmenu.String{this.com_popmenu.Value};
        end
        
        function UpdateComList(this)
            if ~isvalid(this)
                return
            end
            list = extras.hardware.SerialDevice.serialportlist();
            if isempty(list)
                this.com_popmenu.String = ' ';
            else
                this.com_popmenu.String = list;
            end
        end
        
    end
end