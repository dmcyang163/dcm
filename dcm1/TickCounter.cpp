//#include "stdafx.h"
#include "TickCounter.h"
#include <opencv2/opencv.hpp> 

#define __TC__		1

CTickCounter::CTickCounter(void)
{
#if __TC__
	t_time0 = (double)cv::getTickCount();

#endif
}

CTickCounter::CTickCounter(string strInfo)
{
#if __TC__

	m_strInfo = strInfo;
	t_time0 = (double)cv::getTickCount();

#endif
}

CTickCounter::~CTickCounter(void)
{
#if __TC__

	t_time1 = (double)cv::getTickCount();
	t_timeConsumed = (t_time1-t_time0) * 1000. / cv::getTickFrequency();



	printf("%s:\t\t\t%f ms\n", m_strInfo.c_str(), t_timeConsumed);
	//TRACE("%s:\t\t\t%fms\n", m_strInfo.c_str(), t_timeConsumed);

#endif

}