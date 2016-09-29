#include "StdAfx.h"
#include "MosaicProcess.h"
#include "gdal/include/gdal_priv.h"
#include "AerialCore.h"
#include "AuxiliaryFunction.h"
#include "matrixOperation.h"
#include <algorithm>
#include <iostream>
using namespace std;

void TransverseMercator(double pnt[],double rmat[], double t[],double scale,double* tpnt)
{
	tpnt[0] = rmat[0]*pnt[0]+rmat[1]*pnt[1]+rmat[2]*pnt[2];
	tpnt[1] = rmat[3]*pnt[0]+rmat[4]*pnt[1]+rmat[5]*pnt[2];
	tpnt[2] = rmat[6]*pnt[0]+rmat[7]*pnt[1]+rmat[8]*pnt[2];

	float u_ = atan2f(tpnt[0], tpnt[2]);
	float v_ = asinf(tpnt[1] / sqrtf(tpnt[0] * tpnt[0] + tpnt[1] * tpnt[1] + tpnt[2] * tpnt[2]));
	float B = cosf(v_) * sinf(u_);

	tpnt[0] = scale / 2 * logf( (1+B) / (1-B) );
	tpnt[1] = scale * atan2f(tanf(v_), cosf(u_));
}
void Plane(double pnt[],double rmat[], double t[],double scale,double* tpnt)
{
	float x_ = rmat[0] * pnt[0] + rmat[1] * pnt[1] + rmat[2]*pnt[2];
	float y_ = rmat[3] * pnt[0] + rmat[4] * pnt[1] + rmat[5]*pnt[2];
	float z_ = rmat[6] * pnt[0] + rmat[7] * pnt[1] + rmat[8]*pnt[2];

	x_ = t[0] + x_ / z_ * (1 - t[2]);
	y_ = t[1] + y_ / z_ * (1 - t[2]);

	tpnt[0] = scale * x_;
	tpnt[1] = scale * y_;
}
//=================================================================================================================================
void MosaicProcessHomo::MosaicProc_Range(vector<string> imgList,cv::Point2f &pntMin,cv::Point2f &pntMax,vector<cv::Mat> homoList)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	pntMin.x=pntMin.y=999999;
	pntMax.x=pntMax.y=-999999;

	//opencv的方法获取转换后影像的范围
	for(int i=0;i<imgList.size();++i)
	{
		GDALDatasetH m_dataset =GDALOpen(imgList[i].c_str(),GA_ReadOnly);
		int xsize = GDALGetRasterXSize(m_dataset);
		int ysize = GDALGetRasterYSize(m_dataset);
		
		vector<cv::Point2f> pnt1(4),pnt2;
		pnt1[0].x=pnt1[0].y=0;
		pnt1[1].x=xsize;pnt1[1].y=0;
		pnt1[2].x=0;pnt1[2].y=ysize;
		pnt1[3].x=xsize;pnt1[3].y=ysize;
		cv::perspectiveTransform(pnt1,pnt2,homoList[i]);
		pntMin.x=min(min((min(pnt2[0].x,pnt2[1].x)),min(pnt2[2].x,pnt2[3].x)),pntMin.x);
		pntMin.y=min(min((min(pnt2[0].y,pnt2[1].y)),min(pnt2[2].y,pnt2[3].y)),pntMin.y);
		pntMax.x=max(max((max(pnt2[0].x,pnt2[1].x)),max(pnt2[2].x,pnt2[3].x)),pntMax.x);
		pntMax.y=max(max((max(pnt2[0].y,pnt2[1].y)),max(pnt2[2].y,pnt2[3].y)),pntMax.y);
		GDALClose(m_dataset);
	}

}

