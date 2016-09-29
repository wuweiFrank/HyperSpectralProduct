#pragma once
#include"AerialCore.h"
#include<vector>
using namespace std;

class QPDLevel2Process;

//这是一个虚基类，所有特定的POS数据需要继承至此类
class GeoPOSProcess
{
public:
	//读取POS数据，从起始行读取指定行数的POS数据
	virtual long GeoPOSProc_ReadPartPOS(const char* pPOSFile,int nLines,int nBeginLine) = 0;

	//从SBET中解析出POS数据（应该用不上）
	virtual long GeoPOSProc_ExtractSBET(const char* pSBETFile,const char* pEVENTFile,const char* pPOSFile,float fOffsetGPS = 17)=0;

	//从文件中读取EO元素
	virtual long GeoPOSProc_ReadEOFile(const char* pEOFile,double &dB,double &dL,double &dH);

	//从POS数据中解算出外方位元素
	virtual long GeoPOSProc_ExtractEO(const char* pPOSFile,const char* pEOFile,int nLines,int nBeginLine,THREEDPOINT THETA,float* fSetupAngle);

	//根据POS数据计算航带所在的象限，EMMatrix为地心坐标系到成图坐标系的转换
	virtual long GeoPOSProc_EOQuadrant(int nLines,double EMMatrix[],THREEDPOINT &XYZPnt);
	virtual long GeoPOSProc_EOQuadrant(POS curPos,double EMMatrix[],THREEDPOINT &XYZPnt);

	//根据POS获取EO元素和旋转矩阵并输出到文件中
	virtual long GeoPOSProc_EOMatrixTurn(int nCurLine,THREEDPOINT &XYZPnt,int nQuadNum,double EMMatrix[],THREEDPOINT ANGLETHETA, THREEDPOINT POSISTTHETA ,FILE* fEO);

	//根据单个POS数据获取EO元素
	virtual long GeoPOSProc_EOMatrixTurn(POS pdfPOS,THREEDPOINT &XYZPnt,THREEDPOINT THETA,float* pVector ,int nQuadNum,double EMMatrix[],EO &pdfEO);

protected:
	//POS和EO数据的定义
	vector<POS> m_Geo_Pos;
	vector<EO>  m_Geo_EO;

	//测区中心经纬度和平均高度
	double      m_Center_Geo_dB;
	double      m_Center_Geo_dL;
	double      m_Center_Geo_dH;
};

//全谱段数据处理
class QPDGeoPOSProcess : public GeoPOSProcess
{
public:
	friend class QPDLevel2Process;

	//获取POS数据
	virtual long GeoPOSProc_ReadPartPOS(const char* pPOSFile,int nLines,int nBeginLine);

	//从SBET文件中解算出POS数据
	virtual long GeoPOSProc_ExtractSBET(const char* pathSBET, const char* pathEvent, const char* pathPOS, float fOffsetGPS = 17);

	//获取EO元素的行数
	virtual long GeoPOSProc_GetEOLines(const char *pEoFile, int &nEOLines);
};

class UAVGeoPOSProcess : public GeoPOSProcess
{
public:
	friend class LevelUAVProcess;

public:
	//读取POS数据，从起始行读取指定行数的POS数据
	virtual long GeoPOSProc_ReadPartPOS(const char* pPOSFile,int nLines,int nBeginLine);

	//从SBET中解析出POS数据（应该用不上）
	virtual long GeoPOSProc_ExtractSBET(const char* pSBETFile,const char* pEVENTFile,const char* pPOSFile,float fOffsetGPS = 17){return 0;}
public:
	//设置读取POS数据格式
	//经度可以为dL/longitude：1 纬度为dB/latitude：2 高度为dH/height：3 角元素为 roll-pitch-yaw，：4，5，6其他所有符号都为填充位 0
	long GeoPOSProc_SetPOSFormat(char* formatStr,bool arc);
	
	//由单个POS数据解算得到EO元素
	long GeoPOSProc_ExtractEO(POS m_perpos, EO &m_pereo);

private:
	//POS数据格式
	vector<int> formatVec;
	bool        m_arc;
};