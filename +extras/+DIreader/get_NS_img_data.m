function img_data = get_NS_img_data(NS_data, scaleFlag)
%%%%%%%%%%%%%%%%%%%%%%%
% Accepts a structure with information about an image contained within a
% NanoScope file. The info in the NS_data structure is used to retrieve the
% data and return an array of scaled, floating point data. The scaling is
% likely to be accurate for height data only. See get_NS_file_info.m for
% information about the NS_data structure variable with file information.
%
%   scaleFlag=true, scale data to nm
%             false, don't scale data
%
% David Dunlap, Emory University 2016-09-20
%%%%%%%%%%%%%%%%%%%%%%%

if nargin<2
    scaleFlag = true;
end
assert(numel(NS_data)==1,'NS_data should be a single struct, multiple NS files should be specified separately');
fid = fopen(NS_data.name,'rb'); % try to open the file for reading
[message,errnum] = ferror(fid);
if(errnum) % display error if file does not open
    fprintf(1,'I/O Error %d \t %s',[errnum,message]);
    img_data = [];
else
    error = fseek(fid, NS_data.offset, 'bof');
    if ~error % do if fseek was successful
        switch (NS_data.version)
            case 5300003
                raw_data = double(fread(fid, [NS_data.rows, NS_data.columns], 'int16'));
            case 9010300
                raw_data = double(fread(fid, [NS_data.rows, NS_data.columns], 'int16'));
            case 920
                raw_data = double(fread(fid, [NS_data.rows, NS_data.columns], 'int32'));
            otherwise
                disp('Warning! Image from untested version of NanoScope software!')
                raw_data = double(fread(fid, [NS_data.rows, NS_data.columns], 'int16'));
        end
        img_data = rot90(raw_data);
        if (scaleFlag)
            scaler = NS_data.Z_scale/65536*NS_data.Zsens;
            img_data = img_data*scaler; % convert the scaled, integer data to real height measures
        end
    end
    fclose(fid);
end
end

