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

#include "MmpDecoderImage_JpegLib.hpp"
#include "MmpUtil.hpp"
#include "mmp_buffer_mgr.hpp"

extern "C" {
#include "jpeglib.h"
}
#include "MmpImageTool.hpp"

/////////////////////////////////////////////////////////////
//CMmpDecoderImage_JpegLib Member Functions

CMmpDecoderImage_JpegLib::CMmpDecoderImage_JpegLib(struct MmpDecoderCreateConfig *pCreateConfig) : CMmpDecoderImage(pCreateConfig, "jpeglib")
,m_p_buf_imageframe_rgb(NULL)
,m_p_buf_imageframe_yuv(NULL)
{
    
}

CMmpDecoderImage_JpegLib::~CMmpDecoderImage_JpegLib()
{

}

MMP_RESULT CMmpDecoderImage_JpegLib::Open()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderImage::Open();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }
    
    return MMP_SUCCESS;
}


MMP_RESULT CMmpDecoderImage_JpegLib::Close()
{
    MMP_RESULT mmpResult;
    
    mmpResult=CMmpDecoderImage::Close();
    if(mmpResult!=MMP_SUCCESS)
    {
        return mmpResult;
    }

    if(m_p_buf_imageframe_rgb != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_rgb);
        m_p_buf_imageframe_rgb = NULL;
    }

    if(m_p_buf_imageframe_yuv != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_yuv);
        m_p_buf_imageframe_yuv = NULL;
    }
        
    //MMPDEBUGMSG(MMPZONE_MONITOR, (TEXT("[CMmpDecoderImage_JpegLib::Close] Success nForamt=(0x%08x %s) \n\r"), 
      //            m_nFormat, m_szCodecName ));

    return MMP_SUCCESS;
}

#include <setjmp.h>
struct my_error_mgr {
  struct jpeg_error_mgr pub;	/* "public" fields */

  jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
  my_error_ptr myerr = (my_error_ptr) cinfo->err;

  /* Always display the message. */
  /* We could postpone this until after returning, if we chose. */
  (*cinfo->err->output_message) (cinfo);

  /* Return control to the setjmp point */
  longjmp(myerr->setjmp_buffer, 1);
}

