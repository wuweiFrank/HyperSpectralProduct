#pragma once

#include "AerialCore.h"
#include<vector>
using namespace std;

//数据预处理生成的是0级数据
//数据预处理的操作主要是对原始数据进行检测，检测原始数据是否存在漏行并进行漏行修复
class PreProcess
{
public:
	//最终数据预处理的接口
	virtual long PreProc_Interface(const char* pathOrderFile) = 0;

public:
	//获取数据头信息
	virtual long PreProc_GetHeadInfo(FILE *fRAW,DINFO &mDataHeader,vector<short>& nLeakFrameType,vector<int> &nLeadFrameSize,int &nLeakFrameCount) = 0;

	//进行漏行检测与修复
	//关于漏行检测原理的说明：由于影像的存储格式是BIL，在检测过程中有一个标识为标识行号，利用这个标识位以及数据大小可以获取漏行的信息然后进行差值修复
	//nLeakFrameType 0为无漏行；1为漏整数帧；2为漏行；3为漏行且漏帧头文件
	virtual long PreProc_LeakLineCheck(const char *pRAWData, DINFO &mDataHeader, vector<short> &nLeakFrameType, vector<int> &nLeakFrameSize, int &nLeakFrameCount) = 0;

	virtual long PreProc_LeakLineInterpolate(FILE *fRAW, unsigned short *pRepairBuffer, DINFO mDataHeader, vector<short> nLeakFrameType,vector<int> nLeakFrameSize, 
									   int nLoc, __int64 nOffset, unsigned short *pfBuffer, unsigned short *plBuffer, unsigned short *pLeakBuffer)=0;

	//生成0级数据，0级数据指漏行修复后的数据
	virtual long PreProc_GenerateD0Data(const char *pRAWData, const  char *pData, DINFO &mDataHeader, vector<short> &nLeakFrameType, vector<int> &nLeakFrameSize, int &nLeakFrameCount, const int nFixLines)=0;
};


//全谱段数据预处理
class QPDPreProcess: public PreProcess
{
public:
	//最终数据预处理的接口
	virtual long PreProc_Interface(const char* pathOrderFile){return 0;}

public:
	long PreProc_GetHeadInfo(FILE *fRAW, DINFO &mDataHeader, vector<short> &nLeakFrameType, vector<int> &nLeakFrameSize, int &nLeakFrameCount);

	long PreProc_LeakLineCheck(const char *pRAWData, DINFO &mDataHeader, vector<short> &nLeakFrameType, vector<int> &nLeakFrameSize, int &nLeakFrameCount);

	long PreProc_LeakLineInterpolate(FILE *fRAW, unsigned short *pRepairBuffer, DINFO mDataHeader, vector<short> nLeakFrameType, vector<int> nLeakFrameSize,
								int nLoc, __int64 nOffset, unsigned short *pfBuffer, unsigned short *plBuffer, unsigned short *pLeakBuffer);

	long PreProc_GenerateD0Data(const char *pRAWData, const  char *pData, DINFO &mDataHeader, vector<short> &nLeakFrameType, vector<int> &nLeakFrameSize, int &nLeakFrameCount, const int nFixLines);

	//将漏行信息写入文件中
	long PrePRoc_WriteLeakInfo(char *pLeakFile, vector<short> nLeakFrameType, vector<int> nLeakFrameSize, int nLeakFrameCount);

	//从文件中读取漏行信息
	long PreProc_ReadLeakFile(char *pLeakFile, vector<short> &nLeakFrameType, vector<int> &nLeakFrameSize, int &nLeakFrameCount);
};
