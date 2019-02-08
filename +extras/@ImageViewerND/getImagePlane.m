function img = getImagePlane(this,indicies)

indicies = reshape(indicies,1,[]);
id2 = num2cell(indicies);
idx = sub2ind(this.StackDimensions,id2{:});

img = this.ImageStack{idx};

%% load image if needed
if isempty(img)
    if this.UsingBF % Bioformats importer
        seriesIndex = indicies(4);
        zIndex = indicies(3);
        tIndex = indicies(2);
        cIndex = indicies(1);
        
        this.bfreader.setSeries(seriesIndex-1);
        WIDTH = this.bfreader.getSizeX();
        HEIGHT = this.bfreader.getSizeY();
        
        idx2 = this.bfreader.getIndex(zIndex-1,cIndex-1,tIndex-1)+1;
        img = extras.bfmatlab.bfGetPlane(this.bfreader,idx2);
        img = reshape(img,HEIGHT,WIDTH);
        this.ImageStack{idx} = img;
    else
        error('other loaders not implemented yet')
    end
end