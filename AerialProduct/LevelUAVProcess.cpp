#include "StdAfx.h"
#include "LevelUAVProcess.h"
#include "AuxiliaryFunction.h"
#include "PosProc.h"
#include "GDALTools.h"
#include "GDAL/include/gdal_priv.h"
#include "GDAL/include/ogr_spatialref.h"
#include "tsmUTM.h"
#include "matrixOperation.h"
#include <fstream>
#include <numeric>
#include "siftGPU/SiftGPU.h"
#include "AerialProductCvLibImport.h"

#include "MosaicProcess.h"
#include "opencv/include/opencv2/stitching/detail/autocalib.hpp"
#include "opencv/include/opencv2/stitching/detail/motion_estimators.hpp"

using namespace std;
#pragma comment(lib,"gdal_i.lib")

//不写在头文件中因为不想暴露头文件
static void CalcRotation(vector<cv::detail::CameraParams> &cameras,vector<cv::Mat> &homoList)
{
	//因为是顺序的所以都是前一个到后一个
	for(int i=0;i<cameras.size()-1;++i)
	{
		cv::Mat_<double> K_from = cv::Mat::eye(3, 3, CV_64F);
		K_from(0,0) = cameras[i].focal;
		K_from(1,1) = cameras[i].focal * cameras[i].aspect;
		K_from(0,2) = cameras[i].ppx;
		K_from(1,2) = cameras[i].ppy;

		cv::Mat_<double> K_to = cv::Mat::eye(3, 3, CV_64F);
		K_to(0,0) = cameras[i+1].focal;
		K_to(1,1) = cameras[i+1].focal * cameras[i+1].aspect;
		K_to(0,2) = cameras[i+1].ppx;
		K_to(1,2) = cameras[i+1].ppy;

		cv::Mat R = K_from.inv() * homoList[i+1].inv() * K_to;
		cameras[i+1].R = cameras[i].R * R;
	}
}
static void CalcError(cv::Mat &err, cv::Mat cam_params_,vector<vector<Pointf>> &imgPntList,int total_num_matches_)
{
	err.create(total_num_matches_ * 2, 1, CV_64F);

	int match_idx = 0;
	for (size_t edge_idx = 0; edge_idx < imgPntList.size(); ++edge_idx)
	{
		int i = edge_idx;
		int j = edge_idx+1;
		double f1 = cam_params_.at<double>(i * 7, 0);
		double f2 = cam_params_.at<double>(j * 7, 0);
		double ppx1 = cam_params_.at<double>(i * 7 + 1, 0);
		double ppx2 = cam_params_.at<double>(j * 7 + 1, 0);
		double ppy1 = cam_params_.at<double>(i * 7 + 2, 0);
		double ppy2 = cam_params_.at<double>(j * 7 + 2, 0);
		double a1 = cam_params_.at<double>(i * 7 + 3, 0);
		double a2 = cam_params_.at<double>(j * 7 + 3, 0);

		double R1[9];
		cv::Mat R1_(3, 3, CV_64F, R1);
		cv::Mat rvec(3, 1, CV_64F);
		rvec.at<double>(0, 0) = cam_params_.at<double>(i * 7 + 4, 0);
		rvec.at<double>(1, 0) = cam_params_.at<double>(i * 7 + 5, 0);
		rvec.at<double>(2, 0) = cam_params_.at<double>(i * 7 + 6, 0);
		cv::Rodrigues(rvec, R1_);

		double R2[9];
		cv::Mat R2_(3, 3, CV_64F, R2);
		rvec.at<double>(0, 0) = cam_params_.at<double>(j * 7 + 4, 0);
		rvec.at<double>(1, 0) = cam_params_.at<double>(j * 7 + 5, 0);
		rvec.at<double>(2, 0) = cam_params_.at<double>(j * 7 + 6, 0);
		cv::Rodrigues(rvec, R2_);

		cv::Mat_<double> K1 = cv::Mat::eye(3, 3, CV_64F);
		K1(0,0) = f1; K1(0,2) = ppx1;
		K1(1,1) = f1*a1; K1(1,2) = ppy1;

		cv::Mat_<double> K2 = cv::Mat::eye(3, 3, CV_64F);
		K2(0,0) = f2; K2(0,2) = ppx2;
		K2(1,1) = f2*a2; K2(1,2) = ppy2;

		cv::Mat_<double> H = K2 * R2_.inv() * R1_ * K1.inv();

		for (size_t k = 0; k < imgPntList[edge_idx].size()/2; ++k)
		{
			cv::Point2f p1(imgPntList[edge_idx][2*k+0].x,imgPntList[edge_idx][2*k+0].y);
			cv::Point2f p2(imgPntList[edge_idx][2*k+1].x,imgPntList[edge_idx][2*k+1].y);

			double x = H(0,0)*p1.x + H(0,1)*p1.y + H(0,2);
			double y = H(1,0)*p1.x + H(1,1)*p1.y + H(1,2);
			double z = H(2,0)*p1.x + H(2,1)*p1.y + H(2,2);

			err.at<double>(2 * match_idx, 0)	 = p2.x - x/z;
			err.at<double>(2 * match_idx + 1, 0) = p2.y - y/z;
			match_idx++;
		}
	}
}
static void CalcDeriv(const cv::Mat &err1, const cv::Mat &err2, double h, cv::Mat res)
{
	for (int i = 0; i < err1.rows; ++i)
		res.at<double>(i, 0) = (err2.at<double>(i, 0) - err1.at<double>(i, 0)) / h;
}
static void CalcJacobian(cv::Mat &jac,cv::Mat &err1_,cv::Mat &err2_,cv::Mat cam_params_,vector<vector<Pointf>> &imgPntList,int total_num_matches_)
{
	int num_images_ = imgPntList.size()+1;
	jac.create(total_num_matches_ * 2, num_images_ * 7, CV_64F);
	jac.setTo(0);

	double val;
	const double step = 1e-4;

	for (int i = 0; i < num_images_; ++i)
	{
		val = cam_params_.at<double>(i * 7, 0);
		cam_params_.at<double>(i * 7, 0) = val - step;
		CalcError(err1_,cam_params_,imgPntList,total_num_matches_);
		cam_params_.at<double>(i * 7, 0) = val + step;
		CalcError(err2_,cam_params_,imgPntList,total_num_matches_);
		CalcDeriv(err1_, err2_, 2 * step, jac.col(i * 7));
		cam_params_.at<double>(i * 7, 0) = val;

		val = cam_params_.at<double>(i * 7 + 1, 0);
		cam_params_.at<double>(i * 7 + 1, 0) = val - step;
		CalcError(err1_,cam_params_,imgPntList,total_num_matches_);
		cam_params_.at<double>(i * 7 + 1, 0) = val + step;
		CalcError(err2_,cam_params_,imgPntList,total_num_matches_);
		CalcDeriv(err1_, err2_, 2 * step, jac.col(i * 7 + 1));
		cam_params_.at<double>(i * 7 + 1, 0) = val;

		val = cam_params_.at<double>(i * 7 + 2, 0);
		cam_params_.at<double>(i * 7 + 2, 0) = val - step;
		CalcError(err1_,cam_params_,imgPntList,total_num_matches_);
		cam_params_.at<double>(i * 7 + 2, 0) = val + step;
		CalcError(err2_,cam_params_,imgPntList,total_num_matches_);
		CalcDeriv(err1_, err2_, 2 * step, jac.col(i * 7 + 2));
		cam_params_.at<double>(i * 7 + 2, 0) = val;

		val = cam_params_.at<double>(i * 7 + 3, 0);
		cam_params_.at<double>(i * 7 + 3, 0) = val - step;
		CalcError(err1_,cam_params_,imgPntList,total_num_matches_);
		cam_params_.at<double>(i * 7 + 3, 0) = val + step;
		CalcError(err2_,cam_params_,imgPntList,total_num_matches_);
		CalcDeriv(err1_, err2_, 2 * step, jac.col(i * 7 + 3));
		cam_params_.at<double>(i * 7 + 3, 0) = val;

		for (int j = 4; j < 7; ++j)
		{
			val = cam_params_.at<double>(i * 7 + j, 0);
			cam_params_.at<double>(i * 7 + j, 0) = val - step;
			CalcError(err1_,cam_params_,imgPntList,total_num_matches_);
			cam_params_.at<double>(i * 7 + j, 0) = val + step;
			CalcError(err2_,cam_params_,imgPntList,total_num_matches_);
			CalcDeriv(err1_, err2_, 2 * step, jac.col(i * 7 + j));
			cam_params_.at<double>(i * 7 + j, 0) = val;
		}
	}
}

static void CalcRefineCamera(vector<cv::detail::CameraParams> &cameras,cv::Mat &cam_params_)
{
	for (int i = 0; i < cameras.size(); ++i)
	{
		cameras[i].focal = cam_params_.at<double>(i * 7, 0);
		cameras[i].ppx = cam_params_.at<double>(i * 7 + 1, 0);
		cameras[i].ppy = cam_params_.at<double>(i * 7 + 2, 0);
		cameras[i].aspect = cam_params_.at<double>(i * 7 + 3, 0);

		cv::Mat rvec(3, 1, CV_64F);
		rvec.at<double>(0, 0) = cam_params_.at<double>(i * 7 + 4, 0);
		rvec.at<double>(1, 0) = cam_params_.at<double>(i * 7 + 5, 0);
		rvec.at<double>(2, 0) = cam_params_.at<double>(i * 7 + 6, 0);
		Rodrigues(rvec, cameras[i].R);

		cv::Mat tmp;
		cameras[i].R.convertTo(tmp, CV_32F);
		cameras[i].R = tmp;
	}
}
static void CalcFocalsFromHomography(const cv::Mat& H, double &f0, double &f1, bool &f0_ok, bool &f1_ok)
{
	CV_Assert(H.type() == CV_64F && H.size() == cv::Size(3, 3));

	const double* h = H.ptr<double>();

	double d1, d2; // Denominators
	double v1, v2; // Focal squares value candidates

	f1_ok = true;
	d1 = h[6] * h[7];
	d2 = (h[7] - h[6]) * (h[7] + h[6]);
	v1 = -(h[0] * h[1] + h[3] * h[4]) / d1;
	v2 = (h[0] * h[0] + h[3] * h[3] - h[1] * h[1] - h[4] * h[4]) / d2;
	if (v1 < v2) std::swap(v1, v2);
	if (v1 > 0 && v2 > 0) f1 = std::sqrt(std::abs(d1) > std::abs(d2) ? v1 : v2);
	else if (v1 > 0) f1 = std::sqrt(v1);
	else f1_ok = false;

	f0_ok = true;
	d1 = h[0] * h[3] + h[1] * h[4];
	d2 = h[0] * h[0] + h[1] * h[1] - h[3] * h[3] - h[4] * h[4];
	v1 = -h[2] * h[5] / d1;
	v2 = (h[5] * h[5] - h[2] * h[2]) / d2;
	if (v1 < v2) std::swap(v1, v2);
	if (v1 > 0 && v2 > 0) f0 = std::sqrt(std::abs(d1) > std::abs(d2) ? v1 : v2);
	else if (v1 > 0) f0 = std::sqrt(v1);
	else f0_ok = false;
}