void MosaicProcessHomo::MosaicProc_Mosaic(vector<string> imgList,const char* pathOut,cv::Point2f pntMin,cv::Point2f pntMax,vector<cv::Mat> homoList)
{
	//创建图像
	int mosaicx=(pntMax.x-pntMin.x);
	int mosaicy=(pntMax.y-pntMin.y);
	unsigned char* img=new unsigned char[mosaicx*mosaicy];
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	GDALDatasetH m_datasetDst = GDALCreate(GDALGetDriverByName("GTiff"),pathOut,mosaicx,mosaicy,3,GDT_Byte,NULL);

	for(int m=0;m<3;m++)
	{
		memset(img,0,sizeof(unsigned char)*mosaicx*mosaicy);
		//opencv的方法获取转换后影像的范围
		for(int i=0;i<imgList.size();++i)
		{
			GDALDatasetH m_dataset =GDALOpen(imgList[i].c_str(),GA_ReadOnly);
			int xsize = GDALGetRasterXSize(m_dataset);
			int ysize = GDALGetRasterYSize(m_dataset);
			unsigned char* imgTemp=new unsigned char[xsize*ysize];
			GDALRasterIO(GDALGetRasterBand(m_dataset,m+1),GF_Read,0,0,xsize,ysize,imgTemp,xsize,ysize,GDT_Byte,0,0);

			double matrix[9];
			memcpy(matrix,homoList[i].data,sizeof(double)*9);
			double pnt1[3],pnt2[3];
			//每一个点的计算
			for (int j=0;j<xsize;++j)
			{
				for(int k=0;k<ysize;++k)
				{
					pnt1[0]=j;pnt1[1]=k;pnt1[2]=1;
					//MatrixMuti(matrix,3,3,1,pnt1,pnt2);
					pnt2[0] = matrix[0]*pnt1[0]+matrix[1]*pnt1[1]+matrix[2]*pnt1[2];
					pnt2[1] = matrix[3]*pnt1[0]+matrix[4]*pnt1[1]+matrix[5]*pnt1[2];
					pnt2[2] = matrix[6]*pnt1[0]+matrix[7]*pnt1[1]+matrix[8]*pnt1[2];

					pnt2[0]/=pnt2[2];
					pnt2[1]/=pnt2[2];

					int idy = (pnt2[1]-pntMin.y);
					int idx = (pnt2[0]-pntMin.x);
					if(img[idy*mosaicx+idx]==0)
						img[idy*mosaicx+idx]=imgTemp[k*xsize+j];
				}
			}

			delete[]imgTemp;imgTemp=NULL;
			GDALClose(m_dataset);
		}
		GDALRasterIO(GDALGetRasterBand(m_datasetDst,m+1),GF_Write,0,0,mosaicx,mosaicy,img,mosaicx,mosaicy,GDT_Byte,0,0);
	}
	delete[]img;img=NULL;
	GDALClose(m_datasetDst);
}

//=================================================================================================================================
void MosaicProcessRoationTranslate::MosaicProc_TransMat(cv::detail::CameraParams &camera,double* rot,double* trans)
{
	cv::Mat inv ;
	camera.K().convertTo(inv,CV_32F);
	cv::Mat r_kinv=camera.R*inv.inv();
	float *rotf,*transf;
	//rotf=new float[9];
	//transf=new float[3];

	rotf  = (float*)r_kinv.data;
	transf= (float *)camera.t.data;
	for(int i=0;i<9;++i)
		rot[i]=rotf[i];
	for(int i=0;i<3;++i)
		trans[i]=transf[i];

	//delete[]rotf;rotf=NULL;
	//delete[]transf;transf=NULL;
}

void MosaicProcessRoationTranslate::MosaicProc_RangePlane(vector<string> imgList,cv::Point2f &pntMin,cv::Point2f &pntMax,vector<cv::detail::CameraParams> &cameras)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	pntMin.x=pntMin.y=999999;
	pntMax.x=pntMax.y=-999999;

	//获取scale
	//更新单应矩阵
	vector<double> focals;
	for (size_t i = 0; i < cameras.size(); ++i)
	{
		focals.push_back(cameras[i].focal);
	}

	sort(focals.begin(), focals.end());
	float warped_image_scale;
	if (focals.size() % 2 == 1)
		warped_image_scale = static_cast<float>(focals[focals.size() / 2]);
	else
		warped_image_scale = static_cast<float>(focals[focals.size() / 2 - 1] + focals[focals.size() / 2]) * 0.5f;


	//opencv的方法获取转换后影像的范围
	for(int i=0;i<imgList.size();++i)
	{
		GDALDatasetH m_dataset =GDALOpen(imgList[i].c_str(),GA_ReadOnly);
		int xsize = GDALGetRasterXSize(m_dataset);
		int ysize = GDALGetRasterYSize(m_dataset);
		double pnt1[3],pnt2[3],pnt3[3],pnt4[3],rot[9],t[3];
		double tpnt1[3],tpnt2[3],tpnt3[3],tpnt4[3];
		MosaicProc_TransMat(cameras[i],rot,t);
		pnt1[0]=0;pnt1[1]=0;pnt1[2]=1;
		pnt2[0]=0;pnt2[1]=ysize;pnt2[2]=1;
		pnt3[0]=xsize;pnt3[1]=0;pnt3[2]=1;
		pnt4[0]=xsize;pnt4[1]=ysize;pnt4[2]=1;

		TransverseMercator(pnt1,rot,t,warped_image_scale,tpnt1);
		TransverseMercator(pnt2,rot,t,warped_image_scale,tpnt2);
		TransverseMercator(pnt3,rot,t,warped_image_scale,tpnt3);
		TransverseMercator(pnt4,rot,t,warped_image_scale,tpnt4);

		pntMin.x =min(min(min(tpnt1[0],tpnt2[0]),min(tpnt3[0],tpnt4[0])),(double)pntMin.x);
		pntMin.y =min(min(min(tpnt1[1],tpnt2[1]),min(tpnt3[1],tpnt4[1])),(double)pntMin.y);
		pntMax.x =max(max(max(tpnt1[0],tpnt2[0]),max(tpnt3[0],tpnt4[0])),(double)pntMax.x);
		pntMax.y =max(max(max(tpnt1[1],tpnt2[1]),max(tpnt3[1],tpnt4[1])),(double)pntMax.y);
		GDALClose(m_dataset);
	}


}

