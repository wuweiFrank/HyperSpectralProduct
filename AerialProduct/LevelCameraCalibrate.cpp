#include "stdafx.h"
#include "LevelCameraCalibrate.h"
#include "CoordinateTrans.h"
#include "PosProc.h"

//=====================================================================================================================================================================
//从控制点中获取指定的POS
long SetupCalibProcess::SetupCalibProc_GetSelectPOS(const char* pathPOSFile,vector<SetupGCP> m_setup_GCPs,vector<POS> &m_selPOS)
{
	//读取指定行的POS数据
	int posHead = 1;
	//vector<POS> m_setup_Pos;
	//最好先把GCP按照y的值从小到大拍一个序，这样处理起来比较简单


	FILE   *fpin = NULL;
	int fprePos=0;
	double fReadData[20];
	char   cTempchar[2048];
	int realLines = 0;
	if ((fopen_s(&fpin,pathPOSFile, "r")) !=0 )
		return -1;

	//跳过文件头
	for (int i = 0; i < posHead; i++)
	{
		if (feof(fpin))
			return -1;
		fgets(cTempchar, 2048, fpin);
	}

	//实际的经纬度都是以弧度形式给出的
	for (int i = 0; i < m_setup_GCPs.size()&&!feof(fpin); i++)
	{
		if (feof(fpin))
			return -2;
		if(realLines<m_setup_GCPs[i].m_Img_yPnt)
		{
			while(realLines<m_setup_GCPs[i].m_Img_yPnt)
			{
				fgets(cTempchar, 2048, fpin);
				realLines++;
			}

			fprePos = ftell(fpin);
			fgets(cTempchar, 2048, fpin);
			int lError = sscanf_s(
				cTempchar,
				"%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
				&fReadData[0], &fReadData[1], &fReadData[2], &fReadData[3],
				&fReadData[4], &fReadData[5], &fReadData[6], &fReadData[7],
				&fReadData[8], &fReadData[9], &fReadData[10], &fReadData[11],
				&fReadData[12], &fReadData[13], &fReadData[14], &fReadData[15], &fReadData[16]);

			if (lError == 0)
				return lError;

			if (fReadData[2]*180/PI < -180.0 || fReadData[2] * 180 / PI > 180.0)
				return -13;

			if (fReadData[1] * 180 / PI <= -90.0 || fReadData[1] * 180 / PI >= 90.0)
				return -14;

			POS tempPos;
			tempPos.m_latitude = double(fReadData[1]);				 //X
			tempPos.m_longitude = double(fReadData[2]);				 //Y
			tempPos.m_height = double(fReadData[3]);                 //height
			tempPos.m_roll = double(fReadData[4]);					 //roll
			tempPos.m_pitch = double(fReadData[5]);					 //pitch
			tempPos.m_yaw = double(fReadData[6]);					 //heading
			m_selPOS.push_back(tempPos);

			realLines++;
		}
		else
		{
			realLines--;
			fseek(fpin,fprePos,SEEK_SET);
			fgets(cTempchar, 2048, fpin);
			int lError = sscanf_s(
				cTempchar,
				"%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf%lf",
				&fReadData[0], &fReadData[1], &fReadData[2], &fReadData[3],
				&fReadData[4], &fReadData[5], &fReadData[6], &fReadData[7],
				&fReadData[8], &fReadData[9], &fReadData[10], &fReadData[11],
				&fReadData[12], &fReadData[13], &fReadData[14], &fReadData[15], &fReadData[16]);

			if (lError == 0)
				return lError;

			if (fReadData[2]*180/PI < -180.0 || fReadData[2] * 180 / PI > 180.0)
				return -13;

			if (fReadData[1] * 180 / PI <= -90.0 || fReadData[1] * 180 / PI >= 90.0)
				return -14;

			POS tempPos;
			tempPos.m_latitude = double(fReadData[1]);				 //X
			tempPos.m_longitude = double(fReadData[2]);				 //Y
			tempPos.m_height = double(fReadData[3]);                 //height
			tempPos.m_roll = double(fReadData[4]);					 //roll
			tempPos.m_pitch = double(fReadData[5]);					 //pitch
			tempPos.m_yaw = double(fReadData[6]);					 //heading
			m_selPOS.push_back(tempPos);

			realLines++;
		}
	}
	return 0;
}

//从指定的POS数据中得到EO元素
long SetupCalibProcess::SetupCalibProc_ExtractSelectEO(vector<POS> m_selPOS,vector<EO> &m_selEO,THREEDPOINT &setupVec,float *setupAngle)
{
	//安置角和安置矢量
	THREEDPOINT ANGLETHETA;
	THREEDPOINT XYZPnt;
	CoordinateTransBasic m_coordinate;
	float pVector[3];
	ANGLETHETA.dX = setupAngle[0];  ANGLETHETA.dY = setupAngle[1];  ANGLETHETA.dZ = setupAngle[2];
	pVector[0] = setupVec.dX;  pVector[1] = setupVec.dY;   pVector[2] = setupVec.dZ;
	double dL,dB,dH;
	for(int i=0;i<(int)m_selPOS.size();++i)
	{
		dB = m_selPOS[i].m_latitude;
		dL = m_selPOS[i].m_longitude;
		dH = m_selPOS[i].m_height;
		//成图坐标系到地心坐标系
		double EMMatrix[9];
		EMMatrix[0] = -sin(dL);
		EMMatrix[1] = cos(dL);
		EMMatrix[2] = 0;

		EMMatrix[3] = -sin(dB)*cos(dL);
		EMMatrix[4] = -sin(dB)*sin(dL);
		EMMatrix[5] = cos(dB);

		EMMatrix[6] = cos(dB)*cos(dL);
		EMMatrix[7] = cos(dB)*sin(dL);
		EMMatrix[8] = sin(dB);

		//物方坐标系以测区的重心为原点, 方向为东(X)北(Y)天(Z)的局部切平面(大地高为0)坐标系
		m_coordinate.BLHToXYZ(dB, dL, 0, XYZPnt);
		double temps = m_selPOS[i].m_yaw*180.0 / PI;
		int nFlag;
		if (abs(int(temps)) % 360<5)					//North-East--1 quandrant
			nFlag = 1;
		else if (abs(int(temps - 90)) % 360<5)			//North_West--2 quandrant
			nFlag = 2;
		else if (abs(int(temps - 180)) % 360<5)			//South-West--3 quandrant
			nFlag = 3;
		else if (abs(int(temps - 270)) % 360<5)			//South-East--4 quandrant
			nFlag = 4;

		QPDGeoPOSProcess m_POS_Proc;
		EO tempEO;
		m_POS_Proc.GeoPOSProc_EOMatrixTurn(m_selPOS[i],XYZPnt,ANGLETHETA,pVector,nFlag,EMMatrix,tempEO);
		m_selEO.push_back(tempEO);
	}
	return 0;
}

//安置角检校
long SetupCalibProcess::SetupCalibProc_SetupAngle(const char* pathPOSFile,float ifov,vector<SetupGCP> m_setup_GCPs,float &fLen,THREEDPOINT setupVec,float *setupAngle)
{
	vector<POS> m_setup_POS;
	vector<EO>  m_setup_EO;

	//获取指定的POS数据
	SetupCalibProc_GetSelectPOS(pathPOSFile,m_setup_GCPs,m_setup_POS);

	//解算每一个POS的值
	SetupCalibProc_ExtractSelectEO(m_setup_POS,m_setup_EO,setupVec,setupAngle);

	//求导公式有点麻烦，以后需要的时候在进行推导好了
	return 0;
}