//===================================================================================================================================================
long LevelUAVProcess::UAVGeoCorrection_GeoCorrect(const char* pathSrcDir, const char* pathDstDir, const char* pathPOS, const char* pathDEM,
														int IMGbegline, int POSbegline, int lines, double fLen, double fGSD, double AvgHeight)
{
	UAVGeoPOSProcess m_POS_Proc;
	THREEDPOINT theta;
	theta.dX = theta.dY = theta.dZ = 0;
	float setupAngle[] = { 0,0,0 };
	long lError = 0;
	if (lError != 0)
		return lError;
	m_POS_Proc.GeoPOSProc_SetPOSFormat(m_posFormat,false);
	m_POS_Proc.GeoPOSProc_ReadPartPOS(pathPOS, lines, POSbegline);

	for (int i = IMGbegline; i<lines + IMGbegline; i++)
	{
		char pathSrcImg[256], pathDstImg[256], cA[20], cB[20];
		EO tmpEO;
		double tempdL = m_POS_Proc.m_Geo_Pos[i - IMGbegline].m_longitude;
		double tempdB = m_POS_Proc.m_Geo_Pos[i - IMGbegline].m_latitude;
		m_POS_Proc.GeoPOSProc_ExtractEO(m_POS_Proc.m_Geo_Pos[i - IMGbegline], tmpEO);
		//指定格式
		sprintf_s(cA, "\\DSC%005d.JPG", i);
		sprintf_s(cB, "\\DSC%005d.tif", i);

		strcpy_s(pathSrcImg, pathSrcDir);
		strcat_s(pathSrcImg, cA);
		strcpy_s(pathDstImg, pathDstDir);
		strcat_s(pathDstImg, cB);
		lError = UAVGeoCorrection_GeoCorrect(pathSrcImg, pathDstImg, /*m_POS_Proc.m_geo_EO[i - IMGbegline]*/tmpEO, tempdL, tempdB, fLen, fGSD, AvgHeight);
		if (lError != 0)
		{
			printf("%s image correct error\n", pathSrcImg);
			continue;
		}
	}
	return lError;
}

long LevelUAVProcess::UAVGeoCorrection_GeoCorrect(const char* pathSrc, const char* pathDst, EO  m_preeo, double dL, double dB, double fLen, double fGSD, double AvgHeight, char* pathDEM)
{
	UAVGeoPOSProcess m_GeoPOS_Proc;
	int 	nBandCount, ImageWidth, ImageHeight;
	GDALDataset    *poDataset;
	long    lError = 0;
	double  dLatitude = 0;
	double  dLongitude = 0;
	double  dFlightHeight = 0;

	DPOINT  *pPositions = NULL;
	THREEDPOINT* pGoundPnt;

	double  *pImgBuffer = NULL;
	double  *pRegBuffer = NULL;

	int     nEOCounts = 0;
	double  centereast, centernorth;
	int     centerzone;
	OGRSpatialReference  oSRS;
	char *pszSRS_WKT = NULL;

	//判断文件是否存在
	GDALAllRegister();//GDAL数据集
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	poDataset = (GDALDataset *)GDALOpen(pathSrc, GA_ReadOnly);
	if (poDataset == NULL)
		return -2;

	nBandCount = poDataset->GetRasterCount();
	ImageWidth = poDataset->GetRasterXSize();
	ImageHeight = poDataset->GetRasterYSize();

	if (lError != 0)
		return lError;
	dLongitude = dL;
	dLatitude = dB;
	if (dLongitude*180.0 / PI < -180.0 || dLongitude*180.0 / PI > 180.0)
		return -1;
	if (dLatitude*180.0 / PI <= -90.0 || dLatitude*180.0 / PI >= 90.0)
		return -1;

	//计算中心经纬度和投影带
	lError = tsmLatLongToUTM(dLatitude*180.0 / PI, dLongitude*180.0 / PI, &centerzone, &centereast, &centernorth);
	if (lError == 0)
		return -1;

	//均匀选取5个地面点和影像点计算坐标
	bool probOrNot = true;
	if (pathDEM != NULL)
		probOrNot = false;

	if (probOrNot)
	{
		pGoundPnt = new THREEDPOINT[5];
		DPOINT pModelPnts[5];
		for (int i = 0; i<5; i++)
			pGoundPnt[i].dZ = AvgHeight;
		DPOINT pImagePnts[5];
		//计算对应的地面点的坐标
		lError = UAVGeoCorrection_GeoPntsProb(dLatitude, dLongitude, AvgHeight, m_preeo, fLen,ImageWidth,ImageHeight, pGoundPnt, pModelPnts, 5);

		pImagePnts[0].dX = ImageWidth / 2; pImagePnts[0].dY = ImageHeight / 2;
		pImagePnts[1].dX = 0; pImagePnts[1].dY = 0;
		pImagePnts[2].dX = 0; pImagePnts[2].dY = ImageHeight;
		pImagePnts[3].dX = ImageWidth; pImagePnts[3].dY = 0;
		pImagePnts[4].dX = ImageWidth; pImagePnts[4].dY = ImageHeight;
		double* pdfGCPs = new double[5 * 4];
		for (int i = 0; i<5; i++)
		{
			pdfGCPs[4 * i + 0] = pImagePnts[i].dX;
			pdfGCPs[4 * i + 1] = pImagePnts[i].dY;
			pdfGCPs[4 * i + 2] = pGoundPnt[i].dX;
			pdfGCPs[4 * i + 3] = pGoundPnt[i].dY;
		}
		oSRS.SetUTM(centerzone, TRUE);
		oSRS.SetWellKnownGeogCS("WGS84");
		oSRS.exportToWkt(&pszSRS_WKT);
		double dGroundSize[2] = { fGSD,-fGSD };
		GeoGCPProcess m_GcpProc;
		//lError=m_GcpProc.GeoProc_GCPWarpTPS(pathSrc,pathDst,pszSRS_WKT,dGroundSize,GRA_Bilinear,pdfGCPs,64);
		lError = m_GcpProc.GeoProc_GCPWarpOrder(pathSrc, pathDst, 1, TRUE, pszSRS_WKT, dGroundSize, pdfGCPs, 5);
		if (lError != 0)
			return lError;
		CPLFree(pszSRS_WKT);
		delete[]pGoundPnt;
		delete[]pdfGCPs;
		GDALClose(poDataset);
	}
	else
	{
		unsigned char* imgBuffer = NULL;
		unsigned char* dstBuffer = NULL;
		try
		{
			pGoundPnt = new THREEDPOINT[ImageWidth*ImageHeight];
		}
		catch (bad_alloc &e)
		{
			printf("%s", e.what());
			exit(-1);
		}
		//char* pathDEM = "";//定义的地面高程信息，如果知道直接在代码中修改
		lError = UAVGeoCorrection_GeoPntsAccu(dLatitude, dLongitude, AvgHeight, m_preeo, fLen, ImageWidth, ImageHeight, pGoundPnt, pathDEM);
		//初始化最大最小点
		DPOINT minPt, maxPt;
		minPt.dX = MAX_NUM; minPt.dY = MAX_NUM;
		maxPt.dX = MIN_NUM; maxPt.dY = MIN_NUM;
		for (int j = 0; j<ImageWidth*ImageHeight; j++)
		{
			//寻找最大最小点
			if (minPt.dX>pGoundPnt[j].dX)
				minPt.dX = pGoundPnt[j].dX;
			if (minPt.dY>pGoundPnt[j].dY)
				minPt.dY = pGoundPnt[j].dY;
			if (maxPt.dX<pGoundPnt[j].dX)
				maxPt.dX = pGoundPnt[j].dX;
			if (maxPt.dY<pGoundPnt[j].dY)
				maxPt.dY = pGoundPnt[j].dY;
		}
		int dstWidth  = (maxPt.dX - minPt.dX) / fGSD;
		int dstHeight = (maxPt.dX - minPt.dX) / fGSD;
		try
		{
			imgBuffer = new unsigned char[ImageWidth*ImageHeight];
			dstBuffer = new unsigned char[dstWidth*dstHeight];
		}
		catch (bad_alloc &e)
		{
			printf("%s", e.what());
			exit(-1);
		}
		char **papszOptions = NULL;
		papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");
		GDALDatasetH dstDataset = GDALCreate(GDALGetDriverByName("GTiff"), pathDst, dstWidth, dstHeight, nBandCount, GDT_Byte, papszOptions);
		for (size_t i = 0; i < nBandCount; i++)
		{
			poDataset->GetRasterBand(i + 1)->RasterIO(GF_Read, 0, 0, ImageWidth, ImageHeight, imgBuffer, ImageWidth, ImageHeight, GDT_Byte, 0, 0);
			GetImgSample(imgBuffer, minPt, maxPt, pGoundPnt, fGSD, fGSD, ImageWidth, ImageHeight, dstWidth, dstHeight, dstBuffer);
			GDALRasterIO(GDALGetRasterBand(dstDataset, i + 1), GF_Write, 0, 0, dstWidth, dstHeight, dstBuffer, dstWidth, dstHeight, GDT_Byte, 0, 0);

		}
		double dGeoTransform[6];
		//设置仿射变换参数
		dGeoTransform[0] = minPt.dX;
		dGeoTransform[1] = fGSD;
		dGeoTransform[2] = 0;
		dGeoTransform[3] = maxPt.dY;
		dGeoTransform[4] = 0;
		dGeoTransform[5] = fGSD;
		//设置地图投影信息
		int nZone; double x, y;
		tsmLatLongToUTM(dB * 180 / PI, dL * 180 / PI, &nZone, &x, &y);
		oSRS.SetProjCS("UTM (WGS84) in northern hemisphere");
		oSRS.SetWellKnownGeogCS("WGS84");
		oSRS.SetUTM(nZone, TRUE);
		oSRS.exportToWkt(&pszSRS_WKT);
		//写入投影信息和仿射变换参数
		GDALSetProjection(dstDataset, pszSRS_WKT);
		GDALSetGeoTransform(dstDataset,dGeoTransform);
		GDALClose(dstDataset);
		GDALClose(poDataset);
		CPLFree(pszSRS_WKT);
	}

	return 0;
}

