// grid_wgs84_monitor.cpp : 定义控制台应用程序的入口点。
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




int processOne(string inpath  , string outpath  , double left , double resx , double top , double resy )
{
	GDALDataset* inds = (GDALDataset*)GDALOpen(inpath.c_str(), GA_ReadOnly);
	if (inds == 0)
	{
		cout << "Error : can not open " << inpath << endl;
		return 10;
	}

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->CreateCopy(outpath.c_str(), inds, false , 0, 0, 0);

	if (outds == 0)
	{
		cout << "Error : can not createcopy " << inpath << endl;
		GDALClose(inds);
		return 20;
	}

	double geotrans[6];
	geotrans[0] = left; 
	geotrans[1] = resx;
	geotrans[2] = 0;
	geotrans[3] = top;
	geotrans[4] = 0;
	geotrans[5] = resy;//this should be negative value.
	if (resy >= 0)
	{
		cout << "Warning : resolution y may be invalid " << resy << " . The resy should be negative value." << endl;
	}

	outds->SetGeoTransform(geotrans);//17

	OGRSpatialReference osrs;
	char* pszSRS_WKT = 0;
	osrs.SetWellKnownGeogCS("EPSG:4326");
	osrs.exportToWkt(&pszSRS_WKT);
	outds->SetProjection(pszSRS_WKT);
	CPLFree(pszSRS_WKT);

	GDALClose(inds);
	GDALClose(outds);

	return 100;
}




int main(int argc, char** argv)
{
	cout << "A program to give grid data a wgs84 coordinate.2017-11-29." << endl;
	cout << "V1.0  2017-11-29. by wangfeng1@piesat.cn" << endl;

	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "grid_wgs84_monitor startup.txt" << endl;

		cout << "*************** startup.txt example *************" << endl;
		cout << "#indir" << endl;
		cout << "/.../" << endl;
		cout << "#outdir" << endl;
		cout << "/.../" << endl;
		cout << "#infixprefix" << endl;
		cout << "..." << endl;
		cout << "#infixtail" << endl;
		cout << "..." << endl;
		cout << "#left" << endl;
		cout << "-180" << endl;
		cout << "#resx" << endl;
		cout << "0.05" << endl;
		cout << "#top" << endl;
		cout << "90" << endl;
		cout << "#resy" << endl;
		cout << "-0.05" << endl;
		return 101;
	}
	string startup = argv[1];
	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	string outdir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);

	string inprefix = wft_getValueFromExtraParamsFile(startup, "#infixprefix", true);
	string intail = wft_getValueFromExtraParamsFile(startup, "#infixtail", true);

	string leftstr = wft_getValueFromExtraParamsFile(startup, "#left", true);
	string resxstr = wft_getValueFromExtraParamsFile(startup, "#resx", true);
	string topstr = wft_getValueFromExtraParamsFile(startup, "#top", true);
	string resystr = wft_getValueFromExtraParamsFile(startup, "#resy", true);

	double left = atof(leftstr.c_str());
	double resx = atof(resxstr.c_str());
	double top = atof(topstr.c_str());
	double resy = atof(resystr.c_str());


	int todayYmd = wft_current_dateymd_int();

	cout << "Today:" << todayYmd << endl;

	vector<string> allfiles;
	wft_get_allSelectedFiles(indir ,  inprefix , intail , -1 , "" , allfiles );

	GDALAllRegister();

	int num = allfiles.size();
	for (int i = 0; i < num; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string outname = filename + ".wgs84.tif";
		string outpath = outdir + outname;
		if (wft_test_file_exists(outpath) == false )
		{
			cout<<"processing "<<filename<<" -> "<<outname<<" ... "<<endl ;
			processOne( filepath , outpath , left , resx , top , resy  );
		}
	}

	return 0;
}


