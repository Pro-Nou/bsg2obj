// BsgToOBJ.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#define _CRT_SECURE_NO_WARNINGS//禁用警告

#include "BsgToOBJ.h"

using namespace DirectX;
using namespace std;

BsgToOBJ::BsgToOBJ()
{
	blockList = new string[blockCount];
	localPositionList = new XMVECTOR[blockCount];
	localRotationList = new XMVECTOR[blockCount];
	localScaleList = new XMVECTOR[blockCount];
	denyList = new bool[blockCount];
}

BsgToOBJ::~BsgToOBJ()
{
}

//钢筋角度复位
float BsgToOBJ::degReset(float deg)
{
	float degOut = deg;
	while (degOut >= 180.0f)
		degOut -= 180.0f;
	while (degOut < 0.0f)
		degOut += 180.0f;
	return degOut;
}

//初始化零件表
int BsgToOBJ::initList()
{
	ifstream pfile;
	pfile.open("block table.csv");
	for (int i=-1; i<blockCount; i++)
	{
		string SList;
		getline(pfile, SList);
		if (i == -1)
			continue;

		//存零件名
		SList = SList.substr(SList.find(',') + 1);
		blockList[i] = SList.substr(0, SList.find(','));

		//存零件本地位移
		float px, py, pz;
		SList = SList.substr(SList.find(',') + 1);
		px = stof(SList.substr(0, SList.find(',')));
		SList = SList.substr(SList.find(',') + 1);
		py = stof(SList.substr(0, SList.find(',')));
		SList = SList.substr(SList.find(',') + 1);
		pz = stof(SList.substr(0, SList.find(',')));
		localPositionList[i] = XMVectorSet(px, py, pz, 0.0);
		//cout << px << py << pz << endl;

		//存零件本地旋转（欧拉角）
		float rx, ry, rz;
		SList = SList.substr(SList.find(',') + 1);
		rx = stof(SList.substr(0, SList.find(',')));
		rx = (rx / 180.0f)*MathHelper::Pi;
		SList = SList.substr(SList.find(',') + 1);
		ry = stof(SList.substr(0, SList.find(',')));
		ry = (ry / 180.0f)*MathHelper::Pi;
		SList = SList.substr(SList.find(',') + 1);
		rz = stof(SList.substr(0, SList.find(',')));
		rz = (rz / 180.0f)*MathHelper::Pi;
		localRotationList[i] = XMVectorSet(rx, ry, rz, 0.0);
		//cout << rx << ry << rz << endl;

		//存零件本地缩放
		float sx, sy, sz;
		SList = SList.substr(SList.find(',') + 1);
		sx = stof(SList.substr(0, SList.find(',')));//局部X轴反向
		SList = SList.substr(SList.find(',') + 1);
		sy = stof(SList.substr(0, SList.find(',')));
		SList = SList.substr(SList.find(',') + 1);
		sz = stof(SList.substr(0, SList.find(',')));
		localScaleList[i] = XMVectorSet(sx, sy, sz, 0.0);
		//cout << sx << sy << sz << endl;

		//存禁用
		SList = SList.substr(SList.find(',') + 1);
		denyList[i] = stoi(SList);
	}
	pfile.close();

	return 0;

}
//读取设置
int BsgToOBJ::saveSetting()
{
	FILE* pfile = fopen("BSGsetting.txt", "r");
	if (pfile == 0)
	{
		cout << "路径文件不可为空" << endl;
		throwerror = 1;
		return 0;
	}

	//获取文件大小
	long lSize;
	fseek(pfile, 0, SEEK_END);
	lSize = ftell(pfile);
	rewind(pfile);//指针归零

	//文件存buffer
	char* buffer;
	buffer = (char*)malloc(sizeof(char)*lSize);
	fread(buffer, 1, lSize, pfile);
	fclose(pfile);

	string setting = UTF8String::UTF8_To_string(buffer);
	cout << "当前设置：" << endl;
	if (setting.find("<minimal-cylinder-length>") != setting.npos)
		minimalCylinderLength = stof(setting.substr(setting.find("<minimal-cylinder-length>") + 25, setting.find("</minimal-cylinder-length>") - setting.find("<minimal-cylinder-length>") - 25));
	else
		minimalCylinderLength = 0.1f;
	cout << "钢筋、皮筋、绞盘最小长度：" << minimalCylinderLength << endl;

	if (setting.find("<surface-step>") != setting.npos)
		surfaceStep = stof(setting.substr(setting.find("<surface-step>") + 14, setting.find("</surface-step>") - setting.find("<surface-step>") - 14));
	else
		surfaceStep = 0.25f;
	cout << "新蒙皮分辨率步长：" << surfaceStep << endl;

	if (setting.find("<surface-step-angle>") != setting.npos)
		surfaceStepAngle = stof(setting.substr(setting.find("<surface-step-angle>") + 20, setting.find("</surface-step-angle>") - setting.find("<surface-step-angle>") - 20));
	else
		surfaceStepAngle = 5.0f;
	cout << "新蒙皮分辨率步长(基于角度)：" << surfaceStepAngle << endl;

	if (setting.find("<glass-alpha>") != setting.npos)
		glassAlpha = stof(setting.substr(setting.find("<glass-alpha>") + 13, setting.find("</glass-alpha>") - setting.find("<glass-alpha>") - 13));
	else
		glassAlpha = 0.3f;
	cout << "玻璃透明度：" << glassAlpha << endl;
	cout << endl;
	return 1;
}
//0机械，1皮肤，3工坊，读取配置文件，获取路径
string BsgToOBJ::savePath(int SW)
{
	string start;
	string end;
	int strstart = 0;
	int strend = 0;
	switch (SW)
	{
	case 0:
		start = "<saveM>";
		end = "</saveM>";
		break;
	case 1:
		start = "<skin>";
		end = "</skin>";
		break;
	case 3:
		start = "<workShop>";
		end = "</workShop>";
		break;
	}
	//打开文件
	FILE* pfile = fopen("BSGPath.txt", "r");
	if (pfile == 0)
	{
		cout << "路径文件不可为空" << endl;
		throwerror = 1;
		return SFALSE;
	}

	//获取文件大小
	long lSize;
	fseek(pfile, 0, SEEK_END);
	lSize = ftell(pfile);
	rewind(pfile);//指针归零

	//文件存buffer
	char* buffer;
	buffer = (char*)malloc(sizeof(char)*lSize);
	fread(buffer, 1, lSize, pfile);
	fclose(pfile);

	//取路径
	string Path = buffer;
	strstart = (int)Path.find(start);
	strend = (int)Path.find(end);
	Path = Path.substr(strstart + start.length(), strend - strstart - start.length());

	return Path;
}