long LevelUAVProcess::UAVGeoCorrection_GeoPntsProb(double dB, double dL, double dH, EO pEO, double fLen, int width, int height, THREEDPOINT *pGoundPnt, DPOINT* pMapImgPnt, int pntNum)
{
	pMapImgPnt[0].dX = width / 2; pMapImgPnt[0].dY = height / 2;
	pMapImgPnt[1].dX = 0; pMapImgPnt[1].dY = 0;
	pMapImgPnt[2].dX = 0; pMapImgPnt[2].dY = height;
	pMapImgPnt[3].dX = width; pMapImgPnt[3].dY = 0;
	double centerx = width / 2;
	double centery = height/ 2;
	pMapImgPnt[4].dX = width; pMapImgPnt[4].dY = height;
	for (int i = 0; i<5; ++i)
	{
		pMapImgPnt[i].dX = centerx- pMapImgPnt[i].dX;
		pMapImgPnt[i].dY = pMapImgPnt[i].dY- centery;
	}

	CoordinateTransBasic m_coordiTrans;
	int i = 0, j = 0;
	double dPhotoPt[3]; double dModelPt[3];
	memset(dPhotoPt, 0, 3 * sizeof(double));
	memset(dModelPt, 0, 3 * sizeof(double));

	double dXs = 0, dYs = 0, dZs = 0;

	double dRMatrix[9];
	//double ccdsize = 0.0000032;	//单位是m 一般相机ccd大小为3.12um 全画幅
	//对于不同的数据需要设置不同的参数
	double ccdsize=m_ccdSize;
	dXs = pEO.m_dX; dYs = pEO.m_dY; dZs = pEO.m_dZ;
	dRMatrix[0] = pEO.m_dRMatrix[0]; dRMatrix[1] = pEO.m_dRMatrix[1]; dRMatrix[2] = pEO.m_dRMatrix[2];
	dRMatrix[3] = pEO.m_dRMatrix[3]; dRMatrix[4] = pEO.m_dRMatrix[4]; dRMatrix[5] = pEO.m_dRMatrix[5];
	dRMatrix[6] = pEO.m_dRMatrix[6]; dRMatrix[7] = pEO.m_dRMatrix[7]; dRMatrix[8] = pEO.m_dRMatrix[8];

	for (i = 0; i<pntNum; i++)
	{
		memset(dPhotoPt, 0, 3 * sizeof(double));
		memset(dModelPt, 0, 3 * sizeof(double));

		//像平面坐标系转到像空间坐标系
		double img_pnt[2];
		dPhotoPt[0] = pMapImgPnt[i].dX*ccdsize;
		dPhotoPt[1] = pMapImgPnt[i].dY*ccdsize;
		dPhotoPt[2] = -fLen*ccdsize;					//单位是m
		MatrixMuti(dRMatrix, 3, 3, 1, dPhotoPt, dModelPt);
		pGoundPnt[i].dX = dXs + (pGoundPnt[i].dZ - dZs)*dModelPt[0] / dModelPt[2];
		pGoundPnt[i].dY = dYs + (pGoundPnt[i].dZ - dZs)*dModelPt[1] / dModelPt[2];
	}

	THREEDPOINT Pnt;
	THREEDPOINT TemPnt;

	m_coordiTrans.BLHToXYZ(dB, dL, 0, Pnt);
	double dEMMatrix[9], dPt[3];
	double dTempB, dTempL, dTempH;
	double x, y;
	int nZone;

	dEMMatrix[0] = -sin(dL);
	dEMMatrix[1] = -sin(dB)*cos(dL);
	dEMMatrix[2] = cos(dB)*cos(dL);
	dEMMatrix[3] = cos(dL);
	dEMMatrix[4] = -sin(dB)*sin(dL);
	dEMMatrix[5] = cos(dB)*sin(dL);
	dEMMatrix[6] = 0;
	dEMMatrix[7] = cos(dB);
	dEMMatrix[8] = sin(dB);

	//获取各个点的坐标
	for (i = 0; i<pntNum; i++)
	{
		dModelPt[0] = pGoundPnt[i].dX;
		dModelPt[1] = pGoundPnt[i].dY;
		dModelPt[2] = pGoundPnt[i].dZ;
		MatrixMuti(dEMMatrix, 3, 3, 1, dModelPt, dPt);

		TemPnt.dX = Pnt.dX + dPt[0];
		TemPnt.dY = Pnt.dY + dPt[1];
		TemPnt.dZ = Pnt.dZ + dPt[2];

		m_coordiTrans.XYZToBLH(TemPnt, dTempB, dTempL, dTempH);
		pGoundPnt[i].dX = dTempB;
		pGoundPnt[i].dY = dTempL;
		pGoundPnt[i].dZ = dTempH;
		tsmLatLongToUTM(pGoundPnt[i].dX, pGoundPnt[i].dY, &nZone, &x, &y);
		pGoundPnt[i].dX = x;
		pGoundPnt[i].dY = y;
	}
	return 0;
}

