#include "StdAfx.h"
#include "XMLParam.h"
#include<Windows.h>

#include "rapidxml/rapidxml.hpp"
#include"rapidxml/rapidxml_iterators.hpp"
#include"rapidxml/rapidxml_print.hpp"
#include"rapidxml/rapidxml_utils.hpp"
using namespace rapidxml;

long QPDXMLParam::LoadProcMeta(const char* pMetaFile)
{
	long lError = 0;
	char *cSensorType, *cSensorOrder;
	char *cSamples, *cLines, *cBands;
	char *cHeadOffset;
	xml_node<>* tempnode;

	std::locale::global(std::locale(""));
	file<> fdoc(pMetaFile);

	xml_document<> doc;
	doc.parse<0>(fdoc.data());

	xml_node<>* root = doc.first_node();
	if (root == NULL)
	{
		lError = 1;
		return lError;
	}

	tempnode = root->first_node("Sensor");
	if (tempnode == NULL)
	{
		lError = 1;
		return lError;
	}

	tempnode = root->first_node("Producer");
	if (tempnode == NULL)
	{
		lError = 1;
		return lError;
	}

	tempnode = root->first_node("ProduceTime");
	if (tempnode == NULL)
	{
		lError = 1;
		return lError;
	}

	xml_node<> *datainfonode = root->first_node("DataInfo");
	if (datainfonode == NULL)
	{
		lError = 1;
		return lError;
	}

	xml_node<> *infonode;
	infonode = datainfonode->first_node("Format");
	if (infonode == NULL)
	{
		lError = 1;
		return lError;
	}

	infonode = datainfonode->first_node("Level");
	if (infonode == NULL)
	{
		lError = 1;
		return lError;
	}

	infonode = datainfonode->first_node("SensorType");
	if (infonode == NULL)
	{
		lError = 1;
		return lError;
	}
	cSensorType = infonode->value();

	infonode = datainfonode->first_node("SensorOrder");
	if (infonode == NULL)
	{
		lError = 1;
		return lError;
	}
	cSensorOrder = infonode->value();

	infonode = datainfonode->first_node("Samples");
	if (infonode == NULL)
	{
		lError = 1;
		return lError;
	}
	cSamples = infonode->value();

	infonode = datainfonode->first_node("Lines");
	if (infonode == NULL)
	{
		lError = 1;
		return lError;
	}
	cLines = infonode->value();

	infonode = datainfonode->first_node("Bands");
	if (infonode == NULL)
	{
		lError = 1;
		return lError;
	}
	cBands = infonode->value();

	infonode = datainfonode->first_node("HeadOffset");
	if (infonode == NULL)
	{
		lError = 1;
		return lError;
	}
	cHeadOffset = infonode->value();

	this->m_Meta_Param.m_nSensorType = (unsigned int)atoi(cSensorType);
	this->m_Meta_Param.m_nSensorOrder = (unsigned int)atoi(cSensorOrder);
	this->m_Meta_Param.m_nSamples = (unsigned int)atoi(cSamples);
	this->m_Meta_Param.m_nLines = (unsigned int)atoi(cLines);
	this->m_Meta_Param.m_nBands = (unsigned int)atoi(cBands);
	this->m_Meta_Param.m_nHeadOffset = _atoi64(cHeadOffset);

	return lError;
}
long QPDXMLParam::SaveProcMeta(const char* pMetaFile,ParamMeta &m_Meta_Param)
{
	long lError = 0;
	char cSensorType[50], cSensorOrder[50];
	char cWidths[50], cSamples[50], cLines[50], cBands[50];
	char cCTime[100], cPTime[100];
	char cHeadOffset[50];
	char cDark[50], cDarkLine[50];

	_itoa_s(m_Meta_Param.m_nSensorType, cSensorType, 10);
	_itoa_s(m_Meta_Param.m_nSensorOrder, cSensorOrder, 10);
	_itoa_s(m_Meta_Param.m_nWidth, cWidths, 10);
	_itoa_s(m_Meta_Param.m_nSamples, cSamples, 10);
	_itoa_s(m_Meta_Param.m_nLines, cLines, 10);
	_itoa_s(m_Meta_Param.m_nBands, cBands, 10);
	_itoa_s((int)m_Meta_Param.m_nHeadOffset, cHeadOffset, 10);
	_itoa_s(m_Meta_Param.m_bDark, cDark, 10);
	_itoa_s(m_Meta_Param.m_DarkLine, cDarkLine, 10);

	int nPYear = 0, nPMonth = 0, nPDay = 0;
	SYSTEMTIME ct;
	GetLocalTime(&ct);
	nPYear = ct.wYear;
	nPMonth = ct.wMonth;
	nPDay = ct.wDay;
	_itoa_s(m_Meta_Param.m_nYear * 10000 + m_Meta_Param.m_nMonth * 100 + m_Meta_Param.m_nDay, cCTime, 10);
	_itoa_s(m_Meta_Param.m_nYear * 10000 + m_Meta_Param.m_nMonth * 100 + m_Meta_Param.m_nDay, cPTime, 10);

	string text;
	ofstream out;

	std::locale::global(std::locale(""));
	xml_document<> doc;
	xml_node<>* root = doc.allocate_node(node_pi, doc.allocate_string("xml version='1.0'encoding='utf-8'"));
	doc.append_node(root);

	xml_node<>* node = doc.allocate_node(node_element, "D0Meta", NULL);
	doc.append_node(node);

	xml_node<>* sensornode = doc.allocate_node(node_element, "Sensor", "HKGF-3");
	node->append_node(sensornode);

	xml_node<>* producernode = doc.allocate_node(node_element, "Producer", "SITP");
	node->append_node(producernode);

	xml_node<>* ctimernode = doc.allocate_node(node_element, "CollectTime", cCTime);
	node->append_node(ctimernode);

	xml_node<>* ptimernode = doc.allocate_node(node_element, "ProduceTime", cPTime);
	node->append_node(ptimernode);

	xml_node<>* datainfonode = doc.allocate_node(node_element, "DataInfo");

	datainfonode->append_node(doc.allocate_node(node_element, "Format", "RAW"));
	datainfonode->append_node(doc.allocate_node(node_element, "SensorType", cSensorType));
	datainfonode->append_node(doc.allocate_node(node_element, "SensorOrder", cSensorOrder));
	datainfonode->append_node(doc.allocate_node(node_element, "Widths", cWidths));
	datainfonode->append_node(doc.allocate_node(node_element, "Samples", cSamples));
	datainfonode->append_node(doc.allocate_node(node_element, "Lines", cLines));
	datainfonode->append_node(doc.allocate_node(node_element, "Bands", cBands));
	datainfonode->append_node(doc.allocate_node(node_element, "HeadOffset", cHeadOffset));
	node->append_node(datainfonode);

	xml_node<>* darkinfonode = doc.allocate_node(node_element, "DarkInfo");
	darkinfonode->append_node(doc.allocate_node(node_element, "WhetherDark", cDark));
	darkinfonode->append_node(doc.allocate_node(node_element, "DarkLine", cDarkLine));
	node->append_node(darkinfonode);


	string str;
	rapidxml::print(std::back_inserter(str), doc, 0);
	ofstream out1(pMetaFile);
	out1 << doc;
	out1.close();

	return lError;
}

