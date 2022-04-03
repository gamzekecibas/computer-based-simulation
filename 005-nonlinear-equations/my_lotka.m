function yp = my_lotka(t,y)
%LOTKA  Lotka-Volterra predator-prey model.

%   Copyright 1984-2014 The MathWorks, Inc.

yp = diag([1 - y(2), -1 + y(1)])*y;