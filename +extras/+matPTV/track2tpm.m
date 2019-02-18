function tpmStruct = track2tpm(trackResults)
%convert output of ptv2D (and track_fluorescence) to struct expected by
%TPManalysis()

try
tpmStruct.TimeSec = trackResults.TimeSec;
catch
    warning('TimeSec field not found');
end

for n=1:numel(trackResults.track)
    tpmStruct.Bead(n).Xraw = trackResults.track(n).X;
    tpmStruct.Bead(n).Yraw = trackResults.track(n).Y;
end

if nargout < 1
    extras.uiputvar(tpmStruct);
    clear tpmStruct;
end