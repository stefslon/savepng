// % SAVEPNG
// %   Very fast PNG image compression routine.
// %
// %   Input syntax is:
// %   savepng(CDATA,filename[,Compression]);
// %
// %   Optional parameters:
// %       Compression     A number between 0 and 14 controlling the amount of 
// %                       compression to try to achieve with PNG file. 0 implies
// %                       no compresson, fastest option. 14 implies the most
// %                       amount of compression, slowest option. Default
// %                       value is 4.
// %       Resolution      This argument specifies the resolution of the file 
// %                       being saved. Resolution is expressed in Dots-Per-Inch 
// %                       (DPI). Default resolution is 96 DPI.
// %
// %   Example 1:
// %       img     = getframe(gcf);
// %       savepng(img.cdata,'example.png');
// %
// %   Example 2:
// %       img     = getframe(gcf);
// %       savepng(img.cdata,'exampleHighRes.png',10,300);
// %
// %   PNG encoding routine based on fpng (levels 0-2) and libdeflate (3-14):
// %   https://github.com/richgel999/fpng
// %   https://github.com/ebiggers/libdeflate
// %
// 
// % Author: S.Slonevskiy, 02/18/2013
// % File bug reports at: 
// %       https://github.com/stefslon/savepng/issues
// 
// % Versions:
// %   02/18/2013, Initial version
// %   02/22/2013, Added another switch to MEX compile
// %   03/14/2014, Brought miniz.c to the latest version r63 (from Oct 13, 2013)
// %               Changed compression limits from 0 to 10 to align with miniz
// %   08/04/2014, Added option to command image resolution in DPI
// %   11/25/2016, Added support for alpha channel
// %   11/21/2025, Complete re-write to use fpng and libdeflate for faster compression

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "mex.h"
#include "matrix.h"

#include "fpng.h"
#include "libdeflate_amalgamated.h"

static uint8_t fpng_initialized = false;

/*
 * Notes:
 * - imwrite() writes out PNG in data chunks of 8192 bytes
 * - imwrite() also writes tIME chunk 
 * - fpng writes fdEC chunk - this acts as a signal to fpng decoder 
 */

/* Translate a long integer from host byte order to network byte order */
uint32_t htonl(uint32_t x) {
    return ((x & 0x000000FF) << 24) |
           ((x & 0x0000FF00) << 8)  |
           ((x & 0x00FF0000) >> 8)  |
           ((x & 0xFF000000) >> 24);
}

/* Simple PNG writer function by Alex Evans, 2011. Released into the public domain: https://gist.github.com/908299
 * This is actually a modification to support libdeflate */
