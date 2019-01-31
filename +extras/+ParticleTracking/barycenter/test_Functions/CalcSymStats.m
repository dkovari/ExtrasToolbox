function [sigx, sigy, SymRatio] = CalcSymStats(X,Y)
    badI = find(isnan(X)|isnan(Y)|isinf(X)|isinf(Y));
    X(badI) = [];
    Y(badI) = [];
    
    sigx = std(X);
    sigy = std(Y);
    lambda = eig(cov(X,Y));
    lambda = sort(lambda,'descend');
    SymRatio = sqrt(lambda(1)/lambda(2));
end