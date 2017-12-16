// fy4_qc_product.cpp : 定义控制台应用程序的入口点。
//


#include <iostream>
#include "ogr_spatialref.h"
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include "../sharedcodes/wftools.h"

using namespace std;
using std::cout;




int processOne(string inpath, string outpath,string dsprefix , string dstail , string qcprefix , string qctail, 
	int qc0 , int qc1 , double valid0,double valid1, double fill )
{
	string path1 = dsprefix + inpath + dstail ;
	string path2 = qcprefix + inpath + qctail;
	GDALDataset* inds = (GDALDataset*)GDALOpen(path1.c_str(), GA_ReadOnly);
	if (inds == 0)
	{
		cout << "Error : can not open " << path1 << endl;
		return 10;
	}
	GDALDataset* qcds = (GDALDataset*)GDALOpen(path2.c_str(), GA_ReadOnly);
	if (qcds == 0)
	{
		cout << "Error : can not open " << path2 << endl;
		GDALClose(inds);
		return 11;
	}

	int xsize = inds->GetRasterXSize();
	int ysize = inds->GetRasterYSize();
	GDALDataType dt = inds->GetRasterBand(1)->GetRasterDataType();

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(outpath.c_str(), xsize, ysize, 1, dt, 0);
	if (outds == 0)
	{
		cout << "Error : can not createcopy " << inpath << endl;
		GDALClose(inds);
		GDALClose(qcds);
		return 20;
	}

	int allsize = xsize*ysize;
	double * buff = new double[allsize];
	if (buff == 0)
	{
		cout << "Error : can not alloc memory " << endl;
		GDALClose(inds);
		GDALClose(qcds);
		return 30;
	}
	int* qcbuff = new int[allsize];
	if (qcbuff == 0)
	{
		cout << "Error : can not alloc memory " << endl;
		GDALClose(inds);
		GDALClose(qcds);
		delete[] buff;
		return 31;
	}

	inds->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, xsize, ysize, buff, xsize, ysize, GDT_Float64, 0, 0, 0);
	qcds->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, xsize, ysize, qcbuff, xsize, ysize, GDT_Int32, 0, 0, 0);
	GDALClose(inds);
	GDALClose(qcds);
	for (int i = 0; i < allsize; ++i)
	{
		if (buff[i] >= valid0 && buff[i] <= valid1)
		{
			if (qcbuff[i] >= qc0 && qcbuff[i] <= qc1)
			{

			}
			else {
				buff[i] = fill;
			}
		}
	}

	outds->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, xsize, ysize, buff, xsize, ysize, GDT_Float64, 0, 0, 0);
	GDALClose(outds);
	delete[] buff;
	delete[] qcbuff;

	return 100;
}




int main(int argc, char** argv)
{
	cout << "A program to make sst qc 0 product.2017-12-01." << endl;
	cout << "V1.0  2017-11-29. by wangfeng1@piesat.cn" << endl;
	cout << "V2.0  save into daily directory  2017-12-05. by wangfeng1@piesat.cn" << endl;
	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "fy4_qc_product startup.txt" << endl;

		cout << "*************** startup.txt example *************" << endl;
		cout<<"#indir"<<endl;
		cout<<"E:/testdata/fy4sst15min/2017/"<<endl;
		cout<<"#outdir"<<endl;
		cout<<"E:/testdata/fy4sst15min/2017/"<<endl;
		cout<<"#infixprefix"<<endl;
		cout<<"FY4A-_AGRI--_N_DISK"<<endl;
		cout<<"#infixtail"<<endl;
		cout<<".NC"<<endl;
		cout<<"#outprefix"<<endl;
		cout<<"fy4sst_l2_"<<endl;
		cout<<"#outtail"<<endl;
		cout<<"_4km_qc0.tif"<<endl;
		cout<<"#qc0"<<endl;
		cout<<"0"<<endl;
		cout<<"#qc1"<<endl;
		cout<<"0"<<endl;
		cout<<"#valid0"<<endl;
		cout<<"-5"<<endl;
		cout<<"#valid1"<<endl;
		cout<<"45"<<endl;
		cout<<"#fill"<<endl;
		cout<<"-888"<<endl;
		cout<<"#dsprefix"<<endl;
		cout<<"HDF5:\""<<endl;
		cout<<"#dstail"<<endl;
		cout<<"\"://SST"<<endl;
		cout<<"#qcprefix"<<endl;
		cout<<"HDF5:\""<<endl;
		cout<<"#qctail"<<endl;
		cout<<"\"://SST"<<endl;
		cout<<"#ymdloc"<<endl;
		cout<<"44"<<endl;
		return 101;
	}
	string startup = argv[1];
	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	string outdir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);

	string inprefix = wft_getValueFromExtraParamsFile(startup, "#infixprefix", true);
	string intail = wft_getValueFromExtraParamsFile(startup, "#infixtail", true);

	string outprefix = wft_getValueFromExtraParamsFile(startup, "#outprefix", true);
	string outtail = wft_getValueFromExtraParamsFile(startup, "#outtail", true);

	string qc0str = wft_getValueFromExtraParamsFile(startup, "#qc0", true);
	string qc1str = wft_getValueFromExtraParamsFile(startup, "#qc1", true);

	string valid0str = wft_getValueFromExtraParamsFile(startup, "#valid0", true);
	string valid1str = wft_getValueFromExtraParamsFile(startup, "#valid1", true);

	string fillstr = wft_getValueFromExtraParamsFile(startup, "#fill", true);

	string dsprefix = wft_getValueFromExtraParamsFile(startup, "#dsprefix", true);
	string dstail = wft_getValueFromExtraParamsFile(startup, "#dstail", true);

	string qcprefix = wft_getValueFromExtraParamsFile(startup, "#qcprefix", true);
	string qctail = wft_getValueFromExtraParamsFile(startup, "#qctail", true);
	
	string ymdlocstr = wft_getValueFromExtraParamsFile(startup, "#ymdloc", true);
	int ymdloc = wft_str2int(ymdlocstr);

	int qc0 = wft_str2int(qc0str);
	int qc1 = wft_str2int(qc1str);

	double fill = atof(fillstr.c_str());
	double valid0 = atof(valid0str.c_str());
	double valid1 = atof(valid1str.c_str());


	int todayYmd = wft_current_dateymd_int();

	cout << "Today:" << todayYmd << endl;

	vector<string> allfiles;
	wft_get_allSelectedFiles(indir, inprefix, intail, -1, "", allfiles);

	GDALAllRegister();

	int num = allfiles.size();
	for (int i = 0; i < num; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymd14 = filename.substr(ymdloc, 14);
		string ymd8 = filename.substr(ymdloc,8) ;
		string outname = outprefix + ymd14 + outtail;
		string outdir1 = outdir + ymd8 + "/" ;
		mkdir(outdir1.c_str() , 0777) ;
		
		string outpath = outdir + ymd8 + "/" + outname;//2017-12-05

		if (wft_test_file_exists(outpath) == false)
		{
			cout << "processing " << filename << endl;
			processOne(filepath, outpath , dsprefix , dstail , qcprefix , qctail  , qc0 , qc1 ,
				valid0 , valid1 , fill );
		}
	}

	return 0;
}


