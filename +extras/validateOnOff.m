function on_off_str = validateOnOff(val,varargin)
% Validates/converts on/off string
% if logicals (or logical compatible) are passed, they are automatically
% converted to 'on' or 'off'

p = inputParser;
p.addParameter('ArrayInput',true,@(x) isscalar(x));
parse(p,varargin{:});


if isnumeric(val) || islogical(val)
    on_off_str = cell(size(val));
    if ~p.Results.ArrayInput
        assert(isscalar(val),'ArrayInput=false, only scalar input accepted');
    end
    for n=1:numel(val)
        if val(n)
            on_off_str{n} = 'on';
        else
            on_off_str{n} = 'off';
        end
    end
    if numel(on_off_str)==1
        on_off_str = on_off_str{1};
    end
elseif ischar(val) || isstring(val)&&isscalar(val)
    val = lower(val);
    on_off_str = validatestring(val,{'on','off'});
elseif iscellstr(val) && p.Results.ArrayInput
    on_off_str = cell(size(val));
    for n=1:numel(val)
        on_off_str{n} = validatestring(val{n},{'on','off'});
    end
elseif isstring(val) && p.Results.ArrayInput %string array
    on_off_str = cell(size(val));
    for n=1:numel(val)
        on_off_str{n} = validatestring(val(n),{'on','off'});
    end
elseif iscell(val) && p.Results.ArrayInput
    on_off_str = cell(size(val));
    for n=1:numel(val)
        on_off_str{n} = extras.validateOnOff(val{n},p.Results);
    end
else
    if p.Results.ArrayInput
        error('val was unsupported type. Expected types are logical, char array, string, string-array, or cell-str');
    else
         error('val was unsupported type, with ArrayInput=false. Expected types are logical-scalar, char array, string-scalar');
    end
end

    