// OBJTransform.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS//禁用警告

#include "OBJTransform.h"

//int throwerror = 0;



OBJTransform::OBJTransform()
{
}

OBJTransform::~OBJTransform()
{
}

//读文件
string OBJTransform::readFile(string filename)
{
	//读文件
	string modelData;
	ifstream pfile;
	pfile.open(filename);
	if (!pfile)
	{
		throwerrorT = 1;
		return SFALSE;
	}
	while (!pfile.eof())
	{
		string subData;
		getline(pfile, subData);
		modelData += subData+'\n';
	}

	pfile.close();

	//cout << modelData << endl;
	vcount = countStr(modelData, "\nv ");
	vtcount = countStr(modelData, "\nvt ");
	vncount = countStr(modelData, "\nvn ");


	cout << "已读取模型" + filename << endl;
	cout << "顶点数：" << vcount << " 纹理坐标数：" << vtcount << " 法向量数：" << vncount << endl << endl;

	return modelData;

}

string OBJTransform::pickNumFromString(string tgtData)
{
	cout << tgtData << endl;
	string cmpStr = "-1234567890.";
	while (!cmpStr._Equal(tgtData.substr(0, 1)));
	{
		tgtData = tgtData.substr(1);
	}
	while (!cmpStr._Equal(tgtData.substr(tgtData.length() - 1)))
	{
		tgtData.pop_back();
	}
	cout << tgtData << endl;
	return tgtData;
}

//实体分割
OBJTransform::objectNode * OBJTransform::divideObjects(string modelData, objectNode * objects, objectNode * OHead, objectNode * OEnd)
{
	int countO = 0;

	modelData = '\n' + modelData;
	if (modelData.find("\no ") == modelData.npos)
	{
		objectsCount = 1;
		OEnd->objectData = '\n' + modelData;
	}
	else
	{
		while (modelData.find("\no ") != modelData.npos)
		{
			objectsCount++;
			OHead = objects;
			objects->objectData = modelData.substr(modelData.rfind("\no ") + 1);
			modelData = modelData.substr(0, modelData.rfind("\no ") + 1);
			objects->last = new objectNode;
			objects = objects->last;
			objects->next = OHead;
		}
	}
	OHead->last = NULL;
	OEnd->next = NULL;
	return OHead;
}

