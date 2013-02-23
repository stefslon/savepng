function savepng(CDATA,filename,varargin) %#ok<INUSD>
% SAVEPNG
%   Very fast PNG image compression routine.
%
%   Input syntax is:
%   savepng(CDATA,filename[,Compression]);
%
%   Optional parameters:
%       Compression     A number between 0 and 4095 controlling the amount of 
%                       compression to try to achieve with PNG file. 0 implies
%                       no compresson, fastest option. 4095 implies the most
%                       amount of compression, slowest option. Default
%                       value is 8.
%
%   Example:
%       img     = getframe(gcf);
%       savepng(img.cdata,'example.png');
%
%   PNG encoding routine based on public-domain MINIZ library:
%   http://code.google.com/p/miniz/
%

% Author: S.Slonevskiy, 02/18/2013
% File bug reports at: 
%       https://github.com/stefslon/savepng/issues

% Versions:
%   02/18/2013, Initial version
%	02/22/2013, Added another switch to MEX compile

% Compile string
try
    mex savepng.c -DMINIZ_ML_MALLOC -DMINIZ_NO_TIME -DMINIZ_NO_ARCHIVE_APIS -DMINIZ_NO_ARCHIVE_WRITING_APIS -DMINIZ_NO_ZLIB_APIS -DMINIZ_NO_ZLIB_COMPATIBLE_NAMES
catch
    error('Sorry, auto-compilation failed.');
end
