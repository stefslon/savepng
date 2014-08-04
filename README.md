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
|          1	|   0.078092	|   0.037568	|   0.011792	|        6.6	| 
|          2	|   0.074537	|   0.030715	|   0.011478	|        6.5	| 
|          3	|   0.075276	|   0.030899	|   0.014338	|        5.3	| 
|          4	|   0.074957	|   0.031101	|   0.012850	|        5.8	| 
|          5	|   0.074303	|   0.031902	|   0.014533	|        5.1	| 
|          6	|   0.074380	|   0.030610	|   0.021501	|        3.5	| 
|          7	|   0.073926	|   0.031301	|   0.029382	|        2.5	| 
|          8	|   0.075175	|   0.031349	|   0.041195	|        1.8	| 
|          9	|   0.073286	|   0.030612	|   0.050499	|        1.5	| 
|         10	|   0.073114	|   0.030279	|   0.068248	|        1.1	| 
 
### File Size [bytes]

| Compression	|    IMWRITE	| PNGENCODER	|    SAVEPNG	| Improvement	| 
|       ----	|       ----	|       ----	|       ----	|       ----	| 
|          1	|   43536.13	|   66355.20	|   61531.80	|    141.25%	| 
|          2	|   43536.13	|   66355.20	|   53347.23	|    122.62%	| 
|          3	|   43536.13	|   66355.20	|   41360.17	|     95.17%	| 
|          4	|   43536.13	|   66355.20	|   44269.13	|    101.80%	| 
|          5	|   43536.13	|   66355.20	|   41360.17	|     95.17%	| 
|          6	|   43536.13	|   66355.20	|   36781.17	|     84.67%	| 
|          7	|   43536.13	|   66355.20	|   35574.13	|     81.89%	| 
|          8	|   43536.13	|   66355.20	|   35637.77	|     82.07%	| 
|          9	|   43536.13	|   66355.20	|   34875.07	|     80.31%	| 
|         10	|   43536.13	|   66355.20	|   33888.73	|     78.06%	| 
