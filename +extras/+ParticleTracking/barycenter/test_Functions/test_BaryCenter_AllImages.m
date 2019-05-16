function test_BaryCenter_AllImages()
%% Image Dir Info
persistent ImageDir;
d = uigetdir(ImageDir,'Select image folder, cancel for default (./Test_Data) folder');
if d==0
    ImageDir = fullfile('Test_Data','ImageSave_Compressed_Beads7_Images'); 
else
    ImageDir = d;
end
files = dir(fullfile(ImageDir,'*.tif'));

%% Loop over all images
X = NaN(numel(files),1);
Y = NaN(numel(files),1);

hWB = waitbar(0,sprintf('Processing: %d/%d',0,numel(files)));
for n=1:numel(files)
    Img = imread(fullfile(files(n).folder,files(n).name));
    
    [X(n),Y(n)] = ParticleTrackers.BaryCenter(Img,[],5,0.1);
    
    if mod(n,20)==0
        waitbar(n/numel(files),hWB,sprintf('Processing: %d/%d',n,numel(files)));
    end
end
close(hWB);



%% Display Data
hFig = figure;

hX = subplot(2,1,1);

plot(hX,X,'.k','MarkerSize',1);
ylabel('X [px]');

hY = subplot(2,1,2);

plot(hY,Y,'.k','MarkerSize',1);
ylabel('Y [px]');