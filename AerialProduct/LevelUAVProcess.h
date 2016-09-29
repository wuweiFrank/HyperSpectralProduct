#pragma once
#include "Level2Process.h"
#include "opencv/include/opencv2/core/core.hpp"
#include "opencv/include/opencv2/imgproc.hpp"
#include "opencv/include/opencv2/highgui/highgui.hpp"
#include "opencv/include/opencv2/calib3d.hpp"
#include "opencv/include/opencv2/stitching/detail/camera.hpp"
#include "opencv/include/opencv2/stitching/detail/matchers.hpp"

class LevelUAVProcess : public Level2Process
{
//根据POS数据对无人机影像进行几何校正
public:
	//最终2级数据生产的接口
	virtual long Level2Proc_Interface(const char* pathOrderFile){return 0;}
	//接口
	long Level2Proc_Product2A(const char *pFile, const char *pCFile, const char *pEOFile, float fGSDX, float fGSDY, double fElevation, int nEOOffset,
		float fFov, float fIFov, float fFocalLen, bool bIGM, const char *pIGMFile, bool bInverse = false){return 0;}
	//接口
	//与2A级数据产品生产函数相比2B级数据产品生产函数主要增加的接口为DEM输入路径，若输入路径为NULL，则二者相同，如果存在DEM则利用DEM进行解算
	long Level2Proc_Product2B(const char *pFile, const char *pCFile, const char *pEOFile, float fGSDX, float fGSDY, double fElevation, int nEOOffset,
		float fFov, float fIFov, float fFocalLen, const char* pDEMFile, bool bIGM, const char *pIGMFile, bool bInverse = false){return 0;}
//////////////////////////////////////////////////////////////////////////
	//1.对影像集进行校正
	long UAVGeoCorrection_GeoCorrect(const char* pathSrcDir, const char* pathDstDir, const char* pathPOS, const char* pathDEM,int IMGbegline, int POSbegline, int lines,double fLen , double fGSD, double AvgHeight);

	//2.对单个影像进行校正
	long UAVGeoCorrection_GeoCorrect(const char* pathSrc, const char* pathDst, EO  m_preeo, double dL, double dB, double fLen, double fGSD, double AvgHeight, char* pathDEM = NULL);

	//3.计算每个点的地理坐标
	long UAVGeoCorrection_GeoPntsProb(double dB, double dL, double dH, EO pEO, double fLen, int width, int height, THREEDPOINT *pGoundPnt, DPOINT* pMapImgPnt, int pntNum);

	//4.精确计算每个点的坐标然后进行校正
	long UAVGeoCorrection_GeoPntsAccu(double dB, double dL, double dH, EO pEO, double fLen, int width, int height, THREEDPOINT *pGoundPnt,const char* pathDEM);

	//数据测试
	long UAVGeoCorrect_Test();

public:
	double m_ccdSize;	/*像素转换为m的坐标*/
	double m_x0;		//像主点坐标
	double m_y0;
	char   m_posFormat[128];
};

//空中三角测量基类
class LevelAerialTriangle
{
public:
	//初始化
	LevelAerialTriangle(){
		m_mat[0]=1;m_mat[1]=0;
		m_mat[2]=0;m_mat[3]=1;
		m_x0=m_y0=0;
		m_flen=153033;	//这个焦距是测试数据的焦距
		m_pixel_size=1;
	}

	//影像内定向
	virtual long LevelATProc_InnerOrientation(vector<Pointf> &pnt,vector<Pointf> &pntInner);

	//从两张影像中获取影像匹配点
	//显卡加速需要显存比较大的显卡，否则显存不够没法达到需要的效果
	//两张影像的匹配实际上只是测试函数而已，因为实际匹配不可能是这样，
	//实际匹配得到的应该是影像匹配点的idx和特征点，然后再消除所有没有用的点
	virtual long LevelATProc_GetMatchPntsTest(const char* path1,const char* path2);
	//解析影像匹配点
	virtual long LevelATProc_MatchExtract(vector<string> imageList,vector<MatchPair> &match_pairs,vector<vector<Pointf>> &featureList);

	//影像匹配点
	virtual long LevelATProc_MatchFeature(vector<string> imageList,vector<cv::detail::ImageFeatures> &fetures,vector<cv::detail::MatchesInfo> &match_pairs);

	//这两个功能是因为显示的需要，所以从ENVI导入导出方便查看
	//从ENVI中读取影像匹配点
	virtual long LevelATProc_ReadENVIPts(const char* pathENVI);
	//将点导出到ENVI中
	virtual long LevelATProc_SaveAsENVI(const char* pathENVI);

protected:
	vector<Pointf> m_matchPoint;//相邻两个点为一个匹配点对
	int m_mat[4];				//将以像素为单位的影像转换为以mm为单位
	int m_x0,m_y0,m_flen;
	float m_pixel_size;
};

//航带法空中三角测量（包括摄影测量的一系列处理）
//只考虑一条航带进行相对和绝对定向(影像顺序为从左到右的顺序)
class LevelFlightLineAT: public LevelAerialTriangle
{
public:
	//相对定向，输入影像匹配点，输出为相对定向元素
	virtual long LevelATProc_RelativeOrientation(vector<Pointf> pnt1,REO &relOrientation1,vector<Pointf> pnt2,REO &relOrientation2);

	//获取影像模型点坐标
	virtual long LevelATProc_ModelPoint(vector<Pointf> pnt1,REO &relOrientation1,vector<Pointf> pnt2,REO &relOrientation2,vector<Point3f> &modelPoint);
	
	//模型点坐标归一化并计算模型点的摄测坐标
	//由于是单航带模型，所以影像匹配一定是从左到右两两匹配
	virtual long LevelATProc_ModelPointNorm(vector<vector<Pointf>> &imgPntList,vector<vector<Point3f>> &modelPointList,vector<Point3f> &phoCoordi,vector<REO> &relOrientationList);


	//绝对定向，输入影像匹配点，输出绝对定向元素
	virtual long LevelATProc_AbsluteOrientation(vector<SetupGCP> pntGcps,EO &absEO){return 0;}

public:
	void LevelATProc_RelativeOrientationTest();
	
};

//采用计算机视觉的方法进行图像变换矩阵的估计，在这里是估计影像的单应矩阵，然后通过单应矩阵估计相机参数，进行光束法平差
//
class LevelFlightLineComputerVision : public LevelAerialTriangle
{
public:
	//计算影像之间的单应矩阵
	virtual long LevelATProc_HomographyMatrix(vector<vector<Pointf>> &imgPntList,vector<cv::Mat> &homoMatrixList);

	//光束法调整单应矩阵
	virtual long LevelATProc_HomoBundler(vector<vector<Pointf>> &imgPntList,vector<cv::Size> &imgSizeList,vector<cv::Mat> &homoMatrixList,vector<cv::detail::CameraParams> &vec_Cameras);
	virtual long LevelATProc_HomoBundler(vector<cv::detail::ImageFeatures> &fetures,vector<cv::detail::MatchesInfo> &match_pairs,vector<cv::detail::CameraParams> &vec_Cameras);

private:
	//根据单应矩阵估计影像的焦距
	virtual long LevelATProc_HomographyFocal(vector<vector<Pointf>> &imgPntList,vector<cv::Size> &imgSizeList,vector<cv::Mat> &homoMatrixList,vector<double> &focals);

public:
	void LevelATProc_HomographTest();

};