#include "stdafx.h"
#include "HyperSpectralRepair.h"
#include "HyperSpectralEndMember.h"
#include "matrixOperation.h"
#include "..\gdal/include/gdal_priv.h"
#pragma comment(lib,"..//GDAL//lib//gdal_i.lib")


//三个数中最大两个数的平均值
float MaxAvg(float data1, float data2, float data3)
{
	float m = data1;
	if (data1>data2)
		m = data2;
	if (m>data3)
		m = data3;
	if (m == data1)
		return (data2 + data3) / 2;
	else if (m == data2)
		return (data1 + data3) / 2;
	else
		return (data1 + data2) / 2;
}
//三个数中最小两个数的平均值
float MinAvg(float data1, float data2, float data3)
{
	float m = data1;
	if (data1<data2)
		m = data2;
	if (m<data3)
		m = data3;
	if (m == data1)
		return (data2 + data3) / 2;
	else if (m == data2)
		return (data1 + data3) / 2;
	else
		return (data1 + data2) / 2;
}

void HyperSpectralRepair::HyperSpecRepair_GetEdges(const unsigned char* imageMask,const int xsize,const int ysize,vector<RepairEdgePnt> &edgePnts)
{
	//获取图像边界，八方向判断，如果有一个为未知像元则为边界
	int updown[]={-1, 0 ,1};
	int leftright[]={-1,0,1};

	for(int i=0;i<xsize;++i)
	{
		for(int j=0;j<ysize;++j)
		{
			if(imageMask[j*xsize+i]!=0)
				continue;

			//判断是否是边界
			bool isEdge = false;
			//领域判断
			for (int m=0;m<3;m++)
			{
				for(int n=0;n<3;++n)
				{
					int xpos = max(min(i+leftright[m],xsize-1),0);
					int ypos = max(min(j+updown[m],ysize-1),0);
					if(imageMask[ypos*xsize+xpos]!=0)
						isEdge = true;
				}
			}
			
			if(isEdge)
			{
				RepairEdgePnt edgePnt;edgePnt.x=i;edgePnt.y=j;edgePnts.push_back(edgePnt);
			}

		}
	}
}

void HyperSpectralRepair::HyperSpecRepair_GetRepair(const unsigned char* imageMask,const int xsize,const int ysize,vector<RepairEdgePnt> &edgePnts)
{
	for(int i=0;i<xsize;++i)
	{
		for(int j=0;j<ysize;++j)
		{
			if(imageMask[j*xsize+i]==0)
			{
				RepairEdgePnt pnt;pnt.x=i;pnt.y=j;
				edgePnts.push_back(pnt);
			}
		}
	}
}

void HyperSpectralRepair::HyperSpecRepair_MakeRepairMaskRect(const char* image,const char* imgMsk,const char* imageRepair,RepairEdgePnt edgePnts[])
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO" );	//中文路径

	GDALDatasetH m_dataset    = GDALOpen(image,GA_ReadOnly);

	int xsize = GDALGetRasterXSize(m_dataset);
	int ysize = GDALGetRasterYSize(m_dataset);
	int bands = GDALGetRasterCount(m_dataset);

	float* imgData = new float[xsize*ysize];
	unsigned char* imgMask = new unsigned char[xsize*ysize];
	memset(imgMask,1,sizeof(unsigned char)*xsize*ysize);
	//for(int i=0;i<xsize*ysize;++i)
	//	imgMask[i] = 1;
	memset(imgData,0,sizeof(float)*xsize*ysize);
	int beginBand = 0, endBand = 10,curBand = 0;
	GDALDatasetH m_datasetdst = GDALCreate(GDALGetDriverByName("GTiff"),imageRepair,xsize,ysize,bands,GDT_Float32,NULL);
	GDALDatasetH m_datasetMsk = GDALCreate(GDALGetDriverByName("GTiff"),imgMsk,xsize,ysize,1,GDT_Byte,NULL);

	int minx=min(edgePnts[0].x,edgePnts[1].x),maxx=max(edgePnts[0].x,edgePnts[1].x);
	int miny=min(edgePnts[0].y,edgePnts[1].y),maxy=max(edgePnts[0].y,edgePnts[1].y);

	for(int i=0;i<bands;++i)
	{
		GDALRasterIO(GDALGetRasterBand(m_dataset,i+1),GF_Read,0,0,xsize,ysize,imgData,xsize,ysize,GDT_Float32,0,0);

		if(curBand<endBand)
		{
			for(int j=minx;j<maxx;++j)
				for(int k=miny;k<maxy;++k)
					imgData[k*xsize+j] = -1;
			curBand++;
		}
		GDALRasterIO(GDALGetRasterBand(m_datasetdst,i+1),GF_Write,0,0,xsize,ysize,imgData,xsize,ysize,GDT_Float32,0,0);
	}

	//
	for(int j=minx;j<maxx;++j)
		for(int k=miny;k<maxy;++k)
			imgMask[k*xsize+j] = 0;

	GDALRasterIO(GDALGetRasterBand(m_datasetMsk,1),GF_Write,0,0,xsize,ysize,imgMask,xsize,ysize,GDT_Byte,0,0);
	
	delete[]imgData;
	delete[]imgMask;

	GDALClose(m_dataset);
	GDALClose(m_datasetdst);
	GDALClose(m_datasetMsk);
}

