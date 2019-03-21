classdef myC < myA
    methods
        function this = myC(varargin)
            initBy = 'myC';
            if nargin>0
                initBy = [initBy,':',varargin{1}];
            end
            this@myA(initBy);
            fprintf('create myC, val=%d, initBy: %s\n',this.val,initBy);
        end
        function delete(this)
            fprintf('delete myC, val=%d\n',this.val);
        end
    end
end