% Test mxfile

%% write data
w = extras.mxfile.MxFileWriter_mex('new');
isOpen = extras.mxfile.MxFileWriter_mex('isFileOpen',w)
'opening mydata'
extras.mxfile.MxFileWriter_mex('openFile',w,'mydata')
isOpen = extras.mxfile.MxFileWriter_mex('isFileOpen',w)

'write'
s.one = {1,2};
s.two = 222;
s.three='three';
extras.mxfile.MxFileWriter_mex('writeArrays',w,1,2,3,s)

'close'
extras.mxfile.MxFileWriter_mex('closeFile',w)
isOpen = extras.mxfile.MxFileWriter_mex('isFileOpen',w)
'delete'
extras.mxfile.MxFileWriter_mex('delete',w)

%% reader
'press a key to continue'
pause
r = extras.mxfile.MxFileReader_mex('new');
isOpen = extras.mxfile.MxFileReader_mex('isFileOpen',r)
'opening mydata.mxf.gz'
extras.mxfile.MxFileReader_mex('openFile',r,'mydata.mxf.gz')
isOpen = extras.mxfile.MxFileReader_mex('isFileOpen',r)

try
    'read next'
    val = extras.mxfile.MxFileReader_mex('readNextArray',r)
    'read next'
    val = extras.mxfile.MxFileReader_mex('readNextArray',r)
    'read next'
    val = extras.mxfile.MxFileReader_mex('readNextArray',r)
    'read next'
    val = extras.mxfile.MxFileReader_mex('readNextArray',r)
catch ME
    disp(ME.getReport);
end

'close'
extras.mxfile.MxFileReader_mex('closeFile',r)
isOpen = extras.mxfile.MxFileReader_mex('isFileOpen',r)
'delete'
extras.mxfile.MxFileReader_mex('delete',r)