%
%   Benchmark savepng with reference to built-in imwrite
%

N   = 30;               % Number of samples in each test
Q   = [1 4 8 64 4095];    % Compression levels to test

Nq  = numel(Q);

% Pre-allocate results storage
testTime    = zeros(N,Nq,3);
testSize    = zeros(N,Nq,3);

% Setup objectplanet's PngEncoder
if ~exist('PngEncoder','var') && ...
        exist('com.objectplanet.image.PngEncoder.jar','file'),
    javaaddpath('com.objectplanet.image.PngEncoder.jar');
    % Init Java object
    PngEncoder  = com.objectplanet.image.PngEncoder( ...
        com.objectplanet.image.PngEncoder.COLOR_TRUECOLOR, ...
        com.objectplanet.image.PngEncoder.BEST_SPEED);
end


figure('Renderer','zbuffer','Color','w');
pos = get(gcf,'Position');
set(gcf,'Position',[pos(1) pos(2) pos(3)*2 pos(4)*2]);
movegui(gcf,'center');
Z = peaks(100);
mesh(Z);

for iQ=1:Nq,
    for iN=1:N,
        
        % Change captured image every time
        view(iN*2,iN*2);
        
        img     = getframe(gcf);
        
        % Test built-in IMWRITE
        tic;
        imwrite(img.cdata,'example1.png');
        testTime(iN,iQ,1) = toc;
        s = dir('example1.png');
        testSize(iN,iQ,1) = s.bytes;
        
        % Test PngEncoder
        if exist('PngEncoder','var'),
            tic;
            jimage      = im2java(img.cdata);
            imgfile     = java.io.FileOutputStream('example2.png');
            PngEncoder.encode(jimage,imgfile);
            imgfile.flush();
            imgfile.close();
            testTime(iN,iQ,2) = toc;
            s = dir('example2.png');
            testSize(iN,iQ,2) = s.bytes;
        end
        
        % Test SAVEPNG
        tic;
        savepng(img.cdata,'example3.png',Q(iQ));
        testTime(iN,iQ,3) = toc;
        s = dir('example3.png');
        testSize(iN,iQ,3) = s.bytes;
        
    end
end

% Summarize results
fprintf('\nSave Time [sec]\n');
fprintf('%10s\t %10s\t %10s\t %10s\t %10s\n ','Quality','IMWRITE','PNGENCODER','SAVEPNG','Improvement');
fprintf('%10d\t %10f\t %10f\t %10f\t %10f\n ', ...
    cat(1,Q,squeeze(mean(testTime,1)).',mean(testTime(:,:,1)./testTime(:,:,3),1)));


fprintf('\nFile Size [bytes]\n');
fprintf('%10s\t %10s\t %10s\t %10s\t %10s\n ','Quality','IMWRITE','PNGENCODER','SAVEPNG','Improvement');
fprintf('%10d\t %10.2f\t %10.2f\t %10.2f\t %10f\n ', ...
    cat(1,Q,squeeze(mean(testSize,1)).',mean(testSize(:,:,1)./testSize(:,:,3),1)));

% Generate graph
close(gcf);

figure('Color','w');

pos = get(gcf,'Position');
set(gcf,'Position',[pos(1) pos(2) pos(3)*2 pos(4)*1.4]);
movegui(gcf,'center');

subplot(2,1,1); set(gca,'FontSize',8);
plot(testTime(:,1,1).*1e3,'b','LineWidth',2);
hold on;
plot(testTime(:,1,2).*1e3,'r','LineWidth',2);
plot(testTime(:,:,3).*1e3);
grid on;
xlabel('Run #');
ylabel('Time [msec]');
title('\bfSave Time');
legend( cellstr(char('IMWRITE','PNGENCODER',cat(2,repmat('Q = ',Nq,1),num2str(Q')))) ,'Location','EastOutside');

subplot(2,1,2); set(gca,'FontSize',8);
plot(testSize(:,1,1)./1024,'b','LineWidth',2);
hold on;
plot(testSize(:,1,2)./1024,'r','LineWidth',2);
plot(testSize(:,:,3)./1024);
grid on;
xlabel('Run #');
ylabel('File Size [kb]');
title('\bfFile Compression');
legend( cellstr(char('IMWRITE','PNGENCODER',cat(2,repmat('Q = ',Nq,1),num2str(Q')))) ,'Location','EastOutside');

img     = getframe(gcf);
savepng(img.cdata,'Benchmark_Results.png');

% Clean up
delete('example1.png');
delete('example2.png');
delete('example3.png');

