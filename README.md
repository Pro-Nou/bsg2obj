bsg2obj-gui
=
by 摸鱼Pro  
![image](https://github.com/Pro-Nou/bsg2obj/blob/main/tex/title.png)

本软件使用 Visual Studio community 2017，c++10 进行开发编译，DirectX12 函数库进行模型网格转换，MFC 对话框开发 GUI 界面，winsocket 实现前后端通信。软件本身及生成的任何文件仅适用于学习交流，禁止用于商业用途。  
当前版本：v1.0  
详细说明见 readme(简体中文).pdf  


This software developed by using Visual Studio community 2017, c++10, uses DirectX12 to convert model meshes, MFC to design GUI, and winsocket for communication between foreground and background. This software and its files only for study and communication. Do not use them in any commercial activities.  
Current version: v1.0  
Check readme(English).pdf for more details


更新日志
=
v1.0(hotfix) 2021/1/16  
增加guid显示，便于识别问题零件  
修正了当皮肤仅有贴图没有模型时返回的缺失皮肤列表有歧义的问题  
修正了缩放后的蒙皮表面数据丢失导致的程序崩溃的问题  
修正了mod零件导致程序崩溃的问题  
修正了当零件某一轴的缩放比例为0时导致模型不可读的问题  
修正了钢筋、绞绳、皮筋长度错误的问题  
修正了GUI显示错误的问题  

v1.0 2021/1/16  
增加gui，使用winsock与BsgToOBJ.exe进行通信，BsgToOBJ.exe仍可通过控制台独立启动  
增加gui英文翻译，可在gui中选择使用的语言  
增加自定义翻译，在language table.csv中向后添加即可  
增加52桨支持，现在可以导出52桨了  
修改工坊文件夹路径，现在只需要定位到SteamLibrary\steamapps\workshop\即可  
修正当零件被反转时，导出模型时没有正确应用的问题  
修正导出的蒙皮表面厚度只有游戏中一半的问题  

v0.4 2021/1/7  
增加基于角度的新蒙皮分辨率步长，可在配置文件中设置，现在会综合边长和边弧度设置步长  
修正.mtl文件d值，现在可在配置文件中设置玻璃的透明度了  

v0.3 2021/1/6  
增加配置文件，现在可以自定点钢的识别长度阈值了  
增加新蒙皮分辨率步长，可在配置文件中设置  
*blender中.mtl文件Kd值未正确与map_Kd叠乘，在启用着色后新蒙皮的纹理贴图会存储为map_Ks，即blender中的高光，与bsg本体渲染效果有较大差异，须手动设置材质混合  
*blender中.mtl文件d值未正确生效，新蒙皮材质为玻璃时须手动启用对应材质的alpha通道  

v0.2 2021/1/5  
增加新蒙皮支持，现在可以转换新蒙皮了  
增加计时功能，现在生成的模型会以"存档名(生成时间)"为文件名  
修正滑铲和侦测块的角度错误  
修正钢筋、皮筋、绞盘长度计算错误  
*当新蒙皮材质为玻璃时，.mtl文件中对应材质的纹理贴图为相对路径，须保证Blocks文件夹与.mtl文件置于同一文件夹下  

v0.1 2020/10/19  
杂项修正  
正式发布  

a0.5 2020/10/18  
增加基于vt,vn计数的防误读机制改进  
修正法向转换错误  
修正UTF8乱码  
编译模式变更为release  

a0.4 2020/10/17  
修正变形钢角度  
修正当存档以本地形式存储皮肤，但本地无该皮肤，工坊有时，皮肤未被读取的bug  

a0.3 2020/10/16  
增加钢筋、皮筋、绞盘读取并转换  
增加.mtl材质库生成  
修正.obj读取策略，现支持3dmax导出的.obj，同时加入防误读机制  
修正零件表参数  
*变形钢角度错误  

a0.2 2020/10/14  
增加基于prefab的零件表构建  
增加大部分零件模型读取并转换  
*部分零件转换错误  

a0.1 2020/10/13  
增加.obj读取并转换  
增加.bsg读取  
增加.obj生成  
