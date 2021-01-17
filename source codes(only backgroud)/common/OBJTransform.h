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

class OBJTransform
{
public:

	OBJTransform();
	~OBJTransform();

	//实体指针
	struct objectNode
	{
		objectNode *last;
		objectNode *next;
		string objectData;
	};

	//打印实体
	void printOBJs(objectNode* OHead)
	{
		objectNode* objectsOutput = OHead;
		while (objectsOutput)
		{
			cout << objectsOutput->objectData << endl;
			objectsOutput = objectsOutput->next;
		}
	}

	//字符串计数
	int countStr(string Data, string tgtStr)
	{
		int times = 0;
		string subData = Data;
		while (subData.find(tgtStr)!=subData.npos)
		{
			times++;
			subData = subData.substr(subData.find(tgtStr) + tgtStr.length());
		}
		return times;
	}

	string SFALSE = "sfalse";//占位字符串
	string emptyModel = "empty";//空模型
	int throwerrorT = 0;//错误使能

	string TVData;
	string TVTData;
	string TVNData;
	string SmoothingGroup;
	string TFData;
	int vcount = 0;
	int vtcount = 0;
	int vncount = 0;
	int totalVcount = 0;
	int totalVTcount = 0;
	int totalVNcount = 0;
	int objectsCount = 0;


	string readFile(string filename);
	string pickNumFromString(string tgtData);
	void transformOBJ(objectNode* objects, XMMATRIX world, XMMATRIX Normalworld);
	objectNode* divideObjects(string modelData, objectNode* objects, objectNode* OHead, objectNode* OEnd);

	int mainStream(string filename, Common::blockNode* TBlock);


private:

};


