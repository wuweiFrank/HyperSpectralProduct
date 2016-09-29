#include "StdAfx.h"
#include "Level1Process.h"
#include "AuxiliaryFunction.h"
#include <fstream>
#include<omp.h>
using namespace std;
//辐射校正处理=====================================================================================================================================================================================
//绝对辐射校正
long QPDLevel1Process::Level1Proc_RadiationAbsolute(const char* pathImg, const char* pathImgRad, const char* pathAbsRegFile)
{
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");	//中文路径
	GDALAllRegister();
	long lError = 0;
	unsigned short *imgBuffer = NULL;	//影像数据
	float* parametersA = NULL, *parametersB = NULL, *parametersAux = NULL;//校正系数

	GDALDatasetH m_dataset = GDALOpen(pathImg, GA_ReadOnly);
	int xsize = GDALGetRasterXSize(m_dataset);
	int ysize = GDALGetRasterYSize(m_dataset);
	int bands = GDALGetRasterCount(m_dataset);

	char **papszOptions = NULL;
	papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");
	GDALDatasetH m_datasetdst = GDALCreate(GDALGetDriverByName("GTiff"), pathImgRad, xsize, ysize, bands, GDT_UInt16, papszOptions);

	//int nSamples, nLines, nLevels;
	//LevelProc_GetParameterInfo(pathImgRad, nSamples, nLines, nLevels);

	try
	{
		parametersA = new float[bands];
		parametersB = new float[bands];
		imgBuffer = new unsigned short[xsize*ysize];
	}
	catch (bad_alloc)
	{
		printf("allocate memory error\n");
		exit(-1);
	}

	Level1Proc_AbsoluteParameters(pathAbsRegFile, parametersA, parametersB);
	for (int i = 0; i < bands; i++)
	{
		GDALRasterIO(GDALGetRasterBand(m_dataset, i + 1), GF_Read, 0, 0, xsize, ysize, imgBuffer, xsize, ysize, GDT_UInt16, 0, 0);
		for (int j = 0; j < ysize; j++)
		{
			for (int k = 0; k < xsize; k++)
			{
				//扩大100倍精度为0.01
				imgBuffer[k] = (unsigned short)((imgBuffer[j*xsize + k] * parametersA[i] + parametersB[i]) * 100);
			}
		}
		GDALRasterIO(GDALGetRasterBand(m_datasetdst, i + 1), GF_Write, 0, 0, xsize, ysize, imgBuffer, xsize, ysize, GDT_UInt16, 0, 0);
	}

	delete[]parametersA; parametersA = NULL;
	delete[]parametersB; parametersB = NULL;
	delete[]imgBuffer;	 imgBuffer = NULL;
	GDALClose(m_dataset);
	GDALClose(m_datasetdst);
	return lError;
}
//获取绝对辐射校正系数
long QPDLevel1Process::Level1Proc_AbsoluteParameters(const char* pathAbsRegFile, float* paramA, float* paramB, float* paramAux /*= NULL*/)
{
	long lError = 0;
	long lResult = 0;
	int i = 0, j = 0, k = 0;
	int nBands;
	FILE *fCof = NULL;
	errno_t err;
	err = fopen_s(&fCof, pathAbsRegFile, "r");
	if (err)
	{
		lError = 30201;
		goto ErrEnd;
	}
	fscanf_s(fCof, "%d", &nBands);
	for (i = 0; i<nBands; i++)
	{
		lResult = fscanf_s(fCof, "%f%f", paramA + i, paramB + i);
		if (lResult != 2)
		{
			lError = 30202;
			goto ErrEnd;
		}
	}
ErrEnd:
	if (fCof)
	{
		fclose(fCof);
		fCof = NULL;
	}
	return lError;
}

//相对辐射校正 相对辐射校正是不是就是非均匀性校正
long QPDLevel1Process::Level1Proc_RadiationRelative(const char* pathImg, const char* pathImgRad, const char* pathRelRegFile)
{
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");	//中文路径
	GDALAllRegister();
	long lError = 0;
	unsigned short *imgBuffeSrc = NULL;	//影像数据
	unsigned short *imgBuffeDst = NULL;	//影像数据
	float* parametersA = NULL, *parametersB = NULL, *parametersAux = NULL;//校正系数
	float* parametersSgn = NULL;
	GDALDatasetH m_dataset = GDALOpen(pathImg, GA_ReadOnly);
	int xsize = GDALGetRasterXSize(m_dataset);
	int ysize = GDALGetRasterYSize(m_dataset);
	int bands = GDALGetRasterCount(m_dataset);

	//char **papszOptions = NULL;
	//papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");
	//GDALDatasetH m_datasetdst = GDALCreate(GDALGetDriverByName("GTiff"), pathImgRad, xsize, ysize, bands, GDT_UInt16, papszOptions);
	FILE* fDst = NULL;
	if (fopen_s(&fDst, pathImgRad, "wb") != 0)
		exit(-1);
	int nSamples, nBands,nLevels;
	LevelProc_GetParameterInfo(pathRelRegFile, nSamples, nBands, nLevels);

	//数据空间申请
	try
	{
		imgBuffeSrc = new unsigned short[xsize*ysize];
		imgBuffeDst = new unsigned short[xsize*ysize];
		parametersA = new float[nSamples*nBands*(nLevels-1)];
		parametersB = new float[nSamples*nBands*(nLevels-1)];
		parametersAux = new float[nLevels*nBands];
		parametersSgn = new float[nSamples*nBands];
	}
	catch (bad_alloc)
	{
		printf("allocate memory error\n");
		exit(-1);
	}

	if (LevelProc_RelativeParameters(pathRelRegFile, parametersA, parametersB, parametersAux) != 0)
		exit(-1);//读取校正参数异常
	
	//根据nLevel进行校正
	//线性校正
	if (nLevels == 2)
	{
		for (int i = 0; i < bands; ++i)
		{
			printf("process bands:%d\n", i + 1);
			GDALRasterIO(GDALGetRasterBand(m_dataset, i + 1), GF_Read, 0, 0, xsize, ysize, imgBuffeSrc, xsize, ysize, GDT_UInt16, 0, 0);
			for (int j = 0; j < xsize; ++j)
			{
				for (int k = 0; k < ysize; ++k)
				{
					int nImgOffset = k*nSamples + j;
					int nCofOffset = i*nSamples + j;
					imgBuffeDst[nImgOffset] = unsigned short(imgBuffeSrc[nImgOffset] * parametersA[nCofOffset] + parametersB[nCofOffset]);
					parametersSgn[i*nSamples + j] = parametersA[nCofOffset];
				}
			}
			//如果需要进行坏线校正的话
			for (int j = 0; j < nSamples; ++j)
			{
				//第i个波段第j个探元
				int		nFlagA = -1, nFlagB = -1;
				float	fWeightA = 0, fWeightB = 0;
				int		nCofOffset = i*nSamples + j;

				if (parametersSgn[nCofOffset] == 0)
				{
					if (j == 0)
					{
						for (int n = 1; n<nSamples; n++)
						{
							if (parametersSgn[nCofOffset + n] != 0)
							{
								nFlagB = j + n;
								fWeightA = 0;
								fWeightB = 1;
								break;
							}
						}
						nFlagA = -1;
					}
					else if (j == nSamples - 1)
					{
						for (int m = 1; m <= j; m++)
						{
							if (parametersSgn[nCofOffset - m] != 0)
							{
								nFlagA = j - m;
								fWeightA = 1;
								fWeightB = 0;
								break;
							}
						}
						nFlagB = -1;
					}
					else
					{
						for (int m = 1; m <= j; m++)
						{
							if (parametersSgn[nCofOffset - m] != 0 && j - m >= 0)
							{
								nFlagA = j - m;
								break;
							}
						}
						for (int n = 1; n <= nSamples - j; n++)
						{
							if (parametersSgn[nCofOffset + n] != 0 && j + n<nSamples)
							{
								nFlagB = j + n;
								break;
							}
						}
						if (nFlagA != -1 && nFlagB != -1)
						{
							fWeightA = (float)(j - nFlagA) / (float)(nFlagB - nFlagA);
							fWeightB = (float)(nFlagB - j) / (float)(nFlagB - nFlagA);
						}
						if (nFlagA == -1 && nFlagB != -1)
						{
							fWeightA = 0;
							fWeightB = 1;
						}
						if (nFlagA != -1 && nFlagB == -1)
						{
							fWeightA = 1;
							fWeightB = 0;
						}
					}
					if (nFlagA != -1 && nFlagB != -1)
					{
						for (int k = 0; k<ysize; k++)
						{
							imgBuffeDst[k*nSamples + j] = unsigned short(imgBuffeDst[k*nSamples + nFlagA] * fWeightA + imgBuffeDst[k*nSamples + nFlagB] * fWeightB);
						}
					}
					else if (nFlagA == -1 && nFlagB != -1)
					{
						for (int k = 0; k<ysize; k++)
						{
							imgBuffeDst[k*nSamples + j] = imgBuffeDst[k*nSamples + nFlagB];
						}
					}
					else
					{
						for (int k = 0; k<ysize; k++)
						{
							imgBuffeDst[k*nSamples + j] = imgBuffeDst[k*nSamples + nFlagA];
						}
					}
				}
			}
			//数据输出
			fwrite(imgBuffeDst, 2, xsize*ysize, fDst);
			fflush(fDst);
			//GDALRasterIO(GDALGetRasterBand(m_datasetdst, i + 1), GF_Read, 0, 0, xsize, ysize, imgBuffeDst, xsize, ysize, GDT_UInt16, 0, 0);
		}
	}
	//分段线性校正
	if (nLevels == 4)
	{
		float *fLevelValue = new float[nLevels];
		for (int i = 0; i < bands; i++)
		{
			printf("process bands:%d\n", i + 1);
			//获取波段
			GDALRasterIO(GDALGetRasterBand(m_dataset, i + 1), GF_Read, 0, 0, xsize, ysize, imgBuffeSrc, xsize, ysize, GDT_UInt16, 0, 0);
			for (int j = 0; j < nLevels; j++)
			{//获取能级范围
				fLevelValue[j] = parametersAux[i*nLevels + j];
			}
			fLevelValue[0] = 0;
			fLevelValue[nLevels - 1] = 65535;

			for (int j = 0; j < nSamples; j++)
			{
				//#pragma omp parallel for
				for (int k = 0; k < ysize; k++)
				{
					//获取像元
					int nImgOffset = k*nSamples + j;
					int nDN = imgBuffeSrc[nImgOffset];
					for (int m = 0; m < nLevels - 1; m++)
					{ 
						//找到对应能级和参数，进行辐射校正
						if (nDN >= fLevelValue[m] && nDN < fLevelValue[m + 1])
						{
							int nCofOffset = i*nSamples + j + m*xsize*bands;
							imgBuffeDst[nImgOffset] = unsigned short(nDN * parametersA[nCofOffset] + parametersB[nCofOffset]);
							parametersSgn[i*nSamples + j] = parametersA[nCofOffset];
							break;
						}
					}
				}
			}

			//如果需要进行坏线校正的话
			for (int j = 0; j < nSamples; ++j)
			{
				//第i个波段第j个探元
				int		nFlagA = -1, nFlagB = -1;
				float	fWeightA = 0, fWeightB = 0;
				int		nCofOffset = i*nSamples + j;

				if (parametersSgn[nCofOffset] == 0)
				{
					if (j == 0)
					{
						for (int n = 1; n<nSamples; n++)
						{
							if (parametersSgn[nCofOffset + n] != 0)
							{
								nFlagB = j + n;
								fWeightA = 0;
								fWeightB = 1;
								break;
							}
						}
						nFlagA = -1;
					}
					else if (j == nSamples - 1)
					{
						for (int m = 1; m <= j; m++)
						{
							if (parametersSgn[nCofOffset - m] != 0)
							{
								nFlagA = j - m;
								fWeightA = 1;
								fWeightB = 0;
								break;
							}
						}
						nFlagB = -1;
					}
					else
					{
						for (int m = 1; m <= j; m++)
						{
							if (parametersSgn[nCofOffset - m] != 0 && j - m >= 0)
							{
								nFlagA = j - m;
								break;
							}
						}
						for (int n = 1; n <= nSamples - j; n++)
						{
							if (parametersSgn[nCofOffset + n] != 0 && j + n<nSamples)
							{
								nFlagB = j + n;
								break;
							}
						}
						if (nFlagA != -1 && nFlagB != -1)
						{
							fWeightA = (float)(j - nFlagA) / (float)(nFlagB - nFlagA);
							fWeightB = (float)(nFlagB - j) / (float)(nFlagB - nFlagA);
						}
						if (nFlagA == -1 && nFlagB != -1)
						{
							fWeightA = 0;
							fWeightB = 1;
						}
						if (nFlagA != -1 && nFlagB == -1)
						{
							fWeightA = 1;
							fWeightB = 0;
						}
					}
					if (nFlagA != -1 && nFlagB != -1)
					{
						for (int k = 0; k<ysize; k++)
						{
							imgBuffeDst[k*nSamples + j] = unsigned short(imgBuffeDst[k*nSamples + nFlagA] * fWeightA + imgBuffeDst[k*nSamples + nFlagB] * fWeightB);
						}
					}
					else if (nFlagA == -1 && nFlagB != -1)
					{
						for (int k = 0; k<ysize; k++)
						{
							imgBuffeDst[k*nSamples + j] = imgBuffeDst[k*nSamples + nFlagB];
						}
					}
					else
					{
						for (int k = 0; k<ysize; k++)
						{
							imgBuffeDst[k*nSamples + j] = imgBuffeDst[k*nSamples + nFlagA];
						}
					}
				}
			}
			//数据输出
			fwrite(imgBuffeDst, 2, xsize*ysize, fDst);
			fflush(fDst);
			//GDALRasterIO(GDALGetRasterBand(m_datasetdst, i + 1), GF_Read, 0, 0, xsize, ysize, imgBuffeDst, xsize, ysize, GDT_UInt16, 0, 0);
		}
		delete[]fLevelValue; fLevelValue = NULL;

	}
	GDALClose(m_dataset);
	fclose(fDst);
	//GDALClose(m_datasetdst);
	//写头文件
	char drive[_MAX_DRIVE]; char dir[_MAX_DIR]; char filename[_MAX_FNAME]; char ext[_MAX_EXT];
	char path[_MAX_PATH];
	_splitpath_s(pathImgRad, drive, dir, filename, ext);
	_makepath_s(path, drive, dir, filename, "hdr");
	ENVIHeader mENVIHeader;
	memset(&mENVIHeader, 0, sizeof(ENVIHeader));
	mENVIHeader.datatype = 12;
	mENVIHeader.imgWidth = xsize;
	mENVIHeader.imgHeight = ysize;
	mENVIHeader.imgBands = bands;
	mENVIHeader.interleave = "BSQ";
	mENVIHeader.WriteHeaderBasic(path);
	//WriteENVIHeader(path, mENVIHeader);

	delete[]imgBuffeSrc; imgBuffeSrc = NULL;
	delete[]imgBuffeDst; imgBuffeDst = NULL;
	delete[]parametersA; parametersA = NULL;
	delete[]parametersB; parametersB = NULL;
	delete[]parametersAux; parametersAux = NULL;
	delete[]parametersSgn; parametersSgn = NULL;

	return lError;
}
//获取相对辐射校正系数
long QPDLevel1Process::LevelProc_RelativeParameters(const char* pathRelRegFile, float* paramA, float* paramB, float* paramAux/* = NULL*/)
{
	long lError = 0;
	FILE *fCof = NULL;
	errno_t err = 0;
	int i = 0, j = 0, k = 0, p = 0, q = 0;
	int nSamples = 0, nLines = 0, nLevel = 0;
	int nID1 = 0, nID2 = 0;
	int nFrame = 0;

	err = fopen_s(&fCof, pathRelRegFile, "r");
	if (err)
	{
		lError = 1;
		return lError;
	}

	fscanf_s(fCof, "%d", &nSamples);
	fscanf_s(fCof, "%d", &nLines);
	fscanf_s(fCof, "%d", &nLevel);
	nFrame = nSamples*nLines;
	for (i = 0; i < nLines; i++)
	{
		for (j = 0; j < nLevel; j++)
		{
			fscanf_s(fCof, "%f", paramAux + i*nLevel + j);
		}
		for (j = 0; j < nSamples; j++)
		{
			fscanf_s(fCof, "%d	%d	", &nID1, &nID2);
			for (k = 0; k < nLevel - 1; k++)
			{
				fscanf_s(fCof, "%f	%f	", paramA + i*nSamples + j + k*nFrame, paramB + i*nSamples + j + k*nFrame);
			}
		}
	}
	fclose(fCof);
	return lError;
}
//获取相对辐射校正系数的参数
long QPDLevel1Process::LevelProc_GetParameterInfo(const char* pathImgRad, int& nSamples, int &nBands, int &nLevels)
{
	long lError = 0;
	FILE *fCof = NULL;
	errno_t err = 0;
	err = fopen_s(&fCof, pathImgRad, "r");
	if (err)
	{
		lError = 1;
		return lError;
	}
	fscanf_s(fCof, "%d", &nSamples);
	fscanf_s(fCof, "%d", &nBands);
	fscanf_s(fCof, "%d", &nLevels);
	fclose(fCof);
	return lError;
}

