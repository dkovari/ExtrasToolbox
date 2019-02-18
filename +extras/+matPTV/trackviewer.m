function trackviewer(trackResults)
% Display track results

%% StackViewer
hSV = extras.stackviewer(trackResults.ImageStack);
colormap gray;

%% Plot Traces
hold(hSV.ImageAxes,'on');
hTracks = plot(hSV.ImageAxes,[trackResults.track.X],[trackResults.track.Y],'-');

%% Drift Corrections

if isfield(trackResults,'driftX')
    hSV.XData = [1,size(trackResults.ImageStack{1},2)]-trackResults.driftX;
    
    for n = 1:numel(trackResults.track)
        hTracks(n).XData = trackResults.track(n).X-trackResults.driftX;
    end
end
if isfield(trackResults,'driftY')
    hSV.YData = [1,size(trackResults.ImageStack{1},1)]-trackResults.driftY;
    
    for n = 1:numel(trackResults.track)
        hTracks(n).YData = trackResults.track(n).Y-trackResults.driftY;
    end
end


%% Plot Centroids
hPlot = plot(hSV.ImageAxes,NaN,NaN,'xr','MarkerSize',10);

%% define update function
    function ChangeFrame(~,~)
        XX = trackResults.Centroids{hSV.CurrentFrame}(:,1);
        YY = trackResults.Centroids{hSV.CurrentFrame}(:,2);
        if isfield(trackResults,'driftX')
            XX = XX - trackResults.driftX(hSV.CurrentFrame);
        end
        if isfield(trackResults,'driftY')
            YY = YY - trackResults.driftY(hSV.CurrentFrame);
        end
            
        set(hPlot,'XData',XX,...
            'YData',YY);
    end

hSV.ChangeFrameCallback = @ChangeFrame;

%% update plots
ChangeFrame();

end
