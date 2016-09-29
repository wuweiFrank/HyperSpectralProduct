//各种头文件的定义
//author：	吴蔚
//version:	2.0
#include "stdafx.h"

#ifndef AERIAL_CORE_H_
#define AERIAL_CORE_H_
#include<string>
#include<vector>
using namespace std;

//定义常量
#define LOG_FILE_PATH 
#define PI 3.14159265358979323846264338
#define MAX_NUM  99999999
#define MIN_NUM -99999999
#define WGS84LRadius 6378137
#define WGS84Eccentricity 0.0066943850
#define EQUAL_LIMIT 0.000001
const double LIMIT = 0.00003;
#define OVER_LIMIT( var) fabs(var)>LIMIT
#define ISEQUAL(a,b) (fabs(a-b)) < EQUAL_LIMIT  ? true : false

//点数据结构的定义
//二维点和三维点数据结构
struct CPOINT
{
	int x;
	int y;
};
struct DPOINT
{
	double dX;
	double dY;
};
typedef struct FPOINT
{
	float x;
	float y;
}Pointf;
struct THREEDPOINT
{
	double dX;
	double dY;
	double dZ;
};
typedef struct F3POINT
{
	float x;
	float y;
	float z;
}Point3f;
struct MatchPair
{
	int img1_idx;		//影像的编号
	int img2_idx;

	//vector<int> pnt_idx;//匹配点的编号2*i+0，2*i+1;
};

//EO和PO和低昂对定向元素数据结构的定义
typedef struct stExteriorOrientation
{
	//三个线元素
	double m_dX;
	double m_dY;
	double m_dZ;

	//三个角元素
	double m_phia;
	double m_omega;
	double m_kappa;

	//旋转矩阵
	double m_dRMatrix[9];

	void PrintStruct()
	{
		printf("线元素：(%lf)——(%lf)——(%lf)\n",m_dX,m_dY,m_dZ);
		printf("角元素：(%lf)——(%lf)——(%lf)\n",m_phia,m_omega,m_kappa);
		printf("旋转矩阵:\n");
		printf("(%lf)-(%lf)-(%lf)\n",m_dRMatrix[0],m_dRMatrix[1],m_dRMatrix[2]);
		printf("(%lf)-(%lf)-(%lf)\n",m_dRMatrix[3],m_dRMatrix[4],m_dRMatrix[5]);
		printf("(%lf)-(%lf)-(%lf)\n",m_dRMatrix[6],m_dRMatrix[7],m_dRMatrix[8]);
	}
}EO;
typedef struct stPositOrientationSys
{
	//经纬度和高度
	double m_latitude;
	double m_longitude;
	double m_height;

	//侧滚、俯仰和偏航
	double m_roll;
	double m_pitch;
	double m_yaw;
}POS;
typedef struct stRelativeOrientation
{
	//三个线元素
	double m_Bx;
	double m_By;
	double m_Bz;
	//三个角元素
	double m_phia;
	double m_omega;
	double m_kappa;

	//旋转矩阵
	double m_dRMatrix[9];
	void PrintStruct()
	{
		printf("线元素：(%lf)——(%lf)——(%lf)\n",m_Bx,m_By,m_Bz);
		printf("角元素：(%lf)——(%lf)——(%lf)\n",m_phia,m_omega,m_kappa);
		printf("旋转矩阵:\n");
		printf("(%lf)-(%lf)-(%lf)\n",m_dRMatrix[0],m_dRMatrix[1],m_dRMatrix[2]);
		printf("(%lf)-(%lf)-(%lf)\n",m_dRMatrix[3],m_dRMatrix[4],m_dRMatrix[5]);
		printf("(%lf)-(%lf)-(%lf)\n",m_dRMatrix[6],m_dRMatrix[7],m_dRMatrix[8]);
	}
}REO;
typedef struct stSetupGroundControl
{
	//影像检校的控制点
	int m_Img_xPnt;
	int m_Img_yPnt;
	//WGS84椭球UTM坐标系下的坐标
	double m_Ground_xUTMPnt;
	double m_Ground_yUTMPnt;
}SetupGCP;