//==================================================================================================================================================================================================
//进行视场拼接接口函数参数在头文件中说明
//三个视场的拼接
long QPDLevel1Process::Level1Proc_ViewJointThree(const char* pathview1, const char* pathview2, const char* pathview3,
	float over_left_center_x, float over_right_center_x, float err_left_center_y, float err_right_center_y, const char* pathJointView)
{
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");	//中文路径
	GDALAllRegister();
	long lError = 0;
	FILE* fDst = NULL;
	if (fopen_s(&fDst, pathJointView, "wb") != 0)
		exit(-1);

	GDALDatasetH m_dataset_view_left, m_dataset_view_center, m_dataset_view_right;
	m_dataset_view_left = GDALOpen(pathview1, GA_ReadOnly);
	m_dataset_view_center = GDALOpen(pathview2, GA_ReadOnly);
	m_dataset_view_right = GDALOpen(pathview3, GA_ReadOnly);

	int xsize1 = GDALGetRasterXSize(m_dataset_view_left);
	int ysize1 = GDALGetRasterYSize(m_dataset_view_left);

	int xsize2 = GDALGetRasterXSize(m_dataset_view_center);
	int ysize2 = GDALGetRasterYSize(m_dataset_view_center);

	int xsize3 = GDALGetRasterXSize(m_dataset_view_right);
	int ysize3 = GDALGetRasterYSize(m_dataset_view_right);

	int bands = GDALGetRasterCount(m_dataset_view_left);
	GDALDataType m_datatype = GDALGetRasterDataType(GDALGetRasterBand(m_dataset_view_left, 1));
	if (m_datatype != GDT_UInt16)
		exit(-1);//数据类型不是QPD数据类型

	//获取拼接范围和拼接边
	int mosaic_x_size, mosaic_y_size;
	vector<CPOINT> edge_points;
	lError = Level1Proc_ViewJointRange(xsize1, ysize1, xsize2, ysize2, xsize3, ysize3, over_left_center_x, err_left_center_y, over_right_center_x, err_right_center_y, mosaic_x_size, mosaic_y_size);
	if (lError != 0)
		return lError;
	lError = Level1Proc_ViewJointEdge(xsize1, ysize1, xsize2, ysize2, xsize3, ysize3, over_left_center_x, err_left_center_y, over_right_center_x, err_right_center_y, edge_points);
	if (lError != 0)
		return lError;


	//创建输出影像
	//GDALDatasetH m_dataset_out;
	//char **papszOptions = NULL;
	//papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");
	//m_dataset_out = GDALCreate(GDALGetDriverByName("GTiff"), pathJointView, mosaic_x_size, mosaic_y_size, bands, m_datatype, papszOptions);

	unsigned short* mosaic_data_us = NULL;
	try
	{
		mosaic_data_us = new unsigned short[mosaic_x_size*mosaic_y_size];
		memset(mosaic_data_us, 0, sizeof(unsigned short)*mosaic_x_size*mosaic_y_size);
	}
	catch (bad_alloc)
	{
		printf("allocate memory error \n");
		exit(-1);
	}


	//无符号16位整数
	//根据数据类型填写数据
	if (m_datatype == GDT_UInt16)
	{
		unsigned short* imgdata = new unsigned short[xsize1*ysize1 + xsize2*ysize2 + xsize3*ysize3];
		memset(imgdata, 0, sizeof(unsigned short)*(xsize1*ysize1 + xsize2*ysize2 + xsize3*ysize3));
		lError = Level1Proc_ViewJointFillData(imgdata, xsize1, ysize1, xsize2, ysize2, xsize3, ysize3, over_left_center_x, over_right_center_x, err_left_center_y, err_right_center_y,
			m_dataset_view_left, m_dataset_view_center, m_dataset_view_right, mosaic_data_us, fDst, mosaic_x_size, mosaic_y_size, edge_points);
		if (lError != 0)
			exit(-1);
		delete[]imgdata;
	}
	else 
	{
		//数据类型不对，不是QPD数据，此函数只针对QPD数据处理
	}

	//写头文件
	char drive[_MAX_DRIVE]; char dir[_MAX_DIR]; char filename[_MAX_FNAME]; char ext[_MAX_EXT];
	char path[_MAX_PATH];
	_splitpath_s(pathJointView, drive, dir, filename, ext);
	_makepath_s(path, drive, dir, filename, "hdr");
	ENVIHeader mENVIHeader;
	memset(&mENVIHeader, 0, sizeof(ENVIHeader));
	mENVIHeader.datatype = 12;
	mENVIHeader.imgWidth = mosaic_x_size;
	mENVIHeader.imgHeight = mosaic_y_size;
	mENVIHeader.imgBands = bands;
	mENVIHeader.interleave = "BSQ";
	mENVIHeader.WriteHeaderBasic(path);
	//WriteENVIHeader(path, mENVIHeader);

	if (fDst != NULL)
		fclose(fDst);
	if (mosaic_data_us!=NULL)
		delete[]mosaic_data_us;
	if (m_dataset_view_left != NULL)
		GDALClose(m_dataset_view_left);
	if (m_dataset_view_center != NULL)
		GDALClose(m_dataset_view_center);
	if (m_dataset_view_right != NULL)
		GDALClose(m_dataset_view_right);



	//指针为空
	mosaic_data_us = NULL;
	m_dataset_view_left = NULL;
	m_dataset_view_center = NULL;
	m_dataset_view_right = NULL;
	fDst = NULL;

	return lError;
}
//两个视场的拼接(两个视场的拼接比三个视场的拼接要简单的多)
long QPDLevel1Process::Level1Proc_ViewJointTwo(const char* pathview1, const char* pathview2, float over_x, float err_y, const char* pathJointView)
{
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");	//中文路径
	GDALAllRegister();
	long lError = 0;
	GDALDatasetH m_dataset_view_left, m_dataset_view_right;
	m_dataset_view_left = GDALOpen(pathview1, GA_ReadOnly);
	m_dataset_view_right = GDALOpen(pathview2, GA_ReadOnly);

	int xsize1 = GDALGetRasterXSize(m_dataset_view_left);
	int ysize1 = GDALGetRasterYSize(m_dataset_view_left);

	int xsize2 = GDALGetRasterXSize(m_dataset_view_right);
	int ysize2 = GDALGetRasterYSize(m_dataset_view_right);

	int bands = GDALGetRasterCount(m_dataset_view_left);
	GDALDataType m_datatype = GDALGetRasterDataType(GDALGetRasterBand(m_dataset_view_left, 1));
	if (m_datatype == GDT_Byte)
		m_datatype = GDT_UInt16;

	//获取拼接范围和拼接边
	int mosaic_x_size, mosaic_y_size;
	vector<CPOINT> edge_points;
	lError = Level1Proc_ViewJointRange(xsize1, ysize1, xsize2, ysize2, over_x, err_y, mosaic_x_size, mosaic_y_size);
	if (lError != 0)
		return lError;
	lError = Level1Proc_ViewJointEdge(xsize1, ysize1, xsize2, ysize2, over_x, err_y, edge_points);
	if (lError != 0)
		return lError;
	if (m_datatype != GDT_UInt16)
		exit(-1);//数据类型不对

	//创建输出影像
	//GDALDatasetH m_dataset_out;
	//char **papszOptions = NULL;
	//papszOptions = CSLSetNameValue(papszOptions, "INTERLEAVE", "BAND");
	//m_dataset_out = GDALCreate(GDALGetDriverByName("GTiff"), pathJointView, mosaic_x_size, mosaic_y_size, bands, m_datatype, papszOptions);
	FILE* fDst = NULL;
	if (fopen_s(&fDst, pathJointView, "wb") != 0)
		exit(-1);

	unsigned short* mosaic_data_us = NULL;
	try
	{
		mosaic_data_us = new unsigned short[mosaic_x_size*mosaic_y_size];
		memset(mosaic_data_us, 0, sizeof(unsigned short)*mosaic_x_size*mosaic_y_size);
	}
	catch (bad_alloc)
	{
		printf("allocate memory error \n");
		exit(-1);
	}

	//无符号16位整数
	if (m_datatype == GDT_UInt16)
	{
		unsigned short* imgdata = new unsigned short[xsize1*ysize1 + xsize2*ysize2];
		lError = Level1Proc_ViewJointFillData(imgdata, xsize1, ysize1, xsize2, ysize2, over_x, err_y, m_dataset_view_left, m_dataset_view_right, mosaic_data_us, fDst, mosaic_x_size, mosaic_y_size, edge_points);
		if (lError != 0)
			exit(-1);
		delete[]imgdata;
	}
	else
	{
		//数据类型不对
	}
	//写头文件
	char drive[_MAX_DRIVE]; char dir[_MAX_DIR]; char filename[_MAX_FNAME]; char ext[_MAX_EXT];
	char path[_MAX_PATH];
	_splitpath_s(pathJointView, drive, dir, filename, ext);
	_makepath_s(path, drive, dir, filename, "hdr");
	ENVIHeader mENVIHeader;
	memset(&mENVIHeader, 0, sizeof(ENVIHeader));
	mENVIHeader.datatype = 12;
	mENVIHeader.imgWidth = mosaic_x_size;
	mENVIHeader.imgHeight = mosaic_y_size;
	mENVIHeader.imgBands = bands;
	mENVIHeader.interleave = "BSQ";
	mENVIHeader.WriteHeaderBasic(path);
	//WriteENVIHeader(path, mENVIHeader);

	if (mosaic_data_us)
		delete[]mosaic_data_us;
	if (fDst != NULL)
		fclose(fDst);
	if (m_dataset_view_left)
		GDALClose(m_dataset_view_left);
	if (m_dataset_view_right)
		GDALClose(m_dataset_view_right);

	//指针为空
	mosaic_data_us = NULL;
	m_dataset_view_left = NULL;
	m_dataset_view_right = NULL;
	fDst = NULL;

	return lError;
}