//==============================================================================================================================
//TODO:BSBC 修复方法


//==============================================================================================================================
void HyperSpectralRepairExemplar::HyperSpecRepair_NP(float* image,unsigned char* imgMsk,int xsize,int ysize,int xpos,int ypos,float* np)
{
	RepairEdgePnt posLeft, posRight;
	bool left = false, right = false;
	int x=xpos,y=ypos;
	if (image[y*xsize + x - 1] == -1)
	{
		posLeft.x = x - 1;
		posLeft.y = y;
		left = true;
	}
	else if (image[(y - 1)*xsize + x - 1] == -1)
	{
		posLeft.x = x - 1;
		posLeft.y = y-1;
		left = true;
	}
	else if (image[(y + 1)*xsize + x - 1] == -1)
	{
		posLeft.x = x - 1;
		posLeft.y = y - 1;
		left = true;
	}
	else if (image[(y - 1)*xsize + x] == -1)
	{
		posLeft.x = x;
		posLeft.y = y - 1;
		left = true;
	}
	else if (image[(y + 1)*xsize + x ] == -1)
	{
		posLeft.x = x;
		posLeft.y = y - 1;
		left = true;
	}

	/*---------*/
	if (image[y*xsize + x + 1] == -1)
	{
		posRight.x = x - 1;
		posRight.y = y;
		right = true;
	}
	else if (image[(y - 1)*xsize + x + 1] == -1)
	{
		posRight.x = x - 1;
		posRight.y = y - 1;
		right = true;
	}
	else if (image[(y + 1)*xsize + x + 1] == -1)
	{
		posRight.x = x - 1;
		posRight.y = y - 1;
		right = true;
	}
	else if (image[(y - 1)*xsize + x] == -1)
	{
		posRight.x = x;
		posRight.y = y - 1;
		right = true;
	}
	else if (image[(y + 1)*xsize + x] == -1)
	{
		posRight.x = x;
		posRight.y = y - 1;
		right = true;
	}

	//然后判断
	if (!left&&!right)
	{
		np[0] = 100;
		np[1] = 100;
	}
	else if(left&&!right)
	{
		np[0] = -double(y - posLeft.y)/sqrt(double((x - posLeft.x)*(x - posLeft.x)) + double((y - posLeft.y)*(y - posLeft.y)));
		np[1] = -double(x - posLeft.x)/sqrt(double((x - posLeft.x)*(x - posLeft.x)) + double((y - posLeft.y)*(y - posLeft.y)));
	}
	else if (!left&&right)
	{
		np[0] = -double(y - posRight.y) / sqrt(double((x - posRight.x)*(x - posRight.x)) + double((y - posRight.y)*(y - posRight.y)));
		np[1] = -double(x - posRight.x) / sqrt(double((x - posRight.x)*(x - posRight.x)) + double((y - posRight.y)*(y - posRight.y)));
	}
	else
	{
		if (posLeft.x == posRight.x&&posLeft.y == posRight.y)
		{
			np[0] = 1;
			np[1] = 1;
		}
		else
		{
			np[0] = -double(posLeft.y - posRight.y) / sqrt(double((posLeft.x - posRight.x)*(posLeft.x - posRight.x)) + double((posLeft.y - posRight.y)*(posLeft.y - posRight.y)));
			np[1] = -double(posLeft.x - posRight.x) / sqrt(double((posLeft.x - posRight.x)*(posLeft.x - posRight.x)) + double((posLeft.y - posRight.y)*(posLeft.y - posRight.y)));
		}
	}
}

void HyperSpectralRepairExemplar::HyperSpecRepair_IP(float* image,unsigned char* imgMsk,int xsize,int ysize,int xpos,int ypos,float* ip)
{
	int x = xpos, y = ypos;
	double laplacex = 0.5*(MinAvg(MaxAvg(image[x + 2 + y*xsize], image[x + 1 + y*xsize], image[x + y*xsize]),
		max(image[x + 1 + (y + 1)*xsize], image[x + 1 + y*xsize]),
		max(image[x + 1 + (y - 1)*xsize], image[x + 1 + y*xsize]) - MinAvg(MaxAvg(image[x - 2 + y*xsize], image[x - 1 + y*xsize], image[x + y*xsize]), max(image[x - 1 + (y + 1)*xsize], image[x - 1 + y*xsize]), max(image[x - 1 + (y - 1)*xsize], image[x - 1 + y*xsize]))));
	double laplacey = 0.5*(MinAvg(MaxAvg(image[x + (y + 2)*xsize], image[x + (y + 1)*xsize], image[x + y*xsize]),
		max(image[x + 1 + (y + 1)*xsize], image[x + (y + 1)*xsize]),
		max(image[x - 1 + (y + 1)*xsize], image[x + 1 + y*xsize]) - MinAvg(MaxAvg(image[x + (y - 2)*xsize], image[x + (y - 1)*xsize], image[x + y*xsize]), max(image[x + 1 + (y - 1)*xsize], image[x - 1 + y*xsize]), max(image[x - 1 + (y - 1)*xsize], image[x - 1 + y*xsize]))));

	ip[0] = laplacex;
	ip[1] = laplacey;
}

