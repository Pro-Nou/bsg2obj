#define _CRT_SECURE_NO_WARNINGS//禁用警告

#include "SurfaceGenerate.h"

SurfaceGenerate::SurfaceGenerate(bool isquad, float thickness, float surfStep, float surfStepAngle)
{
	this->isquad = isquad;
	if (isquad)
		this->edgeCount = 4;
	else
		this->edgeCount = 3;
	this->thickness = thickness;
	this->surfStep = surfStep;
	this->surfStepAngle = surfStepAngle;
}

SurfaceGenerate::~SurfaceGenerate()
{
}

int SurfaceGenerate::getPos(XMVECTOR position, int i)
{
	this->positions[i] = position;
	return 0;
}

void SurfaceGenerate::setmap()
{
	this->bmap[0][0] = this->positions[0];
	this->bmap[2][0] = this->positions[2];
	this->bmap[2][2] = this->positions[4];
	this->bmap[0][2] = this->positions[6];
	XMVECTOR midpoint1 = (this->positions[0] + this->positions[2]) / 2;
	XMVECTOR midpoint2 = (this->positions[2] + this->positions[4]) / 2;
	XMVECTOR midpoint3 = (this->positions[4] + this->positions[6]) / 2;
	XMVECTOR midpoint4 = (this->positions[6] + this->positions[0]) / 2;
	this->bmap[1][0] = midpoint1 + 2 * (this->positions[1] - midpoint1);
	this->bmap[2][1] = midpoint2 + 2 * (this->positions[3] - midpoint2);
	this->bmap[1][2] = midpoint3 + 2 * (this->positions[5] - midpoint3);
	this->bmap[0][1] = midpoint4 + 2 * (this->positions[7] - midpoint4);
	XMVECTOR midmidpoint = (this->bmap[1][0] + this->bmap[2][1] + this->bmap[1][2] + this->bmap[0][1]) / 4;
	XMVECTOR midedgepoint = (this->positions[0] + this->positions[2] + this->positions[4] + this->positions[6]) / 4;
	this->bmap[1][1] = midedgepoint + 2 * (midmidpoint - midedgepoint);

	XMVECTOR p01 = this->positions[0] - this->positions[1];
	XMVECTOR p21 = this->positions[2] - this->positions[1];
	XMVECTOR p23 = this->positions[2] - this->positions[3];
	XMVECTOR p43 = this->positions[4] - this->positions[3];
	XMVECTOR p45 = this->positions[4] - this->positions[5];
	XMVECTOR p65 = this->positions[6] - this->positions[5];
	XMVECTOR p67 = this->positions[6] - this->positions[7];
	XMVECTOR p07 = this->positions[0] - this->positions[7];
	float edgeLength1 = Common::lengthXMVECTOR(p01) + Common::lengthXMVECTOR(p21);
	float edgeLength2 = Common::lengthXMVECTOR(p23) + Common::lengthXMVECTOR(p43);
	float edgeLength3 = Common::lengthXMVECTOR(p45) + Common::lengthXMVECTOR(p65);
	float edgeLength4 = Common::lengthXMVECTOR(p67) + Common::lengthXMVECTOR(p07);
	float edgeAngle1 = MathHelper::Pi - Common::solveAngleXMVECTOR(p01, p21);
	float edgeAngle2 = MathHelper::Pi - Common::solveAngleXMVECTOR(p23, p43);
	float edgeAngle3 = MathHelper::Pi - Common::solveAngleXMVECTOR(p45, p65);
	float edgeAngle4 = MathHelper::Pi - Common::solveAngleXMVECTOR(p67, p07);
	float maxLengthU = max(edgeLength2, edgeLength4);
	float maxLengthV = max(edgeLength1, edgeLength3);
	float maxAngleU = max(edgeAngle2, edgeAngle4);
	float maxAngleV = max(edgeAngle1, edgeAngle3);
	this->U = max((int)(maxLengthU / this->surfStep), (int)(maxAngleU / ((this->surfStepAngle / 180.0f)*MathHelper::Pi))) + 1;
	this->V = max((int)(maxLengthV / this->surfStep), (int)(maxAngleV / ((this->surfStepAngle / 180.0f)*MathHelper::Pi))) + 1;
	if (this->U < 3)
		this->U = 3;
	if (this->V < 3)
		this->V = 3;
}