//内部接口
//获取视场拼接校园
long QPDLevel1Process::Level1Proc_ViewJointRange(int xsize1, int ysize1, int xsize2, int ysize2, int xsize3, int ysize3, float over_left_centerx,
	float err_left_centery, float over_right_centerx, float err_right_centery, int &xmosaic, int &ymosaic)
{
	/*
	_____   ______                           ______                      _______
	|   _|__|_   |          _______          |   _|___              _____|_    |
	|   ||  ||   |       ___|_   _|_____     |   ||  _|_____    ____|_   ||    |
	|   ||  ||   |       |  ||   ||    |     |   ||  ||    |    |   ||   ||    |
	|___||  |____|       |  ||   ||    |     |___||  ||    |    |   ||   |_____|
	    |____|           |  |____|     |          |___||   |    |    |____|
	                     |___|   |_____|              |____|              |____|
	*/
	xmosaic = xsize1 + xsize2 + xsize3 - over_left_centerx - over_right_centerx;
	if ((err_left_centery>0 && err_right_centery>0) ||
		(err_left_centery<0 && err_right_centery<0))
		ymosaic = max(max(ysize1, ysize2), ysize3) + max(abs(err_left_centery), abs(err_right_centery));
	else
		ymosaic = max(max(ysize1, ysize2), ysize3) + abs(err_left_centery) + abs(err_right_centery);
	return 0;
}
long QPDLevel1Process::Level1Proc_ViewJointRange(int xsize1, int ysize1, int xsize2, int ysize2, float over_x, float err_y, int &xmosaic, int &ymosaic)
{
	xmosaic = xsize1 + xsize2 - over_x;
	ymosaic = max(ysize1, ysize2) + abs(err_y);
	return 0;
}
//获取视场拼接边界
long QPDLevel1Process::Level1Proc_ViewJointEdge(int xsize1, int ysize1, int xsize2, int ysize2, int xsize3, int ysize3, float over_left_centerx, float err_left_centery,
	float over_right_centerx, float err_right_centery, vector<CPOINT>& edge_position)