void HyperSpectralRepairExemplar::HyperSpecRepair_RepairPoint(float* image,unsigned char* imgMsk,int xsize,int ysize,int xpos,int ypos)
{
	int templateSize = 7;
	int bestXPos=0,bestYPos=0;
	float* imgTemplate=new float[(2*templateSize+1)*(2*templateSize+1)];
	memset(imgTemplate,0,sizeof(float)*(2*templateSize+1)*(2*templateSize+1));
	HyperSpecRepair_GetTemplate(image,imgMsk,xsize,ysize,templateSize,xpos,ypos,imgTemplate);
	float dis=_MAX_MATCH_DIS_;

	//获取到了最佳匹配点
	for (int i=templateSize;i<xsize-templateSize;++i)
	{
		for(int j=templateSize;j<ysize-templateSize;++j)
		{
			float tempDis=HyperSpecRepair_MatchTemplate(image,imgMsk,xsize,ysize,templateSize,i,j,imgTemplate);
			if(tempDis<dis)
			{
				bestXPos=i;bestYPos=j;dis=tempDis;
			}
		}
	}

	//输出测试
	printf("repair point :(%d,%d)_match point :(%d,%d)\n",xpos,ypos,bestXPos,bestYPos);

	//通过最佳匹配点进行修复
	image[ypos*xsize+xpos] = image[bestYPos*xsize+bestXPos];
	delete[]imgTemplate;
}

void HyperSpectralRepairExemplar::HyperSpecRepair_RepairBlock(float* image,unsigned char* imgMsk,int xsize,int ysize,int xpos,int ypos)
{
	int templateSize = 5;
	int bestXPos=0,bestYPos=0;
	float* imgTemplate=new float[(2*templateSize+1)*(2*templateSize+1)];
	HyperSpecRepair_GetTemplate(image,imgMsk,xsize,ysize,templateSize,xpos,ypos,imgTemplate);
	float dis=_MAX_MATCH_DIS_;

	//获取到了最佳匹配点
	for (int i=templateSize;i<xsize-templateSize;++i)
	{
		for(int j=templateSize;j<ysize-templateSize;++j)
		{
			float tempDis=HyperSpecRepair_MatchTemplate(image,imgMsk,xsize,ysize,templateSize,i,j,imgTemplate);
			if(tempDis<dis)
			{
				bestXPos=i;bestYPos=j;dis=tempDis;
			}
		}
	}

	//输出测试
	printf("repair point :(%d,%d)_match point :(%d,%d)\n",xpos,ypos,bestXPos,bestYPos);

	//修复
	for (int i=bestXPos-templateSize,m=xpos-templateSize;i<bestXPos+templateSize;++i,++m)
	{
		for(int j=bestYPos-templateSize,n=ypos-templateSize;j<bestYPos+templateSize;++j,++n)
		{
			if(imgMsk[n*xsize+m]==0)
			{
				imgMsk[n*xsize+m]==1;
				image[n*xsize+m]=image[j*xsize+i];
			}
		}
	}
}

RepairEdgePnt HyperSpectralRepairExemplar::HyperSpecRepair_MaxConfPnt(float* image,unsigned char* imgMsk,int xsize,int ysize,vector<RepairEdgePnt>&edgePnts)
{
	int num_points = edgePnts.size();
	float cp=0,dp=0;
	int updown[]={-1, 0 ,1};
	int leftright[]={-1,0,1};
	int maxconfIdx=0;;
	float maxConf = 0;

	double *np = new double[2];
	double *ip = new double[2];

	for (int i=0;i<num_points;++i)
	{
		//获取点的八个领域像素
		for(int m=0;m<3;++m)
		{
			for(int n=0;n<3;++n)
			{
				if(imgMsk[(edgePnts[i].y+n)*xsize+edgePnts[i].x+m]!=0)
					cp+=1;
			}
		}
		cp=cp/8.0f;	//修复置信度

		//修复纹理强度
		



		float cpdp = cp*dp;
		if(cpdp>maxConf)
		{
			maxConf = cpdp;
			maxconfIdx = i;
		}
	}
	return edgePnts[maxconfIdx];
}

