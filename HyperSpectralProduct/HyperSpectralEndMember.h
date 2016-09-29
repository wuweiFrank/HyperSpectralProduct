#pragma once 

//端元解混求解的结果都是丰度影像
//因此读取丰度的时候也是读取丰度影像
class HyperSpectralEndMember
{
public:
	//获取端元光谱（ENVI格式）
	bool HyperSpectralEnd_ImportEndmember(const char* pathEnd,int bands,int endnumbers,float* endmember);
	bool HyperSpectralEnd_ImportEndmember(const char* pathEnd,int bands,int endnumbers,double* endmember);

	//将端元光谱输出到文件中
	void HyperSpectralEnd_ExportEndmember(const char* path,float *enddata,int bands,int endnumbers);
	void HyperSpectralEnd_ExportEndmember(const char* path,double *enddata,int bands,int endnumbers);

	//最小二乘光谱解混
	void HyperSpectralEnd_UnmixLSE(const char* pathEnd,char* pathImg,const char* pathRed,int endnumbers);
	//使用特定波段进行最小二乘解混
	void HyperSpectralEnd_UnmixLSE_SetBands(const char* pathEnd,const char* pathImg,const char* pathRed,int endnumbers,int* bandIdx,int bandIdxNum);

	//计算解混残差
	void HyperSpectralEnd_Residual(const char* pathimg,const char* pathend,const char* pathred,const char* pathres,int endnumbers);

	//求解出来的丰度进行求和(理论上丰度和应该等于1)
	void HyperSpectralEnd_ReduTotal(const char* pathRed,const char* pathTotal);

	//光谱响应函数
	//通过高光谱端元获取多光谱端元
	//判断是输入的多光谱波段的每个波段光谱范围还是中心波长
	//如果是中心波长，则通过比较波长相近的高光谱波段计算光谱响应
	//如果是光谱范围则选择范围内的高光谱波段计算光谱响应
	/*
		char* pathHy:输入的高光谱端元
		char* pathM	:输出的多光谱端元
		float* hysp int bandsHy:高光谱波段的中心波长&波段数
		float* msp  int bandm:多光谱波段的中心波长或波长范围&波段数
		int endnumber:	端元数目
		bool range:判断输入的是光谱范围还是中心波长
	*/
	void HyperSpectralEnd_Response(const char* pathHy,const char* pathM,float* hysp,int bandsHy,float* msp,int bandm,int endnumber,bool range);
	void HyperSpectralEnd_Response_Image(const char* pathHyImage,const char* pathMImage,float* hysp,int bandsHy,float* msp,int bandm,int endnumber,bool range);

	//空间点扩散函数，采用高斯函数进行降采样
	void HyperSpectralEnd_Spatial_Spread(const char* pathImg,const char* pathSample);
};