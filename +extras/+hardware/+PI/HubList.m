classdef HubList < handle
%% Helper class for managing lists of extras.PI.MercuryHub objects
    
    properties (SetAccess=protected)
        List = extras.hardware.PI.MercuryHub.empty
        
        DeleteListener;
    end
    
    methods
        function delete(this)
            delete(this.DeleteListener);
            %delete(this.List);
        end
        
        function add(this,Hubs)
            assert(all(isa(Hubs,'extras.hardware.PI.MercuryHub')),'Hubs must be extras.hardware.PI.MercuryHub class');
            this.List = union(this.List,Hubs);
            
            delete(this.DeleteListener);
            this.DeleteListener = addlistener(this.List,'ObjectBeingDestroyed',@(h,~) this.remove(h));
        end
        function remove(this,Hubs)
%             disp(Hubs)
            assert(all(isa(Hubs,'extras.hardware.PI.MercuryHub')),'Hubs must be extras.hardware.PI.MercuryHub class');
%             for n = 1:numel(Hubs)
%                 try
%                     fprintf('removing: %s\n',Hubs(n).Port);
%                 catch
%                 end
%             end
            this.List = setdiff(this.List,Hubs);
            
            delete(this.DeleteListener);
            this.DeleteListener = addlistener(this.List,'ObjectBeingDestroyed',@(h,~) this.remove(h));
        end
        
        %% Number of Args overload
        function n = numArgumentsFromSubscript(this,s,indexingContext)
            if ~strcmp(s(1).type,'{}')
                %'here'
                n = builtin('numArgumentsFromSubscript',this,s,indexingContext);
                return
            end
            
            assert( numel(s(1).subs)<=1,'Multi-dimensional indexing is not supported');

            if isempty(s(1).subs) || ischar(s(1).subs{1})&&strcmp(s(1).subs{1},':') % obj{} or obj{:}
                n = numel(this.List);
            else
                if iscell(s(1).subs{1})
                    n = numel(s(1).subs{1});
                else
                    n=1;
                end
            end
        end
        
        %% overload subref
        function varargout = subsref(this,s)
            %% Forward non bracket subreferences
            if ~ismember(s(1).type,{'{}','()'})
                [varargout{1:nargout}] = builtin('subsref',this,s);
                return;
            end
            
            assert(numel(s(1).subs)<=1,'Multi-dimension indexing is not allowed');
            
            %% Get list of values to operate on
            if isempty(s(1).subs) || ischar(s(1).subs{1})&&strcmp(s(1).subs{1},':') % Handle no arguments {} and {:} --> returns all
                hlist = this.List;
            elseif isempty(s(1).subs{1}) %return nothing
                hlist = extras.hardware.PI.MercuryHub.empty;
            else% indicies specified
                assert(isnumeric(s(1).subs{1}),'Subscripts must be numeric');
                if any(s(1).subs{1}<1 |  s(1).subs{1}>numel(this.List))
                    error('One of the specified indices is not a valid.');
                end
                hlist = this.List(s(1).subs{1});
            end
            
            if strcmp(s(1).type,'()') && length(s)==1 % ans = obj(...);
                varargout{1} = hlist;
            else
                outvals = cell(size(hlist));
                for n=1:numel(hlist)
                    outvals{n} = hlist(n);
                end
                if length(s)>1
                    try
                        for n=1:numel(hlist)
                            outvals{n} = builtin('subsref',outvals{n},s(2:end));
                        end
                    catch ME
                        disp(ME.getReport)
                        error('Error while applying sub-ref operation to Map Values');
                    end
                end
                
                if strcmp(s(1).type,'{}') %each index goes to separate output variable
                    varargout = outvals;
                else %'()' %idecies are grouped in a cell array
                    varargout{1} = outvals;
                end
            end
            
        end
    end
    
end