void HyperSpectralRepairExemplar::HyperSpecRepair_Iterator(float* image,unsigned char* imgMsk,int xsize,int ysize,RepairType repair_type/*=EP_POINT*/)
{
	//首先获取边界
	vector<RepairEdgePnt> m_repair_edge;

	if(repair_type==EP_POINT)
	{
		HyperSpecRepair_GetEdges(imgMsk,xsize,ysize,m_repair_edge);

		do 
		{
			for(int i=0;i<m_repair_edge.size();++i)
			{
				HyperSpecRepair_RepairPoint(image,imgMsk,xsize,ysize,m_repair_edge[i].x,m_repair_edge[i].y);
			}
			//掩膜图像处理
			for(int i=0;i<m_repair_edge.size();++i)
				imgMsk[m_repair_edge[i].y*xsize+m_repair_edge[i].x] = 1;
			m_repair_edge.clear();
			HyperSpecRepair_GetEdges(imgMsk,xsize,ysize,m_repair_edge);
		} while (!m_repair_edge.empty());
	}
	if(repair_type==OD_POINT)
	{
		HyperSpecRepair_GetRepair(imgMsk,xsize,ysize,m_repair_edge);
		for(int i=0;i<m_repair_edge.size();++i)
		{
			HyperSpecRepair_RepairPoint(image,imgMsk,xsize,ysize,m_repair_edge[i].x,m_repair_edge[i].y);
		}
		m_repair_edge.clear();
	}
	if(repair_type==OD_BLOCK)
	{
		HyperSpecRepair_GetRepair(imgMsk,xsize,ysize,m_repair_edge);
		do 
		{
			HyperSpecRepair_RepairPoint(image,imgMsk,xsize,ysize,m_repair_edge[0].x,m_repair_edge[0].y);
			m_repair_edge.clear();
			HyperSpecRepair_GetRepair(imgMsk,xsize,ysize,m_repair_edge);
		} while (!m_repair_edge.empty());
	}
}

