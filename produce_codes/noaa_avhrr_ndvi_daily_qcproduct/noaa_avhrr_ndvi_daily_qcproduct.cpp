// noaa_avhrr_ndvi_daily_qcproduct.cpp : ¶¨Òå¿ØÖÆÌ¨Ó¦ÓÃ³ÌÐòµÄÈë¿Úµã¡£
//noaaÖ²±»Ö¸ÊýÈÕ²úÆ·¿¼ÂÇQCÊä³ötif£¬Öµ·¶Î§²»±ä»¯£¬ÈÔÈ»ÊÇshortÐÍ¡£

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
	string vipath = "NETCDF:\"" + filepath + "\":NDVI";//2017-12-04
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
	int pos = infilename.find("NOAA-") ;
	if( pos == string::npos )
	{
		string outname = infilename + ".viqa.tif";
		return outdir + outname;
	}else 
	{
		string ymdstr = infilename.substr(pos + 8 , 8) ;
		string outname = "noaa-avhrr-ndvi-avh13c1-" + ymdstr + "-day.viqa.tif";
		return outdir + outname;
	}
}

int processOne(string filepath , string outfilepath )
{
	string vipath = "NETCDF:\"" + filepath + "\":NDVI";
	string qapath = "NETCDF:\"" + filepath + "\":QA";

	GDALDataset* dsvi = (GDALDataset*)GDALOpen(vipath.c_str(), GA_ReadOnly);
	GDALDataset* dsqa = (GDALDataset*)GDALOpen(qapath.c_str(), GA_ReadOnly);

	if (dsvi == 0 || dsqa == 0)
	{
		if (dsvi) GDALClose(dsvi);
		if (dsqa) GDALClose(dsqa);
		cout << "Error : can not open " << vipath << " or " << qapath << endl;
		return 10;
	}

	int rasterXSize = dsvi->GetRasterXSize();
	int rasterYSize = dsvi->GetRasterYSize();
	GDALDataType theDataType = dsvi->GetRasterBand(1)->GetRasterDataType();

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(outfilepath.c_str(), rasterXSize, rasterYSize, 1, theDataType, 0);

	int bufferSize = rasterYSize * rasterXSize;
	int* dataBufferNdvi = new int[bufferSize];
	short* dataBufferQA = new short[bufferSize];
	if (dataBufferNdvi == 0 || dataBufferQA == 0)
	{
		if (dataBufferNdvi) delete[] dataBufferNdvi;
		if (dataBufferQA) delete[] dataBufferQA;
		GDALClose(outds) ;
		GDALClose(dsvi) ;
		GDALClose(dsqa) ;
		cout << "Error : can not alloc memory for ndvi or qa " << bufferSize << endl;
		return 20;
	}

	dsvi->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, rasterXSize, rasterYSize,
		dataBufferNdvi, rasterXSize, rasterYSize, GDT_Int32, 0, 0, 0);
	dsqa->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, rasterXSize, rasterYSize,
		dataBufferQA, rasterXSize, rasterYSize, GDT_Int16, 0, 0, 0);
	GDALClose(dsvi);
	GDALClose(dsqa);
	for (int i = 0; i < bufferSize; ++ i )
	{
		short qaval = dataBufferQA[i];
		if ( wft_isValidNoaaNdvi(dataBufferNdvi[i] , qaval ) )
		{

		}
		else {
			dataBufferNdvi[i] = -9999;//
		}
		wft_term_progress(i, bufferSize);
	}
	outds->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, rasterXSize, rasterYSize,
			dataBufferNdvi, rasterXSize, rasterYSize, GDT_Int32, 0, 0, 0);

	if (dataBufferNdvi) delete[] dataBufferNdvi;
	if (dataBufferQA) delete[] dataBufferQA;
	GDALClose(outds);

	return 100;
}



int main(int argc , char** argv )
{
	cout << "A program to make daily noaa avhrr ndvi qc product.2017-11-28." << endl;
	cout << "V1.0  2017-11-28. by wangfeng1@piesat.cn" << endl;
	cout << "V2.0  bugfixed for isValidFile 2017-12-04." << endl;
	cout << "V2.1  get off qa cloud only  2017-12-17." << endl;
	
	if (argc == 1)
	{
		cout<<"sample call:"<<endl ;
		cout<<"noaa_avhrr_ndvi_daily_qcproduct startup.txt"<<endl ;
		cout<<"***startup.txt example***"<<endl ;
		cout<<"#indir"<<endl;
		cout<<"D:/avhrr_ndvi/"<<endl;
		cout<<"#outdir"<<endl;
		cout<<"D:/avhrr_ndvi/combinations/qcdaily/"<<endl;
		cout<<""<<endl;
		return 101;
	}
	string startup = argv[1];
	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	string outdir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);
	/*string mask = wft_getValueFromExtraParamsFile(startup, "#mask", true);
	string gnuplot = wft_getValueFromExtraParamsFile(startup, "#gnuplot", true);
	string plottem = wft_getValueFromExtraParamsFile(startup, "#plottem", true);
	
	string insert = wft_getValueFromExtraParamsFile(startup, "#insert", true);
	string host = wft_getValueFromExtraParamsFile(startup, "#host", true);
	string user = wft_getValueFromExtraParamsFile(startup, "#user", true);
	string pwd = wft_getValueFromExtraParamsFile(startup, "#pwd", true);
	string db = wft_getValueFromExtraParamsFile(startup, "#db", true);
	string tb = wft_getValueFromExtraParamsFile(startup, "#tb", true);
	string pid = wft_getValueFromExtraParamsFile(startup, "#pid", true);*/

	int todayYmd = wft_current_dateymd_int();

	cout <<"Today:"<< todayYmd << endl;

	vector<string> allfiles;
	wft_get_allSelectedFiles(indir, "AVHRR-Land_" , ".nc" , -1 , "" , allfiles );

	GDALAllRegister();

	int num = allfiles.size();
	for (int i = 0; i < num; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		if (isValidFile(filepath))
		{
			string outpath = makeoutputfilepath(filename, outdir);
			if (wft_test_file_exists(outpath) == false)
			{
				cout << "processing " << filename <<"..." << endl;
				processOne(filepath, outpath );
			}
			else if( isGDALAccessable(outpath) == false )
			{
				cout << "reprocessing " << outpath << "..." << endl;
				processOne(filepath, outpath);
			}
		}

	}

	cout << "all done." << endl;
    return 0;
}