long LevelUAVProcess::UAVGeoCorrection_GeoPntsAccu(double dB, double dL, double dH, EO pEO, double fLen, int width, int height, THREEDPOINT *pGoundPnt, const char* pathDEM)
{
	CoordinateTransBasic m_coordiTrans;
	double dGeoTransform[6] = { 0 };
	double *pDEMPt = NULL;
	int nDEMSamples = 0, nDEMLines = 0;
	int nDEMZone = 0;
	DPOINT originPt;
	double dGSDX = 0, dGSDY = 0;
	int nC = 0, nY = 0;			//坐标取整变量
	double fDX = 0, fDY = 0;	//坐标取余数变量
	double dElevation = 0;
	DPOINT minPt, maxPt;			//影像上的最大最小点的范围
	DPOINT rangPnt[2];			//DEM数据的范围
	//根据中心点的经纬度获取投影带信息
	double dRMatrix[9], dEMMatrix[9];

	//成图坐标系到地心坐标系
	dEMMatrix[0] = -sin(dL);
	dEMMatrix[1] = -sin(dB)*cos(dL);
	dEMMatrix[2] = cos(dB)*cos(dL);
	dEMMatrix[3] = cos(dL);
	dEMMatrix[4] = -sin(dB)*sin(dL);
	dEMMatrix[5] = cos(dB)*sin(dL);
	dEMMatrix[6] = 0;
	dEMMatrix[7] = cos(dB);
	dEMMatrix[8] = sin(dB);

	//初始化最大最小点
	minPt.dX = MAX_NUM; minPt.dY = MAX_NUM;
	maxPt.dX = MIN_NUM; maxPt.dY = MIN_NUM;

	THREEDPOINT XYZPt;
	m_coordiTrans.BLHToXYZ(dB, dL, 0, XYZPt);
	int tmpnZone;
	int projType = 0;
	double tmpx, tmpy;
	tsmLatLongToUTM(dB * 180 / PI, dL * 180 / PI, &tmpnZone, &tmpx, &tmpy);
	//读取DEM文件，获取DEM文件的地理坐标和高程信息
	Level2Proc_ReadDEMFile(pathDEM, nDEMSamples, nDEMLines, rangPnt, dGeoTransform, nDEMZone, projType, pDEMPt);
	if (tmpnZone != nDEMZone)
	{
		//DEM文件区域和图像区域不匹配
		//此处可能存在6度分带和三度分带的问题，目前还没遇到
		printf("影像投影带与DEM投影带不一致\n");
		if (pDEMPt != NULL)
			delete[]pDEMPt;
		exit(-1);
	}
	if (projType != 2)
	{
		printf("影像投影与DEM投影不一致\n");
		if (pDEMPt != NULL)
			delete[]pDEMPt;
		exit(-1);
	}

	int i = 0, j = 0;
	double dPhotoPt[3]; double dModelPt[3], dGroundPnt[3];
	memset(dPhotoPt, 0, 3 * sizeof(double));
	memset(dModelPt, 0, 3 * sizeof(double));
	double dXs = 0, dYs = 0, dZs = 0;
	dXs = pEO.m_dX; dYs = pEO.m_dY; dZs = pEO.m_dZ;
	dRMatrix[0] = pEO.m_dRMatrix[0]; dRMatrix[1] = pEO.m_dRMatrix[1]; dRMatrix[2] = pEO.m_dRMatrix[2];
	dRMatrix[3] = pEO.m_dRMatrix[3]; dRMatrix[4] = pEO.m_dRMatrix[4]; dRMatrix[5] = pEO.m_dRMatrix[5];
	dRMatrix[6] = pEO.m_dRMatrix[6]; dRMatrix[7] = pEO.m_dRMatrix[7]; dRMatrix[8] = pEO.m_dRMatrix[8];


	THREEDPOINT Pnt;
	THREEDPOINT TemPnt;
	double dPt[3];
	double dTempB, dTempL, dTempH;
	double x, y;
	int nZone;
	m_coordiTrans.BLHToXYZ(dB, dL, 0, Pnt);
	originPt.dX = dGeoTransform[0];
	originPt.dY = dGeoTransform[3];
	dGSDX = dGeoTransform[1];
	dGSDY = dGeoTransform[5];
	double ccdSize = 0.00003125;
	//首次计算各个点的坐标
	for (i = 0; i<width; i++)
	{
		for (int j = 0; j < height; ++j)
		{
			memset(dPhotoPt, 0, 3 * sizeof(double));
			memset(dModelPt, 0, 3 * sizeof(double));

			//像平面坐标系转到像空间坐标系
			dPhotoPt[0] = (i - width / 2)*ccdSize;
			dPhotoPt[1] = (height / 2 - j)*ccdSize;
			dPhotoPt[2] = -fLen;
			MatrixMuti(dRMatrix, 3, 3, 1, dPhotoPt, dModelPt);
			pGoundPnt[i].dX = dXs + (pGoundPnt[i].dZ - dZs)*dModelPt[0] / dModelPt[2];
			pGoundPnt[i].dY = dYs + (pGoundPnt[i].dZ - dZs)*dModelPt[1] / dModelPt[2];
		}
	}

	//获取各个点的坐标
	for (i = 0; i<width*height; i++)
	{
		dModelPt[0] = pGoundPnt[i].dX;
		dModelPt[1] = pGoundPnt[i].dY;
		dModelPt[2] = pGoundPnt[i].dZ;
		MatrixMuti(dEMMatrix, 3, 3, 1, dModelPt, dPt);

		TemPnt.dX = Pnt.dX + dPt[0];
		TemPnt.dY = Pnt.dY + dPt[1];
		TemPnt.dZ = Pnt.dZ + dPt[2];

		m_coordiTrans.XYZToBLH(TemPnt, dTempB, dTempL, dTempH);
		pGoundPnt[i].dX = dTempB;
		pGoundPnt[i].dY = dTempL;

		tsmLatLongToUTM(pGoundPnt[i].dX, pGoundPnt[i].dY, &nZone, &x, &y);
		pGoundPnt[i].dX = x;
		pGoundPnt[i].dY = y;
		pGoundPnt[i].dZ = dTempH;
	}
	for (j = 0; j<width*height; j++)
	{
		//寻找最大最小点
		if (minPt.dX>pGoundPnt[j].dX)
			minPt.dX = pGoundPnt[j].dX;
		if (minPt.dY>pGoundPnt[j].dY)
			minPt.dY = pGoundPnt[j].dY;
		if (maxPt.dX<pGoundPnt[j].dX)
			maxPt.dX = pGoundPnt[j].dX;
		if (maxPt.dY<pGoundPnt[j].dY)
			maxPt.dY = pGoundPnt[j].dY;
	}
	double tmp1 = minPt.dX - rangPnt[0].dX;
	double tmp2 = minPt.dY - rangPnt[0].dY;
	double tmp3 = maxPt.dX - rangPnt[1].dX;
	double tmp4 = maxPt.dY - rangPnt[1].dY;
	if (!(tmp1>0 && tmp2<0 && tmp3<0 && tmp4>0))
	{
		printf("DEM范围小于影像范围\n");
		return 0;
	}
	else	//迭代求解
	{
		DPOINT tmpPnt;
		double xError, yError;
		int iterator_number = 0;
		//迭代求解准确的地理坐标
		for (i = 0; i<width; i++)
		{
			for (int j = 0; j < height; ++j)
			{
				memset(dPhotoPt, 0, 3 * sizeof(double));
				memset(dModelPt, 0, 3 * sizeof(double));

				//像平面坐标系转到像空间坐标系
				double img_pnt[2];
				dPhotoPt[0] = (i - width / 2)*ccdSize;
				dPhotoPt[1] = (height / 2 - j)*ccdSize;
				dPhotoPt[2] = -fLen;


				//像平面坐标系转到像空间坐标系
				MatrixMuti(dRMatrix, 3, 3, 1, dPhotoPt, dModelPt);
				pGoundPnt[i].dX = dXs + (pGoundPnt[i].dZ - dZs)*dModelPt[0] / dModelPt[2];
				pGoundPnt[i].dY = dYs + (pGoundPnt[i].dZ - dZs)*dModelPt[1] / dModelPt[2];
				tsmLatLongToUTM(pGoundPnt[i].dX, pGoundPnt[i].dY, &nZone, &x, &y);
				pGoundPnt[i].dX = x;
				pGoundPnt[i].dY = y;

				do
				{
					tmpPnt.dX = pGoundPnt[i].dX;
					tmpPnt.dY = pGoundPnt[i].dY;

					//获取高程值
					x = pGoundPnt[i].dX;
					y = pGoundPnt[i].dY;
					fDX = (x - originPt.dX) / dGSDX;
					fDY = (y - originPt.dY) / dGSDY;
					if (fDX>nDEMSamples - 1 || fDX<0 || fDY>nDEMLines - 1 || fDY<0)
						dElevation = pGoundPnt[i].dZ;
					else
					{
						nC = (int)fDX;
						nY = (int)fDY;
						//进行取余
						fDX -= nC;
						fDY -= nY;
						//双线性差值
						dElevation = (1 - fDX)*(1 - fDY)*pDEMPt[nY*nDEMSamples + nC] + fDX*(1 - fDY)*pDEMPt[nY*nDEMSamples + nC + 1] + (1 - fDX)*fDY*pDEMPt[(nY + 1)*nDEMSamples + nC] + fDX*fDY*pDEMPt[nY*(nDEMSamples + 1) + nC + 1];
					}

					pGoundPnt[i].dZ = dElevation;
					pGoundPnt[i].dX = dXs + (pGoundPnt[i].dZ - dZs)*dModelPt[0] / dModelPt[2];	//共线方程解算Xs点
					pGoundPnt[i].dY = dYs + (pGoundPnt[i].dZ - dZs)*dModelPt[1] / dModelPt[2]; 	//共线方程解算Ys点
					dGroundPnt[0] = pGoundPnt[i].dX;
					dGroundPnt[1] = pGoundPnt[i].dY;
					dGroundPnt[2] = pGoundPnt[i].dZ;
					MatrixMuti(dEMMatrix, 3, 3, 1, dGroundPnt, dPt);

					TemPnt.dX = Pnt.dX + dPt[0];
					TemPnt.dY = Pnt.dY + dPt[1];
					TemPnt.dZ = Pnt.dZ + dPt[2];

					m_coordiTrans.XYZToBLH(TemPnt, dTempB, dTempL, dTempH);
					pGoundPnt[i].dX = dTempB;
					pGoundPnt[i].dY = dTempL;

					tsmLatLongToUTM(pGoundPnt[i].dX, pGoundPnt[i].dY, &nZone, &x, &y);
					pGoundPnt[i].dX = x;
					pGoundPnt[i].dY = y;
					xError = pGoundPnt[i].dY - tmpPnt.dY;
					yError = pGoundPnt[i].dX - tmpPnt.dX;
					iterator_number++;
				} while ((abs(xError)>0.01 || abs(yError)>0.01) && iterator_number<30);

				MatrixMuti(dRMatrix, 3, 3, 1, dPhotoPt, dModelPt);
				pGoundPnt[i].dX = dXs + (pGoundPnt[i].dZ - dZs)*dModelPt[0] / dModelPt[2];
				pGoundPnt[i].dY = dYs + (pGoundPnt[i].dZ - dZs)*dModelPt[1] / dModelPt[2];
			}
		}
	}
	if (pDEMPt != NULL)
		delete[]pDEMPt;
	return 0;
}

long LevelUAVProcess::UAVGeoCorrect_Test()
{
	m_ccdSize = 0.00000488;
	m_x0 = 3659.6230;
	m_y0 = 2453.8943;
	double fLen = 7346.3820;
	char formatPos[128]="dB,dL,dH,roll,pitch,yaw";
	strcpy(m_posFormat,formatPos);
	UAVGeoCorrection_GeoCorrect("E:\\测试数据\\无人机","E:\\测试数据\\无人机","E:\\测试数据\\无人机\\POS.txt",NULL,3,0,4,fLen,0.2,30);
	return 0;
}
//===================================================================================================================================================
long LevelAerialTriangle::LevelATProc_InnerOrientation(vector<Pointf> &pnt,vector<Pointf> &pntInner)
{
	for(int i=0;i<pnt.size();++i)
	{
		Pointf tempPoint;
		tempPoint.x = m_x0+(m_mat[0]*pnt[i].x+m_mat[1]*pnt[i].y)*m_pixel_size;
		tempPoint.y = m_y0+(m_mat[2]*pnt[i].x+m_mat[3]*pnt[i].y)*m_pixel_size;
		pntInner.push_back(tempPoint);
	}
	return 0;
}

long LevelAerialTriangle::LevelATProc_GetMatchPntsTest(const char* path1,const char* path2)
{
	SiftGPU  *sift = new SiftGPU(1);
	SiftMatchGPU *matcher = new SiftMatchGPU(4096);

	vector<float> descriptors1(1), descriptors2(1);
	vector<SiftGPU::SiftKeypoint> keys1(1), keys2(1);    
	int num1 = 0, num2 = 0;

	if(sift->CreateContextGL() != SiftGPU::SIFTGPU_FULL_SUPPORTED) 
		return -1;
	if(sift->RunSIFT(path1))
	{
		num1 = sift->GetFeatureNum();
		keys1.resize(num1);    
		descriptors1.resize(128*num1);
		sift->GetFeatureVector(&keys1[0], &descriptors1[0]);
	}

	//You can have at most one OpenGL-based SiftGPU (per process).
	//Normally, you should just create one, and reuse on all images. 
	if(sift->RunSIFT(path2))
	{
		num2 = sift->GetFeatureNum();
		keys2.resize(num2);    
		descriptors2.resize(128*num2);
		sift->GetFeatureVector(&keys2[0], &descriptors2[0]);
	}
	matcher->VerifyContextGL();//must call once
	//matcher->CreateContextGL();
    //Set descriptors to match, the first argument must be either 0 or 1
    //if you want to use more than 4096 or less than 4096
    //call matcher->SetMaxSift() to change the limit before calling setdescriptor
    matcher->SetDescriptors(0, num1, &descriptors1[0]); //image 1
    matcher->SetDescriptors(1, num2, &descriptors2[0]); //image 2

    int (*match_buf)[2] = new int[num1][2];
    //use the default thresholds. Check the declaration in SiftGPU.h
    int num_match = matcher->GetSiftMatch(num1, match_buf);
    std::cout << num_match << " sift matches were found;\n";
    
    //enumerate all the feature matches
    for(int i  = 0; i < num_match; ++i)
    {
        //How to get the feature matches: 
        Pointf pnt1,pnt2;
		pnt1.x = keys1[match_buf[i][0]].x;
		pnt1.y = keys1[match_buf[i][0]].y;
		pnt2.x = keys2[match_buf[i][1]].x;
		pnt2.y = keys2[match_buf[i][1]].y;
		this->m_matchPoint.push_back(pnt1);
		this->m_matchPoint.push_back(pnt2);
    }

    //*****************GPU Guided SIFT MATCHING***************
    //example: define a homography, and use default threshold 32 to search in a 64x64 window
    //float h[3][3] = {{0.8f, 0, 0}, {0, 0.8f, 0}, {0, 0, 1.0f}};
    //matcher->SetFeatureLocation(0, &keys1[0]); //SetFeatureLocaiton after SetDescriptors
    //matcher->SetFeatureLocation(1, &keys2[0]);
    //num_match = matcher->GetGuidedSiftMatch(num1, match_buf, h, NULL);
    //std::cout << num_match << " guided sift matches were found;\n";
    //if you can want to use a Fundamental matrix, check the function definition
    // clean up..

    delete[] match_buf;

	return 0;
}
long LevelAerialTriangle::LevelATProc_MatchExtract(vector<string> imageList,vector<MatchPair> &match_pairs,vector<vector<Pointf>> &featureList)
{
	SiftGPU  *sift = new SiftGPU(1);
	SiftMatchGPU *matcher = new SiftMatchGPU(4096);
	if(sift->CreateContextGL() != SiftGPU::SIFTGPU_FULL_SUPPORTED) 
		return -1;
	
	int num = 0;
	int image_num = imageList.size();
	vector<vector<float>> descriptorsList;
	vector<vector<SiftGPU::SiftKeypoint>> featureImageList;
	descriptorsList.reserve(image_num);
	featureImageList.reserve(image_num);

	//获取影像的特征点
	for(int i=0;i<image_num;++i)
	{
		vector<float> desc;
		vector<SiftGPU::SiftKeypoint> keys;
		if(sift->RunSIFT(imageList[i].c_str()))
		{
			num = sift->GetFeatureNum();
			desc.resize(128*num);keys.resize(num);
			sift->GetFeatureVector(&keys[0],&desc[0]);
			descriptorsList.push_back(desc);
			featureImageList.push_back(keys);
		}
		desc.clear();
		keys.clear();
	}

	//影像匹配
	matcher->VerifyContextGL();
	for(int i=0;i<match_pairs.size();++i)
	{
		int idx1=match_pairs[i].img1_idx;
		int idx2=match_pairs[i].img2_idx;

		int num1 = featureImageList[idx1].size(),num2 = featureImageList[idx2].size();
		//use the default thresholds. Check the declaration in SiftGPU.h
		matcher->SetDescriptors(0,num1,&descriptorsList[idx1][0]);
		matcher->SetDescriptors(1,num2,&descriptorsList[idx2][0]);
		int (*match_buf)[2] = new int[num1][2];
		int num_match = matcher->GetSiftMatch(num1, match_buf);
		vector<Pointf> vecFeatures;
		for(int k = 0;k<num_match;++k)
		{
			//How to get the feature matches: 
			Pointf pnt1,pnt2;
			pnt1.x = featureImageList[idx1][match_buf[k][0]].x;
			pnt1.y = featureImageList[idx1][match_buf[k][0]].y;
			pnt2.x = featureImageList[idx2][match_buf[k][1]].x;
			pnt2.y = featureImageList[idx2][match_buf[k][1]].y;
			vecFeatures.push_back(pnt1);
			vecFeatures.push_back(pnt2);
		}
		featureList.push_back(vecFeatures);
		delete[]match_buf;
	}
	descriptorsList.clear();
	featureImageList.clear();

	return 0;
}

