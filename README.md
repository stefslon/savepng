## Overview

savepng is a very efficient PNG image compression MEX-routine that outperforms MatLab's built-in imwrite in compression times and produced file sizes. For a typical figure size at the default compression level, savepng is ~5.8 times faster, with a file size identical to imwrite. At the highest compression level, savepng takes about the same amount of time as imwrite, but produces a file sizes that is ~20% smaller.

PNG encoding is based on public-domain [MINIZ library](http://code.google.com/p/miniz/).

## Usage

```matlab
savepng(CDATA,filename[,Compression[,Resolution]])
```

Where,

* `CDATA` is a standard MatLab image m-by-n-by-3 matrix. This matrix can be obtained using `getframe` command or, for a faster implementation, [undocumented hardcopy command](http://www.mathworks.com/support/solutions/en/data/1-3NMHJ5/)
* `filename` file name of the image to write. Don't forget to add .png to the file name.
* `Compression` Optional input argument. This argument takes on a number between 0 and 10 controlling the amount of compression. 0 implies no compresson, fastest option (though with more I/O this is not neccessarily the fastest option). 10 implies the highest level of compression, slowest option. Default value is 4.
* `Resolution` Optional input argument. This argument specifies the resolution of the file being saved. Resolution is expressed in Dots-Per-Inch (DPI). Default resolution is 96 DPI.

## Speed and File Size Comparison

Note: PngEncoder in the figure and tables below is [objectplanet's PngEncoder](http://objectplanet.com/pngencoder/), which is the only other fast alternative I was able to find.

![alt text](https://raw.github.com/stefslon/savepng/master/Benchmark_Results.png "Performance Comparison")

### Save Time [sec]

| Compression	|    IMWRITE	| PNGENCODER	|    SAVEPNG	| Improvement	| 
|       ----	|       ----	|       ----	|       ----	|       ----	| 
|          1	|   0.143407	|   0.084257	|   0.053067	|        7.9	| 
|          2	|   0.121808	|   0.051726	|   0.025867	|        7.9	| 
|          3	|   0.117392	|   0.048554	|   0.028616	|        6.4	| 
|          4	|   0.125937	|   0.051692	|   0.016363	|        8.0	| 
|          5	|   0.107091	|   0.052964	|   0.019517	|        6.0	| 
|          6	|   0.104467	|   0.053038	|   0.030763	|        4.0	| 
|          7	|   0.105115	|   0.035728	|   0.032431	|        3.3	| 
|          8	|   0.125733	|   0.044529	|   0.044563	|        2.8	| 
|          9	|   0.103668	|   0.036897	|   0.057042	|        1.8	| 
|         10	|   0.106583	|   0.035678	|   0.075044	|        1.4	| 
 
### File Size [bytes]

| Compression	|    IMWRITE	| PNGENCODER	|    SAVEPNG	| Improvement	| 
|       ----	|       ----	|       ----	|       ----	|       ----	| 
|          1	|   43536.13	|   66355.20	|   61552.80	|    141.30%	| 
|          2	|   43536.13	|   66355.20	|   53368.23	|    122.67%	| 
|          3	|   43536.13	|   66355.20	|   41381.17	|     95.22%	| 
|          4	|   43536.13	|   66355.20	|   44290.13	|    101.85%	| 
|          5	|   43536.13	|   66355.20	|   41381.17	|     95.22%	| 
|          6	|   43536.13	|   66355.20	|   36802.17	|     84.71%	| 
|          7	|   43536.13	|   66355.20	|   35595.13	|     81.94%	| 
|          8	|   43536.13	|   66355.20	|   35658.77	|     82.12%	| 
|          9	|   43536.13	|   66355.20	|   34896.07	|     80.36%	| 
|         10	|   43536.13	|   66355.20	|   33909.73	|     78.11%	| 
