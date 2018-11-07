function im_flat = imflatten(im,varargin)
% Flatten grayscale image background using polynomial fit
%
% The supplied image is fit to a polynomial function and then the value of
% that polynomial at each pixel location is subtracted from the image.
%
% Input:
%   im: grayscale image to flatten
%
% Parameters:
%   'Order',p: order of the ploynomial to fit the data to
%   'Method',
%       'WholeImage': Fits the entire image to a 2D polynomial
%                       f(x,y) = Ax^n + By^n + ...
%       'Columns': fits each column to independent polynomials p(y)=...
%       'Rows': fits each row to independent polynomials p(x)=...
%
% Output:
%   im_flat: the flattened image
%% Copyright 2018 Daniel Kovari, Emory University
%   All rights reserved

%% Change Log
%   2018-10-25: DTK
%       initial file creation

%% Parse arguemtns
p = inputParser;
addParameter(p,'Method','WholeImage',@(x) ismember(lower(x),{'wholeimage','columns','rows'}));
addParameter(p,'PolynomialOrder',2,@(x) isscalar(x)&&isnumeric(x));

parse(p,varargin{:});

%% process different options

order = round(p.Results.PolynomialOrder);

switch lower(p.Results.Method)
    case 'wholeimage'
        [xx,yy] = meshgrid(1:size(im,2),1:size(im,1));
        xx=reshape(xx,[],1);
        yy = reshape(yy,[],1);
        
        XYmat = [];
        for n=order:-1:1
            XYmat = [XYmat,xx.^n,yy.^n];
        end
        XYmat = [XYmat,ones(size(xx))];
        
        A = mldivide(XYmat,reshape(im,[],1));
        im_flat = reshape(im,[],1)-XYmat*A;
        im_flat = reshape(im_flat,size(im));
    case 'columns'
        %loop over columns
        yy = (1:size(im,1))';
        im_flat = zeros(size(im));
        for n=1:size(im,2)
            pp = polyfit(yy,im(:,n),order);
            im_flat(:,n) = im(:,n)-ppval(pp,yy);
        end
        
    case 'rows'
        %loop over rows
        xx = (1:size(im,2));
        im_flat = zeros(size(im));
        for n=1:size(im,1)
            pp = polyfit(xx,im(n,:),order);
            im_flat(n,:) = im(n,:)-ppval(pp,xx);
        end
end