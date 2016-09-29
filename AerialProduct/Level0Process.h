#pragma once

//0级数据产品处理操作,0级数据产品生产输入的是0级数据，得到的是0级产品
//0级数据产品处理主要包括数据格式转换，影像非均匀性校正，EVENT数据解析等操作
class Level0Process
{
public:
	//最终0级产品生产的接口
	virtual long Level0Proc_Interface(const char* pathOrderFile) = 0;

public:
	//0级数据BIL格式转换为BSQ格式
	virtual long Level0Proc_RawToBSQ(const char* pRawBIL,const char* pD0BSQ);

	//根据实验室定标数据和暗电流数据进行非均匀性校正
	virtual long Level0Proc_NonUniform(const char* pD0BSQ,const char* pNonUniform,const char* pCalibFile=NULL,const char* pDarkFile = NULL);

	//解算得到EVENT文件
	virtual long Level0Proc_ExtractEvent(const char* pRawBIL,const char* pEventFile,const int nEventOffset) = 0;

private:
	void GetNonUniformParameters(const char* pCalibFile,const char* pDarkFile,float *params);
};

//对全谱段的0级数据进行处理得到0级数据产品
class QPDLevel0Process : public Level0Process
{
public:
	//最终0级产品生产的接口
	virtual long Level0Proc_Interface(const char* pathOrderFile) {return 0;}

public:
	//从影像中获取EVENT文件信息
	long Level0Proc_ExtractEvent(const char* pRawBIL,const char* pEventFile,const int nEventOffset);

	//修改短波红外Event信息，由于对于全谱段数据短波Event的百微秒是正确的，整数秒存在异常
	//因此需要对全谱段短波数据进行处理
	long Level0Proc_ModifySWIREvent(const char* pathEvent,const char* pathExEvent, float fTime);

	//短波数据是反序的因此在转换为BSQ的时候要顺便反序排列
	//在进行处理的时候给一个inverse变量判断是否需要进行反序运算
	long Level0Proc_RawToBSQ(const char* pathRawBIL, const char* pathBSQ, bool inverse );
};
