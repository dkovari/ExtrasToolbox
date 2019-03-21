classdef myD < myB & myC
    methods
        function this = myD()
            initBy = 'myD';
            disp('construct B')
            this@myB(initBy);
            
            disp('construct C')
            this@myC(initBy);
            
            fprintf('create myD, val=%d\n',this.val);
        end
        function delete(this)
            fprintf('delete myD, val=%d\n',this.val);
        end
    end
end