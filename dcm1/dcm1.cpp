// dcm1.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "dcmtk/dcmimage/diargimg.h"
#include "dcmtk/dcmimgle/dcmimage.h"
#include "dcmtk/config/osconfig.h"    /* make sure OS specific configuration is included first */
#include "dcmtk/ofstd/ofstdinc.h"

#include "dcmtk/dcmdata/dctk.h"
#include "dcmtk/dcmdata/cmdlnarg.h"
#include "dcmtk/ofstd/ofconapp.h"
#include "dcmtk/dcmdata/dcuid.h"       /* for dcmtk version name */
#include "dcmtk/dcmjpeg/djdecode.h"    /* for dcmjpeg decoders */
#include "dcmtk/dcmjpeg/dipijpeg.h"    /* for dcmimage JPEG plugin */
#include "dcmtk/dcmjpeg/djencode.h"
#include "dcmtk/dcmjpeg/djrplol.h"

#include "opencv2/opencv.hpp"
#include "opencv2/imgproc/types_c.h"
using namespace cv;
using namespace std;

#include "jpeglib.h"


#include "TickCounter.h"

#include "ZstdHelpers.h"

/*
* dcm2Mat：将.dcm格式图像转换为OpenCV的Mat格式
* fileName：CT图像的名字
* mat: 保存转换后的CT图像
*返回值：成功返回true，失败返回false
*/
bool dcm2Mat(const char* const fileName, cv::Mat & mat)
{
	DJDecoderRegistration::registerCodecs(); // register JPEG codecs  
	DcmFileFormat fileformat;                                        //DcmFileFormat fileformat;

	if (!(fileformat.loadFile(fileName).good()))
		return false;

	DcmDataset *dataset = fileformat.getDataset();
	// decompress data set if compressed    
	dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL);
	DcmElement* element = NULL;
	OFCondition result = dataset->findAndGetElement(DCM_PixelData, element);
	if (result.bad() || element == NULL)
		return false;
	unsigned short* pixData;
	result = element->getUint16Array(pixData);


	Uint16 rows;		
	Uint16 cols;
	dataset->findAndGetUint16(DCM_Rows, rows);
	dataset->findAndGetUint16(DCM_Columns, cols);

	if (rows == 0 || cols == 0)
		return false;

	unsigned short *pbuf = new unsigned short[rows*cols];

	memcpy(pbuf, pixData, rows*cols * 2);
	Mat xxx(rows, cols, CV_16UC1, pbuf);
	Mat xx;
	cv::normalize(xxx, xx, 0, 255, NORM_MINMAX, CV_8UC1);  // 像素值归一化到0~255

	//Mat afull(xx.rows, xx.cols, CV_8UC1, 255);	// 全白图像
	//xx = afull - xx;								// 图像颜色反转，黑变白，白变黑
	mat = xx.clone();								// 为什么不直接用xx？ 因为为了防止内存泄漏

	//cvtColor(mat, mat, CV_GRAY2RGB);				// 灰度转彩色，视具体工程而定，如果只需灰度图，可以不转换

	delete[] pbuf;									// 防止内存泄漏
	DJDecoderRegistration::cleanup(); // deregister JPEG codecs 
	return true;
}

void jpegCompress(unsigned char* imageData, unsigned char* compressedBuffer, int imageWidth, int imageHeight, int channels, unsigned long& outSize)
{
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	unsigned char* inImageBuffer = imageData;
	unsigned char* outbuffer;
	outbuffer = NULL;
	JSAMPROW row_pointer[1];
	int row_stride;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_mem_dest(&cinfo, &outbuffer, &outSize);
	cinfo.image_width = imageWidth;
	cinfo.image_height = imageHeight;
	cinfo.input_components = channels;
	cinfo.in_color_space = JCS_GRAYSCALE;
	cinfo.data_precision = 16;
	jpeg_set_defaults(&cinfo);
	jpeg_start_compress(&cinfo, TRUE);
	row_stride = imageWidth;
	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer[0] = &inImageBuffer[cinfo.next_scanline*row_stride];
		(void)jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}
	jpeg_finish_compress(&cinfo);
	for (int i = 0; i < outSize; i++)
	{
		compressedBuffer[i] = outbuffer[i];
	}
	if (NULL != outbuffer)
	{
		free(outbuffer);
		outbuffer = NULL;
	}
	jpeg_destroy_compress(&cinfo);
	return;
}

void JpegInitSource(j_decompress_ptr cinfo)
{
}
boolean JpegFillInputBuffer(j_decompress_ptr cinfo)
{
	return TRUE;
}
void JpegSkipInputData(j_decompress_ptr cinfo, long num_bytes)
{
}
void JpegTermSource(j_decompress_ptr cinfo)
{
}