//读取机械存档
string BsgToOBJ::readMachine(string MPath)
{
	//获取存档名，拼接完整路径
	cout << "输入存档名（不加.bsg后缀）" << endl;
	//saveDataSName = "信浓级大型航空母舰";
	//saveDataSName = "loadtest";
	//saveDataSName = "天龙座";
	//saveDataSName = "bracetest"; 
	//saveDataSName = "ru-Su27A"; 
	//saveDataSName = "sftest3";
	//saveDataSName = "hr-05";
	getline(cin, saveDataSName);
	string orgSaveDataSName = MPath + saveDataSName + ".bsg";
	cout << "当前存档路径" << endl;
	cout << orgSaveDataSName << endl << endl;

	//String转const char*
	const char* saveDataName;
	saveDataName = orgSaveDataSName.c_str();

	//打开存档
	FILE* pfile = fopen(saveDataName, "r");
	if (pfile == 0)
	{
		cout << "存档不存在" << endl;
		throwerror = 1;
		return SFALSE;
	}
	else
		cout << "已找到存档" << endl;

	//获取文件大小
	long lSize;
	fseek(pfile, 0, SEEK_END);
	lSize = ftell(pfile);
	rewind(pfile);//指针归零

	//文件存buffer
	char* buffer;
	buffer = (char*)malloc(sizeof(char)*lSize);
	fread(buffer, 1, lSize, pfile);
	fclose(pfile);

	//buffer存string
	string saveMData = buffer;
	saveMData = saveMData.substr(0, saveMData.find("</Machine>") + strlen("</Machine>"));
	saveMData = UTF8String::UTF8_To_string(saveMData);

	return saveMData;
}

