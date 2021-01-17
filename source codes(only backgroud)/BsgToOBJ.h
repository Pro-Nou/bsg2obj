#pragma once
//零件信息链表
#define _CRT_SECURE_NO_WARNINGS//禁用警告
#include "common/TCPconnect.h"

#include <iostream>
#include <stdio.h>
#include <io.h>
#include <windows.h>
#include <string>
#include <stdlib.h>
#include "DirectXMath.h"//directX数学工具，win10自带
#include "common/MathHelper.h"//常用数学工具
#include "common/Common.h"//杂项辅助工具
#include "common/PathFinder.h"//寻址
#include "common/UTF8String.h"//UTF8，string互转
#include "common/OBJTransform.h"//OBJ模型转换
#include "common/SurfaceGenerate.h"//蒙皮生成

using namespace DirectX;
using namespace std;

class BsgToOBJ
{
public:
	BsgToOBJ();
	~BsgToOBJ();

public:
	int blockCount = 74;//bsg目前存在的零件总数
	string SFALSE = "sfalse";//占位字符串
	string *blockList;//零件名表
	XMVECTOR *localPositionList;//零件本地位移表
	XMVECTOR *localRotationList;//零件本地旋转表(欧拉角)
	XMVECTOR *localScaleList;//零件本地缩放表

	int throwerror = 0;//错误使能
	string saveDataSName="";//存档名
	string versionNum = "v1.0";//版本号

	int totalVcount = 0;//总顶点数
	int totalVTcount = 0;//总纹理坐标数
	int totalVNcount = 0;//总法向量数
	int blocksCount = 1;//存档内的总零件数
	int totalBlocksCount = 0;
	int proStep = 15;

	string **language;//存语言表
	int langRow;
	int langLine;
	int langType = 1;
	int cmdCtrl = 0;

	float minimalCylinderLength = 0.0f;
	float surfaceStep = 0.0f;
	float surfaceStepAngle = 0.0f;
	float glassAlpha = 0.3f;

	TCPconnect BSGtcp;
	bool serverMode = false;

	Common::blockNode* blocks = new Common::blockNode;
	Common::blockNode* head = blocks;
	Common::pointNode* points = new Common::pointNode;
	Common::pointNode* phead = points;
	Common::edgeNode* edges = new Common::edgeNode;
	Common::edgeNode* ehead = edges;
	Common::surfNode* surfs = new Common::surfNode;
	Common::surfNode* shead = surfs;

	bool* denyList;//8删减，64磁铁，71-73新蒙皮(单独处理)，58摄像头，57空间锚，7钢筋，9皮筋，45绞盘

public:
	int saveLang();//读语言表
	int getLang(char * ID);//取语言字串
	float degReset(float deg);//钢筋角度复位
	int initList();//初始化零件表
	int saveSetting();//读取设置
	void checkSetting();//检查设置
	void showSetting();//显示设置
	string savePath(int SW);//0机械，1皮肤，3工坊，读取配置文件，获取路径
	string readMachine(string MPath);//读取机械存档
	void readBlock(string MData);//读取零件信息
	int mainStream(string cmdInput);//主流程，0失败，1成功

};