//航空高光谱数据头信息
typedef struct stDataInfo
{
	unsigned int	nSensorType;
	unsigned int	nSensorOrder;
	unsigned int	nWidths;
	unsigned int	nSamples;
	unsigned int	nLines;
	unsigned int	nBands;
	unsigned int	nYear;
	unsigned int	nMonth;
	unsigned int	nDay;
	unsigned int	nSecond;
	unsigned int	nMode;
	__int64 nHeadOffset;
	__int64 nEofOffset;
}DINFO;

struct MapInfo {
	string projection;	//投影
	string directions;  //方向
	double adfGeotransform[6];//坐标信息
	int    zone;
};
typedef struct stImgHeader {
	//影像基本信息
	int imgWidth;
	int imgHeight;
	int imgBands;

	bool byteorder;	//字节顺序 一般是0 对于某些操作系统字节可能由小到大
	int  datatype;

	bool    bimgGeoProjection;	//是否具有投影信息
	MapInfo mapInfo;			//投影字段
	string coordianteSys;	//坐标字段

	string interleave;		//影像排布方式

	//默认显示波段
	bool bimgDefaultBands;
	vector<int> imgDefaultBands;

	//波段名
	bool bimgBandNames;
	vector<string> imgBandNnames;

	//波长信息
	bool bimgWaveLength;
	vector<float> imgWaveLength;

	//半波宽信息
	bool bimgFWHM;
	vector<float> imgFWHM;

	//写头文件信息包括：1.只写基础的头文件信息；2.写存在的头文件信息；3.写所有头文件信息
	bool WriteHeaderBasic(const char* pathHeader);
	bool WriteHeaderExist(const char* pathHeader);
	bool WriteHeaderTotal(const char* pathHeader);

}ENVIHeader;

//SBET文件结构,一般来说要从SBET文件中求解出POS，不过好像不一定能够用得到可以直接用软件
typedef struct stSbetElement
{
	double dGpsTime;
	double dLatitude;
	double dLongitude;
	double dHeight;
	double dVx;
	double dVy;
	double dVz;
	double dRoll;
	double dPitch;
	double dHeading;
	double dWander;
	double dAx;
	double dAy;
	double dAz;
	double dArx;
	double dAry;
	double dArz;
}SBETELEMENT;

//源数据以及订单文件数据解析参数
struct ParamMeta
{
	int m_nSensorType;	
	int m_nSensorOrder;
	int m_nWidth;
	int m_nSamples;
	int m_nLines;
	int m_nBands;
	int m_nYear,m_nMonth,m_nDay,m_nSecond;
	__int64 m_nHeadOffset;
	__int64 m_nEofOffset;
	bool m_bDark;
	int m_DarkLine;
};
struct ParamPre
{
	//输入路径
	vector<string> m_pRawFile;
	vector<string> m_pDarkFile;

	//输出路径
	vector<string> m_pD0File;
	vector<string> m_pD0DarkFile;
	vector<string> m_pD0XMLFile;
	vector<string> m_pD0JPG;

	bool m_bSegment;		//是否进行分景
	int  m_nSegBeginLine;	//分景的起始行
	int  m_nSegEndLine;		//分景的终止行
	int  m_nAutoSegLine;	//自动分景的行数

	bool m_bQuickView;		//是否有快视图
	bool m_bGrey;			//是否为灰图
	float m_fScale;			//快视图缩放因子
	int m_nUVBand[3];		//紫外快视图波段
	int m_nVNIRBand[3];		//可见近红外快视图波段
	int m_nSWIRBand[3];		//短波红外快视图波段
	int m_nTIHBand[3];		//热红外快视图波段

