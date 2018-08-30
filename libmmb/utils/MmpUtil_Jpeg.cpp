/*
 * Copyright (c) 2014 Anapass Co., Ltd.
 *              http://www.anapass.com/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * Alternatively, Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*******************************************************************************
 * INCLUDE FILES                                                           
 ******************************************************************************/
#include "MmpDefine.h"
#include "MmpUtil.hpp"
#include "TemplateList.hpp"

extern "C" {
#include "jpeglib.h"
}

MMP_RESULT CMmpUtil::Jpeg_SW_YUV420Planar_Enc(unsigned char* Y, unsigned char* U, unsigned char* V, 
                               int image_width, int image_height,
                               char* jpegfilename, int quality) {
    
#if (MMP_OS==MMP_OS_WIN32)
    MMP_RESULT mmpResult = MMP_SUCCESS;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    
    FILE * outfile = NULL;		/* target file */
    
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
    unsigned char *yuv444data = NULL;
    
    int i,j,k;
    int row_stride;		/* physical row width in image buffer */
    int x, y;

    const int color_comp_num = 3;

    if(mmpResult == MMP_SUCCESS) {
    
        if ((outfile = fopen(jpegfilename, "wb")) == NULL) {
            //fprintf(stderr, "can't open %s\n", filename);
            mmpResult = MMP_FAILURE;
        }
    }

    if(mmpResult == MMP_SUCCESS) {
        
        yuv444data = (unsigned char*)malloc(image_width*image_height*color_comp_num);
        if(yuv444data == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {

            for(i = 0, j = 0, k = 0, y = 0; y < image_height; y++) {
            
                for(x = 0; x < image_width; x++) {
                  
                    yuv444data[i] = Y[j];
                    yuv444data[i+1] = U[k];
                    yuv444data[i+2] = V[k];

                    i+=3;
                    j++;
                    if( (x%2)==1) k++;
                }

                if( (y%2)==0) k-= (image_width>>1);

            } /* end of for(y = 0; y < image_height; y++) */
        }
    }

    if(mmpResult == MMP_SUCCESS) {

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

        jpeg_stdio_dest(&cinfo, outfile);

        cinfo.image_width = image_width; 	/* image width and height, in pixels */
        cinfo.image_height = image_height;
        cinfo.input_components = 3;		/* # of color components per pixel */
        cinfo.in_color_space = JCS_YCbCr; 	/* colorspace of input image */
      
        jpeg_set_defaults(&cinfo);

        if (quality < 0) quality = 0;
        else if(quality > 100) quality = 100;
        jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
        //jpeg_set_quality(&cinfo, quality, FALSE);

        jpeg_start_compress(&cinfo, TRUE);
        
        row_stride = image_width*3;

        while (cinfo.next_scanline < cinfo.image_height) {
            /* jpeg_write_scanlines expects an array of pointers to scanlines.
             * Here the array is only one element long, but you could pass
             * more than one scanline at a time if that's more convenient.
             */
            row_pointer[0] = &yuv444data[cinfo.next_scanline * row_stride];
            (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        
        jpeg_destroy_compress(&cinfo);

    }

    if(outfile != NULL) {
        fclose(outfile);
    }

    if(yuv444data != NULL) {
        free(yuv444data);
    }
#else

    MMP_UnusedParameter(Y);
    MMP_UnusedParameter(U);
    MMP_UnusedParameter(V);
    MMP_UnusedParameter(image_width);
    MMP_UnusedParameter(image_height);
    MMP_UnusedParameter(jpegfilename);
    MMP_UnusedParameter(quality);
    
#endif
    return MMP_SUCCESS;
}


/*
    JPEG ColorFormat

    Odysemus 
          Planar
          Semi-planar CBCR (4:4:4 not supported)
          Semi-planar CRCB (4:4:4 not supported)
          422 packed YUYV
          422 packed UYVY
          422 packed YVYU
          422 packed VYUY
          444 packed

     Android JPEG

          GrayScale


    MMPUTIL_JPEG_ENC_FORMAT_YUV444_PACKED_YUV
            
                 Y U V Y U V ...
                 Y U V Y U V ....

    MMPUTIL_JPEG_ENC_FORMAT_YUV422_PACKED_YUYV
    
                 Y U V V Y U V V ...
                 Y U V V Y U V V ...
                 
*/

MMP_RESULT CMmpUtil::Jpeg_SW_YUV420Planar_Enc(unsigned char* Y, unsigned char* U, unsigned char* V, 
                               int image_width, int image_height,
                               char* jpegfilename, int quality, 
                               int enc_format /* 0:yuv444  1:yuv422  2:yuv411  3:gray */
                               ) {

#if (MMP_OS==MMP_OS_WIN32)
    MMP_RESULT mmpResult = MMP_SUCCESS;

    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    
    FILE * outfile = NULL;		/* target file */
    
    JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
    unsigned char *picdata = NULL;
    
    int i,j,k;
    int row_stride;		/* physical row width in image buffer */
    int x, y;

    int color_comp_num = 3;
    J_COLOR_SPACE color_space = JCS_YCbCr;
    int scale_factor[6] = {1, 1, 1, 1, 1, 1};

    if(mmpResult == MMP_SUCCESS) {
    
        if ((outfile = fopen(jpegfilename, "wb")) == NULL) {
            //fprintf(stderr, "can't open %s\n", filename);
            mmpResult = MMP_FAILURE;
        }
    }

    if(mmpResult == MMP_SUCCESS) {
        
        picdata = (unsigned char*)malloc(image_width*image_height*4);
        if(picdata == NULL) {
            mmpResult = MMP_FAILURE;
        }
        else {

            switch(enc_format) {

                case MMPUTIL_JPEG_ENC_FORMAT_GRAY:

                    for(i = 0, j = 0, y = 0; y < image_height; y++) {
                        for(x = 0; x < image_width; x++) {
                          
                            picdata[i] = Y[j];
                            i++;
                            j++;
                        }
                    } /* end of for(y = 0; y < image_height; y++) */

                    color_comp_num = 1;
                    color_space = JCS_GRAYSCALE;

                    scale_factor[0] = 1; //h 
                    scale_factor[1] = 1; //v 

                    row_stride = image_width;
                    break;
            
                case MMPUTIL_JPEG_ENC_FORMAT_YUV422_PACKED_YUYV:

#if 0
                    for(i = 0, j = 0, k = 0, y = 0; y < image_height; y++) {
                        for(x = 0; x < image_width; x++) {
                          
                            picdata[i] = Y[j];

                            if(x%2 == 0)
                                picdata[i+1] = U[k];
                            else
                                picdata[i+1] = V[k];
                            
                            i+=2;
                            j++;
                            if( (x%2)==1) k++;
                        }

                        if( (y%2)==0) k-= (image_width>>1);
                    } /* end of for(y = 0; y < image_height; y++) */
#else
                    for(i = 0, j = 0, k = 0, y = 0; y < image_height; y++) {
                        for(x = 0; x < image_width; x++) {
                          
                            picdata[i] = Y[j];
                            picdata[i+1] = U[k];
                            picdata[i+2] = V[k];

                            i+=3;
                            j++;
                            if( (x%2)==1) k++;
                        }

                        if( (y%2)==0) k-= (image_width>>1);
                    } /* end of for(y = 0; y < image_height; y++) */

#endif

                    color_comp_num = 3;
                    color_space = JCS_YCbCr;

                    scale_factor[0] = 2; //h luma
                    scale_factor[1] = 1; //v luma
                    scale_factor[2] = 1; //h chroma
                    scale_factor[3] = 1; //v chroma
                    scale_factor[4] = 1; //h chroma
                    scale_factor[5] = 1; //v chroma
                    
                    row_stride = image_width*3;

                    break;

                case MMPUTIL_JPEG_ENC_FORMAT_RGB24BIT:

                    CMmpUtil::ColorConvertYUV420PlanarToRGB24(image_width, image_height, Y, U, V, picdata);
                    color_comp_num = 3;
                    color_space = JCS_RGB;

                    scale_factor[0] = 1; //h luma
                    scale_factor[1] = 1; //v luma
                    scale_factor[2] = 1; //h chroma
                    scale_factor[3] = 1; //v chroma
                    scale_factor[4] = 1; //h chroma
                    scale_factor[5] = 1; //v chroma
                    
                    row_stride = image_width*3;

                    break;

                case MMPUTIL_JPEG_ENC_FORMAT_YUV444_PACKED_YUV:
                default:

                    for(i = 0, j = 0, k = 0, y = 0; y < image_height; y++) {
                        for(x = 0; x < image_width; x++) {
                          
                            picdata[i] = Y[j];
                            picdata[i+1] = U[k];
                            picdata[i+2] = V[k];

                            i+=3;
                            j++;
                            if( (x%2)==1) k++;
                        }

                        if( (y%2)==0) k-= (image_width>>1);
                    } /* end of for(y = 0; y < image_height; y++) */

                    color_comp_num = 3;
                    color_space = JCS_YCbCr;

                    scale_factor[0] = 1; //h luma
                    scale_factor[1] = 1; //v luma
                    scale_factor[2] = 1; //h chroma
                    scale_factor[3] = 1; //v chroma
                    scale_factor[4] = 1; //h chroma
                    scale_factor[5] = 1; //v chroma
                    
                    row_stride = image_width*3;

                    break;

            }
        }
    }

    if(mmpResult == MMP_SUCCESS) {

        cinfo.err = jpeg_std_error(&jerr);
        jpeg_create_compress(&cinfo);

        jpeg_stdio_dest(&cinfo, outfile);

        cinfo.image_width = image_width; 	/* image width and height, in pixels */
        cinfo.image_height = image_height;
        cinfo.input_components = color_comp_num;		/* # of color components per pixel */
        cinfo.in_color_space = color_space; 	/* colorspace of input image */
        

        jpeg_set_defaults(&cinfo);

        cinfo.comp_info[0].h_samp_factor = scale_factor[0];
	    cinfo.comp_info[0].v_samp_factor = scale_factor[1];
        if(color_comp_num >= 2) {
	        cinfo.comp_info[1].h_samp_factor = scale_factor[2];
	        cinfo.comp_info[1].v_samp_factor = scale_factor[3];
        }
        if(color_comp_num >= 3) {
	        cinfo.comp_info[2].h_samp_factor = scale_factor[4];
	        cinfo.comp_info[2].v_samp_factor = scale_factor[5];
        }
        
        if (quality < 0) quality = 0;
        else if(quality > 100) quality = 100;
        jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
        //jpeg_set_quality(&cinfo, quality, FALSE);

        jpeg_start_compress(&cinfo, TRUE);
        
        //jpeg_suppress_tables(&cinfo, TRUE);
	    //jpeg_start_compress(&cinfo, FALSE);
        
        while (cinfo.next_scanline < cinfo.image_height) {
            /* jpeg_write_scanlines expects an array of pointers to scanlines.
             * Here the array is only one element long, but you could pass
             * more than one scanline at a time if that's more convenient.
             */
            row_pointer[0] = &picdata[cinfo.next_scanline * row_stride];
            (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
        }

        jpeg_finish_compress(&cinfo);
        
        jpeg_destroy_compress(&cinfo);

    }

    if(outfile != NULL) {
        fclose(outfile);
    }

    if(picdata != NULL) {
        free(picdata);
    }
#else

    MMP_UnusedParameter(Y);
    MMP_UnusedParameter(U);
    MMP_UnusedParameter(V);
    MMP_UnusedParameter(image_width);
    MMP_UnusedParameter(image_height);
    MMP_UnusedParameter(jpegfilename);
    MMP_UnusedParameter(quality);
    MMP_UnusedParameter(enc_format);
    
#endif
    return MMP_SUCCESS;
}

#if 0 //Gray Scale Jpeg Write
extern JSAMPLE * image_buffer;	/* Points to large array of R,G,B-order data */
int image_height;	/* Number of rows in image */
int image_width;		/* Number of columns in image */
int image_quality;	/* Compression Quality */

FILE *out;

extern void write_JPEG_file (JSAMPLE *scan, FILE *out)
{
  extern int image_width, image_height, image_quality;
  struct jpeg_compress_struct cinfo;
  struct jpeg_error_mgr jerr;
  JSAMPROW row_pointer[1];	/* pointer to JSAMPLE row[s] */
  int row_stride;		/* physical row width in image buffer */

  cinfo.err = jpeg_std_error(&jerr);
  /* Now we can initialize the JPEG compression object. */
  jpeg_create_compress(&cinfo);

  jpeg_stdio_dest(&cinfo, out);
  cinfo.image_width = image_width; 	/* image width and height, in pixels */
  cinfo.image_height = image_height;
  cinfo.input_components = 1;		/* # of color components per pixel */
  cinfo.in_color_space = JCS_GRAYSCALE; 	/* colorspace of input image */
  jpeg_set_defaults(&cinfo);
  jpeg_set_quality(&cinfo, image_quality, TRUE /* limit to baseline-JPEG values */);
  jpeg_start_compress(&cinfo, TRUE);
  row_stride = cinfo.input_components*image_width;	/* JSAMPLEs per row in image_buffer */
  while (cinfo.next_scanline < cinfo.image_height) {
    /* jpeg_write_scanlines expects an array of pointers to scanlines.
     * Here the array is only one element long, but you could pass
     * more than one scanline at a time if that's more convenient.
     */
    row_pointer[0] = & scan[cinfo.next_scanline * row_stride];
    (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
  }
  jpeg_finish_compress(&cinfo);
  jpeg_destroy_compress(&cinfo);
}
#endif 


#if 0 //YCbCr Scale Jpeg Write


00001 //
00002 // Copyright 2003 Sony Corporation 
00003 //
00004 // Permission to use, copy, modify, and redistribute this software for
00005 // non-commercial use is hereby granted.
00006 //
00007 // This software is provided "as is" without warranty of any kind,
00008 // either expressed or implied, including but not limited to the
00009 // implied warranties of fitness for a particular purpose.
00010 //
00011 
00012 #include <stdio.h>
00013 #include <stdlib.h>
00014 #include <jpeglib.h>
00015 #include "write_jpeg.h"
00016 
00017 extern void jpeg_mem_dest(j_compress_ptr cinfo, JOCTET* buf, size_t bufsize);
00018 extern int  jpeg_mem_size(j_compress_ptr cinfo);
00019 
00020 int
00021 write_jpeg_mem(unsigned char* YCbCr,
00022                int w, int h, int quality,
00023                unsigned char* dest, int destsize)
00024 {
00025     JSAMPLE* image_buffer = (JSAMPLE*)YCbCr;
00026     int      image_width  = w;
00027     int      image_height = h;
00028 
00029     struct jpeg_compress_struct cinfo;
00030     struct jpeg_error_mgr jerr;
00031 
00032     JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s] */
00033     int row_stride;             /* physical row width in image buffer */
00034     int jpegsize;
00035 
00036     cinfo.err = jpeg_std_error(&jerr);
00037     jpeg_create_compress(&cinfo);
00038 
00039     jpeg_mem_dest(&cinfo, dest, destsize);
00040 
00041     cinfo.image_width = image_width;
00042     cinfo.image_height = image_height;
00043     cinfo.input_components = 3;         /* # of color components per pixel */
00044     cinfo.in_color_space = JCS_YCbCr; /* colorspace of input image */
00045 
00046     jpeg_set_defaults(&cinfo);
00047 
00048     jpeg_set_quality(&cinfo,
00049                      quality, TRUE /* limit to baseline-JPEG values */);
00050 
00051     jpeg_start_compress(&cinfo, TRUE);
00052 
00053     row_stride = image_width * 3;       /* JSAMPLEs per row in image_buffer */
00054 
00055     while (cinfo.next_scanline < cinfo.image_height) {
00056         row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
00057         (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
00058     }
00059 
00060     jpeg_finish_compress(&cinfo);
00061     jpegsize = jpeg_mem_size(&cinfo);
00062 
00063     jpeg_destroy_compress(&cinfo);
00064 
00065     return jpegsize;
00066 }
00067 
00068 void
00069 write_jpeg_file(unsigned char* YCbCr, int w, int h, int quality, FILE* outfile)
00070 {
00071     JSAMPLE* image_buffer = (JSAMPLE*)YCbCr;
00072     int      image_width  = w;
00073     int      image_height = h;
00074 
00075     struct jpeg_compress_struct cinfo;
00076     struct jpeg_error_mgr jerr;
00077 
00078     JSAMPROW row_pointer[1];    /* pointer to JSAMPLE row[s] */
00079     int row_stride;             /* physical row width in image buffer */
00080 
00081     cinfo.err = jpeg_std_error(&jerr);
00082     jpeg_create_compress(&cinfo);
00083 
00084     jpeg_stdio_dest(&cinfo, outfile);
00085 
00086     cinfo.image_width = image_width;
00087     cinfo.image_height = image_height;
00088     cinfo.input_components = 3;         /* # of color components per pixel */
00089     cinfo.in_color_space = JCS_YCbCr; /* colorspace of input image */
00090 
00091     jpeg_set_defaults(&cinfo);
00092 
00093     jpeg_set_quality(&cinfo,
00094                      quality, TRUE /* limit to baseline-JPEG values */);
00095     
00096     jpeg_start_compress(&cinfo, TRUE);
00097 
00098     row_stride = image_width * 3;       /* JSAMPLEs per row in image_buffer */
00099 
00100     while (cinfo.next_scanline < cinfo.image_height) {
00101         row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
00102         (void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
00103     }
00104 
00105     jpeg_finish_compress(&cinfo);
00106 
00107     jpeg_destroy_compress(&cinfo);
00108 }

#endif


#if 0

int
ImagingJpegEncode(Imaging im, ImagingCodecState state, UINT8* buf, int bytes)
{
    JPEGENCODERSTATE* context = (JPEGENCODERSTATE*) state->context;
    int ok;

    if (setjmp(context->error.setjmp_buffer)) {
	/* JPEG error handler */
	jpeg_destroy_compress(&context->cinfo);
	state->errcode = IMAGING_CODEC_BROKEN;
	return -1;
    }

    if (!state->state) {

	/* Setup compression context (very similar to the decoder) */
	context->cinfo.err = jpeg_std_error(&context->error.pub);
	context->error.pub.error_exit = error;
	jpeg_create_compress(&context->cinfo);
	jpeg_buffer_dest(&context->cinfo, &context->destination);

        context->extra_offset = 0;

	/* Ready to encode */
	state->state = 1;

    }

    /* Load the destination buffer */
    context->destination.pub.next_output_byte = buf;
    context->destination.pub.free_in_buffer = bytes;

    switch (state->state) {

    case 1:

	context->cinfo.image_width = state->xsize;
	context->cinfo.image_height = state->ysize;

	switch (state->bits) {
        case 8:
            context->cinfo.input_components = 1;
            context->cinfo.in_color_space = JCS_GRAYSCALE;
            break;
        case 24:
            context->cinfo.input_components = 3;
            if (strcmp(im->mode, "YCbCr") == 0)
                context->cinfo.in_color_space = JCS_YCbCr;
            else
                context->cinfo.in_color_space = JCS_RGB;
            break;
        case 32:
            context->cinfo.input_components = 4;
            context->cinfo.in_color_space = JCS_CMYK;
            break;
        default:
            state->errcode = IMAGING_CODEC_CONFIG;
            return -1;
	}

	/* Compressor configuration */
	jpeg_set_defaults(&context->cinfo);
	if (context->quality > 0)
	    jpeg_set_quality(&context->cinfo, context->quality, 1);
	
	/* Set subsampling options */
	switch (context->subsampling)
	{
		case 0:  /* 1x1 1x1 1x1 (4:4:4) : None */
		{
			context->cinfo.comp_info[0].h_samp_factor = 1;
			context->cinfo.comp_info[0].v_samp_factor = 1;
			context->cinfo.comp_info[1].h_samp_factor = 1;
			context->cinfo.comp_info[1].v_samp_factor = 1;
			context->cinfo.comp_info[2].h_samp_factor = 1;
			context->cinfo.comp_info[2].v_samp_factor = 1;
			break;
		}
		case 1:  /* 2x1, 1x1, 1x1 (4:2:2) : Medium */
		{
			context->cinfo.comp_info[0].h_samp_factor = 2;
			context->cinfo.comp_info[0].v_samp_factor = 1;
			context->cinfo.comp_info[1].h_samp_factor = 1;
			context->cinfo.comp_info[1].v_samp_factor = 1;
			context->cinfo.comp_info[2].h_samp_factor = 1;
			context->cinfo.comp_info[2].v_samp_factor = 1;
			break;
		}
		case 2:  /* 2x2, 1x1, 1x1 (4:1:1) : High */
		{
			context->cinfo.comp_info[0].h_samp_factor = 2;
			context->cinfo.comp_info[0].v_samp_factor = 2;
			context->cinfo.comp_info[1].h_samp_factor = 1;
			context->cinfo.comp_info[1].v_samp_factor = 1;
			context->cinfo.comp_info[2].h_samp_factor = 1;
			context->cinfo.comp_info[2].v_samp_factor = 1;
			break;
		}
		default:
		{
			/* Use the lib's default */
			break;
		}
	}
	if (context->progressive)
	    jpeg_simple_progression(&context->cinfo);
	context->cinfo.smoothing_factor = context->smooth;
	context->cinfo.optimize_coding = (boolean) context->optimize;
        if (context->xdpi > 0 && context->ydpi > 0) {
            context->cinfo.density_unit = 1; /* dots per inch */
            context->cinfo.X_density = context->xdpi;
            context->cinfo.Y_density = context->ydpi;
        }
	switch (context->streamtype) {
	case 1:
	    /* tables only -- not yet implemented */
	    state->errcode = IMAGING_CODEC_CONFIG;
	    return -1;
	case 2:
	    /* image only */
	    jpeg_suppress_tables(&context->cinfo, TRUE);
	    jpeg_start_compress(&context->cinfo, FALSE);
            /* suppress extra section */
            context->extra_offset = context->extra_size;
	    break;
	default:
	    /* interchange stream */
	    jpeg_start_compress(&context->cinfo, TRUE);
	    break;
	}
	state->state++;
	/* fall through */

    case 2:

        if (context->extra) {
            /* copy extra buffer to output buffer */
            unsigned int n = context->extra_size - context->extra_offset;
            if (n > context->destination.pub.free_in_buffer)
                n = context->destination.pub.free_in_buffer;
            memcpy(context->destination.pub.next_output_byte,
                   context->extra + context->extra_offset, n);
            context->destination.pub.next_output_byte += n;
            context->destination.pub.free_in_buffer -= n;
            context->extra_offset += n;
            if (context->extra_offset >= context->extra_size)
                state->state++;
            else
                break;
        } else
              state->state++;

    case 3:

	ok = 1;
	while (state->y < state->ysize) {
	    state->shuffle(state->buffer,
			   (UINT8*) im->image[state->y + state->yoff] +
			   state->xoff * im->pixelsize, state->xsize);
	    ok = jpeg_write_scanlines(&context->cinfo, &state->buffer, 1);
	    if (ok != 1)
		break;
	    state->y++;
	}

	if (ok != 1)
	    break;
	state->state++;
	/* fall through */

    case 4:

	/* Finish compression */
	if (context->destination.pub.free_in_buffer < 100)
	    break;
	jpeg_finish_compress(&context->cinfo);

	/* Clean up */
        if (context->extra)
            free(context->extra);
	jpeg_destroy_compress(&context->cinfo);
	/* if (jerr.pub.num_warnings) return BROKEN; */
	state->errcode = IMAGING_CODEC_END;
	break;

    }

    /* Return number of bytes in output buffer */
    return context->destination.pub.next_output_byte - buf;

}

#endif