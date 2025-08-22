    % FUNCTION: intersection
%
% DESCRIPTION:
% This function solves an optimisation problem to determine the optimal access order and
% accelerations for a set of platoons approaching a T-intersection.
% The objective is to minimise the total time required for the platoons to cross the 
% intersection while ensuring a minimum safety distance between platoons, a maximum
% acceleration rate and a maximum speed limit.
%
% INPUTS:
%   - platoons [Array<struct>]: each platoon is represented by a struct with the following fields:
%           'distance' [double] -> current distance to the intersection (unit: m)
%           'speed'    [double] -> current speed (unit: m/s)
%           'length'   [double] -> sum of the vehicle lengths and inter-vehicle distances (unit: m)
%           'vehCount' [int]    -> total number of vehicles
%           'roadId'   [string] -> identifier of the current road                                
%   - safetyDistance [double]: minimum distance gap between two platoons (unit: m)
%   - maxAcc [double]: absolute value of maximum acceleration allowed (unit m/s^2)
%   - maxSpeed [double]: maximum speed allowed (unit: m/s)
%   - alpha: balancing parameter between time and acceleration minimisation. This parameter must be
%            between 0 and 1, a value of 0 makes the problem focus only on the acceleration minimisation
%            while a value of 1 considers only the times.
%               
% OUTPUTS:
%   - optT [Array<double>]: intersection access times, i-th index for the i-th platoon
%   - optA [Array<double>]: required accelerations between the starting point and the intersection
%   - optC [double]: the total cost of the maneuver , result of the objective function
%   - optV [Array<double>]: expected speed at the intersection
%
% NOTE:
%   1. if maxSpeed < initial speed of any platoon it's impossible to find a solution

