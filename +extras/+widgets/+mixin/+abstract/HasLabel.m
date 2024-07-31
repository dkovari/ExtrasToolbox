classdef (Abstract) HasLabel < handle & extras.widgets.mixin.AssignNV
    % Helper class for graphical objects which have a label
    
    properties(SetObservable=true,AbortSet=true)
        Label %char array or string specifying label text
    end
    
    properties(Access=protected)
        StringObjects = {}; % list of objects who's "String" Property should be Set
    end
    
    %% hidden
    methods (Hidden,Static)
        function ApplyStringToObject(string,objects)
        % set the String property of each object in the obects
            if isempty(objects)
                return;
            end
            if iscell(objects)
                for n=1:numel(objects)
                    extras.widgets.mixin.abstract.HasLabel.ApplyStringToObject(string,objects{n});
                end
                return;
            end
            
            if numel(objects)>1
                for n = 1:numel(objects)
                    if isprop(objects(n),'String')
                        try
                            objects(n).String = string;
                        catch ME
                            disp(ME.getReport);
                            warning('Could not set String');
                        end
                    end
                end
            elseif isprop(objects,'String')
                try
                    objects.String = string;
                catch ME
                    disp(ME.getReport);
                    warning('Could not set String');
                end
            end
        end
    end
    
    %% get/set
    methods 
        function set.Label(this,val)
            if isstring(val)
                val = char(val);
            end
            if isnumeric(val)
                val = num2str(val);
            end
            assert(ischar(val),'Only char array can be used for Label');
            this.Label = val;
            this.ApplyStringToObject(val,this.StringObjects);
            this.onLabelChanged();
        end
    end
    
    %% Override
    methods(Access=protected)
        function onLabelChanged(~)
        end
    end
    
    %% StringObjects
    methods (Access=protected)
        function addLabelStringObject(this,obj)
            if isempty(obj)
                return;
            end
            if iscell(obj) %cell containing objects
                for n=1:numel(obj)
                    this.addStringObject(obj{n});
                end
                return;
            elseif numel(obj)>1
                for n=1:numel(obj)
                    this.addStringObject(obj(n));
                end
                return;
            else %scalar
                for n=1:numel(this.StringObjects)
                    if isequal(this.StringObjects{n},obj)
                        return; %don't add, already there
                    end
                end
                this.StringObjects{end+1} = obj; %add to end
            end
        end
        
        function removeLabelStringObject(this,obj)
            if isempty(obj)
                return;
            end
            if iscell(obj) %cell containing objects
                for n=1:numel(obj)
                    this.removeStringObject(obj{n});
                end
                return;
            elseif numel(obj)>1
                for n=1:numel(obj)
                    this.removeStringObject(obj(n));
                end
                return;
            else %scalar
                for n=1:numel(this.StringObjects)
                    if isequal(this.StringObjects{n},obj)
                        this.StringObjects(n) = [];
                        return; %done
                    end
                end
            end
        end
    end
end