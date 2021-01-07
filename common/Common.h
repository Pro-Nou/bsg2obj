#pragma once
#define _CRT_SECURE_NO_WARNINGS//禁用警告
#include <iostream>
#include <stdio.h>
#include <io.h>
#include <windows.h>
#include <string>
#include <stdlib.h>
#include "MathHelper.h"
#include "DirectXMath.h"

using namespace DirectX;
using namespace std; 

class Common
{
public:
	struct blockNode
	{
		int id;
		string blockName;
		XMVECTOR position;
		XMVECTOR rotation;
		XMVECTOR scale;
		XMVECTOR localPosition;
		XMVECTOR localRotation;
		XMVECTOR localScale;
		string skinName;
		string skinID;
		blockNode *next;
	};
	struct pointNode
	{
		int id;
		string blockName = "NULL";
		string guid;
		XMVECTOR position;
		pointNode *next;
	};
	struct edgeNode
	{
		int id;
		string blockName = "NULL";
		string guid;
		string guidStart;//起点guid
		string guidEnd;//终点guid
		XMVECTOR position;
		edgeNode *next;
	};
	struct surfNode
	{
		int id;
		string blockName = "NULL";

		int surfMat = 0;//材质
		bool painted = false;//着色
		bool isquad = false;//是否四边
		bool isglass = false;//是否为玻璃

		XMVECTOR RGBA = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		float sat;//饱和度
		float lum;//明度

		string guidEdges[4];//边guid
		float thickness;//厚度
		string skinName;
		string skinID;
		string matCode;//材质码，区分同贴图不同RGBA、sat、lum的材质
		surfNode *next;
	};

	static void printLine(int n)
	{
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
			FOREGROUND_RED | FOREGROUND_GREEN);
		for (int i = 0; i < n; i++)
			cout << '-';
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
			FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
		cout << endl;
	}
	static void printXMVECTOR(XMVECTOR a, bool w)
	{
		cout << a.m128_f32[0] << ",";
		cout << a.m128_f32[1] << ",";
		cout << a.m128_f32[2];
		if (w)
			cout << "," << a.m128_f32[3] << endl;
		else
			cout << endl;
	}
	//输出零件信息
	static void printBlock(blockNode* block)
	{
		cout << "零件ID：" << block->id << endl;
		cout << "零件名：" << block->blockName << endl;
		cout << "平移坐标：";
		printXMVECTOR(block->position, 0);
		cout << "四元数：";
		printXMVECTOR(block->rotation, 1);
		cout << "缩放：";
		printXMVECTOR(block->scale, 0);
		cout << "本地平移坐标：";
		printXMVECTOR(block->localPosition, 0);
		cout << "本地旋转欧拉角：";
		printXMVECTOR(block->localRotation, 0);
		cout << "本地缩放：";
		printXMVECTOR(block->localScale, 0);
		cout << "皮肤名称：" + block->skinName << endl;
		cout << "工坊ID：" + block->skinID << endl << endl;
	}
	//输出点信息
	static void printPoints(pointNode* point)
	{
		cout << "零件名：" << point->blockName << endl;
		cout << "guid：" << point->guid << endl;
		cout << "点位坐标：";
		printXMVECTOR(point->position, 0);
		cout << endl;
	}
	//输出边信息
	static void printEdges(edgeNode* edge)
	{
		cout << "零件名：" << edge->blockName << endl;
		cout << "guid：" << edge->guid << endl;
		cout << "点位坐标：";
		printXMVECTOR(edge->position, 0);
		cout << "起点guid：" << edge->guidStart << endl;
		cout << "终点guid：" << edge->guidEnd << endl << endl;
	}
	static void printSurfs(surfNode* surf)
	{
		int times = 0;
		cout << "零件名：" << surf->blockName << endl;
		cout << "材质：" << surf->surfMat << endl;
		cout << "着色：" << surf->painted << endl;
		if (surf->painted)
		{
			cout << "RGBA：";
			printXMVECTOR(surf->RGBA, 1);
			cout << "饱和度：" << surf->sat << endl;
			cout << "明度：" << surf->lum << endl;
			cout << "材质码：" << surf->matCode << endl;
		}
		cout << "厚度：" << surf->thickness << endl;
		cout << "皮肤名称：" + surf->skinName << endl;
		cout << "工坊ID：" + surf->skinID << endl;
		cout << "边数：";
		if (surf->isquad)
		{
			times = 4;
			cout << 4 << endl;
		}
		else
		{
			times = 3;
			cout << 3 << endl;
		}
		/*for (int i = 0; i < times; i++)
		{
			cout << "边guid：" << surf->guidEdges[i] << endl;
		}*/
		cout << endl;
	}
	static float lengthXMVECTOR(XMVECTOR a)
	{
		float x = a.m128_f32[0];
		float y = a.m128_f32[1];
		float z = a.m128_f32[2];
		float l = sqrt(x * x + y * y + z * z);
		return l;
	}
	static float solveAngleXMVECTOR(XMVECTOR a, XMVECTOR b)
	{
		a = XMVector3Normalize(a);
		b = XMVector3Normalize(b);
		float ab = a.m128_f32[0] * b.m128_f32[0] + a.m128_f32[1] * b.m128_f32[1] + a.m128_f32[2] * b.m128_f32[2];
		float abAngle = acosf(ab);
		if (abAngle < 0.0f)
			abAngle = 0.0f;
		if (abAngle > MathHelper::Pi)
			abAngle = MathHelper::Pi;
		return abAngle;
	}

};