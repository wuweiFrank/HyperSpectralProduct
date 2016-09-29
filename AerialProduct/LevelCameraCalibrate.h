#pragma once
#include "AerialCore.h"
#include <vector>
using namespace std;
//相机定标和检校

//对安置角，安置矢量进行检校
//最后还是没有写出来，具体原因是因为求导过程稍微有点麻烦，没有深入推导了
class SetupCalibProcess
{
public:
	//安置角检校接口
	long SetupCalibProc_SetupAngle(const char* pathPOSFile,float ifov,vector<SetupGCP> m_setup_GCPs,float &fLen,THREEDPOINT setupVec,float *setupAngle);

	//安置矢量检校接口
	long SetupCalibProc_SetupVector(const char* pathPOSFile,float ifov,vector<SetupGCP> m_setup_GCPs,float &fLen,THREEDPOINT &setupVec,float *setupAngle);

	//联合检校接口
	long SetupCalibProc_SetupComb(const char* pathPOSFile,float ifov,vector<SetupGCP> m_setup_GCPs,float &fLen,THREEDPOINT &setupVec,float *setupAngle);

private:
	//获取指定的行的POS数据
	long SetupCalibProc_GetSelectPOS(const char* pathPOSFile,vector<SetupGCP> m_setup_GCPs,vector<POS> &m_selPOS);

	//从指定行的POS数据中解算出EO
	long SetupCalibProc_ExtractSelectEO(vector<POS> m_selPOS,vector<EO> &m_selEO,THREEDPOINT &setupVec,float *setupAngle);
};

//相机内参数检校方法
class CameraInstrinsicCalibProcess
{
public:
	//相机内参检校，如果有足够多的控制点，则直接后方交会检校相机参数
	void CameraInstrinsicCalibProc_Resection(vector<CPOINT> pntImage,vector<THREEDPOINT> pntGcps,float* instrinsicMat);

	//用OpenCV中的定标方法求解相机参数(针对双目相机，不知道一个相机应该如何处理)
	void CameraInstrinsicCalibProc_CV(vector<string> imagePairs);
};