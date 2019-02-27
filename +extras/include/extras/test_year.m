% datenum to year

dd = 2:200:2000*366;

YY = year(dd);

yy = floor((dd-1)/(365+97/400));

d2 = 1+yy*365 + floor((yy-1)/4) - floor((yy-1)/100.0) + floor((yy-1)/400.0);

D2 = datenum(YY,ones(size(YY)),ones(size(YY)))-1;


res=[dd;YY;yy;YY-yy];

res2 = [dd;D2;d2;D2-d2];