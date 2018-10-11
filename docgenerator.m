function outputfile = docgenerator(topic,outputdir,calledfrom)
%% extras.docgenerator(topic,outputdir)
%
% Recursively Generate HTML Help files for a given topic
% Help files are generated using MATLAB's built-in help system
% If your functions have been documented using MATLAB's prefered
% documentation methods then those comments should automatically be
% incorporated in the help files
%
% Example:
%   To generate HTML help files for the extras package run:
%       >> extras.docgenerator('extras','OUTPUT_DOC_DIR');
%   Documentation for all of the classes and functions inside the +extras
%   package will automatically be generated and stored in "OUTPUT_DOC_DIR".
%
% Note: This function will only generate help pages that have not yet been
% created. If you update your toolbox/functions then you should delete or 
% empty the output director before calling extras.docgenerator(...) again.
% Otherwise, the documentation will not be re-generated.

%% Copyright 2018, Daniel Kovari, Emory University
% All rights reserved

fprintf('Generating: %s.html\n',topic);

%% handle inputs
if nargin<2
    outputdir = '';
end

if ~isempty(outputdir) && ~isfolder(outputdir)
    mkdir(outputdir);
end

if nargin < 3
    calledfrom = {};
end

ROOT_CALL = isempty(calledfrom);

if ROOT_CALL
    rootdir = outputdir;
    contentdir = [topic,'_contents'];
    outputdir = fullfile(outputdir,contentdir);
    mkdir(outputdir);
end

%% generate HTML
currentPageHTML = help2html(topic);

%% Determine if topic is a package
mp = meta.package.fromName(topic);
if ~isempty(mp)
    %fprintf('\tIs a package\n');
    %% find the body section
    [bodyStart,bodyEnd] = regexp(currentPageHTML,'(?<=<body>).*(?=</body>)');
    bodyHTML = currentPageHTML(bodyStart:bodyEnd);
    
    %% add sub packages section to 
    if ~isempty(mp.PackageList)
        
        subPackHTML = sprintf('\n<div class="subpackages"><h2>Sub-packages</h2><pre>\n');

        for n=1:numel(mp.PackageList)
            subPackHTML = [subPackHTML,...
                sprintf('<a href="matlab:helpwin %s">%s</a>\n',mp.PackageList(n).Name,mp.PackageList(n).Name)];
        end
        subPackHTML = [subPackHTML,sprintf('</pre></div>\n')];
        bodyHTML = [bodyHTML,subPackHTML];
        currentPageHTML = [currentPageHTML(1:bodyStart-1),bodyHTML,currentPageHTML(bodyEnd+1:end)];
    end
    
    %% Look for calls to matlab:helpwin ___ and recursively generate help files and replace hyperlinks
    [startIdx,endIdx] = regexp(currentPageHTML,'href="matlab:helpwin\s\S+"');
    
    for n=numel(startIdx):-1:1 %start at back so that we can replace the text as we go
        callStr = currentPageHTML(startIdx(n):endIdx(n));
        
        %get the function name
        func_name = regexp(callStr,'helpwin\s\S+','match');
        func_name = func_name{1};
        func_name = func_name(9:end-1);
        
        %recursively generate help for the function and save in outputdir
        
        subfile = [func_name,'.html'];
        if ~ismember(func_name,calledfrom) && ~isfile(fullfile(outputdir,subfile)) %only generate if needed 
            subfile = extras.docgenerator(func_name,outputdir,[calledfrom,func_name]);
        end
        
        %replace helpwin call with hyperlink to generated file
        currentPageHTML = [currentPageHTML(1:startIdx(n)-1),...
            'href="',subfile,'"',...
            currentPageHTML(endIdx(n)+1:end)];
    end
end


%% Look for calls to matlab:helpwin() and recursively generate help files and replace hyperlinks
[startIdx,endIdx] = regexp(currentPageHTML,'href="matlab:helpwin\(\''\S+''\)"');

for n=numel(startIdx):-1:1 %start at back so that we can replace the text as we go
    callStr = currentPageHTML(startIdx(n):endIdx(n));
    
    %get the function name
    func_name = regexp(callStr,'\(''\S+''\)','match');
    func_name = func_name{1};
    func_name = func_name(3:end-2);
    
    %recursively generate help for the function and save in outputdir
    subfile = [func_name,'.html'];
    if ~ismember(func_name,calledfrom) && ~isfile(fullfile(outputdir,subfile)) %only generate if needed 
        subfile = extras.docgenerator(func_name,outputdir,[calledfrom,func_name]);
    end
    
    %replace helpwin call with hyperlink to generated file
    currentPageHTML = [currentPageHTML(1:startIdx(n)-1),...
        'href="',subfile,'"',...
        currentPageHTML(endIdx(n)+1:end)];
end

%% Look for calls to matlab:helpwin ___ and recursively generate help files and replace hyperlinks
% [startIdx,endIdx] = regexp(currentPageHTML,'href="matlab:helpwin\s\S+"');
% 
% for n=numel(startIdx):-1:1 %start at back so that we can replace the text as we go
%     callStr = currentPageHTML(startIdx(n):endIdx(n));
%     
%     %get the function name
%     func_name = regexp(callStr,'helpwin\s\S+','match');
%     func_name = func_name{1};
%     func_name = func_name(9:end-1);
%     
%     %recursively generate help for the function and save in outputdir
%     
%     subfile = [func_name,'.html'];
%     if ~ismember(func_name,calledfrom) && ~isfile(fullfile(outputdir,subfile)) %only generate if needed 
%         subfile = extras.docgenerator(func_name,outputdir,[calledfrom,func_name]);
%     end
%     
%     %replace helpwin call with hyperlink to generated file
%     currentPageHTML = [currentPageHTML(1:startIdx(n)-1),...
%         'href="',subfile,'"',...
%         currentPageHTML(endIdx(n)+1:end)];
% end

%% Fix the included style sheet
if ~isfile(fullfile(outputdir,'helpwin.css'))
    copyfile(fullfile(matlabroot,'toolbox','matlab','helptools','private','helpwin.css'),fullfile(outputdir,'helpwin.css'));
end
[sI,eI] = regexp(currentPageHTML,'(?<=href=").+helpwin\.css(?=")');
currentPageHTML = [currentPageHTML(1:sI-1),outputdir,'/helpwin.css',currentPageHTML(eI+1:end)];

%% Save the result to an html file

outputfile = [topic,'.html'];

[pth,fl,ext]=fileparts(outputfile);
if ~isfolder(fullfile(outputdir,pth))
    mkdir(fullfile(outputdir,pth));
end

fid = fopen(fullfile(outputdir,pth,[fl,ext]),'w');
fprintf(fid,'%s',currentPageHTML);
fclose(fid);

if ROOT_CALL
    fid = fopen(fullfile(rootdir,'index.html'),'w');
    fprintf(fid,['<meta http-equiv="refresh" content="1; url=%s/%s">\n',...
        '<script>window.location.href = "%s/%s"</script>\n',...
        '<title>Page Redirection</title>\n',...
        'If you are not redirected automatically, follow the <a href="%s/%s">link to %s</a>'],...
        contentdir,outputfile,...
        contentdir,outputfile,...
        contentdir,outputfile,topic);
    fclose(fid);
end

%% supress output
if nargout < 1
    clear outputfile;
end