MMP_RESULT CMmpDecoderImage_JpegLib::DecodeAu(class mmp_buffer_imagestream* p_buf_imagestream, class mmp_buffer_imageframe** pp_buf_imageframe) {

    MMP_RESULT mmpResult = MMP_FAILURE;
    struct jpeg_decompress_struct cinfo;
    struct my_error_mgr jerr;
    /* More stuff */
    FILE * infile;		/* source file */
    JSAMPARRAY buffer;		/* Output row buffer */
    int row_stride;		/* physical row width in output buffer */
    enum MMP_FOURCC fourcc_rgb;
    

    if(pp_buf_imageframe != NULL) {
        *pp_buf_imageframe = NULL;
    }

    /* In this example we want to open the input file before doing anything else,
    * so that the setjmp() error recovery below can assume the file is open.
    * VERY IMPORTANT: use "b" option to fopen() if you are on a machine that
    * requires it in order to read binary files.
    */

#if 0
    if ((infile = fopen(filename, "rb")) == NULL) {
        fprintf(stderr, "can't open %s\n", filename);
        return mmpResult;
    }
#else
    infile = p_buf_imagestream->get_fp_imagefile();
    fseek(infile, 0, SEEK_SET);
#endif

    cinfo.err = jpeg_std_error(&jerr.pub);
    jerr.pub.error_exit = my_error_exit;
    /* Establish the setjmp return context for my_error_exit to use. */
    if (setjmp(jerr.setjmp_buffer)) {
        /* If we get here, the JPEG code has signaled an error.
         * We need to clean up the JPEG object, close the input file, and return.
         */
        jpeg_destroy_decompress(&cinfo);
        return mmpResult;
    }
  
    /* Now we can initialize the JPEG decompression object. */
    jpeg_create_decompress(&cinfo);

    /* Step 2: specify data source (eg, a file) */
    jpeg_stdio_src(&cinfo, infile);

    /* Step 3: read file parameters with jpeg_read_header() */
    (void) jpeg_read_header(&cinfo, TRUE);

    /* We can ignore the return value from jpeg_read_header since
    *   (a) suspension is not possible with the stdio data source, and
    *   (b) we passed TRUE to reject a tables-only JPEG file as an error.
    * See libjpeg.doc for more info.
    */

    /* Step 4: set parameters for decompression */

    /* In this example, we don't need to change any of the defaults set by
    * jpeg_read_header(), so we do nothing here.
    */

    /* Step 5: Start decompressor */
    (void) jpeg_start_decompress(&cinfo);
  
    /* We can ignore the return value since suspension is not possible
    * with the stdio data source.
    */

    /* We may need to do some setup of our own at this point before reading
    * the data.  After jpeg_start_decompress() we have the correct scaled
    * output image dimensions available, as well as the output colormap
    * if we asked for color quantization.
    * In this example, we need to make an output work buffer of the right size.
    */ 
    /* JSAMPLEs per row in output buffer */
    row_stride = cinfo.output_width * cinfo.output_components;
    /* Make a one-row-high sample array that will go away when done with image */
    //buffer = (JSAMPARRAY)malloc(cinfo.output_width*4);
    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    if(m_p_buf_imageframe_yuv != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_yuv);
        m_p_buf_imageframe_yuv = NULL;
    }
    if(m_p_buf_imageframe_rgb != NULL) {
        mmp_buffer_mgr::get_instance()->free_media_buffer(m_p_buf_imageframe_rgb);
        m_p_buf_imageframe_rgb = NULL;
    }
    
    
    m_bih_in.biWidth = cinfo.output_width;
    m_bih_in.biHeight = cinfo.output_height;
    m_bih_out.biWidth = cinfo.output_width;
    m_bih_out.biHeight = cinfo.output_height;
    
    if(cinfo.output_components == 3) {
        fourcc_rgb = MMP_FOURCC_IMAGE_RGB888;
        m_bih_out.biSizeImage = cinfo.output_width*cinfo.output_height*3;
    }
    else if(cinfo.output_components == 4) {
        fourcc_rgb = MMP_FOURCC_IMAGE_ARGB8888;
        m_bih_out.biSizeImage = cinfo.output_width*cinfo.output_height*4;
    }
    else {
        fourcc_rgb = MMP_FOURCC_IMAGE_RGB888;
        m_bih_out.biSizeImage = cinfo.output_width*cinfo.output_height*3;
    }
    m_p_buf_imageframe_rgb = mmp_buffer_mgr::get_instance()->alloc_media_imageframe(m_bih_out.biWidth, m_bih_out.biHeight, fourcc_rgb);
    m_p_buf_imageframe_yuv = mmp_buffer_mgr::get_instance()->alloc_media_imageframe(m_bih_out.biWidth, m_bih_out.biHeight, MMP_FOURCC_IMAGE_YUV420M);
    
    //m_bih_out.biCompression = fourcc_rgb;
    m_bih_out.biCompression = MMP_FOURCC_IMAGE_YUV420M;

    /* Step 6: while (scan lines remain to be read) */
    /*           jpeg_read_scanlines(...); */

    /* Here we use the library's state variable cinfo.output_scanline as the
    * loop counter, so that we don't have to keep track ourselves.
    */
    MMP_U8* p_image;
    p_image = (MMP_U8*)(m_p_buf_imageframe_rgb->get_buf_vir_addr());
    //p_image = (MMP_U8*)(m_p_buf_imageframe->get_buf_vir_addr() + m_p_buf_imageframe->get_buf_stride()*(m_p_buf_imageframe->get_pic_height()-1) );
    while (cinfo.output_scanline < cinfo.output_height) {
        /* jpeg_read_scanlines expects an array of pointers to scanlines.
        * Here the array is only one element long, but you could ask for
        * more than one scanline at a time if that's more convenient.
        */
        (void) jpeg_read_scanlines(&cinfo, buffer, 1);
        /* Assume put_scanline_someplace wants a pointer and sample count. */
//#ifndef WIN32
        //put_scanline_someplace(buffer[0], row_stride);
//#endif
        memcpy(p_image, (void*)(*((unsigned int*)buffer)), row_stride);
        p_image += m_p_buf_imageframe_rgb->get_stride();
        //p_image -= m_p_buf_imageframe->get_buf_stride();
    }

    /* Step 7: Finish decompression */
    (void) jpeg_finish_decompress(&cinfo);

    /* We can ignore the return value since suspension is not possible
    * with the stdio data source.
    */

    /* Step 8: Release JPEG decompression object */

    /* This is an important step since it will release a good deal of memory. */
    jpeg_destroy_decompress(&cinfo);

    if(pp_buf_imageframe != NULL) {

        if(m_bih_out.biCompression == MMP_FOURCC_IMAGE_YUV420M ) {

#if 0
            CMmpImageTool::ConvertRGBtoYUV420_P3((MMP_U8*)m_p_buf_imageframe_rgb->get_buf_vir_addr(),
                                                  m_bih_out.biWidth, m_bih_out.biHeight, fourcc_rgb,
                                                  (MMP_U8*)m_p_buf_imageframe_yuv->get_buf_vir_addr_y(),
                                                  (MMP_U8*)m_p_buf_imageframe_yuv->get_buf_vir_addr_cb(),
                                                  (MMP_U8*)m_p_buf_imageframe_yuv->get_buf_vir_addr_cr(),
                                                  m_p_buf_imageframe_yuv->get_stride(0),
                                                  m_p_buf_imageframe_yuv->get_stride(1),
                                                  m_p_buf_imageframe_yuv->get_stride(2)
                                                  );
#endif

            *pp_buf_imageframe = m_p_buf_imageframe_yuv;
        }
        else {

            *pp_buf_imageframe = m_p_buf_imageframe_rgb;
        }
    }

    return mmpResult;
}