void SurfaceGenerate::generateBezier(XMVECTOR p0, XMVECTOR p1, XMVECTOR p2, int t)
{
	this->bezierSpline = new XMVECTOR[t];
	for (int i = 0; i < t; i++)
	{
		XMVECTOR k1 = p0 + (p1 - p0)*((float)i / (float)(t - 1));
		XMVECTOR k2 = p1 + (p2 - p1)*((float)i / (float)(t - 1));
		this->bezierSpline[i] = k1 + (k2 - k1)*((float)i / (float)(t - 1));
	}
}

void SurfaceGenerate::setpos()
{
	this->posmap = new XMVECTOR*[V];
	for (int i = 0; i < V; i++)
		this->posmap[i] = new XMVECTOR[U];
	for (int i = 0; i < 3; i++)
	{
		generateBezier(this->bmap[i][0], this->bmap[i][1], this->bmap[i][2], U);
		for (int j = 0; j < U; j++)
			this->posmap[i][j] = this->bezierSpline[j];
		delete[]this->bezierSpline;
	}
	for (int i = 0; i < U; i++)
	{
		generateBezier(this->posmap[0][i], this->posmap[1][i], this->posmap[2][i], V);
		for (int j = 0; j < V; j++)
			this->posmap[j][i] = this->bezierSpline[j];
		delete[]this->bezierSpline;
	}
}

XMVECTOR SurfaceGenerate::generateNormal(XMVECTOR p0, XMVECTOR p1, XMVECTOR p2)
{
	XMVECTOR n1 = p1 - p0;
	XMVECTOR n2 = p2 - p1;
	XMVECTOR vn = XMVector3Cross(n1, n2);
	vn = XMVector3Normalize(vn);
	return vn;
}

void SurfaceGenerate::setfn()
{
	this->fnmap = new XMVECTOR*[V - 1];
	for (int i = 0; i < V - 1; i++)
		this->fnmap[i] = new XMVECTOR[U - 1];
	for (int i = 0; i < V - 1; i++)
		for (int j = 0; j < U - 1; j++)
			this->fnmap[i][j] = generateNormal(this->posmap[i][j], this->posmap[i + 1][j], this->posmap[i + 1][j + 1]);
}

void SurfaceGenerate::setvn()
{
	this->vnmap = new XMVECTOR*[V];
	for (int i = 0; i < V; i++)
		this->vnmap[i] = new XMVECTOR[U];

	//处理边角
	if (this->isquad)
	{
		this->vnmap[0][0] = this->fnmap[0][0];
		this->vnmap[0][U - 1] = this->fnmap[0][U - 2];
	}
	else
	{
		XMVECTOR totaln=XMVectorSet(0.0f,0.0f,0.0f,0.0f);
		for (int i = 0; i < U - 1; i++)
			totaln += fnmap[0][i];
		this->vnmap[0][0] = XMVector3Normalize(totaln / (float)(U - 1));
		for (int i = 1; i < U; i++)
			this->vnmap[0][i] = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	}
	this->vnmap[V - 1][0] = this->fnmap[V - 2][0];
	this->vnmap[V - 1][U - 1] = this->fnmap[V - 2][U - 2];
	
	//处理边界
	for (int i = 1; i < U - 1; i++)
	{
		if(this->isquad)
			this->vnmap[0][i] = XMVector3Normalize((this->fnmap[0][i - 1] + this->fnmap[0][i]) / 2.0f);
		this->vnmap[V - 1][i] = XMVector3Normalize((this->fnmap[V - 2][i - 1] + this->fnmap[V - 2][i]) / 2.0f);
	}
	for (int i = 1; i < V - 1; i++)
	{
		this->vnmap[i][0] = XMVector3Normalize((this->fnmap[i - 1][0] + this->fnmap[i][0]) / 2.0f);
		this->vnmap[i][U - 1] = XMVector3Normalize((this->fnmap[i - 1][U - 2] + this->fnmap[i][U - 2]) / 2.0f);
	}

	//处理内部
	for (int i = 1; i < V - 1; i++)
		for (int j = 1; j < U - 1; j++)
			this->vnmap[i][j] = XMVector3Normalize((this->fnmap[i - 1][j - 1] + this->fnmap[i - 1][j] + this->fnmap[i][j - 1] + this->fnmap[i][j]) / 4.0f);
		
}

