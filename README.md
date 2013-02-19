## Overview

savepng is a very efficient PNG image compression routine that outperforms MatLab's built-in imwrite in compression times and smaller file sizes.

PNG encoding is based on public-domain [MINIZ library](http://code.google.com/p/miniz/).

## Usage

```matlab
savepng(CDATA,filename[,Compression])
```

Where,

* `CDATA` is a standard MatLab image m-by-n-by-3 matrix. This matrix can be obtained using `getframe` command or, for a faster implementation, [undocumented hardcopy command](http://www.mathworks.com/support/solutions/en/data/1-3NMHJ5/)
* `filename` file name of the image to write. Don't forget to add .png to the file name.
* `Compression` Optional input argument. This argument takes on a number between 0 and 4095 controlling the amount of                     compression to try to achieve with PNG file. 0 implies no compresson, fastest option. 4095 implies the most amount of compression, slowest option. Default value is 8.

## Speed and File Size Comparison

Note: PngEncoder in the figure and tables below is [objectplanet's PngEncoder](http://objectplanet.com/pngencoder/), which is the only other fast alternative I was able to find.

![alt text](https://raw.github.com/stefslon/savepng/master/Benchmark_Results.png "Performance Comparison")

### Save Time [sec]

|   Quality	|    IMWRITE	| PNGENCODER	|    SAVEPNG	| Improvement	| 
|       ----	|       ----	|       ----	|       ----	|       ----	| 
|          1	|   0.141927	|   0.064678	|   0.025635	|   5.727868	| 
|          4	|   0.127750	|   0.052499	|   0.022406	|   5.733489	| 
|          8	|   0.132577	|   0.055713	|   0.022846	|   5.821804	| 
|         64	|   0.128273	|   0.052453	|   0.029980	|   4.288976	| 
|       4095	|   0.128502	|   0.052923	|   0.046395	|   2.777765	| 
 
### File Size [bytes]

|   Quality	|    IMWRITE	| PNGENCODER	|    SAVEPNG	| Improvement	| 
|       ----	|       ----	|       ----	|       ----	|       ----	| 
|          1	|   69394.53	|  107448.83	|   96119.83	|   0.727982	| 
|          4	|   69394.53	|  107448.83	|   85958.17	|   0.811632	| 
|          8	|   69394.53	|  107448.83	|   81293.93	|   0.856658	| 
|         64	|   69394.53	|  107448.83	|   63869.70	|   1.087186	| 
|       4095	|   69394.53	|  107448.83	|   58686.30	|   1.180958	| 
