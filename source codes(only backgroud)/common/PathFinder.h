#pragma once
#include <iostream>
#include <string>
#include <windows.h>
#include <io.h>
using namespace std;

class PathFinder
{
public:

	static string getFiles(string path, string tgtFolder, int times, int stopTimes)
	{
		//cout << path << endl;
		//文件句柄  
		intptr_t   hFile = 0;
		//文件信息  
		struct _finddata_t fileinfo;
		string p;

		if ((hFile = _findfirst((path + "\\*").c_str(), &fileinfo)) != -1)
		{
			do
			{
				//如果是目录,迭代之  
				//如果不是,加入列表  
				if (fileinfo.attrib ==  _A_SUBDIR)
				{
					//cout << "here" << endl;
					//cout << fileinfo.name << endl;

					if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					{
						//cout << fileinfo.name << endl;
						if (strcmp(fileinfo.name, tgtFolder.c_str()) == 0)
							return path + '\\' + fileinfo.name;
						//	cout << fileinfo.name << endl;
						if (times < stopTimes)
						{
							string subPath = getFiles(path + "\\" + fileinfo.name, tgtFolder, times + 1, stopTimes);
							if (strcmp(subPath.c_str(), "false") != 0)
								return subPath;
						}
					}
				}
			} while (_findnext(hFile, &fileinfo) == 0);
			_findclose(hFile);
		}
		return "false";
	}
};
