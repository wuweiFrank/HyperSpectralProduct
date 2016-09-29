#include "stdafx.h"
#include "HyperSpectralEndMember.h"
#include <fstream>
#include <ctime>
#include "..\gdal/include/gdal_priv.h"
#include "..\ImgProcLibrary\ImgProcSample.h"
#include "matrixOperation.h"
#pragma comment(lib,"..//GDAL//lib//gdal_i.lib")
#pragma comment(lib,"..//Debug//ImgProcLibrary.lib")
using namespace std;

#ifndef PI
#define PI  3.1415926535898
#endif


//获取光谱库
bool HyperSpectralEndMember::HyperSpectralEnd_ImportEndmember(const char* pathEnd,int bands,int endnumbers,float* endmember)
{
	ifstream ifs;
	ifs.open(pathEnd,ios_base::in);
	if(ifs.is_open()!=true)
		return false;
	char tempchr[2048];
	for(int i=0;i<endnumbers+2;i++)
		ifs.getline(tempchr,2048);

	printf("获取光谱库...\n");
	for (int i=0;i<bands;i++)
	{
		float temp;
		ifs>>temp;
		for (int j=0;j<endnumbers;j++)
		{
			ifs>>temp;
			endmember[i*endnumbers+j]=temp;
		}
	}

	return true;
}
bool HyperSpectralEndMember::HyperSpectralEnd_ImportEndmember(const char* pathEnd,int bands,int endnumbers,double* endmember)
{
	ifstream ifs;
	ifs.open(pathEnd,ios_base::in);
	if(ifs.is_open()!=true)
		return false;
	char tempchr[2048];
	for(int i=0;i<endnumbers+2;i++)
		ifs.getline(tempchr,2048);

	printf("获取光谱库...\n");
	for (int i=0;i<bands;i++)
	{
		float temp;
		ifs>>temp;
		for (int j=0;j<endnumbers;j++)
		{
			ifs>>temp;
			endmember[i*endnumbers+j]=temp;
		}
	}

	return true;
}

//导出光谱库（ENVI格式方便查看）
void HyperSpectralEndMember::HyperSpectralEnd_ExportEndmember(const char* path,float *enddata,int bands,int endnumbers)
{
	ofstream ofs(path,ios_base::out);
	if(ofs.fail()==true)
	{
		printf("打开输出文件失败！");
		return ;
	}

	//输出
	time_t curtime;
	curtime=time(NULL);
	ofs<<"ENVI ASCII Plot File:";
	ofs<<ctime(&curtime);
	ofs<<"Column 1 Wavelength"<<endl;
	for (int i=0;i<endnumbers;i++)
		ofs<<"Column "<<i+1<<": Unknown "<<endl;

	//数据输出
	for (int i=0;i<bands;i++)
	{
		ofs<<i+1<<" ";
		for (int j=0;j<endnumbers;j++)
		{
			ofs<<enddata[i*endnumbers+j]<<" ";
		}
		ofs<<endl;
	}

	ofs.close();

}
void HyperSpectralEndMember::HyperSpectralEnd_ExportEndmember(const char* path,double *enddata,int bands,int endnumbers)
{
	ofstream ofs(path,ios_base::out);
	if(ofs.fail()==true)
	{
		printf("打开输出文件失败！");
		return ;
	}

	//输出
	time_t curtime;
	curtime=time(NULL);
	ofs<<"ENVI ASCII Plot File:";
	ofs<<ctime(&curtime);
	ofs<<"Column 1 Wavelength"<<endl;
	for (int i=0;i<endnumbers;i++)
		ofs<<"Column "<<i+1<<": Unknown "<<endl;

	//数据输出
	for (int i=0;i<bands;i++)
	{
		ofs<<i+1<<" ";
		for (int j=0;j<endnumbers;j++)
		{
			ofs<<enddata[i*endnumbers+j]<<" ";
		}
		ofs<<endl;
	}

	ofs.close();

}

