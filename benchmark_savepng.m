%
%   Benchmark savepng with reference to built-in imwrite 
%

N   = 30;                        % Number of samples in each test
C   = [1 2 3 4 5 6 7 8 9 10 11]; % Compression levels to test

Nc  = numel(C);

% Pre-allocate results storage
testTime    = zeros(N,Nc,2);
testSize    = zeros(N,Nc,2);

% Make sure MEX is loaded into memory 
try
    savepng
catch
end


figure('Renderer','zbuffer','Color','w');
pos = get(gcf,'Position');
set(gcf,'Position',[pos(1) pos(2) pos(3)*1.5 pos(4)*1.5]);
movegui(gcf,'center');
Z = peaks(100);
surf(Z);

for iC=1:Nc
    for iN=1:N
        
        % Change captured image every time
        view(iN*2,iN*2);
        
        img     = getframe(gcf);
        
        % Test built-in IMWRITE
        tic;
        imwrite(img.cdata,'example1.png');
        testTime(iN,iC,1) = toc;
        s = dir('example1.png');
        testSize(iN,iC,1) = s.bytes;
        
        % Test SAVEPNG
        tic;
        savepng(img.cdata,'example2.png',C(iC));
        testTime(iN,iC,2) = toc;
        s = dir('example2.png');
        testSize(iN,iC,2) = s.bytes;
        
    end
end

% Summarize results in a GitHub flavours markdown format
fprintf('\nSave Time [sec]\n\n');
fprintf('| %10s\t| %10s\t| %10s\t| %10s\t| \n','Compression','IMWRITE','SAVEPNG','Improvement');
fprintf('| %10s\t| %10s\t| %10s\t| %10s\t| \n','----','----','----','----');
fprintf('| %10d\t| %10f\t| %10f\t| %10.1f\t| \n', ...
    cat(1,C,squeeze(mean(testTime,1)).',mean(testTime(:,:,1)./testTime(:,:,2),1)));


fprintf('\nFile Size [bytes]\n\n');
fprintf('| %10s\t| %10s\t| %10s\t| %10s\t| \n','Compression','IMWRITE','SAVEPNG','Improvement');
fprintf('| %10s\t| %10s\t| %10s\t| %10s\t| \n','----','----','----','----');
fprintf('| %10d\t| %10.2f\t| %10.2f\t| %9.2f%%\t| \n', ...
    cat(1,C,squeeze(mean(testSize,1)).',mean(testSize(:,:,2)./testSize(:,:,1),1)*100));

% Generate graph
close(gcf);

figure('Color','w');

pos = get(gcf,'Position');
set(gcf,'Position',[pos(1) pos(2) pos(3)*2 pos(4)*1.4]);
movegui(gcf,'center');

subplot(2,1,1); set(gca,'FontSize',8);
plot(testTime(:,1,1).*1e3,'b','LineWidth',2);
hold on;
plot(testTime(:,:,2).*1e3);
grid on;
xlabel('Run #');
ylabel('Time [msec]');
title('\bfSave Time');
legend( cellstr(char('IMWRITE',cat(2,repmat('C = ',Nc,1),num2str(C')))) ,'Location','EastOutside');

subplot(2,1,2); set(gca,'FontSize',8);
plot(testSize(:,1,1)./1024,'b','LineWidth',2);
hold on;
plot(testSize(:,:,2)./1024);
grid on;
xlabel('Run #');
ylabel('File Size [kb]');
title('\bfFile Compression');
legend( cellstr(char('IMWRITE',cat(2,repmat('C = ',Nc,1),num2str(C')))) ,'Location','EastOutside');

img     = getframe(gcf);
savepng(img.cdata,'Benchmark_Results.png');

% Clean up
if exist('example1.png','file'), delete('example1.png'); end
if exist('example2.png','file'), delete('example2.png'); end

