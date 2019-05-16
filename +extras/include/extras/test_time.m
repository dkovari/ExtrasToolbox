% test date

Y = 2002:2801;
DD = datenum(Y,ones(size(Y)),ones(size(Y)));
DD = DD-1;


dd = mydate(Y);

res = [Y;DD-dd];

function dd = mydate(y)
%                    %days from 1900 to 2000 %days upto 1900d

dd = ... 
    + (y-2001)*365 + floor((y-2001)/4) -floor((y-2001)/100) + floor((y-2001)/400) ...
    + 36524+366 ... %days from 1900 to 2001
    + 693961; %days from 0 to 1900

end