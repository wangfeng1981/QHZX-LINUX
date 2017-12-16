// fy4_disk2wgs84.cpp : 定义控制台应用程序的入口点。
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




int processOne( string filepath ,string outpathgeos ,
 string outpathwgs84 , string gdaltranslate , string gdalwarp , string nodata )
{
	GDALDataset* inds = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	if (inds == 0)
	{
		cout << "Error : can not open " << filepath << endl;
		return 10;
	}
	GDALClose(inds) ;

	string cmd1 = gdaltranslate +  " -a_srs \"+proj=geos +h=35785863 +a=6378137.0 +b=6356752.3 +lon_0=104.7 +no_defs\" -a_ullr -5496000 5496000 5496000 -5496000 " + filepath + " " + outpathgeos ;
	int res1 = system( cmd1.c_str() ) ;
	cout<<"gdal_translate return code "<<res1<<endl ;

	if( wft_test_file_exists(outpathgeos) )
	{
		string cmd2 = gdalwarp + " -overwrite  -t_srs \"+proj=latlong +ellps=WGS84 +pm=0\" -tr 0.04 0.04 -te 20 -85 180 85 -srcnodata "
			+ nodata + " -dstnodata " + nodata + " " + outpathgeos + " " + outpathwgs84 ;
		int res2 = system( cmd2.c_str() ) ;
		cout<<"gdalwarp return code "<<res2<<endl ;	
		if( wft_test_file_exists(outpathwgs84) )
		{
			cout<<"Successfully make "<<outpathwgs84<<endl ;
		}else {
			cout<<"Error failed to make "<<outpathwgs84<<endl ;
		}
		wft_remove_file(outpathgeos) ;
	}else {
		cout<<"Error failed to make "<<outpathgeos<<endl ;
	}
	return 0;
}




int main(int argc, char** argv)
{
	cout << "A program to make fy4 disk to wgs84." << endl;
	cout << "V1.0  2017-12-07. by wangfeng1@piesat.cn" << endl;

	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "fy4_disk2wgs84 startup.txt" << endl;

		cout << "*************** startup.txt example *************" << endl;
		cout << "#indir" << endl;
		cout << "/.../" << endl;
		cout << "#outdir" << endl;
		cout << "/.../" << endl;
		cout << "#infixprefix" << endl;
		cout << "..." << endl;
		cout << "#infixtail" << endl;
		cout << "..." << endl;
		cout << "#gdaltranslate" << endl;
		cout << "..." << endl;
		cout << "#gdalwarp" << endl;
		cout << "..." << endl;
		cout << "#nodata" << endl;
		cout << "..." << endl;

		return 101;
	}
	string startup = argv[1];
	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	string outdir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);

	string inprefix = wft_getValueFromExtraParamsFile(startup, "#infixprefix", true);
	string intail = wft_getValueFromExtraParamsFile(startup, "#infixtail", true);

	string gdalwarp = wft_getValueFromExtraParamsFile(startup, "#gdalwarp", true);
	string gdaltranslate = wft_getValueFromExtraParamsFile(startup, "#gdaltranslate", true);
	string nodata = wft_getValueFromExtraParamsFile(startup, "#nodata", true);

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
		string geosname = filename + ".geos.tif" ;
		string wgs84name = filename + ".wgs84.tif";
		string outpathgeos = outdir + geosname;
		string outpathwgs84 = outdir + wgs84name ;
		if (wft_test_file_exists(outpathwgs84) == false )
		{
			cout<<"processing "<<filename<<endl;
			processOne( filepath , outpathgeos , outpathwgs84 , gdaltranslate , gdalwarp ,nodata );
			cout<<i<<"/"<<num<<endl ;
		}
	}

	return 0;
}