void OBJTransform::transformOBJ(objectNode * objects, XMMATRIX world, XMMATRIX Normalworld)
{
	string VData = objects->objectData;
	string VTData = objects->objectData;
	string VNData = objects->objectData;
	string FData = objects->objectData;

	//cout << objectsCount << endl;
	//cout << objects->objectData << endl;
	//点
	while (VData.find("\nv ")!=VData.npos)
	{
		float vx, vy, vz;
		VData = VData.substr(VData.find("\nv ") + 3);
		while (VData[0] == ' ')
			VData = VData.substr(1);
		vx = stof(VData.substr(0, VData.find(' ')));
		VData = VData.substr(VData.find(' ') + 1);
		while (VData[0] == ' ')
			VData = VData.substr(1);
		vy = stof(VData.substr(0, VData.find(' ')));
		VData = VData.substr(VData.find(' ') + 1);
		while (VData[0] == ' ')
			VData = VData.substr(1);
		vz = stof(VData.substr(0, VData.find('\n')));
		VData = VData.substr(VData.find('\n'));
		//cout << vx << ' ' << vy << ' ' << vz << endl;

		XMVECTOR orgV = XMVectorSet(vx, vy, vz, 1.0);
		XMVECTOR newV = XMVector3TransformCoord(orgV, world);

		TVData += "v " + to_string(newV.m128_f32[0]) + ' ' + to_string(newV.m128_f32[1]) + ' ' + to_string(newV.m128_f32[2]) + '\n';
	}
	
	//纹理
	while (VTData.find("\nvt ") != VTData.npos)
	{
		VTData = VTData.substr(VTData.find("\nvt ") + 1);
		TVTData += VTData.substr(0, VTData.find('\n') + 1);
		VTData = VTData.substr(VTData.find('\n'));
	}

	//法向
	while (VNData.find("\nvn ") != VNData.npos)
	{
		float vnx, vny, vnz;
		VNData = VNData.substr(VNData.find("\nvn ") + 4);
		while (VNData[0] == ' ')
			VNData = VNData.substr(1);
		vnx = stof(VNData.substr(0, VNData.find(' ')));
		VNData = VNData.substr(VNData.find(' ') + 1);
		while (VNData[0] == ' ')
			VNData = VNData.substr(1);
		vny = stof(VNData.substr(0, VNData.find(' ')));
		VNData = VNData.substr(VNData.find(' ') + 1);
		while (VNData[0] == ' ')
			VNData = VNData.substr(1);
		vnz = stof(VNData.substr(0, VNData.find('\n')));
		VNData = VNData.substr(VNData.find('\n'));
		//cout << vnx << ' ' << vny << ' ' << vnz << endl;

		XMVECTOR orgVN = XMVectorSet(vnx, vny, vnz, 0.0);
		orgVN = XMVector3Normalize(orgVN);

		XMVECTOR newVN = XMVector3TransformNormal(orgVN, Normalworld);
		newVN = XMVector3Normalize(newVN);
		//XMVECTOR newVN = orgVN;
		TVNData += "vn " + to_string(newVN.m128_f32[0]) + ' ' + to_string(newVN.m128_f32[1]) + ' ' + to_string(newVN.m128_f32[2]) + '\n';
	}
	
	//面
	while (FData.find("\nf ") != VNData.npos)
	{
		FData = FData.substr(FData.find("\nf ") + 3);
		string subF = FData.substr(0, FData.find('\n'));
		while (subF[subF.length() - 1] == ' ')
			subF.pop_back();
		subF += " end";
		TFData += "f ";
		//cout << subF << endl;
		while(!subF._Equal("end"))
		{
			int vNO, vtNO, vnNO;
			while (subF[0] == ' ')
				subF = subF.substr(1);
			vNO = stoi(subF.substr(0, subF.find('/')));
			vNO += totalVcount;
			subF = subF.substr(subF.find('/') + 1);
			vtNO = stoi(subF.substr(0, subF.find('/')));
			vtNO += totalVTcount;
			subF = subF.substr(subF.find('/') + 1);
			vnNO = stoi(subF.substr(0, subF.find(' ')));
			vnNO += totalVNcount;
			subF = subF.substr(subF.find(' ') + 1);
			//cout << vNO << '/' << vtNO << '/' << vnNO << ' ';
			TFData += to_string(vNO) + '/' + to_string(vtNO) + '/' + to_string(vnNO) + ' ';
			//TFData += to_string(vNO) + '/' + to_string(vtNO) + ' ';
		}
		TFData.pop_back();
		TFData += '\n';
		//cout << endl;
	}

	//光滑组
	if (objects->objectData.find("s off") != objects->objectData.npos)
		SmoothingGroup = "s off\n";
	else
		SmoothingGroup = "s 1\n";

}

