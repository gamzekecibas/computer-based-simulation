% Gamze Keçibaş
% 60211
% MECH534: Computer Based Modeling & Simulation
% Spring 2022
% Homework - 5: Solving Nonlinear Equations
clc; clear all; close all;

% Given inital conditions
t0 = 0;
tf = 10;
y0 = [0.5; 0.5];

% Define ode23 solution via Lotka - Volterra system
[t,y1] = ode23(@my_lotka,[t0 tf],y0);

% Plot the ode23 solution
figure(1)
plot(t,y1)
title('Solution for RK23')
xlabel('Time [sec]')
ylabel('Population')
legend('Prey','Predators')

% Define ode45 solution via Lotka - Volterra system
[t,y2] = ode45(@my_lotka,[t0 tf],y0);

% Plot the ode45 solution
figure(2)
plot(t,y2)
title('Solution for RK45')
xlabel('Time [sec]')
ylabel('Population')
legend('Prey','Predators')

% Read & Plot C++ solutions
fid = fopen('C:\Users\gkeciba1\Desktop\GamzeKecibas_60211_MECH534_HW5\mech534hw5\mech534hw5\gkecibas16_hw5_output.txt','rt');
C = textscan(fid, '%f%f%f', 'MultipleDelimsAsOne',true, 'Delimiter','[,', 'HeaderLines',2);
fclose(fid);

figure(3)
C = cell2mat(C);
t = linspace(0,10,100);
plot(C(:,3), C(:,1),C(:,3), C(:,2)) 
title('Solution for C++')
xlabel('Time [sec]')
ylabel('Population')
legend('Prey','Predators')