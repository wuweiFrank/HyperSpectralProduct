/************************************************************************/
/*                       高光谱影像修复操作                             */
/************************************************************************/
#include <vector>
using namespace std;

#define _MAX_MATCH_DIS_ 9999999

struct RepairEdgePnt
{
	int x;
	int y;
};

class HyperSpectralRepair
{
public:
	//进行修复
	virtual void HyperSpecRepair_RepairImage(const char* image,const char* imgMsk,const char* imageRepair) = 0;

	virtual void HyperSpecRepair_MakeRepairMaskRect(const char* image,const char* imgMsk,const char* imageRepair,RepairEdgePnt edgePnts[]);

protected:
	//获取影像带修复区域边界点
	virtual void HyperSpecRepair_GetEdges(const unsigned char* imageMask,const int xsize,const int ysize,vector<RepairEdgePnt> &edgePnts);

	//获取所有带修复的点
	virtual void HyperSpecRepair_GetRepair(const unsigned char* imageMask,const int xsize,const int ysize,vector<RepairEdgePnt> &edgePnts);

	//获取待修复点周围模板区域
	virtual void HyperSpecRepair_GetTemplate(const float* imageBuffer,const unsigned char* imageMask,const int &xsize,const int &ysize,
																	const int &templateSize,const int &xpos,const int &ypos,float* imgTemplate)
	{
		int leftpos =max(min(xsize-1,xpos-templateSize),0);
		int rightpos=max(min(xsize-1,xpos+templateSize),0);
		int uppos   =max(min(ysize-1,ypos-templateSize),0);
		int bottompos=max(min(ysize-1,ypos+templateSize),0);

		for (int i=leftpos,m=0;i<rightpos;++i,++m)
		{
			for(int j=uppos,n=0;j<bottompos;++j,++n)
			{
				if(imageMask[j*xsize+i]!=0)
					imgTemplate[n*templateSize+m]=imageBuffer[j*xsize+i];
			}
		}
	}

	//与指定点进行模板匹配
	float HyperSpecRepair_MatchTemplate(const float* imageBuffer,const unsigned char* imageMask,const int &xsize,const int &ysize,
															  const int &templateSize,const int &xpos,const int &ypos,const float* imgTemplate)
	{
		int leftpos =max(min(xsize-1,xpos-templateSize),0);
		int rightpos=max(min(xsize-1,xpos+templateSize),0);
		int uppos   =max(min(ysize-1,ypos-templateSize),0);
		int bottompos=max(min(ysize-1,ypos+templateSize),0);

		float matchconf = 0.0f;
		int totalNum =0 ;
		for (int i=leftpos,m=0;i<rightpos;++i,++m)
		{
			for(int j=uppos,n=0;j<bottompos;++j,++n)
			{
				if(imageMask[j*xsize+i]==0)
					return _MAX_MATCH_DIS_;
				if(imgTemplate[n*templateSize+m]!=0)
				{
					matchconf+=fabs(imgTemplate[n*templateSize+m]-imageBuffer[j*xsize+i]);
					totalNum++;
				}
			}
		}
		return matchconf/*/float(totalNum)*/;
	}
};


//偏微分方程的修复方法
class HyperSpectralRepairPDE : public HyperSpectralRepair
{
public:
	//进行BSCB修复
	virtual void HyperSpecRepair_RepairImage(const char* image,const char* imgMsk,const char* imageRepair);

	//修复的迭代过程
	void HyperSpecRepair_Iter(float* image,float* repairImage,int xsize,int ysize);

private:
	//信息扩散过程
	void HyperSpecRepair_Diffusion(float* image,float* imageDiffus,int xsize,int ysize,int xpos,int ypos);
	//进行修复的过程
	void HyperSpecRepair_RepairPnt(float* image,float* imageRepair,float delta,int xsize,int ysize,int xpos,int ypos);
	//二阶梯度差分
	float HyperSpecRepair_Laplace(float* image,int xsize,int ysize,int xpos,int ypos);
	//修复初始化
	void HyperSpecRepair_Init(float* image,float* imageInit,float* imgMsk,int xsize,int ysize);

};

