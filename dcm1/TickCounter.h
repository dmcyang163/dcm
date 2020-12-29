#pragma once
#include <string>
using namespace std;


class CTickCounter
{
public:
	CTickCounter(void);
	CTickCounter(string strInfo);
	virtual ~CTickCounter(void);

private:
	double t_time0,t_time1,t_timeConsumed;
	


	std::string m_strInfo;
};