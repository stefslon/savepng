function savepng(CDATA,filename,varargin) %#ok<INUSD>
% SAVEPNG
%   Very fast PNG image compression routine.
%
%   Input syntax is:
%   savepng(CDATA,filename[,Compression]);
%
%   Optional parameters:
%       Compression     A number between 0 and 14 controlling the amount of 
%                       compression to try to achieve with PNG file. 0 implies
%                       no compresson, fastest option. 14 implies the most
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
%   PNG encoding routine based on fpng (levels 0-2) and libdeflate (3-14):
%   https://github.com/richgel999/fpng
%   https://github.com/ebiggers/libdeflate
%

% Author: S.Slonevskiy, 02/18/2013
% File bug reports at: 
%       https://github.com/stefslon/savepng/issues

% Versions:
%   02/18/2013, Initial version
%   02/22/2013, Added another switch to MEX compile
%   03/14/2014, Brought miniz.c to the latest version r63 (from Oct 13, 2013)
%               Changed compression limits from 0 to 10 to align with miniz
%   08/04/2014, Added option to command image resolution in DPI
%   11/25/2016, Added support for alpha channel
%   11/21/2025, Complete re-write to use fpng and libdeflate for faster compression

% Compile string
try
    mex -c libdeflate_amalgamated.c -largeArrayDims
    mex savepng.cpp fpng.cpp libdeflate_amalgamated.obj -largeArrayDims -DFPNG_NO_SSE=0 CXXFLAGS="$CXXFLAGS -msse4.1 -mpclmul"
    delete libdeflate_amalgamated.obj
catch
    error('Sorry, auto-compilation failed.');
end