{
	int left_center_x_st = xsize1 - over_left_centerx;
	int left_center_x_end = xsize1;
	int right_center_x_st = xsize1 + xsize2 - over_left_centerx - over_right_centerx;
	int right_center_x_end = xsize1 + xsize2 - over_left_centerx;

	int ylc = min(ysize1, ysize2);
	int yrc = min(ysize2, ysize3);

	double left_center_y_st; int left_center_y_end;
	double right_center_y_st; int right_center_y_end;
	/*
	//有四种情况
	//1.假左右两景影像都高于视场影像
	//2.假设左右视场影像都低于中间视场影像
	//3.假设左视场影像高于中视场影像，右视场影像低于中视场影像
	//4.假设左视场影像低于中视场影像，右视场影像高于中视场影像
	_____   ______                           ______                      _______
	|   _|__|_   |          _______          |   _|___              _____|_    |
	|   ||  ||   |       ___|_   _|_____     |   ||  _|_____    ____|_   ||    |
	|   ||  ||   |       |  ||   ||    |     |   ||  ||    |    |   ||   ||    |
	|___||  |____|       |  ||   ||    |     |___||  ||    |    |   ||   |_____|
	|____|           |  |_____|    |         |___||    |    |   |_____|
	|___|   |_____|             |_____|    |____|
	*/
	if (err_left_centery == 0 && err_right_centery == 0)
	{
		left_center_y_st = right_center_y_st = 0;
		left_center_y_end = ylc;
		right_center_y_end = yrc;
	}
	if ((err_left_centery>0 && err_right_centery>0))
	{
		right_center_y_st = left_center_y_st = max(abs(err_left_centery), abs(err_right_centery));
		if (err_left_centery<err_right_centery)
		{
			left_center_y_end = ysize1 + err_right_centery - err_left_centery;
			right_center_y_end = ysize3;
		}
		if (err_left_centery >= err_right_centery)
		{
			left_center_y_end = ysize1;
			right_center_y_end = ysize3 + err_left_centery - err_right_centery;
		}
	}
	if ((err_left_centery<0 && err_right_centery<0))
	{
		left_center_y_st = abs(err_left_centery);
		right_center_y_st = abs(err_right_centery);
		left_center_y_end = ylc;
		right_center_y_end = yrc;
	}
	if (err_left_centery>0 && err_right_centery<0)
	{
		left_center_y_st = 0;
		left_center_y_end = ysize1;
		right_center_y_st = err_left_centery + abs(err_right_centery);
		right_center_y_end = err_left_centery + ysize2;
	}
	if (err_left_centery<0 && err_right_centery>0)
	{
		left_center_y_st = abs(err_left_centery) + abs(err_right_centery);
		left_center_y_end = ysize2 + abs(err_left_centery);
		right_center_y_st = err_right_centery;
		right_center_y_end = ysize2;
	}

	//边缘左中视场x像元位置
	if (err_left_centery != 0)
	{
		for (int i = left_center_x_st; i<left_center_x_end; ++i)
		{
			CPOINT pnt;
			pnt.x = i;
			pnt.y = left_center_y_end;
			edge_position.push_back(pnt);
		}
	}
	//边缘中右视场x像元位置
	if (err_right_centery != 0)
	{
		for (int i = right_center_x_st; i<right_center_x_end; ++i)
		{
			CPOINT pnt;
			pnt.x = i;
			pnt.y = right_center_y_end;
			edge_position.push_back(pnt);
		}
	}
	//边缘左中视场y像元位置
	for (int i = left_center_y_st; i<left_center_y_end; ++i)
	{
		CPOINT pnt;
		pnt.x = left_center_x_st;
		pnt.y = i;
		edge_position.push_back(pnt);
	}
	//边缘中右视场y像元位置
	for (int i = right_center_y_st; i<right_center_y_end; ++i)
	{
		CPOINT pnt;
		pnt.x = right_center_x_st;
		pnt.y = i;
		edge_position.push_back(pnt);
	}

	return 0;
}
long QPDLevel1Process::Level1Proc_ViewJointEdge(int xsize1, int ysize1, int xsize2, int ysize2, float over_x, float err_y, vector<CPOINT>& edge_position)
{
	for (int i = xsize1 - over_x; i<xsize1; ++i)
	{
		CPOINT pt;
		pt.x = i; pt.y = abs(err_y);
		edge_position.push_back(pt);
	}
	for (int i = abs(err_y); i<ysize1; ++i)
	{
		CPOINT pt;
		pt.x = xsize1 - over_x; pt.y = abs(i);
		edge_position.push_back(pt);
	}
	return 0;
}
//视场边界的羽化处理
long QPDLevel1Process::Level1Proc_ViewJointFeather(unsigned short* imagedata, int xsize, int ysize, vector<CPOINT>& edge_position)
{
	int edge_pixel = edge_position.size();
	float *feather_data = new float[edge_pixel];

	//对边缘进行均值滤波
	int featherSize = 3;
	for (int i = 0; i<edge_pixel; ++i)
	{
		int total = 0;
		float temp_data = 0;
		int x_start = max((-int(featherSize / 2) + edge_position[i].x), 0);
		int x_end = min((int(featherSize / 2) + edge_position[i].x), xsize);

		int y_start = max((-int(featherSize / 2) + edge_position[i].y), 0);
		int y_end = min((int(featherSize / 2) + edge_position[i].y), ysize);
		for (int j = x_start; j<x_end; ++j)
		{
			for (int k = y_start; k<y_end; ++k)
			{
				++total;
				temp_data += imagedata[k*xsize + j];
			}
		}
		feather_data[i] = (temp_data) / total;
	}

	for (int i = 0; i<edge_pixel; ++i)
		imagedata[edge_position[i].y*xsize + edge_position[i].x] = (unsigned short)(feather_data[i]);
	delete[]feather_data;

	return 0;
}
//向拼接视场中填写数据
long QPDLevel1Process::Level1Proc_ViewJointFillData(unsigned short* imgViewData, int xsize1, int ysize1, int xsize2, int ysize2, int xsize3, int ysize3, float over_left_center_x, float over_right_center_x,
	float err_left_center_y, float err_right_center_y, GDALDatasetH m_datasetLeft, GDALDatasetH m_datasetCenter, GDALDatasetH m_datasetRight,
	unsigned short* datamosaic, FILE* fDst, int xmosaic, int ymosaic, vector<CPOINT>& edge_position)
{
	//做判断 填数据
	/*
	______  ______                           ______                      _______
	|   _|__|_   |          _______          |   _|___              _____|_    |
	|   ||  ||   |       ___|_   _|_____     |   ||  _|_____    ____|_   ||    |
	|   ||  ||   |       |  ||   ||    |     |   ||  ||    |    |   ||   ||    |
	|___||  |____|       |  ||   ||    |     |___||  ||    |    |   ||   |_____|
	|____|       |       |___|    |          |___||    |    |   |_____|
	|___|   |_____|              |_____|     |____|
	*/
	if (datamosaic == NULL)
		return -1;
	int bands = GDALGetRasterCount(m_datasetLeft);
	long lError = 0;

	if (err_left_center_y>0 && err_right_center_y>0)
	{
		//左视场高
		if (err_left_center_y>err_right_center_y)
		{
			for (int i = 0; i<bands; ++i)
			{
				printf("joint band: %d\n", i + 1);
				GDALRasterIO(GDALGetRasterBand(m_datasetLeft, i + 1), GF_Read, 0, 0, xsize1, ysize1, imgViewData, xsize1, ysize1, GDT_UInt16, 0, 0);
				GDALRasterIO(GDALGetRasterBand(m_datasetCenter, i + 1), GF_Read, 0, 0, xsize2, ysize2, imgViewData + xsize1*ysize1, xsize2, ysize2, GDT_UInt16, 0, 0);
				GDALRasterIO(GDALGetRasterBand(m_datasetRight, i + 1), GF_Read, 0, 0, xsize3, ysize3, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize3, ysize3, GDT_UInt16, 0, 0);
				Level1Proc_ColorMatch(imgViewData, imgViewData + xsize1*ysize1, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize1, ysize1, xsize2, ysize2, xsize3, ysize3, over_left_center_x, over_right_center_x, err_left_center_y, err_right_center_y);

				lError = Level1Proc_ViewJointFillData(imgViewData, xsize1, ysize1, 0, 0, datamosaic, xmosaic, ymosaic);
				lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1, xsize2, ysize2, xsize1 - over_left_center_x, err_left_center_y, datamosaic, xmosaic, ymosaic);
				lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize3, ysize3, xsize1 + xsize2 - over_left_center_x - over_right_center_x, err_left_center_y - err_right_center_y, datamosaic, xmosaic, ymosaic);

				if (lError != 0)
					return lError;
				Level1Proc_ViewJointFeather(datamosaic, xmosaic, ymosaic, edge_position);
				fwrite(datamosaic, 2, xmosaic*ymosaic, fDst);
				fflush(fDst);
				//GDALRasterIO(GDALGetRasterBand(m_datasetOut, i + 1), GF_Write, 0, 0, xmosaic, ymosaic, datamosaic, xmosaic, ymosaic, GDT_UInt16, 0, 0);

			}
		}
		//右视场高
		if (err_left_center_y <= err_right_center_y)
		{
			for (int i = 0; i<bands; ++i)
			{
				printf("joint band: %d\n", i + 1);
				GDALRasterIO(GDALGetRasterBand(m_datasetLeft, i + 1), GF_Read, 0, 0, xsize1, ysize1, imgViewData, xsize1, ysize1, GDT_UInt16, 0, 0);
				GDALRasterIO(GDALGetRasterBand(m_datasetCenter, i + 1), GF_Read, 0, 0, xsize2, ysize2, imgViewData + xsize1*ysize1, xsize2, ysize2, GDT_UInt16, 0, 0);
				GDALRasterIO(GDALGetRasterBand(m_datasetRight, i + 1), GF_Read, 0, 0, xsize3, ysize3, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize3, ysize3, GDT_UInt16, 0, 0);
				Level1Proc_ColorMatch(imgViewData, imgViewData + xsize1*ysize1, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize1, ysize1, xsize2, ysize2, xsize3, ysize3, over_left_center_x, over_right_center_x, err_left_center_y, err_right_center_y);

				lError = Level1Proc_ViewJointFillData(imgViewData, xsize1, ysize1, 0, err_right_center_y - err_left_center_y, datamosaic, xmosaic, ymosaic);
				lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1, xsize2, ysize2, xsize1 - over_left_center_x, err_right_center_y, datamosaic, xmosaic, ymosaic);
				lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize2, ysize2, xsize1 + xsize2 - over_left_center_x - over_right_center_x, 0, datamosaic, xmosaic, ymosaic);
				if (lError != 0)
					return lError;
				Level1Proc_ViewJointFeather(datamosaic, xmosaic, ymosaic, edge_position);
				fwrite(datamosaic, 2, xmosaic*ymosaic, fDst);
				fflush(fDst);
				//GDALRasterIO(GDALGetRasterBand(m_datasetOut, i + 1), GF_Write, 0, 0, xmosaic, ymosaic, datamosaic, xmosaic, ymosaic, GDT_UInt16, 0, 0);
			}
		}
	}
	if (err_left_center_y<0 && err_right_center_y<0)
	{
		//左右视场都低
		for (int i = 0; i<bands; ++i)
		{
			printf("joint band: %d\n", i + 1);
			GDALRasterIO(GDALGetRasterBand(m_datasetLeft, i + 1), GF_Read, 0, 0, xsize1, ysize1, imgViewData, xsize1, ysize1, GDT_UInt16, 0, 0);
			GDALRasterIO(GDALGetRasterBand(m_datasetCenter, i + 1), GF_Read, 0, 0, xsize2, ysize2, imgViewData + xsize1*ysize1, xsize2, ysize2, GDT_UInt16, 0, 0);
			GDALRasterIO(GDALGetRasterBand(m_datasetRight, i + 1), GF_Read, 0, 0, xsize3, ysize3, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize3, ysize3, GDT_UInt16, 0, 0);
			Level1Proc_ColorMatch(imgViewData, imgViewData + xsize1*ysize1, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize1, ysize1, xsize2, ysize2, xsize3, ysize3, over_left_center_x, over_right_center_x, err_left_center_y, err_right_center_y);

			lError = Level1Proc_ViewJointFillData(imgViewData, xsize1, ysize1, 0, abs(err_right_center_y), datamosaic, xmosaic, ymosaic);
			lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1, xsize2, ysize2, xsize1 - over_left_center_x, 0, datamosaic, xmosaic, ymosaic);
			lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize3, ysize3, xsize1 + xsize2 - over_left_center_x - over_right_center_x, abs(err_right_center_y), datamosaic, xmosaic, ymosaic);

			if (lError != 0)
				return lError;

			Level1Proc_ViewJointFeather(datamosaic, xmosaic, ymosaic, edge_position);
			fwrite(datamosaic, 2, xmosaic*ymosaic, fDst);
			fflush(fDst);
			//GDALRasterIO(GDALGetRasterBand(m_datasetOut, i + 1), GF_Write, 0, 0, xmosaic, ymosaic, datamosaic, xmosaic, ymosaic, GDT_UInt16, 0, 0);
		}
	}
	if (err_left_center_y>0 && err_right_center_y<0)
	{
		//左视场高右视场低
		for (int i = 0; i<bands; ++i)
		{
			printf("joint band: %d\n", i + 1);
			GDALRasterIO(GDALGetRasterBand(m_datasetLeft, i + 1), GF_Read, 0, 0, xsize1, ysize1, imgViewData, xsize1, ysize1, GDT_UInt16, 0, 0);
			GDALRasterIO(GDALGetRasterBand(m_datasetCenter, i + 1), GF_Read, 0, 0, xsize2, ysize2, imgViewData + xsize1*ysize1, xsize2, ysize2, GDT_UInt16, 0, 0);
			GDALRasterIO(GDALGetRasterBand(m_datasetRight, i + 1), GF_Read, 0, 0, xsize3, ysize3, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize3, ysize3, GDT_UInt16, 0, 0);
			Level1Proc_ColorMatch(imgViewData, imgViewData + xsize1*ysize1, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize1, ysize1, xsize2, ysize2, xsize3, ysize3, over_left_center_x, over_right_center_x, err_left_center_y, err_right_center_y);

			lError = Level1Proc_ViewJointFillData(imgViewData, xsize1, ysize1, 0, 0, datamosaic, xmosaic, ymosaic);
			lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1, xsize2, ysize2, xsize1 - over_left_center_x, err_left_center_y, datamosaic, xmosaic, ymosaic);
			lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1 + xsize2 + ysize2, xsize3, ysize3, xsize1 + xsize2 - over_left_center_x - over_right_center_x, err_left_center_y + abs(err_right_center_y), datamosaic, xmosaic, ymosaic);

			if (lError != 0)
				return lError;
			Level1Proc_ViewJointFeather(datamosaic, xmosaic, ymosaic, edge_position);
			fwrite(datamosaic, 2, xmosaic*ymosaic, fDst);
			fflush(fDst);
			//GDALRasterIO(GDALGetRasterBand(m_datasetOut, i + 1), GF_Write, 0, 0, xmosaic, ymosaic, datamosaic, xmosaic, ymosaic, GDT_UInt16, 0, 0);
		}
	}
	if (err_left_center_y<0 && err_right_center_y>0)
	{
		//左视场低右视场高
		for (int i = 0; i<bands; ++i)
		{
			printf("joint band: %d\n", i + 1);
			GDALRasterIO(GDALGetRasterBand(m_datasetLeft, i + 1), GF_Read, 0, 0, xsize1, ysize1, imgViewData, xsize1, ysize1, GDT_UInt16, 0, 0);
			GDALRasterIO(GDALGetRasterBand(m_datasetCenter, i + 1), GF_Read, 0, 0, xsize2, ysize2, imgViewData + xsize1*ysize1, xsize2, ysize2, GDT_UInt16, 0, 0);
			GDALRasterIO(GDALGetRasterBand(m_datasetRight, i + 1), GF_Read, 0, 0, xsize3, ysize3, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize3, ysize3, GDT_UInt16, 0, 0);
			Level1Proc_ColorMatch(imgViewData, imgViewData + xsize1*ysize1, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize1, ysize1, xsize2, ysize2, xsize3, ysize3, over_left_center_x, over_right_center_x, err_left_center_y, err_right_center_y);

			lError = Level1Proc_ViewJointFillData(imgViewData, xsize1, ysize1, 0, abs(err_left_center_y) + abs(err_right_center_y), datamosaic, xmosaic, ymosaic);
			lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1, xsize2, ysize2, xsize1 - over_left_center_x, err_right_center_y, datamosaic, xmosaic, ymosaic);
			lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize3, ysize3, xsize1 + xsize2 - over_left_center_x - over_right_center_x, 0, datamosaic, xmosaic, ymosaic);

			if (lError != 0)
				return lError;
			Level1Proc_ViewJointFeather(datamosaic, xmosaic, ymosaic, edge_position);
			fwrite(datamosaic, 2, xmosaic*ymosaic, fDst);
			fflush(fDst);
			//GDALRasterIO(GDALGetRasterBand(m_datasetOut, i + 1), GF_Write, 0, 0, xmosaic, ymosaic, datamosaic, xmosaic, ymosaic, GDT_UInt16, 0, 0);
		}
	}
	if (err_left_center_y == 0 && err_right_center_y == 0)
	{
		for (int i = 0; i<bands; ++i)
		{
			printf("joint band: %d\n", i + 1);
			GDALRasterIO(GDALGetRasterBand(m_datasetLeft, i + 1), GF_Read, 0, 0, xsize1, ysize1, imgViewData, xsize1, ysize1, GDT_UInt16, 0, 0);
			GDALRasterIO(GDALGetRasterBand(m_datasetCenter, i + 1), GF_Read, 0, 0, xsize2, ysize2, imgViewData + xsize1*ysize1, xsize2, ysize2, GDT_UInt16, 0, 0);
			GDALRasterIO(GDALGetRasterBand(m_datasetRight, i + 1), GF_Read, 0, 0, xsize3, ysize3, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize3, ysize3, GDT_UInt16, 0, 0);
			Level1Proc_ColorMatch(imgViewData, imgViewData + xsize1*ysize1, imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize1, ysize1, xsize2, ysize2, xsize3, ysize3, over_left_center_x, over_right_center_x, err_left_center_y, err_right_center_y);

			lError = Level1Proc_ViewJointFillData(imgViewData, xsize1, ysize1, 0, 0, datamosaic, xmosaic, ymosaic);
			lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1, xsize2, ysize2, xsize1 - over_left_center_x, 0, datamosaic, xmosaic, ymosaic);
			lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1 + xsize2*ysize2, xsize3, ysize3, xsize1 + xsize2 - over_left_center_x - over_right_center_x, 0, datamosaic, xmosaic, ymosaic);
			if (lError != 0)
				return lError;
			Level1Proc_ViewJointFeather(datamosaic, xmosaic, ymosaic, edge_position);
			fwrite(datamosaic, 2, xmosaic*ymosaic, fDst);
			fflush(fDst);
			//GDALRasterIO(GDALGetRasterBand(m_datasetOut, i + 1), GF_Write, 0, 0, xmosaic, ymosaic, datamosaic, xmosaic, ymosaic, GDT_UInt16, 0, 0);
		}
	}
	//GDALClose(m_datasetOut);
	//m_datasetOut = NULL;
	return 0;
}
long QPDLevel1Process::Level1Proc_ViewJointFillData(unsigned short* imgViewData, int xsize1, int ysize1, int xsize2, int ysize2, float over_x, float err_y, GDALDatasetH m_datasetLeft, GDALDatasetH m_datasetRight,
	unsigned short* datamosaic, FILE* fDst, int xmosaic, int ymosaic, vector<CPOINT>& edge_position)
{
	if (datamosaic == NULL)
		return -1;
	int bands = GDALGetRasterCount(m_datasetLeft);
	long lError = 0;
	//左视场高
	if (err_y>0)
	{
		for (int i = 0; i<bands; ++i)
		{
			GDALRasterIO(GDALGetRasterBand(m_datasetLeft, i + 1), GF_Read, 0, 0, xsize1, ysize1, imgViewData, xsize1, ysize1, GDT_UInt16, 0, 0);
			GDALRasterIO(GDALGetRasterBand(m_datasetRight, i + 1), GF_Read, 0, 0, xsize2, ysize2, imgViewData + xsize1*ysize1, xsize2, ysize2, GDT_UInt16, 0, 0);
			Level1Proc_ColorMatch(imgViewData, imgViewData + xsize1*ysize1, xsize1, ysize1, xsize2, ysize2, over_x, err_y);

			lError = Level1Proc_ViewJointFillData(imgViewData, xsize1, ysize1, 0, 0, datamosaic, xmosaic, ymosaic);
			lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1, xsize2, ysize2, xsize1 - over_x, err_y, datamosaic, xmosaic, ymosaic);

			if (lError != 0)
				return lError;
			Level1Proc_ViewJointFeather(datamosaic, xmosaic, ymosaic, edge_position);
			fwrite(datamosaic, 2, xmosaic*ymosaic, fDst);
			fflush(fDst);
			//GDALRasterIO(GDALGetRasterBand(m_datasetOut, i + 1), GF_Write, 0, 0, xmosaic, ymosaic, datamosaic, xmosaic, ymosaic, GDT_UInt16, 0, 0);

		}
	}
	else
	{
		for (int i = 0; i<bands; ++i)
		{
			GDALRasterIO(GDALGetRasterBand(m_datasetLeft, i + 1), GF_Read, 0, 0, xsize1, ysize1, imgViewData, xsize1, ysize1, GDT_UInt16, 0, 0);
			GDALRasterIO(GDALGetRasterBand(m_datasetRight, i + 1), GF_Read, 0, 0, xsize2, ysize2, imgViewData + xsize1*ysize1, xsize2, ysize2, GDT_UInt16, 0, 0);
			Level1Proc_ColorMatch(imgViewData, imgViewData + xsize1*ysize1, xsize1, ysize1, xsize2, ysize2, over_x, err_y);

			lError = Level1Proc_ViewJointFillData(imgViewData, xsize1, ysize1, 0, abs(err_y), datamosaic, xmosaic, ymosaic);
			lError = Level1Proc_ViewJointFillData(imgViewData + xsize1*ysize1, xsize2, ysize2, xsize1 - over_x, 0, datamosaic, xmosaic, ymosaic);

			if (lError != 0)
				return lError;
			Level1Proc_ViewJointFeather(datamosaic, xmosaic, ymosaic, edge_position);
			fwrite(datamosaic, 2, xmosaic*ymosaic, fDst);
			fflush(fDst);
			//GDALRasterIO(GDALGetRasterBand(m_datasetOut, i + 1), GF_Write, 0, 0, xmosaic, ymosaic, datamosaic, xmosaic, ymosaic, GDT_UInt16, 0, 0);

		}
	}
	return lError;
}
long QPDLevel1Process::Level1Proc_ViewJointFillData(unsigned short* imgViewData, int xsize, int ysize, float stposx, float stposy, unsigned short* jointData, int xmosaic, int ymosaic)
{
	//第一个像素对准，第二个像素以后插值
	int int_start_posx = int(stposx);
	int int_start_posy = int(stposy);
	float err_y = stposy - (float)int_start_posy;

	if (err_y<float(1) / float(20))
	{
		for (int i = 0; i<xsize; ++i)
			for (int j = 0; j<ysize; ++j)
			{
				jointData[(j + int_start_posy)*xmosaic + i + int_start_posx] = imgViewData[j*xsize + i];
			}
	}
	else
	{
		//取整的第一行
		for (int i = 0; i<xsize; ++i)
		{
			jointData[(int_start_posy)*xmosaic + i + int_start_posx] = imgViewData[i];
		}
		for (int i = 0; i<xsize; ++i)
		{
			for (int j = 1; j<ysize; ++j)
			{
				jointData[(int_start_posy + j)*xmosaic + i + int_start_posx] = unsigned short(err_y*float(imgViewData[(j - 1)*xsize + i]) + (1.0f - err_y)*float(imgViewData[j*xsize + i]));
			}
		}
	}
	return 0;
}
//重叠区域直方图匹配进行色调调整
long QPDLevel1Process::Level1Proc_ColorMatch(unsigned short* imgBuffer1, unsigned short* imgBuffer2, unsigned short* imgBuffer3, int xsize1, int ysize1, int xsize2, int ysize2, int xsize3, int ysize3,
	int over_left_center_x, int over_right_center_x, int err_left_center_y, int err_right_center_y)
{
	//重叠范围数据
	int ysize_left_center = min(ysize1, ysize2);
	int ysize_right_center = min(ysize2, ysize3);

	double *overlap12 = NULL; 
	double *overlap21 = NULL;
	double *overlap23 = NULL;
	double *overlap32 = NULL;
	try
	{
		overlap12 = new double[over_left_center_x*(ysize_left_center - abs(err_left_center_y))];
		overlap21 = new double[over_left_center_x*(ysize_left_center - abs(err_left_center_y))];

		overlap23 = new double[over_right_center_x*(ysize_right_center - abs(err_right_center_y))];
		overlap32 = new double[over_right_center_x*(ysize_right_center - abs(err_right_center_y))];
	}
	catch (bad_alloc)
	{
		printf("allocate memory error\n");
		exit(-1);
	}

	//设置初始值
	memset(overlap12, 0, sizeof(double)*over_left_center_x*(ysize_left_center - abs(err_left_center_y)));
	memset(overlap21, 0, sizeof(double)*over_left_center_x*(ysize_left_center - abs(err_left_center_y)));

	memset(overlap32, 0, sizeof(double)*over_right_center_x*(ysize_right_center - abs(err_right_center_y)));
	memset(overlap23, 0, sizeof(double)*over_right_center_x*(ysize_right_center - abs(err_right_center_y)));

	//左中影像重叠区域像素值
	for (int i = 0; i<over_left_center_x; ++i)
	{
		for (int j = 0; j<(ysize_left_center - abs(err_left_center_y)); ++j)
		{
			if (err_left_center_y>0)
			{
				overlap12[j*over_left_center_x + i] = imgBuffer1[j*xsize1 + xsize1 - over_left_center_x + i];
				overlap21[j*over_left_center_x + i] = imgBuffer2[(j + err_left_center_y)*xsize2 + i];
			}
			else
			{
				overlap12[j*over_left_center_x + i] = imgBuffer1[(j + abs(err_left_center_y))*xsize1 + xsize1 - over_left_center_x + i];
				overlap21[j*over_left_center_x + i] = imgBuffer2[j*xsize2 + i];
			}
		}
	}

	//中右影像重叠区域像素值
	for (int i = 0; i<over_right_center_x; ++i)
	{
		for (int j = 0; j<(ysize_right_center - abs(err_right_center_y)); ++j)
		{
			if (err_left_center_y>0)
			{
				overlap23[j*over_right_center_x + i] = imgBuffer2[j*xsize2 + xsize2 - over_right_center_x + i];
				overlap32[j*over_right_center_x + i] = imgBuffer3[(j + err_right_center_y)*xsize3 + i];
			}
			else
			{
				overlap23[j*over_right_center_x + i] = imgBuffer2[j*xsize2 + xsize2 - over_right_center_x + i];
				overlap32[j*over_right_center_x + i] = imgBuffer3[(j + abs(err_right_center_y))*xsize3 + i];
			}
		}
	}

	//获取重叠范围影像最大最小值
	int minPixel = 0x0000;
	int maxPixel = 0xffff + 1;
	int *mapPixel12 = NULL, *mapPixel23 = NULL;
	try 
	{
		mapPixel12 = new int[maxPixel];
		mapPixel23 = new int[maxPixel];
	}
	catch(bad_alloc)
	{
		printf("allocate memory error\n");
		exit(-1);
	}
	GetImgHistroMatch(overlap12, overlap21, over_left_center_x, ysize_left_center - abs(err_left_center_y), over_left_center_x, ysize_left_center - abs(err_left_center_y), minPixel, maxPixel, mapPixel12);
	GetImgHistroMatch(overlap32, overlap23, over_right_center_x, ysize_right_center - abs(err_right_center_y), over_right_center_x, ysize_right_center - abs(err_right_center_y), minPixel, maxPixel, mapPixel23);

	//考虑用OMP加速
//#pragma omp parallel for
	for (size_t i = 0; i < xsize1*ysize1; i++)
		imgBuffer1[i] = mapPixel12[imgBuffer1[i] - minPixel];
//#pragma omp parallel for
	for (size_t i = 0; i < xsize3*ysize3; i++)
		imgBuffer3[i] = mapPixel23[imgBuffer3[i] - minPixel];

	//清理内存
	delete[]mapPixel12;
	delete[]mapPixel23;
	delete[]overlap12;
	delete[]overlap21;
	delete[]overlap23;
	delete[]overlap32;

	return 0;
}
long QPDLevel1Process::Level1Proc_ColorMatch(unsigned short* imgBuffer1, unsigned short* imgBuffer2, int xsize1, int ysize1, int xsize2, int ysize2, int over_x, int err_y)
{
	long lError = 0;

	//重叠范围数据
	int ytemp = min(ysize1, ysize2);

	double *overlap12 = new double[over_x*(ytemp - abs(err_y))];
	double *overlap21 = new double[over_x*(ytemp - abs(err_y))];

	//设置初始值
	memset(overlap12, 0, sizeof(double)*over_x*(ytemp - abs(err_y)));
	memset(overlap21, 0, sizeof(double)*over_x*(ytemp - abs(err_y)));

	//左中影像重叠区域像素值
	for (int i = 0; i<over_x; ++i)
	{
		for (int j = 0; j<(ytemp - abs(err_y)); ++j)
		{
			if (err_y>0)
			{
				overlap12[j*over_x + i] = imgBuffer1[j*xsize1 + xsize1 - over_x + i];
				overlap21[j*over_x + i] = imgBuffer2[(j + err_y)*xsize2 + i];
			}
			else
			{
				overlap12[j*over_x + i] = imgBuffer1[(j + abs(err_y))*xsize1 + xsize1 - over_x + i];
				overlap21[j*over_x + i] = imgBuffer2[j*xsize2 + i];
			}
		}
	}

	//获取重叠范围影像最大最小值
	int minPixel = 0x0000;
	int maxPixel = 0xffff + 1;
	int *mapPixel=NULL;
	try {
		mapPixel = new int[maxPixel+1];
	}
	catch (bad_alloc)
	{
		printf("allocate memory error\n");
		exit(-1);
	}
	GetImgHistroMatch(overlap12, overlap21, over_x, ytemp - abs(err_y), over_x, ytemp - abs(err_y), minPixel, maxPixel, mapPixel);
#pragma omp parallel for
	for (int i = 0; i < xsize1*ysize1; i++)
		imgBuffer1[i] = mapPixel[imgBuffer1[i]];

	////是不是考虑再做一次校正
	//double a12[2];
	//double mean1 = 0, mean2 = 0, sum1 = 0, sum2 = 0;
	//for (size_t i = 0; i < over_x*(ytemp - abs(err_y)); i++)
	//{
	//	mean1 += double(overlap12[i]) / double(over_x*(ytemp - abs(err_y)));
	//	mean2 += double(overlap21[i]) / double(over_x*(ytemp - abs(err_y)));
	//}
	//for (int i = 0; i<over_x*(ytemp - abs(err_y)); ++i)
	//	sum1 += overlap12[i] * overlap21[i] / mean1 / mean2 - 1;
	//for (int i = 0; i<over_x*(ytemp - abs(err_y)); ++i)
	//	sum2 += overlap12[i] * overlap12[i] / mean1 / mean1 - 1;
	//a12[0] = mean2 / mean1*sum1 / sum2;
	//a12[1] = mean2 - a12[0] * mean1;
	//for (int i = 0; i<xsize1*ysize1; ++i)
	//	imgBuffer1[i] = unsigned short(a12[0] * double(imgBuffer1[i]) + a12[1]);


	delete[]overlap12;
	delete[]overlap21;
	return lError;
}
//===================================================================================================================================================================================================

