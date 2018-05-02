clear all
[filename, path] = uigetfile('*.csv', 'Select a csv file');

% Open the text file.
logs = csvread(strcat(path,filename));

logs = logs(~(logs(:,2) == 0), :);
logs = logs(~(logs(:,3) == 0), :);

% calculate runtimes
runTimes = logs(:,3) - logs(:,2);

% find data that isn't correct and remove it
prunedDataIndex = runTimes > 0;
logs = logs(prunedDataIndex,:);
runTimes = runTimes(prunedDataIndex);

smallestTime = min([logs(:,2); logs(:,3)]);
largestTime = max([logs(:,2); logs(:,3)]);

% start time at reletive zero
logs(:,2:3) = logs(:,2:3) - smallestTime;

% calculate stats
averageRunTime = mean(runTimes);


uniqueIds = unique(logs(:,1));

idIndex = false(length(logs(:,1)),length(uniqueIds));

for i = 1:length(uniqueIds)
    idIndex(:,i) = logs(:,1) == uniqueIds(i);
end

bplotFreq = [0,0];
for i = 1:length(uniqueIds)
    period{i} = abs(diff(logs(idIndex(:,i),2)));
    freq{i} = 1./period{i};
    
%     ax = subplot(1,8,i);
%     boxplot(ax, freq{i},'ExtremeMode','clip','DataLim',[0, 40]);
%     hold on
%     plot(mean(freq{i}), 'dg')
%     hold off
    
    P = [freq{i}, (zeros(length(period{i}),1) + i)];
    bplotFreq = [bplotFreq ; P];
end

bplotFreq = bplotFreq(2:end,:);

figure;
boxplot(bplotFreq(:,1), bplotFreq(:,2),'ExtremeMode','clip','DataLim',[0, 40]);
title('Frequency Box Plot');
ylabel('Frequency (Hz)');
xlabel('Task');

for i = 1:length(uniqueIds)
    disp(sprintf('median period of task %d is: %f', uniqueIds(i), median(period{i})))
    disp(sprintf('median frequency of task %d is: %f', uniqueIds(i), median(freq{i})))
    disp(sprintf('WCET of task %d is: %f', uniqueIds(i), max(runTimes(idIndex(:,i)))))
    meanRunTime(i) = mean(runTimes(idIndex(:,i)));
    disp(sprintf('mean execution time of task %d is: %f', uniqueIds(i), meanRunTime(i)))
    disp(sprintf('median excution time of task %d is: %f', uniqueIds(i), median(runTimes(idIndex(:,i)))))
    stdRunTime(i) = std(runTimes(idIndex(:,i)));
    disp(sprintf('Std of execution time of task %d is: %f', uniqueIds(i), stdRunTime(i)))
end

figure;
ax = axes(gcf);

for i = 1:length(uniqueIds)
    plotOutliers(ax,i,runTimes(idIndex(:,i)), meanRunTime(i)+2*stdRunTime(i));
end

function a = plotOutliers(ax, center, executionTimes, thresh)
    hold(ax,'on');
    line(ax, [(center - .5) (center + .5)], [thresh thresh], 'Color', 'r');
    outliersY = executionTimes(executionTimes > thresh);
    outliersX = zeros(size(outliersY)) + center;
    line(ax, outliersX, outliersY, 'LineStyle', 'none', 'Marker', '*');
    ax.XLim = [0 center];
end