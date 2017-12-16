// ymd7toymd8.cpp : 定义控制台应用程序的入口点。
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


string getDotExtName( string filename )
{
	int pos = filename.rfind(".") ;
	if( pos != string::npos )
	{
		int len = filename.length() - pos ;
		return filename.substr( pos , len ) ;
	}
	else {
			return "" ;
	}

}


int main(int argc, char** argv)
{
	cout << "A program to convert yyyyddd to yyyyMMdd ." << endl;
	cout << "V1.0  2017-11-29. by wangfeng1@piesat.cn" << endl;

	if (argc != 11)
	{
		cout << "sample call:" << endl;
		cout << "ymd7toymd8 -indir /.../ -outdir /.../ -loc 7 -inprefix ppp_ -intail .nc" << endl;

		return 101;
	}
	
	string indir , outdir , locstr , inprefix , intail  ;
	wft_has_param( argc , argv , "-indir" , indir  , true ) ;
	wft_has_param( argc , argv , "-outdir" , outdir  , true ) ;
	wft_has_param( argc , argv , "-loc" , locstr  , true ) ;
	wft_has_param( argc , argv , "-inprefix" , inprefix  , true ) ;
	wft_has_param( argc , argv , "-intail" , intail  , true ) ;

	int loc = wft_str2int(locstr) ;

	vector<string> allfiles;
	wft_get_allSelectedFiles(indir ,  inprefix , intail , -1 , "" , allfiles );


	int num = allfiles.size();
	for (int i = 0; i < num; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);

		string ymd7 = filename.substr(loc,7) ;
		int year = wft_str2int(ymd7.substr(0,4)) ;
		int doy = wft_str2int( ymd7.substr(4,3) ) ;
		int mon = 0 ;
		int day = 0 ;
		wft_convertdayofyear2monthday( year , doy , mon , day ) ;
		int ymd8 = year * 10000 + mon * 100 + day ;
		string ymd8str = wft_int2str(ymd8) ;

		string ntail =  getDotExtName( filename ) ;

		string outname = filename + "." + ymd8str + "000000" + ntail ;
		string outpath = outdir + outname;
		if (wft_test_file_exists(outpath) == false )
		{
			cout<<"rename "<<filename <<" -> " << outname <<endl;
			string cmd = "cp " + filepath + " " + outpath ;
			int r = system( cmd.c_str() ) ;
			cout<<i<<"/"<<num<<" r:"<<r<<endl; 
		}
	}

	return 0;
}