void HyperSpectralRepairExemplar::HyperSpecRepair_Repair_OD_POINT(const char* image,const char* imgMsk,const char* imageRepair)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO" );	//中文路径

	GDALDatasetH m_dataset    = GDALOpen(image,GA_ReadOnly);
	GDALDatasetH m_datasetMsk = GDALOpen(imgMsk,GA_ReadOnly);

	int xsize = GDALGetRasterXSize(m_dataset);
	int ysize = GDALGetRasterYSize(m_dataset);
	int bands = GDALGetRasterCount(m_dataset);

	float* imgData = new float[xsize*ysize];
	unsigned char* imgMask = new unsigned char[xsize*ysize];
	memset(imgMask,0,sizeof(unsigned char)*xsize*ysize);
	memset(imgData,0,sizeof(float)*xsize*ysize);

	int beginBand = 0, endBand = 1,curBand = 1;
	int outBands = endBand - beginBand;
	GDALDatasetH m_datasetdst = GDALCreate(GDALGetDriverByName("GTiff"),imageRepair,xsize,ysize,outBands,GDT_Float32,NULL);
	for (int i=beginBand;i<endBand;++i,++curBand)
	{
		GDALRasterIO(GDALGetRasterBand(m_dataset,i+1),GF_Read,0,0,xsize,ysize,imgData,xsize,ysize,GDT_Float32,0,0);
		HyperSpecRepair_Iterator(imgData,imgMask,xsize,ysize,OD_POINT);
		GDALRasterIO(GDALGetRasterBand(m_datasetdst,curBand),GF_Write,0,0,xsize,ysize,imgMask,xsize,ysize,GDT_Float32,0,0);
	}

	delete[]imgData;
	delete[]imgMask;

	GDALClose(m_dataset);
	GDALClose(m_datasetdst);
	GDALClose(m_datasetMsk);
}
void HyperSpectralRepairExemplar::HyperSpecRepair_Repair_OD_BLOCK(const char* image,const char* imgMsk,const char* imageRepair)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO" );	//中文路径

	GDALDatasetH m_dataset    = GDALOpen(image,GA_ReadOnly);
	GDALDatasetH m_datasetMsk = GDALOpen(imgMsk,GA_ReadOnly);

	int xsize = GDALGetRasterXSize(m_dataset);
	int ysize = GDALGetRasterYSize(m_dataset);
	int bands = GDALGetRasterCount(m_dataset);

	float* imgData = new float[xsize*ysize];
	unsigned char* imgMask = new unsigned char[xsize*ysize];
	memset(imgMask,0,sizeof(unsigned char)*xsize*ysize);
	memset(imgData,0,sizeof(float)*xsize*ysize);

	int beginBand = 0, endBand = 1,curBand = 1;
	int outBands = endBand - beginBand;
	GDALDatasetH m_datasetdst = GDALCreate(GDALGetDriverByName("GTiff"),imageRepair,xsize,ysize,outBands,GDT_Float32,NULL);
	for (int i=beginBand;i<endBand;++i,++curBand)
	{
		GDALRasterIO(GDALGetRasterBand(m_dataset,i+1),GF_Read,0,0,xsize,ysize,imgData,xsize,ysize,GDT_Float32,0,0);
		HyperSpecRepair_Iterator(imgData,imgMask,xsize,ysize,OD_BLOCK);
		GDALRasterIO(GDALGetRasterBand(m_datasetdst,curBand),GF_Write,0,0,xsize,ysize,imgMask,xsize,ysize,GDT_Float32,0,0);
	}

	delete[]imgData;
	delete[]imgMask;

	GDALClose(m_dataset);
	GDALClose(m_datasetdst);
	GDALClose(m_datasetMsk);
}
void HyperSpectralRepairExemplar::HyperSpecRepair_Repair_CD_POINT(const char* image,const char* imgMsk,const char* imageRepair)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO" );	//中文路径

	GDALDatasetH m_dataset    = GDALOpen(image,GA_ReadOnly);
	GDALDatasetH m_datasetMsk = GDALOpen(imgMsk,GA_ReadOnly);

	int xsize = GDALGetRasterXSize(m_dataset);
	int ysize = GDALGetRasterYSize(m_dataset);
	int bands = GDALGetRasterCount(m_dataset);

	float* imgData = new float[xsize*ysize];
	unsigned char* imgMask = new unsigned char[xsize*ysize];
	memset(imgMask,0,sizeof(unsigned char)*xsize*ysize);
	memset(imgData,0,sizeof(float)*xsize*ysize);

	int beginBand = 0, endBand = 1,curBand = 1;
	int outBands = endBand - beginBand;
	GDALDatasetH m_datasetdst = GDALCreate(GDALGetDriverByName("GTiff"),imageRepair,xsize,ysize,outBands,GDT_Float32,NULL);
	for (int i=beginBand;i<endBand;++i,++curBand)
	{
		GDALRasterIO(GDALGetRasterBand(m_dataset,i+1),GF_Read,0,0,xsize,ysize,imgData,xsize,ysize,GDT_Float32,0,0);
		HyperSpecRepair_Iterator(imgData,imgMask,xsize,ysize,CD_POINT);
		GDALRasterIO(GDALGetRasterBand(m_datasetdst,curBand),GF_Write,0,0,xsize,ysize,imgMask,xsize,ysize,GDT_Float32,0,0);
	}

	delete[]imgData;
	delete[]imgMask;

	GDALClose(m_dataset);
	GDALClose(m_datasetdst);
	GDALClose(m_datasetMsk);
}
void HyperSpectralRepairExemplar::HyperSpecRepair_Repair_CD_BLOCK(const char* image,const char* imgMsk,const char* imageRepair)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO" );	//中文路径

	GDALDatasetH m_dataset    = GDALOpen(image,GA_ReadOnly);
	GDALDatasetH m_datasetMsk = GDALOpen(imgMsk,GA_ReadOnly);

	int xsize = GDALGetRasterXSize(m_dataset);
	int ysize = GDALGetRasterYSize(m_dataset);
	int bands = GDALGetRasterCount(m_dataset);

	float* imgData = new float[xsize*ysize];
	unsigned char* imgMask = new unsigned char[xsize*ysize];
	memset(imgMask,0,sizeof(unsigned char)*xsize*ysize);
	memset(imgData,0,sizeof(float)*xsize*ysize);

	int beginBand = 0, endBand = 1,curBand = 1;
	int outBands = endBand - beginBand;
	GDALDatasetH m_datasetdst = GDALCreate(GDALGetDriverByName("GTiff"),imageRepair,xsize,ysize,outBands,GDT_Float32,NULL);
	for (int i=beginBand;i<endBand;++i,++curBand)
	{
		GDALRasterIO(GDALGetRasterBand(m_dataset,i+1),GF_Read,0,0,xsize,ysize,imgData,xsize,ysize,GDT_Float32,0,0);
		HyperSpecRepair_Iterator(imgData,imgMask,xsize,ysize,CD_BLOCK);
		GDALRasterIO(GDALGetRasterBand(m_datasetdst,curBand),GF_Write,0,0,xsize,ysize,imgMask,xsize,ysize,GDT_Float32,0,0);
	}

	delete[]imgData;
	delete[]imgMask;

	GDALClose(m_dataset);
	GDALClose(m_datasetdst);
	GDALClose(m_datasetMsk);
}
void HyperSpectralRepairExemplar::HyperSpecRepair_Repair_EP_POINT(const char* image,const char* imgMsk,const char* imageRepair)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO" );	//中文路径
	GDALDatasetH m_dataset    = GDALOpen(image,GA_ReadOnly);
	GDALDatasetH m_datasetMsk = GDALOpen(imgMsk,GA_ReadOnly);

	int xsize = GDALGetRasterXSize(m_dataset);
	int ysize = GDALGetRasterYSize(m_dataset);
	int bands = GDALGetRasterCount(m_dataset);

	float* imgData = new float[xsize*ysize];
	unsigned char* imgMask = new unsigned char[xsize*ysize];
	memset(imgMask,0,sizeof(unsigned char)*xsize*ysize);
	memset(imgData,0,sizeof(float)*xsize*ysize);

	int beginBand = 0, endBand = 1,curBand = 1;
	int outBands = endBand - beginBand;
	GDALDatasetH m_datasetdst = GDALCreate(GDALGetDriverByName("GTiff"),imageRepair,xsize,ysize,outBands,GDT_Float32,NULL);
	GDALRasterIO(GDALGetRasterBand(m_datasetMsk,1),GF_Read,0,0,xsize,ysize,imgMask,xsize,ysize,GDT_Byte,0,0);

	for (int i=beginBand;i<endBand;++i,++curBand)
	{
		GDALRasterIO(GDALGetRasterBand(m_dataset,i+1),GF_Read,0,0,xsize,ysize,imgData,xsize,ysize,GDT_Float32,0,0);
		HyperSpecRepair_Iterator(imgData,imgMask,xsize,ysize,EP_POINT);
		GDALRasterIO(GDALGetRasterBand(m_datasetdst,curBand),GF_Write,0,0,xsize,ysize,imgData,xsize,ysize,GDT_Float32,0,0);
	}

	delete[]imgData;
	delete[]imgMask;

	GDALClose(m_dataset);
	GDALClose(m_datasetdst);
	GDALClose(m_datasetMsk);
}

