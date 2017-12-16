// fy4snc_qcproduct.cpp : ¶¨Òå¿ØÖÆÌ¨Ó¦ÓÃ³ÌÐòµÄÈë¿Úµã¡£
//fy4 snc²úÆ·0-255ÊýÖµ×ª1-10Êä³ötif only use 0300-0600

#include <iostream>
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include "../sharedcodes/wftools.h"
using namespace std;
using std::cout;


bool isInsideDb(string productFilepath, string host, string user, string pwd, string db, string tb, string pid)
{
	return false;
}

bool isGDALAccessable(string filepath)
{
	GDALDataset* ds = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	if (ds == 0)
	{
		return false;
	}
	else {
		GDALClose(ds);
		return true;
	}
	
}

bool isValidFile(string filepath)
{
	string vipath = "NETCDF:\"" + filepath + "\":NDVI";//2017-12-04 maybe HDF5 in windows.
	GDALDataset* ds = (GDALDataset*)GDALOpen(vipath.c_str(), GA_ReadOnly);
	if (ds == 0)
	{
		return false;
	}
	else {
		GDALClose(ds) ;//2017-12-04
		return true;
	}
}

string makeoutputfilepath(string infilename, string outdir)
{
	int pos = infilename.find("NOAA-");
	if (pos == string::npos)
	{
		string outname = infilename + ".viqa.tif";
		return outdir + outname;
	}
	else
	{
		string ymdstr = infilename.substr(pos + 8, 8);
		string outname = "noaa-avhrr-ndvi-avh13c1-" + ymdstr + "-day.viqa.tif";
		return outdir + outname;
	}
}

int processOne(string filepath , string outfilepath )
{
	string dspath = "HDF5:" + filepath + "://SNC";

	GDALDataset* ds0 = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);

	if (ds0 == 0  )
	{
		cout << "Error : can not open " << filepath  << endl;
		return 10;
	}

	int rasterXSize = ds0->GetRasterXSize();
	int rasterYSize = ds0->GetRasterYSize();
	
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(outfilepath.c_str(), rasterXSize, rasterYSize, 1, GDT_Byte , 0);

	int bufferSize = rasterYSize * rasterXSize;
	short* buffer = new short[bufferSize];

	if (buffer == 0 )
	{
		GDALClose(outds) ;
		GDALClose(ds0) ;
		cout << "Error : can not alloc memory for snow " << bufferSize << endl;
		return 20;
	}
	ds0->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, rasterXSize, rasterYSize,
		buffer, rasterXSize, rasterYSize, GDT_Int16, 0, 0, 0);
	GDALClose(ds0);
	for (int i = 0; i < bufferSize; ++ i )
	{
		short snc0 = buffer[i];
		switch (snc0)
		{
		case 255: buffer[i] = 1; break;
		case 0:buffer[i] = 2; break;
		case 11:buffer[i] = 3; break;
		case 254:buffer[i] = 4; break;
		case 1:buffer[i] = 5; break;
		case 240:buffer[i] = 5; break;
		case 50:buffer[i] = 6; break;
		case 39:buffer[i] = 7; break;
		case 37:buffer[i] = 7; break;
		case 25:buffer[i] = 8; break;
		case 100:buffer[i] = 9; break;
		case 200:buffer[i] = 10; break;
		default:
			buffer[i] = 0;
			break;
		}
	}
	outds->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, rasterXSize, rasterYSize,
			buffer, rasterXSize, rasterYSize, GDT_Int16, 0, 0, 0);

	if (buffer) delete[] buffer;
	GDALClose(outds);
	return 100;
}



int main(int argc , char** argv )
{
	cout << "A program to make daily noaa avhrr ndvi qc product.2017-11-28." << endl;
	cout << "V1.0  2017-11-28. by wangfeng1@piesat.cn" << endl;
	cout << "V2.0  bugfixed for isValidFile 2017-12-04." << endl;
	
	if (argc == 1)
	{
		cout<<"sample call:"<<endl ;
		cout<<"noaa_avhrr_ndvi_daily_qcproduct startup.txt"<<endl ;
		cout<<"***startup.txt example***"<<endl ;
		cout<<"#indir"<<endl;
		cout<<"D:/fy4sncl2/"<<endl;
		cout<<"#outdir"<<endl;
		cout<<"D:/fy4sncl2v2/"<<endl;
		cout<<""<<endl;
		return 101;
	}
	string startup = argv[1];
	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	string outdir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);

	int todayYmd = wft_current_dateymd_int();

	cout <<"Today:"<< todayYmd << endl;

	//FY4A-_AGRI--_N_DISK_1047E_L2-_SNC-_MULT_NOM_20170731063000_20170731064459_4000M_V0001.NC
	vector<string> allfiles;
	wft_get_allSelectedFiles(indir, "FY4A-_AGRI--_N_DISK_" , ".NC" , -1 , "" , allfiles );

	GDALAllRegister();

	int num = allfiles.size();
	for (int i = 0; i < num; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string dtstr = filename.substr(44, 14);
		string lonstr = filename.substr(20, 4);
		int hhmm = wft_str2int(dtstr.substr(8, 4));
		if (hhmm >= 300 && hhmm <= 600)
		{
			string outname = "fy4a_snc_l2v2_" + lonstr + "_"+ dtstr + ".tif";
			string outpath = outdir + outname;
			if (wft_test_file_exists(outpath) == false)
			{
				cout << "making " << outname << "..." << endl;
				processOne(filepath, outpath);
			}
		}
		cout << i << "/" << num << endl;
	}

	cout << "all done." << endl;
    return 0;
}