void SurfaceGenerate::setuv()
{
	this->uvmap = new XMVECTOR*[V];
	for (int i = 0; i < V; i++)
		this->uvmap[i] = new XMVECTOR[U];

	for (int i = 0; i < V; i++)
		for (int j = 0; j < U; j++)
			this->uvmap[i][j] = XMVectorSet(0.1f + 0.8*((float)i / (float)(V - 1)), 0.1f + 0.8*((float)j / (float)(U - 1)), 0.0f, 0.0f);
	if (!this->isquad)
		for (int i = 0; i < U - 1; i++)
			this->uvmap[0][i] = (this->uvmap[0][i] + this->uvmap[0][i + 1]) / 2.0f;
}

void SurfaceGenerate::generateMesh()
{
	this->TVData.clear();
	this->TVTData.clear();
	this->TVNData.clear();
	this->TFData.clear();
	this->posmap2 = new XMVECTOR*[V];
	for (int i = 0; i < V; i++)
		this->posmap2[i] = new XMVECTOR[U];

	//上下面
	for (int i = 0; i < V; i++)
	{
		for (int j = 0; j < U; j++)
		{
			this->posmap2[i][j] = this->posmap[i][j] - (this->thickness)*this->vnmap[i][j];
			this->posmap[i][j] = this->posmap[i][j] + (this->thickness)*this->vnmap[i][j];

			if (i == 0 && !this->isquad)
				break;
		}
	}

	//存点
	for (int i = 0; i < V; i++)
		for (int j = 0; j < U; j++)
		{
			this->TVData += "v ";
			this->TVData += to_string(this->posmap[i][j].m128_f32[0]) + " ";
			this->TVData += to_string(this->posmap[i][j].m128_f32[1]) + " ";
			this->TVData += to_string(this->posmap[i][j].m128_f32[2]) + "\n";
			if (i == 0 && !this->isquad)
				break;
		}
	for (int i =0; i < V; i++)
		for (int j = 0; j < U; j++)
		{
			this->TVData += "v ";
			this->TVData += to_string(this->posmap2[i][j].m128_f32[0]) + " ";
			this->TVData += to_string(this->posmap2[i][j].m128_f32[1]) + " ";
			this->TVData += to_string(this->posmap2[i][j].m128_f32[2]) + "\n";
			if (i == 0 && !this->isquad)
				break;
		}

	//存uv
	for (int i = 0; i < V; i++)
		for (int j = 0; j < U; j++)
		{
			if (i == 0 && j == U - 1 && !this->isquad)
				break;
			this->TVTData += "vt ";
			this->TVTData += to_string(this->uvmap[i][j].m128_f32[0]) + " ";
			this->TVTData += to_string(this->uvmap[i][j].m128_f32[1]) + "\n";
		}


	//存法向
	for (int i = 0; i < V; i++)
		for (int j = 0; j < U; j++)
		{
			this->TVNData += "vn ";
			this->TVNData += to_string(this->vnmap[i][j].m128_f32[0]) + " ";
			this->TVNData += to_string(this->vnmap[i][j].m128_f32[1]) + " ";
			this->TVNData += to_string(this->vnmap[i][j].m128_f32[2]) + "\n";
			if (i == 0 && !this->isquad)
				break;
		}
	for (int i = 0; i < V; i++)
		for (int j = 0; j < U; j++)
		{
			//this->vnmap[i][j] = -this->vnmap[i][j];
			this->TVNData += "vn ";
			this->TVNData += to_string(this->vnmap[i][j].m128_f32[0]) + " ";
			this->TVNData += to_string(this->vnmap[i][j].m128_f32[1]) + " ";
			this->TVNData += to_string(this->vnmap[i][j].m128_f32[2]) + "\n";
			if (i == 0 && !this->isquad)
				break;
		}

	//存上下面
	for (int t = 0; t < 2; t++)
	{
		//第一行
		if (!this->isquad)
		{
			for (int i = 0; i < U - 1; i++)
			{
				this->TFData += "f ";
				this->TFData += to_string(totalVcount + 1) + '/';
				this->TFData += to_string(totalVTcount + i + 1) + '/';
				this->TFData += to_string(totalVNcount + 1);

				this->TFData += ' ';
				this->TFData += to_string(totalVcount + i + 2) + '/';
				this->TFData += to_string(totalVTcount + i + U) + '/';
				this->TFData += to_string(totalVNcount + i + 2);

				this->TFData += ' ';
				this->TFData += to_string(totalVcount + i + 3) + '/';
				this->TFData += to_string(totalVTcount + i + U + 1) + '/';
				this->TFData += to_string(totalVNcount + i + 3) + '\n';
			}
			for (int i = 1; i < V - 1; i++)
			{
				for (int j = 0; j < U - 1; j++)
				{
					this->TFData += "f ";
					this->TFData += to_string(totalVcount + (i - 1) * U + j + 2) + '/';
					this->TFData += to_string(totalVTcount + i * U + j) + '/';
					this->TFData += to_string(totalVNcount + (i - 1) * U + j + 2);

					this->TFData += ' ';
					this->TFData += to_string(totalVcount + i * U + j + 2) + '/';
					this->TFData += to_string(totalVTcount + (i + 1) * U + j) + '/';
					this->TFData += to_string(totalVNcount + i * U + j + 2);

					this->TFData += ' ';
					this->TFData += to_string(totalVcount + i * U + j + 3) + '/';
					this->TFData += to_string(totalVTcount + (i + 1) * U + j + 1) + '/';
					this->TFData += to_string(totalVNcount + i * U + j + 3);

					this->TFData += ' ';
					this->TFData += to_string(totalVcount + (i - 1) * U + j + 3) + '/';
					this->TFData += to_string(totalVTcount + i * U + j + 1) + '/';
					this->TFData += to_string(totalVNcount + (i - 1) * U + j + 3) + '\n';
				}
			}
			totalVcount += U * (V - 1) + 1;
			totalVNcount += U * (V - 1) + 1;
			Vcount += U * (V - 1) + 1;
			VNcount += U * (V - 1) + 1;
		}
		else
		{
			for (int i = 0; i < V - 1; i++)
			{
				for (int j = 0; j < U - 1; j++)
				{
					this->TFData += "f ";
					this->TFData += to_string(totalVcount + i * U + j + 1) + '/';
					this->TFData += to_string(totalVTcount + i * U + j + 1) + '/';
					this->TFData += to_string(totalVNcount + i * U + j + 1);

					this->TFData += ' ';
					this->TFData += to_string(totalVcount + (i + 1) * U + j + 1) + '/';
					this->TFData += to_string(totalVTcount + (i + 1) * U + j + 1) + '/';
					this->TFData += to_string(totalVNcount + (i + 1) * U + j + 1);
					
					this->TFData += ' ';
					this->TFData += to_string(totalVcount + (i + 1) * U + j + 2) + '/';
					this->TFData += to_string(totalVTcount + (i + 1) * U + j + 2) + '/';
					this->TFData += to_string(totalVNcount + (i + 1) * U + j + 2);
					
					this->TFData += ' ';
					this->TFData += to_string(totalVcount + i * U + j + 2) + '/';
					this->TFData += to_string(totalVTcount + i * U + j + 2) + '/';
					this->TFData += to_string(totalVNcount + i * U + j + 2) + '\n';
				}
			}
			totalVcount += U * V;
			totalVNcount += U * V;
			Vcount += U * V;
			VNcount += U * V;
		}
	}
	totalVTcount += U * V;
	VTcount += U * V;
	if (!this->isquad)
		totalVTcount--;	

	//存边1面
	for (int i = 0; i < V; i++)
	{
		this->TVData += "v ";
		this->TVData += to_string(this->posmap[i][0].m128_f32[0]) + " ";
		this->TVData += to_string(this->posmap[i][0].m128_f32[1]) + " ";
		this->TVData += to_string(this->posmap[i][0].m128_f32[2]) + "\n";
		this->TVData += "v ";
		this->TVData += to_string(this->posmap2[i][0].m128_f32[0]) + " ";
		this->TVData += to_string(this->posmap2[i][0].m128_f32[1]) + " ";
		this->TVData += to_string(this->posmap2[i][0].m128_f32[2]) + "\n";
		this->TVTData += "vt ";
		this->TVTData += to_string(0.1f + 0.8f*((float)i / (float)(V - 1))) + " ";
		this->TVTData += to_string(0.01f) + "\n";
		this->TVTData += "vt ";
		this->TVTData += to_string(0.1f + 0.8f*((float)i / (float)(V - 1))) + " ";
		this->TVTData += to_string(0.1f) + "\n";

	}
	XMVECTOR *edgeN = new XMVECTOR[V];
	for (int i = 0; i < V - 1; i++)
		edgeN[i] = generateNormal(this->posmap[i][0], this->posmap[i + 1][0], this->posmap2[i + 1][0]);
	edgeN[V - 1] = edgeN[V - 2];
	for (int i = 1; i < V - 1; i++)
		edgeN[i] = (edgeN[i] + edgeN[i - 1]) / 2.0f;
	for (int i = 0; i < V; i++)
	{
		this->TVNData += "vn ";
		this->TVNData += to_string(edgeN[i].m128_f32[0]) + " ";
		this->TVNData += to_string(edgeN[i].m128_f32[1]) + " ";
		this->TVNData += to_string(edgeN[i].m128_f32[2]) + "\n";
	}
	delete[]edgeN;
	for (int i = 0; i < V - 1; i++)
	{
		this->TFData += "f ";
		this->TFData += to_string(totalVcount + i * 2 + 1) + '/';
		this->TFData += to_string(totalVTcount + i * 2 + 1) + '/';
		this->TFData += to_string(totalVNcount + i + 1);

		this->TFData += " ";
		this->TFData += to_string(totalVcount + i * 2 + 3) + '/';
		this->TFData += to_string(totalVTcount + i * 2 + 3) + '/';
		this->TFData += to_string(totalVNcount + i + 2);

		this->TFData += " ";
		this->TFData += to_string(totalVcount + i * 2 + 4) + '/';
		this->TFData += to_string(totalVTcount + i * 2 + 4) + '/';
		this->TFData += to_string(totalVNcount + i + 2);

		this->TFData += " ";
		this->TFData += to_string(totalVcount + i * 2 + 2) + '/';
		this->TFData += to_string(totalVTcount + i * 2 + 2) + '/';
		this->TFData += to_string(totalVNcount + i + 1) + '\n';
	}
	totalVcount += 2 * V;
	totalVTcount += 2 * V;
	totalVNcount += V;
	Vcount += 2 * V;
	VTcount += 2 * V;
	VNcount += V;

	//存边2面
	for (int i = 0; i < U; i++)
	{
		this->TVData += "v ";
		this->TVData += to_string(this->posmap[V - 1][i].m128_f32[0]) + " ";
		this->TVData += to_string(this->posmap[V - 1][i].m128_f32[1]) + " ";
		this->TVData += to_string(this->posmap[V - 1][i].m128_f32[2]) + "\n";
		this->TVData += "v ";
		this->TVData += to_string(this->posmap2[V - 1][i].m128_f32[0]) + " ";
		this->TVData += to_string(this->posmap2[V - 1][i].m128_f32[1]) + " ";
		this->TVData += to_string(this->posmap2[V - 1][i].m128_f32[2]) + "\n";
		this->TVTData += "vt ";
		this->TVTData += to_string(0.9f) + " ";
		this->TVTData += to_string(0.1f + 0.8f*((float)i / (float)(U - 1))) + "\n";
		this->TVTData += "vt ";
		this->TVTData += to_string(0.99f) + " ";
		this->TVTData += to_string(0.1f + 0.8f*((float)i / (float)(U - 1))) + "\n";

	}
	edgeN = new XMVECTOR[U];
	for (int i = 0; i < U - 1; i++)
		edgeN[i] = generateNormal(this->posmap[V - 1][i], this->posmap[V - 1][i + 1], this->posmap2[V - 1][i + 1]);
	edgeN[U - 1] = edgeN[U - 2];
	for (int i = 1; i < U - 1; i++)
		edgeN[i] = (edgeN[i] + edgeN[i - 1]) / 2.0f;
	for (int i = 0; i < U; i++)
	{
		this->TVNData += "vn ";
		this->TVNData += to_string(edgeN[i].m128_f32[0]) + " ";
		this->TVNData += to_string(edgeN[i].m128_f32[1]) + " ";
		this->TVNData += to_string(edgeN[i].m128_f32[2]) + "\n";
	}
	delete[]edgeN;
	for (int i = 0; i < U - 1; i++)
	{
		this->TFData += "f ";
		this->TFData += to_string(totalVcount + i * 2 + 1) + '/';
		this->TFData += to_string(totalVTcount + i * 2 + 1) + '/';
		this->TFData += to_string(totalVNcount + i + 1);

		this->TFData += " ";
		this->TFData += to_string(totalVcount + i * 2 + 3) + '/';
		this->TFData += to_string(totalVTcount + i * 2 + 3) + '/';
		this->TFData += to_string(totalVNcount + i + 2);

		this->TFData += " ";
		this->TFData += to_string(totalVcount + i * 2 + 4) + '/';
		this->TFData += to_string(totalVTcount + i * 2 + 4) + '/';
		this->TFData += to_string(totalVNcount + i + 2);

		this->TFData += " ";
		this->TFData += to_string(totalVcount + i * 2 + 2) + '/';
		this->TFData += to_string(totalVTcount + i * 2 + 2) + '/';
		this->TFData += to_string(totalVNcount + i + 1) + '\n';
	}
	totalVcount += 2 * U;
	totalVTcount += 2 * U;
	totalVNcount += U;
	Vcount += 2 * U;
	VTcount += 2 * U;
	VNcount += U;

	//存边3面（三角）
	if (!this->isquad)
	{
		this->TVData += "v ";
		this->TVData += to_string(this->posmap[0][0].m128_f32[0]) + " ";
		this->TVData += to_string(this->posmap[0][0].m128_f32[1]) + " ";
		this->TVData += to_string(this->posmap[0][0].m128_f32[2]) + "\n";
		this->TVData += "v ";
		this->TVData += to_string(this->posmap2[0][0].m128_f32[0]) + " ";
		this->TVData += to_string(this->posmap2[0][0].m128_f32[1]) + " ";
		this->TVData += to_string(this->posmap2[0][0].m128_f32[2]) + "\n";
		this->TVTData += "vt ";
		this->TVTData += to_string(0.1f) + " ";
		this->TVTData += to_string(0.9f) + "\n";
		this->TVTData += "vt ";
		this->TVTData += to_string(0.1f) + " ";
		this->TVTData += to_string(0.99f) + "\n";

		for (int i = 1; i < V; i++)
		{
			this->TVData += "v ";
			this->TVData += to_string(this->posmap[i][U-1].m128_f32[0]) + " ";
			this->TVData += to_string(this->posmap[i][U-1].m128_f32[1]) + " ";
			this->TVData += to_string(this->posmap[i][U-1].m128_f32[2]) + "\n";
			this->TVData += "v ";
			this->TVData += to_string(this->posmap2[i][U-1].m128_f32[0]) + " ";
			this->TVData += to_string(this->posmap2[i][U-1].m128_f32[1]) + " ";
			this->TVData += to_string(this->posmap2[i][U-1].m128_f32[2]) + "\n";
			this->TVTData += "vt ";
			this->TVTData += to_string(0.1f + 0.8f*((float)i / (float)(V - 1))) + " ";
			this->TVTData += to_string(0.9f) + "\n";
			this->TVTData += "vt ";
			this->TVTData += to_string(0.1f + 0.8f*((float)i / (float)(V - 1))) + " ";
			this->TVTData += to_string(0.99f) + "\n";
		}
		edgeN = new XMVECTOR[V];
		edgeN[0] = generateNormal(this->posmap[0][0], this->posmap[1][U - 1], this->posmap2[1][U - 1]);
		for (int i = 1; i < V - 1; i++)
			edgeN[i] = generateNormal(this->posmap[i][U - 1], this->posmap[i + 1][U - 1], this->posmap2[i + 1][U - 1]);
		edgeN[V - 1] = edgeN[V - 2];
		for (int i = 1; i < V - 1; i++)
			edgeN[i] = (edgeN[i] + edgeN[i - 1]) / 2.0f;
		for (int i = 0; i < V; i++)
		{
			this->TVNData += "vn ";
			this->TVNData += to_string(edgeN[i].m128_f32[0]) + " ";
			this->TVNData += to_string(edgeN[i].m128_f32[1]) + " ";
			this->TVNData += to_string(edgeN[i].m128_f32[2]) + "\n";
		}
		delete[]edgeN;
		for (int i = 0; i < V - 1; i++)
		{
			this->TFData += "f ";
			this->TFData += to_string(totalVcount + i * 2 + 1) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 1) + '/';
			this->TFData += to_string(totalVNcount + i + 1);

			this->TFData += " ";
			this->TFData += to_string(totalVcount + i * 2 + 3) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 3) + '/';
			this->TFData += to_string(totalVNcount + i + 2);

			this->TFData += " ";
			this->TFData += to_string(totalVcount + i * 2 + 4) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 4) + '/';
			this->TFData += to_string(totalVNcount + i + 2);

			this->TFData += " ";
			this->TFData += to_string(totalVcount + i * 2 + 2) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 2) + '/';
			this->TFData += to_string(totalVNcount + i + 1) + '\n';
		}
		totalVcount += 2 * V;
		totalVTcount += 2 * V;
		totalVNcount += V;
		Vcount += 2 * U;
		VTcount += 2 * U;
		VNcount += U;
	}
	//存边3、4面（四边形）
	else
	{
		//存边3面
		for (int i = 0; i < V; i++)
		{
			this->TVData += "v ";
			this->TVData += to_string(this->posmap[i][U-1].m128_f32[0]) + " ";
			this->TVData += to_string(this->posmap[i][U-1].m128_f32[1]) + " ";
			this->TVData += to_string(this->posmap[i][U-1].m128_f32[2]) + "\n";
			this->TVData += "v ";
			this->TVData += to_string(this->posmap2[i][U-1].m128_f32[0]) + " ";
			this->TVData += to_string(this->posmap2[i][U-1].m128_f32[1]) + " ";
			this->TVData += to_string(this->posmap2[i][U-1].m128_f32[2]) + "\n";
			this->TVTData += "vt ";
			this->TVTData += to_string(0.1f + 0.8f*((float)i / (float)(V - 1))) + " ";
			this->TVTData += to_string(0.9f) + "\n";
			this->TVTData += "vt ";
			this->TVTData += to_string(0.1f + 0.8f*((float)i / (float)(V - 1))) + " ";
			this->TVTData += to_string(0.99f) + "\n";
		}
		edgeN = new XMVECTOR[V];
		for (int i = 0; i < V - 1; i++)
			edgeN[i] = generateNormal(this->posmap[i][U - 1], this->posmap[i + 1][U - 1], this->posmap2[i + 1][U - 1]);
		edgeN[V - 1] = edgeN[V - 2];
		for (int i = 1; i < V - 1; i++)
			edgeN[i] = (edgeN[i] + edgeN[i - 1]) / 2.0f;
		for (int i = 0; i < V; i++)
		{
			this->TVNData += "vn ";
			this->TVNData += to_string(edgeN[i].m128_f32[0]) + " ";
			this->TVNData += to_string(edgeN[i].m128_f32[1]) + " ";
			this->TVNData += to_string(edgeN[i].m128_f32[2]) + "\n";
		}
		delete[]edgeN;
		for (int i = 0; i < V - 1; i++)
		{
			this->TFData += "f ";
			this->TFData += to_string(totalVcount + i * 2 + 1) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 1) + '/';
			this->TFData += to_string(totalVNcount + i + 1);

			this->TFData += " ";
			this->TFData += to_string(totalVcount + i * 2 + 3) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 3) + '/';
			this->TFData += to_string(totalVNcount + i + 2);

			this->TFData += " ";
			this->TFData += to_string(totalVcount + i * 2 + 4) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 4) + '/';
			this->TFData += to_string(totalVNcount + i + 2);

			this->TFData += " ";
			this->TFData += to_string(totalVcount + i * 2 + 2) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 2) + '/';
			this->TFData += to_string(totalVNcount + i + 1) + '\n';
		}
		totalVcount += 2 * V;
		totalVTcount += 2 * V;
		totalVNcount += V;
		Vcount += 2 * V;
		VTcount += 2 * V;
		VNcount += V;

		//存边4面
		for (int i = 0; i < U; i++)
		{
			this->TVData += "v ";
			this->TVData += to_string(this->posmap[0][i].m128_f32[0]) + " ";
			this->TVData += to_string(this->posmap[0][i].m128_f32[1]) + " ";
			this->TVData += to_string(this->posmap[0][i].m128_f32[2]) + "\n";
			this->TVData += "v ";
			this->TVData += to_string(this->posmap2[0][i].m128_f32[0]) + " ";
			this->TVData += to_string(this->posmap2[0][i].m128_f32[1]) + " ";
			this->TVData += to_string(this->posmap2[0][i].m128_f32[2]) + "\n";
			this->TVTData += "vt ";
			this->TVTData += to_string(0.01f) + " ";
			this->TVTData += to_string(0.1f + 0.8f*((float)i / (float)(U - 1))) + "\n";
			this->TVTData += "vt ";
			this->TVTData += to_string(0.1f) + " ";
			this->TVTData += to_string(0.1f + 0.8f*((float)i / (float)(U - 1))) + "\n";

		}
		edgeN = new XMVECTOR[U];
		for (int i = 0; i < U - 1; i++)
			edgeN[i] = generateNormal(this->posmap[0][i], this->posmap[0][i + 1], this->posmap2[0][i + 1]);
		edgeN[U - 1] = edgeN[U - 2];
		for (int i = 1; i < U - 1; i++)
			edgeN[i] = (edgeN[i] + edgeN[i - 1]) / 2.0f;
		for (int i = 0; i < U; i++)
		{
			this->TVNData += "vn ";
			this->TVNData += to_string(edgeN[i].m128_f32[0]) + " ";
			this->TVNData += to_string(edgeN[i].m128_f32[1]) + " ";
			this->TVNData += to_string(edgeN[i].m128_f32[2]) + "\n";
		}
		delete[]edgeN;
		for (int i = 0; i < U - 1; i++)
		{
			this->TFData += "f ";
			this->TFData += to_string(totalVcount + i * 2 + 1) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 1) + '/';
			this->TFData += to_string(totalVNcount + i + 1);

			this->TFData += " ";
			this->TFData += to_string(totalVcount + i * 2 + 3) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 3) + '/';
			this->TFData += to_string(totalVNcount + i + 2);

			this->TFData += " ";
			this->TFData += to_string(totalVcount + i * 2 + 4) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 4) + '/';
			this->TFData += to_string(totalVNcount + i + 2);

			this->TFData += " ";
			this->TFData += to_string(totalVcount + i * 2 + 2) + '/';
			this->TFData += to_string(totalVTcount + i * 2 + 2) + '/';
			this->TFData += to_string(totalVNcount + i + 1) + '\n';
		}
		totalVcount += 2 * U;
		totalVTcount += 2 * U;
		totalVNcount += U;
		Vcount += 2 * U;
		VTcount += 2 * U;
		VNcount += U;
	}
}