//图像修复接口
void HyperSpectralRepairExemplar::HyperSpecRepair_RepairImage(const char* image,const char* imgMsk,const char* imageRepair)
{
	if(m_repair_type==OD_POINT)
		HyperSpecRepair_Repair_OD_POINT(image,imgMsk,imageRepair);
	if(m_repair_type==OD_BLOCK)
		HyperSpecRepair_Repair_OD_BLOCK(image,imgMsk,imageRepair);
	if(m_repair_type==CD_POINT)
		HyperSpecRepair_Repair_CD_POINT(image,imgMsk,imageRepair);
	if(m_repair_type==CD_BLOCK)
		HyperSpecRepair_Repair_CD_BLOCK(image,imgMsk,imageRepair);
	if(m_repair_type==EP_POINT)
		HyperSpecRepair_Repair_EP_POINT(image,imgMsk,imageRepair);
}
void HyperSpectralRepairExemplar::HyperSpecRepair_SetMethod(RepairType repair_type/*=EP_POINT*/)
{
	m_repair_type = repair_type;
}
//测试用例
void HyperSpectralRepairExemplar::HyperSpecRepair_TestExample()
{
	//char* pathImage = "E:\\测试数据\\影像修复\\普通图像\\Lighthouse";
	//char* pathMask  = "E:\\测试数据\\影像修复\\普通图像\\LighthouseMsk.tif";
	//char* pathSimu  = "E:\\测试数据\\影像修复\\普通图像\\LighthouseSimu.tif";
	//char* pathRepair= "E:\\测试数据\\影像修复\\普通图像\\LighthouseRepair.tif";

	//char* pathImage = "E:\\测试数据\\影像修复\\tm影像\\can_tmr.img";
	//char* pathMask  = "E:\\测试数据\\影像修复\\tm影像\\can_tmrMsk.tif";
	//char* pathSimu  = "E:\\测试数据\\影像修复\\tm影像\\can_tmrSimu.tif";
	//char* pathRepair= "E:\\测试数据\\影像修复\\tm影像\\can_tmrRepair.tif";

	char* pathImage = "E:\\测试数据\\影像修复\\高光谱影像\\cup95eff.int";
	char* pathMask  = "E:\\测试数据\\影像修复\\高光谱影像\\cup95effMsk.tif";
	char* pathSimu  = "E:\\测试数据\\影像修复\\高光谱影像\\cup95effSimu.tif";
	char* pathRepair= "E:\\测试数据\\影像修复\\高光谱影像\\cup95effRepair.tif";

	RepairEdgePnt pnt[2];
	pnt[0].x = 120;pnt[0].y = 150;
	pnt[1].x = 140;pnt[1].y = 180;

	HyperSpecRepair_MakeRepairMaskRect(pathImage,pathMask,pathSimu,pnt);
	HyperSpecRepair_SetMethod();
	HyperSpecRepair_Repair_EP_POINT(pathSimu,pathMask,pathRepair);
}

//========================================================================================================================
void HyperSpectralRepairSpecral::HyperSpecRepair_PartSpectralMacth(float* image,unsigned char* imgMsk,int xsize,int ysize,int bands,RepairEdgePnt &pntRepair,vector<RepairEdgePnt> &pntMatches, float conf/* = 0.8*/)
{
	int begRepair=0,endRepair = 10;
	int correctRepair = bands-(endRepair - begRepair+1);
	float* repairSpectral = new float[correctRepair];
	
	//获取光谱
	for(int i=0,j=0;i<bands;++i)
	{
		if(i<begRepair||i>endRepair)
		{
			repairSpectral[j] = image[i*xsize*ysize+pntRepair.y*xsize+pntRepair.x];
			++j;
		}
	}

	//匹配
	for (int i=0;i<xsize;++i)
	{
		for(int j=0;j<ysize;++j)
		{
			float matchConf = 0;
			double tmpab=0,tmp_aa=0,tmp_bb=0;
			if(imgMsk[j*xsize+i]==0)
				continue;
			for(int k=0,l=0;k<bands;++k)
			{
				if(k<begRepair||k>endRepair)
				{
					//tmpab+= repairSpectral[l]*image[k*xsize*ysize+j*xsize+i];
					//tmp_aa+= repairSpectral[l]*repairSpectral[l];
					//tmp_bb+=image[k*xsize*ysize+j*xsize+i]*image[k*xsize*ysize+j*xsize+i];
					tmpab+=fabs(repairSpectral[l]-image[k*xsize*ysize+j*xsize+i])/correctRepair;
					++l;
				}
			}
			//matchConf = (tmpab/sqrt(tmp_aa)/sqrt(tmp_bb));
			matchConf = tmpab;
			if(matchConf<conf)
			{
				RepairEdgePnt p;p.x=i;p.y=j;
				pntMatches.push_back(p);
			}
		}
	}
	delete[]repairSpectral;
}
void HyperSpectralRepairSpecral::HyperSpecRepair_PartRepairPer(float* image,int xsize,int ysize,int bands,RepairEdgePnt &pntRepair,vector<RepairEdgePnt> &pntMatches)
{
	//这个本应该作为参数，但是觉得麻烦暂时就这样 需要的时候改
	//TODO:
	int begRepair=0,endRepair = 10;
	int correctRepair = bands-(endRepair - begRepair+1);

	//修复 加权权重首先给均值
	int matchNum = pntMatches.size();
	for (int j=0;j<bands;++j)
	{
		if(j>=begRepair&&j<=endRepair)
		{
			image[j*xsize*ysize+pntRepair.y*xsize+pntRepair.x]  = 0;
			for (int i=0;i<matchNum;++i)
				image[j*xsize*ysize+pntRepair.y*xsize+pntRepair.x]+=image[j*xsize*ysize+pntMatches[i].y*xsize+pntMatches[i].x]/float(matchNum);
		}
	}
}

