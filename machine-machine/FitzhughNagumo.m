%% FitzHugh-Nagumo
% Parameters
epsilon = 0.01;
alpha = 0.1;
gamma = 0.5;
Iapp = 0.5;

% starting values
v = 0.4;
w = 0.5;
t = 0;

% chosen time step
dt = 0.01;

% Given equations
i = 0;
while i < 299
    f = v.*(1 - v).*(v - alpha);      % for 0 < alpha < 1, epsilon << 1

    dv = (dt./epsilon) .* (f - w + Iapp);
    dw = dt.*(v - gamma.*w);

    t = t + dt;
    v = v + dv;
    w = w + dw;
    
    t_array(1 + i) = t;
    v_array(1 + i) = v;
    w_array(1 + i) = w;
    i = i + 1;
end

%plot(t_array, v_array)
% xlabel("time")
% ylabel("v")
% 
 figure
 plot(v_array, w_array)
% xlabel("v")
% ylabel("w")

%plot(t_array, v_array)

%% 