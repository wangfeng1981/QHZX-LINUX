// image_subtract.cpp : 定义控制台应用程序的入口点。
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

bool isDemical(double val)
{
	int a = (int)val;
	double b = val - a;
	if (b > 0.000001)
	{
		return true;
	}
	else {
		return false;
	}
}


int processOne(string apath , string bpath , string outpath,
	double a0,double  a1,double  b0, double  b1,
	double ascale,double  aoffset,double bscale,double boffset,
	double fill)
{
	GDALDataset* ads = (GDALDataset*)GDALOpen(apath.c_str(), GA_ReadOnly);
	if (ads == 0)
	{
		cout << "Error : can not open " << apath << endl;
		return 10;
	}
	GDALDataset* bds = (GDALDataset*)GDALOpen(bpath.c_str(), GA_ReadOnly);
	if (bds == 0)
	{
		cout << "Error : can not open " << bpath << endl;
		GDALClose(ads);
		return 11;
	}

	int xsize = ads->GetRasterXSize();
	int ysize = ads->GetRasterYSize();
	int bxsize = bds->GetRasterXSize();
	int bysize = bds->GetRasterYSize();
	if (xsize != bxsize || ysize != bysize)
	{
		cout << "Error : a size "<<xsize <<" " << ysize << " is different with b size "<< bxsize<<" "<<bysize << "." << endl;
		GDALClose(ads);
		GDALClose(bds);
		return 20;
	}

	GDALDataType dt = ads->GetRasterBand(1)->GetRasterDataType();
	if (dt == GDT_Float64 || dt == GDT_Float32)
	{
	}
	else if( isDemical(ascale) || isDemical(bscale) ){
		dt = GDT_Float32;
	}

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(outpath.c_str(), xsize, ysize, 1, dt, 0);
	//2017-12-03
	{//srs
		double geotrans[6] ;
		ads->GetGeoTransform(geotrans) ;
		outds->SetGeoTransform(geotrans) ;
		outds->SetProjection( ads->GetProjectionRef() ) ;
	}

	if (outds == 0)
	{
		cout << "Error : can not create " << outpath << endl;
		GDALClose(ads);
		GDALClose(bds);
		return 20;
	}

	double* abuff = new double[xsize];
	if (abuff == 0)
	{
		GDALClose(ads);
		GDALClose(bds);
		GDALClose(outds);
		cout << "Error : can not alloc memory . "<< endl;
		return 30;
	}
	double* bbuff = new double[xsize];
	if (bbuff == 0)
	{
		GDALClose(ads);
		GDALClose(bds);
		GDALClose(outds);
		delete[] abuff;
		cout << "Error : can not alloc memory . " << endl;
		return 31;
	}

	for (int iy = 0; iy < ysize; ++iy)
	{
		ads->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, xsize, 1, abuff, xsize, 1, GDT_Float64, 0, 0, 0);
		bds->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, xsize, 1, bbuff, xsize, 1, GDT_Float64, 0, 0, 0);
		for (int ix = 0; ix < xsize; ++ix)
		{
			if (abuff[ix] >= a0 && abuff[ix] <= a1 && bbuff[ix] >= b0 && bbuff[ix] <= b1)
			{
				double aval = abuff[ix] * ascale + aoffset;
				double bval = bbuff[ix] * bscale + boffset;
				aval = aval - bval;
				abuff[ix] = aval;
			}
			else {
				abuff[ix] = fill;
			}
		}
		outds->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, xsize, 1, abuff, xsize, 1, GDT_Float64, 0, 0, 0);
	}


	delete[] abuff;
	delete[] bbuff;
	GDALClose(ads);
	GDALClose(bds);
	GDALClose(outds);

	return 100;
}


void extractYmdInt(vector<string>& files, vector<int>& dtvec , int loc , int len )
{
	int num = files.size();
	for (int i = 0; i < num ; ++i)
	{
		string fname = wft_base_name(files[i]);
		string ymd = fname.substr(loc, len );
		int dt = wft_str2int(ymd);
		dtvec.push_back(dt);
	}
}




