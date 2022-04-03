% Gamze Keçibaş
% 60211
% MECH534: Computer Based Modeling & Simulation
% Spring 2022

fid = fopen('C:\Users\gkeciba1\Desktop\GamzeKecibas_60211_MECH534_HW5\mech534hw5\mech534hw5\gkecibas16_hw5_output.txt','rt');
C = textscan(fid, '%f%f%f', 'MultipleDelimsAsOne',true, 'Delimiter','[,', 'HeaderLines',2);
fclose(fid);

function dx1dt = vdp1(t,x1)
   dx1dt = [x1; x1*(b1 - c1*x2)]; 
end

function dx2dt = vdp1(t,x2)
   dx2dt = [x2; x2*(-b1 + c1*x1)]; 
end