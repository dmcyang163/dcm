#include "stdafx.h"
#include <iostream>
#include <string.h>
#include "jpeglib.h"

#include "imghelpers.h"

CImgHelpers::CImgHelpers()
{
}
CImgHelpers::~CImgHelpers()
{
}


// 读取JPG图片数据，并解压到内存中，*rgb_buffer需要自行释放
int CImgHelpers::read_jpeg_file(const char* jpeg_file, unsigned char** rgb_buffer, int* size, int* width, int* height)
{
    struct jpeg_decompress_struct cinfo;
    struct jpeg_error_mgr jerr;
    FILE* fp;

    JSAMPARRAY buffer;
    int row_stride = 0;
    unsigned char* tmp_buffer = NULL;
    int rgb_size;

    fp = fopen(jpeg_file, "rb");
    if (fp == NULL)
    {
        printf("open file %s failed.\n", jpeg_file);
        return -1;
    }

    cinfo.err = jpeg_std_error(&jerr);


    jpeg_create_decompress(&cinfo);

    jpeg_stdio_src(&cinfo, fp);

    jpeg_read_header(&cinfo, TRUE);

    //cinfo.out_color_space = JCS_RGB; //JCS_YCbCr;  // 设置输出格式

    jpeg_start_decompress(&cinfo);

    row_stride = cinfo.output_width * cinfo.output_components;
    *width = cinfo.output_width;
    *height = cinfo.output_height;

    rgb_size = row_stride * cinfo.output_height; // 总大小
    *size = rgb_size;

    buffer = (*cinfo.mem->alloc_sarray)((j_common_ptr) &cinfo, JPOOL_IMAGE, row_stride, 1);

    *rgb_buffer = (unsigned char *)malloc(sizeof(char) * rgb_size);    // 分配总内存

    printf("debug--:\nrgb_size: %d, size: %d w: %d h: %d row_stride: %d \n", rgb_size,
           cinfo.image_width * cinfo.image_height * 3,
           cinfo.image_width,
           cinfo.image_height,
           row_stride);
    tmp_buffer = *rgb_buffer;
    while (cinfo.output_scanline < cinfo.output_height) // 解压每一行
    {
        jpeg_read_scanlines(&cinfo, buffer, 1);
        // 复制到内存
        memcpy(tmp_buffer, buffer[0], row_stride);
        tmp_buffer += row_stride;
    }

    jpeg_finish_decompress(&cinfo);
    jpeg_destroy_decompress(&cinfo);

    fclose(fp);

    return 0;
}

int CImgHelpers::write_jpeg_file(const char* jpeg_file, unsigned char* rgb_buffer, int width, int height, int quality)
{
    struct jpeg_compress_struct cinfo;
    struct jpeg_error_mgr jerr;
    int row_stride = 0;
    FILE* fp = NULL;
    JSAMPROW row_pointer[1];

    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_compress(&cinfo);
    fp = fopen(jpeg_file, "wb");
    if (fp == NULL)
    {
        printf("open file %s failed.\n", jpeg_file);
        return -1;
    }
    jpeg_stdio_dest(&cinfo, fp);
    cinfo.image_width = width;
    cinfo.image_height = height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, 1);  // todo 1 == true
    jpeg_start_compress(&cinfo, TRUE);
    row_stride = width * cinfo.input_components;

    while (cinfo.next_scanline < cinfo.image_height)
    {
        row_pointer[0] = &rgb_buffer[cinfo.next_scanline * row_stride];
        jpeg_write_scanlines(&cinfo, row_pointer, 1);
    }

    jpeg_finish_compress(&cinfo);
    jpeg_destroy_compress(&cinfo);
    fclose(fp);

    return 0;
}

//jpeg_buf：用于接收jpeg图片的数据，jpeg_len：jpeg图片的大小，bmp_data：传入bmp数据
/*----------------------将图片bmp压缩成jpg------------------------------*/
void CImgHelpers::get_jpeg(unsigned char **jpeg_buf, long unsigned int *jpeg_len, char *bmp_data, int w, int h, int depth)
{
    // int jpegWidth = ctrl_tmp.width;
    // int jpegHeight = ctrl_tmp.height;

    struct jpeg_compress_struct toWriteCinfo;
    struct jpeg_error_mgr jerr;
    JSAMPROW row_pointer[1];
    int row_stride;

    //开始进行jpg的数据写入
    toWriteCinfo.err = jpeg_std_error(&jerr);
    jpeg_create_compress(&toWriteCinfo);
    //确定要用于输出压缩的jpeg的数据空间
    jpeg_mem_dest(&toWriteCinfo, jpeg_buf, jpeg_len);

    toWriteCinfo.image_width = w;
    toWriteCinfo.image_height = h;
    toWriteCinfo.input_components = 3;
    toWriteCinfo.in_color_space = JCS_RGB;

    jpeg_set_defaults(&toWriteCinfo);
    jpeg_set_quality(&toWriteCinfo, 90, 1);
    jpeg_start_compress(&toWriteCinfo, 1);
    row_stride = toWriteCinfo.image_width * 3;
    while (toWriteCinfo.next_scanline < h)
    {
        row_pointer[0] = (JSAMPROW)(& bmp_data[toWriteCinfo.next_scanline * row_stride]);
        (void)jpeg_write_scanlines(&toWriteCinfo, row_pointer, 1);
    }
    jpeg_finish_compress(&toWriteCinfo);
    jpeg_destroy_compress(&toWriteCinfo);
}

#if 0
int main (int argc, char *argv [])
{
    unsigned char* rgb_buffer = NULL;
    int w, h, size;
    w = h = size = 0;

    read_jpeg_file("4.jpg", &rgb_buffer, &size, &w, &h);
    write_jpeg_file("5.jpg", rgb_buffer, w, h, 90);

    unsigned char* jpeg_buf = NULL;
    long unsigned int jpeg_len = 0;
    get_jpeg(&jpeg_buf, &jpeg_len, (char*)rgb_buffer, w, h, 3);

    FILE* fp = NULL;
    fp = fopen("6.jpg", "wb");
    //size_t fwrite(const void* buffer, size_t size, size_t count, FILE* stream);
    fwrite(jpeg_buf, 1, jpeg_len, fp);
    fclose(fp);
    fp = NULL;

    return 0;
}
#endif