void SurfaceGenerate::printmap()
{
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			Common::printXMVECTOR(this->bmap[i][j],0);
			cout << "     ";
		}
		cout << endl;
	}
	cout << endl;
}

void SurfaceGenerate::printPosmap()
{
	for (int i = 0; i < V; i++)
	{
		for (int j = 0; j < U; j++)
		{
			Common::printXMVECTOR(this->posmap[i][j],0);
			cout << "      ";
		}
		cout << endl;
	}
	cout << endl;
}

void SurfaceGenerate::printPosmap2()
{
	for (int i = 0; i < V; i++)
	{
		for (int j = 0; j < U; j++)
		{
			Common::printXMVECTOR(this->posmap2[i][j],0);
			cout << "      ";
		}
		cout << endl;
	}
	cout << endl;
}

void SurfaceGenerate::printfnmap()
{
	for (int i = 0; i < V-1; i++)
	{
		for (int j = 0; j < U-1; j++)
		{
			Common::printXMVECTOR(this->fnmap[i][j],0);
			cout << "      ";
		}
		cout << endl;
	}
	cout << endl;
}

void SurfaceGenerate::printvnmap()
{
	for (int i = 0; i < V; i++)
	{
		for (int j = 0; j < U; j++)
		{
			Common::printXMVECTOR(this->vnmap[i][j],0);
			cout << "      ";
		}
		cout << endl;
	}
	cout << endl;
}

int SurfaceGenerate::mainStream()
{
	this->setmap();
	//this->printmap();
	this->setpos();
	//this->printPosmap();
	this->setfn();
	//this->printfnmap();
	this->setvn();
	//this->printvnmap();
	this->setuv();
	this->generateMesh();
	//this->printPosmap();
	//cout << this->TVNData << endl;
	cout << "分辨率(U,V)：" << this->U << ',' << this->V << endl;
	cout << "顶点数：" << Vcount << " 纹理坐标数：" << VTcount << " 法向量数：" << VNcount << endl << endl;
	cout << "已生成蒙皮" << endl;
	cout << "当前总顶点数：" << totalVcount << " 总纹理坐标数：" << totalVTcount << " 总法向量数：" << totalVNcount << endl << endl;
	return 1;
}