//读取零件信息
void BsgToOBJ::readBlock(string MData)
{
	while(1)
	{
		//是否遍历完
		if (MData.find("<Block id=") == MData.npos)
		{
			blocks->id=-1;
			break;
		}
		else
			MData = MData.substr(MData.find("<Block id="));

		//截取零件字段
		string blockData = MData.substr(MData.find("<Block id="), MData.find("</Block>") - MData.find("<Block id="));
		//cout << blockData << endl;

		//存id
		blocks->id = stoi(blockData.substr(blockData.find("id=\"") + 4, blockData.find("\" guid") - blockData.find("id=\"") - 4));
		//cout << blockData.substr(blockData.find("id=\"") + 4, blockData.find("\" guid")) << endl;
		//cout << blockData.find("id=\"") + 4 << ',' << blockData.find("\" guid") << endl;
		//跳过禁用零件和新蒙皮(building surface)
		if (blocks->id > 73 || denyList[blocks->id])
		{
			if (blocks->id == 71)
			{
				//cout<< blockData.substr(blockData.find("guid=\"") + 6, blockData.find("\">")- blockData.find("guid=\"") - 6) << endl;
				//存名称
				points->blockName = blockList[blocks->id];
				points->id = blocks->id;
				//存guid
				points->guid = blockData.substr(blockData.find("guid=\"") + 6, blockData.find("\">") - blockData.find("guid=\"") - 6);
				
				//存点位置
				blockData = blockData.substr(blockData.find("<Position"));
				float px, py, pz;
				px = stof(blockData.substr(blockData.find("x=\"") + 3, blockData.find("\" y") - blockData.find("x=\"") - 3));
				py = stof(blockData.substr(blockData.find("y=\"") + 3, blockData.find("\" z") - blockData.find("y=\"") - 3));
				pz = stof(blockData.substr(blockData.find("z=\"") + 3, blockData.find("\" />") - blockData.find("z=\"") - 3));
				points->position = XMVectorSet(px, py, pz, 0.0);

				//下一个
				points->next = new Common::pointNode;
				points = points->next;
			}
			else if (blocks->id == 72)
			{
				//存名称
				edges->blockName = blockList[blocks->id];
				edges->id = blocks->id;
				//存guid
				edges->guid = blockData.substr(blockData.find("guid=\"") + 6, blockData.find("\">") - blockData.find("guid=\"") - 6);

				//存边中点位置
				blockData = blockData.substr(blockData.find("<Position"));
				float px, py, pz;
				px = stof(blockData.substr(blockData.find("x=\"") + 3, blockData.find("\" y") - blockData.find("x=\"") - 3));
				py = stof(blockData.substr(blockData.find("y=\"") + 3, blockData.find("\" z") - blockData.find("y=\"") - 3));
				pz = stof(blockData.substr(blockData.find("z=\"") + 3, blockData.find("\" />") - blockData.find("z=\"") - 3));
				edges->position = XMVectorSet(px, py, pz, 0.0);

				//存起点guid
				blockData = blockData.substr(blockData.find("start\">"));
				edges->guidStart= blockData.substr(7, blockData.find("</String>") - 7);

				//存终点guid
				blockData = blockData.substr(blockData.find("end\">"));
				edges->guidEnd = blockData.substr(5, blockData.find("</String>") - 5);

				//下一个
				edges->next = new Common::edgeNode;
				edges = edges->next;
			}
			else if (blocks->id == 73)
			{
				//存名称
				surfs->blockName = blockList[blocks->id];
				surfs->id = blocks->id;
				//存皮肤
				if (blockData.find("<Skin") == blockData.npos)
				{
					surfs->skinName = "Template";
					surfs->skinID = "default";
				}
				else
				{
					blockData = blockData.substr(blockData.find("<Skin"));
					surfs->skinName = blockData.substr(blockData.find("name=\"") + 6, blockData.find("\" id") - blockData.find("name=\"") - 6);
					surfs->skinID = blockData.substr(blockData.find("id=\"") + 4, blockData.find("\" />") - blockData.find("id=\"") - 4);
				}
				surfs->skinName += "\\";
				surfs->skinID += "\\";

				//存材质
				blockData = blockData.substr(blockData.find("surfMat\">"));
				surfs->surfMat = stoi(blockData.substr(9, 1));
				if (surfs->surfMat == 2 && surfs->skinID.find("default") != surfs->skinID.npos)
				{
					surfs->skinName = "Glass\\";
					surfs->isglass = true;
				}

				//是否着色
				blockData = blockData.substr(blockData.find("painted\">"));
				if (blockData.substr(9, blockData.find("</Boolean>") - 9) == "True")
					surfs->painted = true;
				else
					surfs->painted = false;

				if (surfs->painted)
				{
					blockData = blockData.substr(blockData.find("<Color key=\"bmt-hue\">"));
					float hueR = stof(blockData.substr(blockData.find("<R>") + 3, blockData.find("<\R>") - blockData.find("<R>") - 3));
					float hueG = stof(blockData.substr(blockData.find("<G>") + 3, blockData.find("<\G>") - blockData.find("<G>") - 3));
					float hueB = stof(blockData.substr(blockData.find("<B>") + 3, blockData.find("<\B>") - blockData.find("<B>") - 3));
					surfs->RGBA = XMVectorSet(hueR, hueG, hueB, 0.0f);
					blockData = blockData.substr(blockData.find("sat\">"));
					surfs->sat = stof(blockData.substr(5, blockData.find("</Single>") - 5));
					blockData = blockData.substr(blockData.find("lum\">"));
					surfs->lum = stof(blockData.substr(5, blockData.find("</Single>") - 5));

					string matCode;
					matCode += to_string((int)(hueR * 255.0f)) + '-';
					matCode += to_string((int)(hueG * 255.0f)) + '-';
					matCode += to_string((int)(hueB * 255.0f)) + '|';
					matCode += to_string((int)(surfs->sat * 255.0f)) + '|';
					matCode += to_string((int)(surfs->lum * 255.0f));
					surfs->matCode = matCode;
				}


				//存厚度
				blockData = blockData.substr(blockData.find("bmt-thickness\">"));
				float thickness = stof(blockData.substr(blockData.find("bmt-thickness\">") + 15, blockData.find("</Single>") - blockData.find("bmt-thickness\">") - 15));
				if (thickness < 0.005f)
					thickness = (float)0.005f;
				else if (thickness > 0.2f)
					thickness = 0.2f;
				surfs->thickness = thickness;

				//存边
				blockData = blockData.substr(blockData.find("edges\">"));
				string edges = blockData.substr(blockData.find("edges\">") + 7, blockData.find("</String>") - blockData.find("edges\">") - 7);
				int edgesCount = (int)count(edges.begin(), edges.end(), '|') + 1;
				if (edgesCount > 3)
					surfs->isquad = true;
				for (int i = 0; i < edgesCount; i++)
				{
					if (i < edgesCount - 1)
					{
						surfs->guidEdges[i] = edges.substr(0, edges.find('|'));
						edges = edges.substr(edges.find('|') + 1);
					}
					else
						surfs->guidEdges[i] = edges;
				}

				//下一个
				surfs->next = new Common::surfNode;
				surfs = surfs->next;
			}
			MData = MData.substr(MData.find("</Block>"));
			continue;
		}

		blockData = blockData.substr(blockData.find("<Position"));

		//存本地转换
		blocks->localPosition = localPositionList[blocks->id];
		blocks->localRotation = localRotationList[blocks->id];
		blocks->localScale = localScaleList[blocks->id];

		//存平移
		float px, py, pz;
		px = stof(blockData.substr(blockData.find("x=\"") + 3, blockData.find("\" y") - blockData.find("x=\"") - 3));
		py = stof(blockData.substr(blockData.find("y=\"") + 3, blockData.find("\" z") - blockData.find("y=\"") - 3));
		pz = stof(blockData.substr(blockData.find("z=\"") + 3, blockData.find("\" />") - blockData.find("z=\"") - 3));
		blocks->position = XMVectorSet(px, py, pz, 0.0);
		blockData = blockData.substr(blockData.find("<Rotation"));
		//cout << px << endl << py << endl << pz << endl;

		//存旋转
		float rx, ry, rz, rw;
		rx = stof(blockData.substr(blockData.find("x=\"") + 3, blockData.find("\" y") - blockData.find("x=\"") - 3));
		ry = stof(blockData.substr(blockData.find("y=\"") + 3, blockData.find("\" z") - blockData.find("y=\"") - 3));
		rz = stof(blockData.substr(blockData.find("z=\"") + 3, blockData.find("\" w") - blockData.find("z=\"") - 3));
		rw = stof(blockData.substr(blockData.find("w=\"") + 3, blockData.find("\" />") - blockData.find("w=\"") - 3));
		blocks->rotation = XMVectorSet(rx, ry, rz, rw);
		blockData = blockData.substr(blockData.find("<Scale"));
		//cout << rx << endl << ry << endl << rz << endl << rw << endl;

		//存缩放
		float sx, sy, sz;
		sx = stof(blockData.substr(blockData.find("x=\"") + 3, blockData.find("\" y") - blockData.find("x=\"") - 3));
		sy = stof(blockData.substr(blockData.find("y=\"") + 3, blockData.find("\" z") - blockData.find("y=\"") - 3));
		sz = stof(blockData.substr(blockData.find("z=\"") + 3, blockData.find("\" />") - blockData.find("z=\"") - 3));
		blocks->scale = XMVectorSet(sx, sy, sz, 0.0);
		//cout << sx << endl << sy << endl << sz << endl;

		//存皮肤
		if (blockData.find("<Skin") == blockData.npos)
		{
			blocks->skinName = "Template";
			blocks->skinID = "default";
		}
		else
		{
			blockData = blockData.substr(blockData.find("<Skin"));
			blocks->skinName = blockData.substr(blockData.find("name=\"") + 6, blockData.find("\" id") - blockData.find("name=\"") - 6);
			blocks->skinID = blockData.substr(blockData.find("id=\"") + 4, blockData.find("\" />") - blockData.find("id=\"") - 4);
		}
		blocks->skinName += "\\";
		blocks->skinID += "\\";
		blocks->blockName = blockList[blocks->id];

		//钢筋特殊处理
		if (blocks->id == 7 || blocks->id == 9 || blocks->id == 45)
		{
			blockData = blockData.substr(blockData.find("Data"));

			float spx, spy, spz;
			string startPosition = blockData.substr(blockData.find("start-position"), blockData.find("</Vector3>"));
			spx = stof(startPosition.substr(startPosition.find("<X>") + 3, startPosition.find("</X>") - startPosition.find("<X>") - 3));
			spy = stof(startPosition.substr(startPosition.find("<Y>") + 3, startPosition.find("</Y>") - startPosition.find("<Y>") - 3));
			spz = stof(startPosition.substr(startPosition.find("<Z>") + 3, startPosition.find("</Z>") - startPosition.find("<Z>") - 3));
			blockData = blockData.substr(blockData.find("</Vector3>") + 11);
			//cout << spx << ',' << spy << ',' << spz << endl;

			float epx, epy, epz;
			string endPosition = blockData.substr(blockData.find("end-position"), blockData.find("</Vector3>"));
			epx = stof(endPosition.substr(endPosition.find("<X>") + 3, endPosition.find("</X>") - endPosition.find("<X>") - 3));
			epy = stof(endPosition.substr(endPosition.find("<Y>") + 3, endPosition.find("</Y>") - endPosition.find("<Y>") - 3));
			epz = stof(endPosition.substr(endPosition.find("<Z>") + 3, endPosition.find("</Z>") - endPosition.find("<Z>") - 3));
			blockData = blockData.substr(blockData.find("</Vector3>") + 11);
			//cout << epx << ',' << epy << ',' << epz << endl;

			float srx, sry, srz;
			string startRotation = blockData.substr(blockData.find("start-rotation"), blockData.find("</Vector3>"));
			srx = stof(startRotation.substr(startRotation.find("<X>") + 3, startRotation.find("</X>") - startRotation.find("<X>") - 3));
			sry = stof(startRotation.substr(startRotation.find("<Y>") + 3, startRotation.find("</Y>") - startRotation.find("<Y>") - 3));
			srz = stof(startRotation.substr(startRotation.find("<Z>") + 3, startRotation.find("</Z>") - startRotation.find("<Z>") - 3));
			blockData = blockData.substr(blockData.find("</Vector3>") + 11);
			//cout << srx << ',' << sry << ',' << srz << endl;

			float erx, ery, erz;
			string endRotation = blockData.substr(blockData.find("end-rotation"), blockData.find("</Vector3>"));
			erx = stof(endRotation.substr(endRotation.find("<X>") + 3, endRotation.find("</X>") - endRotation.find("<X>") - 3));
			ery = stof(endRotation.substr(endRotation.find("<Y>") + 3, endRotation.find("</Y>") - endRotation.find("<Y>") - 3));
			erz = stof(endRotation.substr(endRotation.find("<Z>") + 3, endRotation.find("</Z>") - endRotation.find("<Z>") - 3));
			blockData = blockData.substr(blockData.find("</Vector3>") + 11);
			//cout << erx << ',' << ery << ',' << erz << endl;

			//柱长
			float localx = blocks->localScale.m128_f32[0];
			float localy = blocks->localScale.m128_f32[1];
			float localz = blocks->localScale.m128_f32[2];
			float CylinderLength = sqrt(pow((spx - epx)*localx, 2) + pow((spy - epy)*localy, 2) + pow((spz - epz)*localz, 2));

			//角度转弧度
			srx = (srx / 180.0f)*MathHelper::Pi;
			sry = (sry / 180.0f)*MathHelper::Pi;
			srz = (srz / 180.0f)*MathHelper::Pi;
			erx = (erx / 180.0f)*MathHelper::Pi;
			ery = (ery / 180.0f)*MathHelper::Pi;
			erz = (erz / 180.0f)*MathHelper::Pi;


			//头
			blocks->blockName = blockList[blocks->id] + 'A';
			blocks->localPosition = XMVectorSet(spx, spy, spz, 0.0);
			blocks->localRotation = XMVectorSet(srx, sry, srz, 0.0);

			//点判断
			if (CylinderLength <= minimalCylinderLength)
			{
				switch (blocks->id)
				{
				case 7:
				{
					cout << "检测到点钢，跳过柱、尾" << endl;
					break;
				}
				case 9:
				{
					cout << "检测到点皮筋，跳过柱、尾" << endl;
					break;
				}
				case 45:
				{
					cout << "检测到点绞盘，跳过柱、尾" << endl;
					break;
				}
				}
			}
			else
			{
				//尾
				blocks->next = new Common::blockNode;
				blocks->next->id = blocks->id;
				blocks->next->blockName = blockList[blocks->id] + 'B';
				blocks->next->position = blocks->position;
				blocks->next->rotation = blocks->rotation;
				blocks->next->scale = blocks->scale;
				blocks->next->localPosition = XMVectorSet(epx, epy, epz, 0.0);
				blocks->next->localRotation = XMVectorSet(erx, ery, erz, 0.0);
				blocks->next->localScale = blocks->localScale;
				blocks->next->skinName = blocks->skinName;
				blocks->next->skinID = blocks->skinID;
				blocks = blocks->next;

				//中段
				blocks->next = new Common::blockNode;
				blocks->next->id = blocks->id;
				blocks->next->blockName = blockList[blocks->id] + "Cylinder";
				blocks->next->position = blocks->position;
				blocks->next->rotation = blocks->rotation;
				blocks->next->scale = blocks->scale;
				blocks->next->localPosition = XMVectorSet((spx + epx) / 2.0f, (spy + epy) / 2.0f, (spz + epz) / 2.0f, 0.0);

				XMFLOAT3 eulerA;
				eulerA.x = 0.0;
				eulerA.y = atan2(epx - spx, epz - spz) - MathHelper::Pi / 2.0f;
				eulerA.z = atan2(epy - spy, sqrt(pow(epx - spx, 2) + pow(epz - spz, 2))) + MathHelper::Pi / 2.0f;

				blocks->next->localRotation = XMVectorSet(eulerA.x, eulerA.y, eulerA.z, 0.0);
				blocks->next->localScale = XMVectorSet(-1.0, CylinderLength, 1.0, 0.0);
				blocks->next->skinName = blocks->skinName;
				blocks->next->skinID = blocks->skinID;
				blocks = blocks->next;
			}

		}
		//cout << blocks->skinName << endl << blocks->skinID << endl;

		//printBlock(blocks);

		//下一个
		MData = MData.substr(MData.find("</Block>"));
		blocks->next = new Common::blockNode;
		blocks = blocks->next;
	}
	Common::printLine(100);

}

