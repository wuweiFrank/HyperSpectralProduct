#pragma once
#include"AerialCore.h"
#include <string>
#include<vector>
using namespace std;

class XMLParam
{
public:
	//原始数据元数据读取
	virtual long LoadProcMeta(const char* pMetaFile) = 0;

	//原始数据元数据写入
	virtual long SaveProcMeta(const char* pMetaFile,ParamMeta &m_Meta_Param) = 0;


	//解析数据预处理订单文件获取数据预处理参数
	virtual long LoadPreProcXML(const char* pCatalogOrderFile) = 0;

	//将数据预处理参数写入订单文件中
	virtual long SavePreProcXML(const char* pCatalogOrderFile,ParamPre &pPreProc_Param)=0;

	//解析0级数据处理订单文件获取数据0级数据订单文件参数
	virtual long LoadP0ProcXML(const char* pCatalogOrderFile) = 0;

	//将0级数据处理参数写入订单文件中
	virtual long SaveP0ProcXML(const char* pCatalogOrderFile,ParamP0 &pP0Proc_Param)=0;

	//解析1级数据处理订单文件获取数据1级数据处理订单文件
	virtual long LoadP1ProcXML(const char* pCatalogOrderFile) = 0;

	//将1级数据处理参数写入订单文件中
	virtual long SaveP1ProcXML(const char* pCatalogOrderFile,ParamP1 &pP1Proc_Param)=0;

	//解析2级数据处理订单文件获取数据预处理参数
	virtual long LoadP2ProcXML(const char* pCatalogOrderFile) = 0;

	//将2级数据处理参数写入订单文件中
	virtual long SaveP2ProcXML(const char* pCatalogOrderFile,ParamP2 &pP2Proc_Param)=0;

protected:
	ParamMeta		m_Meta_Param;
	ParamPre		m_PreProc_Param;
	ParamP0			m_P0Proc_Param;
	ParamP1			m_P1Proc_Param;
	ParamP2			m_P2Proc_Param;
};

//解析全谱段数据订单文件参数
class QPDXMLParam : public XMLParam
{
	friend class LevelPreProcess;
	friend class Level0Process;
	friend class Level1Process;
	friend class Level2Process;

public:
	//原始数据元数据读取
	virtual long LoadProcMeta(const char* pMetaFile);
	//原始数据元数据写入
	virtual long SaveProcMeta(const char* pMetaFile,ParamMeta &m_Meta_Param);

	//解析数据预处理订单文件获取数据预处理参数
	virtual long LoadPreProcXML(const char* pCatalogOrderFile);
	//将数据预处理参数写入订单文件中
	virtual long SavePreProcXML(const char* pCatalogOrderFile,ParamPre &pPreProc_Param);

	//解析0级数据处理订单文件获取数据0级数据订单文件参数
	virtual long LoadP0ProcXML(const char* pCatalogOrderFile);
	//将0级数据处理参数写入订单文件中
	virtual long SaveP0ProcXML(const char* pCatalogOrderFile,ParamP0 &pP0Proc_Param);

	//解析1级数据处理订单文件获取数据1级数据处理订单文件
	virtual long LoadP1ProcXML(const char* pCatalogOrderFile);
	//将1级数据处理参数写入订单文件中
	virtual long SaveP1ProcXML(const char* pCatalogOrderFile,ParamP1 &pP1Proc_Param);

	//解析2级数据处理订单文件获取数据预处理参数
	virtual long LoadP2ProcXML(const char* pCatalogOrderFile) ;
	//将2级数据处理参数写入订单文件中
	virtual long SaveP2ProcXML(const char* pCatalogOrderFile,ParamP2 &pP2Proc_Param);
};