//最小二乘光谱解混
void HyperSpectralEndMember::HyperSpectralEnd_UnmixLSE(const char* pathEnd,char* pathImg,const char* pathRed,int endnumbers)
{
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");	//中文路径

	GDALDatasetH m_dataset;
	m_dataset=GDALOpen(pathImg,GA_ReadOnly);
	int xsize=GDALGetRasterXSize(m_dataset);
	int ysize=GDALGetRasterYSize(m_dataset);
	int bands=GDALGetRasterCount(m_dataset);

	ofstream ofs;
	ofs.open("test.txt");

	if (endnumbers>bands)
	{
		//无法求解
		printf("端元数大于波段个数，无法进行最小二乘求解！\n");
		return;
	}

	float *imgData=new float[xsize*ysize*bands];
	float *endData=new float[bands*endnumbers];

	//获取端元光谱
	HyperSpectralEnd_ImportEndmember(pathEnd,bands,endnumbers,endData);

	float *dendData=new float[bands*endnumbers];
	//端元数据按比例缩小
	for (int i=0;i<endnumbers*bands;i++)
		dendData[i]=endData[i]/*/10.0f*/;

	//获取影像数据
	for (int i=0;i<bands;i++)
		GDALRasterIO(GDALGetRasterBand(m_dataset,i+1),GF_Read,0,0,xsize,ysize,imgData+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	GDALClose(m_dataset);

	//最小二乘解算输出
	float* data =new float[bands];
	float* datamuti=new float[endnumbers];

	float* res  =new float[endnumbers];
	float* endTrans=new float[endnumbers*bands];
	float* endMuti =new float[endnumbers*endnumbers];
	float* endInv  =new float[endnumbers*endnumbers];
	float* resImgae=new float[endnumbers*xsize*ysize];


	//将结果输出为影像算了......
	for (int i=0;i<xsize*ysize;i++)
	{
		for(int j=0;j<bands;j++)
			data[j]=imgData[j*xsize*ysize+i];

		//矩阵运算
		MatrixTrans(dendData,bands,endnumbers,endTrans);
		MatrixMuti(endTrans,endnumbers,bands,endnumbers,dendData,endMuti);
		MatrixInverse(endMuti,endnumbers,endInv);
		MatrixMuti(endTrans,endnumbers,bands,1,data,datamuti);
		MatrixMuti(endInv,endnumbers,endnumbers,1,datamuti,res);

		//输出求解结果
		printf("\r最小二乘计算丰度处理像素：%d/%d",xsize*ysize,i+1);

		//结果输出为影像
		for (int j=0;j<endnumbers;j++)
			resImgae[j*xsize*ysize+i]=res[j];

	}
	printf("\n finished...\n");

	GDALDatasetH m_datasetout;
	m_datasetout=GDALCreate(GDALGetDriverByName("GTiff"),pathRed,xsize,ysize,endnumbers,GDT_Float32,NULL);
	for (int i=0;i<endnumbers;i++)
		GDALRasterIO(GDALGetRasterBand(m_datasetout,i+1),GF_Write,0,0,xsize,ysize,resImgae+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	GDALClose(m_datasetout);

	//清除数据
	delete[]imgData;imgData=NULL;
	delete[]endData;endData=NULL;

	delete[]data;   data=NULL;
	delete[]res;	res=NULL;
	delete[]endTrans;endTrans=NULL;
	delete[]endMuti;endMuti=NULL;
	delete[]endInv; endInv=NULL;
	delete[]resImgae;resImgae=NULL;
	delete[]dendData;dendData=NULL;
	delete[]datamuti;datamuti=NULL;

	return ;
}
void HyperSpectralEndMember::HyperSpectralEnd_UnmixLSE_SetBands(const char* pathEnd,const char* pathImg,const char* pathRed,int endnumbers,int* bandIdx,int bandIdxNum)
{
	GDALAllRegister();
	CPLSetConfigOption( "GDAL_FILENAME_IS_UTF8", "NO" );	//中文路径

	GDALDatasetH m_dataset;
	m_dataset=GDALOpen(pathImg,GA_ReadOnly);
	int xsize=GDALGetRasterXSize(m_dataset);
	int ysize=GDALGetRasterYSize(m_dataset);
	int bands=GDALGetRasterCount(m_dataset);

	ofstream ofs;
	ofs.open("test.txt");

	if (endnumbers>bandIdxNum)
	{
		//无法求解
		printf("端元数大于波段个数，无法进行最小二乘求解！\n");
		return;
	}

	//数据空间申请
	float *imgData=new float[xsize*ysize*bandIdxNum];
	float *endData=new float[bands*endnumbers];
	float *endDataIdx = new float[bandIdxNum*endnumbers];


	HyperSpectralEnd_ImportEndmember(pathEnd,bands,endnumbers,endData);
	for(int i=0;i<bandIdxNum;++i)
		for(int j=0;j<endnumbers;++j)
			endDataIdx[endnumbers*i+j]=endData[bandIdx[i]*endnumbers+j]/*/10.0f*/;

	//获取端元光谱
	HyperSpectralEnd_ImportEndmember(pathEnd,bands,endnumbers,endData);
	//获取影像数据
	for (int i=0;i<bandIdxNum;i++)
		GDALRasterIO(GDALGetRasterBand(m_dataset,bandIdx[i]+1),GF_Read,0,0,xsize,ysize,imgData+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	GDALClose(m_dataset);

	//最小二乘解算输出
	float* data    =new float[bandIdxNum];
	float* datamuti=new float[endnumbers];

	float* res  =new float[endnumbers];
	float* endTrans=new float[endnumbers*bandIdxNum];
	float* endMuti =new float[endnumbers*endnumbers];
	float* endInv  =new float[endnumbers*endnumbers];
	float* resImgae=new float[endnumbers*xsize*ysize];

	//将结果输出为影像算了......
	for (int i=0;i<xsize*ysize;i++)
	{
		for(int j=0;j<bandIdxNum;j++)
			data[j]=imgData[j*xsize*ysize+i];

		//矩阵运算
		MatrixTrans(endDataIdx,bandIdxNum,endnumbers,endTrans);
		MatrixMuti(endTrans,endnumbers,bandIdxNum,endnumbers,endDataIdx,endMuti);
		MatrixInverse(endMuti,endnumbers,endInv);
		MatrixMuti(endTrans,endnumbers,bandIdxNum,1,data,datamuti);
		MatrixMuti(endInv,endnumbers,endnumbers,1,datamuti,res);

		//输出求解结果
		printf("\r最小二乘计算丰度处理像素：%d/%d",xsize*ysize,i+1);

		//结果输出为影像
		for (int j=0;j<endnumbers;j++)
			resImgae[j*xsize*ysize+i]=res[j];

	}
	printf("\n finished...\n");

	GDALDatasetH m_datasetout;
	m_datasetout=GDALCreate(GDALGetDriverByName("GTiff"),pathRed,xsize,ysize,endnumbers,GDT_Float32,NULL);
	for (int i=0;i<endnumbers;i++)
		GDALRasterIO(GDALGetRasterBand(m_datasetout,i+1),GF_Write,0,0,xsize,ysize,resImgae+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	GDALClose(m_datasetout);

	//清除数据
	delete[]imgData;imgData=NULL;
	delete[]endData;endData=NULL;

	delete[]data;   data=NULL;
	delete[]res;	res=NULL;
	delete[]endTrans;endTrans=NULL;
	delete[]endMuti;endMuti=NULL;
	delete[]endInv; endInv=NULL;
	delete[]resImgae;resImgae=NULL;
	delete[]endDataIdx;endDataIdx=NULL;
	delete[]datamuti;datamuti=NULL;

	return ;
}


//计算解混残差
void HyperSpectralEndMember::HyperSpectralEnd_Residual(const char* pathimg,const char* pathend,const char* pathred,const char* pathres,int endnumbers)
{
	GDALAllRegister();
	GDALDatasetH m_dataset   =GDALOpen(pathred,GA_ReadOnly);
	GDALDatasetH m_datasetimg=GDALOpen(pathimg,GA_ReadOnly);
	int xsize=GDALGetRasterXSize(m_datasetimg);
	int ysize=GDALGetRasterYSize(m_datasetimg);
	int bands=GDALGetRasterCount(m_datasetimg);

	float* datared=new float[xsize*ysize*bands];
	float* dataimg=new float[xsize*ysize*bands];
	float* dataend=new float[bands*endnumbers];
	float* dataout=new float[(bands+1)*xsize*ysize];
	memset(dataout,0,sizeof(float)*xsize*ysize*(bands+1));

	for(int i=0;i<bands;i++)
		GDALRasterIO(GDALGetRasterBand(m_dataset,i+1),GF_Read,0,0,xsize,ysize,datared+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	for(int i=0;i<bands;i++)
		GDALRasterIO(GDALGetRasterBand(m_datasetimg,i+1),GF_Read,0,0,xsize,ysize,dataimg+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	HyperSpectralEnd_ImportEndmember(pathend,bands,endnumbers,dataend);
	GDALClose(m_dataset);
	GDALClose(m_datasetimg);

	for (int i=0;i<xsize*ysize;i++)
	{
		for (int j=0;j<bands;j++)
			for(int k=0;k<endnumbers;k++)
				dataout[j*xsize*ysize+i]+=datared[k*xsize*ysize+i]*dataend[j*endnumbers+k];

		//最后一个波段为总的残差
		for (int j=0;j<bands;j++)
			dataout[(bands)*xsize*ysize+i]+=dataout[j*xsize*ysize+i]-dataimg[j*xsize*ysize+i];
	}

	GDALDatasetH m_datasetout=GDALCreate(GDALGetDriverByName("GTiff"),pathres,xsize,ysize,bands+1,GDT_Float32,NULL);
	for(int i=0;i<bands+1;i++)
		GDALRasterIO(GDALGetRasterBand(m_datasetout,i+1),GF_Write,0,0,xsize,ysize,dataout+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	GDALClose(m_datasetout);

	delete[]dataend;
	delete[]dataimg;
	delete[]dataout;
	delete[]datared;
}

//求解出来的丰度进行求和(理论上丰度和应该等于1)
void HyperSpectralEndMember::HyperSpectralEnd_ReduTotal(const char* pathRed,const char* pathTotal)
{
	GDALAllRegister();
	GDALDatasetH datasetred=GDALOpen(pathRed,GA_ReadOnly);

	int xsize=GDALGetRasterXSize(datasetred);
	int ysize=GDALGetRasterYSize(datasetred);
	int bands=GDALGetRasterCount(datasetred);
	char **papszOptions = NULL;  
	papszOptions = CSLSetNameValue( papszOptions, "INTERLEAVE", "BAND" ); 
	GDALDatasetH datasettotal=GDALCreate(GDALGetDriverByName("GTiff"),pathTotal,xsize,ysize,1,GDT_Float32,papszOptions);
	//数据空间的申请
	float *datared  =new float[xsize*ysize];memset(datared  ,0,sizeof(float)*xsize*ysize);
	float *datatotal=new float[xsize*ysize];memset(datatotal,0,sizeof(float)*xsize*ysize);

	for (int i=0;i<bands;i++)
	{
		printf("\r波段求和%d",i+1);
		GDALRasterIO(GDALGetRasterBand(datasetred,i+1),GF_Read,0,0,xsize,ysize,datared,xsize,ysize,GDT_Float32,0,0);
		for(int j=0;j<xsize*ysize;j++)
			datatotal[j]+=datared[j];
	}
	printf("\n");
	GDALRasterIO(GDALGetRasterBand(datasettotal,1),GF_Write,0,0,xsize,ysize,datatotal,xsize,ysize,GDT_Float32,0,0);
	GDALClose(datasetred);
	GDALClose(datasettotal);
	delete[]datared;
	delete[]datatotal;
}

//通过光谱响应函数进行光谱响应采样
//光谱响应函数
float CalculateR(float wavelength,float cntlength,float stddev)
{
	float r=0;
	r=(1/(stddev*sqrt(2*PI)))*exp(-((wavelength-cntlength)*(wavelength-cntlength))/(2*stddev*stddev));
	return r;
}
void  Gauss_Response(float* dataHy,float *hyLength,float mLength,float &dataM,int num)
{
	float stddev=0.05f;
	float r=0;
	float updata=0;
	float rdown=0;
	float tempdata=0;
	for (int j=0;j<num;j++)
	{
		r=CalculateR(hyLength[j],mLength,stddev);
		updata+=dataHy[j]*r;
		rdown+=r;
	}
	dataM=updata/rdown;
}
void HyperSpectralEndMember::HyperSpectralEnd_Response(const char* pathHy,const char* pathM,float* hysp,int bandsHy,float* msp,int bandm,int endnumber,bool range)
{
	//高光谱和多光谱端元的空间申请
	float *endHyData=new float[bandsHy*endnumber];
	float *endMsData=new float[bandm*endnumber];
	memset(endHyData,0,sizeof(float)*bandsHy*endnumber);
	memset(endMsData,0,sizeof(float)*bandm*endnumber);

	HyperSpectralEnd_ImportEndmember(pathHy,bandsHy,endnumber,endHyData);

	//无输入波段波长的情况下
	if(hysp==NULL&&msp==NULL)
	{
		//直接对应求均值 按照波段平均划分
		int hmMap=bandsHy/bandm;
		for (int i=0;i<bandm;i++)
		{
			for (int k=0;k<endnumber;k++)
			{
				for (int j=0;j<hmMap;j++)
					endMsData[i*endnumber+k]+=endHyData[(i*hmMap+j)*endnumber+k];
				endMsData[i*endnumber+k]/=float(hmMap);
			}
		}
	}

	//有输入波长的情况下  首先获取多光谱波段对应的高光谱波段 然后进行处理
	if(hysp!=NULL&&msp!=NULL)	
	{
		//获取高光谱对应多光谱波段的分组
		int *group=new int[bandsHy];	//分组情况
		memset(group,0,sizeof(int)*bandsHy);

		int *groupNum=new int[bandm];	//多光谱一个波段对应高光谱波段个数

		//对高光谱计算各个波段对应的多光谱波段
		if(range==true)		//若输入的是光谱范围
		{
			for (int i=0;i<bandsHy;i++)
			{
				for(int j=0;j<bandm;j++)
				{
					if(hysp[j]<msp[2*j+1])
					{
						group[j]=j;
						break;
					}
				}
			}
		}
		else
		{
			//获取最近的光谱
			for(int i=0;i<bandsHy;i++)//直接输入的是光谱
			{
				int indexm=0;
				float mindata=1000;;
				for (int j=0;j<bandm;j++)
				{
					if(mindata>abs(hysp[i]-msp[j]))
					{
						mindata=abs(hysp[i]-msp[j]);
						indexm=j;
					}
				}
				group[i]=indexm;
			}
		}//对高光谱数据分组结束

		//每组高光谱波段进行计算
		memset(groupNum,0,sizeof(int)*bandm);
		for (int i=0;i<bandsHy;i++)
			groupNum[group[i]]++;

		//对每一组进行处理得到多光谱端元
		//对每个端元进行处理
		int total=0;
		for (int l=0;l<endnumber;l++)
		{
			for (int i=0;i<bandm;i++)
			{
				float *datahy =new float[groupNum[i]];
				float *datalen=new float[groupNum[i]];
				for (int j=0,k=0;j<bandsHy;j++)
				{
					if(group[j]==i)
					{
						datahy[k]=endHyData[j*endnumber+l];
						datalen[k]=hysp[j];
						k++;
					}
				}
				Gauss_Response(datahy,datalen,msp[i],endMsData[i*endnumber+l],groupNum[i]);
				delete[]datahy;datahy=NULL;
				delete[]datalen;datalen=NULL;
			}
		}
		delete[]group;group=NULL;
		delete[]groupNum;groupNum=NULL;
	}//有输入波长情况处理完成

	//输出处理后的端元
	HyperSpectralEnd_ExportEndmember(pathM,endMsData,bandm,endnumber);

	//清除指针
	delete[]endMsData;endMsData=NULL;
	delete[]endHyData;endHyData=NULL;

	return ;
}
void HyperSpectralEndMember::HyperSpectralEnd_Response_Image(const char* pathHyImage,const char* pathMImage,float* hysp,int bandsHy,float* msp,int bandm,int endnumber,bool range)
{
	//获取影像
	GDALAllRegister();
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO" );	//中文路径


	GDALDatasetH m_dataet=GDALOpen(pathHyImage,GA_ReadOnly);
	int xsize=GDALGetRasterXSize(m_dataet);
	int ysize=GDALGetRasterYSize(m_dataet);
	int bands=GDALGetRasterCount(m_dataet);

	//创建输出影像数据集
	char **papszOptions = NULL;  
	papszOptions = CSLSetNameValue( papszOptions, "INTERLEAVE", "BAND" ); 
	GDALDatasetH m_datasetout=GDALCreate(GDALGetDriverByName("GTiff"),pathMImage,xsize,ysize,bandm,GDT_Float32,papszOptions);

	//获取影像数据
	float* dataHy=new float[xsize*ysize*bands];
	float* dataM =new float[xsize*ysize*bandm];
	for (int i=0;i<bands;i++)
		GDALRasterIO(GDALGetRasterBand(m_dataet,i+1),GF_Read,0,0,xsize,ysize,dataHy+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	GDALClose(m_dataet);

	//无输入波段波长的情况下
	if(hysp==NULL&&msp==NULL)
	{
		//直接对应求均值 按照波段平均划分
		int hmMap=bandsHy/bandm;
		for (int i=0;i<bandm;i++)
		{
			for (int k=0;k<xsize*ysize;k++)
			{
				for (int j=0;j<hmMap;j++)
					dataHy[i*xsize*ysize+k]+=dataHy[(i*hmMap+j)*xsize*ysize+k];
				dataM[i*xsize*ysize+k]/=float(hmMap);
			}
		}
		//影像输出
		for (int i=0;i<bandm;i++)
			GDALRasterIO(GDALGetRasterBand(m_datasetout,i+1),GF_Write,0,0,xsize,ysize,dataM+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
		GDALClose(m_datasetout);

		delete[]dataHy;
		delete[]dataM;
	}

	//有输入波长的情况下  首先获取多光谱波段对应的高光谱波段 然后进行处理
	if(hysp!=NULL&&msp!=NULL)	
	{
		//获取高光谱对应多光谱波段的分组
		int *group=new int[bandsHy];	//分组情况
		memset(group,0,sizeof(int)*bandsHy);

		int *groupNum=new int[bandm];	//多光谱一个波段对应高光谱波段个数

		//对高光谱计算各个波段对应的多光谱波段
		if(range==true)		//若输入的是光谱范围
		{
			for (int i=0;i<bandsHy;i++)
			{
				for(int j=0;j<bandm;j++)
				{
					if(hysp[j]<msp[2*j+1])
					{
						group[j]=j;
						break;
					}
				}
			}
		}
		else
		{
			//获取最近的光谱
			for(int i=0;i<bandsHy;i++)//直接输入的是光谱
			{
				int indexm=0;
				float mindata=1000;;
				for (int j=0;j<bandm;j++)
				{
					if(mindata>abs(hysp[i]-msp[j]))
					{
						mindata=abs(hysp[i]-msp[j]);
						indexm=j;
					}
				}
				group[i]=indexm;
			}
		}//对高光谱数据分组结束

		//每组高光谱波段进行计算
		memset(groupNum,0,sizeof(int)*bandm);
		for (int i=0;i<bandsHy;i++)
			groupNum[group[i]]++;

		//对每个像素进行处理
		int total=0;
		for (int l=0;l<xsize*ysize;l++)
		{
			printf("\r高光谱光谱响应处理像素：%d/%d",xsize*ysize,l+1);
			for (int i=0;i<bandm;i++)
			{
				float *datathy=new float[groupNum[i]];
				float *datalen=new float[groupNum[i]];
				for (int j=0,k=0;j<bandsHy;j++)
				{
					if(group[j]==i)
					{
						datathy[k]=dataHy[j*xsize*ysize+l];
						datalen[k]=hysp[j];
						k++;
					}
				}
				Gauss_Response(datathy,datalen,msp[i],dataM[i*xsize*ysize+l],groupNum[i]);
				delete[]datathy;datathy=NULL;
				delete[]datalen;datalen=NULL;
			}
		}
		printf("\n");


		delete[]group;group=NULL;
		delete[]groupNum;groupNum=NULL;
	}//有输入波长情况处理完成



	//影像输出
	for (int i=0;i<bandm;i++)
		GDALRasterIO(GDALGetRasterBand(m_datasetout,i+1),GF_Write,0,0,xsize,ysize,dataM+i*xsize*ysize,xsize,ysize,GDT_Float32,0,0);
	GDALClose(m_datasetout);

	delete[]dataHy;
	delete[]dataM;

}

//空间响应函数（采用高斯函数进行空间响应）
void HyperSpectralEndMember::HyperSpectralEnd_Spatial_Spread(const char* pathImg,const char* pathSample)
{
	CSampleFunc samplefunc;
	samplefunc.gauss_down_sample(pathImg,3,3,pathSample);
	return ;
}