uint8_t* write_image_to_png_file_in_memory(void *img, int32_t w, int32_t h, int32_t numchans, int8_t level, uint32_t dpm, uint32_t &len_out) 
{
    // Scan line length
    int32_t p = w * numchans;
    
    // Prepare the raw data buffer
    size_t raw_len = (size_t)(1 + p) * h;
    uint8_t *raw_buf = (uint8_t*)malloc(raw_len);
    if (!raw_buf) return 0;

    for (int32_t y = 0; y < h; ++y) {
        raw_buf[y * (1 + p)] = 0; // Filter type 0 (None)
        memcpy(raw_buf + y * (1 + p) + 1, ((uint8_t*)img) + y * p, p);
    }

    // Set up libdeflate compressor
    struct libdeflate_compressor *compressor = libdeflate_alloc_compressor(level);
    if (!compressor) {
        free(raw_buf);
        return 0;
    }

    // Calculate bound and allocate output buffer
    // Overhead: 62 (Header) + 4 (IDAT CRC) + 12 (IEND Chunk) = 78 bytes
    size_t bound = libdeflate_zlib_compress_bound(compressor, raw_len);
    uint8_t *zbuf = (uint8_t*)malloc(78 + bound); 
    if (!zbuf) {
        libdeflate_free_compressor(compressor);
        free(raw_buf);
        return 0;
    }

    // Compress
    // Output writes to zbuf + 62, leaving room for the PNG header
    size_t compressed_size = libdeflate_zlib_compress(compressor, raw_buf, raw_len, zbuf + 62, bound);
    
    libdeflate_free_compressor(compressor);
    free(raw_buf);

    if (compressed_size == 0) {
        free(zbuf);
        return 0;
    }

    len_out = (uint32_t)compressed_size;

    // Construct PNG Header (IHDR + IDAT start)
    static const uint8_t chans[] = { 0x00, 0x00, 0x04, 0x02, 0x06 };
    uint8_t pnghdr[62] = { 0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, // PNG signature
                /*  8 */   0x00, 0x00, 0x00, 0x0d,    // IHDR chunk size
                /* 12 */   0x49, 0x48, 0x44, 0x52,    // IHDR
                /* 16 */   0x00, 0x00, (uint8_t)(w>>8), (uint8_t)w, //
                           0x00, 0x00, (uint8_t)(h>>8), (uint8_t)h, // dimensions field
                           0x08, chans[numchans], 0x00, 0x00, 0x00, //
                /* 29 */   0x00, 0x00, 0x00, 0x00,    // CRC
                /* 33 */   0x00, 0x00, 0x00, 0x09,    // pHYs chunk size
                /* 37 */   0x70, 0x48, 0x59, 0x73,    // pHYs
                /* 41 */   0x00, 0x00, (uint8_t)(dpm>>8), (uint8_t)dpm, // Pixels per unit, X axis
                           0x00, 0x00, (uint8_t)(dpm>>8), (uint8_t)dpm, // Pixels per unit, Y axis
                           0x01,  // Unit specifier (meters)
                /* 50 */   0x00, 0x00, 0x00, 0x00,    // CRC
                /* 54 */   (uint8_t)(len_out>>24), (uint8_t)(len_out>>16) ,(uint8_t)(len_out>>8), (uint8_t)len_out, // IDAT chunk size
                /* 58 */   0x49, 0x44, 0x41, 0x54 };  // IDAT
    
    // Calculate CRC for IHDR and pHYs
    *(uint32_t*)(pnghdr+29) = htonl(libdeflate_crc32(0, pnghdr+12, 17));
    *(uint32_t*)(pnghdr+50) = htonl(libdeflate_crc32(0, pnghdr+37, 13));
    
    memcpy(zbuf, pnghdr, 62);

    // Calculate CRC for IDAT chunk
    // CRC includes chunk type "IDAT" (4 bytes) + Compressed Data
    // "IDAT" is located at zbuf + 58
    *(uint32_t*)(zbuf + 62 + len_out) = htonl(libdeflate_crc32(0, zbuf + 62 - 4, 4 + len_out));;

    // Append IEND chunk (Length 0, "IEND", CRC)
    // Fixed: Explicitly writing the 4-byte length (0) which was uninitialized in the original gist
    uint8_t footer[12] = { 0x00, 0x00, 0x00, 0x00, 
                           0x49, 0x45, 0x4e, 0x44,     // IEND
                           0xae, 0x42, 0x60, 0x82 };   // CRC
    memcpy(zbuf + 62 + len_out + 4, footer, 12);

    len_out += 78;
    return zbuf;
}

