## Overview

savepng is a very efficient PNG image compression MEX-routine that outperforms MATLAB's built-in imwrite in compression times and produced file sizes. For a typical figure size at the default compression level, savepng is ~5.8 times faster, with a file size identical to imwrite. At the highest compression level, savepng can take more time than imwrite, but produce a file that is ~40% smaller.

Compression levels 0-2 are based on public-domain [fpng](https://github.com/richgel999/fpng)

Compression levels 3-14 are based on MIT licensed [libdeflate](https://github.com/ebiggers/libdeflate)

## Usage

```matlab
savepng(CDATA,filename[,Compression[,Resolution]])
```

Where,

* `CDATA` is a standard MATLAB image m-by-n-by-3 or m-by-n-by-4 (when supplying alpha channel) matrix. This matrix can be obtained using `getframe` command or, for a faster implementation use [undocumented hardcopy command](http://www.mathworks.com/support/solutions/en/data/1-3NMHJ5/)
* `filename` file name of the image to write. Don't forget to add .png to the file name.
* `Compression` Optional input argument. This argument takes on a number between 0 and 10 controlling the amount of compression. 0 implies no compresson, fastest option (though with more I/O this is not neccessarily the fastest option). 10 implies the highest level of compression, slowest option. Default value is 4.
* `Resolution` Optional input argument. This argument specifies the resolution of the file being saved. Resolution is expressed in Dots-Per-Inch (DPI). Default resolution is 96 DPI.

## Speed and File Size Comparison

![alt text](https://raw.github.com/stefslon/savepng/master/Benchmark_Results.png "Performance Comparison")

### Save Time [sec]

| Compression	|    IMWRITE	|  OLD SAVEPNG	|    SAVEPNG	| Improvement	| 
|       ----	|       ----	|       ----	|       ----	|       ----	| 
|          1	|   0.040558	|   0.011868	|   0.005611	|        7.3	| 
|          2	|   0.040394	|   0.011964	|   0.006555	|        6.2	| 
|          3	|   0.040248	|   0.014300	|   0.007754	|        5.3	| 
|          4	|   0.040182	|   0.013509	|   0.009586	|        4.2	| 
|          5	|   0.040481	|   0.014809	|   0.010105	|        4.0	| 
|          6	|   0.040486	|   0.018222	|   0.009939	|        4.1	| 
|          7	|   0.040066	|   0.021582	|   0.010500	|        3.8	| 
|          8	|   0.039954	|   0.027397	|   0.010973	|        3.6	| 
|          9	|   0.039727	|   0.032898	|   0.011946	|        3.3	| 
|         10	|   0.040032	|   0.046404	|   0.016177	|        2.5	| 
|         11	|   0.039856	|      N/A  	|   0.019164	|        2.1	| 
 
### File Size [bytes]

| Compression	|    IMWRITE	|  OLD SAVEPNG	|    SAVEPNG	| Improvement	| 
|       ----	|       ----	|       ----	|       ----	|       ----	| 
|          1	|  196774.30	|  151091.37	|  323900.30	|    164.20%	| 
|          2	|  196774.30	|  136489.40	|  279062.47	|    141.81%	| 
|          3	|  196774.30	|  120649.57	|  141057.57	|     72.91%	| 
|          4	|  196774.30	|  124359.17	|  126171.43	|     65.32%	| 
|          5	|  196774.30	|  120649.57	|  124357.17	|     64.46%	| 
|          6	|  196774.30	|  117487.83	|  123128.23	|     63.81%	| 
|          7	|  196774.30	|  116918.57	|  118990.70	|     61.66%	| 
|          8	|  196774.30	|  116715.43	|  117603.43	|     60.95%	| 
|          9	|  196774.30	|  116388.90	|  116300.00	|     60.26%	| 
|         10	|  196774.30	|  115713.00	|  115652.83	|     59.82%	| 
|         11	|  196774.30	|      N/A  	|  115602.50	|     59.79%	| 
