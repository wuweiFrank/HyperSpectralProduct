#pragma once

#ifndef _SPFilter_
#define _SPFilter_

//GDAL封装
#include <vector>
#include <algorithm>

#ifndef PI
#define PI 3.1415926534
#endif

#ifndef DllExport
#define DllExport   __declspec( dllexport)
#endif

#include "..\gdal/include/gdal_priv.h"
#pragma comment(lib,"..//GDAL//lib//gdal_i.lib")
#include "ImgProcBlock.h"

using namespace std;

//影像空域滤波
class DllExport spFilter
{
public:

	//构造函数和析构函数初始化
	spFilter(){
		m_is_core=false;
		m_Filter_core=NULL;
	}
	~spFilter(){
		if (m_is_core&&m_Filter_core!=NULL)
		{
			for (int i=0;i<m_core_ysize;i++)
			{
				delete[]m_Filter_core[i];
			}
			delete[]m_Filter_core;
		}
	}

	//设置滤波核
	void spFilterCore(float** core,int xsize,int ysize);
	void spFilterCore(float*  core,int xsize,int ysize);

	//进行滤波操作
	//GDAL封装
	void spitalFilter(char* pathSrc,char* pathDst,int xpos,int ypos,int xoffset,int yoffset);
	void spitalFilter(char* pathSrc,char* pathDst);

	//拉普拉斯算子进行滤波
	void LaplaceFilter(char* pathSrc,char* pathDst,int xpos,int ypos,int xoffset,int yoffset);
	void LaplaceFilter(char* pathSrc,char* pathDst);

	//sobel算子进行滤波
	void SobelFilter(char* pathSrc,char* pathDst,int xpos,int ypos,int xoffset,int yoffset);
	void SobelFilter(char* pathSrc,char* pathDst);

	//均值滤波
	void MeanFilter(char* pathSrc,char* pathDst,int xpos,int ypos,int xoffset,int yoffset);
	void MeanFilter(char* pathSrc,char* pathDst);

	//0值滤波
	void ZeroFilter(char* pathSrc,char* pathDst,int xpos,int ypos,int xoffset,int yoffset);
	void ZeroFilter(char* pathSrc,char* pathDst);


	//中值滤波
	void MedFilter(float*imgBufferIn,int width,int heigh,int xpos,int ypos,
		int xoffset,int yoffset,int xfsize,int yfsize,float* imgBufferOut);
	void MedFilter(char* pathSrc,char* pathDst,int xpos,int ypos,int xoffset,int yoffset);
	void MedFilter(char* pathSrc,char* pathDst);


	//滤波操作核
	void spFilterOperation(float*imgBufferIn,int width,int heigh,int xpos,int ypos,int xoffset,int yoffset,float* imgBufferOut);
private:

	//滤波操作核
	float __fastcall spFilterOperationCore(float*imgBufferIn,int xpos,int ypos);

	//成员变量
	bool m_is_core;				//滤波核是否存在
	int m_core_xsize;			//滤波核x尺寸大小
	int m_core_ysize;			//滤波核y尺寸大小
	int m_width;				//影像的width
	int m_heigh;				//影像的heigh
	float **m_Filter_core;		//影像滤波核
};


//设置高斯滤波核函数
long DllExport SetGaussCore(float **kernel,float sigma, int xsize=3,int ysize=3);
long DllExport SetGaussCore(float *kernel, float sigma, int xsize=3,int ysize=3);


//影像进行高斯滤波
long DllExport GaussFilter(float *srcImg,float *dstImg,int xImg,int yImg,float ** kernel,int xkernel=3,int ykernel=3);

//GDAL封装影像高斯滤波
long DllExport GaussFilter(char* pathSrc,char* pathDst,float** kernel, int xkernel=3, int ykernel=3);


//进行高斯差分
long DllExport GaussDifference(float* srcImg1,float* srcImg2,float* dstImg,int xImg,int yImg);

//GDAL封装的高斯差分
long DllExport GaussDifference(char* pathsrc1,char* pathsrc2,char* pathdst);


//  [2/6/2015 wuwei just you]
//TODO：影像分块进行滤波，滤波后影像分块写入文件中
//		增加多线程处理，提高运行效率
//影像分块滤波
long DllExport ImageSpaitalBlockFilter(char* pathsrc,char* pathdst,float* FilterCore,int xcoresize,int ycoresize);

//多线程影像分块滤波
long DllExport ThreadImageSpaitalFilter(char* pathsrc,char* pathdst,float* FilterCore,int xcoresize,int ycoresize);


//  [4/11/2015 wuwei just you]
//影像形态学滤波，形态学滤波本质上就是最大值或最小值滤波
//设置合适的滤波窗口和滤波核中心点位置
class DllExport MorphologyFilter
{
	//构造函数和析构函数初始化
	MorphologyFilter(){
		m_is_core=false;
		m_Filter_core=NULL;
	}
	~MorphologyFilter(){
		if (m_is_core&&m_Filter_core!=NULL)
		{
			for (int i=0;i<m_core_ysize;i++)
			{
				delete[]m_Filter_core[i];
			}
			delete[]m_Filter_core;
		}
	}

	//设置形态学滤波核
	void MorphologyCore(float** core,int xsize,int ysize,int centerx,int centery);
	void MorphologyCore(float*  core,int xsize,int ysize,int centerx,int centery);

	//形态学腐蚀
	void MorphologyCorrode(float* dataIn,int sizex,int sizey,int bands,float* dataOut);

	//形态学膨胀
	void MorphologyDilation(float* dataIn,int sizex,int sizey,int bands,float* dataOut);

	//GDAL封装的形态学腐蚀与膨胀
	//形态学腐蚀
	void MorphologyCorrode(char* pathIn,char* pathOut);

	//形态学膨胀
	void MorphologyDilation(char* pathIn,char* pathOut);

private:
	//成员变量
	bool m_is_core;				//滤波核是否存在
	int m_core_xsize;			//滤波核x尺寸大小
	int m_core_ysize;			//滤波核y尺寸大小
	int m_width;				//影像的width
	int m_heigh;				//影像的heigh
	int m_centerx;				//滤波核中心x位置
	int m_centery;				//滤波核中心y位置
	float **m_Filter_core;		//影像滤波核
};

#endif
//  [4/9/2014 wuwei just you]
//	version 1.0影像的空间域滤波和高斯滤波