void HyperSpectralRepairSpecral::HyperSpecRepair_SpectralRepair(float* image,unsigned char* imgMsk,int xsize,int ysize,int bands)
{
	vector<RepairEdgePnt> _repair_image;
	vector<RepairEdgePnt> _matcher_pnts;
	HyperSpecRepair_GetRepair(imgMsk,xsize,ysize,_repair_image);

	//进行修复
	for (int i=0;i<_repair_image.size();++i)
	{
		HyperSpecRepair_PartSpectralMacth(image,imgMsk,xsize,ysize,bands,_repair_image[i],_matcher_pnts,5);
		HyperSpecRepair_PartRepairPer(image,xsize,ysize,bands,_repair_image[i],_matcher_pnts);
		//输出测试
		printf("repair point :(%d,%d) match points: %d\n",_repair_image[i].x,_repair_image[i].y,_matcher_pnts.size());
		_matcher_pnts.clear();

	}
}

//端元丰度进行重建
void HyperSpectralRepairSpecral::HyperSpecRepair_SpectralAbundance(const char* pathImg,const char* pathEnd,const char* pathRed,int bands,int endNumbers,int begRepair,int endRepair)
{
	int correctBand = bands-(endRepair-begRepair+1);
	int* bandIdx = new int[correctBand];

	for(int i=0,j=0;i<bands;++i)
	{
		if(i<begRepair||i>endRepair)
		{
			bandIdx[j]=i;++j;
		}
	}

	//求解丰度
	HyperSpectralEndMember extractEnd;
	extractEnd.HyperSpectralEnd_UnmixLSE_SetBands(pathEnd,pathImg,pathRed,endNumbers,bandIdx,correctBand);
	char* pathResidual = "res.tif";
	extractEnd.HyperSpectralEnd_Residual(pathImg,pathEnd,pathRed,pathResidual,15);
	delete[]bandIdx;
}
void HyperSpectralRepairSpecral::HyperSpecRepair_AbundanceRestruct(float* image,unsigned char* imgMsk,float* endMember,const char* pathRed,int xsize,int ysize,int bands,int endNumber)
{
	//进行修复
	float *img = new float[bands];
	float *tAbund = new float [endNumber];

	int begRepair=0,endRepair = 10;
	int correctRepair = bands-(endRepair - begRepair+1);

	//获取影像丰度
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO" );	//中文路径
	GDALDatasetH m_datasetRed = GDALOpen(pathRed,GA_ReadOnly);
	float* abundance=new float[xsize*ysize*endNumber];
	for(int i=0;i<endNumber;++i)
		GDALRasterIO(GDALGetRasterBand(m_datasetRed,i+1),GF_Read,0,0,xsize,ysize,abundance+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	GDALClose(m_datasetRed);

	for (int i=0;i<xsize;++i)
	{
		for(int j=0;j<ysize;++j)
		{
			if(imgMsk[j*xsize+i]==0)
			{
				for(int k=0;k<endNumber;++k)
					tAbund[k]=abundance[k*xsize*ysize+j*xsize+i];
				printf("repair point :(%d,%d)\n",i,j);
				MatrixMuti(tAbund,1,endNumber,bands,endMember,img);
				for(int k = begRepair; k <endRepair ;++k)
					image[k*xsize*ysize+j*xsize+i] = img[k];
			}
		}
	}
	delete[]img;img=NULL;
	delete[]tAbund;tAbund=NULL;
	delete[]abundance;abundance=NULL;
}


void HyperSpectralRepairSpecral::HyperSpecRepair_SpecMatchRepair(const char* pathImage,const char* pathImgMsk,const char* pathDst)
{
	//获取影像数据
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO" );	//中文路径

	GDALDatasetH m_datasetImg = GDALOpen(pathImage,GA_ReadOnly);
	GDALDatasetH m_datasetMsk = GDALOpen(pathImgMsk,GA_ReadOnly);

	int xsize = GDALGetRasterXSize(m_datasetImg);
	int ysize = GDALGetRasterYSize(m_datasetImg);
	int bands = GDALGetRasterCount(m_datasetImg);

	float* image = new float[xsize*ysize*bands];
	unsigned char* imgMsk=new unsigned char[xsize*ysize];
	for(int i=0;i<bands;++i)
		GDALRasterIO(GDALGetRasterBand(m_datasetImg,i+1),GF_Read,0,0,xsize,ysize,image+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	GDALRasterIO(GDALGetRasterBand(m_datasetMsk,1),GF_Read,0,0,xsize,ysize,imgMsk,xsize,ysize,GDT_Byte,0,0);
	//创建输出影像
	GDALDatasetH m_datasetdst = GDALCreate(GDALGetDriverByName("GTiff"),pathDst,xsize,ysize,bands,GDT_Float32,NULL);
	HyperSpecRepair_SpectralRepair(image,imgMsk,xsize,ysize,bands);
	for(int i=0;i<bands;++i)
		GDALRasterIO(GDALGetRasterBand(m_datasetdst,i+1),GF_Write,0,0,xsize,ysize,image+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	
	GDALClose(m_datasetdst);
	GDALClose(m_datasetMsk);
	GDALClose(m_datasetImg);
	delete[]image;image=NULL;
	delete[]imgMsk;imgMsk=NULL;
}
void HyperSpectralRepairSpecral::HyperSpecRepair_EndMemberRepair(const char* pathImg, const char* pathEnd,const char* pathMsk,const char* pathDst,int bands,int endNumber)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO" );	//中文路径
	int begRepair=0,endRepair = 10;
	char* pathTempRed = "~tempRed.tif";
	HyperSpecRepair_SpectralAbundance(pathImg,pathEnd,pathTempRed,bands,endNumber,begRepair,endRepair);
	
	GDALDatasetH m_dataset = GDALOpen(pathImg,GA_ReadOnly);
	GDALDatasetH m_datasetMsk = GDALOpen(pathMsk,GA_ReadOnly);
	int xsize = GDALGetRasterXSize(m_dataset);
	int ysize = GDALGetRasterYSize(m_dataset);
	bands = GDALGetRasterCount(m_dataset);
	float* imgData = new float [xsize*xsize*bands];
	float* endMemberData = new float[bands*endNumber];
	unsigned char* imgMsk = new unsigned char[xsize*ysize];

	//获取影像和端元
	HyperSpectralEndMember m_hyperEnd;
  	m_hyperEnd.HyperSpectralEnd_ImportEndmember(pathEnd,bands,endNumber,endMemberData);
	for (int i=0;i<bands;++i)
		GDALRasterIO(GDALGetRasterBand(m_dataset,i+1),GF_Read,0,0,xsize,ysize,imgData+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	GDALRasterIO(GDALGetRasterBand(m_datasetMsk,1),GF_Read,0,0,xsize,ysize,imgMsk,xsize,ysize,GDT_Byte,0,0);
	HyperSpecRepair_AbundanceRestruct(imgData,imgMsk,endMemberData,pathTempRed,xsize,ysize,bands,endNumber);

	GDALDatasetH m_datasetDst = GDALCreate(GDALGetDriverByName("GTiff"),pathDst,xsize,ysize,bands,GDT_Float32,NULL);
	for (int i=0;i<bands;++i)
		GDALRasterIO(GDALGetRasterBand(m_datasetDst,i+1),GF_Write,0,0,xsize,ysize,imgData+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	GDALClose(m_dataset);
	GDALClose(m_datasetMsk);
	GDALClose(m_datasetDst);
	
	HyperSpectralEndMember m_spectral;
	char* pathTempRes = "~image_residual.tif";
	m_spectral.HyperSpectralEnd_Residual(pathImg,pathEnd,pathTempRed,pathTempRes,endNumber);

	//删除临时文件
	//remove(pathTempRed);
	//remove(pathTempRes);

	delete[]imgData;imgData=NULL;
	delete[]endMemberData;endMemberData=NULL;
	delete[]imgMsk;imgMsk=NULL;
}

void HyperSpectralRepairSpecral::HyperSpecRepair_TestExample()
{
	//测试用例
	char* pathImage = "E:\\测试数据\\影像修复\\高光谱影像\\cup95eff.int";
	char* pathMask  = "E:\\测试数据\\影像修复\\高光谱影像\\cup95effMsk.tif";
	char* pathSimu  = "E:\\测试数据\\影像修复\\高光谱影像\\cup95effSimu.tif";
	char* pathEnd   = "E:\\测试数据\\影像修复\\高光谱影像\\endMemberSpectral.txt";
	char* pathRepair= "E:\\测试数据\\影像修复\\高光谱影像\\cup95effRepairEnd.tif";
	char* pathRed	= "E:\\测试数据\\影像修复\\高光谱影像\\cup95effRepairRed.tif";

	RepairEdgePnt pnt[2];
	pnt[0].x = 120;pnt[0].y = 150;
	pnt[1].x = 140;pnt[1].y = 180;
	HyperSpectralEndMember m_endMember;
	//m_endMember.HyperSpectralEnd_UnmixLSE(pathEnd,pathImage,pathRed,15);

	//HyperSpecRepair_MakeRepairMaskRect(pathImage,pathMask,pathSimu,pnt);
	//HyperSpecRepair_SpecMatchRepair(pathSimu,pathMask,pathRepair);
	HyperSpecRepair_EndMemberRepair(pathSimu,pathEnd,pathMask,pathRepair,50,15);
}