long LevelAerialTriangle::LevelATProc_MatchFeature(vector<string> imageList,vector<cv::detail::ImageFeatures> &fetures,vector<cv::detail::MatchesInfo> &match_pairs)
{
	SiftGPU  *sift = new SiftGPU(1);
	SiftMatchGPU *matcher = new SiftMatchGPU(4096);
	if(sift->CreateContextGL() != SiftGPU::SIFTGPU_FULL_SUPPORTED) 
		return -1;

	int num = 0;
	int image_num = imageList.size();

	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");

	//获取影像的特征点
	printf("获取影像特征点...\n");
	for(int i=0;i<image_num;++i)
	{
		vector<float> desc;
		vector<SiftGPU::SiftKeypoint> keys;
		cv::detail::ImageFeatures imgFeatures;
		
		//获取图像的大小
		GDALDatasetH m_dataset = GDALOpen(imageList[i].c_str(),GA_ReadOnly);
		int xsize = GDALGetRasterXSize(m_dataset);
		int ysize = GDALGetRasterYSize(m_dataset);
		GDALClose(m_dataset);

		imgFeatures.img_idx=i;
		imgFeatures.img_size=cv::Size(xsize,ysize);

		if(sift->RunSIFT(imageList[i].c_str()))
		{
			cv::KeyPoint keyPnt;
			num = sift->GetFeatureNum();
			desc.resize(128*num);keys.resize(num);
			sift->GetFeatureVector(&keys[0],&desc[0]);
			/*descriptorsList.push_back(desc);*/
			cv::UMat descMat=cv::UMat(num,128,CV_32FC1);
			memcpy(descMat.u->data,desc.data(),desc.size()*sizeof(float));
			for(int j=0;j<num;++j)
			{
				keyPnt.pt.x=keys[j].x;
				keyPnt.pt.y=keys[j].y;
				imgFeatures.keypoints.push_back(keyPnt);
			}
			descMat.copyTo(imgFeatures.descriptors);
		}
		desc.clear();
		keys.clear();
		fetures.push_back(imgFeatures);
	}
	float match_conf = 0.3;
	printf("特征点匹配...\n");
	cv::detail::BestOf2NearestMatcher feature_matcher(false, match_conf);
	feature_matcher(fetures, match_pairs);
	feature_matcher.collectGarbage();

	return 0;
}

long LevelAerialTriangle::LevelATProc_SaveAsENVI(const char* pathENVI)
{
	ofstream file1;
	long lError = 0;
	file1.open(pathENVI,ios::out);
	if(!file1.is_open())
		return -1;
	file1<<"; ENVI Image to Image GCP File "<<endl;
	file1<<"; base file: image 1"<<endl;
	file1<<"; warp file: image 1"<<endl;
	file1<<"; Base Image (x,y),warp Image(x,y)"<<endl;
	file1<<";"<<endl;

	for(size_t i=0;i<m_matchPoint.size()/2;++i)
	{
		file1<<setw(10)<<m_matchPoint[2*i+0].x<<setw(10)<<m_matchPoint[2*i+0].y
			<<setw(10)<<m_matchPoint[2*i+1].x<<setw(10)<<m_matchPoint[2*i+1].y<<endl;
	}
	if(file1.is_open())
		file1.close();
	return 0;
}

long LevelAerialTriangle::LevelATProc_ReadENVIPts(const char* pathENVI)
{
	//从文件中读取匹配点
	ifstream ifs(pathENVI,ios_base::in);
	if(!ifs.is_open())
		return -1;
	char chrTemp[2048];
	for(int i=0;i<5;++i)
		ifs.getline(chrTemp,2048);
	do 
	{
		ifs.getline(chrTemp,2048);
		Pointf pnt1,pnt2;
		sscanf(chrTemp,"%f%f%f%f",&pnt1.x,&pnt1.y,&pnt2.x,&pnt2.y);
		m_matchPoint.push_back(pnt1);
		m_matchPoint.push_back(pnt2);
	} while (!ifs.eof());
	ifs.close();
	return 0;
}
//===================================================================================================================================================
long LevelFlightLineAT::LevelATProc_RelativeOrientation(vector<Pointf> pnt1,REO &relOrientation1,vector<Pointf> pnt2,REO &relOrientation2)
{
	//获取内定向元素
	vector<Pointf> pnt1In,pnt2In;
	LevelATProc_InnerOrientation(pnt1,pnt1In);
	LevelATProc_InnerOrientation(pnt2,pnt2In);

	//进行相对定向
	//然后进行相对定向获取相对定向元素
	int num_point = pnt1.size();
	double var_add[5] = { 100,100,100,100,100 };				//五个变量的增量
	//临时变量
	double var[5];											//5个参数
	memset(var, 0, 5 * sizeof(double));
	double B[3];
	B[0] =200000;/*测试数据用*/ /*pnt1[1].x - pnt2[1].x;*///Bx本应当取2号点(6个点时)的视差，此时随机取2号点

	unsigned MaxTime = 0;					 //最大运算次数
	double *A = new double[5 * num_point];
	double *BP = new double[4 * num_point];
	double *L = new double[num_point];

	double FA[25];							//法方程系数
	double CH[5];							//常数项
	double a1r, a2r, a3r, b1r, b2r, b3r, c1r, c2r, c3r;
	double a1l, a2l, a3l, b1l, b2l, b3l, c1l, c2l, c3l;

	double phial = relOrientation1.m_phia;
	double omegal = relOrientation1.m_omega;
	double kappal = relOrientation1.m_kappa;

	a1l = cos(phial)*cos(kappal) + sin(phial)*sin(omegal)*sin(kappal);
	a2l = -cos(phial)*sin(omegal) - sin(phial)*sin(omegal)*sin(kappal);
	a3l = -sin(phial)*cos(omegal);

	b1l = cos(omegal)*sin(kappal);
	b2l = cos(omegal)*cos(kappal);
	b3l = -sin(omegal);

	c1l = sin(phial)*cos(kappal) + cos(phial)*sin(omegal)*sin(kappal);
	c2l = -sin(omegal)*sin(kappal) + cos(phial)*sin(omegal)*sin(kappal);
	c3l = cos(phial)*cos(omegal);

	int iteratornumber = 0;
	while ((OVER_LIMIT(var_add[0]) || OVER_LIMIT(var_add[1]) || OVER_LIMIT(var_add[2]) ||
		OVER_LIMIT(var_add[3]) || OVER_LIMIT(var_add[4])) /*&& MaxTime < 20*/)
	{
		memset(FA, 0, sizeof(double) * 25);
		memset(CH, 0, sizeof(double) * 5);
		int num = 0;

		//不严密求解
		double phiar = var[0]; double omegar = var[1]; double kappar = var[2];
		//微小角运算
		//计算X2,Y2,Z2; X1, Y1, Z1
		a1r = cos(phiar)*cos(kappar) - sin(phiar)*sin(omegar)*sin(kappar);
		a2r = -cos(phiar)*sin(kappar) - sin(phiar)*sin(omegar)*cos(kappar);
		a3r = -sin(phiar)*cos(omegar);

		b1r = cos(omegar)*sin(kappar);
		b2r = cos(omegar)*cos(kappar);
		b3r = -sin(omegar);

		c1r = sin(phiar)*cos(kappar) + cos(phiar)*sin(omegar)*sin(kappar);
		c2r = -sin(phiar)*sin(kappar) + cos(phiar)*sin(omegar)*cos(kappar);
		c3r = cos(phiar)*cos(omegar);

		//计算By,Bz,N',N,Q
		B[1] = /*tan(var[3]) * B[0];	*/					var[3]*B[0];
		B[2] =/* tan(var[4]) * B[0] / cos(var[3]);*/		var[4]*B[0];

		for (int i = 0; i< num_point; ++i)
		{
			num++;
			double R_XYZ[3], L_XYZ[3];

			//计算模型点坐标
			R_XYZ[0] = a1r*pnt2In[i].x + a2r*pnt2In[i].y - a3r*m_flen;
			R_XYZ[1] = b1r*pnt2In[i].x + b2r*pnt2In[i].y - b3r*m_flen;
			R_XYZ[2] = c1r*pnt2In[i].x + c2r*pnt2In[i].y - c3r*m_flen;

			L_XYZ[0] = a1l*pnt1In[i].x + a2l*pnt1In[i].y - a3l*m_flen;
			L_XYZ[1] = b1l*pnt1In[i].x + b2l*pnt1In[i].y - b3l*m_flen;
			L_XYZ[2] = c1l*pnt1In[i].x + c2l*pnt1In[i].y - c3l*m_flen;

			double RN = (B[0] * L_XYZ[2] - B[2] * L_XYZ[0]) / (L_XYZ[0] * R_XYZ[2] - L_XYZ[2] * R_XYZ[0]);   //N'
			double LN = (B[0] * R_XYZ[2] - B[2] * R_XYZ[0]) / (L_XYZ[0] * R_XYZ[2] - L_XYZ[2] * R_XYZ[0]);   //N
			L[i] = LN*L_XYZ[1] - RN*R_XYZ[1] - B[1];

			//计算系数
			A[5 * i + 0] = -(R_XYZ[0] * R_XYZ[1] / R_XYZ[2]) * RN;
			A[5 * i + 1] = -(R_XYZ[2] + (R_XYZ[1] * R_XYZ[1]) / R_XYZ[2]) * RN;
			A[5 * i + 2] = R_XYZ[0] * RN;
			A[5 * i + 3] = B[0];
			A[5 * i + 4] = -R_XYZ[1] * B[0] / R_XYZ[2];

			for (int j = 0; j<5; ++j)
			{
				CH[j] += A[5 * i + j] * L[i];
				for (int m = 0; m<5; ++m)
					FA[j * 5 + m] += A[5 * i + j] * A[5 * i + m];
			}
		}

		double IFA[5 * 5];
		double MCH[5];

		MatrixInverse(FA, 5, IFA);
		MatrixMuti(CH, 1, 5, 5, IFA, MCH);

		for (int ii = 0; ii < 5; ++ii)
			var_add[ii] = MCH[ii];
		MaxTime++;
		for (int ii = 0; ii<5; ++ii)
			var[ii] += var_add[ii];
		iteratornumber++;
		printf("\r迭代次数：%d", iteratornumber);
	}
	printf("\n");

	//求解结果输出
	relOrientation2.m_Bx	 = B[0];
	relOrientation2.m_By	 = B[1];
	relOrientation2.m_Bz	 = B[2];
	relOrientation2.m_phia  = var[0];
	relOrientation2.m_omega = var[1];
	relOrientation2.m_kappa = var[2];
	MatrixRotate(relOrientation2.m_dRMatrix, var[0], var[1], var[2]);

	delete[]A;
	delete[]L;
	pnt1In.clear();
	pnt2In.clear();

	return 0;
}