void MosaicProcessRoationTranslate::MosaicProc_MosaicPlane(vector<string> imgList,const char* pathOut,cv::Point2f pntMin,cv::Point2f pntMax,vector<cv::detail::CameraParams> &cameras)
{
	//创建图像
	int mosaicx=(pntMax.x-pntMin.x+1);
	int mosaicy=(pntMax.y-pntMin.y+1);
	unsigned char* img=new unsigned char[mosaicx*mosaicy];
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");
	GDALDatasetH m_datasetDst = GDALCreate(GDALGetDriverByName("GTiff"),pathOut,mosaicx,mosaicy,3,GDT_Byte,NULL);
	vector<double> focals;
	for (size_t i = 0; i < cameras.size(); ++i)
	{
		focals.push_back(cameras[i].focal);
	}

	sort(focals.begin(), focals.end());
	float warped_image_scale;
	if (focals.size() % 2 == 1)
		warped_image_scale = static_cast<float>(focals[focals.size() / 2]);
	else
		warped_image_scale = static_cast<float>(focals[focals.size() / 2 - 1] + focals[focals.size() / 2]) * 0.5f;

	for(int m=0;m<3;m++)
	{
		memset(img,0,sizeof(unsigned char)*mosaicx*mosaicy);
		//opencv的方法获取转换后影像的范围
		for(int i=0;i<imgList.size();++i)
		{
			GDALDatasetH m_dataset =GDALOpen(imgList[i].c_str(),GA_ReadOnly);
			int xsize = GDALGetRasterXSize(m_dataset);
			int ysize = GDALGetRasterYSize(m_dataset);
			//DPOINT *imgMapPoints=new DPOINT[xsize*ysize];

			unsigned char* imgTemp=new unsigned char[xsize*ysize];
			GDALRasterIO(GDALGetRasterBand(m_dataset,m+1),GF_Read,0,0,xsize,ysize,imgTemp,xsize,ysize,GDT_Byte,0,0);

			double rmatrix[9],trans[3];
			MosaicProc_TransMat(cameras[i],rmatrix,trans);
			double pnt1[3],pnt2[3];
			//每一个点的计算
			for (int j=0;j<xsize;++j)
			{
				for(int k=0;k<ysize;++k)
				{
					pnt1[0]=j;pnt1[1]=k;pnt1[2]=1;
					TransverseMercator(pnt1,rmatrix,trans,warped_image_scale,pnt2);
					int idy = (pnt2[1]-pntMin.y+1);
					int idx = (pnt2[0]-pntMin.x+1);
					if(img[idy*mosaicx+idx]==0)
						img[idy*mosaicx+idx]=imgTemp[k*xsize+j];
				}
			}
			GDALClose(m_dataset);
			delete[]imgTemp;imgTemp=NULL;
		}
		GDALRasterIO(GDALGetRasterBand(m_datasetDst,m+1),GF_Write,0,0,mosaicx,mosaicy,img,mosaicx,mosaicy,GDT_Byte,0,0);
	}
	delete[]img;img=NULL;
	GDALClose(m_datasetDst);
}
