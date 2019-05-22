function pt = getClick(hax)
% Wait until user clicks point in axes
% if user presses escape then pt returns empty
    if nargin<1
        hax=gca;
    end
    
    orig_ButtonDwn = hax.Parent.WindowButtonDownFcn;
    orig_Key = hax.Parent.WindowKeyPressFcn;
    
    drawnow();

    pt = [];
    bDwn = false;
    escape = false;
    function MouseClick(~,evt)
        pt = get(hax, 'CurrentPoint');
        bDwn = true;
    end

    function KeyFn(~,evt)
        val = uint8(evt.Character);
        if val==27
            escape = true;
        end
    end

    hax.Parent.WindowButtonDownFcn = @MouseClick;
    hax.Parent.WindowKeyPressFcn = @KeyFn;
    
    while ~bDwn && ~escape && isvalid(hax)
        drawnow();
    end
    
    try
    hax.Parent.WindowButtonDownFcn = orig_ButtonDwn;
    hax.Parent.WindowKeyPressFcn = orig_Key;
    catch ME
        disp(ME.getReport)
    end
    if escape
        pt = [];
    end


end

