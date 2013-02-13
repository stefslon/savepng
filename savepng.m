%
%   PNG encoding routine based on MINIZ library:
%   http://code.google.com/p/miniz/
%
%   The syntax is:
%
%       savepng(CDATA,filename[,Compression]);
%
%   Optional parameters:
%       Compression     A number between 0 and 4095 controlling the amount of 
%                       compression to try to achieve with PNG file. 0 implies
%                       no compresson, fastest option. 4095 implies the most
%                       amount of compression, slowest option.
%
%   Example:
%       img     = getframe(gcf);
%       savepng(img.cdata,'example.png');
%

% Author: S.Slonevskiy, 02/13/2013
% File bug reports at: 
%       https://github.com/stefslon/exportToPPTX/issues

% Versions:
%   02/13/2013, Initial version


% Compile string
mex savepng.c -DMINIZ_NO_TIME -DMINIZ_NO_ARCHIVE_APIS -DMINIZ_NO_ARCHIVE_WRITING_APIS -DMINIZ_NO_ZLIB_APIS -DMINIZ_NO_ZLIB_COMPATIBLE_NAMES 