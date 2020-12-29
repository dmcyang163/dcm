#ifndef _IMGHELPERS_H_
#define _IMGHELPERS_H_





class CImgHelpers
{
public:
	CImgHelpers();
	~CImgHelpers();



	// 读取JPG图片数据，并解压到内存中，*rgb_buffer需要自行释放  
	static int read_jpeg_file(const char* jpeg_file, unsigned char** rgb_buffer, int* size, int* width, int* height);  
	static int write_jpeg_file(const char* jpeg_file, unsigned char* rgb_buffer, int width, int height, int quality);
	
	//jpeg_buf：用于接收jpeg图片的数据，jpeg_len：jpeg图片的大小，bmp_data：传入bmp数据
	static void get_jpeg(unsigned char **jpeg_buf, long unsigned int *jpeg_len, char *bmp_data, int w, int h, int depth);

};



#endif