//接口函数
long THRLevel1Process::Level1Proc_SpectarlCalibTIR(const char* hyperModtran, const char* bandInfo, const char* fileImg,
	const char* plank_line_coeff, float bin, float begWaveLength, float endWaveLength,
	const char* pathShift)
{
	if (endWaveLength<begWaveLength)
		return -1;
	if (bin <= 0)
		return -1;

	//获取影像信息
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");	//中文路径
	GDALAllRegister();
	GDALDataset* m_dataset = (GDALDataset*)GDALOpen(fileImg, GA_ReadOnly);
	if (m_dataset == NULL)
		return -1;

	int bandsNumber = 256;
	int imgSamples, imgLines;
	imgSamples = m_dataset->GetRasterXSize();
	imgLines = m_dataset->GetRasterYSize();
	bandsNumber = m_dataset->GetRasterCount();
	int imgSelLine = 300;	//减小数据量只读取300行

	//数据空间计算
	int n_bin = int(((endWaveLength - begWaveLength) / bin) + 1);
	long lError = 0;

	FILE* fModtrans = NULL;
	FILE* fBandInfo = NULL;

	//所有数据空间的申请
	float* nWLens = NULL;
	float *hyperSpectral = NULL;
	float *band_waveLength = NULL;
	float *band_FWHM = NULL;
	float **img_data = NULL;
	float **optParam = NULL;
	float *plank_coef = NULL;
	try
	{
		nWLens = new float[n_bin];				  //MODTRAN 模拟光谱每个波段的波长
		for (int i = 0; i<n_bin; i++)
			nWLens[i] = i*bin + begWaveLength;
		hyperSpectral = new float[n_bin];		  //MODTRAN 模拟光谱的辐亮度
		band_waveLength = new float[bandsNumber];//影像实验室定标的中心波长
		band_FWHM = new float[bandsNumber];	  //影像实验室定标的半波宽
		img_data = new float*[bandsNumber];	  //影像数据空间
		for (int i = 0; i<bandsNumber; i++)
			img_data[i] = new float[imgSamples*imgSelLine];
		optParam = new float*[imgSamples];	   //解算得到的光谱定标参数（每一列有一个定标参数）
		for (int i = 0; i<imgSamples; i++)
			optParam[i] = new float[2];
		plank_coef = new float[2 * bandsNumber];	   //光谱辐射校正普朗克校正系数

	}
	catch (bad_alloc* e)
	{
		lError = -1;
		goto Err_End;
	}

	int iterator_param1 = 0;
	char szTemp[1024];

	//读取MODTRAN模拟的反射率和辐亮度
	if (fopen_s(&fModtrans, hyperModtran, "r")!=0)
	{
		lError = -1;
		goto Err_End;
	}
	//跳过11行
	for (int i = 0; i < 11; i++)
		fgets(szTemp, 1024, fModtrans);
	//ifs.getline(szTemp, 1024);

	//读取数据
	float dataHyper[16];
	for (int i = 0; i<n_bin; i++)
	{
		fscanf_s(fModtrans, "%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f", &dataHyper[0], &dataHyper[1], &dataHyper[2], &dataHyper[3], &dataHyper[4],
			&dataHyper[5], &dataHyper[6], &dataHyper[7], &dataHyper[8], &dataHyper[9],
			&dataHyper[10], &dataHyper[11], &dataHyper[12], &dataHyper[13], &dataHyper[14],
			&dataHyper[15]);
		hyperSpectral[i] = dataHyper[8] * 1000.0f;
	}
	fclose(fModtrans); fModtrans = NULL;


	//读取中心波长信息和半波宽
	iterator_param1 = 0;
	if (fopen_s(&fBandInfo, bandInfo, "r") != 0)
	{
		lError = -1;
		goto Err_End;
	}
	do
	{
		fscanf_s(fBandInfo, "%f%f", &band_waveLength[iterator_param1], &band_FWHM[iterator_param1]);
		iterator_param1++;
	} while (!feof(fBandInfo) && iterator_param1<bandsNumber);
	fclose(fBandInfo); fBandInfo = NULL;

	if (iterator_param1 != bandsNumber)
	{
		lError = -1;
		goto Err_End;
	}

	//读取普朗克线性化系数
	//跳过第一行
	FILE *fPlank = NULL;
	if (fopen_s(&fPlank, plank_line_coeff,"r")!=0)
	{
		lError = -1;
		goto Err_End;
	}
	fgets(szTemp, 1024, fPlank);
	for (int i = 0; i<bandsNumber; i++)
	{
		fscanf_s(fPlank, "%f%f", &plank_coef[2 * i + 0], &plank_coef[2 * i + 1]);
	}
	fclose(fPlank); fPlank = NULL;

	//大气吸收峰
	float wlen_min = 11.65;		//自己定义的水汽吸收峰
	float wlen_max = 11.825;

	int index_min, index_max;
	int sel_number_band;		//大气吸收峰的波段数目
	for (int i = 0; i<bandsNumber; i++)
	{
		if (band_waveLength[i] <= wlen_max)
			index_max = i;
		if (band_waveLength[i] <= wlen_min)
			index_min = i;
	}
	index_min++;

	//读取影像数据
	for (int i = 0; i<bandsNumber; i++)
		GDALRasterIO(GDALGetRasterBand(m_dataset, i + 1), GF_Read, 0, imgLines - imgSelLine, imgSamples,
		imgSelLine, img_data[i], imgSamples, imgSelLine, GDT_Float32, 0, 0);
	GDALClose((GDALDatasetH)m_dataset);

	//转换为辐亮度
	for (int i = 0; i<bandsNumber; i++)
	{
		for (int j = 0; j<imgSamples*imgSelLine; j++)
			img_data[i][j] = img_data[i][j] * plank_coef[2 * i + 1] + plank_coef[2 * i + 0];
	}

	//针对每一列进行光谱定标
	//每一列的光谱定标数据	
	int totalproc = 0;
#pragma omp parallel for
	for (int j = 0; j<imgSamples; j++)
	{
		totalproc++;
		float *spectral_slice = new float[bandsNumber];  //用做处理的列均值光谱
		memset(spectral_slice, 0, sizeof(float)*bandsNumber);

		printf("\rprocess sample %d", totalproc);
		for (int i = 0; i<bandsNumber; i++)
		{
			for (int k = 0; k<imgSelLine; k++)
				spectral_slice[i] += img_data[i][k*imgSamples + j];
		}
		for (int i = 0; i<bandsNumber; i++)
			spectral_slice[i] /= imgSelLine;

		Level1Proc_Powell(optParam[j], hyperSpectral, nWLens, n_bin, band_waveLength, band_FWHM, spectral_slice, bandsNumber, index_min, index_max);
		delete[]spectral_slice;
	}
	printf("\n");

	FILE* fShift = NULL;
	fopen_s(&fShift, pathShift, "w");
	for (int i = 0; i < imgSamples; i++)
		fprintf(fShift, "%lf  %lf\n", optParam[i][0], optParam[i][1]);
	fclose(fShift); fShift = NULL;

	//清除指针并退出
Err_End:
	if (nWLens != NULL)
		delete[]nWLens;
	if (hyperSpectral != NULL)
		delete[]hyperSpectral;
	if (band_waveLength != NULL)
		delete[]band_waveLength;
	if (band_FWHM != NULL)
		delete[]band_FWHM;
	if (img_data != NULL)
	{
		for (int i = 0; i<bandsNumber; i++)
			delete[]img_data[i];
		delete[]img_data;
	}

	if (optParam != NULL)
	{
		for (int i = 0; i<imgSamples; i++)
			delete[]optParam[i];
		delete[]optParam;
	}
	if (plank_coef != NULL)
		delete[]plank_coef;
	return lError;
}
long THRLevel1Process::Level1Proc_SpectarlCalibTIRSample(const char* hyperModtran, const char* bandInfo, const char* fileImg,
	const char* plank_line_coeff, float bin, float begWaveLength,
	float endWaveLength, int sampleNum, const char* pathShift)
{
	if (endWaveLength<begWaveLength)
		return -1;
	if (bin <= 0)
		return -1;

	//获取影像信息
	CPLSetConfigOption("GDAL_FILENAME_IS_UTF8", "NO");	//中文路径
	GDALAllRegister();
	GDALDataset* m_dataset = (GDALDataset*)GDALOpen(fileImg, GA_ReadOnly);
	if (m_dataset == NULL)
		return -1;

	int bandsNumber = 256;
	int imgSamples, imgLines;
	imgSamples = m_dataset->GetRasterXSize();
	imgLines = m_dataset->GetRasterYSize();
	bandsNumber = m_dataset->GetRasterCount();
	int imgSelLine = 300;	//减小数据量只读取300行

	//数据空间计算
	int n_bin = int(((endWaveLength - begWaveLength) / bin) + 1);
	long lError = 0;
	int sample_interval = int(imgSamples / sampleNum);			//采样间隔

	FILE* fModtrans = NULL;
	FILE* fBandInfo = NULL;

	//所有数据空间的申请
	float *nWLens = NULL;
	float *hyperSpectral = NULL;
	float *band_waveLength = NULL;
	float *band_FWHM = NULL;
	float **img_data = NULL;
	float **optParam = NULL;
	float *plank_coef = NULL;
	try
	{
		nWLens = new float[n_bin];					//MODTRAN 模拟光谱每个波段的波长
		for (int i = 0; i<n_bin; i++)
			nWLens[i] = i*bin + begWaveLength;
		hyperSpectral = new float[n_bin];				//MODTRAN 模拟光谱的辐亮度
		band_waveLength = new float[bandsNumber];		//影像实验室定标的中心波长
		band_FWHM = new float[bandsNumber];			//影像实验室定标的半波宽
		img_data = new float*[bandsNumber];			//影像数据空间
		for (int i = 0; i<bandsNumber; i++)
			img_data[i] = new float[imgSamples*imgSelLine];
		optParam = new float*[sampleNum];			//解算得到的光谱定标参数（每一列有一个定标参数）
		for (int i = 0; i<sampleNum; i++)
			optParam[i] = new float[2];
		plank_coef = new float[2 * bandsNumber];		//光谱辐射校正普朗克校正系数

	}
	catch (bad_alloc* e)
	{
		lError = -1;
		goto Err_End;
	}

	int iterator_param1 = 0;
	char szTemp[1024];

	//读取MODTRAN模拟的反射率和辐亮度
	//读取MODTRAN模拟的反射率和辐亮度
	if (fopen_s(&fModtrans, hyperModtran, "r") != 0)
	{
		lError = -1;
		goto Err_End;
	}
	//跳过11行
	for (int i = 0; i < 11; i++)
		fgets(szTemp, 1024, fModtrans);
	//ifs.getline(szTemp, 1024);

	//读取数据
	float dataHyper[16];
	for (int i = 0; i<n_bin; i++)
	{
		fscanf_s(fModtrans, "%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f%f", &dataHyper[0], &dataHyper[1], &dataHyper[2], &dataHyper[3], &dataHyper[4],
			&dataHyper[5], &dataHyper[6], &dataHyper[7], &dataHyper[8], &dataHyper[9],
			&dataHyper[10], &dataHyper[11], &dataHyper[12], &dataHyper[13], &dataHyper[14],
			&dataHyper[15]);
		hyperSpectral[i] = dataHyper[8] * 1000.0f;
	}
	fclose(fModtrans); fModtrans = NULL;


	//读取中心波长信息和半波宽
	iterator_param1 = 0;
	if (fopen_s(&fBandInfo, bandInfo, "r") != 0)
	{
		lError = -1;
		goto Err_End;
	}
	do
	{
		fscanf_s(fBandInfo, "%f%f", &band_waveLength[iterator_param1], &band_FWHM[iterator_param1]);
		iterator_param1++;
	} while (!feof(fBandInfo) && iterator_param1<bandsNumber);
	fclose(fBandInfo); fBandInfo = NULL;

	if (iterator_param1 != bandsNumber)
	{
		lError = -1;
		goto Err_End;
	}

	//读取普朗克线性化系数
	//跳过第一行
	FILE *fPlank = NULL;
	if (fopen_s(&fPlank, plank_line_coeff, "r") != 0)
	{
		lError = -1;
		goto Err_End;
	}
	fgets(szTemp, 1024, fPlank);
	for (int i = 0; i<bandsNumber; i++)
	{
		fscanf_s(fPlank, "%f%f", &plank_coef[2 * i + 0], &plank_coef[2 * i + 1]);
	}
	fclose(fPlank); fPlank = NULL;

	//大气吸收峰
	float wlen_min = 11.65;		//自己定义的水汽吸收峰
	float wlen_max = 11.825;

	int index_min, index_max;
	int sel_number_band;		//大气吸收峰的波段数目
	for (int i = 0; i<bandsNumber; i++)
	{
		if (band_waveLength[i] <= wlen_max)
			index_max = i;
		if (band_waveLength[i] <= wlen_min)
			index_min = i;
	}
	index_min++;

	//读取影像数据
	for (int i = 0; i<bandsNumber; i++)
		GDALRasterIO(GDALGetRasterBand(m_dataset, i + 1), GF_Read, 0, imgLines - imgSelLine, imgSamples,
		imgSelLine, img_data[i], imgSamples, imgSelLine, GDT_Float32, 0, 0);
	GDALClose((GDALDatasetH)m_dataset);

	//转换为辐亮度
	for (int i = 0; i<bandsNumber; i++)
	{
		for (int j = 0; j<imgSamples*imgSelLine; j++)
			img_data[i][j] = img_data[i][j] * plank_coef[2 * i + 1] + plank_coef[2 * i + 0];
	}

	//针对每一列进行光谱定标
	//每一列的光谱定标数据	
	int totalproc = 0;
#pragma omp parallel for
	for (int j = 0; j<sampleNum; j++)
	{
		totalproc++;
		float *spectral_slice = new float[bandsNumber];  //用做处理的列均值光谱
		memset(spectral_slice, 0, sizeof(float)*bandsNumber);

		printf("\rprocess sample %d", totalproc);
		for (int i = 0; i<bandsNumber; i++)
		{
			for (int k = 0; k<imgSelLine; k++)
				spectral_slice[i] += img_data[i][k*imgSamples + j*sample_interval];
		}
		for (int i = 0; i<bandsNumber; i++)
			spectral_slice[i] /= imgSelLine;

		Level1Proc_Powell(optParam[j], hyperSpectral, nWLens, n_bin, band_waveLength, band_FWHM, spectral_slice, bandsNumber, index_min, index_max);
		delete[]spectral_slice;
	}
	printf("\n");


	FILE* fShift = NULL;
	fopen_s(&fShift, pathShift, "w");
	for (int i = 0; i < imgSamples; i++)
		fprintf(fShift, "%d  %lf  %lf\n", i*sample_interval,optParam[i][0], optParam[i][1]);
	fclose(fShift); fShift = NULL;

	//清除指针并退出
Err_End:
	if (optParam != NULL)
	{
		for (int i = 0; i<sampleNum; i++)
			delete[]optParam[i];
		delete[]optParam;
	}
	if (nWLens != NULL)
		delete[]nWLens;
	if (hyperSpectral != NULL)
		delete[]hyperSpectral;
	if (band_waveLength != NULL)
		delete[]band_waveLength;
	if (band_FWHM != NULL)
		delete[]band_FWHM;
	if (img_data != NULL)
	{
		for (int i = 0; i<bandsNumber; i++)
			delete[]img_data[i];
		delete[]img_data;
	}
	if (plank_coef != NULL)
		delete[]plank_coef;
	return lError;
}

