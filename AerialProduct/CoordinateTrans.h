#pragma once
//坐标系的转换，不知道是不是可以不用定义成类结构
//author：	吴蔚
//version:	2.0

#include "AerialCore.h"

class CoordinateTransBasic
{
public:
	//经纬度和高度到地心直角坐标系的转换（这个转换是建立在WGS84坐标系下，换一个坐标系需要自己扩展）
	virtual long BLHToXYZ(double dB,double dL,double dH,THREEDPOINT &XYZPnt);

	//计算成图坐标系下中心点的经纬度坐标
	virtual long XYZToBLHS(double dB,double dL,double dH,THREEDPOINT *pGroundPnt,int pixelNum);

	//计算地心直角坐标系的点对应的经纬度的值
	virtual long XYZToBLH(THREEDPOINT XYZPnt,double &dB,double &dL,double &dH);
};