int main(int argc, char** argv)
{
	cout << "A program to use imageA - imageB .2017-12-01." << endl;
	cout << "V1.0  2017-12-01. by wangfeng1@piesat.cn" << endl;
	cout << "V1.1  2017-12-03. result has imageA srs." << endl;

	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "image_subtract startup.txt" << endl;

		cout << "*************** startup.txt example *************" << endl;
		cout << "#adir" << endl;
		cout << "D:/avhrr_ndvi/combinations/qcdaily/" << endl;
		cout << "#bdir" << endl;
		cout << "D:/avhrr-ndvi-qhc-max/" << endl;
		cout << "#aprefix" << endl;
		cout << "noaa-avhrr-ndvi-avh13c1-" << endl;
		cout << "#atail" << endl;
		cout << "-day.viqa.tif" << endl;
		cout << "#bprefix" << endl;
		cout << "qhc_day" << endl;
		cout << "#btail" << endl;
		cout << "_max_qa1.tif" << endl;
		cout << "#outdir" << endl;
		cout << "D:/avhrr_ndvi/daily-juping-qa1/" << endl;
		cout << "#outprefix" << endl;
		cout << "avhrr-day-jpmax-" << endl;
		cout << "#outtail" << endl;
		cout << ".tif" << endl;
		cout << "#a0" << endl;
		cout << "-2000" << endl;
		cout << "#a1" << endl;
		cout << "10000" << endl;
		cout << "#b0" << endl;
		cout << "-2000" << endl;
		cout << "#b1" << endl;
		cout << "10000" << endl;
		cout << "#ascale" << endl;
		cout << "1" << endl;
		cout << "#aoffset" << endl;
		cout << "0" << endl;
		cout << "#bscale" << endl;
		cout << "1" << endl;
		cout << "#boffset" << endl;
		cout << "0" << endl;
		cout << "#fill" << endl;
		cout << "-32768" << endl;
		cout << "#aymdloc" << endl;
		cout << "24" << endl;
		cout << "#adtloc" << endl;
		cout << "28" << endl;
		cout << "#bdtloc" << endl;
		cout << "17" << endl;
		cout << "#dtlen" << endl;
		cout << "4" << endl;
		cout << "" << endl;
		cout << "" << endl;
		cout << "" << endl;
		return 101;
	}
	string startup = argv[1];
	string adir = wft_getValueFromExtraParamsFile(startup, "#adir", true);
	string bdir = wft_getValueFromExtraParamsFile(startup, "#bdir", true);

	string aprefix = wft_getValueFromExtraParamsFile(startup, "#aprefix", true);
	string atail = wft_getValueFromExtraParamsFile(startup, "#atail", true);

	string bprefix = wft_getValueFromExtraParamsFile(startup, "#bprefix", true);
	string btail = wft_getValueFromExtraParamsFile(startup, "#btail", true);

	string outdir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);
	string outprefix = wft_getValueFromExtraParamsFile(startup, "#outprefix", true);
	string outtail = wft_getValueFromExtraParamsFile(startup, "#outtail", true);

	double a0, a1, b0, b1, ascale , aoffset , bscale , boffset , fill;
	string a0str = wft_getValueDoubleFromExtraParamsFile(startup, "#a0", a0, true);
	string a1str = wft_getValueDoubleFromExtraParamsFile(startup, "#a1", a1, true);

	string b0str = wft_getValueDoubleFromExtraParamsFile(startup, "#b0", b0, true);
	string b1str = wft_getValueDoubleFromExtraParamsFile(startup, "#b1", b1, true);

	string ascalestr = wft_getValueDoubleFromExtraParamsFile(startup, "#ascale", ascale, true);
	string aoffsetstr = wft_getValueDoubleFromExtraParamsFile(startup, "#aoffset", aoffset , true);

	string bscalestr = wft_getValueDoubleFromExtraParamsFile(startup, "#bscale", bscale, true);
	string boffsetstr = wft_getValueDoubleFromExtraParamsFile(startup, "#boffset", boffset, true);

	string fillstr = wft_getValueDoubleFromExtraParamsFile(startup, "#fill" , fill , true);

	int aloc, bloc , dtlen , aymdloc  ;
	string aymdlocstr = wft_getValueIntFromExtraParamsFile(startup, "#aymdloc", aymdloc, true);
	string alocstr = wft_getValueIntFromExtraParamsFile(startup, "#adtloc", aloc , true);
	string blocstr = wft_getValueIntFromExtraParamsFile(startup, "#bdtloc", bloc , true);
	string dtlenstr = wft_getValueIntFromExtraParamsFile(startup, "#dtlen", dtlen , true);

	int todayYmd = wft_current_dateymd_int();

	cout << "Today:" << todayYmd << endl;

	vector<string> afiles , bfiles ;
	wft_get_allSelectedFiles(adir, aprefix, atail, -1, "", afiles);
	wft_get_allSelectedFiles(bdir, bprefix, btail, -1, "", bfiles);

	vector<int> aymdvec, bymdvec;
	extractYmdInt(afiles, aymdvec, aloc , dtlen  );
	extractYmdInt(bfiles, bymdvec, bloc , dtlen );

	GDALAllRegister();

	int num = afiles.size();
	int bnum = bfiles.size();
	for (int i = 0; i < num; ++i)
	{
		int adt = aymdvec[i];
		for (int ib = 0; ib < bnum; ++ib)
		{
			if (bymdvec[ib] == adt)
			{
				string afilename = wft_base_name(afiles[i]);
				string ymdstr = afilename.substr(aymdloc, 8);
				string outname = outprefix + ymdstr + outtail;
				string outpath = outdir + outname;
				if (wft_test_file_exists(outpath) == false)
				{
					cout << "making " << outname << endl;
					processOne( afiles[i] , bfiles[ib] , outpath,
						a0 , a1 , b0 , b1 , 
						ascale , aoffset , bscale , boffset , 
						fill );
				}
			}
		}
	}

	return 0;
}



