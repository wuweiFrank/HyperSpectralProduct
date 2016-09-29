#pragma once
#include <vector>
#include "opencv/include/opencv2/core/core.hpp"
#include "opencv/include/opencv2/stitching/detail/camera.hpp"
using namespace  std;

class MosaicProcessHomo
{
public:
	//图像拼接处理获取拼接后影像的范围
	void MosaicProc_Range(vector<string> imgList,cv::Point2f &pntMin,cv::Point2f &pntMax,vector<cv::Mat> homoList);

	//影像拼接
	void MosaicProc_Mosaic(vector<string> imgList,const char* pathOut,cv::Point2f pntMin,cv::Point2f pntMax,vector<cv::Mat> homoList);
};

class MosaicProcessRoationTranslate
{
public:
	//图像拼接处理获取拼接后影像的范围
	void MosaicProc_RangePlane(vector<string> imgList,cv::Point2f &pntMin,cv::Point2f &pntMax,vector<cv::detail::CameraParams> &cameras);

	//影像拼接
	void MosaicProc_MosaicPlane(vector<string> imgList,const char* pathOut,cv::Point2f pntMin,cv::Point2f pntMax,vector<cv::detail::CameraParams> &cameras);

private:
	//转换为矩阵形式
	void MosaicProc_TransMat(cv::detail::CameraParams &camera,double* rot,double* trans);
};