long LevelFlightLineAT::LevelATProc_ModelPoint(vector<Pointf> pnt1,REO &relOrientation1,vector<Pointf> pnt2,REO &relOrientation2,vector<Point3f> &modelPoint)
{
	//计算转换后的坐标
	for (int i=0;i<pnt1.size();++i)
	{
		double pntModel1[3],pntModel2[3];
		double pnt1Ex[3],pnt2Ex[3];
		pnt1Ex[0]=pnt1[i].x;pnt1Ex[1]=pnt1[i].y;pnt1Ex[2]=-m_flen;
		pnt2Ex[0]=pnt2[i].x;pnt2Ex[1]=pnt2[i].y;pnt2Ex[2]=-m_flen;

		//相对定向后的坐标
		MatrixMuti(relOrientation1.m_dRMatrix,3,3,1,pnt1Ex,pntModel1);
		MatrixMuti(relOrientation2.m_dRMatrix,3,3,1,pnt2Ex,pntModel2);

		//计算视差
		float bx=relOrientation2.m_Bx,by=relOrientation2.m_By,bz=relOrientation2.m_Bz;
		float N1=(bx*pntModel2[2]-bz*pntModel2[0])/(pntModel1[0]*pntModel2[2]-pntModel2[0]*pntModel1[2]);
		float N2=(bx*pntModel1[2]-bz*pntModel1[0])/(pntModel1[0]*pntModel2[2]-pntModel2[0]*pntModel1[2]); 
		float QQ=N1*pntModel1[1]-N2*pntModel2[1]-by; 
		
		Point3f pntMod;
		pntMod.x = 0.0 + N1*pntModel1[0];
		pntMod.y =(0.0 + N1*pntModel1[1]+by+N2*pntModel2[1])/2;
		pntMod.z = N1 * pntModel1[2];
		modelPoint.push_back(pntMod);
	}
	return 0;
}

long LevelFlightLineAT::LevelATProc_ModelPointNorm(vector<vector<Pointf>> &imgPntList,vector<vector<Point3f>> &modelPointList,vector<Point3f> &phoCoordi,vector<REO> &relOrientationList)
{
	//由于是单航带所以直接从后往前直接计算就好了
	int size_models = imgPntList.size();

	//计算归一化系数
	int kp=1;
	vector<double> normal_param;
	for(int i=size_models-1;i>=1;++i)
	{
		//两个模型中的匹配点的个数
		int sizePntR = imgPntList[i].size()/2;		//右模型
		int sizePntL = imgPntList[i-1].size()/2;	//左模型

		vector<double> k;
		for(int m=0;m<sizePntL;++m)
		{
			for(int n=0;n<sizePntR;++n)
			{
				//左模型中右影像的点和右模型中左影像的点进行匹配
				if(ISEQUAL(imgPntList[i-1][2*m+1].x,imgPntList[i][2*n+0].x)&&ISEQUAL(imgPntList[i-1][2*m+1].y,imgPntList[i][2*n+0].y))
				{
					//计算归一化系数
					double pnt31L[3],pnt31R[3],pnt32L[3],pnt32R[3];
					double pM31L[3],pM31R[3],pM32L[3],pM32R[3];

					pnt31L[0]=imgPntList[i-1][2*m+0].x;pnt31L[1]=imgPntList[i-1][2*m+0].y;pnt31L[2]=-m_flen;
					pnt31R[0]=imgPntList[i-1][2*m+1].x;pnt31R[1]=imgPntList[i-1][2*m+1].y;pnt31R[2]=-m_flen;
					MatrixMuti(relOrientationList[2*(i-1)+0].m_dRMatrix,3,3,1,pnt31L,pM31L);
					MatrixMuti(relOrientationList[2*(i-1)+1].m_dRMatrix,3,3,1,pnt31R,pM31R);
					//左边模型
					double B[3];
					B[0]=relOrientationList[2*(i-1)+1].m_Bx;
					B[1]=relOrientationList[2*(i-1)+1].m_By;
					B[2]=relOrientationList[2*(i-1)+1].m_Bz;

					double LN1= (B[0] * pM31L[2] - B[2] * pM31L[0]) / (pM31L[0] * pM31R[2] - pM31L[2] * pM31R[0]);
					double RN1= (B[0] * pM31R[2] - B[2] * pM31R[0]) / (pM31L[0] * pM31R[2] - pM31L[2] * pM31R[0]);

					pnt32L[0]=imgPntList[i][2*n+0].x;pnt32L[1]=imgPntList[i][2*n+0].y;pnt32L[2]=-m_flen;
					pnt32R[0]=imgPntList[i][2*n+1].x;pnt32R[1]=imgPntList[i][2*n+1].y;pnt32R[2]=-m_flen;
					MatrixMuti(relOrientationList[2*i+0].m_dRMatrix,3,3,1,pnt32L,pM32L);
					MatrixMuti(relOrientationList[2*i+1].m_dRMatrix,3,3,1,pnt32R,pM32R);
					//右边模型
					B[0]=relOrientationList[2*i+1].m_Bx;
					B[1]=relOrientationList[2*i+1].m_By;
					B[2]=relOrientationList[2*i+1].m_Bz;
					double LN2=(B[0] * pM32L[2] - B[2] * pM32L[0]) / (pM32L[0] * pM32R[2] - pM32L[2] * pM32R[0]);
					double RN2=(B[0] * pM32R[2] - B[2] * pM32R[0]) / (pM32L[0] * pM32R[2] - pM32L[2] * pM32R[0]);
					double tk =(LN2*pM31R[2]-relOrientationList[2*i+1].m_Bx)/(LN2*pM32L[2]);k.push_back(tk);
					k.push_back(tk);
				}
			}
		}
		//获取均值
		double meank=0;
		for(int t=0;t<k.size();++t)
			meank+=k[t]/k.size();
		kp*=meank;
		normal_param.push_back(meank);
		normal_param.push_back(1);

		//计算归一化模型坐标
		for (int j=i;j<size_models;++j)
		{
			relOrientationList[2*j+1].m_Bx*=kp;
			relOrientationList[2*j+1].m_By*=kp;
			relOrientationList[2*j+1].m_Bz*=kp;
			for(int t=0;t<modelPointList[j].size();++t)
			{
				modelPointList[j][t].x*=kp;
				modelPointList[j][t].y*=kp;			
				modelPointList[j][t].z*=kp;
			}
		}
	}

	//模型摄测点坐标的计算
	double m=1;	//坐标放大倍数
	double Xps=0,Yps=0,Zps=m*m_flen;
	int sizek=normal_param.size();
	for (int i=0;i<modelPointList.size();++i)
	{
		for(int j=0;j<modelPointList[i].size();++j)
		{
			double pnt31L[3],pnt31R[3];
			double pM31L[3],pM31R[3];

			pnt31L[0]=imgPntList[i-1][2*m+0].x;pnt31L[1]=imgPntList[i-1][2*m+0].y;pnt31L[2]=-m_flen;
			pnt31R[0]=imgPntList[i-1][2*m+1].x;pnt31R[1]=imgPntList[i-1][2*m+1].y;pnt31R[2]=-m_flen;
			MatrixMuti(relOrientationList[2*i+0].m_dRMatrix,3,3,1,pnt31L,pM31L);
			MatrixMuti(relOrientationList[2*i+1].m_dRMatrix,3,3,1,pnt31R,pM31R);
			//左边模型
			double B[3];
			B[0]=relOrientationList[2*i+1].m_Bx;
			B[1]=relOrientationList[2*i+1].m_By;
			B[2]=relOrientationList[2*i+1].m_Bz;

			double LN1= (B[0] * pM31L[2] - B[2] * pM31L[0]) / (pM31L[0] * pM31R[2] - pM31L[2] * pM31R[0]);
			double RN1= (B[0] * pM31R[2] - B[2] * pM31R[0]) / (pM31L[0] * pM31R[2] - pM31L[2] * pM31R[0]);

			Point3f Mp;
			double deltaXps = 0;
			double deltaZps = 0;
			double deltaYps = m*normal_param[sizek-1-i]*relOrientationList[2*i+1].m_By;
			Mp.x = deltaXps+Xps + m*LN1*pM31L[0];
			Mp.y = (deltaYps + m*normal_param[sizek-1-i]*LN1*pM31L[1]+Yps+deltaYps+normal_param[sizek-1-i]*m*RN1*pM31R[1])/2;
			Mp.z = Zps+deltaZps+normal_param[sizek-1-i]*m*LN1*pM31L[2];
			
			if(i>0)
			{
				deltaXps = relOrientationList[2*(i-1)+1].m_Bx*normal_param[sizek-1-i+1]*m;
				deltaZps = relOrientationList[2*(i-1)+1].m_Bz;
			}
			Xps+=deltaXps;
			Yps+=deltaYps;
			Zps+=deltaZps;
			phoCoordi.push_back(Mp);
		}
	}
	return 0;
}


