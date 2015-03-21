function savepng(CDATA,filename,varargin) %#ok<INUSD>
% SAVEPNG
%   Very fast PNG image compression routine.
%
%   Input syntax is:
%   savepng(CDATA,filename[,Compression]);
%
%   Optional parameters:
%       Compression     A number between 0 and 10 controlling the amount of 
%                       compression to try to achieve with PNG file. 0 implies
%                       no compresson, fastest option. 10 implies the most
%                       amount of compression, slowest option. Default
%                       value is 4.
%       Resolution      This argument specifies the resolution of the file 
%                       being saved. Resolution is expressed in Dots-Per-Inch 
%                       (DPI). Default resolution is 96 DPI.
%
%   Example 1:
%       img     = getframe(gcf);
%       savepng(img.cdata,'example.png');
%
%   Example 2:
%       img     = getframe(gcf);
%       savepng(img.cdata,'exampleHighRes.png',10,300);
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
%   03/14/2014, Brought miniz.c to the latest version r63 (from Oct 13, 2013)
%               Changed compression limits from 0 to 10 to align with miniz
%   08/04/2014, Added option to command image resolution in DPI

% Compile string
try
    mex savepng.cpp -DMINIZ_NO_TIME -DMINIZ_NO_ARCHIVE_APIS -DMINIZ_NO_ARCHIVE_WRITING_APIS -DMINIZ_NO_ZLIB_APIS -DMINIZ_NO_ZLIB_COMPATIBLE_NAMES
catch
    error('Sorry, auto-compilation failed.');
end
