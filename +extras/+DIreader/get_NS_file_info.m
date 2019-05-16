function [NS_data] = get_NS_file_info(file_name)
%%%%%%%%%%%%%%%%%%%%%%%
% Accepts a file name and parses the header information in the
% DI/Veeco/Bruker image files to find the offsets of image data in the file
% and scale information. It returns a structure with information about the
% images contained within a single file without including the image data.
% The info in the NS_data structure can be used to retreive the data using
% the get_NS_img_data.m function.
% David Dunlap, Emory University 2016-09-20
%%%%%%%%%%%%%%%%%%%%%%%

fid = fopen(file_name,'r'); % try to open the file for reading
[message,errnum] = ferror(fid);
if(errnum) % display error if file does not open
    fprintf(1,'I/O Error %d \t %s',[errnum,message]);
else
    eoh = 0; % clear end-of-header flag
    count = 0;
    while(~eoh)
        file_position = ftell(fid);
        line = fgetl(fid); % get the next line
        if isempty(line) % if read past the end of the header/file ...
            eoh = 1; % then set end-of-header flag to step out
        else
            if strfind(line,'\Version: 0x') % If this is the line with the version number of the system ...
                Key   = 'Version: 0x'; % load the value into the variable version.
                Index = strfind(line, Key);
                value = sscanf(line(Index(1) + length(Key):end), '%d', 1);
                SoftVer = value;
            else
                if strfind(line,'\@Sens. Zsens: V') % If this is the line with the Z sensitivity of the system ...
                    Key   = 'Sens. Zsens: V'; % load the value into the variable Zsens.
                    Index = strfind(line, Key);
                    value = sscanf(line(Index(1) + length(Key):end-4), '%d', 1);
                    Zsens = value;
                else
                    if strfind(line,'\*Ciao image list') % this line contains this string ...
                        count = count + 1; % increment the image count
                        image_hdrs(count) = file_position; % save the file offset and ...
                    else
                        if strfind(line,'\*File list end') % reached the end of the header?
                            eoh = 1;
                        end
                    end
                end
            end
        end
    end
        
    fseek(fid, image_hdrs(1),'bof'); % point to the byte position of image info in the header
    % specify a structure to hold NanoScope images
    NS_data = repmat(struct('name',{}),count-1); % preallocate a structue array for image info
    searchstring = { ... % define an array of header field labels
        '*Ciao image list', ... % 1
        'Data offset:', ... % 2
        'Data length:', ... % 3
        'Bytes/pixel:', ... %4
        'Samps/line:', ... % 5
        'Number of lines:', ... % 6
        'Scan Size:', ... % 7
        '@2:Image Data:', ... % 8
        '@Z magnify:',... % 9
        '@2:Z scale:', ... % 10
        '@2:Z offset:', ... %11
        'Line Direction:', ... % 12
        'Note:', ... %13
        '*File list end', ... %14
        };
    n_strings = length(searchstring);
    imgs = 1; % initialize image index
    firstline = 1; % set a logical flag to avoid over counting headers
    NS_data(imgs).name = file_name; % add the file name to the image info
    eoh = 0; % clear end-of-header flag
    while ~eoh % read lines until the end-of-header is raised
        line = fgetl(fid); % retrieve the next header line
        unmatched = 1; % set the flag indicating no match with the current string
        curr_string = 1; % initialize the string index
        while (unmatched && (curr_string <= n_strings))% test for a match with any of the input field names
            if isempty(strfind(line,searchstring{curr_string}))
                curr_string = curr_string+1; % increment to the next input field name
            else
                unmatched = 0; % clear "unmatched" flag when a match is found
                %                    disp(line); % for debugging
            end
        end
        NS_data(imgs).Zsens = Zsens;
        NS_data(imgs).version = SoftVer;
        switch(curr_string) % branch to scan the matched input field
            case 2 % Data offset
                Key   = 'Data offset:';
                Index = strfind(line, Key);
                value = sscanf(line(Index(1) + length(Key):end), '%d', 1);
                NS_data(imgs).offset = value;
            case 4 % Bytes/pixel - This does not seem to be a reliable parameter from NanoScope!
                Key   = 'Bytes/pixel:';
                Index = strfind(line, Key);
                value = sscanf(line(Index(1) + length(Key):end), '%d', 1);
                NS_data(imgs).Pbytes = value;
            case 5 % Samps/line
                Key   = 'Samps/line:';
                Index = strfind(line, Key);
                value = sscanf(line(Index(1) + length(Key):end), '%d', 1);
                NS_data(imgs).columns = value;
            case 6 % Number of lines
                Key   = 'Number of lines:';
                Index = strfind(line, Key);
                value = sscanf(line(Index(1) + length(Key):end), '%d', 1);
                NS_data(imgs).rows = value;
            case 7 % scan size
                Key   = 'Scan Size:';
                Index = strfind(line, Key);
                dim = sscanf(line(Index(1) + length(Key):end-2), '%f', 2);
                NS_data(imgs).width = dim(1);
                NS_data(imgs).height = dim(2);
                unit = sscanf(line(end:end), '%c');
                switch (unit)
                    case 'm'
                        NS_data(imgs).unit = 'µm';
                    case 'n'
                        NS_data(imgs).unit = 'nm';
                end
                NS_data(imgs).hscale = NS_data(imgs).width/NS_data(imgs).columns;
                NS_data(imgs).vscale = NS_data(imgs).height/NS_data(imgs).rows;
            case 8 % type
                Key   = '"';
                Index = strfind(line, Key);
                NS_data(imgs).type = line(Index(1)+1:Index(2)-1);
            case 9 % Z magnify
                Key   = '[2:Z scale]';
                Index = strfind(line, Key);
                NS_data(imgs).Z_mag = sscanf(line(Index(1) + length(Key):end), '%f', 1);
            case 10 % Z scale
                Key   = 'V/LSB)';
                Index = strfind(line, Key);
                NS_data(imgs).Z_scale = sscanf(line(Index(1) + length(Key):end-1), '%f', 1);
            case 11 % Z offset
                Key   = 'V/LSB)';
                Index = strfind(line, Key);
                NS_data(imgs).Z_offset = sscanf(line(Index(1) + length(Key):end-1), '%f', 1);
            case 12 % Line Direction
                Key   = 'Direction: ';
                Index = strfind(line, Key);
                NS_data(imgs).direction = line(Index(1) + length(Key):end);
            case 13 % Note
                Key   = 'Note: ';
                Index = strfind(line, Key);
                NS_data(imgs).note = line(Index(1) + length(Key):end);
            case 14 % end of header "\*File list end"
                %                disp(NS_data(imgs)); % for debugging
                eoh = 1;
            case 1 % start of another image header "\*Ciao image list"
                if firstline
                    firstline = 0; % clear the firstline flag
                else
                    %                    disp(NS_data(imgs)); % for debugging
                    imgs = imgs + 1;
                    NS_data(imgs).name = file_name; % add the file name to the next image info structure
                end
        end
    end
end


fclose(fid);
end

