/*                                                              
 *
 *  PNG encoding routine based on MINIZ library:
 *  http://code.google.com/p/miniz/
 *
 *  The syntax is:
 *
 *      savepng(CDATA,filename[,Compression]);
 *
 *  Optional parameters:
 *      Compression     A number between 0 and 4095 controlling the amount of 
 *                      compression to try to achieve with PNG file. 0 implies
 *                      no compresson, fastest option. 4095 implies the most
 *                      amount of compression, slowest option.
 *
 */


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
    unsigned char *outdata;
    FILE* file;
    
    char *filename;
    size_t filenamelen;
    size_t filelen;
    
    /* Default number of probes */
    max_probes = 8;
    
    /* Check for proper number of arguments */
    if(nrhs<2) {
        mexErrMsgIdAndTxt("savepng:nrhs","At least two inputs required.");
    }
    
    /* Check if compression level is commanded */
    if(nrhs==3) {
        max_probes = mxGetScalar(prhs[2]);
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
    filename = mxMalloc(filenamelen);
    mxGetString(prhs[1], filename, (mwSize)filenamelen);   
    
    /* some debug information */
    // mexPrintf("Input dimensions %d by %d \n",width,height);
    // mexPrintf("Dimensions from array %d, %d, %d \n",dim_array[0],dim_array[1],dim_array[2]);
    // mexPrintf("Input filename %s \n",filename);
    
    /* Convert MatLab image to raw pixels */
    imgdata = mxMalloc(width * height * 3);
    idx = 0;
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++) 
        {
            imgdata[idx++] = indata[x*height + y]; /* red */
            imgdata[idx++] = indata[1*width*height + x*height + y]; /* green */
            imgdata[idx++] = indata[2*width*height + x*height + y]; /* blue */
            //imgdata[idx++] = 255;               /* alpha */
        }
    }
    
    /* Encode PNG in memory */
    // Parameter "3" implies RGB pixel format
    outdata = tdefl_write_image_to_png_file_in_memory(imgdata, width, height, max_probes, 3, &filelen);
    
    /* Write to file */
    file = fopen(filename, "wb" );
    if(!file) return 0;
    fwrite((char*)outdata, 1, filelen, file);
    fclose(file);

    /* When finished using image data and filename string, deallocate it. */
    mxFree(filename);
    mxFree(imgdata);
    mxFree(outdata);  

}


