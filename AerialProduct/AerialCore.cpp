#include "stdafx.h"

#include "AerialCore.h"

bool stImgHeader::WriteHeaderBasic(const char* pathHeader)
{
	FILE* fpHead = NULL;
	errno_t err = fopen_s(&fpHead, pathHeader, "w");
	if (err != 0)
		exit(-1);

	if (!fpHead)
		exit(-1);

	fprintf(fpHead, "%s\n%s\n%s\n", "ENVI", "description = {", "File Imported into ENVI.}");
	fprintf(fpHead, "samples = %d\n", imgWidth);					//写入行像元数
	fprintf(fpHead, "lines = %d\n", imgHeight);				    //写入行数
	fprintf(fpHead, "bands = %d\n", imgBands);					//写入波段数
	fprintf(fpHead, "file type = %s\n", "ENVI Standard");				    //写入标示
	fprintf(fpHead, "data type = %d\n", datatype);
	fprintf(fpHead, "interleave = %s\n", interleave.c_str());
	fprintf(fpHead, "sensor type = %s\n", "Unknown");
	fprintf(fpHead, "byte order = %d\n", byteorder);
	fprintf(fpHead, "wavelength units = %s\n", "Unknown");

	fclose(fpHead);		//关闭文件
	return true;
}

bool ENVIHeader::WriteHeaderExist(const char* pathHeader)
{
	FILE* fpHead = NULL;
	errno_t err = fopen_s(&fpHead, pathHeader, "w");
	if (err != 0)
		exit(-1);

	if (!fpHead)
		exit(-1);

	fprintf(fpHead, "%s\n%s\n%s\n", "ENVI", "description = {", "File Imported into ENVI.}");
	fprintf(fpHead, "samples = %d\n", imgWidth);					//写入行像元数
	fprintf(fpHead, "lines = %d\n", imgHeight);				    //写入行数
	fprintf(fpHead, "bands = %d\n", imgBands);					//写入波段数
	fprintf(fpHead, "file type = %s\n", "ENVI Standard");				    //写入标示
	fprintf(fpHead, "data type = %d\n", datatype);
	fprintf(fpHead, "interleave = %s\n", interleave.c_str());
	fprintf(fpHead, "sensor type = %s\n", "Unknown");
	fprintf(fpHead, "byte order = %d\n", byteorder);
	fprintf(fpHead, "wavelength units = %s\n", "Unknown");

	//投影和坐标系统
	if (bimgGeoProjection)
	{
		fprintf(fpHead, "map info = { %s,", mapInfo.projection.c_str());
		fprintf(fpHead, "%lf,", mapInfo.adfGeotransform[1]);
		fprintf(fpHead, "%lf,", mapInfo.adfGeotransform[5]);
		fprintf(fpHead, "%lf,", mapInfo.adfGeotransform[0]);
		fprintf(fpHead, "%lf,", mapInfo.adfGeotransform[3]);
		fprintf(fpHead, "%lf,", mapInfo.adfGeotransform[2]);
		fprintf(fpHead, "%lf,", mapInfo.adfGeotransform[4]);
		fprintf(fpHead, "%d,", mapInfo.zone);
		fprintf(fpHead, "%s\n", mapInfo.directions.c_str());
		fprintf(fpHead, "%s\n", coordianteSys.c_str());
	}

	//波长
	if (bimgWaveLength)
	{
		fprintf(fpHead, "wavelength = {");
		for (int i = 0; i < (int)imgWaveLength.size(); ++i)
		{
			for (int j = 0; j + i < (int)imgWaveLength.size(); ++j)
				fprintf(fpHead, "%lf", imgWaveLength[j + i]);
			fprintf(fpHead, "\n");
		}
		fprintf(fpHead, "}\n");
	}

	//半波宽
	if (bimgFWHM)
	{
		fprintf(fpHead, "fwhm = {");
		for (int i = 0; i < (int)imgFWHM.size(); ++i)
		{
			for (int j = 0; j + i < (int)imgFWHM.size(); ++j)
				fprintf(fpHead, "%lf", imgFWHM[j + i]);
			fprintf(fpHead, "\n");
		}
		fprintf(fpHead, "}\n");
	}

	//波段名
	if (bimgBandNames)
	{
		fprintf(fpHead, "band names = {");
		for (int i = 0; i < (int)imgBandNnames.size(); ++i)
		{
			for (int j = 0; j + i < (int)imgBandNnames.size(); ++j)
				fprintf(fpHead, "%s", imgBandNnames[j + i].c_str());
			fprintf(fpHead, "\n");
		}
		fprintf(fpHead, "}\n");
	}

	fclose(fpHead);		//关闭文件
	return true;
}
