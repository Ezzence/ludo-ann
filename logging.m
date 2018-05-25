f = fopen('AClogbad.txt');   
g = textscan(f,'');
fclose(f)

gdat = [g{1:end}];

for i=1:length(g{1,1})
  weight{i} = gdat(:, i);
end

%x = 1:1:size(weight{1}, 1);
x = 1:2500;
Y = [weight{1}(1:2500) weight{2}(1:2500) weight{3}(1:2500) weight{4}(1:2500)];
subplot(2, 1, 1)
plot (x, Y, "linewidth", 2);
set(gca, "linewidth", 1, "fontsize", 12)
title ("Sensory Input weight, and win rate progression");
xlabel ("Game Number");
ylabel ("Input Weights");

x = 1:2500;
Y = [weight{5}(1:2500)];
subplot(2, 1, 2)
plot (x, Y, "linewidth", 2);
set(gca, "linewidth", 1, "fontsize", 12)
title ("");
xlabel ("Game Number");
ylabel ("Win Rate");


%-------------------------------------------------------------------------------------

Y2 = [0.5755 0.594 0.5885 0.588 0.597 0.5835 0.603 0.591 0.5845 0.6059]
Y = [0.501 0.474 0.467 0.4965 0.4435 0.2395 0.5 0.464 0.476 0.491]
boxplot({Y, Y2})
title ("Win rate statistics");
set(gca (), "xtick", [1 2], "xticklabel", {"A-C 2000", "ICO 2000"})
axis ([0,3]);

size = 1851
x = 1:size;
Y = [weight{1}(1:size) weight{2}(1:size) weight{3}(1:size) weight{4}(1:size)];
subplot(2, 1, 1)
plot (x, Y, "linewidth", 1.2);
set(gca, "linewidth", 1, "fontsize", 11)
title ("Input weight, and win rate progression");
xlabel ("Game Number");
ylabel ("Input Weights");

x = 1:size;
Y = [weight{5}(1:size)];
subplot(2, 1, 2)
plot (x, Y, "linewidth", 1.2);
set(gca, "linewidth", 1, "fontsize", 11)
title ("");
xlabel ("Game Number");
ylabel ("Win Rate");



%-------------------------------------------------------


size = 2000
x = 1:size;
Y = [weight{1}(1:size) weight{2}(1:size) weight{3}(1:size) weight{4}(1:size)];
subplot(1, 2, 1)
plot (x, Y, "linewidth", 1.2);
set(gca, "linewidth", 1, "fontsize", 11)
title ("Optimal run");
xlabel ("Game Number");
ylabel ("Input Weights");


size = 2000
x = 1:size;
Y = [weight{1}(1:size) weight{2}(1:size) weight{3}(1:size) weight{4}(1:size)];
subplot(1, 2, 2)
plot (x, Y, "linewidth", 1.2);
set(gca, "linewidth", 1, "fontsize", 11)
title ("Outlier run");
xlabel ("Game Number");
ylabel ("Input Weights");


