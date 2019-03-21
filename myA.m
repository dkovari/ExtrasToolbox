classdef (Abstract) myA < handle
    
    properties(SetAccess=immutable)
        val
    end
    methods
        function this = myA(varargin)
            this.val = randi(1000);
            initBy = 'myA';
            if nargin>0
                initBy = [initBy,':',varargin{1}];
            end
            
            fprintf('create myA, val=%d, initBy: %s\n',this.val,initBy);
            
            
            
        end
        function delete(this)
            fprintf('delete myA, val=%d\n',this.val);
        end
    end
end