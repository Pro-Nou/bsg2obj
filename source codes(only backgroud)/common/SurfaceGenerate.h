#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <stdio.h>
#include <string>
#include <stdlib.h>
#include "MathHelper.h"
#include "DirectXMath.h"
#include "Common.h"


using namespace DirectX;
using namespace std;

class SurfaceGenerate
{
public:
	SurfaceGenerate(bool isquad, float thickness, float surfStep, float surfStepAngle);
	~SurfaceGenerate();

	XMVECTOR positions[8];//新蒙皮基准点
	XMVECTOR bmap[3][3];//贝塞尔曲面基准点
	XMVECTOR **posmap;//上曲面点阵
	XMVECTOR **posmap2;//下曲面点阵
	XMVECTOR *bezierSpline;//贝塞尔曲线暂存
	XMVECTOR **fnmap;//面法向
	XMVECTOR **vnmap;//点法向
	XMVECTOR **uvmap;//uv图

	int U;//行下标
	int V;//列下标
	float surfStep;//分辨率步长
	float surfStepAngle;//分辨率步长(基于角度)
	float thickness;//厚度
	int Vcount = 0;
	int VTcount = 0;
	int VNcount = 0;
	int totalVcount = 0;
	int totalVTcount = 0;
	int totalVNcount = 0;
	string TVData;
	string TVTData;
	string TVNData;
	string TFData;
	string SmoothingGroup = "s 1\n";

	int edgeCount;//边数
	bool isquad;
	int getPos(XMVECTOR position, int i);//直接传数组会非常卡，不知道为什么
	void setmap();//bmap初始化
	void generateBezier(XMVECTOR p0, XMVECTOR p1, XMVECTOR p2, int n);//生成贝塞尔曲线
	void setpos();//生成点阵
	XMVECTOR generateNormal(XMVECTOR p0, XMVECTOR p1, XMVECTOR p2);//生成法向
	void setfn();//生成面法向
	void setvn();//生成点法向
	void setuv();//生成UV图
	void generateMesh();//生成新蒙皮数据

	void printmap();
	void printPosmap();
	void printPosmap2();
	void printfnmap();
	void printvnmap();

	int mainStream();
private:

};