enum RepairType
{
	OD_POINT,	//顺序逐点修复
	OD_BLOCK,	//顺序逐块修复
	CD_POINT,	//置信度逐点修复
	CD_BLOCK,	//置信度逐块修复
	EP_POINT    //边界优先逐点修复
};
//影像模板匹配的修复方法
class HyperSpectralRepairExemplar : public HyperSpectralRepair
{
public:
	//进行BSCB修复
	void HyperSpecRepair_SetMethod(RepairType repair_type=EP_POINT);
	virtual void HyperSpecRepair_RepairImage(const char* image,const char* imgMsk,const char* imageRepair);

	//测试用例
	void HyperSpecRepair_TestExample();

private:
	RepairType m_repair_type;
	
protected:
	//不同的修复方式接口
	void HyperSpecRepair_Repair_OD_POINT(const char* image,const char* imgMsk,const char* imageRepair);
	void HyperSpecRepair_Repair_OD_BLOCK(const char* image,const char* imgMsk,const char* imageRepair);
	void HyperSpecRepair_Repair_CD_POINT(const char* image,const char* imgMsk,const char* imageRepair);
	void HyperSpecRepair_Repair_CD_BLOCK(const char* image,const char* imgMsk,const char* imageRepair);
	void HyperSpecRepair_Repair_EP_POINT(const char* image,const char* imgMsk,const char* imageRepair);

private:
	//逐点和逐块修复
	void HyperSpecRepair_RepairPoint(float* image,unsigned char* imgMsk,int xsize,int ysize,int xpos,int ypos);
	void HyperSpecRepair_RepairBlock(float* image,unsigned char* imgMsk,int xsize,int ysize,int xpos,int ypos);

	//迭代进行修复
	void HyperSpecRepair_Iterator(float* image,unsigned char* imgMsk,int xsize,int ysize,RepairType repair_type=EP_POINT);
	//计算边界修复置信度
	RepairEdgePnt HyperSpecRepair_MaxConfPnt(float* image,unsigned char* imgMsk,int xsize,int ysize,vector<RepairEdgePnt>&edgePnts);

	//计算Np和Ip
	void HyperSpecRepair_IP(float* image,unsigned char* imgMsk,int xsize,int ysize,int xpos,int ypos,float* ip);
	void HyperSpecRepair_NP(float* image,unsigned char* imgMsk,int xsize,int ysize,int xpos,int ypos,float* np);

};

//影像光谱域的修复方法
class HyperSpectralRepairSpecral : public HyperSpectralRepair
{
public:
	void HyperSpecRepair_RepairImage(const char* image,const char* imgMsk,const char* imageRepair){}

	void HyperSpecRepair_TestExample();

private:
	//部分光谱范围的光谱匹配
	void HyperSpecRepair_PartSpectralMacth(float* image,unsigned char* imgMsk,int xsize,int ysize,int bands,RepairEdgePnt &pntRepair,vector<RepairEdgePnt> &pntMatches, float conf = 80);
	//修复单个像素
	void HyperSpecRepair_PartRepairPer(float* image,int xsize,int ysize,int bands,RepairEdgePnt &pntRepair,vector<RepairEdgePnt> &pntMatches);
	//光谱匹配
	void HyperSpecRepair_SpectralRepair(float* image,unsigned char* imgMsk,int xsize,int ysize,int bands);
	//光谱匹配的影像修复
	void HyperSpecRepair_SpecMatchRepair(const char* pathImage,const char* pathImgMsk,const char* pathDst);

	//根据丰度对端元光谱进行重建
	void HyperSpecRepair_SpectralAbundance(const char* pathImg,const char* pathEnd,const char* pathRed,int bands,int endNumbers,int begRepair,int endRepair);
	void HyperSpecRepair_AbundanceRestruct(float* image,unsigned char* imgMsk,float* endMember,const char* pathRed,int xsize,int ysize,int bands,int endNumber);
	
	//根据端元丰度对光谱进行
	void HyperSpecRepair_EndMemberRepair(const char* pathImg, const char* pathEnd,const char* pathMsk,const char* pathDst,int bands,int endNumber);
};