int BsgToOBJ::mainStream()
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
		FOREGROUND_GREEN);
	cout << "BsgToOBJ " << versionNum << endl;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_INTENSITY |
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	Common::printLine(100);
	//零件存表
	initList();
	if (saveSetting() == 0)
	{
		cout << "缺失文件BSGsetting.txt" << endl;
		system("pause");
		return 0;
	};
	if (throwerror == 1)
	{
		system("pause");
		return 0;
	}

	//获取存档文件夹路径
	string MPath = savePath(0);
	if (throwerror == 1)
	{
		system("pause");
		return 0;
	}
	cout << "当前存档文件夹路径" << endl;
	cout << MPath << endl << endl;

	//获取皮肤文件夹路径
	string skinPath = savePath(1);
	if (throwerror == 1)
	{
		system("pause");
		return 0;
	}
	cout << "当前皮肤文件夹路径" << endl;
	cout << skinPath << endl << endl;

	//获取工坊文件夹路径
	string WSPath = savePath(3);
	if (throwerror == 1)
	{
		system("pause");
		return 0;
	}
	cout << "当前工坊文件夹路径" << endl;
	cout << WSPath << endl << endl;

	Common::printLine(100);
	//读取机械
	string MData=readMachine(MPath);
	if (throwerror == 1)
	{
		system("pause");
		return 0;
	}
	if (MData.find("<Block id=") == MData.npos)
	{
		cout << "存档为空" << endl;
		system("pause");
		return 0;
	}
	//cout << MData << endl << endl;

	SYSTEMTIME sys;
	string localTime = "";
	GetLocalTime(&sys);
	localTime += to_string(sys.wYear) + '-' + to_string(sys.wMonth) + '-' + to_string(sys.wDay) + ' ';
	localTime += to_string(sys.wHour) + "'" + to_string(sys.wMinute) + "'" + to_string(sys.wSecond);

	readBlock(MData);

	blocks = head;

	string fileHead = "#transformed by BsgToOBJ "+ versionNum +"\n";
	saveDataSName += '(' + localTime + ')';
	string saveDataNameUTF8 = UTF8String::string_To_UTF8(saveDataSName);
	string mtllib = "mtllib " + saveDataNameUTF8 + ".mtl\n";

	FILE* Mfile = fopen((saveDataSName + ".obj").c_str(), "w");
	fwrite(fileHead.c_str(), sizeof(const char)*fileHead.length(), 1, Mfile);
	fwrite(mtllib.c_str(), sizeof(const char)*mtllib.length(), 1, Mfile);

	FILE* MTLfile = fopen((saveDataSName + ".mtl").c_str(), "w");
	fwrite(fileHead.c_str(), sizeof(const char)*fileHead.length(), 1, MTLfile);

	string MTLs;

	string skinNotFound;
	//存模型&贴图
	while (blocks->id>-1)
	{

		//if(cmpDeny(blocks->id))
		string modelPath;
		Common::printBlock(blocks);

		//皮肤模型寻址
		struct _finddata_t objfileinfo;
		if (blocks->id == 7 || blocks->id == 9 || blocks->id == 45)
		{
			modelPath = "Blocks\\" + blockList[blocks->id] + '\\' + blocks->blockName + ".obj";
			cout << "读取模型：" << endl;
			cout << modelPath << endl << endl;

		}
		else if (_findfirst((skinPath + blocks->skinName + blockList[blocks->id] + "\\*.obj").c_str(), &objfileinfo) != -1)
		{
			modelPath = skinPath + blocks->skinName + blockList[blocks->id] + '\\' + objfileinfo.name;
			cout << "读取模型：" << endl;
			cout << modelPath << endl << endl;
		}
		else if (_findfirst((WSPath + blocks->skinID + blocks->skinName + blockList[blocks->id] + "\\*.obj").c_str(), &objfileinfo) != -1)
		{
			modelPath = WSPath + blocks->skinID + blocks->skinName + blockList[blocks->id] + '\\' + objfileinfo.name;
			cout << "读取模型：" << endl;
			cout << modelPath << endl << endl;
		}
		else
		{
			string WSPath1 = WSPath;
			string skinName1 = blocks->skinName;
			WSPath1.pop_back();
			skinName1.pop_back();
			string LocalToWSPath = PathFinder::getFiles(WSPath1, skinName1, 0, 1);
			if (_findfirst((LocalToWSPath+ "\\" + blockList[blocks->id] + "\\*.obj").c_str(), &objfileinfo) != -1)
			{
				modelPath = LocalToWSPath + "\\" + blockList[blocks->id] + '\\' + objfileinfo.name;
				cout << "读取模型：" << endl;
				cout << modelPath << endl << endl;
			}
			else
			{
				if (skinNotFound.find(blocks->skinName) == skinNotFound.npos)
				{
					skinNotFound += blocks->skinName + '\n';
				}
				_findfirst((skinPath + "Template\\" + blockList[blocks->id] + "\\*.obj").c_str(), &objfileinfo);
				modelPath = skinPath + "Template\\" + blockList[blocks->id] + '\\' + objfileinfo.name;
				cout << "皮肤不存在，读取原始模型：" << endl;
				cout << modelPath << endl << endl;
			}
		}

		//转换
		OBJTransform OBJT;
		OBJT.totalVcount = totalVcount;
		OBJT.totalVTcount = totalVTcount;
		OBJT.totalVNcount = totalVNcount;
		int SWTrans = OBJT.mainStream(modelPath, blocks);
		if (SWTrans == 0)
		{
			cout << endl;
			system("pause");
			blocks = blocks->next;
			continue;
			//return 0;
		}
		else if (SWTrans == 1)
			cout << "完成" << endl << endl;
		else if (SWTrans == 2)
		{
			cout << "模型为空，跳过" << endl << endl;
			blocks = blocks->next;
			continue;
		}
		totalVcount = OBJT.totalVcount;
		totalVTcount = OBJT.totalVTcount;
		totalVNcount = OBJT.totalVNcount;

		string oName = "\no " + to_string(blocksCount) + ":" + blocks->blockName + ":" + blocks->skinName;
		oName.pop_back();
		oName += '\n';
		string usemtl = "usemtl " + blockList[blocks->id] + ":" + blocks->skinName;
		usemtl.pop_back();
		usemtl += '\n';
		cout << "生成材质："<<usemtl;
		if (MTLs.find(usemtl) == MTLs.npos)
		{
			MTLs += usemtl;
			string newmtl = "\nnewmtl " + blockList[blocks->id] + ":" + blocks->skinName;
			newmtl.pop_back();
			newmtl += '\n';
			newmtl = UTF8String::string_To_UTF8(newmtl);
			fwrite(newmtl.c_str(), sizeof(const char)*newmtl.length(), 1, MTLfile);

			string texPath;
			struct _finddata_t pngfileinfo;
			if (_findfirst((skinPath + blocks->skinName + blockList[blocks->id] + "\\*.png").c_str(), &pngfileinfo) != -1)
			{
				texPath = skinPath + blocks->skinName + blockList[blocks->id] + '\\' + pngfileinfo.name;
				cout << "使用贴图：" << endl;
				cout << texPath << endl << endl;
			}
			else if (_findfirst((WSPath + blocks->skinID + blocks->skinName + blockList[blocks->id] + "\\*.png").c_str(), &pngfileinfo) != -1)
			{
				texPath = WSPath + blocks->skinID + blocks->skinName + blockList[blocks->id] + '\\' + pngfileinfo.name;
				cout << "使用贴图：" << endl;
				cout << texPath << endl << endl;
			}
			else
			{
				if (skinNotFound.find(blocks->skinName) == skinNotFound.npos)
				{
					skinNotFound += blocks->skinName + '\n';
				}
				blocks->skinName = "Template\\";
				_findfirst((skinPath + blocks->skinName + blockList[blocks->id] + "\\*.png").c_str(), &pngfileinfo);
				texPath = skinPath + blocks->skinName + blockList[blocks->id] + '\\' + pngfileinfo.name;
				cout << "贴图不存在，使用原始贴图：" << endl;
				cout << texPath << endl << endl;
			}
			texPath = "map_Kd " + texPath + '\n';
			texPath = UTF8String::string_To_UTF8(texPath);
			fwrite(texPath.c_str(), sizeof(const char)*texPath.length(), 1, MTLfile);
		}
		else
			cout << "材质已存在，跳过" << endl << endl;

		oName = UTF8String::string_To_UTF8(oName);
		fwrite(oName.c_str(), sizeof(const char)*oName.length(), 1, Mfile);
		fwrite(OBJT.TVData.c_str(), sizeof(const char)*OBJT.TVData.length(), 1, Mfile);
		fwrite(OBJT.TVTData.c_str(), sizeof(const char)*OBJT.TVTData.length(), 1, Mfile);
		fwrite(OBJT.TVNData.c_str(), sizeof(const char)*OBJT.TVNData.length(), 1, Mfile);
		usemtl = UTF8String::string_To_UTF8(usemtl);
		fwrite(usemtl.c_str(), sizeof(const char)*usemtl.length(), 1, Mfile);
		fwrite(OBJT.SmoothingGroup.c_str(), sizeof(const char)*OBJT.SmoothingGroup.length(), 1, Mfile);
		fwrite(OBJT.TFData.c_str(), sizeof(const char)*OBJT.TFData.length(), 1, Mfile);

		cout << blocks->blockName << "已完成转换" << endl;
		Common::printLine(100);

		blocks = blocks->next;
		blocksCount++;
	}

	//存新蒙皮(buildingSurface)
	points = phead;
	edges = ehead;
	surfs = shead;

	while (surfs->blockName != "NULL")
	{
		Common::printSurfs(surfs);
		int edgesCount;
		if (surfs->isquad)
			edgesCount = 4;
		else
			edgesCount = 3;

		//guid寻点
		XMVECTOR *positions = new XMVECTOR[edgesCount * 3];
		string *guid = new string[edgesCount * 3];
		edges = ehead;
		int times = 0;
		while (edges->blockName != "NULL"&&times < edgesCount)
		{
			for (int i = 0; i < edgesCount; i++)
			{
				if (surfs->guidEdges[i].find(edges->guid) != surfs->guidEdges[i].npos)
				{
					positions[i * 3 + 1] = edges->position;
					guid[i * 3 + 1] = edges->guid;
					points = phead;
					int etimes = 0;
					while (points->blockName != "NULL"&&etimes < 2)
					{
						if (edges->guidEnd.find(points->guid) != edges->guidEnd.npos)
						{
							positions[i * 3 + 2] = points->position;
							guid[i * 3 + 2] = points->guid;
							etimes++;
						}
						if (edges->guidStart.find(points->guid) != edges->guidStart.npos)
						{
							positions[i * 3] = points->position;
							guid[i * 3] = points->guid;
							etimes++;
						}
						points = points->next;
					}
					break;
				}
			}
			edges = edges->next;
		}

		//移除重复点位，连接边
		XMVECTOR bposition[8];
		bposition[0] = positions[0];
		bposition[1] = positions[1];
		bposition[2] = positions[2];
		guid[0] = "NULL";
		guid[1] = "NULL";
		int mark = 2;
		int count = 2;
		for (int i = 0; i < edgesCount - 1; i++)
		{
			for (int j = 0; j < edgesCount * 3; j++)
			{
				if (j == mark)
					continue;
				if (guid[mark].find(guid[j]) != guid[mark].npos)
				{
					if (j % 3 == 0)
					{
						bposition[count + 1] = positions[j + 1];
						bposition[count + 2] = positions[j + 2];
						guid[j] = "NULL";
						guid[j + 1] = "NULL";
						guid[mark] = "NULL";
						mark = j + 2;
						count = count + 2;
					}
					else
					{
						bposition[count + 1] = positions[j - 1];
						bposition[count + 2] = positions[j - 2];
						guid[j] = "NULL";
						guid[j - 1] = "NULL";
						guid[mark] = "NULL";
						mark = j - 2;
						count = count + 2;
					}
					break;
				}
			}
		}
		if (!surfs->isquad)
		{
			bposition[6] = bposition[0];
			bposition[7] = bposition[0];
		}

		//生成新蒙皮
		SurfaceGenerate surfGenerate(surfs->isquad, surfs->thickness, surfaceStep, surfaceStepAngle);
		surfGenerate.totalVcount = totalVcount;
		surfGenerate.totalVNcount = totalVNcount;
		surfGenerate.totalVTcount = totalVTcount;
		for (int i = 0; i < 8; i++)
		{
			surfGenerate.getPos(bposition[i], i);
			//printXMVECTOR(bposition[i], 0);
		}
		int SFTrans = surfGenerate.mainStream();
		if (SFTrans == 1)
		{
			cout << "完成" << endl << endl;
		}

		totalVcount = surfGenerate.totalVcount;
		totalVNcount = surfGenerate.totalVNcount;
		totalVTcount = surfGenerate.totalVTcount;
		string oName = "\no " + to_string(blocksCount) + ":" + surfs->blockName + ":" + surfs->skinName;
		oName.pop_back();
		oName += '\n';
		string usemtl = "usemtl " + blockList[surfs->id] + ":" + surfs->skinName;
		usemtl.pop_back();
		if (!surfs->painted)
			usemtl += "(unpainted)";
		else
			usemtl += '(' + surfs->matCode + ')';
		usemtl += '\n';
		cout << "生成材质：" << usemtl;

		if (MTLs.find(usemtl) == MTLs.npos)
		{
			MTLs += usemtl;
			string newmtl = "\nnewmtl " + blockList[surfs->id] + ":" + surfs->skinName;
			newmtl.pop_back();
			if (!surfs->painted)
				newmtl += "(unpainted)";
			else
				newmtl += '(' + surfs->matCode + ')';
			newmtl += '\n';
			newmtl = UTF8String::string_To_UTF8(newmtl);
			fwrite(newmtl.c_str(), sizeof(const char)*newmtl.length(), 1, MTLfile);

			string texPath;
			if (surfs->isglass)
			{
				texPath = "Blocks\\BuildSurface(Glass)\\GlassTex.png";
				cout << "使用贴图：" << endl;
				cout << texPath << endl << endl;
			}
			else
			{
				struct _finddata_t pngfileinfo;
				if (_findfirst((skinPath + surfs->skinName + blockList[surfs->id] + "\\*.png").c_str(), &pngfileinfo) != -1)
				{
					texPath = skinPath + surfs->skinName + blockList[surfs->id] + '\\' + pngfileinfo.name;
					cout << "使用贴图：" << endl;
					cout << texPath << endl << endl;
				}
				else if (_findfirst((WSPath + surfs->skinID + surfs->skinName + blockList[surfs->id] + "\\*.png").c_str(), &pngfileinfo) != -1)
				{
					texPath = WSPath + surfs->skinID + surfs->skinName + blockList[surfs->id] + '\\' + pngfileinfo.name;
					cout << "使用贴图：" << endl;
					cout << texPath << endl << endl;
				}
				else
				{
					if (skinNotFound.find(surfs->skinName) == skinNotFound.npos)
					{
						skinNotFound += surfs->skinName + '\n';
					}
					surfs->skinName = "Template\\";
					_findfirst((skinPath + surfs->skinName + blockList[surfs->id] + "\\*.png").c_str(), &pngfileinfo);
					texPath = skinPath + surfs->skinName + blockList[surfs->id] + '\\' + pngfileinfo.name;
					cout << "贴图不存在，使用原始贴图：" << endl;
					cout << texPath << endl << endl;
				}
			}
			if (!surfs->painted)
			{
				texPath = "map_Kd " + texPath + '\n';
				texPath = UTF8String::string_To_UTF8(texPath);
				if (surfs->isglass)
				{
					string alphad = "d ";
					alphad += to_string(glassAlpha) + '\n';
					string illum = "illum 9\n";
					fwrite(alphad.c_str(), sizeof(const char)*alphad.length(), 1, MTLfile);
					fwrite(illum.c_str(), sizeof(const char)*illum.length(), 1, MTLfile);

				}
				fwrite(texPath.c_str(), sizeof(const char)*texPath.length(), 1, MTLfile);
			}
			else
			{
				texPath = "map_Ks " + texPath + '\n';
				texPath = UTF8String::string_To_UTF8(texPath);
				float R = surfs->RGBA.m128_f32[0];
				float G = surfs->RGBA.m128_f32[1];
				float B = surfs->RGBA.m128_f32[2];
				float sat = surfs->sat;
				float lum = surfs->lum;
				lum = pow(lum, 1.0f / 2.2f);

				float avgRGB = (R + G + B) / 3.0f;
				R = (avgRGB + (R - avgRGB)*sat)*lum;
				G = (avgRGB + (G - avgRGB)*sat)*lum;
				B = (avgRGB + (B - avgRGB)*sat)*lum;

				string Kd = "Kd " + to_string(R) + ' ' + to_string(G) + ' ' + to_string(B) + '\n';
				fwrite(Kd.c_str(), sizeof(const char)*Kd.length(), 1, MTLfile);
				fwrite(texPath.c_str(), sizeof(const char)*texPath.length(), 1, MTLfile);
			}
		}
		else
			cout << "材质已存在，跳过" << endl << endl;

		oName = UTF8String::string_To_UTF8(oName);
		fwrite(oName.c_str(), sizeof(const char)*oName.length(), 1, Mfile);
		fwrite(surfGenerate.TVData.c_str(), sizeof(const char)*surfGenerate.TVData.length(), 1, Mfile);
		fwrite(surfGenerate.TVTData.c_str(), sizeof(const char)*surfGenerate.TVTData.length(), 1, Mfile);
		fwrite(surfGenerate.TVNData.c_str(), sizeof(const char)*surfGenerate.TVNData.length(), 1, Mfile);
		usemtl = UTF8String::string_To_UTF8(usemtl);
		fwrite(usemtl.c_str(), sizeof(const char)*usemtl.length(), 1, Mfile);
		fwrite(surfGenerate.SmoothingGroup.c_str(), sizeof(const char)*surfGenerate.SmoothingGroup.length(), 1, Mfile);
		fwrite(surfGenerate.TFData.c_str(), sizeof(const char)*surfGenerate.TFData.length(), 1, Mfile);

		cout << surfs->blockName << "已完成转换" << endl;
		Common::printLine(100);

		blocksCount++;
		surfs = surfs->next;
	}
	cout << saveDataSName << ".obj已完成转换" << endl;
	cout << "开始时间：" << localTime << endl;
	localTime.clear();
	GetLocalTime(&sys);
	localTime += to_string(sys.wYear) + '-' + to_string(sys.wMonth) + '-' + to_string(sys.wDay) + ' ';
	localTime += to_string(sys.wHour) + "'" + to_string(sys.wMinute) + "'" + to_string(sys.wSecond);
	cout << "结束时间：" << localTime << endl;
	cout << "总实体数：" << blocksCount - 1 << endl;
	cout << "总顶点数：" << totalVcount << endl;
	cout << "总法向量数：" << totalVNcount << endl;
	cout << "总纹理贴图顶点数：" << totalVTcount << endl;
	cout << "未找到的皮肤：" << endl;
	cout << skinNotFound << endl;
	fclose(Mfile);
	fclose(MTLfile);
	system("pause");

	return 1;

}
//使用例
int main()
{
	BsgToOBJ a;
	int b = a.mainStream();
	return b;
}