long QPDXMLParam::LoadPreProcXML(const char* pCatalogOrderFile)
{
	long lError = 0;
	char *cSeg = "";
	char *cAuto = "";
	char *cLine = "";
	char *cQuickView = "";
	char *cBand = "";
	char *buf = "";
	char *temp = "";
	char *cScale = "";
	int i = 0;

	std::locale::global(std::locale(""));
	file<> fdoc(pCatalogOrderFile);

	xml_document<> doc;
	doc.parse<0>(fdoc.data());

	xml_node<>* root = doc.first_node();
	if (root == NULL)
	{
		lError = 1;
		return lError;
	}

	//==============================获取输入数据===============================================
	xml_node<> *inputnode = root->first_node("InputFilelist");
	if (inputnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		//获取原始图像数据
		xml_node<>* RawData;
		RawData = inputnode->first_node("UVRawData1");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());
		RawData = inputnode->first_node("UVRawData2");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());
		RawData = inputnode->first_node("UVRawData3");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());
		RawData = inputnode->first_node("VNIRRawData1");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());
		RawData = inputnode->first_node("VNIRRawData2");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());
		RawData = inputnode->first_node("VNIRRawData3");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());
		RawData = inputnode->first_node("SWIRRawData1");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());
		RawData = inputnode->first_node("SWIRRawData2");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());
		RawData = inputnode->first_node("SWIRRawData3");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());
		RawData = inputnode->first_node("TIHRawData1");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());
		RawData = inputnode->first_node("TIHRawData2");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());
		RawData = inputnode->first_node("TIHRawData3");
		if (RawData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pRawFile.push_back(RawData->value());

		//获取暗电流数据
		xml_node<>* DarkData;
		DarkData = inputnode->first_node("UVDarkData1");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());
		DarkData = inputnode->first_node("UVDarkData2");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());
		DarkData = inputnode->first_node("UVDarkData3");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());
		DarkData = inputnode->first_node("VNIRDarkData1");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());
		DarkData = inputnode->first_node("VNIRDarkData2");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());
		DarkData = inputnode->first_node("VNIRDarkData3");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());
		DarkData = inputnode->first_node("SWIRDarkData1");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());
		DarkData = inputnode->first_node("SWIRDarkData2");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());
		DarkData = inputnode->first_node("SWIRDarkData3");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());
		DarkData = inputnode->first_node("TIHDarkData1");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());
		DarkData = inputnode->first_node("TIHDarkData2");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());
		DarkData = inputnode->first_node("TIHDarkData3");
		if (DarkData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(DarkData->value());

	}


	//==============================获取输出数据===============================================
	xml_node<> *outputnode = root->first_node("OutputFilelist");
	if (outputnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		//获取0级数据
		xml_node<>* D0Data;
		D0Data = outputnode->first_node("UVD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = outputnode->first_node("UVD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = outputnode->first_node("UVD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = outputnode->first_node("VNIRD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = outputnode->first_node("VNIRD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = outputnode->first_node("VNIRD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = outputnode->first_node("SWIRD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = outputnode->first_node("SWIRD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = outputnode->first_node("SWIRD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = outputnode->first_node("TIHD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = outputnode->first_node("TIHD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = outputnode->first_node("TIHD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0File.push_back(D0Data->value());

		//获取0级数据暗电流数据
		xml_node<>* D0Dark;
		D0Dark = outputnode->first_node("UVD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());
		D0Dark = outputnode->first_node("UVD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());
		D0Dark = outputnode->first_node("UVD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());
		D0Dark = outputnode->first_node("VNIRD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());
		D0Dark = outputnode->first_node("VNIRD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());
		D0Dark = outputnode->first_node("VNIRD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());
		D0Dark = outputnode->first_node("SWIRD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());
		D0Dark = outputnode->first_node("SWIRD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());
		D0Dark = outputnode->first_node("SWIRD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());
		D0Dark = outputnode->first_node("TIHD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());
		D0Dark = outputnode->first_node("TIHD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());
		D0Dark = outputnode->first_node("TIHD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0DarkFile.push_back(D0Dark->value());

		//获取元数据文件
		xml_node<>* D0XML;
		D0XML = outputnode->first_node("UVD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());
		D0XML = outputnode->first_node("UVD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());
		D0XML = outputnode->first_node("UVD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());
		D0XML = outputnode->first_node("VNIRD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());
		D0XML = outputnode->first_node("VNIRD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());
		D0XML = outputnode->first_node("VNIRD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());
		D0XML = outputnode->first_node("SWIRD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());
		D0XML = outputnode->first_node("SWIRD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());
		D0XML = outputnode->first_node("SWIRD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());
		D0XML = outputnode->first_node("TIHD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());
		D0XML = outputnode->first_node("TIHD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());
		D0XML = outputnode->first_node("TIHD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0XMLFile.push_back(D0XML->value());

		//获取快视图
		xml_node<>* D0JPG;
		D0JPG = outputnode->first_node("UVD0JPG1");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
		D0JPG = outputnode->first_node("UVD0JPG2");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
		D0JPG = outputnode->first_node("UVD0JPG3");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
		D0JPG = outputnode->first_node("VNIRD0JPG1");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
		D0JPG = outputnode->first_node("VNIRD0JPG2");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
		D0JPG = outputnode->first_node("VNIRD0JPG3");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
		D0JPG = outputnode->first_node("SWIRD0JPG1");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
		D0JPG = outputnode->first_node("SWIRD0JPG2");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
		D0JPG = outputnode->first_node("SWIRD0JPG3");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
		D0JPG = outputnode->first_node("TIHD0JPG1");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
		D0JPG = outputnode->first_node("TIHD0JPG2");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
		D0JPG = outputnode->first_node("TIHD0JPG3");
		if (D0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_PreProc_Param.m_pD0JPG.push_back(D0JPG->value());
	}


	//==============================获取参数===============================================
	xml_node<> *paranode = root->first_node("Parament");
	if (paranode == NULL)
	{
		lError = 1;
		return lError;
	}
	//获取分割参数
	xml_node<>* segnode;
	segnode = paranode->first_node("Segment");
	if (segnode == NULL)
	{
		lError = 1;
		return lError;
	}
	xml_node<>* segpara;
	segpara = segnode->first_node("WhetherSeg");
	if (segpara == NULL)
	{
		lError = 1;
		return lError;
	}
	cSeg = segpara->value();
	if (cSeg[0] == '0')
	{
		m_PreProc_Param.m_bSegment = 0;
	}
	else
	{
		m_PreProc_Param.m_bSegment = 1;
	}
	segpara = segnode->first_node("WhertherAuto");
	if (segpara == NULL)
	{
		lError = 1;
		return lError;
	}
	cAuto = segpara->value();

	segpara = segnode->first_node("SegLine");
	if (segpara == NULL)
	{
		lError = 1;
		return lError;
	}
	cLine = segpara->value();
	if (cAuto[0] == '0')
	{
		m_PreProc_Param.m_nAutoSegLine = 0;
		temp = strtok_s(cLine, ",", &buf);
		if (temp)
		{
			m_PreProc_Param.m_nSegBeginLine = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			m_PreProc_Param.m_nSegEndLine = atoi(temp);
		}
		else
		{
			return lError;
		}
	}
	else
	{
		m_PreProc_Param.m_nAutoSegLine = atoi(cLine);
		m_PreProc_Param.m_nSegBeginLine = 0;
		m_PreProc_Param.m_nSegEndLine = 0;
	}
	//获取快视图参数
	xml_node<>* jpgnode;
	jpgnode = paranode->first_node("QuickView");
	if (jpgnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* jpgpara;
		jpgpara = jpgnode->first_node("WhetherQuickView");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cQuickView = jpgpara->value();
		if (cQuickView[0] == '0')
		{
			m_PreProc_Param.m_bQuickView = 0;
		}
		else
		{
			m_PreProc_Param.m_bQuickView = 1;
		}
		jpgpara = jpgnode->first_node("QuickViewScale");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cScale = jpgpara->value();
		m_PreProc_Param.m_fScale = (float)atof(cScale);

		jpgpara = jpgnode->first_node("UVQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_PreProc_Param.m_nUVBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
		jpgpara = jpgnode->first_node("VNIRQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_PreProc_Param.m_nVNIRBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
		jpgpara = jpgnode->first_node("SWIRQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_PreProc_Param.m_nSWIRBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
		jpgpara = jpgnode->first_node("TIHQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_PreProc_Param.m_nTIHBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
	}

	//获取处理进度文件
	xml_node<>* repnode = NULL;
	repnode = paranode->first_node("ReportFile");
	if (repnode == NULL)
	{
		lError = 1;
		return lError;
	}
	m_PreProc_Param.m_pProFile = repnode->value();
	return lError;
}
long QPDXMLParam::SavePreProcXML(const char* pCatalogOrderFile,ParamPre &pPreProc_Param)
{
	long lError = 0;
	char cSegLine[50];
	char cUVBand[50], cVNIRBand[50], cSWIRBand[50], cTIHBand[50];
	char cScale[50];

	string text;
	ofstream out;

	std::locale::global(std::locale(""));
	xml_document<> doc;
	xml_node<>* root = doc.allocate_node(node_pi, doc.allocate_string("xml version='1.0'encoding='utf-8'"));
	doc.append_node(root);

	xml_node<>* node = doc.allocate_node(node_element, "Task", NULL);
	doc.append_node(node);

	xml_node<>* inputnode = doc.allocate_node(node_element, "InputFilelist"); //num = '24', "36"
	inputnode->append_node(doc.allocate_node(node_element, "UVRawData1",pPreProc_Param.m_pRawFile[0].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVRawData2", pPreProc_Param.m_pRawFile[1].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVRawData3", pPreProc_Param.m_pRawFile[2].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRRawData1", pPreProc_Param.m_pRawFile[3].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRRawData2", pPreProc_Param.m_pRawFile[4].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRRawData3", pPreProc_Param.m_pRawFile[5].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRRawData1", pPreProc_Param.m_pRawFile[6].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRRawData2", pPreProc_Param.m_pRawFile[7].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRRawData3", pPreProc_Param.m_pRawFile[8].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHRawData1", pPreProc_Param.m_pRawFile[9].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHRawData2", pPreProc_Param.m_pRawFile[10].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHRawData3", pPreProc_Param.m_pRawFile[11].c_str()));

	inputnode->append_node(doc.allocate_node(node_element, "UVDarkData1", pPreProc_Param.m_pDarkFile[0].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVDarkData2", pPreProc_Param.m_pDarkFile[1].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVDarkData3", pPreProc_Param.m_pDarkFile[2].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRDarkData1", pPreProc_Param.m_pDarkFile[3].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRDarkData2", pPreProc_Param.m_pDarkFile[4].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRDarkData3", pPreProc_Param.m_pDarkFile[5].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRDarkData1", pPreProc_Param.m_pDarkFile[6].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRDarkData2", pPreProc_Param.m_pDarkFile[7].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRDarkData3", pPreProc_Param.m_pDarkFile[8].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHDarkData1", pPreProc_Param.m_pDarkFile[9].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHDarkData2", pPreProc_Param.m_pDarkFile[10].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHDarkData3", pPreProc_Param.m_pDarkFile[11].c_str()));
	node->append_node(inputnode);

	xml_node<>* outputnode = doc.allocate_node(node_element, "OutputFilelist");// num = '24'
	outputnode->append_node(doc.allocate_node(node_element, "UVD0Data1", pPreProc_Param.m_pD0File[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVD0Data2", pPreProc_Param.m_pD0File[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVD0Data3", pPreProc_Param.m_pD0File[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data1", pPreProc_Param.m_pD0File[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data2", pPreProc_Param.m_pD0File[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data3", pPreProc_Param.m_pD0File[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data1", pPreProc_Param.m_pD0File[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data2", pPreProc_Param.m_pD0File[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data3", pPreProc_Param.m_pD0File[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0Data1", pPreProc_Param.m_pD0File[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0Data2", pPreProc_Param.m_pD0File[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0Data3", pPreProc_Param.m_pD0File[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVD0Dark1", pPreProc_Param.m_pD0DarkFile[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVD0Dark2", pPreProc_Param.m_pD0DarkFile[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVD0Dark3", pPreProc_Param.m_pD0DarkFile[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark1", pPreProc_Param.m_pD0DarkFile[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark2", pPreProc_Param.m_pD0DarkFile[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark3", pPreProc_Param.m_pD0DarkFile[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark1", pPreProc_Param.m_pD0DarkFile[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark2", pPreProc_Param.m_pD0DarkFile[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark3", pPreProc_Param.m_pD0DarkFile[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark1", pPreProc_Param.m_pD0DarkFile[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark2", pPreProc_Param.m_pD0DarkFile[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark3", pPreProc_Param.m_pD0DarkFile[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVD0XML1", pPreProc_Param.m_pD0XMLFile[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVD0XML2", pPreProc_Param.m_pD0XMLFile[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVD0XML3", pPreProc_Param.m_pD0XMLFile[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML1", pPreProc_Param.m_pD0XMLFile[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML2", pPreProc_Param.m_pD0XMLFile[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML3", pPreProc_Param.m_pD0XMLFile[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML1", pPreProc_Param.m_pD0XMLFile[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML2", pPreProc_Param.m_pD0XMLFile[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML3", pPreProc_Param.m_pD0XMLFile[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0XML1", pPreProc_Param.m_pD0XMLFile[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0XML2", pPreProc_Param.m_pD0XMLFile[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0XML3", pPreProc_Param.m_pD0XMLFile[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVD0JPG1", pPreProc_Param.m_pD0JPG[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVD0JPG2", pPreProc_Param.m_pD0JPG[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVD0JPG3", pPreProc_Param.m_pD0JPG[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0JPG1", pPreProc_Param.m_pD0JPG[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0JPG2", pPreProc_Param.m_pD0JPG[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRD0JPG3", pPreProc_Param.m_pD0JPG[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0JPG1", pPreProc_Param.m_pD0JPG[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0JPG2", pPreProc_Param.m_pD0JPG[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRD0JPG3", pPreProc_Param.m_pD0JPG[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0JPG1", pPreProc_Param.m_pD0JPG[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0JPG2", pPreProc_Param.m_pD0JPG[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHD0JPG3", pPreProc_Param.m_pD0JPG[11].c_str()));
	node->append_node(outputnode);

	xml_node<>* paranode = doc.allocate_node(node_element, "Parament");
	node->append_node(paranode);

	xml_node<>* segnode = doc.allocate_node(node_element, "Segment");
	if (m_PreProc_Param.m_bSegment == 0)
	{
		segnode->append_node(doc.allocate_node(node_element, "WhetherSeg", "0"));
		segnode->append_node(doc.allocate_node(node_element, "WhertherAuto", "0"));
		segnode->append_node(doc.allocate_node(node_element, "SegLine", "0"));
	}
	else
	{
		segnode->append_node(doc.allocate_node(node_element, "WhetherSeg", "1"));
		if (m_PreProc_Param.m_nAutoSegLine == 0)
		{
			segnode->append_node(doc.allocate_node(node_element, "WhertherAuto", "0"));
			sprintf_s(cSegLine, sizeof(cSegLine), "%d,%d",m_PreProc_Param.m_nSegBeginLine, m_PreProc_Param.m_nSegEndLine);
			segnode->append_node(doc.allocate_node(node_element, "SegLine", cSegLine));
		}
		else
		{
			segnode->append_node(doc.allocate_node(node_element, "WhertherAuto", "1"));
			sprintf_s(cSegLine, sizeof(cSegLine), "%d", m_PreProc_Param.m_nAutoSegLine);
			segnode->append_node(doc.allocate_node(node_element, "SegLine", cSegLine));
		}
	}
	paranode->append_node(segnode);

	xml_node<>* jpgnode = doc.allocate_node(node_element, "QuickView");
	if (m_PreProc_Param.m_bQuickView == 0)
	{
		jpgnode->append_node(doc.allocate_node(node_element, "WhetherQuickView", "0"));
		jpgnode->append_node(doc.allocate_node(node_element, "QuickViewScale", "0"));
		jpgnode->append_node(doc.allocate_node(node_element, "UVQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "VNIRQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "SWIRQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "TIHQuickBand", "-1"));
	}
	else
	{
		jpgnode->append_node(doc.allocate_node(node_element, "WhetherQuickView", "1"));
		sprintf_s(cScale, sizeof(cScale), "%f", m_PreProc_Param.m_fScale);
		jpgnode->append_node(doc.allocate_node(node_element, "QuickViewScale", cScale));
		sprintf_s(cUVBand, sizeof(cUVBand), "%d,%d,%d", m_PreProc_Param.m_nUVBand[0], m_PreProc_Param.m_nUVBand[1], m_PreProc_Param.m_nUVBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "UVQuickBand", cUVBand));
		sprintf_s(cVNIRBand, sizeof(cVNIRBand), "%d,%d,%d", m_PreProc_Param.m_nVNIRBand[0], m_PreProc_Param.m_nVNIRBand[1], m_PreProc_Param.m_nVNIRBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "VNIRQuickBand", cVNIRBand));
		sprintf_s(cSWIRBand, sizeof(cSWIRBand), "%d,%d,%d", m_PreProc_Param.m_nSWIRBand[0], m_PreProc_Param.m_nSWIRBand[1], m_PreProc_Param.m_nSWIRBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "SWIRQuickBand", cSWIRBand));
		sprintf_s(cTIHBand, sizeof(cTIHBand), "%d,%d,%d", m_PreProc_Param.m_nTIHBand[0], m_PreProc_Param.m_nTIHBand[1], m_PreProc_Param.m_nTIHBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "TIHQuickBand", cTIHBand));
	}
	paranode->append_node(jpgnode);

	xml_node<>* repnode = doc.allocate_node(node_element, "ReportFile", m_PreProc_Param.m_pProFile.c_str());
	paranode->append_node(repnode);


	string str;
	rapidxml::print(std::back_inserter(str), doc, 0);
	ofstream out1(pCatalogOrderFile);
	out1 << doc;
	out1.close();

	return lError;
}

long QPDXMLParam::LoadP0ProcXML(const char* pCatalogOrderFile)
{
	long lError = 0;
	char *cSeg = "";
	char *cAuto = "";
	char *cLine = "";
	char *cQuickView = "";
	char *cBand = "";
	char *buf = "";
	char *temp = "";
	char *cScale = "";
	int i = 0;

	std::locale::global(std::locale(""));
	file<> fdoc(pCatalogOrderFile);

	xml_document<> doc;
	doc.parse<0>(fdoc.data());

	xml_node<>* root = doc.first_node();
	if (root == NULL)
	{
		lError = 1;
		return lError;
	}

	//==============================获取输入数据===============================================
	xml_node<> *inputnode = root->first_node("InputFilelist");
	if (inputnode == NULL)
	{
		lError = 1;
		return lError;
	}
	//获取原始图像数据
	xml_node<>* D0Data;
	D0Data = inputnode->first_node("UVD0Data1");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());
	D0Data = inputnode->first_node("UVD0Data2");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());
	D0Data = inputnode->first_node("UVD0Data3");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());
	D0Data = inputnode->first_node("VNIRD0Data1");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());
	D0Data = inputnode->first_node("VNIRD0Data2");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());
	D0Data = inputnode->first_node("VNIRD0Data3");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());
	D0Data = inputnode->first_node("SWIRD0Data1");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());
	D0Data = inputnode->first_node("SWIRD0Data2");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());
	D0Data = inputnode->first_node("SWIRD0Data3");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());
	D0Data = inputnode->first_node("TIHD0Data1");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());
	D0Data = inputnode->first_node("TIHD0Data2");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());
	D0Data = inputnode->first_node("TIHD0Data3");
	if (D0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pD0File.push_back(D0Data->value());

	//获取暗电流数据
	xml_node<>* D0Dark;
	D0Dark = inputnode->first_node("UVD0Dark1");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());
	D0Dark = inputnode->first_node("UVD0Dark2");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());
	D0Dark = inputnode->first_node("UVD0Dark3");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());
	D0Dark = inputnode->first_node("VNIRD0Dark1");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());
	D0Dark = inputnode->first_node("VNIRD0Dark2");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());
	D0Dark = inputnode->first_node("VNIRD0Dark3");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());
	D0Dark = inputnode->first_node("SWIRD0Dark1");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());
	D0Dark = inputnode->first_node("SWIRD0Dark2");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());
	D0Dark = inputnode->first_node("SWIRD0Dark3");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());
	D0Dark = inputnode->first_node("TIHD0Dark1");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());
	D0Dark = inputnode->first_node("TIHD0Dark2");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());
	D0Dark = inputnode->first_node("TIHD0Dark3");
	if (D0Dark == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0DarkFile.push_back(D0Dark->value());

	//获取0级产品元数据文件
	xml_node<>* D0XML;
	D0XML = inputnode->first_node("UVD0XML1");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());
	D0XML = inputnode->first_node("UVD0XML2");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());
	D0XML = inputnode->first_node("UVD0XML3");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());
	D0XML = inputnode->first_node("VNIRD0XML1");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());
	D0XML = inputnode->first_node("VNIRD0XML2");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());
	D0XML = inputnode->first_node("VNIRD0XML3");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());
	D0XML = inputnode->first_node("SWIRD0XML1");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());
	D0XML = inputnode->first_node("SWIRD0XML2");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());
	D0XML = inputnode->first_node("SWIRD0XML3");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());
	D0XML = inputnode->first_node("TIHD0XML1");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());
	D0XML = inputnode->first_node("TIHD0XML2");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());
	D0XML = inputnode->first_node("TIHD0XML3");
	if (D0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.pD0MetaXML.push_back(D0XML->value());

	//==============================获取输出数据===============================================
	xml_node<> *outputnode = root->first_node("OutputFilelist");
	if (outputnode == NULL)
	{
		lError = 1;
		return lError;
	}
	//获取0级产品
	xml_node<>* P0Data;
	P0Data = outputnode->first_node("UVP0Data1");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());
	P0Data = outputnode->first_node("UVP0Data2");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());
	P0Data = outputnode->first_node("UVP0Data3");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());
	P0Data = outputnode->first_node("VNIRP0Data1");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());
	P0Data = outputnode->first_node("VNIRP0Data2");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());
	P0Data = outputnode->first_node("VNIRP0Data3");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());
	P0Data = outputnode->first_node("SWIRP0Data1");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());
	P0Data = outputnode->first_node("SWIRP0Data2");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());
	P0Data = outputnode->first_node("SWIRP0Data3");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());
	P0Data = outputnode->first_node("TIHP0Data1");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());
	P0Data = outputnode->first_node("TIHP0Data2");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());
	P0Data = outputnode->first_node("TIHP0Data3");
	if (P0Data == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0Product.push_back(P0Data->value());


	//获取0级产品元数据文件
	xml_node<>* P0XML;
	P0XML = outputnode->first_node("UVP0XML1");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
	P0XML = outputnode->first_node("UVP0XML2");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
	P0XML = outputnode->first_node("UVP0XML3");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
	P0XML = outputnode->first_node("VNIRP0XML1");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
	P0XML = outputnode->first_node("VNIRP0XML2");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
	P0XML = outputnode->first_node("VNIRP0XML3");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
	P0XML = outputnode->first_node("SWIRP0XML1");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
	P0XML = outputnode->first_node("SWIRP0XML2");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
	P0XML = outputnode->first_node("SWIRP0XML3");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
	P0XML = outputnode->first_node("TIHP0XML1");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
	P0XML = outputnode->first_node("TIHP0XML2");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
	P0XML = outputnode->first_node("TIHP0XML3");
	if (P0XML == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0ProductXML.push_back(P0XML->value());

	//获取快视图
	xml_node<>* P0JPG;
	P0JPG = outputnode->first_node("UVP0JPG1");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());
	P0JPG = outputnode->first_node("UVP0JPG2");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());
	P0JPG = outputnode->first_node("UVP0JPG3");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());
	P0JPG = outputnode->first_node("VNIRP0JPG1");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());
	P0JPG = outputnode->first_node("VNIRP0JPG2");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());
	P0JPG = outputnode->first_node("VNIRP0JPG3");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());
	P0JPG = outputnode->first_node("SWIRP0JPG1");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());
	P0JPG = outputnode->first_node("SWIRP0JPG2");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());
	P0JPG = outputnode->first_node("SWIRP0JPG3");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());
	P0JPG = outputnode->first_node("TIHP0JPG1");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());
	P0JPG = outputnode->first_node("TIHP0JPG2");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());
	P0JPG = outputnode->first_node("TIHP0JPG3");
	if (P0JPG == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pP0JPG.push_back(P0JPG->value());

	//获取EVENT
	xml_node<>* Event;
	Event = outputnode->first_node("UVEvent1");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	Event = outputnode->first_node("UVEvent2");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	Event = outputnode->first_node("UVEvent3");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	Event = outputnode->first_node("VNIREvent1");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	Event = outputnode->first_node("VNIREvent2");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	Event = outputnode->first_node("VNIREvent3");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	Event = outputnode->first_node("SWIREvent1");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	Event = outputnode->first_node("SWIREvent2");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	Event = outputnode->first_node("SWIREvent3");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	Event = outputnode->first_node("TIHEvent1");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	Event = outputnode->first_node("TIHEvent2");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	Event = outputnode->first_node("TIHEvent3");
	if (Event == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pEvent.push_back(Event->value());
	//==============================获取参数===============================================
	xml_node<> *paranode = root->first_node("Parament");
	if (paranode == NULL)
	{
		lError = 1;
		return lError;
	}
	//获取快视图参数
	xml_node<>* jpgnode;
	jpgnode = paranode->first_node("QuickView");
	if (jpgnode == NULL)
	{
		lError = 1;
		return lError;
	}
	xml_node<>* jpgpara;
	jpgpara = jpgnode->first_node("WhetherQuickView");
	if (jpgpara == NULL)
	{
		lError = 1;
		return lError;
	}
	cQuickView = jpgpara->value();
	if (cQuickView[0] == '0')
	{
		m_P0Proc_Param.m_bQuickView = 0;
	}
	else
	{
		m_P0Proc_Param.m_bQuickView = 1;
	}
	jpgpara = jpgnode->first_node("QuickViewScale");
	if (jpgpara == NULL)
	{
		lError = 1;
		return lError;
	}
	cScale = jpgpara->value();
	m_P0Proc_Param.m_fScale = (float)atof(cScale);

	jpgpara = jpgnode->first_node("UVQuickBand");
	if (jpgpara == NULL)
	{
		lError = 1;
		return lError;
	}
	cBand = jpgpara->value();
	i = 0;
	temp = strtok_s(cBand, ",", &buf);
	while (temp && i<3)
	{
		m_P0Proc_Param.m_nUVBand[i] = atoi(temp);
		temp = strtok_s(NULL, ",", &buf);
		i++;
	}
	jpgpara = jpgnode->first_node("VNIRQuickBand");
	if (jpgpara == NULL)
	{
		lError = 1;
		return lError;
	}
	cBand = jpgpara->value();
	i = 0;
	temp = strtok_s(cBand, ",", &buf);
	while (temp && i<3)
	{
		m_P0Proc_Param.m_nVNIRBand[i] = atoi(temp);
		temp = strtok_s(NULL, ",", &buf);
		i++;
	}
	jpgpara = jpgnode->first_node("SWIRQuickBand");
	if (jpgpara == NULL)
	{
		lError = 1;
		return lError;
	}
	cBand = jpgpara->value();
	i = 0;
	temp = strtok_s(cBand, ",", &buf);
	while (temp && i<3)
	{
		m_P0Proc_Param.m_nSWIRBand[i] = atoi(temp);
		temp = strtok_s(NULL, ",", &buf);
		i++;
	}
	jpgpara = jpgnode->first_node("TIHQuickBand");
	if (jpgpara == NULL)
	{
		lError = 1;
		return lError;
	}
	cBand = jpgpara->value();
	i = 0;
	temp = strtok_s(cBand, ",", &buf);
	while (temp && i<3)
	{
		m_P0Proc_Param.m_nTIHBand[i] = atoi(temp);
		temp = strtok_s(NULL, ",", &buf);
		i++;
	}
	//获取处理进度文件
	xml_node<>* repnode = NULL;
	repnode = paranode->first_node("ReportFile");
	if (repnode == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P0Proc_Param.m_pProFile = repnode->value();
	return lError;
}
long QPDXMLParam::SaveP0ProcXML(const char* pCatalogOrderFile,ParamP0 &pP0Proc_Param)
{
	long lError = 0;
	char cUVBand[50], cVNIRBand[50], cSWIRBand[50], cTIHBand[50];
	char cScale[50];

	string text;
	ofstream out;

	std::locale::global(std::locale(""));
	xml_document<> doc;
	xml_node<>* root = doc.allocate_node(node_pi, doc.allocate_string("xml version='1.0'encoding='utf-8'"));
	doc.append_node(root);

	xml_node<>* node = doc.allocate_node(node_element, "P0Task", NULL);
	doc.append_node(node);

	xml_node<>* inputnode = doc.allocate_node(node_element, "InputFilelist"); //num = '24', "36"
	inputnode->append_node(doc.allocate_node(node_element, "UVD0Data1", pP0Proc_Param.m_pD0File[0].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0Data2", pP0Proc_Param.m_pD0File[1].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0Data3", pP0Proc_Param.m_pD0File[2].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data1", pP0Proc_Param.m_pD0File[3].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data2", pP0Proc_Param.m_pD0File[4].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data3", pP0Proc_Param.m_pD0File[5].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data1", pP0Proc_Param.m_pD0File[6].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data2", pP0Proc_Param.m_pD0File[7].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data3", pP0Proc_Param.m_pD0File[8].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Data1", pP0Proc_Param.m_pD0File[9].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Data2", pP0Proc_Param.m_pD0File[10].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Data3", pP0Proc_Param.m_pD0File[11].c_str()));

	inputnode->append_node(doc.allocate_node(node_element, "UVD0Dark1", pP0Proc_Param.pD0DarkFile[0].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0Dark2", pP0Proc_Param.pD0DarkFile[1].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0Dark3", pP0Proc_Param.pD0DarkFile[2].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark1", pP0Proc_Param.pD0DarkFile[3].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark2", pP0Proc_Param.pD0DarkFile[4].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark3", pP0Proc_Param.pD0DarkFile[5].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark1", pP0Proc_Param.pD0DarkFile[6].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark2", pP0Proc_Param.pD0DarkFile[7].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark3", pP0Proc_Param.pD0DarkFile[8].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark1", pP0Proc_Param.pD0DarkFile[9].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark2", pP0Proc_Param.pD0DarkFile[10].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark3", pP0Proc_Param.pD0DarkFile[11].c_str()));

	inputnode->append_node(doc.allocate_node(node_element, "UVD0XML1", pP0Proc_Param.pD0MetaXML[0].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0XML2", pP0Proc_Param.pD0MetaXML[1].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0XML3", pP0Proc_Param.pD0MetaXML[2].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML1", pP0Proc_Param.pD0MetaXML[3].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML2", pP0Proc_Param.pD0MetaXML[4].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML3", pP0Proc_Param.pD0MetaXML[5].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML1", pP0Proc_Param.pD0MetaXML[6].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML2", pP0Proc_Param.pD0MetaXML[7].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML3", pP0Proc_Param.pD0MetaXML[8].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0XML1", pP0Proc_Param.pD0MetaXML[9].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0XML2", pP0Proc_Param.pD0MetaXML[10].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0XML3", pP0Proc_Param.pD0MetaXML[11].c_str()));

	node->append_node(inputnode);

	xml_node<>* outputnode = doc.allocate_node(node_element, "OutputFilelist");// num = '24'
	outputnode->append_node(doc.allocate_node(node_element, "UVP0Data1", pP0Proc_Param.m_pP0Product[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0Data2", pP0Proc_Param.m_pP0Product[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0Data3", pP0Proc_Param.m_pP0Product[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0Data1", pP0Proc_Param.m_pP0Product[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0Data2", pP0Proc_Param.m_pP0Product[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0Data3", pP0Proc_Param.m_pP0Product[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0Data1", pP0Proc_Param.m_pP0Product[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0Data2", pP0Proc_Param.m_pP0Product[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0Data3", pP0Proc_Param.m_pP0Product[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0Data1", pP0Proc_Param.m_pP0Product[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0Data2", pP0Proc_Param.m_pP0Product[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0Data3", pP0Proc_Param.m_pP0Product[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP0XML1", pP0Proc_Param.m_pP0ProductXML[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0XML2", pP0Proc_Param.m_pP0ProductXML[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0XML3", pP0Proc_Param.m_pP0ProductXML[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0XML1", pP0Proc_Param.m_pP0ProductXML[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0XML2", pP0Proc_Param.m_pP0ProductXML[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0XML3", pP0Proc_Param.m_pP0ProductXML[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0XML1", pP0Proc_Param.m_pP0ProductXML[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0XML2", pP0Proc_Param.m_pP0ProductXML[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0XML3", pP0Proc_Param.m_pP0ProductXML[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0XML1", pP0Proc_Param.m_pP0ProductXML[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0XML2", pP0Proc_Param.m_pP0ProductXML[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0XML3", pP0Proc_Param.m_pP0ProductXML[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP0JPG1", pP0Proc_Param.m_pP0JPG[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0JPG2", pP0Proc_Param.m_pP0JPG[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0JPG3", pP0Proc_Param.m_pP0JPG[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0JPG1", pP0Proc_Param.m_pP0JPG[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0JPG2", pP0Proc_Param.m_pP0JPG[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0JPG3", pP0Proc_Param.m_pP0JPG[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0JPG1", pP0Proc_Param.m_pP0JPG[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0JPG2", pP0Proc_Param.m_pP0JPG[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0JPG3", pP0Proc_Param.m_pP0JPG[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0JPG1", pP0Proc_Param.m_pP0JPG[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0JPG2", pP0Proc_Param.m_pP0JPG[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0JPG3", pP0Proc_Param.m_pP0JPG[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVEvent1", pP0Proc_Param.m_pEvent[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVEvent2", pP0Proc_Param.m_pEvent[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVEvent3", pP0Proc_Param.m_pEvent[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIREvent1", pP0Proc_Param.m_pEvent[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIREvent2", pP0Proc_Param.m_pEvent[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIREvent3", pP0Proc_Param.m_pEvent[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIREvent1", pP0Proc_Param.m_pEvent[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIREvent2", pP0Proc_Param.m_pEvent[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIREvent3", pP0Proc_Param.m_pEvent[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHEvent1", pP0Proc_Param.m_pEvent[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHEvent2", pP0Proc_Param.m_pEvent[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHEvent3", pP0Proc_Param.m_pEvent[11].c_str()));

	node->append_node(outputnode);

	xml_node<>* paranode = doc.allocate_node(node_element, "Parament");
	node->append_node(paranode);


	xml_node<>* jpgnode = doc.allocate_node(node_element, "QuickView");
	if (pP0Proc_Param.m_bQuickView == 0)
	{
		jpgnode->append_node(doc.allocate_node(node_element, "WhetherQuickView", "0"));
		jpgnode->append_node(doc.allocate_node(node_element, "QuickViewScale", "0"));
		jpgnode->append_node(doc.allocate_node(node_element, "UVQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "VNIRQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "SWIRQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "TIHQuickBand", "-1"));
	}
	else
	{
		jpgnode->append_node(doc.allocate_node(node_element, "WhetherQuickView", "1"));
		sprintf_s(cScale, sizeof(cScale), "%f", pP0Proc_Param.m_fScale);
		jpgnode->append_node(doc.allocate_node(node_element, "QuickViewScale", cScale));
		sprintf_s(cUVBand, sizeof(cUVBand), "%d,%d,%d", pP0Proc_Param.m_nUVBand[0], pP0Proc_Param.m_nUVBand[1], pP0Proc_Param.m_nUVBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "UVQuickBand", cUVBand));
		sprintf_s(cVNIRBand, sizeof(cVNIRBand), "%d,%d,%d", pP0Proc_Param.m_nVNIRBand[0], pP0Proc_Param.m_nVNIRBand[1], pP0Proc_Param.m_nVNIRBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "VNIRQuickBand", cVNIRBand));
		sprintf_s(cSWIRBand, sizeof(cSWIRBand), "%d,%d,%d", pP0Proc_Param.m_nSWIRBand[0], pP0Proc_Param.m_nSWIRBand[1], pP0Proc_Param.m_nSWIRBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "SWIRQuickBand", cSWIRBand));
		sprintf_s(cTIHBand, sizeof(cTIHBand), "%d,%d,%d", pP0Proc_Param.m_nTIHBand[0], pP0Proc_Param.m_nTIHBand[1], pP0Proc_Param.m_nTIHBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "TIHQuickBand", cTIHBand));
	}
	paranode->append_node(jpgnode);

	xml_node<>* repnode = doc.allocate_node(node_element, "ReportFile", pP0Proc_Param.m_pProFile.c_str());
	paranode->append_node(repnode);


	string str;
	rapidxml::print(std::back_inserter(str), doc, 0);
	ofstream out1(pCatalogOrderFile);
	out1 << doc;
	out1.close();

	return lError;
}

long QPDXMLParam::LoadP1ProcXML(const char* pCatalogOrderFile)
{
	long lError = 0;
	char *cQuickView = "";
	char *cBand = "";
	char *buf = "";
	char *temp = "";
	char *cScale = "";
	int i = 0, j = 0;

	std::locale::global(std::locale(""));
	file<> fdoc(pCatalogOrderFile);

	xml_document<> doc;
	doc.parse<0>(fdoc.data());

	xml_node<>* root = doc.first_node();
	if (root == NULL)
	{
		lError = 1;
		return lError;
	}

	//==============================获取输入数据===============================================
	xml_node<> *inputnode = root->first_node("InputFilelist");
	if (inputnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		//获取原始图像数据
		xml_node<>* D0Data;
		D0Data = inputnode->first_node("UVD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("UVD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("UVD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("VNIRD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("VNIRD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("VNIRD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("SWIRD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("SWIRD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("SWIRD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("TIHD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("TIHD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("TIHD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pD0File.push_back(D0Data->value());

		//获取暗电流数据
		xml_node<>* D0Dark;
		D0Dark = inputnode->first_node("UVD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("UVD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("UVD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("VNIRD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("VNIRD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("VNIRD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("SWIRD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("SWIRD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("SWIRD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("TIHD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("TIHD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("TIHD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0DarkFile.push_back(D0Dark->value());

		//获取0级产品元数据文件
		xml_node<>* D0XML;
		D0XML = inputnode->first_node("UVD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("UVD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("UVD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("VNIRD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("VNIRD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("VNIRD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("SWIRD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("SWIRD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("SWIRD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("TIHD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("TIHD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("TIHD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.pD0MetaXML.push_back(D0XML->value());
	}


	//==============================获取输出数据===============================================
	xml_node<> *outputnode = root->first_node("OutputFilelist");
	if (outputnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		//获取0级产品=================================
		xml_node<>* P0Data;
		P0Data = outputnode->first_node("UVP0Data1");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("UVP0Data2");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("UVP0Data3");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("VNIRP0Data1");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("VNIRP0Data2");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("VNIRP0Data3");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("SWIRP0Data1");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("SWIRP0Data2");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("SWIRP0Data3");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("TIHP0Data1");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("TIHP0Data2");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("TIHP0Data3");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0Product.push_back(P0Data->value());

		//获取0级产品元数据文件
		xml_node<>* P0XML;
		P0XML = outputnode->first_node("UVP0XML1");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("UVP0XML2");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("UVP0XML3");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("VNIRP0XML1");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("VNIRP0XML2");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("VNIRP0XML3");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("SWIRP0XML1");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("SWIRP0XML2");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("SWIRP0XML3");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("TIHP0XML1");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("TIHP0XML2");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("TIHP0XML3");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0ProductXML.push_back(P0XML->value());

		//获取0级快视图
		xml_node<>* P0JPG;
		P0JPG = outputnode->first_node("UVP0JPG1");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("UVP0JPG2");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("UVP0JPG3");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("VNIRP0JPG1");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("VNIRP0JPG2");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("VNIRP0JPG3");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("SWIRP0JPG1");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("SWIRP0JPG2");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("SWIRP0JPG3");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("TIHP0JPG1");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("TIHP0JPG2");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("TIHP0JPG3");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP0JPG.push_back(P0JPG->value());

		//获取EVENT
		xml_node<>* Event;
		Event = outputnode->first_node("UVEvent1");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());
		Event = outputnode->first_node("UVEvent2");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());
		Event = outputnode->first_node("UVEvent3");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());
		Event = outputnode->first_node("VNIREvent1");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());
		Event = outputnode->first_node("VNIREvent2");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());
		Event = outputnode->first_node("VNIREvent3");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());
		Event = outputnode->first_node("SWIREvent1");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());
		Event = outputnode->first_node("SWIREvent2");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());
		Event = outputnode->first_node("SWIREvent3");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());
		Event = outputnode->first_node("TIHEvent1");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());
		Event = outputnode->first_node("TIHEvent2");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());
		Event = outputnode->first_node("TIHEvent3");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pEvent.push_back(Event->value());

		//获取P1A级产品====================================
		xml_node<>* P1AData;
		P1AData = outputnode->first_node("UVP1AData1");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("UVP1AData2");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("UVP1AData3");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("VNIRP1AData1");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("VNIRP1AData2");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("VNIRP1AData3");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("SWIRP1AData1");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("SWIRP1AData2");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("SWIRP1AData3");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("TIHP1AData1");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("TIHP1AData2");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("TIHP1AData3");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AData.push_back(P1AData->value());


		//获取0级产品元数据文件
		xml_node<>* P1AXML;
		P1AXML = outputnode->first_node("UVP1AXML1");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("UVP1AXML2");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("UVP1AXML3");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("VNIRP1AXML1");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("VNIRP1AXML2");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("VNIRP1AXML3");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("SWIRP1AXML1");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("SWIRP1AXML2");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("SWIRP1AXML3");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("TIHP1AXML1");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("TIHP1AXML2");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("TIHP1AXML3");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AXML.push_back(P1AXML->value());

		//获取P1A快视图
		xml_node<>* P1AJPG;
		P1AJPG = outputnode->first_node("UVP1AJPG1");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("UVP1AJPG2");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("UVP1AJPG3");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("VNIRP1AJPG1");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("VNIRP1AJPG2");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("VNIRP1AJPG3");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("SWIRP1AJPG1");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("SWIRP1AJPG2");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("SWIRP1AJPG3");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("TIHP1AJPG1");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("TIHP1AJPG2");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("TIHP1AJPG3");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());

		//获取P1B级产品
		xml_node<>* P1BData;
		P1BData = outputnode->first_node("UVP1BData1");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("UVP1BData2");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("UVP1BData3");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("VNIRP1BData1");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("VNIRP1BData2");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("VNIRP1BData3");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("SWIRP1BData1");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("SWIRP1BData2");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("SWIRP1BData3");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("TIHP1BData1");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("TIHP1BData2");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("TIHP1BData3");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BData.push_back(P1BData->value());


		//获取P1B级产品元数据文件
		xml_node<>* P1BXML;
		P1BXML = outputnode->first_node("UVP1BXML1");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("UVP1BXML2");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("UVP1BXML3");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("VNIRP1BXML1");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("VNIRP1BXML2");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("VNIRP1BXML3");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("SWIRP1BXML1");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("SWIRP1BXML2");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("SWIRP1BXML3");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("TIHP1BXML1");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("TIHP1BXML2");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("TIHP1BXML3");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BXML.push_back(P1BXML->value());

		//获取P1B快视图
		xml_node<>* P1BJPG;
		P1BJPG = outputnode->first_node("UVP1BJPG1");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("UVP1BJPG2");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("UVP1BJPG3");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("VNIRP1BJPG1");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("VNIRP1BJPG2");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("VNIRP1BJPG3");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("SWIRP1BJPG1");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("SWIRP1BJPG2");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("SWIRP1BJPG3");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("TIHP1BJPG1");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("TIHP1BJPG2");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("TIHP1BJPG3");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());

		//获取P1C级产品
		xml_node<>* P1CData;
		P1CData = outputnode->first_node("UVP1CData1");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("UVP1CData2");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("UVP1CData3");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("VNIRP1CData1");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("VNIRP1CData2");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("VNIRP1CData3");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("SWIRP1CData1");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("SWIRP1CData2");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("SWIRP1CData3");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("TIHP1CData1");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("TIHP1CData2");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("TIHP1CData3");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CData.push_back(P1CData->value());


		//获取P1C级产品元数据文件
		xml_node<>* P1CXML;
		P1CXML = outputnode->first_node("UVP1CXML1");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("UVP1CXML2");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("UVP1CXML3");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("VNIRP1CXML1");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("VNIRP1CXML2");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("VNIRP1CXML3");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("SWIRP1CXML1");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("SWIRP1CXML2");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("SWIRP1CXML3");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("TIHP1CXML1");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("TIHP1CXML2");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("TIHP1CXML3");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CXML.push_back(P1CXML->value());

		//获取P1C快视图
		xml_node<>* P1CJPG;
		P1CJPG = outputnode->first_node("UVP1CJPG1");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("UVP1CJPG2");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("UVP1CJPG3");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("VNIRP1CJPG1");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("VNIRP1CJPG2");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("VNIRP1CJPG3");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("SWIRP1CJPG1");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("SWIRP1CJPG2");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("SWIRP1CJPG3");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("TIHP1CJPG1");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("TIHP1CJPG2");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("TIHP1CJPG3");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());

		//获取P1D级产品
		xml_node<>* P1DData;
		P1DData = outputnode->first_node("UVP1DData");
		if (P1DData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DData.push_back(P1DData->value());
		P1DData = outputnode->first_node("VNIRP1DData");
		if (P1DData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DData.push_back(P1DData->value());
		P1DData = outputnode->first_node("SWIRP1DData");
		if (P1DData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DData.push_back(P1DData->value());
		P1DData = outputnode->first_node("TIHP1DData");
		if (P1DData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DData.push_back(P1DData->value());

		//获取P1D级产品元数据文件
		xml_node<>* P1DXML;
		P1DXML = outputnode->first_node("UVP1DXML");
		if (P1DXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DXML.push_back(P1DXML->value());
		P1DXML = outputnode->first_node("VNIRP1DXML");
		if (P1DXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DXML.push_back(P1DXML->value());
		P1DXML = outputnode->first_node("SWIRP1DXML");
		if (P1DXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DXML.push_back(P1DXML->value());
		P1DXML = outputnode->first_node("TIHP1DXML");
		if (P1DXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DXML.push_back(P1DXML->value());

		//获取P1D快视图
		xml_node<>* P1DJPG;
		P1DJPG = outputnode->first_node("UVP1DJPG");
		if (P1DJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DJPG.push_back(P1DJPG->value());
		P1DJPG = outputnode->first_node("VNIRP1DJPG");
		if (P1DJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DJPG.push_back(P1DJPG->value());
		P1DJPG = outputnode->first_node("SWIRP1DJPG");
		if (P1DJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DJPG.push_back(P1DJPG->value());
		P1DJPG = outputnode->first_node("TIHP1DJPG");
		if (P1DJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1DJPG.push_back(P1DJPG->value());

		//获取P1E级产品
		xml_node<>* P1EData;
		P1EData = outputnode->first_node("UVP1EData");
		if (P1EData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EData.push_back(P1EData->value());
		P1EData = outputnode->first_node("VNIRP1EData");
		if (P1EData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EData.push_back(P1EData->value());
		P1EData = outputnode->first_node("SWIRP1EData");
		if (P1EData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EData.push_back(P1EData->value());
		P1EData = outputnode->first_node("TIHP1EData");
		if (P1EData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EData.push_back(P1EData->value());

		//获取P1E级产品元数据文件
		xml_node<>* P1EXML;
		P1EXML = outputnode->first_node("UVP1EXML");
		if (P1EXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EXML.push_back(P1EXML->value());
		P1EXML = outputnode->first_node("VNIRP1EXML");
		if (P1EXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EXML.push_back(P1EXML->value());
		P1EXML = outputnode->first_node("SWIRP1EXML");
		if (P1EXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EXML.push_back(P1EXML->value());
		P1EXML = outputnode->first_node("TIHP1EXML");
		if (P1EXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EXML.push_back(P1EXML->value());

		//获取P1E快视图
		xml_node<>* P1EJPG;
		P1EJPG = outputnode->first_node("UVP1EJPG");
		if (P1EJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EJPG.push_back(P1EJPG->value());
		P1EJPG = outputnode->first_node("VNIRP1EJPG");
		if (P1EJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EJPG.push_back(P1EJPG->value());
		P1EJPG = outputnode->first_node("SWIRP1EJPG");
		if (P1EJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EJPG.push_back(P1EJPG->value());
		P1EJPG = outputnode->first_node("TIHP1EJPG");
		if (P1EJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pP1EJPG.push_back(P1EJPG->value());

	}

	//==============================获取参数===============================================
	xml_node<> *paranode = root->first_node("Parament");
	if (paranode == NULL)
	{
		lError = 1;
		return lError;
	}
	//获取相对辐射校正系数
	xml_node<> *recofnode = paranode->first_node("RadiateReleCof");
	if (recofnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* ReleCof;
		ReleCof = recofnode->first_node("UVReleCof1");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("UVReleCof2");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("UVReleCof3");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("VNIRReleCof1");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("VNIRReleCof2");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("VNIRReleCof3");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("SWIRReleCof1");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("SWIRReleCof2");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("SWIRReleCof3");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("TIHReleCof1");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("TIHReleCof2");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("TIHReleCof3");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pReleCof.push_back(ReleCof->value());
	}
	//获取绝对辐射校正系数
	xml_node<> *abcofnode = paranode->first_node("RadiateAbCof");
	if (abcofnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* AbCof;
		AbCof = abcofnode->first_node("UVAbCof1");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("UVAbCof2");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("UVAbCof3");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("VNIRAbCof1");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("VNIRAbCof2");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("VNIRAbCof3");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("SWIRAbCof1");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("SWIRAbCof2");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("SWIRAbCof3");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("TIHAbCof1");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("TIHAbCof2");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("TIHAbCof3");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pAbCof.push_back(AbCof->value());
	}
	//获取中心波长半波宽信息
	xml_node<> *splnode = paranode->first_node("WaveLength");
	if (splnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* WaveLen;
		WaveLen = splnode->first_node("Modtran");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pModtran = WaveLen->value();
		WaveLen = splnode->first_node("UVWaveLen1");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("UVWaveLen2");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("UVWaveLen3");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("VNIRWaveLen1");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("VNIRWaveLen2");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("VNIRWaveLen3");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("SWIRWaveLen1");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("SWIRWaveLen2");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("SWIRWaveLen3");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("TIHWaveLen1");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("TIHWaveLen2");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("TIHWaveLen3");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P1Proc_Param.m_pWaveLen.push_back(WaveLen->value());
	}

	//获取视场拼接参数
	xml_node<>* folapnode;
	folapnode = paranode->first_node("FieldOverLap");
	if (folapnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* FOLP;
		char *cFOLap;
		i = 0, j = 0;
		FOLP = folapnode->first_node("VNIRFieldOverLap");
		if (FOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = FOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<2)
			{
				m_P1Proc_Param.m_fFOLapX[i] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<2)
			{
				m_P1Proc_Param.m_fFOLapY[j] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
		i = 0, j = 0;
		FOLP = folapnode->first_node("SWIRFieldOverLap");
		if (FOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = FOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<2)
			{
				m_P1Proc_Param.m_fFOLapX[i + 2] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<2)
			{
				m_P1Proc_Param.m_fFOLapY[j + 2] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
		i = 0, j = 0;
		FOLP = folapnode->first_node("TIHFieldOverLap");
		if (FOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = FOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<2)
			{
				m_P1Proc_Param.m_fFOLapX[i + 4] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<2)
			{
				m_P1Proc_Param.m_fFOLapY[j + 4] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
		i = 0, j = 0;
		FOLP = folapnode->first_node("UVFieldOverLap");
		if (FOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = FOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<2)
			{
				m_P1Proc_Param.m_fFOLapX[i + 6] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<2)
			{
				m_P1Proc_Param.m_fFOLapY[j + 6] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}


	}

	//获取谱段拼接参数
	xml_node<>* solapnode;
	solapnode = paranode->first_node("SpectrumOverLap");
	if (solapnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* SOLP;
		char *cFOLap;
		i = 0, j = 0;
		SOLP = solapnode->first_node("UVSpectrumOverLap");
		if (SOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = SOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<1)
			{
				m_P1Proc_Param.m_fSOLapX[i] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<1)
			{
				m_P1Proc_Param.m_fSOLapY[j] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
		i = 0, j = 0;
		SOLP = solapnode->first_node("SWIRSpectrumOverLap");
		if (SOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = SOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<1)
			{
				m_P1Proc_Param.m_fSOLapX[i + 1] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<1)
			{
				m_P1Proc_Param.m_fSOLapY[j + 1] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
		i = 0, j = 0;
		SOLP = solapnode->first_node("TIHSpectrumOverLap");
		if (SOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = SOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<1)
			{
				m_P1Proc_Param.m_fSOLapX[i + 2] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<1)
			{
				m_P1Proc_Param.m_fSOLapY[j + 2] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
	}


	//获取快视图参数
	xml_node<>* jpgnode;
	jpgnode = paranode->first_node("QuickView");
	if (jpgnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* jpgpara;
		jpgpara = jpgnode->first_node("WhetherQuickView");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cQuickView = jpgpara->value();
		if (cQuickView[0] == '0')
		{
			m_P1Proc_Param.m_bQuickView = 0;
		}
		else
		{
			m_P1Proc_Param.m_bQuickView = 1;
		}
		jpgpara = jpgnode->first_node("QuickViewScale");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cScale = jpgpara->value();
		m_P1Proc_Param.m_fScale = (float)atof(cScale);

		jpgpara = jpgnode->first_node("UVQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_P1Proc_Param.m_nUVBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
		jpgpara = jpgnode->first_node("VNIRQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_P1Proc_Param.m_nVNIRBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
		jpgpara = jpgnode->first_node("SWIRQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_P1Proc_Param.m_nSWIRBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
		jpgpara = jpgnode->first_node("TIHQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_P1Proc_Param.m_nTIHBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
	}

	//获取处理进度文件
	xml_node<>* repnode = NULL;
	repnode = paranode->first_node("ReportFile");
	if (repnode == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P1Proc_Param.m_pProFile = repnode->value();
	return lError;
}
long QPDXMLParam::SaveP1ProcXML(const char* pCatalogOrderFile,ParamP1 &pP1Proc_Param)
{
	long lError = 0;
	char cUVBand[50], cVNIRBand[50], cSWIRBand[50], cTIHBand[50];
	char cScale[50];

	string text;
	ofstream out;

	std::locale::global(std::locale(""));
	xml_document<> doc;
	xml_node<>* root = doc.allocate_node(node_pi, doc.allocate_string("xml version='1.0'encoding='utf-8'"));
	doc.append_node(root);

	xml_node<>* node = doc.allocate_node(node_element, "P1Task", NULL);
	doc.append_node(node);

	xml_node<>* inputnode = doc.allocate_node(node_element, "InputFilelist"); //num = '24', "36"
	inputnode->append_node(doc.allocate_node(node_element, "UVD0Data1", pP1Proc_Param.m_pD0File[0].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0Data2", pP1Proc_Param.m_pD0File[1].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0Data3", pP1Proc_Param.m_pD0File[2].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data1", pP1Proc_Param.m_pD0File[3].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data2", pP1Proc_Param.m_pD0File[4].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data3", pP1Proc_Param.m_pD0File[5].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data1", pP1Proc_Param.m_pD0File[6].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data2", pP1Proc_Param.m_pD0File[7].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data3", pP1Proc_Param.m_pD0File[8].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Data1", pP1Proc_Param.m_pD0File[9].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Data2", pP1Proc_Param.m_pD0File[10].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Data3", pP1Proc_Param.m_pD0File[11].c_str()));

	inputnode->append_node(doc.allocate_node(node_element, "UVD0Dark1", pP1Proc_Param.pD0DarkFile[0].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0Dark2", pP1Proc_Param.pD0DarkFile[1].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0Dark3", pP1Proc_Param.pD0DarkFile[2].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark1", pP1Proc_Param.pD0DarkFile[3].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark2", pP1Proc_Param.pD0DarkFile[4].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark3", pP1Proc_Param.pD0DarkFile[5].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark1", pP1Proc_Param.pD0DarkFile[6].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark2", pP1Proc_Param.pD0DarkFile[7].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark3", pP1Proc_Param.pD0DarkFile[8].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark1", pP1Proc_Param.pD0DarkFile[9].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark2", pP1Proc_Param.pD0DarkFile[10].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark3", pP1Proc_Param.pD0DarkFile[11].c_str()));

	inputnode->append_node(doc.allocate_node(node_element, "UVD0XML1", pP1Proc_Param.pD0MetaXML[0].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0XML2", pP1Proc_Param.pD0MetaXML[1].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "UVD0XML3", pP1Proc_Param.pD0MetaXML[2].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML1", pP1Proc_Param.pD0MetaXML[3].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML2", pP1Proc_Param.pD0MetaXML[4].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML3", pP1Proc_Param.pD0MetaXML[5].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML1", pP1Proc_Param.pD0MetaXML[6].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML2", pP1Proc_Param.pD0MetaXML[7].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML3", pP1Proc_Param.pD0MetaXML[8].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0XML1", pP1Proc_Param.pD0MetaXML[9].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0XML2", pP1Proc_Param.pD0MetaXML[10].c_str()));
	inputnode->append_node(doc.allocate_node(node_element, "TIHD0XML3", pP1Proc_Param.pD0MetaXML[11].c_str()));

	node->append_node(inputnode);

	xml_node<>* outputnode = doc.allocate_node(node_element, "OutputFilelist");
	outputnode->append_node(doc.allocate_node(node_element, "UVP0Data1", pP1Proc_Param.m_pP0Product[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0Data2",  pP1Proc_Param.m_pP0Product[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0Data3",  pP1Proc_Param.m_pP0Product[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0Data1",  pP1Proc_Param.m_pP0Product[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0Data2",  pP1Proc_Param.m_pP0Product[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0Data3",  pP1Proc_Param.m_pP0Product[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0Data1",  pP1Proc_Param.m_pP0Product[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0Data2",  pP1Proc_Param.m_pP0Product[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0Data3",  pP1Proc_Param.m_pP0Product[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0Data1",  pP1Proc_Param.m_pP0Product[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0Data2",  pP1Proc_Param.m_pP0Product[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0Data3",  pP1Proc_Param.m_pP0Product[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP0XML1", pP1Proc_Param.m_pP0ProductXML[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0XML2", pP1Proc_Param.m_pP0ProductXML[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0XML3", pP1Proc_Param.m_pP0ProductXML[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0XML1", pP1Proc_Param.m_pP0ProductXML[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0XML2", pP1Proc_Param.m_pP0ProductXML[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0XML3", pP1Proc_Param.m_pP0ProductXML[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0XML1", pP1Proc_Param.m_pP0ProductXML[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0XML2", pP1Proc_Param.m_pP0ProductXML[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0XML3", pP1Proc_Param.m_pP0ProductXML[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0XML1", pP1Proc_Param.m_pP0ProductXML[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0XML2", pP1Proc_Param.m_pP0ProductXML[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0XML3", pP1Proc_Param.m_pP0ProductXML[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP0JPG1", pP1Proc_Param.m_pP0JPG[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0JPG2", pP1Proc_Param.m_pP0JPG[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP0JPG3", pP1Proc_Param.m_pP0JPG[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0JPG1", pP1Proc_Param.m_pP0JPG[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0JPG2", pP1Proc_Param.m_pP0JPG[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP0JPG3", pP1Proc_Param.m_pP0JPG[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0JPG1", pP1Proc_Param.m_pP0JPG[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0JPG2", pP1Proc_Param.m_pP0JPG[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP0JPG3", pP1Proc_Param.m_pP0JPG[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0JPG1", pP1Proc_Param.m_pP0JPG[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0JPG2", pP1Proc_Param.m_pP0JPG[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP0JPG3", pP1Proc_Param.m_pP0JPG[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVEvent1", pP1Proc_Param.m_pEvent[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVEvent2", pP1Proc_Param.m_pEvent[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVEvent3", pP1Proc_Param.m_pEvent[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIREvent1", pP1Proc_Param.m_pEvent[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIREvent2", pP1Proc_Param.m_pEvent[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIREvent3", pP1Proc_Param.m_pEvent[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIREvent1", pP1Proc_Param.m_pEvent[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIREvent2", pP1Proc_Param.m_pEvent[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIREvent3", pP1Proc_Param.m_pEvent[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHEvent1", pP1Proc_Param.m_pEvent[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHEvent2", pP1Proc_Param.m_pEvent[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHEvent3", pP1Proc_Param.m_pEvent[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1AData1", pP1Proc_Param.m_pP1AData[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1AData2", pP1Proc_Param.m_pP1AData[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1AData3", pP1Proc_Param.m_pP1AData[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AData1", pP1Proc_Param.m_pP1AData[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AData2", pP1Proc_Param.m_pP1AData[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AData3", pP1Proc_Param.m_pP1AData[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AData1", pP1Proc_Param.m_pP1AData[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AData2", pP1Proc_Param.m_pP1AData[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AData3", pP1Proc_Param.m_pP1AData[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1AData1", pP1Proc_Param.m_pP1AData[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1AData2", pP1Proc_Param.m_pP1AData[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1AData3", pP1Proc_Param.m_pP1AData[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1AXML1", pP1Proc_Param.m_pP1AXML[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1AXML2", pP1Proc_Param.m_pP1AXML[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1AXML3", pP1Proc_Param.m_pP1AXML[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AXML1", pP1Proc_Param.m_pP1AXML[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AXML2", pP1Proc_Param.m_pP1AXML[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AXML3", pP1Proc_Param.m_pP1AXML[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AXML1", pP1Proc_Param.m_pP1AXML[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AXML2", pP1Proc_Param.m_pP1AXML[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AXML3", pP1Proc_Param.m_pP1AXML[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1AXML1", pP1Proc_Param.m_pP1AXML[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1AXML2", pP1Proc_Param.m_pP1AXML[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1AXML3", pP1Proc_Param.m_pP1AXML[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1AJPG1", pP1Proc_Param.m_pP1AJPG[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1AJPG2", pP1Proc_Param.m_pP1AJPG[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1AJPG3", pP1Proc_Param.m_pP1AJPG[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AJPG1", pP1Proc_Param.m_pP1AJPG[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AJPG2", pP1Proc_Param.m_pP1AJPG[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AJPG3", pP1Proc_Param.m_pP1AJPG[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AJPG1", pP1Proc_Param.m_pP1AJPG[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AJPG2", pP1Proc_Param.m_pP1AJPG[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AJPG3", pP1Proc_Param.m_pP1AJPG[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1AJPG1", pP1Proc_Param.m_pP1AJPG[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1AJPG2", pP1Proc_Param.m_pP1AJPG[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1AJPG3", pP1Proc_Param.m_pP1AJPG[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1BData1", pP1Proc_Param.m_pP1BData[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1BData2", pP1Proc_Param.m_pP1BData[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1BData3", pP1Proc_Param.m_pP1BData[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BData1", pP1Proc_Param.m_pP1BData[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BData2", pP1Proc_Param.m_pP1BData[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BData3", pP1Proc_Param.m_pP1BData[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BData1", pP1Proc_Param.m_pP1BData[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BData2", pP1Proc_Param.m_pP1BData[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BData3", pP1Proc_Param.m_pP1BData[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1BData1", pP1Proc_Param.m_pP1BData[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1BData2", pP1Proc_Param.m_pP1BData[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1BData3", pP1Proc_Param.m_pP1BData[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1BXML1",  pP1Proc_Param.m_pP1BXML[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1BXML2",  pP1Proc_Param.m_pP1BXML[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1BXML3",  pP1Proc_Param.m_pP1BXML[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BXML1",  pP1Proc_Param.m_pP1BXML[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BXML2",  pP1Proc_Param.m_pP1BXML[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BXML3",  pP1Proc_Param.m_pP1BXML[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BXML1",  pP1Proc_Param.m_pP1BXML[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BXML2",  pP1Proc_Param.m_pP1BXML[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BXML3",  pP1Proc_Param.m_pP1BXML[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1BXML1",  pP1Proc_Param.m_pP1BXML[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1BXML2",  pP1Proc_Param.m_pP1BXML[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1BXML3",  pP1Proc_Param.m_pP1BXML[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1BJPG1",  pP1Proc_Param.m_pP1BJPG[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1BJPG2",  pP1Proc_Param.m_pP1BJPG[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1BJPG3",  pP1Proc_Param.m_pP1BJPG[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BJPG1",  pP1Proc_Param.m_pP1BJPG[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BJPG2",  pP1Proc_Param.m_pP1BJPG[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BJPG3",  pP1Proc_Param.m_pP1BJPG[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BJPG1",  pP1Proc_Param.m_pP1BJPG[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BJPG2",  pP1Proc_Param.m_pP1BJPG[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BJPG3",  pP1Proc_Param.m_pP1BJPG[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1BJPG1",  pP1Proc_Param.m_pP1BJPG[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1BJPG2",  pP1Proc_Param.m_pP1BJPG[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1BJPG3",  pP1Proc_Param.m_pP1BJPG[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1CData1",  pP1Proc_Param.m_pP1CData[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1CData2",  pP1Proc_Param.m_pP1CData[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1CData3",  pP1Proc_Param.m_pP1CData[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CData1",  pP1Proc_Param.m_pP1CData[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CData2",  pP1Proc_Param.m_pP1CData[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CData3",  pP1Proc_Param.m_pP1CData[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CData1",  pP1Proc_Param.m_pP1CData[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CData2",  pP1Proc_Param.m_pP1CData[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CData3",  pP1Proc_Param.m_pP1CData[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1CData1",  pP1Proc_Param.m_pP1CData[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1CData2",  pP1Proc_Param.m_pP1CData[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1CData3",  pP1Proc_Param.m_pP1CData[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1CXML1",  pP1Proc_Param.m_pP1CXML[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1CXML2",  pP1Proc_Param.m_pP1CXML[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1CXML3",  pP1Proc_Param.m_pP1CXML[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CXML1",  pP1Proc_Param.m_pP1CXML[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CXML2",  pP1Proc_Param.m_pP1CXML[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CXML3",  pP1Proc_Param.m_pP1CXML[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CXML1",  pP1Proc_Param.m_pP1CXML[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CXML2",  pP1Proc_Param.m_pP1CXML[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CXML3",  pP1Proc_Param.m_pP1CXML[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1CXML1",  pP1Proc_Param.m_pP1CXML[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1CXML2",  pP1Proc_Param.m_pP1CXML[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1CXML3",  pP1Proc_Param.m_pP1CXML[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1CJPG1",  pP1Proc_Param.m_pP1CJPG[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1CJPG2",  pP1Proc_Param.m_pP1CJPG[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "UVP1CJPG3",  pP1Proc_Param.m_pP1CJPG[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CJPG1",  pP1Proc_Param.m_pP1CJPG[3].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CJPG2",  pP1Proc_Param.m_pP1CJPG[4].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CJPG3",  pP1Proc_Param.m_pP1CJPG[5].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CJPG1",  pP1Proc_Param.m_pP1CJPG[6].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CJPG2",  pP1Proc_Param.m_pP1CJPG[7].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CJPG3",  pP1Proc_Param.m_pP1CJPG[8].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1CJPG1",  pP1Proc_Param.m_pP1CJPG[9].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1CJPG2",  pP1Proc_Param.m_pP1CJPG[10].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1CJPG3",  pP1Proc_Param.m_pP1CJPG[11].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1DData",  pP1Proc_Param.m_pP1DData[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1DData",  pP1Proc_Param.m_pP1DData[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1DData",  pP1Proc_Param.m_pP1DData[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1DData",  pP1Proc_Param.m_pP1DData[3].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1DXML",  pP1Proc_Param.m_pP1DXML[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1DXML",  pP1Proc_Param.m_pP1DXML[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1DXML",  pP1Proc_Param.m_pP1DXML[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1DXML",  pP1Proc_Param.m_pP1DXML[3].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1DJPG",  pP1Proc_Param.m_pP1DJPG[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1DJPG",  pP1Proc_Param.m_pP1DJPG[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1DJPG",  pP1Proc_Param.m_pP1DJPG[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1DJPG",  pP1Proc_Param.m_pP1DJPG[3].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1EData",  pP1Proc_Param.m_pP1EData[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1EData",  pP1Proc_Param.m_pP1EData[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1EData",  pP1Proc_Param.m_pP1EData[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1EData",  pP1Proc_Param.m_pP1EData[3].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1EXML",  pP1Proc_Param.m_pP1EXML[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1EXML",  pP1Proc_Param.m_pP1EXML[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1EXML",  pP1Proc_Param.m_pP1EXML[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1EXML",  pP1Proc_Param.m_pP1EXML[3].c_str()));

	outputnode->append_node(doc.allocate_node(node_element, "UVP1EJPG",  pP1Proc_Param.m_pP1EJPG[0].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "VNIRP1EJPG",  pP1Proc_Param.m_pP1EJPG[1].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "SWIRP1EJPG",  pP1Proc_Param.m_pP1EJPG[2].c_str()));
	outputnode->append_node(doc.allocate_node(node_element, "TIHP1EJPG",  pP1Proc_Param.m_pP1EJPG[3].c_str()));

	node->append_node(outputnode);

	xml_node<>* paranode = doc.allocate_node(node_element, "Parament");
	node->append_node(paranode);

	xml_node<>* recofnode = doc.allocate_node(node_element, "RadiateReleCof");
	recofnode->append_node(doc.allocate_node(node_element, "UVReleCof1",  pP1Proc_Param.m_pReleCof[0].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "UVReleCof2",  pP1Proc_Param.m_pReleCof[1].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "UVReleCof3",  pP1Proc_Param.m_pReleCof[2].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "VNIRReleCof1",  pP1Proc_Param.m_pReleCof[3].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "VNIRReleCof2",  pP1Proc_Param.m_pReleCof[4].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "VNIRReleCof3",  pP1Proc_Param.m_pReleCof[5].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "SWIRReleCof1",  pP1Proc_Param.m_pReleCof[6].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "SWIRReleCof2",  pP1Proc_Param.m_pReleCof[7].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "SWIRReleCof3",  pP1Proc_Param.m_pReleCof[8].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "TIHReleCof1",  pP1Proc_Param.m_pReleCof[9].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "TIHReleCof2",  pP1Proc_Param.m_pReleCof[10].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "TIHReleCof3",  pP1Proc_Param.m_pReleCof[11].c_str()));
	paranode->append_node(recofnode);

	xml_node<>* abcofnode = doc.allocate_node(node_element, "RadiateAbCof");
	abcofnode->append_node(doc.allocate_node(node_element, "UVAbCof1",  pP1Proc_Param.m_pAbCof[0].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "UVAbCof2",  pP1Proc_Param.m_pAbCof[1].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "UVAbCof3",  pP1Proc_Param.m_pAbCof[2].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "VNIRAbCof1",  pP1Proc_Param.m_pAbCof[3].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "VNIRAbCof2",  pP1Proc_Param.m_pAbCof[4].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "VNIRAbCof3",  pP1Proc_Param.m_pAbCof[5].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "SWIRAbCof1",  pP1Proc_Param.m_pAbCof[6].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "SWIRAbCof2",  pP1Proc_Param.m_pAbCof[7].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "SWIRAbCof3",  pP1Proc_Param.m_pAbCof[8].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "TIHAbCof1",  pP1Proc_Param.m_pAbCof[9].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "TIHAbCof2",  pP1Proc_Param.m_pAbCof[10].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "TIHAbCof3",  pP1Proc_Param.m_pAbCof[11].c_str()));
	paranode->append_node(abcofnode);

	xml_node<>* splnode = doc.allocate_node(node_element, "WaveLength");
	splnode->append_node(doc.allocate_node(node_element, "Modtran",  pP1Proc_Param.m_pModtran.c_str()));
	splnode->append_node(doc.allocate_node(node_element, "UVWaveLen1",  pP1Proc_Param.m_pWaveLen[0].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "UVWaveLen2",  pP1Proc_Param.m_pWaveLen[1].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "UVWaveLen3",  pP1Proc_Param.m_pWaveLen[2].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "VNIRWaveLen1",  pP1Proc_Param.m_pWaveLen[3].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "VNIRWaveLen2",  pP1Proc_Param.m_pWaveLen[4].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "VNIRWaveLen3",  pP1Proc_Param.m_pWaveLen[5].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "SWIRWaveLen1",  pP1Proc_Param.m_pWaveLen[6].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "SWIRWaveLen2",  pP1Proc_Param.m_pWaveLen[7].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "SWIRWaveLen3",  pP1Proc_Param.m_pWaveLen[8].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "TIHWaveLen1",  pP1Proc_Param.m_pWaveLen[9].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "TIHWaveLen2",  pP1Proc_Param.m_pWaveLen[10].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "TIHWaveLen3",  pP1Proc_Param.m_pWaveLen[11].c_str()));
	paranode->append_node(splnode);

	char cUVFOLap[200], cVNIRFOLap[200], cSWIRFOLap[200], cTIHFOLap[200];
	xml_node<>* folapnode = doc.allocate_node(node_element, "FieldOverLap");
	sprintf_s(cVNIRFOLap, sizeof(cVNIRFOLap), "%f,%f,%f,%f",  pP1Proc_Param.m_fFOLapX[0],  pP1Proc_Param.m_fFOLapX[1],  pP1Proc_Param.m_fFOLapY[0],  pP1Proc_Param.m_fFOLapY[1]);
	folapnode->append_node(doc.allocate_node(node_element, "VNIRFieldOverLap", cVNIRFOLap));
	sprintf_s(cSWIRFOLap, sizeof(cSWIRFOLap), "%f,%f,%f,%f",  pP1Proc_Param.m_fFOLapX[2],  pP1Proc_Param.m_fFOLapX[3],  pP1Proc_Param.m_fFOLapY[2],  pP1Proc_Param.m_fFOLapY[3]);
	folapnode->append_node(doc.allocate_node(node_element, "SWIRFieldOverLap", cSWIRFOLap));
	sprintf_s(cTIHFOLap, sizeof(cTIHFOLap), "%f,%f,%f,%f",  pP1Proc_Param.m_fFOLapX[4],  pP1Proc_Param.m_fFOLapX[5],  pP1Proc_Param.m_fFOLapY[4],  pP1Proc_Param.m_fFOLapY[5]);
	folapnode->append_node(doc.allocate_node(node_element, "TIHFieldOverLap", cTIHFOLap));
	sprintf_s(cUVFOLap, sizeof(cUVFOLap), "%f,%f,%f,%f",  pP1Proc_Param.m_fFOLapX[6],  pP1Proc_Param.m_fFOLapX[7],  pP1Proc_Param.m_fFOLapY[6],  pP1Proc_Param.m_fFOLapY[7]);
	folapnode->append_node(doc.allocate_node(node_element, "UVFieldOverLap", cUVFOLap));
	paranode->append_node(folapnode);

	char cUVSOLap[100], cSWIRSOLap[100], cTIHSOLap[100];
	xml_node<>* solapnode = doc.allocate_node(node_element, "SpectrumOverLap");
	sprintf_s(cUVSOLap, sizeof(cUVSOLap), "%f,%f",  pP1Proc_Param.m_fSOLapX[0],  pP1Proc_Param.m_fSOLapY[0]);
	solapnode->append_node(doc.allocate_node(node_element, "UVSpectrumOverLap", cUVSOLap));
	sprintf_s(cSWIRSOLap, sizeof(cSWIRSOLap), "%f,%f",  pP1Proc_Param.m_fSOLapX[1],  pP1Proc_Param.m_fSOLapY[1]);
	solapnode->append_node(doc.allocate_node(node_element, "SWIRSpectrumOverLap", cSWIRSOLap));
	sprintf_s(cTIHSOLap, sizeof(cTIHSOLap), "%f,%f",  pP1Proc_Param.m_fSOLapX[2],  pP1Proc_Param.m_fSOLapY[2]);
	solapnode->append_node(doc.allocate_node(node_element, "TIHSpectrumOverLap", cTIHSOLap));
	paranode->append_node(solapnode);

	xml_node<>* jpgnode = doc.allocate_node(node_element, "QuickView");
	if ( pP1Proc_Param.m_bQuickView == 0)
	{
		jpgnode->append_node(doc.allocate_node(node_element, "WhetherQuickView", "0"));
		jpgnode->append_node(doc.allocate_node(node_element, "QuickViewScale", "0"));
		jpgnode->append_node(doc.allocate_node(node_element, "UVQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "VNIRQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "SWIRQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "TIHQuickBand", "-1"));
	}
	else
	{
		jpgnode->append_node(doc.allocate_node(node_element, "WhetherQuickView", "1"));
		sprintf_s(cScale, sizeof(cScale), "%f", pP1Proc_Param.m_fScale);
		jpgnode->append_node(doc.allocate_node(node_element, "QuickViewScale", cScale));
		sprintf_s(cUVBand, sizeof(cUVBand), "%d,%d,%d",  pP1Proc_Param.m_nUVBand[0],  pP1Proc_Param.m_nUVBand[1],  pP1Proc_Param.m_nUVBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "UVQuickBand", cUVBand));
		sprintf_s(cVNIRBand, sizeof(cVNIRBand), "%d,%d,%d",  pP1Proc_Param.m_nVNIRBand[0],  pP1Proc_Param.m_nVNIRBand[1],  pP1Proc_Param.m_nVNIRBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "VNIRQuickBand", cVNIRBand));
		sprintf_s(cSWIRBand, sizeof(cSWIRBand), "%d,%d,%d",  pP1Proc_Param.m_nSWIRBand[0],  pP1Proc_Param.m_nSWIRBand[1],  pP1Proc_Param.m_nSWIRBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "SWIRQuickBand", cSWIRBand));
		sprintf_s(cTIHBand, sizeof(cTIHBand), "%d,%d,%d",  pP1Proc_Param.m_nTIHBand[0],  pP1Proc_Param.m_nTIHBand[1],  pP1Proc_Param.m_nTIHBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "TIHQuickBand", cTIHBand));
	}
	paranode->append_node(jpgnode);

	xml_node<>* repnode = doc.allocate_node(node_element, "ReportFile",  pP1Proc_Param.m_pProFile.c_str());
	paranode->append_node(repnode);

	string str;
	rapidxml::print(std::back_inserter(str), doc, 0);
	ofstream out1(pCatalogOrderFile);
	out1 << doc;
	out1.close();

	return lError;
}

long QPDXMLParam::LoadP2ProcXML(const char* pCatalogOrderFile)
{
	long lError = 0;
	char *cQuickView = "";
	char *cBand = "";
	char *buf = "";
	char *temp = "";
	char *cScale = "";
	int i = 0, j = 0;

	std::locale::global(std::locale(""));
	file<> fdoc(pCatalogOrderFile);

	xml_document<> doc;
	doc.parse<0>(fdoc.data());

	xml_node<>* root = doc.first_node();
	if (root == NULL)
	{
		lError = 1;
		return lError;
	}

	//==============================获取输入数据===============================================
	xml_node<> *inputnode = root->first_node("InputFilelist");
	if (inputnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		//获取原始图像数据
		xml_node<>* D0Data;
		D0Data = inputnode->first_node("UVD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("UVD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("UVD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("VNIRD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("VNIRD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("VNIRD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("SWIRD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("SWIRD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("SWIRD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("TIHD0Data1");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("TIHD0Data2");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());
		D0Data = inputnode->first_node("TIHD0Data3");
		if (D0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pD0File.push_back(D0Data->value());

		//获取暗电流数据
		xml_node<>* D0Dark;
		D0Dark = inputnode->first_node("UVD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("UVD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("UVD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("VNIRD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("VNIRD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("VNIRD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("SWIRD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("SWIRD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("SWIRD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("TIHD0Dark1");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("TIHD0Dark2");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());
		D0Dark = inputnode->first_node("TIHD0Dark3");
		if (D0Dark == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0DarkFile.push_back(D0Dark->value());

		//获取0级产品元数据文件
		xml_node<>* D0XML;
		D0XML = inputnode->first_node("UVD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("UVD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("UVD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("VNIRD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("VNIRD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("VNIRD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("SWIRD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("SWIRD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("SWIRD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("TIHD0XML1");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("TIHD0XML2");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
		D0XML = inputnode->first_node("TIHD0XML3");
		if (D0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.pD0MetaXML.push_back(D0XML->value());
	}


	//==============================获取输出数据===============================================
	xml_node<> *outputnode = root->first_node("OutputFilelist");
	if (outputnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		//获取0级产品=================================
		xml_node<>* P0Data;
		P0Data = outputnode->first_node("UVP0Data1");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("UVP0Data2");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("UVP0Data3");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("VNIRP0Data1");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("VNIRP0Data2");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("VNIRP0Data3");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("SWIRP0Data1");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("SWIRP0Data2");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("SWIRP0Data3");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("TIHP0Data1");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("TIHP0Data2");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());
		P0Data = outputnode->first_node("TIHP0Data3");
		if (P0Data == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0Product.push_back(P0Data->value());

		//获取0级产品元数据文件
		xml_node<>* P0XML;
		P0XML = outputnode->first_node("UVP0XML1");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("UVP0XML2");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("UVP0XML3");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("VNIRP0XML1");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("VNIRP0XML2");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("VNIRP0XML3");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("SWIRP0XML1");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("SWIRP0XML2");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("SWIRP0XML3");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("TIHP0XML1");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("TIHP0XML2");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());
		P0XML = outputnode->first_node("TIHP0XML3");
		if (P0XML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0ProductXML.push_back(P0XML->value());

		//获取0级快视图
		xml_node<>* P0JPG;
		P0JPG = outputnode->first_node("UVP0JPG1");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("UVP0JPG2");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("UVP0JPG3");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("VNIRP0JPG1");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("VNIRP0JPG2");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("VNIRP0JPG3");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("SWIRP0JPG1");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("SWIRP0JPG2");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("SWIRP0JPG3");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("TIHP0JPG1");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("TIHP0JPG2");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());
		P0JPG = outputnode->first_node("TIHP0JPG3");
		if (P0JPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(P0JPG->value());

		//获取EVENT
		xml_node<>* Event;
		Event = outputnode->first_node("UVEvent1");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());
		Event = outputnode->first_node("UVEvent2");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());
		Event = outputnode->first_node("UVEvent3");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());
		Event = outputnode->first_node("VNIREvent1");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());
		Event = outputnode->first_node("VNIREvent2");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());
		Event = outputnode->first_node("VNIREvent3");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());
		Event = outputnode->first_node("SWIREvent1");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());
		Event = outputnode->first_node("SWIREvent2");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());
		Event = outputnode->first_node("SWIREvent3");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());
		Event = outputnode->first_node("TIHEvent1");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());
		Event = outputnode->first_node("TIHEvent2");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());
		Event = outputnode->first_node("TIHEvent3");
		if (Event == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP0JPG.push_back(Event->value());

		//获取P1A级产品====================================
		xml_node<>* P1AData;
		P1AData = outputnode->first_node("UVP1AData1");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("UVP1AData2");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("UVP1AData3");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("VNIRP1AData1");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("VNIRP1AData2");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("VNIRP1AData3");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("SWIRP1AData1");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("SWIRP1AData2");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("SWIRP1AData3");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("TIHP1AData1");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("TIHP1AData2");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());
		P1AData = outputnode->first_node("TIHP1AData3");
		if (P1AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AData->value());


		//获取0级产品元数据文件
		xml_node<>* P1AXML;
		P1AXML = outputnode->first_node("UVP1AXML1");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("UVP1AXML2");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("UVP1AXML3");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("VNIRP1AXML1");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("VNIRP1AXML2");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("VNIRP1AXML3");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("SWIRP1AXML1");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("SWIRP1AXML2");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("SWIRP1AXML3");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("TIHP1AXML1");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("TIHP1AXML2");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());
		P1AXML = outputnode->first_node("TIHP1AXML3");
		if (P1AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AData.push_back(P1AXML->value());

		//获取P1A快视图
		xml_node<>* P1AJPG;
		P1AJPG = outputnode->first_node("UVP1AJPG1");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("UVP1AJPG2");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("UVP1AJPG3");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("VNIRP1AJPG1");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("VNIRP1AJPG2");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("VNIRP1AJPG3");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("SWIRP1AJPG1");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("SWIRP1AJPG2");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("SWIRP1AJPG3");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("TIHP1AJPG1");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("TIHP1AJPG2");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());
		P1AJPG = outputnode->first_node("TIHP1AJPG3");
		if (P1AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1AJPG.push_back(P1AJPG->value());

		//获取P1B级产品
		xml_node<>* P1BData;
		P1BData = outputnode->first_node("UVP1BData1");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("UVP1BData2");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("UVP1BData3");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("VNIRP1BData1");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("VNIRP1BData2");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("VNIRP1BData3");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("SWIRP1BData1");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("SWIRP1BData2");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("SWIRP1BData3");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("TIHP1BData1");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("TIHP1BData2");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());
		P1BData = outputnode->first_node("TIHP1BData3");
		if (P1BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BData.push_back(P1BData->value());


		//获取P1B级产品元数据文件
		xml_node<>* P1BXML;
		P1BXML = outputnode->first_node("UVP1BXML1");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("UVP1BXML2");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("UVP1BXML3");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("VNIRP1BXML1");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("VNIRP1BXML2");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("VNIRP1BXML3");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("SWIRP1BXML1");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("SWIRP1BXML2");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("SWIRP1BXML3");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("TIHP1BXML1");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("TIHP1BXML2");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());
		P1BXML = outputnode->first_node("TIHP1BXML3");
		if (P1BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BXML.push_back(P1BXML->value());

		//获取P1B快视图
		xml_node<>* P1BJPG;
		P1BJPG = outputnode->first_node("UVP1BJPG1");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("UVP1BJPG2");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("UVP1BJPG3");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("VNIRP1BJPG1");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("VNIRP1BJPG2");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("VNIRP1BJPG3");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("SWIRP1BJPG1");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("SWIRP1BJPG2");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("SWIRP1BJPG3");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("TIHP1BJPG1");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("TIHP1BJPG2");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());
		P1BJPG = outputnode->first_node("TIHP1BJPG3");
		if (P1BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1BJPG.push_back(P1BJPG->value());

		//获取P1C级产品
		xml_node<>* P1CData;
		P1CData = outputnode->first_node("UVP1CData1");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("UVP1CData2");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("UVP1CData3");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("VNIRP1CData1");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("VNIRP1CData2");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("VNIRP1CData3");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("SWIRP1CData1");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("SWIRP1CData2");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("SWIRP1CData3");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("TIHP1CData1");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("TIHP1CData2");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());
		P1CData = outputnode->first_node("TIHP1CData3");
		if (P1CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CData.push_back(P1CData->value());


		//获取P1C级产品元数据文件
		xml_node<>* P1CXML;
		P1CXML = outputnode->first_node("UVP1CXML1");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("UVP1CXML2");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("UVP1CXML3");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("VNIRP1CXML1");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("VNIRP1CXML2");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("VNIRP1CXML3");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("SWIRP1CXML1");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("SWIRP1CXML2");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("SWIRP1CXML3");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("TIHP1CXML1");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("TIHP1CXML2");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());
		P1CXML = outputnode->first_node("TIHP1CXML3");
		if (P1CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CXML.push_back(P1CXML->value());

		//获取P1C快视图
		xml_node<>* P1CJPG;
		P1CJPG = outputnode->first_node("UVP1CJPG1");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("UVP1CJPG2");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("UVP1CJPG3");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("VNIRP1CJPG1");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("VNIRP1CJPG2");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("VNIRP1CJPG3");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("SWIRP1CJPG1");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("SWIRP1CJPG2");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("SWIRP1CJPG3");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("TIHP1CJPG1");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("TIHP1CJPG2");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());
		P1CJPG = outputnode->first_node("TIHP1CJPG3");
		if (P1CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1CJPG.push_back(P1CJPG->value());

		//获取P1D级产品
		xml_node<>* P1DData;
		P1DData = outputnode->first_node("UVP1DData");
		if (P1DData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DData.push_back(P1DData->value());
		P1DData = outputnode->first_node("VNIRP1DData");
		if (P1DData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DData.push_back(P1DData->value());
		P1DData = outputnode->first_node("SWIRP1DData");
		if (P1DData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DData.push_back(P1DData->value());
		P1DData = outputnode->first_node("TIHP1DData");
		if (P1DData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DData.push_back(P1DData->value());

		//获取P1D级产品元数据文件
		xml_node<>* P1DXML;
		P1DXML = outputnode->first_node("UVP1DXML");
		if (P1DXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DXML.push_back(P1DXML->value());
		P1DXML = outputnode->first_node("VNIRP1DXML");
		if (P1DXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DXML.push_back(P1DXML->value());
		P1DXML = outputnode->first_node("SWIRP1DXML");
		if (P1DXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DXML.push_back(P1DXML->value());
		P1DXML = outputnode->first_node("TIHP1DXML");
		if (P1DXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DXML.push_back(P1DXML->value());

		//获取P1D快视图
		xml_node<>* P1DJPG;
		P1DJPG = outputnode->first_node("UVP1DJPG");
		if (P1DJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DJPG.push_back(P1DJPG->value());
		P1DJPG = outputnode->first_node("VNIRP1DJPG");
		if (P1DJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DJPG.push_back(P1DJPG->value());
		P1DJPG = outputnode->first_node("SWIRP1DJPG");
		if (P1DJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DJPG.push_back(P1DJPG->value());
		P1DJPG = outputnode->first_node("TIHP1DJPG");
		if (P1DJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1DJPG.push_back(P1DJPG->value());

		//获取P1E级产品
		xml_node<>* P1EData;
		P1EData = outputnode->first_node("UVP1EData");
		if (P1EData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EData.push_back(P1EData->value());
		P1EData = outputnode->first_node("VNIRP1EData");
		if (P1EData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EData.push_back(P1EData->value());
		P1EData = outputnode->first_node("SWIRP1EData");
		if (P1EData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EData.push_back(P1EData->value());
		P1EData = outputnode->first_node("TIHP1EData");
		if (P1EData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EData.push_back(P1EData->value());

		//获取P1E级产品元数据文件
		xml_node<>* P1EXML;
		P1EXML = outputnode->first_node("UVP1EXML");
		if (P1EXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EXML.push_back(P1EXML->value());
		P1EXML = outputnode->first_node("VNIRP1EXML");
		if (P1EXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EXML.push_back(P1EXML->value());
		P1EXML = outputnode->first_node("SWIRP1EXML");
		if (P1EXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EXML.push_back(P1EXML->value());
		P1EXML = outputnode->first_node("TIHP1EXML");
		if (P1EXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EXML.push_back(P1EXML->value());

		//获取P1E快视图
		xml_node<>* P1EJPG;
		P1EJPG = outputnode->first_node("UVP1EJPG");
		if (P1EJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EJPG.push_back(P1EJPG->value());
		P1EJPG = outputnode->first_node("VNIRP1EJPG");
		if (P1EJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EJPG.push_back(P1EJPG->value());
		P1EJPG = outputnode->first_node("SWIRP1EJPG");
		if (P1EJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EJPG.push_back(P1EJPG->value());
		P1EJPG = outputnode->first_node("TIHP1EJPG");
		if (P1EJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP1EJPG.push_back(P1EJPG->value());

		//获取P2A级产品
		xml_node<>* P2AData;
		P2AData = outputnode->first_node("UVP2AData");
		if (P2AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AData.push_back(P2AData->value());
		P2AData = outputnode->first_node("VNIRP2AData");
		if (P2AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AData.push_back(P2AData->value());
		P2AData = outputnode->first_node("SWIRP2AData");
		if (P2AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AData.push_back(P2AData->value());
		P2AData = outputnode->first_node("TIHP2AData");
		if (P2AData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AData.push_back(P2AData->value());

		//获取P2A级产品元数据文件
		xml_node<>* P2AXML;
		P2AXML = outputnode->first_node("UVP2AXML");
		if (P2AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AXML.push_back(P2AXML->value());
		P2AXML = outputnode->first_node("VNIRP2AXML");
		if (P2AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AXML.push_back(P2AXML->value());
		P2AXML = outputnode->first_node("SWIRP2AXML");
		if (P2AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AXML.push_back(P2AXML->value());
		P2AXML = outputnode->first_node("TIHP2AXML");
		if (P2AXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AXML.push_back(P2AXML->value());

		//获取P2A快视图
		xml_node<>* P2AJPG;
		P2AJPG = outputnode->first_node("UVP2AJPG");
		if (P2AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AJPG.push_back(P2AJPG->value());
		P2AJPG = outputnode->first_node("VNIRP2AJPG");
		if (P2AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AJPG.push_back(P2AJPG->value());
		P2AJPG = outputnode->first_node("SWIRP2AJPG");
		if (P2AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AJPG.push_back(P2AJPG->value());
		P2AJPG = outputnode->first_node("TIHP2AJPG");
		if (P2AJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2AJPG.push_back(P2AJPG->value());

		//获取P2B级产品
		xml_node<>* P2BData;
		P2BData = outputnode->first_node("UVP2BData");
		if (P2BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BData.push_back(P2BData->value());
		P2BData = outputnode->first_node("VNIRP2BData");
		if (P2BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BData.push_back(P2BData->value());
		P2BData = outputnode->first_node("SWIRP2BData");
		if (P2BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BData.push_back(P2BData->value());
		P2BData = outputnode->first_node("TIHP2BData");
		if (P2BData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BData.push_back(P2BData->value());

		//获取P2B级产品元数据文件
		xml_node<>* P2BXML;
		P2BXML = outputnode->first_node("UVP2BXML");
		if (P2BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BXML.push_back(P2BXML->value());
		P2BXML = outputnode->first_node("VNIRP2BXML");
		if (P2BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BXML.push_back(P2BXML->value());
		P2BXML = outputnode->first_node("SWIRP2BXML");
		if (P2BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BXML.push_back(P2BXML->value());
		P2BXML = outputnode->first_node("TIHP2BXML");
		if (P2BXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BXML.push_back(P2BXML->value());

		//获取P2B快视图
		xml_node<>* P2BJPG;
		P2BJPG = outputnode->first_node("UVP2BJPG");
		if (P2BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BJPG.push_back(P2BJPG->value());
		P2BJPG = outputnode->first_node("VNIRP2BJPG");
		if (P2BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BJPG.push_back(P2BJPG->value());
		P2BJPG = outputnode->first_node("SWIRP2BJPG");
		if (P2BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BJPG.push_back(P2BJPG->value());
		P2BJPG = outputnode->first_node("TIHP2BJPG");
		if (P2BJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2BJPG.push_back(P2BJPG->value());
		//获取P2C级产品
		xml_node<>* P2CData;
		P2CData = outputnode->first_node("UVP2CData");
		if (P2CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CData.push_back(P2CData->value());
		P2CData = outputnode->first_node("VNIRP2CData");
		if (P2CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CData.push_back(P2CData->value());
		P2CData = outputnode->first_node("SWIRP2CData");
		if (P2CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CData.push_back(P2CData->value());
		P2CData = outputnode->first_node("TIHP2CData");
		if (P2CData == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CData.push_back(P2CData->value());

		//获取P2C级产品元数据文件
		xml_node<>* P2CXML;
		P2CXML = outputnode->first_node("UVP2CXML");
		if (P2CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CXML.push_back(P2CXML->value());
		P2CXML = outputnode->first_node("VNIRP2CXML");
		if (P2CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CXML.push_back(P2CXML->value());
		P2CXML = outputnode->first_node("SWIRP2CXML");
		if (P2CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CXML.push_back(P2CXML->value());
		P2CXML = outputnode->first_node("TIHP2CXML");
		if (P2CXML == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CXML.push_back(P2CXML->value());

		//获取P2C快视图
		xml_node<>* P2CJPG;
		P2CJPG = outputnode->first_node("UVP2CJPG");
		if (P2CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CJPG.push_back(P2CJPG->value());
		P2CJPG = outputnode->first_node("VNIRP2CJPG");
		if (P2CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CJPG.push_back(P2CJPG->value());
		P2CJPG = outputnode->first_node("SWIRP2CJPG");
		if (P2CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CJPG.push_back(P2CJPG->value());
		P2CJPG = outputnode->first_node("TIHP2CJPG");
		if (P2CJPG == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pP2CJPG.push_back(P2CJPG->value());

		//获取pos
		xml_node<>* POS;
		POS = outputnode->first_node("UVPOS1");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());
		POS = outputnode->first_node("UVPOS2");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());
		POS = outputnode->first_node("UVPOS3");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());
		POS = outputnode->first_node("VNIRPOS1");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());
		POS = outputnode->first_node("VNIRPOS2");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());
		POS = outputnode->first_node("VNIRPOS3");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());
		POS = outputnode->first_node("SWIRPOS1");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());
		POS = outputnode->first_node("SWIRPOS2");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());
		POS = outputnode->first_node("SWIRPOS3");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());
		POS = outputnode->first_node("TIHPOS1");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());
		POS = outputnode->first_node("TIHPOS2");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());
		POS = outputnode->first_node("TIHPOS3");
		if (POS == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pPOSFile.push_back(POS->value());

		//获取Eof
		xml_node<>* Eof;
		Eof = outputnode->first_node("UVEOF1");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
		Eof = outputnode->first_node("UVEOF2");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
		Eof = outputnode->first_node("UVEOF3");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
		Eof = outputnode->first_node("VNIREOF1");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
		Eof = outputnode->first_node("VNIREOF2");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
		Eof = outputnode->first_node("VNIREOF3");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
		Eof = outputnode->first_node("SWIREOF1");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
		Eof = outputnode->first_node("SWIREOF2");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
		Eof = outputnode->first_node("SWIREOF3");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
		Eof = outputnode->first_node("TIHEOF1");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
		Eof = outputnode->first_node("TIHEOF2");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
		Eof = outputnode->first_node("TIHEOF3");
		if (Eof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pEOFile.push_back(Eof->value());
	}



	//==============================获取参数===============================================
	xml_node<> *paranode = root->first_node("Parament");
	if (paranode == NULL)
	{
		lError = 1;
		return lError;
	}
	//获取相对辐射校正系数
	xml_node<> *recofnode = paranode->first_node("RadiateReleCof");
	if (recofnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* ReleCof;
		ReleCof = recofnode->first_node("UVReleCof1");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("UVReleCof2");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("UVReleCof3");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("VNIRReleCof1");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("VNIRReleCof2");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("VNIRReleCof3");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("SWIRReleCof1");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("SWIRReleCof2");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("SWIRReleCof3");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("TIHReleCof1");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("TIHReleCof2");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
		ReleCof = recofnode->first_node("TIHReleCof3");
		if (ReleCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pReleCof.push_back(ReleCof->value());
	}

	//获取绝对辐射校正系数
	xml_node<> *abcofnode = paranode->first_node("RadiateAbCof");
	if (abcofnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* AbCof;
		AbCof = abcofnode->first_node("UVAbCof1");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("UVAbCof2");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("UVAbCof3");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("VNIRAbCof1");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("VNIRAbCof2");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("VNIRAbCof3");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("SWIRAbCof1");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("SWIRAbCof2");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("SWIRAbCof3");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("TIHAbCof1");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("TIHAbCof2");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
		AbCof = abcofnode->first_node("TIHAbCof3");
		if (AbCof == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pAbCof.push_back(AbCof->value());
	}

	//获取中心波长半波宽信息
	xml_node<> *splnode = paranode->first_node("WaveLength");
	if (splnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* WaveLen;
		WaveLen = splnode->first_node("Modtran");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pModtran = WaveLen->value();
		WaveLen = splnode->first_node("UVWaveLen1");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("UVWaveLen2");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("UVWaveLen3");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("VNIRWaveLen1");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("VNIRWaveLen2");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("VNIRWaveLen3");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("SWIRWaveLen1");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("SWIRWaveLen2");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("SWIRWaveLen3");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("TIHWaveLen1");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("TIHWaveLen2");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
		WaveLen = splnode->first_node("TIHWaveLen3");
		if (WaveLen == NULL)
		{
			lError = 1;
			return lError;
		}
		m_P2Proc_Param.m_pWaveLen.push_back(WaveLen->value());
	}

	//获取SBET文件
	xml_node<>* sbetnode = NULL;
	sbetnode = paranode->first_node("Sbet");
	if (sbetnode == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P2Proc_Param.m_pSBETFile = sbetnode->value();

	//获取DEM文件
	xml_node<>* demnode = NULL;
	demnode = paranode->first_node("DEM");
	if (demnode == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P2Proc_Param.m_pDEMFile = demnode->value();

	//获取视场拼接参数
	xml_node<>* folapnode;
	folapnode = paranode->first_node("FieldOverLap");
	if (folapnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* FOLP;
		char *cFOLap;
		i = 0, j = 0;
		FOLP = folapnode->first_node("VNIRFieldOverLap");
		if (FOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = FOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<2)
			{
				m_P2Proc_Param.m_fFOLapX[i] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<2)
			{
				m_P2Proc_Param.m_fFOLapY[j] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
		i = 0, j = 0;
		FOLP = folapnode->first_node("SWIRFieldOverLap");
		if (FOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = FOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<2)
			{
				m_P2Proc_Param.m_fFOLapX[i + 2] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<2)
			{
				m_P2Proc_Param.m_fFOLapY[j + 2] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
		i = 0, j = 0;
		FOLP = folapnode->first_node("TIHFieldOverLap");
		if (FOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = FOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<2)
			{
				m_P2Proc_Param.m_fFOLapX[i + 4] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<2)
			{
				m_P2Proc_Param.m_fFOLapY[j + 4] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
		i = 0, j = 0;
		FOLP = folapnode->first_node("UVFieldOverLap");
		if (FOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = FOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<2)
			{
				m_P2Proc_Param.m_fFOLapX[i + 6] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<2)
			{
				m_P2Proc_Param.m_fFOLapY[j + 6] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}


	}

	//获取谱段拼接参数
	xml_node<>* solapnode;
	solapnode = paranode->first_node("SpectrumOverLap");
	if (solapnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* SOLP;
		char *cFOLap;
		i = 0, j = 0;
		SOLP = solapnode->first_node("UVSpectrumOverLap");
		if (SOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = SOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<1)
			{
				m_P2Proc_Param.m_fSOLapX[i] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<1)
			{
				m_P2Proc_Param.m_fSOLapY[j] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
		i = 0, j = 0;
		SOLP = solapnode->first_node("SWIRSpectrumOverLap");
		if (SOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = SOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<1)
			{
				m_P2Proc_Param.m_fSOLapX[i + 1] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<1)
			{
				m_P2Proc_Param.m_fSOLapY[j + 1] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
		i = 0, j = 0;
		SOLP = solapnode->first_node("TIHSpectrumOverLap");
		if (SOLP == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFOLap = SOLP->value();
			temp = strtok_s(cFOLap, ",", &buf);
			while (temp && i<1)
			{
				m_P2Proc_Param.m_fSOLapX[i + 2] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
			while (temp && j<1)
			{
				m_P2Proc_Param.m_fSOLapY[j + 2] = (float)atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				j++;
			}
		}
	}

	//获取视场角、瞬时视场角、焦距
	char *cFov, *cIFov, *cFocalLen;
	xml_node<>* geonode;
	geonode = paranode->first_node("GeoCof");
	if (geonode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* geocof;
		i = 0;
		geocof = geonode->first_node("FOV");
		if (geocof == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFov = geocof->value();
			temp = strtok_s(cFov, ",", &buf);
			while (temp && i<4)
			{
				m_P2Proc_Param.m_fFov[i] = atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
		}

		i = 0;
		geocof = geonode->first_node("IFOV");
		if (geocof == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cIFov = geocof->value();
			temp = strtok_s(cIFov, ",", &buf);
			while (temp && i<8)
			{
				m_P2Proc_Param.m_fIFov[i] = atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
		}

		i = 0;
		geocof = geonode->first_node("FocalLen");
		if (geocof == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cFocalLen = geocof->value();
			temp = strtok_s(cFocalLen, ",", &buf);
			while (temp && i<4)
			{
				m_P2Proc_Param.m_fFocalLen[i] = atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
		}
	}

	char *cBoresightMis, *cGNSSOffset, *cXYZOffset, *cBands, *cSamples;
	//获取视准轴偏差和偏移距离参数
	xml_node<>* misnode;
	misnode = paranode->first_node("POSMiss");
	if (misnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* posmiss;
		i = 0;
		posmiss = misnode->first_node("BoresightMis");
		if (posmiss == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cBoresightMis = posmiss->value();
			temp = strtok_s(cBoresightMis, ",", &buf);
			while (temp && i<3)
			{
				m_P2Proc_Param.m_dBoresightMis[i] = atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
		}
		i = 0;
		posmiss = misnode->first_node("GNSSOffset");
		if (posmiss == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cGNSSOffset = posmiss->value();
			temp = strtok_s(cGNSSOffset, ",", &buf);
			while (temp && i<3)
			{
				m_P2Proc_Param.m_dGNSSOffset[i] = atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
		}
		i = 0;
		posmiss = misnode->first_node("XYZOffset");
		if (posmiss == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cXYZOffset = posmiss->value();
			temp = strtok_s(cXYZOffset, ",", &buf);
			while (temp && i<3)
			{
				m_P2Proc_Param.m_dXYZOffset[i] = atof(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
		}
	}

	//获取数据压缩后的波段和像元数目
	xml_node<>* mergenode;
	mergenode = paranode->first_node("Merge");
	if (misnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* mix;
		i = 0;
		mix = mergenode->first_node("Band");
		if (mix == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cBands = mix->value();
			temp = strtok_s(cBands, ",", &buf);
			while (temp && i < 4)
			{
				m_P2Proc_Param.m_nBand[i] = atoi(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
		}
		i = 0;
		mix = mergenode->first_node("Samples");
		if (mix == NULL)
		{
			lError = 1;
			return lError;
		}
		else
		{
			cSamples = mix->value();
			temp = strtok_s(cSamples, ",", &buf);
			while (temp && i < 4)
			{
				m_P2Proc_Param.m_nSamples[i] = atoi(temp);
				temp = strtok_s(NULL, ",", &buf);
				i++;
			}
		}
	}
	//获取快视图参数
	xml_node<>* jpgnode;
	jpgnode = paranode->first_node("QuickView");
	if (jpgnode == NULL)
	{
		lError = 1;
		return lError;
	}
	else
	{
		xml_node<>* jpgpara;
		jpgpara = jpgnode->first_node("WhetherQuickView");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cQuickView = jpgpara->value();
		if (cQuickView[0] == '0')
		{
			m_P2Proc_Param.m_bQuickView = 0;
		}
		else
		{
			m_P2Proc_Param.m_bQuickView = 1;
		}
		jpgpara = jpgnode->first_node("QuickViewScale");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cScale = jpgpara->value();
		m_P2Proc_Param.m_fScale = (float)atof(cScale);

		jpgpara = jpgnode->first_node("UVQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_P2Proc_Param.m_nUVBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
		jpgpara = jpgnode->first_node("VNIRQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_P2Proc_Param.m_nVNIRBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
		jpgpara = jpgnode->first_node("SWIRQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_P2Proc_Param.m_nSWIRBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
		jpgpara = jpgnode->first_node("TIHQuickBand");
		if (jpgpara == NULL)
		{
			lError = 1;
			return lError;
		}
		cBand = jpgpara->value();
		i = 0;
		temp = strtok_s(cBand, ",", &buf);
		while (temp && i<3)
		{
			m_P2Proc_Param.m_nTIHBand[i] = atoi(temp);
			temp = strtok_s(NULL, ",", &buf);
			i++;
		}
	}

	//获取处理进度文件
	xml_node<>* repnode = NULL;
	repnode = paranode->first_node("ReportFile");
	if (repnode == NULL)
	{
		lError = 1;
		return lError;
	}
	m_P2Proc_Param.m_pProFile = repnode->value();
	return lError;
}
long QPDXMLParam::SaveP2ProcXML(const char* pCatalogOrderFile,ParamP2 &pP2Proc_Param)
{
	long lError = 0;

	string text;
	ofstream out;

	std::locale::global(std::locale(""));
	xml_document<> doc;
	xml_node<>* root = doc.allocate_node(node_pi, doc.allocate_string("xml version='1.0'encoding='utf-8'"));
	doc.append_node(root);

	xml_node<>* node = doc.allocate_node(node_element, "P2Task", NULL);
	doc.append_node(node);

	xml_node<>* inputnode = doc.allocate_node(node_element, "InputFilelist");
	if (inputnode != 0)
	{
		inputnode->append_node(doc.allocate_node(node_element, "UVD0Data1", pP2Proc_Param.m_pD0File[0].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "UVD0Data2", pP2Proc_Param.m_pD0File[1].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "UVD0Data3", pP2Proc_Param.m_pD0File[2].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data1", pP2Proc_Param.m_pD0File[3].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data2", pP2Proc_Param.m_pD0File[4].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Data3", pP2Proc_Param.m_pD0File[5].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data1", pP2Proc_Param.m_pD0File[6].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data2", pP2Proc_Param.m_pD0File[7].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Data3", pP2Proc_Param.m_pD0File[8].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "TIHD0Data1", pP2Proc_Param.m_pD0File[9].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "TIHD0Data2", pP2Proc_Param.m_pD0File[10].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "TIHD0Data3", pP2Proc_Param.m_pD0File[11].c_str()));

		inputnode->append_node(doc.allocate_node(node_element, "UVD0Dark1", pP2Proc_Param.pD0DarkFile[0].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "UVD0Dark2", pP2Proc_Param.pD0DarkFile[1].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "UVD0Dark3", pP2Proc_Param.pD0DarkFile[2].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark1", pP2Proc_Param.pD0DarkFile[3].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark2", pP2Proc_Param.pD0DarkFile[4].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "VNIRD0Dark3", pP2Proc_Param.pD0DarkFile[5].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark1", pP2Proc_Param.pD0DarkFile[6].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark2", pP2Proc_Param.pD0DarkFile[7].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "SWIRD0Dark3", pP2Proc_Param.pD0DarkFile[8].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark1", pP2Proc_Param.pD0DarkFile[9].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark2", pP2Proc_Param.pD0DarkFile[10].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "TIHD0Dark3", pP2Proc_Param.pD0DarkFile[11].c_str()));

		inputnode->append_node(doc.allocate_node(node_element, "UVD0XML1", pP2Proc_Param.pD0MetaXML[0].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "UVD0XML2", pP2Proc_Param.pD0MetaXML[1].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "UVD0XML3", pP2Proc_Param.pD0MetaXML[2].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML1", pP2Proc_Param.pD0MetaXML[3].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML2", pP2Proc_Param.pD0MetaXML[4].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "VNIRD0XML3", pP2Proc_Param.pD0MetaXML[5].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML1", pP2Proc_Param.pD0MetaXML[6].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML2", pP2Proc_Param.pD0MetaXML[7].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "SWIRD0XML3", pP2Proc_Param.pD0MetaXML[8].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "TIHD0XML1", pP2Proc_Param.pD0MetaXML[9].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "TIHD0XML2", pP2Proc_Param.pD0MetaXML[10].c_str()));
		inputnode->append_node(doc.allocate_node(node_element, "TIHD0XML3", pP2Proc_Param.pD0MetaXML[11].c_str()));

		node->append_node(inputnode);
	}

	xml_node<>* outputnode = doc.allocate_node(node_element, "OutputFilelist");
	if (outputnode != 0)
	{
		outputnode->append_node(doc.allocate_node(node_element, "UVP0Data1", pP2Proc_Param.m_pP0Product[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP0Data2", pP2Proc_Param.m_pP0Product[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP0Data3", pP2Proc_Param.m_pP0Product[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP0Data1", pP2Proc_Param.m_pP0Product[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP0Data2", pP2Proc_Param.m_pP0Product[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP0Data3", pP2Proc_Param.m_pP0Product[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP0Data1", pP2Proc_Param.m_pP0Product[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP0Data2", pP2Proc_Param.m_pP0Product[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP0Data3", pP2Proc_Param.m_pP0Product[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP0Data1", pP2Proc_Param.m_pP0Product[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP0Data2", pP2Proc_Param.m_pP0Product[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP0Data3", pP2Proc_Param.m_pP0Product[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP0XML1", pP2Proc_Param.m_pP0ProductXML[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP0XML2", pP2Proc_Param.m_pP0ProductXML[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP0XML3", pP2Proc_Param.m_pP0ProductXML[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP0XML1", pP2Proc_Param.m_pP0ProductXML[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP0XML2", pP2Proc_Param.m_pP0ProductXML[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP0XML3", pP2Proc_Param.m_pP0ProductXML[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP0XML1", pP2Proc_Param.m_pP0ProductXML[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP0XML2", pP2Proc_Param.m_pP0ProductXML[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP0XML3", pP2Proc_Param.m_pP0ProductXML[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP0XML1", pP2Proc_Param.m_pP0ProductXML[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP0XML2", pP2Proc_Param.m_pP0ProductXML[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP0XML3", pP2Proc_Param.m_pP0ProductXML[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP0JPG1", pP2Proc_Param.m_pP0JPG[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP0JPG2", pP2Proc_Param.m_pP0JPG[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP0JPG3", pP2Proc_Param.m_pP0JPG[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP0JPG1", pP2Proc_Param.m_pP0JPG[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP0JPG2", pP2Proc_Param.m_pP0JPG[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP0JPG3", pP2Proc_Param.m_pP0JPG[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP0JPG1", pP2Proc_Param.m_pP0JPG[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP0JPG2", pP2Proc_Param.m_pP0JPG[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP0JPG3", pP2Proc_Param.m_pP0JPG[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP0JPG1", pP2Proc_Param.m_pP0JPG[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP0JPG2", pP2Proc_Param.m_pP0JPG[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP0JPG3", pP2Proc_Param.m_pP0JPG[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVEvent1", pP2Proc_Param.m_pEvent[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVEvent2", pP2Proc_Param.m_pEvent[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVEvent3", pP2Proc_Param.m_pEvent[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIREvent1", pP2Proc_Param.m_pEvent[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIREvent2", pP2Proc_Param.m_pEvent[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIREvent3", pP2Proc_Param.m_pEvent[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIREvent1", pP2Proc_Param.m_pEvent[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIREvent2", pP2Proc_Param.m_pEvent[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIREvent3", pP2Proc_Param.m_pEvent[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHEvent1", pP2Proc_Param.m_pEvent[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHEvent2", pP2Proc_Param.m_pEvent[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHEvent3", pP2Proc_Param.m_pEvent[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1AData1", pP2Proc_Param.m_pP1AData[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1AData2", pP2Proc_Param.m_pP1AData[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1AData3", pP2Proc_Param.m_pP1AData[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AData1", pP2Proc_Param.m_pP1AData[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AData2", pP2Proc_Param.m_pP1AData[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AData3", pP2Proc_Param.m_pP1AData[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AData1", pP2Proc_Param.m_pP1AData[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AData2", pP2Proc_Param.m_pP1AData[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AData3", pP2Proc_Param.m_pP1AData[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1AData1", pP2Proc_Param.m_pP1AData[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1AData2", pP2Proc_Param.m_pP1AData[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1AData3", pP2Proc_Param.m_pP1AData[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1AXML1", pP2Proc_Param.m_pP1AXML[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1AXML2", pP2Proc_Param.m_pP1AXML[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1AXML3", pP2Proc_Param.m_pP1AXML[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AXML1", pP2Proc_Param.m_pP1AXML[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AXML2", pP2Proc_Param.m_pP1AXML[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AXML3", pP2Proc_Param.m_pP1AXML[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AXML1", pP2Proc_Param.m_pP1AXML[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AXML2", pP2Proc_Param.m_pP1AXML[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AXML3", pP2Proc_Param.m_pP1AXML[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1AXML1", pP2Proc_Param.m_pP1AXML[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1AXML2", pP2Proc_Param.m_pP1AXML[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1AXML3", pP2Proc_Param.m_pP1AXML[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1AJPG1", pP2Proc_Param.m_pP1AJPG[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1AJPG2", pP2Proc_Param.m_pP1AJPG[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1AJPG3", pP2Proc_Param.m_pP1AJPG[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AJPG1", pP2Proc_Param.m_pP1AJPG[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AJPG2", pP2Proc_Param.m_pP1AJPG[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1AJPG3", pP2Proc_Param.m_pP1AJPG[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AJPG1", pP2Proc_Param.m_pP1AJPG[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AJPG2", pP2Proc_Param.m_pP1AJPG[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1AJPG3", pP2Proc_Param.m_pP1AJPG[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1AJPG1", pP2Proc_Param.m_pP1AJPG[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1AJPG2", pP2Proc_Param.m_pP1AJPG[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1AJPG3", pP2Proc_Param.m_pP1AJPG[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1BData1", pP2Proc_Param.m_pP1BData[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1BData2", pP2Proc_Param.m_pP1BData[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1BData3", pP2Proc_Param.m_pP1BData[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BData1", pP2Proc_Param.m_pP1BData[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BData2", pP2Proc_Param.m_pP1BData[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BData3", pP2Proc_Param.m_pP1BData[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BData1", pP2Proc_Param.m_pP1BData[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BData2", pP2Proc_Param.m_pP1BData[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BData3", pP2Proc_Param.m_pP1BData[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1BData1", pP2Proc_Param.m_pP1BData[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1BData2", pP2Proc_Param.m_pP1BData[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1BData3", pP2Proc_Param.m_pP1BData[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1BXML1", pP2Proc_Param.m_pP1BXML[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1BXML2", pP2Proc_Param.m_pP1BXML[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1BXML3", pP2Proc_Param.m_pP1BXML[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BXML1", pP2Proc_Param.m_pP1BXML[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BXML2", pP2Proc_Param.m_pP1BXML[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BXML3", pP2Proc_Param.m_pP1BXML[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BXML1", pP2Proc_Param.m_pP1BXML[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BXML2", pP2Proc_Param.m_pP1BXML[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BXML3", pP2Proc_Param.m_pP1BXML[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1BXML1", pP2Proc_Param.m_pP1BXML[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1BXML2", pP2Proc_Param.m_pP1BXML[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1BXML3", pP2Proc_Param.m_pP1BXML[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1BJPG1", pP2Proc_Param.m_pP1BJPG[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1BJPG2", pP2Proc_Param.m_pP1BJPG[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1BJPG3", pP2Proc_Param.m_pP1BJPG[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BJPG1", pP2Proc_Param.m_pP1BJPG[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BJPG2", pP2Proc_Param.m_pP1BJPG[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1BJPG3", pP2Proc_Param.m_pP1BJPG[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BJPG1", pP2Proc_Param.m_pP1BJPG[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BJPG2", pP2Proc_Param.m_pP1BJPG[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1BJPG3", pP2Proc_Param.m_pP1BJPG[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1BJPG1", pP2Proc_Param.m_pP1BJPG[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1BJPG2", pP2Proc_Param.m_pP1BJPG[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1BJPG3", pP2Proc_Param.m_pP1BJPG[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1CData1", pP2Proc_Param.m_pP1CData[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1CData2", pP2Proc_Param.m_pP1CData[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1CData3", pP2Proc_Param.m_pP1CData[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CData1", pP2Proc_Param.m_pP1CData[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CData2", pP2Proc_Param.m_pP1CData[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CData3", pP2Proc_Param.m_pP1CData[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CData1", pP2Proc_Param.m_pP1CData[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CData2", pP2Proc_Param.m_pP1CData[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CData3", pP2Proc_Param.m_pP1CData[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1CData1", pP2Proc_Param.m_pP1CData[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1CData2", pP2Proc_Param.m_pP1CData[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1CData3", pP2Proc_Param.m_pP1CData[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1CXML1", pP2Proc_Param.m_pP1CXML[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1CXML2", pP2Proc_Param.m_pP1CXML[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1CXML3", pP2Proc_Param.m_pP1CXML[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CXML1", pP2Proc_Param.m_pP1CXML[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CXML2", pP2Proc_Param.m_pP1CXML[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CXML3", pP2Proc_Param.m_pP1CXML[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CXML1", pP2Proc_Param.m_pP1CXML[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CXML2", pP2Proc_Param.m_pP1CXML[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CXML3", pP2Proc_Param.m_pP1CXML[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1CXML1", pP2Proc_Param.m_pP1CXML[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1CXML2", pP2Proc_Param.m_pP1CXML[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1CXML3", pP2Proc_Param.m_pP1CXML[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1CJPG1", pP2Proc_Param.m_pP1CJPG[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1CJPG2", pP2Proc_Param.m_pP1CJPG[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVP1CJPG3", pP2Proc_Param.m_pP1CJPG[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CJPG1", pP2Proc_Param.m_pP1CJPG[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CJPG2", pP2Proc_Param.m_pP1CJPG[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1CJPG3", pP2Proc_Param.m_pP1CJPG[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CJPG1", pP2Proc_Param.m_pP1CJPG[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CJPG2", pP2Proc_Param.m_pP1CJPG[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1CJPG3", pP2Proc_Param.m_pP1CJPG[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1CJPG1", pP2Proc_Param.m_pP1CJPG[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1CJPG2", pP2Proc_Param.m_pP1CJPG[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1CJPG3", pP2Proc_Param.m_pP1CJPG[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1DData", pP2Proc_Param.m_pP1DData[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1DData", pP2Proc_Param.m_pP1DData[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1DData", pP2Proc_Param.m_pP1DData[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1DData", pP2Proc_Param.m_pP1DData[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1DXML", pP2Proc_Param.m_pP1DXML[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1DXML", pP2Proc_Param.m_pP1DXML[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1DXML", pP2Proc_Param.m_pP1DXML[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1DXML", pP2Proc_Param.m_pP1DXML[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1DJPG", pP2Proc_Param.m_pP1DJPG[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1DJPG", pP2Proc_Param.m_pP1DJPG[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1DJPG", pP2Proc_Param.m_pP1DJPG[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1DJPG", pP2Proc_Param.m_pP1DJPG[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1EData", pP2Proc_Param.m_pP1EData[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1EData", pP2Proc_Param.m_pP1EData[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1EData", pP2Proc_Param.m_pP1EData[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1EData", pP2Proc_Param.m_pP1EData[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1EXML", pP2Proc_Param.m_pP1EXML[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1EXML", pP2Proc_Param.m_pP1EXML[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1EXML", pP2Proc_Param.m_pP1EXML[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1EXML", pP2Proc_Param.m_pP1EXML[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP1EJPG", pP2Proc_Param.m_pP1EJPG[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP1EJPG", pP2Proc_Param.m_pP1EJPG[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP1EJPG", pP2Proc_Param.m_pP1EJPG[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP1EJPG", pP2Proc_Param.m_pP1EJPG[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP2AData", pP2Proc_Param.m_pP2AData[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP2AData", pP2Proc_Param.m_pP2AData[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP2AData", pP2Proc_Param.m_pP2AData[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP2AData", pP2Proc_Param.m_pP2AData[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP2AXML", pP2Proc_Param.m_pP2AXML[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP2AXML", pP2Proc_Param.m_pP2AXML[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP2AXML", pP2Proc_Param.m_pP2AXML[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP2AXML", pP2Proc_Param.m_pP2AXML[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP2AJPG", pP2Proc_Param.m_pP2AJPG[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP2AJPG", pP2Proc_Param.m_pP2AJPG[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP2AJPG", pP2Proc_Param.m_pP2AJPG[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP2AJPG", pP2Proc_Param.m_pP2AJPG[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP2BData", pP2Proc_Param.m_pP2BData[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP2BData", pP2Proc_Param.m_pP2BData[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP2BData", pP2Proc_Param.m_pP2BData[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP2BData", pP2Proc_Param.m_pP2BData[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP2BXML", pP2Proc_Param.m_pP2BXML[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP2BXML", pP2Proc_Param.m_pP2BXML[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP2BXML", pP2Proc_Param.m_pP2BXML[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP2BXML", pP2Proc_Param.m_pP2BXML[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP2BJPG", pP2Proc_Param.m_pP2BJPG[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP2BJPG", pP2Proc_Param.m_pP2BJPG[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP2BJPG", pP2Proc_Param.m_pP2BJPG[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP2BJPG", pP2Proc_Param.m_pP2BJPG[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP2CData", pP2Proc_Param.m_pP2CData[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP2CData", pP2Proc_Param.m_pP2CData[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP2CData", pP2Proc_Param.m_pP2CData[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP2CData", pP2Proc_Param.m_pP2CData[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP2CXML", pP2Proc_Param.m_pP2CXML[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP2CXML", pP2Proc_Param.m_pP2CXML[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP2CXML", pP2Proc_Param.m_pP2CXML[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP2CXML", pP2Proc_Param.m_pP2CXML[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVP2CJPG", pP2Proc_Param.m_pP2CJPG[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRP2CJPG", pP2Proc_Param.m_pP2CJPG[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRP2CJPG", pP2Proc_Param.m_pP2CJPG[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHP2CJPG", pP2Proc_Param.m_pP2CJPG[3].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVPOS1", pP2Proc_Param.m_pPOSFile[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVPOS2", pP2Proc_Param.m_pPOSFile[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVPOS3", pP2Proc_Param.m_pPOSFile[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRPOS1", pP2Proc_Param.m_pPOSFile[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRPOS2", pP2Proc_Param.m_pPOSFile[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIRPOS3", pP2Proc_Param.m_pPOSFile[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRPOS1", pP2Proc_Param.m_pPOSFile[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRPOS2", pP2Proc_Param.m_pPOSFile[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIRPOS3", pP2Proc_Param.m_pPOSFile[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHPOS1", pP2Proc_Param.m_pPOSFile[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHPOS2", pP2Proc_Param.m_pPOSFile[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHPOS3", pP2Proc_Param.m_pPOSFile[11].c_str()));

		outputnode->append_node(doc.allocate_node(node_element, "UVEOF1", pP2Proc_Param.m_pEOFile[0].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVEOF2", pP2Proc_Param.m_pEOFile[1].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "UVEOF3", pP2Proc_Param.m_pEOFile[2].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIREOF1", pP2Proc_Param.m_pEOFile[3].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIREOF2", pP2Proc_Param.m_pEOFile[4].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "VNIREOF3", pP2Proc_Param.m_pEOFile[5].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIREOF1", pP2Proc_Param.m_pEOFile[6].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIREOF2", pP2Proc_Param.m_pEOFile[7].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "SWIREOF3", pP2Proc_Param.m_pEOFile[8].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHEOF1", pP2Proc_Param.m_pEOFile[9].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHEOF2", pP2Proc_Param.m_pEOFile[10].c_str()));
		outputnode->append_node(doc.allocate_node(node_element, "TIHEOF3", pP2Proc_Param.m_pEOFile[11].c_str()));

		node->append_node(outputnode);
	}

	xml_node<>* paranode = doc.allocate_node(node_element, "Parament");
	node->append_node(paranode);

	xml_node<>* recofnode = doc.allocate_node(node_element, "RadiateReleCof");
	recofnode->append_node(doc.allocate_node(node_element, "UVReleCof1", pP2Proc_Param.m_pReleCof[0].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "UVReleCof2", pP2Proc_Param.m_pReleCof[1].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "UVReleCof3", pP2Proc_Param.m_pReleCof[2].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "VNIRReleCof1", pP2Proc_Param.m_pReleCof[3].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "VNIRReleCof2", pP2Proc_Param.m_pReleCof[4].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "VNIRReleCof3", pP2Proc_Param.m_pReleCof[5].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "SWIRReleCof1", pP2Proc_Param.m_pReleCof[6].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "SWIRReleCof2", pP2Proc_Param.m_pReleCof[7].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "SWIRReleCof3", pP2Proc_Param.m_pReleCof[8].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "TIHReleCof1", pP2Proc_Param.m_pReleCof[9].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "TIHReleCof2", pP2Proc_Param.m_pReleCof[10].c_str()));
	recofnode->append_node(doc.allocate_node(node_element, "TIHReleCof3", pP2Proc_Param.m_pReleCof[11].c_str()));
	paranode->append_node(recofnode);

	xml_node<>* abcofnode = doc.allocate_node(node_element, "RadiateAbCof");
	abcofnode->append_node(doc.allocate_node(node_element, "UVAbCof1", pP2Proc_Param.m_pAbCof[0].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "UVAbCof2", pP2Proc_Param.m_pAbCof[1].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "UVAbCof3", pP2Proc_Param.m_pAbCof[2].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "VNIRAbCof1", pP2Proc_Param.m_pAbCof[3].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "VNIRAbCof2", pP2Proc_Param.m_pAbCof[4].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "VNIRAbCof3", pP2Proc_Param.m_pAbCof[5].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "SWIRAbCof1", pP2Proc_Param.m_pAbCof[6].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "SWIRAbCof2", pP2Proc_Param.m_pAbCof[7].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "SWIRAbCof3", pP2Proc_Param.m_pAbCof[8].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "TIHAbCof1", pP2Proc_Param.m_pAbCof[9].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "TIHAbCof2", pP2Proc_Param.m_pAbCof[10].c_str()));
	abcofnode->append_node(doc.allocate_node(node_element, "TIHAbCof3", pP2Proc_Param.m_pAbCof[11].c_str()));
	paranode->append_node(abcofnode);

	xml_node<>* splnode = doc.allocate_node(node_element, "WaveLength");
	splnode->append_node(doc.allocate_node(node_element, "Modtran", pP2Proc_Param.m_pModtran.c_str()));
	splnode->append_node(doc.allocate_node(node_element, "UVWaveLen1", pP2Proc_Param.m_pWaveLen[0].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "UVWaveLen2", pP2Proc_Param.m_pWaveLen[1].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "UVWaveLen3", pP2Proc_Param.m_pWaveLen[2].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "VNIRWaveLen1", pP2Proc_Param.m_pWaveLen[3].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "VNIRWaveLen2", pP2Proc_Param.m_pWaveLen[4].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "VNIRWaveLen3", pP2Proc_Param.m_pWaveLen[5].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "SWIRWaveLen1", pP2Proc_Param.m_pWaveLen[6].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "SWIRWaveLen2", pP2Proc_Param.m_pWaveLen[7].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "SWIRWaveLen3", pP2Proc_Param.m_pWaveLen[8].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "TIHWaveLen1", pP2Proc_Param.m_pWaveLen[9].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "TIHWaveLen2", pP2Proc_Param.m_pWaveLen[10].c_str()));
	splnode->append_node(doc.allocate_node(node_element, "TIHWaveLen3", pP2Proc_Param.m_pWaveLen[11].c_str()));
	paranode->append_node(splnode);

	xml_node<>* sbetnode = doc.allocate_node(node_element, "Sbet", pP2Proc_Param.m_pSBETFile.c_str());
	paranode->append_node(sbetnode);

	xml_node<>* demnode = doc.allocate_node(node_element, "DEM", pP2Proc_Param.m_pDEMFile.c_str());
	paranode->append_node(demnode);

	char cUVFOLap[200], cVNIRFOLap[200], cSWIRFOLap[200], cTIHFOLap[200];
	xml_node<>* folapnode = doc.allocate_node(node_element, "FieldOverLap");
	sprintf_s(cVNIRFOLap, sizeof(cVNIRFOLap), "%f,%f,%f,%f", pP2Proc_Param.m_fFOLapX[0], pP2Proc_Param.m_fFOLapX[1], pP2Proc_Param.m_fFOLapY[0], pP2Proc_Param.m_fFOLapY[1]);
	folapnode->append_node(doc.allocate_node(node_element, "VNIRFieldOverLap", cVNIRFOLap));
	sprintf_s(cSWIRFOLap, sizeof(cSWIRFOLap), "%f,%f,%f,%f", pP2Proc_Param.m_fFOLapX[2], pP2Proc_Param.m_fFOLapX[3], pP2Proc_Param.m_fFOLapY[2], pP2Proc_Param.m_fFOLapY[3]);
	folapnode->append_node(doc.allocate_node(node_element, "SWIRFieldOverLap", cSWIRFOLap));
	sprintf_s(cTIHFOLap, sizeof(cTIHFOLap), "%f,%f,%f,%f", pP2Proc_Param.m_fFOLapX[4], pP2Proc_Param.m_fFOLapX[5], pP2Proc_Param.m_fFOLapY[4], pP2Proc_Param.m_fFOLapY[5]);
	folapnode->append_node(doc.allocate_node(node_element, "TIHFieldOverLap", cTIHFOLap));
	sprintf_s(cUVFOLap, sizeof(cUVFOLap), "%f,%f,%f,%f", pP2Proc_Param.m_fFOLapX[6], pP2Proc_Param.m_fFOLapX[7], pP2Proc_Param.m_fFOLapY[6], pP2Proc_Param.m_fFOLapY[7]);
	folapnode->append_node(doc.allocate_node(node_element, "UVFieldOverLap", cUVFOLap));
	paranode->append_node(folapnode);

	char cUVSOLap[100], cSWIRSOLap[100], cTIHSOLap[100];
	xml_node<>* solapnode = doc.allocate_node(node_element, "SpectrumOverLap");
	sprintf_s(cUVSOLap, sizeof(cUVSOLap), "%f,%f", pP2Proc_Param.m_fSOLapX[0], pP2Proc_Param.m_fSOLapY[0]);
	solapnode->append_node(doc.allocate_node(node_element, "UVSpectrumOverLap", cUVSOLap));
	sprintf_s(cSWIRSOLap, sizeof(cSWIRSOLap), "%f,%f", pP2Proc_Param.m_fSOLapX[1], pP2Proc_Param.m_fSOLapY[1]);
	solapnode->append_node(doc.allocate_node(node_element, "SWIRSpectrumOverLap", cSWIRSOLap));
	sprintf_s(cTIHSOLap, sizeof(cTIHSOLap), "%f,%f", pP2Proc_Param.m_fSOLapX[2], pP2Proc_Param.m_fSOLapY[2]);
	solapnode->append_node(doc.allocate_node(node_element, "TIHSpectrumOverLap", cTIHSOLap));
	paranode->append_node(solapnode);

	char cFov[200], cIFov[200], cFocalLen[200];
	xml_node<>* gemnode = doc.allocate_node(node_element, "GeoCof");
	sprintf_s(cFov, sizeof(cFov), "%f,%f,%f,%f", pP2Proc_Param.m_fFov[0], pP2Proc_Param.m_fFov[1], pP2Proc_Param.m_fFov[2], pP2Proc_Param.m_fFov[3]);
	gemnode->append_node(doc.allocate_node(node_element, "FOV", cFov));
	sprintf_s(cIFov, sizeof(cIFov), "%f,%f,%f,%f,%f,%f,%f,%f", pP2Proc_Param.m_fIFov[0], pP2Proc_Param.m_fIFov[1], pP2Proc_Param.m_fIFov[2], pP2Proc_Param.m_fIFov[3], pP2Proc_Param.m_fIFov[4],pP2Proc_Param.m_fIFov[5], pP2Proc_Param.m_fIFov[6], pP2Proc_Param.m_fIFov[7]);
	gemnode->append_node(doc.allocate_node(node_element, "IFOV", cIFov));
	sprintf_s(cFocalLen, sizeof(cFocalLen), "%f,%f,%f,%f", pP2Proc_Param.m_fFocalLen[0], pP2Proc_Param.m_fFocalLen[1], pP2Proc_Param.m_fFocalLen[2], pP2Proc_Param.m_fFocalLen[3]);
	gemnode->append_node(doc.allocate_node(node_element, "FocalLen", cFocalLen));
	paranode->append_node(gemnode);

	char cBoresightMis[200], cGNSSOffset[200], cXYZOffset[200], cBand[50], cSamples[50];
	xml_node<>* misnode = doc.allocate_node(node_element, "POSMiss");
	sprintf_s(cBoresightMis, sizeof(cBoresightMis), "%lf,%lf,%lf", pP2Proc_Param.m_dBoresightMis[0], pP2Proc_Param.m_dBoresightMis[1], pP2Proc_Param.m_dBoresightMis[2]);
	misnode->append_node(doc.allocate_node(node_element, "BoresightMis", cBoresightMis));
	sprintf_s(cGNSSOffset, sizeof(cGNSSOffset), "%lf,%lf,%lf", pP2Proc_Param.m_dGNSSOffset[0], pP2Proc_Param.m_dGNSSOffset[1], pP2Proc_Param.m_dGNSSOffset[2]);
	misnode->append_node(doc.allocate_node(node_element, "GNSSOffset", cGNSSOffset));
	sprintf_s(cXYZOffset, sizeof(cXYZOffset), "%lf,%lf,%lf", pP2Proc_Param.m_dXYZOffset[0], pP2Proc_Param.m_dXYZOffset[1], pP2Proc_Param.m_dXYZOffset[2]);
	misnode->append_node(doc.allocate_node(node_element, "XYZOffset", cXYZOffset));
	paranode->append_node(misnode);

	xml_node<>* mergenode = doc.allocate_node(node_element, "Merge");
	sprintf_s(cBand, sizeof(cBand), "%d,%d,%d,%d", pP2Proc_Param.m_nBand[0], pP2Proc_Param.m_nBand[1], pP2Proc_Param.m_nBand[2], pP2Proc_Param.m_nBand[3]);
	mergenode->append_node(doc.allocate_node(node_element, "Band", cBand));
	sprintf_s(cSamples, sizeof(cSamples), "%d,%d,%d,%d", pP2Proc_Param.m_nSamples[0], pP2Proc_Param.m_nSamples[1], pP2Proc_Param.m_nSamples[2], pP2Proc_Param.m_nSamples[3]);
	mergenode->append_node(doc.allocate_node(node_element, "Samples", cSamples));
	paranode->append_node(mergenode);

	char cUVBand[50], cVNIRBand[50], cSWIRBand[50], cTIHBand[50];
	char cScale[50];
	xml_node<>* jpgnode = doc.allocate_node(node_element, "QuickView");
	if (pP2Proc_Param.m_bQuickView == 0)
	{
		jpgnode->append_node(doc.allocate_node(node_element, "WhetherQuickView", "0"));
		jpgnode->append_node(doc.allocate_node(node_element, "QuickViewScale", "0"));
		jpgnode->append_node(doc.allocate_node(node_element, "UVQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "VNIRQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "SWIRQuickBand", "-1"));
		jpgnode->append_node(doc.allocate_node(node_element, "TIHQuickBand", "-1"));
	}
	else
	{
		jpgnode->append_node(doc.allocate_node(node_element, "WhetherQuickView", "1"));
		sprintf_s(cScale, sizeof(cScale), "%f", pP2Proc_Param.m_fScale);
		jpgnode->append_node(doc.allocate_node(node_element, "QuickViewScale", cScale));
		sprintf_s(cUVBand, sizeof(cUVBand), "%d,%d,%d", pP2Proc_Param.m_nUVBand[0], pP2Proc_Param.m_nUVBand[1], pP2Proc_Param.m_nUVBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "UVQuickBand", cUVBand));
		sprintf_s(cVNIRBand, sizeof(cVNIRBand), "%d,%d,%d", pP2Proc_Param.m_nVNIRBand[0], pP2Proc_Param.m_nVNIRBand[1], pP2Proc_Param.m_nVNIRBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "VNIRQuickBand", cVNIRBand));
		sprintf_s(cSWIRBand, sizeof(cSWIRBand), "%d,%d,%d", pP2Proc_Param.m_nSWIRBand[0], pP2Proc_Param.m_nSWIRBand[1], pP2Proc_Param.m_nSWIRBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "SWIRQuickBand", cSWIRBand));
		sprintf_s(cTIHBand, sizeof(cTIHBand), "%d,%d,%d", pP2Proc_Param.m_nTIHBand[0], pP2Proc_Param.m_nTIHBand[1], pP2Proc_Param.m_nTIHBand[2]);
		jpgnode->append_node(doc.allocate_node(node_element, "TIHQuickBand", cTIHBand));
	}
	paranode->append_node(jpgnode);

	xml_node<>* repnode = doc.allocate_node(node_element, "ReportFile", pP2Proc_Param.m_pProFile.c_str());
	paranode->append_node(repnode);

	string str;
	rapidxml::print(std::back_inserter(str), doc, 0);
	ofstream out1(pCatalogOrderFile);
	out1 << doc;
	out1.close();

	return lError;
}