//没有找到合适的数据来测试模型坐标归一化的准确性但是我认为应该是准确的
void LevelFlightLineAT::LevelATProc_RelativeOrientationTest()
{
	vector<vector<Pointf>>  imgPntList;
	vector<vector<Point3f>> modelPointList;
	vector<Point3f> phoCoordi;
	vector<REO> relOrientationList;

	char* pathPoint1="C:\\Users\\Administrator\\Downloads\\341455air_survey\\DATA\\1.TXT";
	ifstream ifs1(pathPoint1,ios_base::in);
	if(!ifs1.is_open())
		return ;
	char chrTemp[2048];
	vector<Pointf> pnt1,pnt2;
	vector<Pointf> vecpnt1;
	do 
	{
		Pointf pt1,pt2;
		int num;
		ifs1.getline(chrTemp,2048);
		if(chrTemp[0]!=0)
		{
			sscanf(chrTemp,"%d%f%f%f%f",&num,&pt1.x,&pt1.y,&pt2.x,&pt2.y);
			pnt1.push_back(pt1);
			pnt2.push_back(pt2);

			vecpnt1.push_back(pt1);
			vecpnt1.push_back(pt2);
		}
	} while (!ifs1.eof());
	ifs1.close();

	REO reoL1,reoR1;
	reoL1.m_phia=reoL1.m_omega=reoL1.m_kappa=0;
	reoL1.m_Bx=reoL1.m_By=reoL1.m_Bz=0;
	MatrixRotate(reoL1.m_dRMatrix,0,0,0);
	LevelATProc_RelativeOrientation(pnt1,reoL1,pnt2,reoR1);
	vector<Point3f> pntModel1;
	LevelATProc_ModelPoint(pnt1,reoL1,pnt2,reoR1,pntModel1);


	char* pathPoint2="C:\\Users\\Administrator\\Downloads\\341455air_survey\\DATA\\2.TXT";
	ifstream ifs2(pathPoint2,ios_base::in);
	if(!ifs2.is_open())
		return ;
	 pnt1.clear();pnt2.clear();
	vector<Pointf> vecpnt2;
	do 
	{
		Pointf pt1,pt2;
		int num;
		ifs2.getline(chrTemp,2048);
		if(chrTemp[0]!=0)
		{
			sscanf(chrTemp,"%d%f%f%f%f",&num,&pt1.x,&pt1.y,&pt2.x,&pt2.y);
			pnt1.push_back(pt1);
			pnt2.push_back(pt2);

			vecpnt2.push_back(pt1);
			vecpnt2.push_back(pt2);
		}
	} while (!ifs2.eof());
	ifs2.close();

	REO reoL2,reoR2;
	reoL2.m_phia=reoL2.m_omega=reoL2.m_kappa=0;
	reoL2.m_Bx=reoL2.m_By=reoL2.m_Bz=0;
	MatrixRotate(reoL2.m_dRMatrix,0,0,0);
	LevelATProc_RelativeOrientation(pnt1,reoL2,pnt2,reoR2);
	vector<Point3f> pntModel2;
	LevelATProc_ModelPoint(pnt1,reoL2,pnt2,reoR2,pntModel2);


	char* pathPoint3="C:\\Users\\Administrator\\Downloads\\341455air_survey\\DATA\\3.TXT";
	ifstream ifs3(pathPoint1,ios_base::in);
	if(!ifs3.is_open())
		return ;
	 pnt1.clear();pnt2.clear();
	vector<Pointf> vecpnt3;
	do 
	{
		Pointf pt1,pt2;
		int num;
		ifs3.getline(chrTemp,2048);
		if(chrTemp[0]!=0)
		{
			sscanf(chrTemp,"%d%f%f%f%f",&num,&pt1.x,&pt1.y,&pt2.x,&pt2.y);
			pnt1.push_back(pt1);
			pnt2.push_back(pt2);

			vecpnt3.push_back(pt1);
			vecpnt3.push_back(pt2);
		}
	} while (!ifs3.eof());
	ifs3.close();

	REO reoL3,reoR3;
	reoL3.m_phia=reoL3.m_omega=reoL3.m_kappa=0;
	reoL3.m_Bx=reoL3.m_By=reoL3.m_Bz=0;
	MatrixRotate(reoL3.m_dRMatrix,0,0,0);
	LevelATProc_RelativeOrientation(pnt1,reoL3,pnt2,reoR3);
	vector<Point3f> pntModel3;
	LevelATProc_ModelPoint(pnt1,reoL3,pnt2,reoR3,pntModel3);

	imgPntList.push_back(vecpnt1);
	modelPointList.push_back(pntModel1);
	relOrientationList.push_back(reoL1);
	relOrientationList.push_back(reoR1);

	imgPntList.push_back(vecpnt2);
	modelPointList.push_back(pntModel2);
	relOrientationList.push_back(reoL2);
	relOrientationList.push_back(reoR3);

	imgPntList.push_back(vecpnt3);
	modelPointList.push_back(pntModel3);
	relOrientationList.push_back(reoL3);
	relOrientationList.push_back(reoR3);

	LevelATProc_ModelPointNorm(imgPntList,modelPointList,phoCoordi,relOrientationList);
	int i=0;
}

//==========================================================================================================================================================
#define ENABLE_LOG 1
long LevelFlightLineComputerVision::LevelATProc_HomographyMatrix(vector<vector<Pointf>> &imgPntList,vector<cv::Mat> &homoMatrixList)
{
	vector<cv::Point2f> m_leftPoints,m_rightPoints;
	vector<cv::Mat>     homoMatrixs;
	for(int i=0;i<imgPntList.size();++i)
	{
		m_leftPoints.clear();
		m_rightPoints.clear();
		for(int j=0;j<imgPntList[i].size()/2;++j)
		{
			m_leftPoints.push_back(cv::Point2f(imgPntList[i][2*j+0].x,imgPntList[i][2*j+0].y));
			m_rightPoints.push_back(cv::Point2f(imgPntList[i][2*j+1].x,imgPntList[i][2*j+1].y));
		}
		cv::Mat homo=cv::findHomography(m_rightPoints,m_leftPoints,CV_RANSAC,13);
		homoMatrixs.push_back(homo);
	}
	cv::Mat homoSequence = cv::Mat::eye(3,3,CV_64F);
	homoMatrixList.push_back(cv::Mat::eye(3,3,CV_64F));
	for(int i=0;i<homoMatrixs.size();++i)
	{
		homoSequence=homoSequence*homoMatrixs[i];
		cv::Mat temp;
		homoSequence.copyTo(temp);
		homoMatrixList.push_back(temp);
	}

	homoMatrixs.clear();

	int center = homoMatrixList.size()/2;
	cv::Mat r_inv = homoMatrixList[center].inv();
	for(int i=0;i<homoMatrixList.size();++i)
		homoMatrixList[i]=r_inv*homoMatrixList[i];


	return 0;
}