function [optT, optA, optC, optV] = intersection(platoons, safetyDistance, maxAcc, maxSpeed, alpha)

    % VARIABLES
    n = length(platoons);       % number of platoons
    d = [platoons.distance];    % array of distances, d(i) = distance of the i-th platoon
    v = [platoons.speed];       % array of initial speeds, v(i) = initial speed of the i-th platoon
    l = [platoons.length];      % array of lengths, l(i) = length of the i-th platoon
    h = [platoons.vehCount];    % array of number of vehicles, h(i) = number of vehicles of the i-th platoon
    r = [platoons.roadId];      % array of road IDs, r(i) = road ID of the i-th platoon
    roadIds = unique(r);        % array of possible road IDs

    % OPTIMISATION VARIABLE: 1xn array of access times
    t = optimvar('t', 1, n);


    % OBJECTIVE FUNCTION: sum of squared access times
    % The sum of squared times offers a convex function easy to minimise
    function mt = minTime(t)
        mt = sum(t.^2);
    end

    function ma = minAcc(t)
        a = 2 .* (-v .* t + d) ./ (t .^ 2);
        ma = sum(a.^2);
    end

    function o = weightedObjectives(t)
        wt = 1e3;           % times weight
        cT = minTime(t);    % times cost

        wa = 1/32;          % accelerations weight
        cA = minAcc(t);     % accelerations cost

        o = (alpha/wt) * cT + ((1-alpha)/wa) * cA;
    end


    % LINEAR EQUALITY CONSTRAINTS [ Aeq * t' = beq ]
    Aeq = [];
    beq = [];


    % LINEAR INEQUALITY CONSTRAINTS [A * t <= b]
    A = [];
    b = [];

    % LI1. No overtake: two platoons on the same road cannot change their respective order.
    % Given two platoons Pi and Pj, if they share the same road and Pi is closer to the intersection
    % than Pj, the constraint t(i) < t(j) must be verified.
    vehRoad = [sum(strcmp(r, roadIds(1))), sum(strcmp(r, roadIds(2)))];     % vehRoad(i) = number of vehicles on the i-th road
    cNum = 0;                                                               % num of constraints to add
    if vehRoad(1) > 1 cNum = cNum + nchoosek(vehRoad(1), 2); end
    if vehRoad(2) > 1 cNum = cNum + nchoosek(vehRoad(2), 2); end
    A1 = zeros(cNum, n);                                                    % A matrix for this constraint
    b1 = zeros(1, cNum);                                                    % b vector for this constraint

    cIndex = 1;                                                             % constraint index
    for i=1:n                                                               % fill A1 and b1
        for j=1:n
            if(strcmp(r(i), r(j)) && d(i) < d(j))
                A1(cIndex, i) = +1;
                A1(cIndex, j) = -1;
                b1(cIndex) = 0;
                cIndex = cIndex+1;
            end
        end
    end
    A = [A, A1];                                                            % add the new constraints to the problem
    b = [b, b1];


    % NON-LINEAR CONSTRAINTS
    function [c, ceq] = nonlcon(t) 
        c = [];     % set of inequality constraints where c(t) <= 0
        ceq = [];   % set of equality constraints where ceq(t) = 0

        % NI1. Safety distance: every platoon must cross the intersection at a minimum distance to the previous one.
        % Given two platoons Pi and Pj, if Pi crosses the intersection before Pj (t(i) < t(j)) then t(j) must be
        % greater than t(i) + the time needed by Pi to cross the intersection and travel the safety distance.
        % 
        % NOTE:
        % Since it isn't possible to direcly compare t(i) and t(j) the function max(0, abs(t(j)-t(i))/(t(j)-t(i)))
        % is used to establish if t(i) < t(j) (result = 1) or t(i) > t(j) (result 0)
        cNum = nchoosek(n, 2);                                              % number of constraints to add
        c1 = zeros(1, cNum);                                                % constraints array

        cIndex = 1;                                                         % constraint index
        for i=1:n                                                           % fill c1
            ai = 2 * (-v(i) * t(i) + d(i)) / (t(i) ^ 2);                    % acceleration of Pi
            vfi = ai * t(i) + v(i);                                         % final speed of Pi
            tfi = t(i) + (l(i) + safetyDistance) / (50/3.6);                % approximation                  
            for j=1:n
                if i~=j && max(0, abs(t(j)-t(i))/(t(j)-t(i)))
                    c1(cIndex) = tfi - t(j);
                    cIndex = cIndex+1;
                end
            end
        end
        c = [c, c1];

        % NI2. Max speed: every platoon cannot exceed a maximum speed
        c2 = zeros(1, n);
        cIndex = 1;
        for i=1:n
            ai = 2 * (-v(i) * t(i) + d(i)) / (t(i) ^ 2);                    % acceleration of Pi
            vfi = ai * t(i) + v(i);                                         % final speed of Pi
            c2(cIndex)  = max(v(i), vfi) - maxSpeed;
            cIndex = cIndex+1;
        end
        c = [c, c2];

        % NI3. Max acceleration: every platoon cannot exceed a maximum acceleration
        c3 = zeros(1, n);
        cIndex = 1;
        for i=1:n
            ai = 2 * (-v(i) * t(i) + d(i)) / (t(i) ^ 2);
            c3(cIndex) = abs(ai) - maxAcc;
            cIndex = cIndex+1;
        end
        c = [c, c3];

    end


    % SOLVER
    options = optimoptions('fmincon', 'Display', 'none', 'Algorithm', 'interior-point');    % solver options
    lb = d ./ maxSpeed;                                                                     % solution lower bound
    ub = inf(n,1);                                                                          % solution upper bound                               
  
    % initial guesses
    t0 = d ./ maxSpeed;                     % max speed - min time
    t1 = d ./ (50/3.6);                     % no acceleration
    t2 = d ./ ((50/3.6 + maxSpeed) / 2);    % mid case

    optC = inf;
    optT = inf;

    % find optimal times with three different initial guesses
    guesses = [t0; t1; t2];
    for i=1:length(guesses)
        [runT, runC, ~, out] = fmincon(@weightedObjectives, guesses(i,:), A, b, Aeq, beq, lb, ub, @nonlcon, options);
        if out.bestfeasible.constrviolation==0 && runC < optC
            optC = runC;
            optT = runT;
        end
    end

    if optC == inf
        error("<!> Error: no feasible solution found");
    end

    % compute optimal acceleration and crossing speed
    optA = 2 .* (-v .* optT + d) ./ (optT .^ 2);
    optV = optA .* optT + v;

    fprintf("MATLAB problem inputs:\n");
    fprintf("Safety distance: %+5.2f\n", safetyDistance);
    fprintf("dist:  P0: %+5.2f   P1: %+5.2f    P2: %+5.2f\n", d);
    fprintf("speed: P0: %+5.2f   P1: %+5.2f    P2: %+5.2f\n", v);

    fprintf("\n-- MATLAB -------------------------------------------------\n");
    fprintf("| Optimisation problem solved!                            |\n");
    fprintf("| Access time      P0: %+5.2f   P1: %+5.2f    P2: %+5.2f  |\n", optT);
    fprintf("| Acceleration     P0: %+5.4f  P1: %+5.4f   P2: %+5.4f |\n", optA);
    fprintf("| Final speed      P0: %+5.3f  P1: %+5.3f   P2: %+5.3f |\n", optV);
    fprintf("| Maneuver cost    %-8.2f                               |\n", optC); 
    fprintf("-----------------------------------------------------------\n\n");
    
    %return optimal times, accelerations, cost and crossing speed
end