bool jpegUnCompress(const char * jpeg_data, int jpeg_size, char *rgb_data, int rgb_size, int w, int h)
{
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	struct jpeg_source_mgr jpegSrcManager;
	int ret;
	JSAMPROW rowPointer[1];
	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_decompress(&cinfo);

	jpegSrcManager.init_source = JpegInitSource;
	jpegSrcManager.fill_input_buffer = JpegFillInputBuffer;
	jpegSrcManager.skip_input_data = JpegSkipInputData;
	jpegSrcManager.resync_to_restart = jpeg_resync_to_restart;
	jpegSrcManager.term_source = JpegTermSource;
	jpegSrcManager.next_input_byte = (unsigned char*)jpeg_data;
	jpegSrcManager.bytes_in_buffer = jpeg_size;
	cinfo.src = &jpegSrcManager;

	jpeg_read_header(&cinfo, TRUE);
	cinfo.out_color_space = JCS_RGB;
	jpeg_start_decompress(&cinfo);
	if (cinfo.output_width != (unsigned int)w && cinfo.output_height != (unsigned int)h)
	{
		jpeg_destroy_decompress(&cinfo);
		return false;
	}
	for (int dy = 0; cinfo.output_scanline < cinfo.output_height; dy++)
	{
		rowPointer[0] = (unsigned char *)(rgb_data + w*dy * 3);
		ret = jpeg_read_scanlines(&cinfo, rowPointer, 1);
	}
	jpeg_finish_decompress(&cinfo);
	jpeg_destroy_decompress(&cinfo);
	return true;
}
void compressDCM(char* fileName)
{
	CTickCounter tc(__FUNCTION__);

	DJEncoderRegistration::registerCodecs(); // register JPEG codecs
	DcmFileFormat fileformat;

	if (fileformat.loadFile(fileName).good())
	{

		DcmDataset *dataset = fileformat.getDataset();
		DcmItem *metaInfo = fileformat.getMetaInfo();
		DJ_RPLossless params; // codec parameters, we use the defaults
							  // this causes the lossless JPEG version of the dataset to be created

		dataset->putAndInsertString(DCM_PhotometricInterpretation, "MONOCHROME2");
		//dataset->removeAllButCurrentRepresentations();
		string txt = dataset->chooseRepresentation(EXS_JPEGProcess14SV1, &params).text(); //EXS_JPEGProcess14 EXS_JPEGProcess14SV1

		if (dataset->chooseRepresentation(EXS_JPEGProcess14SV1, &params).good())
		{
			if (dataset->canWriteXfer(EXS_JPEGProcess14SV1))
			{
				// force the meta-header UIDs to be re-generated when storing the file
				// since the UIDs in the data set may have changed
				delete metaInfo->remove(DCM_MediaStorageSOPClassUID);
				delete metaInfo->remove(DCM_MediaStorageSOPInstanceUID);
				// store in lossless JPEG format

				fileformat.saveFile("test_jpeg.dcm", EXS_JPEGProcess14SV1);
			}
		}
	}

	DJEncoderRegistration::cleanup(); // deregister JPEG codecs
}

void unCompressDCM()
{
	CTickCounter tc(__FUNCTION__);

	DJDecoderRegistration::registerCodecs(); // register JPEG codecs
	DcmFileFormat fileformat;
	if (fileformat.loadFile("test_jpeg.dcm").good())
	{
		DcmDataset *dataset = fileformat.getDataset();
		// decompress data set if compressed
		if (dataset->chooseRepresentation(EXS_LittleEndianExplicit, NULL).good() &&
			dataset->canWriteXfer(EXS_LittleEndianExplicit))
		{
			fileformat.saveFile("test_decompressed.dcm", EXS_LittleEndianExplicit);
		}
	}
	DJDecoderRegistration::cleanup(); // deregister JPEG codecs
}

int main(int argc, TCHAR*argv[])
{
#if 0


	DcmFileFormat dfile;
	dfile.loadFile("E:\\ueg-img\\test\\dcm1\\x64\\Release\\000001.dcm");

	DcmMetaInfo*Metalnfo = dfile.getMetaInfo();

	DcmTag Tag;
	Tag = Metalnfo->getTag();

	Uint16 G_tag = Tag.getGTag();
	cout << "G_tag: " << G_tag << std::endl;

	DcmDataset*data = dfile.getDataset();
	DcmElement*element = NULL;

	data->findAndGetElement(DCM_PixelData, element);


	double element_len = element->getLength();
	cout << "elemetn_len " << element_len << std::endl;;

	OFString patientName;
	data->findAndGetOFString(DCM_PatientName, patientName);
	cout << "patientName: " << patientName.data() << std::endl;;

	OFString patientId;
	data->findAndGetOFString(DCM_PatientID, patientId);
	cout << "patientId: " << patientId << std::endl;;

	OFString patientAge;
	data->findAndGetOFString(DCM_PatientAge, patientAge);
	cout << "patientAge: " << patientAge.data() << std::endl;;

	OFString PixelSpacing;
	data->findAndGetOFString(DCM_PixelSpacing, PixelSpacing);
	cout << "PixelSpacing: " << PixelSpacing.data() << std::endl;;

	Uint16* pixData16;
	element->getUint16Array(pixData16);
	cout << element->getLength() << std::endl;

	Uint16 width;		//获取图像的窗宽高
	Uint16 height;
	data->findAndGetUint16(DCM_Rows, height);
	data->findAndGetUint16(DCM_Columns, width);
	cout << "width :" << width << endl;
	cout << "height " << height << endl;
	cv::Mat img = cv::Mat(height, width, CV_16UC1, pixData16);


	for (int i = 0; i < 100; i++)
	{
		cout << *(pixData16 + i) << " ";
	}

	Uint32 data_len = data->getLength();
	for (int i = 0; i < width * 497; i++)
	{
		;
		//*(pixData16 + i) *= 100; // 灰度拉伸
	}

	Mat img2;
	//dcm2Mat("E:\\ueg-img\\test\\dcm1\\x64\\Release\\000001.dcm", img2);
	//cv::imshow("image", img2);
	//cv::waitKey(0);
#endif
	compressDCM("E:\\ueg-img\\test\\dcm1\\x64\\Release\\000000.dcm");
	unCompressDCM();
	//cv::waitKey(0);

#if 0


	const char* const inFilename = "E:\\ueg-img\\test\\dcm1\\x64\\Release\\000002.dcm";
	char* const outFilename = CZstdHelpers::createOutFilename_orDie(inFilename);
	CZstdHelpers::compress_orDie(inFilename, outFilename);
	free(outFilename);
#endif
	getchar();
	return 0;
}