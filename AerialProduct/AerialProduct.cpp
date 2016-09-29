// AerialProduct.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "CoordinateTrans.h"
#include "PosProc.h"
#include "XMLParam.h"
#include "LevelPreProcess.h"
#include "Level0Process.h"
#include "Level1Process.h"
#include "Level2Process.h"

#include "LevelUAVProcess.h"
#include "MosaicProcess.h"

void POSProcTest();
void LevelPreProcTest();
void Level0ProcTest();

int _tmain(int argc, _TCHAR* argv[])
{
	//POSProcTest();
	//LevelPreProcTest();
	//Level0ProcTest();

	//LevelUAVProcess UAVGeo;
	//UAVGeo.UAVGeoCorrect_Test();

	LevelFlightLineComputerVision flightAT;
	flightAT.LevelATProc_HomographTest();

	return 0;
}


void POSProcTest()
{
	char* pathSBet = "H:\\73\\auxfile\\20160225_GNSS_SBET.out";
	
	char* pathEvent1="H:\\73\\auxfile\\EV_0101_20160229_115734783.txt";
	char* pathEvent2="H:\\73\\auxfile\\EV_0401_20160229_115734783.txt";
	char* pathEvent3="H:\\73\\auxfile\\EV_0501_20160229_115734783.txt";
	char* pathEvent4="H:\\73\\auxfile\\EV_0601_20160229_115734783.txt";
	char* pathEvent5="H:\\73\\auxfile\\EV_0701_20160229_115734783.txt";
	
	char* pathPOS1="H:\\73\\auxfile\\POS_0101_20160229_115734783.txt";
	char* pathPOS2="H:\\73\\auxfile\\POS_0401_20160229_115734783.txt";
	char* pathPOS3="H:\\73\\auxfile\\POS_0501_20160229_115734783.txt";
	char* pathPOS4="H:\\73\\auxfile\\POS_0601_20160229_115734783.txt";
	char* pathPOS5="H:\\73\\auxfile\\POS_0701_20160229_115734783.txt";

	char* pathEO1="H:\\73\\auxfile\\EO_0101_20160229_115734783.txt";
	char* pathEO2="H:\\73\\auxfile\\EO_0401_20160229_115734783.txt";
	char* pathEO3="H:\\73\\auxfile\\EO_0501_20160229_115734783.txt";
	char* pathEO4="H:\\73\\auxfile\\EO_0601_20160229_115734783.txt";
	char* pathEO5="H:\\73\\auxfile\\EO_0701_20160229_115734783.txt";

	QPDGeoPOSProcess m_QPD_Pos;
	m_QPD_Pos.GeoPOSProc_ExtractSBET(pathSBet,pathEvent1,pathPOS1);
	m_QPD_Pos.GeoPOSProc_ExtractSBET(pathSBet,pathEvent2,pathPOS2);
	m_QPD_Pos.GeoPOSProc_ExtractSBET(pathSBet,pathEvent3,pathPOS3);
	m_QPD_Pos.GeoPOSProc_ExtractSBET(pathSBet,pathEvent4,pathPOS4);
	m_QPD_Pos.GeoPOSProc_ExtractSBET(pathSBet,pathEvent5,pathPOS5);

	THREEDPOINT theta;
	theta.dX=theta.dY=theta.dZ=0;
	float fSetupAngle[] ={0,0,0};
	m_QPD_Pos.GeoPOSProc_ExtractEO(pathPOS1,pathEO1,1000,2,theta,fSetupAngle);
	m_QPD_Pos.GeoPOSProc_ExtractEO(pathPOS2,pathEO2,1000,2,theta,fSetupAngle);
	m_QPD_Pos.GeoPOSProc_ExtractEO(pathPOS3,pathEO3,1000,2,theta,fSetupAngle);
	m_QPD_Pos.GeoPOSProc_ExtractEO(pathPOS4,pathEO4,1000,2,theta,fSetupAngle);
	m_QPD_Pos.GeoPOSProc_ExtractEO(pathPOS5,pathEO5,1000,2,theta,fSetupAngle);
}
void LevelPreProcTest()
{
	char* pathDataRAW1="H:\\73\\rawdata\\_0101_20160229_115734783.dat";
	char* pathDataRAW2="H:\\73\\rawdata\\_0301_20160229_115734783.dat";
	char* pathDataRAW3="H:\\73\\rawdata\\_0401_20160229_115734783.dat";
	char* pathDataRAW4="H:\\73\\rawdata\\_0501_20160229_115734783.dat";
	char* pathDataRAW5="H:\\73\\rawdata\\_0601_20160229_115734783.dat";
	char* pathDataRAW6="H:\\73\\rawdata\\_0701_20160229_115734783.dat";

	
	char* pathData1="H:\\73\\catalog\\D0_0101_20160229_115734783.dat";
	char* pathData2="H:\\73\\catalog\\D0_0301_20160229_115734783.dat";
	char* pathData3="H:\\73\\catalog\\D0_0401_20160229_115734783.dat";
	char* pathData4="H:\\73\\catalog\\D0_0501_20160229_115734783.dat";
	char* pathData5="H:\\73\\catalog\\D0_0601_20160229_115734783.dat";
	char* pathData6="H:\\73\\catalog\\D0_0701_20160229_115734783.dat";


	DINFO mHead1,mHead2,mHead3,mHead4,mHead5,mHead6;
	vector<short> mLeakType1,mLeakType2,mLeakType3,mLeakType4,mLeakType5,mLeakType6;
	vector<int> mLeakSize1,mLeakSize2,mLeakSize3,mLeakSize4,mLeakSize5,mLeakSize6;
	int mLeakCount1,mLeakCount2,mLeakCount3,mLeakCount4,mLeakCount5,mLeakCount6;

	QPDPreProcess m_QPD_Process;
	m_QPD_Process.PreProc_GenerateD0Data(pathDataRAW1,pathData1,mHead1,mLeakType1,mLeakSize1,mLeakCount1,100);
	m_QPD_Process.PreProc_GenerateD0Data(pathDataRAW2,pathData2,mHead2,mLeakType2,mLeakSize2,mLeakCount2,100);
	m_QPD_Process.PreProc_GenerateD0Data(pathDataRAW3,pathData3,mHead3,mLeakType3,mLeakSize3,mLeakCount3,100);
	m_QPD_Process.PreProc_GenerateD0Data(pathDataRAW4,pathData4,mHead4,mLeakType4,mLeakSize4,mLeakCount4,100);
	m_QPD_Process.PreProc_GenerateD0Data(pathDataRAW5,pathData5,mHead5,mLeakType5,mLeakSize5,mLeakCount5,100);
	m_QPD_Process.PreProc_GenerateD0Data(pathDataRAW6,pathData6,mHead6,mLeakType6,mLeakSize6,mLeakCount6,100);
}
void Level0ProcTest()
{
	QPDLevel0Process m_Level0Proc;
	char* pathDataRAW1="H:\\73\\rawdata\\_0101_20160229_115734783.dat";
	char* pathDataRAW2="H:\\73\\rawdata\\_0301_20160229_115734783.dat";
	char* pathDataRAW3="H:\\73\\rawdata\\_0401_20160229_115734783.dat";
	char* pathDataRAW4="H:\\73\\rawdata\\_0501_20160229_115734783.dat";
	char* pathDataRAW5="H:\\73\\rawdata\\_0601_20160229_115734783.dat";
	char* pathDataRAW6="H:\\73\\rawdata\\_0701_20160229_115734783.dat";

	char* pathData1="H:\\73\\catalog\\D0_0101_20160229_115734783.dat";
	char* pathData2="H:\\73\\catalog\\D0_0301_20160229_115734783.dat";
	char* pathData3="H:\\73\\catalog\\D0_0401_20160229_115734783.dat";
	char* pathData4="H:\\73\\catalog\\D0_0501_20160229_115734783.dat";
	char* pathData5="H:\\73\\catalog\\D0_0601_20160229_115734783.dat";
	char* pathData6="H:\\73\\catalog\\D0_0701_20160229_115734783.dat";

	char* pathEvent1 = "H:\\73\\product0\\EV_0101_20160229_115734783.txt";
	char* pathEvent2 = "H:\\73\\product0\\EV_0301_20160229_115734783.txt";
	char* pathEvent3 = "H:\\73\\product0\\EV_0401_20160229_115734783.txt";
	char* pathEvent4 = "H:\\73\\product0\\EV_0501_20160229_115734783.txt";
	char* pathEvent5 = "H:\\73\\product0\\EV_0601_20160229_115734783.txt";
	char* pathEvent6 = "H:\\73\\product0\\EV_0701_20160229_115734783.txt";

	char* pathProduct1="H:\\73\\product0\\P0_0101_20160229_115734783.dat";
	char* pathProduct2="H:\\73\\product0\\P0_0301_20160229_115734783.dat";
	char* pathProduct3="H:\\73\\product0\\P0_0401_20160229_115734783.dat";
	char* pathProduct4="H:\\73\\product0\\P0_0501_20160229_115734783.dat";
	char* pathProduct5="H:\\73\\product0\\P0_0601_20160229_115734783.dat";
	char* pathProduct6="H:\\73\\product0\\P0_0701_20160229_115734783.dat";

	m_Level0Proc.Level0Proc_ExtractEvent(pathData1,pathEvent1,17);
	m_Level0Proc.Level0Proc_ExtractEvent(pathData2,pathEvent2,17);
	m_Level0Proc.Level0Proc_ExtractEvent(pathData3,pathEvent3,17);
	m_Level0Proc.Level0Proc_ExtractEvent(pathData4,pathEvent4,17);
	m_Level0Proc.Level0Proc_ExtractEvent(pathData5,pathEvent5,17);
	m_Level0Proc.Level0Proc_ExtractEvent(pathData6,pathEvent6,17);

	m_Level0Proc.Level0Proc_RawToBSQ(pathData1,pathProduct1,0);
	m_Level0Proc.Level0Proc_RawToBSQ(pathData2,pathProduct2,0);
	m_Level0Proc.Level0Proc_RawToBSQ(pathData3,pathProduct3,0);
	m_Level0Proc.Level0Proc_RawToBSQ(pathData4,pathProduct4,0);
	m_Level0Proc.Level0Proc_RawToBSQ(pathData5,pathProduct5,0);
	m_Level0Proc.Level0Proc_RawToBSQ(pathData6,pathProduct6,0);
}