function RGB = colorname2rgb(str)
% convert color name string into rgb colors
%
% Input:
%   str: char array or cell array of color names;
%
% Output:
%   RGB: array of color values for each specified color
%       if str is a cell each row of RGB corresponds to a different element
%       of str.


persistent colornames;
if isempty(colornames)
    colornames = containers.Map('KeyType','char','ValueType','any');
    colornames('y') = [1,1,0];
    colornames('yellow') = [1,1,0];
    colornames('m') = [1,0,1];
    colornames('magenta') = [1,0,1];
    colornames('c') = [0,1,1];
    colornames('cyan')=[0,1,1];
    colornames('r')=[1,0,0];
    colornames('red')=[1,0,0];
    colornames('g')=[0,1,0];
    colornames('green')=[0,1,0];
    colornames('b')=[0,0,1];
    colornames('blue')=[0,0,1];
    colornames('w')=[1,1,1];
    colornames('white')=[1,1,1];
    colornames('k')=[0,0,0];
    colornames('black')=[0,0,0];
    colornames('forestgreen')=[0,0.5,0];
end

assert(ischar(str)||iscellstr(str),'str must be char or cellstr');

if ischar(str)
    str = {str};
end

RGB = NaN(numel(str),3);

for n=1:numel(str)
    try
        RGB(n,:) = colornames(lower(str{n}));
    catch
    end
end


