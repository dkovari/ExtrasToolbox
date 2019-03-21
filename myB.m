classdef myB < myA
    
    methods
        function this = myB(varargin)
            initBy = 'myB';
            if nargin>0
                initBy = [initBy,':',varargin{1}];
            end
            this@myA(initBy);
            fprintf('create myB, val=%d, initBy: %s\n',this.val,initBy);
        end
        function delete(this)
            fprintf('delete myB, val=%d\n',this.val);
        end
    end
end