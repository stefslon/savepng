// % SAVEPNG
// %   Very fast PNG image compression routine.
// %
// %   Input syntax is:
// %   savepng(CDATA,filename[,Compression]);
// %
// %   Optional parameters:
// %       Compression     A number between 0 and 10 controlling the amount of 
// %                       compression to try to achieve with PNG file. 0 implies
// %                       no compresson, fastest option. 10 implies the most
// %                       amount of compression, slowest option. Default
// %                       value is 4.
// %
// %   Example:
// %       img     = getframe(gcf);
// %       savepng(img.cdata,'example.png');
// %
// %   PNG encoding routine based on public-domain MINIZ library:
// %   http://code.google.com/p/miniz/
// %
// 
// % Author: S.Slonevskiy, 02/18/2013
// % File bug reports at: 
// %       https://github.com/stefslon/savepng/issues
// 
// % Versions:
// %   02/18/2013, Initial version
// %   02/22/2013, Some miniz.c fixes to allow for compilation on LCC
// %   03/14/2014, Brought miniz.c to the latest version r63 (from Oct 13, 2013)
// %               Changed compression limits from 0 to 10 to align with miniz

#include <stdio.h>
#include <stdlib.h>

#include "mex.h"
#include "matrix.h"
        
#include "miniz.c"

/* The gateway function */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    unsigned char *imgdata;         /* packed raw pixel matrix */
    unsigned char *indata;          /* input image data matrix */
    unsigned width, height;         /* size of matrix */
    unsigned char max_probes;       /* compression level */
    const mwSize *dim_array; 
    unsigned x, y, chan, idx;
    unsigned dpm;                   /* dots per meter */
    unsigned char *outdata;
    FILE* file;
    
    char *filename;
    size_t filenamelen;
    size_t filelen;
    
    /* Default number of probes */
    max_probes = 4;
    
    /* Default image resolution */
    dpm = (96.0*39.36996);
    
    /* Check for proper number of arguments */
    if(nrhs<2) {
        mexErrMsgIdAndTxt("savepng:nrhs","At least two inputs required.");
    }
    
    /* Check if compression level is commanded */
    if(nrhs>=3) {
        max_probes = mxGetScalar(prhs[2]);
    }
    
    /* Check if image resolution is commanded */
    if(nrhs>=4) {
        dpm = ((double)mxGetScalar(prhs[3])*39.36996);
    }
    
    /* Check probes range */
    if((max_probes<0) || (max_probes>4095)) {
        mexErrMsgIdAndTxt("savepng:nrhs","Compression level must be between 0 and 4095.");
    }
    
    /* Get the number of dimensions in the input argument. */
    dim_array = mxGetDimensions(prhs[0]);
    
    if((!mxIsUint8(prhs[0])) || (mxGetNumberOfDimensions(prhs[0])!=3) || (dim_array[2]!=3)) {
        mexErrMsgIdAndTxt("savepng:nrhs","Input must in the image data format of MxNx3 matrix of uint8.");
    }

    /* Pointer to image input data */
    indata = (unsigned char *)mxGetPr(prhs[0]); 

    /* Get dimensions of input matrices */
    height = mxGetM(prhs[0]);  
    width = mxGetN(prhs[0])/3;     /* this dimension concatenates with the 3rd dimension */
    
    if(!mxIsUint8(prhs[0])) {
        mexErrMsgIdAndTxt("savepng:nrhs","Input must in the image data format of MxNx3 matrix of uint8.");
    }
    
    /* Fetch output filename */
    filenamelen = mxGetN(prhs[1])*sizeof(mxChar)+1;
    filename = (char *)mxMalloc(filenamelen);
    mxGetString(prhs[1], filename, (mwSize)filenamelen);
    
    /* Convert MatLab image to raw pixels */
    /* indata format: RRRRRR..., GGGGGG..., BBBBBB... */
    /* outdata format: RGB, RGB, RGB, ... */
    imgdata = (unsigned char *)mxMalloc(width * height * 3);
    
    idx = 0;    
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++) 
        {
            imgdata[idx++] = indata[x*height + y]; /* red */
            imgdata[idx++] = indata[1*width*height + x*height + y]; /* green */
            imgdata[idx++] = indata[2*width*height + x*height + y]; /* blue */
            /*imgdata[idx++] = 255;*/              /* alpha */
        }
    }
    
    /* Encode PNG in memory */
    /* Parameter "3" implies RGB pixel format */
    outdata = (unsigned char * )tdefl_write_image_to_png_file_in_memory_ex((unsigned char *)imgdata, width, height, 3, dpm, &filelen, max_probes, MZ_FALSE);
	
    /* Write to file */
    file = fopen(filename, "wb" );
    if(!file) return;
    fwrite((char*)outdata, 1, filelen, file);
    fclose(file);

    /* When finished using image data and filename string, deallocate it. */
    mxFree(filename);
    mxFree(imgdata);
    mxFree(outdata);  

}