/* The gateway function */
void mexFunction( int nlhs, mxArray *plhs[], int nrhs, const mxArray *prhs[])
{
    uint8_t *imgdata = NULL;  /* packed raw pixel matrix */
    uint8_t *indata;          /* input image data matrix */
    uint32_t width, height, nchan;  /* size of matrix */
    uint8_t comp_level;       /* compression level */
    const mwSize *dim_array; 
    uint32_t x, y, idx;
    uint32_t dpm;             /* dots per meter */
    FILE* file;
    
    char *filename = NULL;
    size_t filenamelen;
    //size_t filelen;
    uint32_t filelen;
    
    /* Default number of probes */
    comp_level = 4;
    
    /* Default image resolution */
    dpm = (96.0*39.36996); // convert 96 DPI to DPM
    
    /* Check for proper number of arguments */
    if(nrhs<2) {
        mexErrMsgIdAndTxt("savepng:nrhs","At least two inputs required.");
    }
    
    /* Check if compression level is commanded */
    if(nrhs>=3) {
        comp_level = mxGetScalar(prhs[2]);
    }
    
    /* Check if image resolution is commanded and convert to DPI to DPM */
    if(nrhs>=4) {
        dpm = ((double)mxGetScalar(prhs[3])*39.36996);
    }
    
    /* Check probes range */
    if((comp_level<0) || (comp_level>15)) {
        mexErrMsgIdAndTxt("savepng:nrhs","Compression level must be between 0 and 14.");
    }
    
    /* Get the number of dimensions in the input argument. */
    dim_array = mxGetDimensions(prhs[0]);
    
    if((!mxIsUint8(prhs[0])) || (mxGetNumberOfDimensions(prhs[0])!=3) || !(dim_array[2]==3 || dim_array[2]==4)) {
        mexErrMsgIdAndTxt("savepng:nrhs","Input must in the image data format of MxNx3 or MxNx4 matrix of uint8.");
    }
    
    if (~fpng_initialized) {
        fpng::fpng_init();
        fpng_initialized = 1;
    }

    /* Pointer to image input data */
    indata = (uint8_t *)mxGetPr(prhs[0]); 

    /* Get dimensions of input matrices */
    nchan = dim_array[2];
    height = dim_array[0];  
    width = dim_array[1];

    /* Fetch output filename */
    filenamelen = mxGetN(prhs[1])*sizeof(mxChar)+1;
    filename = (char *)malloc(filenamelen);
    mxGetString(prhs[1], filename, (mwSize)filenamelen);
    
    /* Convert MATLAB image to raw pixels */
    /* indata format: RRRRRR..., GGGGGG..., BBBBBB... */
    /* outdata format: RGB, RGB, RGB, ... */
    imgdata = (uint8_t *)malloc(width * height * nchan);
    
    idx = 0;    
    for(y = 0; y < height; y++)
    {
        for(x = 0; x < width; x++) 
        {
            imgdata[idx++] = indata[x*height + y];                      /* red */
            imgdata[idx++] = indata[1*width*height + x*height + y];     /* green */
            imgdata[idx++] = indata[2*width*height + x*height + y];     /* blue */
            if (nchan==4)
            	imgdata[idx++] = indata[3*width*height + x*height + y]; /* alpha */
        }
    }
    
    /* Encode PNG in memory */
    if (comp_level<=2) {
        uint32_t fpng_flags = 0;
        if (comp_level==0)
            fpng_flags |= fpng::FPNG_FORCE_UNCOMPRESSED;
        else if (comp_level==2)
            fpng_flags |= fpng::FPNG_ENCODE_SLOWER;

        std::vector<uint8_t> outdata;
        if (fpng::fpng_encode_image_to_memory((uint8_t *)imgdata, width, height, nchan, outdata, fpng_flags))
        {
            /* Write to file */
            file = fopen(filename, "wb" );
            if(!file) return;
            fwrite(outdata.data(), 1, outdata.size(), file);
            fclose(file);
        }
    }
    else {
        uint8_t *outdata = NULL;
        outdata = (uint8_t * )write_image_to_png_file_in_memory((uint8_t *)imgdata, width, height, nchan, comp_level-2, dpm, filelen);

        /* Write to file */
        file = fopen(filename, "wb" );
        if(!file) return;
        fwrite(outdata, 1, filelen, file);
        fclose(file);
        
        if (outdata) free(outdata);
    }
    
    /* When finished using image data and filename string, deallocate it. */
    if (filename) free(filename);
    if (imgdata) free(imgdata);
    
}


