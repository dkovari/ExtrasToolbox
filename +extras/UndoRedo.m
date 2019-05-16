classdef UndoRedo <  matlab.mixin.SetGet
% Implements and Undo and Redo system
%
% Usage:
%   >> UD = UndoRedo(3); %create UndoRedo object with intial value =3
%   >> UD.applyOp(@(x,y) x+y, 7); %add 7 to initial value (val->10)
%   >> UD.applyOp(@(x) x*50); %multiply by 50 (val->500)
%   >> UD.undo %undo last op (val->10)
%   >> UD.redo %redo op (val->500)
%
% Properties
%   BufferLength (default=10): number of pre-computed values to store in
%       the undo and redo buffers.
%       If more Operations have been applied, then user will be prompted
%       as to whether the operations should be re-applied from the begining

    
    %% Create
    methods
        function this = UndoRedo(InitData)
            this.InitialData = InitData;
            this.DataUndo{1} = InitData;
        end
    end
    
    %% Internal Use Only
    properties %(Access=protected)
        InitialData;
        DataUndo = {}; 
        DataRedo = {};
                
        OpList = {} %array holding history of operations
        OpListRedo = {};
        OpArgs = {}
        OpArgsRedo = {};
    end
    methods
        function set.OpList(this,val)
            this.OpList = val;
            this.UndoAvailable = numel(val)>0;
        end
        function set.OpListRedo(this,val)
            this.OpListRedo = val;
            this.RedoAvailable = numel(val)>0;
        end
    end
    
    %% Public visible
    properties (SetAccess=protected,SetObservable=true,AbortSet=true)
        CurrentData; % current value
        UndoAvailable = false; %t/f flag if undo actions are available
        RedoAvailable = true; %t/f flag if redo actions are available
    end
    
    properties
        BufferLength = 10; %max number of steps in undo/redo buffer
    end
    methods
        function set.BufferLength(this,len)
            assert(isscalar(len)&&isnumeric(len)&&len>=1,'Buffer Length must be scalar >1');
            this.BufferLength = len;
            
            %% change buffer lengths
            if numel(this.DataUndo)>len
                this.DataUndo = this.DataUndo(end-(len-1):end);
            end
            if numel(this.DataRedo)>len
                this.DataRedo = this.DataRedo(end-(len-1):end);
            end
        end
        function set.DataUndo(this,D)
            if numel(D)>this.BufferLength
                this.DataUndo = D(end-(this.BufferLength-1):end);
            else
                this.DataUndo = D;
            end
            this.CurrentData = this.DataUndo{end};
        end
        function set.DataRedo(this,D)
            if numel(D)>this.BufferLength
                this.DataRedo = D(end-(this.BufferLength-1):end);
            else
                this.DataRedo = D;
            end
        end
    end
    
    methods
        function this = applyOp(this,op_fn,varargin)
        % Apply the operation to the data
        %   op_fn: function to use on data
        %   should have syntax:
        %       x = op_fn(x,varargin)
            
            %% apply the operation
            this.DataUndo{end+1} = ...
                feval(op_fn,this.DataUndo{end},varargin{:});
            %% clear redo
            this.DataRedo = {};
            this.OpListRedo = {};
            this.OpArgsRedo = {};
            
            %% add variables to OpList
            this.OpList{end+1} = op_fn;
            this.OpArgs{end+1} = varargin;
        end
        
        function this = undo(this,ForceUndo)
        %Undo last operation
        %   ForceUndo (optional): t/f flag if undo should be forced, even
        %   if undo buffer is empty
        %   If ForceUndo is not specified, user is prompted when the buffer
        %   is exhausted.
            if numel(this.DataUndo)<2 && ~isempty(this.OpList)
                if nargin<2
                    answer = questdlg(sprintf(['There is no data in the Undo Buffer.\n',...
                        'Would you like to re-apply operations up to previous step?']),...
                        'Undo Buffer Empty','Yes','No','Yes');
                    ForceUndo = strcmpi(answer,'Yes');
                end
                if ~ForceUndo
                    return;
                end
                OL = this.OpList;
                OA = this.OpArgs;
                this.OpList = {};
                this.OpArgs = {};
                
                lastData = this.DataUndo{end};
                
                DR = this.DataRedo;
                OLR = this.OpListRedo;
                OAR = this.OpArgsRedo;
                
                this.DataUndo{1} = this.InitialData;
                
                t1 = tic;
                hWB = 0;
                NOP = numel(OL)-1;
                for n=1:NOP
                    this.applyOp(OL{n},OA{n}{:});
                    if toc(t1)>2 && n<NOP
                        if hWB ==0 %need to create waitbar
                            hWB = waitbar(n/NOP,sprintf('Applying Op %d/%d',n,NOP),'CreateCancelBtn',@(h,~) delete(ancestor(h,'figure')));
                        elseif ~isvalid(hWB) %user canceled
                            this.DataRedo = DR;
                            this.OpListRedo = OLR;
                            this.OpArgsRedo = OAR;
                            this.OpList = OL;
                            this.OpArgs = OA;
                            this.DataUndo{1} = lastData;
                            return;
                        else
                            waitbar(n/NOP,hWB,sprintf('Applying Op %d/%d',n,NOP));
                        end
                    end    
                end
                try
                    delete(hWB)
                catch
                end
                
                this.DataRedo = [DR,lastData];
                this.OpListRedo = OLR;
                this.OpListRedo{end+1} = OL{end};
                
                this.OpArgsRedo = OAR;
                this.OpArgsRedo{end+1} = OA{end};
                
            elseif numel(this.DataUndo)>1
                this.DataRedo{end+1} = this.DataUndo{end};
                this.OpListRedo{end+1} = this.OpList{end};
                this.OpArgsRedo{end+1} = this.OpArgs{end};
                
                this.OpList(end) = [];
                this.OpArgs(end) = [];
                this.DataUndo(end) = [];
            else
                warning('nothing to undo');
            end
        end
        
        function this = redo(this)
            if ~isempty(this.DataRedo)
                this.DataUndo{end+1} = this.DataRedo{end};
                this.OpList{end+1} = this.OpListRedo{end};
                this.OpArgs{end+1} = this.OpArgsRedo{end};
                this.DataRedo(end) = [];
                this.OpListRedo(end) = [];
                this.OpArgsRedo(end) = [];
            elseif ~isempty(this.OpListRedo) %redo data isempty, try to apply OpListRedo operation
                OLR = this.OpListRedo;
                OAR = this.OpArgsRedo;
                
                this.applyOp(OLR{end},OAR{end}{:});
                this.OpListRedo = OLR(1:end-1);
                this.OpArgsRedo = OAR(1:end-1);
                
            else
                warning('nothing to redo');
            end
        end
    end

end