// wgs84_to_albers.cpp : ¶¨Òå¿ØÖÆÌ¨Ó¦ÓÃ³ÌÐòµÄÈë¿Úµã¡£
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


int processOne(string inpath, string outpath , string gdalwarp , string nodata )
{
	if ( isGDALAccessable(inpath) == false )
	{
		cout << "Error : can not open " << inpath << endl;
		return 10;
	}

	string cmd1 = gdalwarp + " -overwrite -t_srs \"+proj=aea +ellps=krass +lon_0=105 +lat_1=25 +lat_2=47\" -srcnodata " + nodata
		+ " -dstnodata " + nodata + " " + inpath + " " + outpath;
	
	int res = system(cmd1.c_str());
	cout << "gdalwarp result:" << res << endl;

	if (isGDALAccessable(outpath) == false )
	{
		cout << "Error : failed to open " << outpath << endl;
		return 20;
	}

	return 100;
}




int main(int argc, char** argv)
{
	cout << "A program to convert wgs84 tif into albers tif.2017-11-30." << endl;
	cout << "V1.0  2017-11-30. by wangfeng1@piesat.cn" << endl;

	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "wgs84_to_albers startup.txt" << endl;

		cout << "*************** startup.txt example *************" << endl;
		cout << "#indir" << endl;
		cout << "/.../" << endl;
		cout << "#outdir" << endl;
		cout << "/.../" << endl;
		cout << "#infixprefix" << endl;
		cout << "..." << endl;
		cout << "#infixtail" << endl;
		cout << "..." << endl;
		cout << "#gdalwarp" << endl;
		cout << "gdalwarp" << endl;
		cout << "#nodata" << endl;
		cout << "-9999" << endl;

		return 101;
	}
	string startup = argv[1];
	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	string outdir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);

	string inprefix = wft_getValueFromExtraParamsFile(startup, "#infixprefix", true);
	string intail = wft_getValueFromExtraParamsFile(startup, "#infixtail", true);

	string gdalwarp = wft_getValueFromExtraParamsFile(startup, "#gdalwarp", true);
	string nodata = wft_getValueFromExtraParamsFile(startup, "#nodata", true);

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
		string outname = filename + ".albers.tif";
		string outpath = outdir + outname;
		if (wft_test_file_exists(outpath) == false)
		{
			cout << "processing " << filename << "..." << endl;
			processOne(filepath, outpath , gdalwarp , nodata );
		}
	}

	return 0;
}