//=============================================================================================================================================
//内部接口
long THRLevel1Process::Level1Proc_MODTRAN_Batch(const char* tap_name, const char* modtranDir, char* SURREF1, float V11, float V21)
{
	if (tap_name == NULL)
		return -1;

	//Card1：21个参数
	char MODTRAN = 'T';
	char SPEED = 's';
	char BINARY = ' ';
	char LYMOLC = ' ';
	char MODEL = '2';
	char T_BEST = ' ';
	char *ModelT = "2";
	char ITYPE[10]; sprintf_s(ITYPE, "%4s", "2");
	char IEMSCT[10]; sprintf_s(IEMSCT, "%5s", "2");
	char IMULT[10]; sprintf_s(IMULT, "%5s", "1");
	char M1[10]; sprintf_s(M1, "%5s", ModelT);
	char M2[10]; sprintf_s(M2, "%5s", ModelT);
	char M3[10]; sprintf_s(M3, "%5s", ModelT);
	char M4[10]; sprintf_s(M4, "%5s", ModelT);
	char M5[10]; sprintf_s(M5, "%5s", ModelT);
	char M6[10]; sprintf_s(M6, "%5s", ModelT);
	char MDEF[10]; sprintf_s(MDEF, "%5s", "0");
	char I_RD2C[10]; sprintf_s(I_RD2C, "%5s", "0");
	char CKPRNT = ' ';
	char NOPRNT[10]; sprintf_s(NOPRNT, "%4s", "0");
	char TPTEMP[20]; sprintf_s(TPTEMP, "% 8s", "300.000");
	char SURREF[20]; sprintf_s(SURREF, "% 7s", "0.000");

	//Card1A：22个参数
	char DIS = 't';
	char DISAZM = 't';
	char DISALB = ' ';
	char NSTR[10]; sprintf_s(NSTR, "%3s", "8");
	char SFWM[10]; sprintf_s(SFWM, "%4s", "0");
	char CO2MX[20]; sprintf_s(CO2MX, "%10s", "390.00000");
	char H2OSTR[20]; sprintf_s(H2OSTR, "%10s", "1.00000");
	char O3STR[20]; sprintf_s(O3STR, "%10s", "1.00000");
	char C_PROF = ' ';
	char LSUNFL = 'f';
	char* LBMNAM = " t";
	char* LFLTNM = " f";
	char* H2OAER = " t";
	char* CDTDIR = "  ";
	char SLEVEL = ' ';
	char SOLCON[20]; sprintf_s(SOLCON, "%9s", "0");
	char CDASTM = ' ';
	char ASTMC[20]; sprintf_s(ASTMC, "%9s", " ");
	char ASTMX[20]; sprintf_s(ASTMX, "%10s", "0");
	char ASTMO[20]; sprintf_s(ASTMO, "%10s", " ");
	char AERRH[20]; sprintf_s(AERRH, "%10s", " ");
	char NSSALB[20]; sprintf_s(NSSALB, "%10s", "0");

	//card1A2：1个参数
	char* BMNAME = "p1_2009";

	//card2：14个参数
	char* APLUS = "  ";
	char IHAZE[10]; sprintf_s(IHAZE, "%3s", "4");
	char CNOVAM = ' ';
	char ISEASN[10]; sprintf_s(ISEASN, "%4s", "0");
	char ARUSS[10]; sprintf_s(ARUSS, "%3s", " ");
	char IVULCN[10]; sprintf_s(IVULCN, "%2s", "0");
	char ICSTL[10]; sprintf_s(ICSTL, "%5s", "0");
	char ICLD[10]; sprintf_s(ICLD, "%5s", "0");
	char IVSA[10]; sprintf_s(IVSA, "%5s", "0");
	char VIS[20]; sprintf_s(VIS, "%10s", "30.00000");
	char WSS[20]; sprintf_s(WSS, "%10s", "0.00000");
	char WHH[20]; sprintf_s(WHH, "%10s", "0.00000");
	char RAINRT[20]; sprintf_s(RAINRT, "%10s", "0.00000");
	char GNDALT[20]; sprintf_s(GNDALT, "%10s", "0.00300");

	//card3：9个参数（默认2c选项）
	char H1ALT[20]; sprintf_s(H1ALT, "%10s", "2.003");
	char H2ALT[20]; sprintf_s(H2ALT, "%10s", "0.003");
	char OBSZEN[20]; sprintf_s(OBSZEN, "%10s", "0.000");
	char HRANGE[20]; sprintf_s(HRANGE, "%10s", "2.000");
	char BETA[20];  sprintf_s(BETA, "%10s", "0.000");
	char RAD_E[20];  sprintf_s(RAD_E, "%10s", "0.000");
	char LENN[10];  sprintf_s(LENN, "%5s", "0");
	char BCKZEN[30]; sprintf_s(BCKZEN, "%15s", "0.000");
	char CKRANG[20]; sprintf_s(CKRANG, "%10s", "0.000");

	//card3A1：4个参数
	char IPARM[10]; sprintf_s(IPARM, "%5s", "1");
	char IPH[10]; sprintf_s(IPH, "%5s", "2");
	char IDAY[10]; sprintf_s(IDAY, "%5s", "157");
	char ISOURC[10]; sprintf_s(ISOURC, "%5s", "0");

	//card3A2：8个参数
	char PARM1[20]; sprintf_s(PARM1, "%10s", "29.93642");
	char PARM2[20]; sprintf_s(PARM2, "%10s", "237.64168");
	char PARM3[20]; sprintf_s(PARM3, "%10s", "0.00000");
	char PARM4[20]; sprintf_s(PARM4, "%10s", "0.00000");

	char GMTIME[20]; sprintf_s(GMTIME, "%10s", "8.00000");
	char TRUEAZ[20]; sprintf_s(TRUEAZ, "%10s", "0.00000");
	char ANGLEM[20]; sprintf_s(ANGLEM, "%10s", "0.00000");
	char G[20]; sprintf_s(G, "%10s", "0.00000");

	//card4：16个参数
	char V1[20]; sprintf_s(V1, "%10s", "0.320");
	char V2[20]; sprintf_s(V2, "%10s", "2.600");
	char DV[20]; sprintf_s(DV, "%10s", "0.001");
	char FWHM[20]; sprintf_s(FWHM, "%10s", "0.002");
	char YFLAG = 't';
	char XFLAG = 'm';
	char DLIMIT[20]; sprintf_s(DLIMIT, "%8s", " ");
	char FLAGS11 = 'm';
	char FLAGS22 = '2';
	char FLAGS33 = 'a';
	char FLAGS44 = 'a';
	char FLAGS55 = ' ';
	char FLAGS66 = ' ';
	char FLAGS77 = ' ';
	char MLFLX[20]; sprintf_s(MLFLX, "%10s", " ");
	char VRFRAC[20]; sprintf_s(VRFRAC, "%10s", " ");

	//card5：1个参数
	char IRPT[10]; sprintf_s(IRPT, "%5s", "0");

	//重新设置参数
	sprintf_s(SURREF, "%7s", SURREF1);
	sprintf_s(V1, "%10.3f", V11);
	sprintf_s(V2, "%10.3f", V21);

	char modExeIn1[256];
	sprintf_s(modExeIn1, "%s\\%s", modtranDir, tap_name);
	ofstream ofs(modExeIn1);
	if (!ofs.is_open())
		return -1;

	//card1 参数输出
	ofs << MODTRAN << SPEED << BINARY << LYMOLC << MODEL << T_BEST << ITYPE << IEMSCT <<
		IMULT << M1 << M2 << M3 << M4 << M5 << M6 << MDEF <<
		I_RD2C << CKPRNT << NOPRNT << TPTEMP << SURREF << "     " << "!card1" << endl;
	//card1A 参数输出
	ofs << DIS << DISAZM << DISALB << NSTR << SFWM << CO2MX << H2OSTR << O3STR <<
		C_PROF << LSUNFL << LBMNAM << LFLTNM << H2OAER << CDTDIR << SLEVEL <<
		SOLCON << CDASTM << ASTMC << ASTMX << ASTMO << AERRH << NSSALB << "     " << "!card1A" << endl;
	//card1A2 参数输出
	ofs << BMNAME << endl;
	//card2 参数输出
	ofs << APLUS << IHAZE << CNOVAM << ISEASN << ARUSS << IVULCN << ICSTL << ICLD <<
		IVSA << VIS << WSS << WHH << RAINRT << GNDALT << "     " << "!card2" << endl;
	//card3 参数输出
	ofs << H1ALT << H2ALT << OBSZEN << HRANGE << BETA << RAD_E << LENN << BCKZEN << CKRANG << "     " << "!card3" << endl;
	//card3A1 参数输出
	ofs << IPARM << IPH << IDAY << ISOURC << "     " << "!card3A1" << endl;
	//card3A2 参数输出
	ofs << PARM1 << PARM2 << PARM3 << PARM4 << GMTIME << TRUEAZ << ANGLEM << G << "     " << "!card3A2" << endl;
	//card4  参数输出
	ofs << V1 << V2 << DV << FWHM << YFLAG << XFLAG << DLIMIT << FLAGS11 << FLAGS22 << FLAGS33 << FLAGS44 <<
		FLAGS55 << FLAGS66 << FLAGS77 << MLFLX << VRFRAC << "     " << "!card4" << endl;
	//card5 参数输出
	ofs << IRPT << "     " << "!card5" << endl;
	ofs.close();

	char modExeIn2[256];
	sprintf_s(modExeIn2, "%s\\%s", modtranDir, "mod5root.in");
	ofs.open(modExeIn2);
	if (!ofs.is_open())
		return -1;
	ofs << modExeIn1 << endl;
	ofs.close();

	//调用MODTRAN生成模拟光谱
	printf("execute MODTRAN simulate the spectral\n");
	char cmdLine[1024];
	sprintf_s(cmdLine, "%s\\%s", modtranDir, "Mod53qwin.exe");
	system(cmdLine);
	return 0;
}
long THRLevel1Process::Level1Proc_Simulate(float *hyperRadiance, float* hyperWaveLength, float *center_wave_Length, int hyperNum,
	float *band_FWHM, int bandNum, float shift_lamda, float delta_shift_lamda, float* simuluate_radiance)
{
	//判断输入是否有误
	if (hyperRadiance == NULL || hyperWaveLength == NULL ||
		center_wave_Length == NULL || band_FWHM == NULL ||
		simuluate_radiance == NULL)
		return -1;

	//获取模拟光谱
	memset(simuluate_radiance, 0, sizeof(float)*bandNum);
	float* rsp = new float[hyperNum];
	for (int i = 0; i<bandNum; i++)
	{
		for (int j = 0; j<hyperNum; j++)
			rsp[j] = exp(-pow((hyperWaveLength[j] - center_wave_Length[i] - shift_lamda / 1000.0f), 2) / (pow(band_FWHM[i] + delta_shift_lamda / 1000, 2) / 2.7735));

		for (int j = 0; j<hyperNum; j++)
		{
			if (rsp[j]<0.0001)
				rsp[j] = 0;
		}
		float tmp1 = 0, tmp2 = 0;
		for (int j = 0; j<hyperNum; j++)
		{
			tmp1 += rsp[j] * hyperRadiance[j];
			tmp2 += rsp[j];
		}
		simuluate_radiance[i] = tmp1 / tmp2*1000.0f;
	}
	delete[]rsp;

	return 0;
}
long  THRLevel1Process::Level1Proc_NODD(float* radiance, int index_min, int index_max, float* radiance_NODD)
{
	//判断数据输入是否正确
	if (radiance == NULL || radiance_NODD == NULL)
		return -1;

	//数据空间申请
	float *tmpRadiacneNODD1 = new float[index_max - index_min + 1];
	float *tmpRadiacneNODD2 = new float[index_max - index_min];

	//1.辐亮度光谱取负对数
	for (int i = index_min; i <= index_max; i++)
		tmpRadiacneNODD1[i - index_min] = -log(radiance[i]);

	//2.取负对数后求导，得到微分光谱（一阶差分）
	for (int i = index_min; i <= index_max - 1; i++)
		tmpRadiacneNODD2[i - index_min] = tmpRadiacneNODD1[i - index_min + 1] - tmpRadiacneNODD1[i - index_min];

	//3.对一阶微分光谱进行正态变换
	float tmpmean = 0, tmpstdvar = 0;
	for (int i = 0; i<index_max - index_min; i++)
		tmpmean += tmpRadiacneNODD2[i];
	tmpmean /= (index_max - index_min);
	for (int i = 0; i<index_max - index_min; i++)
		tmpstdvar += (tmpRadiacneNODD2[i] - tmpmean)*(tmpRadiacneNODD2[i] - tmpmean);
	tmpstdvar /= (index_max - index_min);
	tmpstdvar = sqrt(tmpstdvar);
	for (int i = 0; i<(index_max - index_min); i++)
		radiance_NODD[i] = (tmpRadiacneNODD2[i] - tmpmean) / tmpstdvar;

	return 0;
}
float THRLevel1Process::Level1Proc_MeritFunc(float* simulateRadiance, float* realRadiance, int index_min, int index_max)
{
	//检测输入是否正确
	if (simulateRadiance == NULL || realRadiance == NULL)
		return -1;

	float* simulate_radiance_NODD = new float[index_max - index_min];
	float* real_radiance_NODD = new float[index_max - index_min];

	long lError = 0;
	lError = Level1Proc_NODD(simulateRadiance, index_min, index_max, simulate_radiance_NODD);
	if (lError != 0)
		goto ErrEnd;
	lError = Level1Proc_NODD(realRadiance, index_min, index_max, real_radiance_NODD);
	if (lError != 0)
		goto ErrEnd;

	float gamma = 0.5;
	float tmpSD = 0, tmpSA = 0;
	for (int i = 0; i<index_max - index_min; i++)
		tmpSD += (simulate_radiance_NODD[i] - real_radiance_NODD[i])*(simulate_radiance_NODD[i] - real_radiance_NODD[i]);
	tmpSD /= (index_max - index_min);

	float tmp1 = 0, tmp2 = 0, tmp3 = 0;
	for (int j = 0; j<index_max - index_min; j++)
	{
		tmp1 += simulate_radiance_NODD[j] * real_radiance_NODD[j];
		tmp2 += simulate_radiance_NODD[j] * simulate_radiance_NODD[j];
		tmp3 += real_radiance_NODD[j] * real_radiance_NODD[j];
	}

	tmpSA = PI / 2 * acos(tmp1 / sqrt(tmp2*tmp3));
	delete[]simulate_radiance_NODD;
	delete[]real_radiance_NODD;
	return (1 - gamma)*tmpSD + gamma*tmpSA;

ErrEnd:
	delete[]simulate_radiance_NODD;
	delete[]real_radiance_NODD;
	return -1.0f;
}
long  THRLevel1Process::Level1Proc_Powell(float* optParam, float* hyperSpectral, float *hyperWavelength, int hyperNum,
	float* bandCenterLength, float* bandFWHM, float* realSpectral, int realNum,
	int index_min, int index_max)
{
	//检查数据输入
	if (optParam == NULL || hyperSpectral == NULL ||
		hyperWavelength == NULL || bandCenterLength == NULL ||
		bandFWHM == NULL || realSpectral == NULL)
		return -1;

	//设置初始值为0 初始方向 误差限 迭代次数
	optParam[0] = optParam[1] = 0;
	float iniDirect1[2], iniDirect2[2];
	iniDirect1[0] = 1; iniDirect1[1] = 0;
	iniDirect2[0] = 0; iniDirect2[1] = 1;

	float errThres = 0.0001;
	float merit = 0;

	float lamdaStep = 10.0f;		//步长
	float belta = 0.1;		    //收缩因子
	float acclerFactor = 1.0f;	//加速因子

	int   iteratorNum = 0;
	long  lError = 0;
	//开始进行迭代求解
	float *simulate_radiance = new float[realNum];
	//第一次求解得到模拟光谱和误差
	lError = Level1Proc_Simulate(hyperSpectral, hyperWavelength, bandCenterLength, hyperNum, bandFWHM, realNum, optParam[0], optParam[1], simulate_radiance);

	////测试模拟光谱的求解是否正确：输出到文件中绘图
	//ofstream ofs("temp.txt");
	//for (int i=0;i<realNum;i++)
	//	ofs<<bandCenterLength[i]<<" "<<simulate_radiance[i]<<endl;
	//ofs.close();

	if (lError != 0)
	{
		delete[]simulate_radiance;
		return lError;
	}

	merit = Level1Proc_MeritFunc(simulate_radiance, realSpectral, index_min, index_max);

	//在这里采用模式搜索方法求解
	float merit_pre = 0;
	float tmpParam1[2], tmpParam2[2], preParam[2], tmpParam[2];
	float merit1, merit2;
	bool  isLastSmaller = false;
	float lamdaStepPre = lamdaStep;
	merit_pre = merit;

	//初始化
	tmpParam[0] = optParam[0];
	tmpParam[1] = optParam[1];
	do
	{
		iteratorNum++;
		preParam[0] = optParam[0];
		preParam[1] = optParam[1];
		isLastSmaller = false;

		//x方向正向
		tmpParam1[0] = tmpParam[0] + lamdaStep*iniDirect1[0];
		tmpParam1[1] = tmpParam[1] + lamdaStep*iniDirect1[1];
		lError = Level1Proc_Simulate(hyperSpectral, hyperWavelength, bandCenterLength, hyperNum, bandFWHM, realNum, tmpParam1[0], tmpParam1[1], simulate_radiance);
		merit1 = Level1Proc_MeritFunc(simulate_radiance, realSpectral, index_min, index_max);
		//x方向负向
		tmpParam2[0] = tmpParam[0] - lamdaStep*iniDirect1[0];
		tmpParam2[1] = tmpParam[1] - lamdaStep*iniDirect1[1];
		lError = Level1Proc_Simulate(hyperSpectral, hyperWavelength, bandCenterLength, hyperNum, bandFWHM, realNum, tmpParam2[0], tmpParam2[1], simulate_radiance);
		merit2 = Level1Proc_MeritFunc(simulate_radiance, realSpectral, index_min, index_max);
		if (merit1<merit_pre)
		{
			tmpParam[0] = tmpParam1[0];
			tmpParam[1] = tmpParam1[1];
			merit_pre = merit1;
			isLastSmaller = true;
		}
		else if (merit2<merit_pre)
		{
			tmpParam[0] = tmpParam2[0];
			tmpParam[1] = tmpParam2[1];
			merit_pre = merit2;
			isLastSmaller = true;
		}

		//y方向正向
		tmpParam1[0] = tmpParam[0] + lamdaStep*iniDirect2[0];
		tmpParam1[1] = tmpParam[1] + lamdaStep*iniDirect2[1];
		lError = Level1Proc_Simulate(hyperSpectral, hyperWavelength, bandCenterLength, hyperNum, bandFWHM, realNum, tmpParam1[0], tmpParam1[1], simulate_radiance);
		merit1 = Level1Proc_MeritFunc(simulate_radiance, realSpectral, index_min, index_max);
		//y方向负向
		tmpParam2[0] = tmpParam[0] - lamdaStep*iniDirect2[0];
		tmpParam2[1] = tmpParam[1] - lamdaStep*iniDirect2[1];
		lError = Level1Proc_Simulate(hyperSpectral, hyperWavelength, bandCenterLength, hyperNum, bandFWHM, realNum, tmpParam2[0], tmpParam2[1], simulate_radiance);
		merit2 = Level1Proc_MeritFunc(simulate_radiance, realSpectral, index_min, index_max);
		if (merit1<merit_pre)
		{
			tmpParam[0] = tmpParam1[0];
			tmpParam[1] = tmpParam1[1];
			merit_pre = merit1;
			isLastSmaller = true;
		}
		else if (merit2<merit_pre)
		{
			tmpParam[0] = tmpParam2[0];
			tmpParam[1] = tmpParam2[1];
			merit_pre = merit2;
			isLastSmaller = true;
		}


		if (isLastSmaller)
		{
			optParam[0] = tmpParam[0];
			optParam[1] = tmpParam[1];

			//下一个参考点
			tmpParam[0] = 2 * optParam[0] - preParam[0];
			tmpParam[1] = 2 * optParam[1] - preParam[1];
			continue;
		}
		else
		{
			if (tmpParam[0] == preParam[0] && tmpParam[1] == preParam[1])
			{
				if (lamdaStep>errThres)
				{
					lamdaStepPre = lamdaStep;
					lamdaStep = lamdaStepPre*belta;
				}
				else
					break;
			}
			else
			{
				tmpParam[0] = preParam[0];
				tmpParam[1] = preParam[1];
			}
		}
	} while (true);

	//清除数据并退出
	delete[]simulate_radiance;
	return lError;
}

