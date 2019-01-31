function str = logical2onoff(bool)
% convert logical to 'on' or 'off' strings

bool = logical(bool);

    str = cell(size(bool));
    for n=1:numel(str)
        if bool(n)
            str{n} = 'on';
        else
            str{n} = 'off';
        end
    end
    if numel(str)==1
        str = str{1};
    end

end