long LevelFlightLineComputerVision::LevelATProc_HomoBundler(vector<vector<Pointf>> &imgPntList,vector<cv::Size> &imgSizeList,vector<cv::Mat> &homoMatrixList,vector<cv::detail::CameraParams> &vec_Cameras)
{
	vector<double> focal;
	LevelATProc_HomographyFocal(imgPntList,imgSizeList,homoMatrixList,focal);

	vec_Cameras.assign(imgSizeList.size(),cv::detail::CameraParams());
	for(int i=0;i<vec_Cameras.size();++i)
	{
		vec_Cameras[i].focal = focal[i];
		vec_Cameras[i].aspect=1.0;
	}

	CalcRotation(vec_Cameras,homoMatrixList);
	//中心点坐标
	for(int i=0;i<imgSizeList.size();++i)
	{
		vec_Cameras[i].ppx += 0.5 * imgSizeList[i].width;
		vec_Cameras[i].ppy += 0.5 * imgSizeList[i].height;
	}
	for (size_t i = 0; i < vec_Cameras.size(); ++i)
	{
		cv::Mat R;
		vec_Cameras[i].R.convertTo(R, CV_32F);
		vec_Cameras[i].R = R;
	}

	//光束法
	//计算总的匹配点数目
	int total_num_matches_=0;
	for(int i=0;i<imgPntList.size();++i)
		total_num_matches_+=imgPntList[i].size()/2;

	int num_params_per_cam_ = 7;
	int num_errs_per_measurement_ =2;
	int num_images_ = imgSizeList.size();
	
	cv::TermCriteria term_criteria_;
	cv:: Mat cam_params_;

	//初始化参数
	cam_params_.create(num_images_ * 7, 1, CV_64F);
	cv::SVD svd;
	for (int i = 0; i < num_images_; ++i)
	{
		cam_params_.at<double>(i * 7, 0) = vec_Cameras[i].focal;
		cam_params_.at<double>(i * 7 + 1, 0) = vec_Cameras[i].ppx;
		cam_params_.at<double>(i * 7 + 2, 0) = vec_Cameras[i].ppy;
		cam_params_.at<double>(i * 7 + 3, 0) = vec_Cameras[i].aspect;

		svd(vec_Cameras[i].R, cv::SVD::FULL_UV);
		cv::Mat R = svd.u * svd.vt;
		if (determinant(R) < 0)
			R *= -1;

		cv::Mat rvec;
		cv::Rodrigues(R, rvec);
		CV_Assert(rvec.type() == CV_32F);
		cam_params_.at<double>(i * 7 + 4, 0) = rvec.at<float>(0, 0);
		cam_params_.at<double>(i * 7 + 5, 0) = rvec.at<float>(1, 0);
		cam_params_.at<double>(i * 7 + 6, 0) = rvec.at<float>(2, 0);
	}


	CvLevMarq solver(num_images_ * num_params_per_cam_,
		total_num_matches_ * num_errs_per_measurement_,
		term_criteria_);

	cv::Mat err, jac;
	cv::Mat err1,err2;
	CvMat matParams = cam_params_;
	cvCopy(&matParams, solver.param);
	int iter = 0; //迭代次数
	while(true)
	{
		const CvMat* _param = 0;
		CvMat* _jac = 0;
		CvMat* _err = 0;

		bool proceed = solver.update(_param, _jac, _err);

		cvCopy(_param, &matParams);

		if (!proceed || !_err)
			break;

		if (_jac)
		{
			CalcJacobian(jac,err1,err2,cam_params_,imgPntList,total_num_matches_);
			CvMat tmp = jac;
			cvCopy(&tmp, _jac);
		}

		if (_err)
		{
			CalcError(err,cam_params_,imgPntList,total_num_matches_);
			iter++;
			CvMat tmp = err;
			cvCopy(&tmp, _err);
		}
		cout<<"Bundle adjustment, final RMS error: " << std::sqrt(err.dot(err) / total_num_matches_)<<endl;
		cout<<"Bundle adjustment, iterations done: " << iter<<endl;
	}

	//以第一张图像为中心
	CalcRefineCamera(vec_Cameras,cam_params_);
	int center = vec_Cameras.size()/2;
	cv::Mat r_inv = vec_Cameras[center].R.inv();
	for(int i=0;i<vec_Cameras.size();++i)
		vec_Cameras[i].R=r_inv*vec_Cameras[i].R;


	homoMatrixList.clear();
	return 0;

 }
long LevelFlightLineComputerVision::LevelATProc_HomoBundler(vector<cv::detail::ImageFeatures> &fetures,vector<cv::detail::MatchesInfo> &match_pairs,vector<cv::detail::CameraParams> &vec_Cameras)
{
	printf("光束法平差...\n");
	cv::detail::HomographyBasedEstimator estimator;
	if (!estimator(fetures, match_pairs, vec_Cameras))
	{
		printf("Homography estimation failed.\n");
		return -1;
	}

	for (size_t i = 0; i < vec_Cameras.size(); ++i)
	{
		cv::Mat R;
		vec_Cameras[i].R.convertTo(R, CV_32F);
		vec_Cameras[i].R = R;
	}

	float conf_thresh = 1;
	cv::Ptr<cv::detail::BundleAdjusterBase> adjuster;
	adjuster = cv::makePtr<cv::detail::BundleAdjusterReproj>();
	adjuster->setConfThresh(conf_thresh);
	cv::Mat_<uchar> refine_mask = cv::Mat::zeros(3, 3, CV_8U);
	refine_mask(0,0) = 1;refine_mask(0,1) = 1;
	refine_mask(0,2) = 1;refine_mask(1,1) = 1;
	refine_mask(1,2) = 1;
	adjuster->setRefinementMask(refine_mask);
	if (!(*adjuster)(fetures, match_pairs, vec_Cameras))
	{
		printf("Camera parameters adjusting failed.\n");
		return -1;
	}

	return 0;
}

//估计焦距
long LevelFlightLineComputerVision::LevelATProc_HomographyFocal(vector<vector<Pointf>> &imgPntList,vector<cv::Size> &imgSizeList,vector<cv::Mat> &homoMatrixList,vector<double> &focals)
{
	int homoSize=homoMatrixList.size();
	for(int i=0;i<homoSize;++i)
	{
		double f1,f2;
		bool f1ok,f2ok;
		CalcFocalsFromHomography(homoMatrixList[i],f1,f2,f1ok,f2ok);
		if (f1ok && f2ok)  
			focals.push_back(sqrt(f1 * f2)); 
		CalcFocalsFromHomography(homoMatrixList[i].inv(),f1,f2,f1ok,f2ok);
		if (f1ok && f2ok)  
			focals.push_back(sqrt(f1 * f2));
	}
	if (static_cast<int>(focals.size()) >= homoSize-1)
	{
		double median;
		sort(focals.begin(), focals.end());
		if (focals.size() % 2 == 1)
			median = focals[focals.size() / 2];
		else
			median = (focals[focals.size() / 2 - 1] + focals[focals.size() / 2]) * 0.5;
		focals.resize(homoSize);
		for (int i = 0; i < homoSize; ++i)
			focals[i] = median;
	}
	else
	{
		focals.clear();
		double focals_sum = 0;
		for (int i = 0; i < homoSize; ++i)
			focals_sum += imgSizeList[i].width + imgSizeList[i].height;
		for (int i = 0; i < homoSize; ++i)
			focals.push_back(focals_sum / homoSize);
	}

	//均值
	//probFocal=accumulate(focals.begin(),focals.end(),0)/focals.size();
	return 0;
}

//随便找两幅影像测试效果
void LevelFlightLineComputerVision::LevelATProc_HomographTest()
{
	//影像
	string str1="E:\\测试数据\\影像匹配\\DSC00003s.JPG";
	string str2="E:\\测试数据\\影像匹配\\DSC00004s.JPG";
	string str3="E:\\测试数据\\影像匹配\\DSC00005s.JPG";
	string str4="E:\\测试数据\\影像匹配\\DSC00006s.JPG";
	string str5="E:\\测试数据\\影像匹配\\DSC00007s.JPG";
	string str6="E:\\测试数据\\影像匹配\\DSC00008s.JPG";
	string str7="E:\\测试数据\\影像匹配\\DSC00009s.JPG";
	string str8="E:\\测试数据\\影像匹配\\DSC00010s.JPG";
	string str9="E:\\测试数据\\影像匹配\\DSC00011s.JPG";
	string str10="E:\\测试数据\\影像匹配\\DSC00010s.JPG";
	string str11="E:\\测试数据\\影像匹配\\DSC00012s.JPG";
	string str12="E:\\测试数据\\影像匹配\\DSC00013s.JPG";
	string str13="E:\\测试数据\\影像匹配\\DSC00014s.JPG";
	string str14="E:\\测试数据\\影像匹配\\DSC00015s.JPG";
	string str15="E:\\测试数据\\影像匹配\\DSC00016s.JPG";
	string str16="E:\\测试数据\\影像匹配\\DSC00017s.JPG";
	string str17="E:\\测试数据\\影像匹配\\DSC00018s.JPG";
	string str18="E:\\测试数据\\影像匹配\\DSC00019s.JPG";
	string str19="E:\\测试数据\\影像匹配\\DSC00020s.JPG";
	string str20="E:\\测试数据\\影像匹配\\DSC00021s.JPG";
	string str21="E:\\测试数据\\影像匹配\\DSC00022s.JPG";
	//LevelATProc_GetMatchPntsTest(str1.c_str(),str2.c_str());

	vector<string> m_imageList;
	m_imageList.push_back(str1);
	m_imageList.push_back(str2);
	m_imageList.push_back(str3);
	m_imageList.push_back(str4);
	m_imageList.push_back(str5);
	m_imageList.push_back(str6);
	m_imageList.push_back(str7);
	m_imageList.push_back(str8);
	m_imageList.push_back(str9);
	m_imageList.push_back(str10);
	m_imageList.push_back(str11);
	m_imageList.push_back(str12);
	m_imageList.push_back(str13);
	m_imageList.push_back(str14);
	m_imageList.push_back(str15);
	m_imageList.push_back(str16);
	m_imageList.push_back(str17);
	m_imageList.push_back(str18);
	m_imageList.push_back(str19);
	m_imageList.push_back(str20);
	m_imageList.push_back(str21);
	vector<MatchPair> vec_matchPairs;
	for(int i=0;i<m_imageList.size()-1;++i)
	{
		MatchPair pairs;
		pairs.img1_idx=i;
		pairs.img2_idx=i+1;
		vec_matchPairs.push_back(pairs);
	}

	//vector<vector<Pointf>> featureList;
	//LevelATProc_MatchExtract(m_imageList,vec_matchPairs,featureList);

	vector<cv::detail::ImageFeatures> imgFeatures;
	vector<cv::detail::MatchesInfo>  matchPairs;
	LevelATProc_MatchFeature(m_imageList,imgFeatures,matchPairs);
	vector<cv::detail::CameraParams> cameras;
	LevelATProc_HomoBundler(imgFeatures,matchPairs,cameras);


	//vector<cv::detail::CameraParams> cameras;
	//vector<cv::Size> imgSizeList;
	//GDALAllRegister();
	//CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	//for(int i=0;i<m_imageList.size();++i)
	//{
	//	GDALDatasetH m_dataset = GDALOpen(m_imageList[i].c_str(),GA_ReadOnly);
	//	int x = GDALGetRasterXSize(m_dataset);
	//	int y = GDALGetRasterXSize(m_dataset);
	//	GDALClose(m_dataset);
	//	cv::Size size1(x,y);
	//	imgSizeList.push_back(size1);
	//}	
	//vector<cv::Mat> homoMatList;
	//LevelATProc_HomographyMatrix(featureList,homoMatList);
	//LevelATProc_HomoBundler(featureList,imgSizeList,homoMatList,cameras);

	MosaicProcessRoationTranslate cameraMosaic;
	MosaicProcessHomo homoMosaic;
	cv::Point2f pnt1,pnt2;
	//homoMosaic.MosaicProc_Range(m_imageList,pnt1,pnt2,homoMatList);
	cameraMosaic.MosaicProc_RangePlane(m_imageList,pnt1,pnt2,cameras);
	char* pathOut="E:\\测试数据\\影像匹配\\mosaicHomo.tif";
	//homoMosaic.MosaicProc_Mosaic(m_imageList,pathOut,pnt1,pnt2,homoMatList);
	cameraMosaic.MosaicProc_MosaicPlane(m_imageList,pathOut,pnt1,pnt2,cameras);

}