//主流程，0错误，1完成，2空模型
int OBJTransform::mainStream(string filename, Common::blockNode* TBlock)
{
	//读模型
	if (TBlock->isFlipped)
		TBlock->localRotation.m128_f32[2] = -TBlock->localRotation.m128_f32[2];
	string modelData = readFile(filename);
	if (throwerrorT == 1)
	{
		//	throwerror = 1;
		return 0;
	}
	if (modelData.find("\nf ") == modelData.npos || vcount == 0)
	{
		return 2;
	}
	if (vtcount == 0)
	{
		return 3;
	}
	if (vncount == 0)
	{
		return 4;
	}

	//生成变换矩阵
	OBJTransform::objectNode* objects = new OBJTransform::objectNode;
	OBJTransform::objectNode* OEnd = objects;
	OBJTransform::objectNode* OHead = objects;

	XMFLOAT4X4 World = MathHelper::Identity4x4();
	/*
	Identity4x4:
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
		*/
	XMFLOAT4X4 NormalWorld = MathHelper::Identity4x4();
	XMMATRIX world;//vertices
	XMMATRIX Normalworld;//normal


	//局部X反向
	//world = XMLoadFloat4x4(&World);
	//XMStoreFloat4x4(&World, world*XMMatrixScaling(-1.0, 1.0, 1.0));
	//本地缩放local scaling
	world = XMLoadFloat4x4(&World);
	Normalworld = XMLoadFloat4x4(&NormalWorld);
	XMStoreFloat4x4(&World, world*XMMatrixScalingFromVector(TBlock->localScale));
	//while scaling normals, use reciprocals
	XMStoreFloat4x4(&NormalWorld, Normalworld*XMMatrixScaling(1.0f/TBlock->localScale.m128_f32[0], 1.0f / TBlock->localScale.m128_f32[1], 1.0f / TBlock->localScale.m128_f32[2]));
	//本地旋转local rotate
	world = XMLoadFloat4x4(&World);
	Normalworld = XMLoadFloat4x4(&NormalWorld);
	if (TBlock->id == 7 || TBlock->id == 9 || TBlock->id == 45)//Brace?
	{
		//I devided brace into A, B and Cylinder
		if (TBlock->blockName[TBlock->blockName.length() - 1] == 'A' || TBlock->blockName[TBlock->blockName.length() - 1] == 'B')//A|B?
		{
			//Inverse quaternion
			XMVECTOR invRotation = XMVectorSet(-TBlock->rotation.m128_f32[0], -TBlock->rotation.m128_f32[1], -TBlock->rotation.m128_f32[2], TBlock->rotation.m128_f32[3]);
			//RPY first, and then inv quaternion
			XMStoreFloat4x4(&World, world*XMMatrixRotationRollPitchYawFromVector(TBlock->localRotation)*XMMatrixRotationQuaternion(invRotation));
			XMStoreFloat4x4(&NormalWorld, Normalworld*XMMatrixRotationRollPitchYawFromVector(TBlock->localRotation)*XMMatrixRotationQuaternion(invRotation));
		}
		else if (TBlock->blockName.find("Cylinder") != TBlock->blockName.npos)//Cylinder?
		{
			XMStoreFloat4x4(&World, world*XMMatrixRotationZ(TBlock->localRotation.m128_f32[2])*XMMatrixRotationY(TBlock->localRotation.m128_f32[1]));
			XMStoreFloat4x4(&NormalWorld, Normalworld*XMMatrixRotationZ(TBlock->localRotation.m128_f32[2])*XMMatrixRotationY(TBlock->localRotation.m128_f32[1]));
		}
	}
	else//other blocks
	{
		XMStoreFloat4x4(&World, world*XMMatrixRotationRollPitchYawFromVector(TBlock->localRotation));
		XMStoreFloat4x4(&NormalWorld, Normalworld*XMMatrixRotationRollPitchYawFromVector(TBlock->localRotation));
	}
	//本地平移local position
	world = XMLoadFloat4x4(&World);
	Normalworld = XMLoadFloat4x4(&NormalWorld);
	XMStoreFloat4x4(&World, world*XMMatrixTranslationFromVector(TBlock->localPosition));
	XMStoreFloat4x4(&NormalWorld, Normalworld*XMMatrixTranslationFromVector(TBlock->localPosition));
	//全局缩放global scaling
	world = XMLoadFloat4x4(&World);
	Normalworld = XMLoadFloat4x4(&NormalWorld);
	XMStoreFloat4x4(&World, world*XMMatrixScalingFromVector(TBlock->scale));
	XMStoreFloat4x4(&NormalWorld, Normalworld*XMMatrixScaling(1.0f / TBlock->scale.m128_f32[0], 1.0f / TBlock->scale.m128_f32[1], 1.0f / TBlock->scale.m128_f32[2]));
	//全局旋转global rotate
	world = XMLoadFloat4x4(&World);
	Normalworld = XMLoadFloat4x4(&NormalWorld);
	XMStoreFloat4x4(&World, world*XMMatrixRotationQuaternion(TBlock->rotation));
	XMStoreFloat4x4(&NormalWorld, Normalworld*XMMatrixRotationQuaternion(TBlock->rotation));
	//全局平移global postion
	world = XMLoadFloat4x4(&World);
	Normalworld = XMLoadFloat4x4(&NormalWorld);
	XMStoreFloat4x4(&World, world*XMMatrixTranslationFromVector(TBlock->position));
	XMStoreFloat4x4(&NormalWorld, Normalworld*XMMatrixTranslationFromVector(TBlock->position));
	//全局X反向
	//world = XMLoadFloat4x4(&World);
	//XMStoreFloat4x4(&World, world*XMMatrixScaling(-1.0, 1.0, 1.0));
	//转matrix
	world = XMLoadFloat4x4(&World);
	Normalworld = XMLoadFloat4x4(&NormalWorld);

	OHead = divideObjects(modelData, objects, OHead, OEnd);

	//printOBJs(OHead);
	objects = OHead;
	while (objects)
	{
		//if (TBlock->id == 34)
			//cout << modelData << endl;
		transformOBJ(objects, world, Normalworld);
		objects = objects->next;
	}
	totalVcount += vcount;
	totalVTcount += vtcount;
	totalVNcount += vncount;
	cout << "已转换模型" << endl;
	cout << "当前总顶点数：" << totalVcount << " 总纹理坐标数：" << totalVTcount << " 总法向量数：" << totalVNcount << endl << endl;

	//cout << TVData << endl;
	//cout << TVTData << endl;
	//cout << TVNData << endl;
	//cout << TFData << endl;
	//printBlock(TBlock);
	return 1;
}