	string m_pProFile;		//处理进度文件
};
struct ParamP0
{
	vector<string> m_pD0File;	//0级数据
	vector<string> pD0MetaXML;	//0级数据源文件
	vector<string> pD0DarkFile;
	vector<string> m_pP0Product;
	vector<string> m_pP0ProductXML;
	vector<string> m_pP0JPG;
	vector<string> m_pEvent;
	bool m_bQuickView;
	bool m_bGrey;
	float m_fScale; 
	int m_nUVBand[3];
	int m_nVNIRBand[3]; 
	int m_nSWIRBand[3]; 
	int m_nTIHBand[3];
	string m_pProFile;

};
struct ParamP1
{
	vector<string> m_pD0File;	//0级数据
	vector<string> pD0MetaXML;	//0级数据源文件
	vector<string> pD0DarkFile;
	short m_nP0,  m_nP1A,  m_nP1B,  m_nP1C,  m_nP1D,  m_nP1E;
	vector<string> m_pP0Product;
	vector<string> m_pP0ProductXML;
	vector<string> m_pP0JPG;
	vector<string> m_pP1AData;
	vector<string> m_pP1AXML;
	vector<string> m_pP1AJPG;

	vector<string> m_pP1BData;
	vector<string> m_pP1BXML;
	vector<string> m_pP1BJPG; 

	vector<string> m_pP1CData; 
	vector<string> m_pP1CXML; 
	vector<string> m_pP1CJPG;

	vector<string> m_pP1DData; 
	vector<string> m_pP1DXML; 
	vector<string> m_pP1DJPG; 

	vector<string> m_pP1EData; 
	vector<string> m_pP1EXML; 
	vector<string> m_pP1EJPG;

	vector<string> &m_pReleCof; 
	vector<string> &m_pAbCof;
	string m_pModtran; 
	vector<string> m_pWaveLen; 
	vector<string> m_pEvent;
	float m_fFOLapX[8],  m_fFOLapY[8],  m_fSOLapX[3],  m_fSOLapY[3];
	bool m_bQuickView;
	bool m_bGrey;
	float m_fScale; 
	int m_nUVBand[3];
	int m_nVNIRBand[3]; 
	int m_nSWIRBand[3]; 
	int m_nTIHBand[3];
	string m_pProFile;
};
struct ParamP2
{
	vector<string> m_pD0File;	//0级数据
	vector<string> pD0MetaXML;	//0级数据源文件
	vector<string> pD0DarkFile;
	short m_nP0,  m_nP1A,  m_nP1B,  m_nP1C,  m_nP1D,  m_nP1E;
	vector<string> m_pP0Product;
	vector<string> m_pP0ProductXML;
	vector<string> m_pP0JPG;
	vector<string> m_pP1AData;
	vector<string> m_pP1AXML;
	vector<string> m_pP1AJPG;

	vector<string> m_pP1BData;
	vector<string> m_pP1BXML;
	vector<string> m_pP1BJPG; 

	vector<string> m_pP1CData; 
	vector<string> m_pP1CXML; 
	vector<string> m_pP1CJPG;

	vector<string> m_pP1DData; 
	vector<string> m_pP1DXML; 
	vector<string> m_pP1DJPG; 

	vector<string> m_pP1EData; 
	vector<string> m_pP1EXML; 
	vector<string> m_pP1EJPG;

	vector<string> m_pP2AData; 
	vector<string> m_pP2AXML; 
	vector<string> m_pP2AJPG;

	vector<string> m_pP2BData; 
	vector<string> m_pP2BXML; 
	vector<string> m_pP2BJPG;

	vector<string> m_pP2CData; 
	vector<string> m_pP2CXML; 
	vector<string> m_pP2CJPG;

	vector<string> &m_pReleCof; 
	vector<string> &m_pAbCof;
	string m_pModtran; 
	vector<string> m_pWaveLen; 
	vector<string> m_pEvent;
	string m_pSBETFile;
	vector<string> m_pPOSFile;
	vector<string> m_pEOFile;
	string m_pDEMFile;

	float m_fFOLapX[8],  m_fFOLapY[8],  m_fSOLapX[3],  m_fSOLapY[3];
	float m_fFov[4], m_fIFov[8], m_fFocalLen[4];
	double m_dBoresightMis[3], m_dGNSSOffset[3], m_dXYZOffset[3];
	int m_nBand[4], m_nSamples[4];
	bool m_bQuickView;
	bool m_bGrey;
	float m_fScale; 
	int m_nUVBand[3];
	int m_nVNIRBand[3]; 
	int m_nSWIRBand[3]; 
	int m_nTIHBand[3];
	string m_pProFile;

};


#endif