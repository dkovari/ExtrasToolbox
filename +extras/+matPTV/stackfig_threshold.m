function thresh = stackfig_threshold(stack,thresh)
% GUI Tool for selecting image threshold
%
% Input:
%   stack: stack data in cell or [w,h,nF] format
%   thresh: initial threshold;

%% convert stack to cell if needed
if isnumeric(stack)
    s2 = cell(size(stack,3),1);
    for n=1:size(stack,3)
        s2{n} = stack(:,:,n);
    end
    stack = s2;
end

if ~iscell(stack)
    error('stack must be cell of images');
end

if nargin<2
    thresh = nanmean(stack{1}(:));
end



%% create gui
hFig_hist = figure();
d = stack{1};

max_stack = -Inf;
nF = numel(stack);
for f=1:nF
    max_stack = max(max_stack,max(stack{f}(:)));
end
hist_edges = linspace(0,max_stack,80);
hHist = histogram(d(:),hist_edges);
hHist.HitTest = 'off';

hAx = gca;
set(hAx,'Yscale','log');
xlabel('Intensity');
ylabel('Bin Count');
title('Click on graph to select Intensity Threshold, Close when done');
ylim([1,numel(d)]);
hold on;
hPlt = plot([thresh,thresh],[1,numel(d)],'--r','LineWidth',2);

    %callback for changed frame
    function ChFrame(hObj,~)
        hHist.Data = stack{hObj.CurrentFrame};
        hObj.StackData{hObj.CurrentFrame} = stack{hObj.CurrentFrame}>thresh;
    end

hSV = extras.stackviewer(stack,'ChangeFrameCallback',@ChFrame);
colormap gray;

ChFrame(hSV);
title('Click on histogram to select Intensity Threshold, Close when done');
    function ClickAx(~,evt)
        if evt.Button==1
            thresh = evt.IntersectionPoint(1);
            
            hHist.Data = stack{hSV.CurrentFrame};
            hSV.StackData{hSV.CurrentFrame} = stack{hSV.CurrentFrame}>thresh;
            
            set(hPlt,'xdata',[thresh,thresh]);
        end
    end

set(hAx,'ButtonDownFcn',@ClickAx);

    function CloseRq(~,~)
        try
            delete(hFig_hist);
        catch
        end
        try
            delete(hSV);
        catch
        end
    end
set(hFig_hist,'CloseRequestFcn',@CloseRq);
addlistener(hSV,'ObjectBeingDestroyed',@CloseRq);
waitfor(hFig_hist);

end