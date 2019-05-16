load clown; %get creepy clown image
hFig = figure;
imh = imagesc(X); %display creepy clown

%create colormapUI
cui = extras.colormapUI([0,2,26,43,77],{'k','g','r','r','w'},'Image',imh);


