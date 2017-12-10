// fy3bham_to_grid_monitor.cpp : ¶¨Òå¿ØÖÆÌ¨Ó¦ÓÃ³ÌÐòµÄÈë¿Úµã¡£
//
#include <iostream>
#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include "../sharedcodes/wftools.h"
using namespace std;
using std::cout;



// FY3B_VIRRX_VVHH_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS
std::string fyVertFenFuHaoArray[18] = { "80","70","60","50","40",
"30","20","10","00",
"90","A0","B0","C0","D0",
"E0","F0","G0","H0" };

std::string fyHoriFenFuHaoArray[36] = {
	"Z0","Y0","X0","W0","V0" ,
	"U0","T0","S0","R0","Q0",
	"P0","O0","N0","M0","L0",
	"K0","J0","I0",
	"00","10","20","30","40",
	"50","60","70","80","90",
	"A0","B0","C0","D0","E0",
	"F0","G0","H0" };



int newymdFromFy3bTileFileName(string filename, int ymdloc)
{
	string ymdL0 = filename.substr(ymdloc, 8);
	int ymdL1 = wft_str2int(ymdL0);
	int dayL = ymdL1 % 100;

	int newDay = 1;
	if (dayL < 11) newDay = 1;
	else if (dayL < 21) newDay = 11;
	else newDay = 21;

	int newymd = ymdL1 - dayL + newDay;
	cout << "convert fy3b ymd from " << ymdL1 << " to " << newymd << endl;
	return newymd;
}

int processOne( string onetilefilepath , int ymdloc , int vhloc ,
	string dsprefix , string dstail , string outpath  )
{
	//HDF5:"FY3B_VIRRX_80Q0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF" ://1KM_Monthly_NDVI
	//HDF5 : "FY3B_VIRRX_4080_L3_NVI_MLT_HAM_20170510_AOTD_1000M_MS.HDF" ://1KM_10day_NDVI
	// /root/data/fy3/ndvi/FY3B_VIRRX_8090_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS

	double rx = 0.05;
	double ry = 0.05;

	string tilefilename = wft_base_name(onetilefilepath);
	string oldymd = tilefilename.substr(ymdloc, 8);
	string vvhh = tilefilename.substr(vhloc, 4);
	int pos0 = onetilefilepath.rfind(vvhh)  ;
	int pos1 = onetilefilepath.rfind(oldymd);
	int len1 = pos1 - pos0 - 4;

	string part1 = onetilefilepath.substr(0, pos0);
	string part2 = onetilefilepath.substr(pos0 + 4, len1 );
	string part3 = onetilefilepath.substr(pos1 + 8);
	
	int ocean = -32750;
	int fill = -32768;
	int tileXSize = 1000;//fy3b
	int tileYSize = 1000;
	double fy3resolution = 1002.228;
	const double earthRadius = 6378137;
	const float scale = 0.0001f;
	const float offset = 0.f;

	vector<string> validDsPath;
	vector<int> vtileIndexVector;
	vector<int> htileIndexVector;
	for (int iv = 0; iv < 18; ++iv)
	{
		string vname = fyVertFenFuHaoArray[iv];
		for (int ih = 0; ih < 36; ++ih)
		{
			//HDF5:"FY3B_VIRRX_80Q0_L3_NVI_MLT_HAM_20170831_AOAM_1000M_MS.HDF" ://1KM_Monthly_NDVI
			//HDF5 : "FY3B_VIRRX_4080_L3_NVI_MLT_HAM_20170510_AOTD_1000M_MS.HDF" ://1KM_10day_NDVI
			string hname = fyHoriFenFuHaoArray[ih];
			string filepath = part1 + vname + hname + part2 + oldymd + part3 ;
			if (wft_test_file_exists(filepath))
			{
				string dspath = dsprefix + filepath + dstail;
				validDsPath.push_back(dspath);
				vtileIndexVector.push_back(iv);
				htileIndexVector.push_back(ih);
			}
			else {
				cout << "Warning can not open " << filepath << endl;
			}
		}
	}

	if (validDsPath.size() == 0) {
		cout << "Error : no valid fy3 file founded. out." << endl;
		return 10;
	}

	int outXSize = (int)(360 / rx);
	int outYSize = (int)(180 / ry);

	std::string sumFile = outpath + ".sum.tif";
	std::string cntFile = outpath + ".cnt.tif";
	std::string stdevFile = outpath + ".stdev.tif";
	//std::string ndviFile = outpath + ".ndvi.tif";
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outDs = driver->Create(outpath.c_str(), outXSize, outYSize, 1, GDT_Int16, 0);
	GDALDataset* sumDs = driver->Create(sumFile.c_str(), outXSize, outYSize, 1, GDT_Int32, 0);
	GDALDataset* cntDs = driver->Create(cntFile.c_str(), outXSize, outYSize, 1, GDT_Int16, 0);
	GDALDataset* stdevDs = driver->Create(stdevFile.c_str(), outXSize, outYSize, 1, GDT_Float32, 0);
	//GDALDataset* ndviDs = driver->Create(ndviFile.c_str(), outXSize, outYSize, 1, GDALDataType::GDT_Float32, nullptr);
	{
		//#include "ogr_spatialref.h"
		double adfGeoTrans[6] = { -180 , rx , 0 , 90 , 0 , -ry };
		OGRSpatialReference osrs;
		char* pszSRS_WKT = 0;
		outDs->SetGeoTransform(adfGeoTrans);
		cntDs->SetGeoTransform(adfGeoTrans);
		//ndviDs->SetGeoTransform(adfGeoTrans);
		stdevDs->SetGeoTransform(adfGeoTrans);
		osrs.SetWellKnownGeogCS("EPSG:4326");
		osrs.exportToWkt(&pszSRS_WKT);
		outDs->SetProjection(pszSRS_WKT);
		cntDs->SetProjection(pszSRS_WKT);
		//ndviDs->SetProjection(pszSRS_WKT);
		stdevDs->SetProjection(pszSRS_WKT);
		CPLFree(pszSRS_WKT);
	}

	int* tileBuffer = new int[tileXSize];
	for (size_t ids = 0; ids < validDsPath.size(); ++ids)
	{
		cout << "processing tile " << ids << "/" << validDsPath.size() << endl;
		GDALDataset* tileDs = (GDALDataset*)GDALOpen(validDsPath[ids].c_str(), GA_ReadOnly);
		if (tileDs == 0)
		{
			cout << "Error can not open " << validDsPath[ids] << endl;
		}
		else
		{
			double ffLeftBottomX0 = (htileIndexVector[ids] - 18) * tileXSize * fy3resolution + fy3resolution / 2;
			double ffLeftBottomY0 = (8 - vtileIndexVector[ids])* tileYSize * fy3resolution + fy3resolution / 2;
			for (int iy = 0; iy < tileYSize; ++iy)
			{
				tileDs->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, tileXSize, 1, tileBuffer, tileXSize, 1, GDT_Int32, 0, 0, 0);
				for (int ix = 0; ix < tileXSize; ++ix)
				{
					if (tileBuffer[ix] != fill)
					{
						double mapx = ffLeftBottomX0 + ix * fy3resolution;
						double mapy = ffLeftBottomY0 + (tileYSize - 1 - iy)*fy3resolution;
						double mapx2 = mapx / earthRadius;
						double mapy2 = mapy / earthRadius;
						//=SQRT(1-(O2/4)*(O2/4)-(O3/2)*(O3/2))
						double tempZ = sqrt(1.0 - (mapx2 / 4)*(mapx2 / 4) - (mapy2 / 2)*(mapy2 / 2));
						//= 2 * ATAN(O5*O2 / 2 / (2 * O5*O5 - 1)) * 180 / 3.14;
						double fy3lon = 2 * atan(tempZ*mapx2 / 2 / (2 * tempZ*tempZ - 1)) * 180 / M_PI;
						//=ASIN(O5*O3)*180/3.14
						double fy3lat = asin(tempZ*mapy2) * 180 / M_PI;
						int gridx = (fy3lon + 180.0) / rx;//
						int gridy = (90.0 - fy3lat) / ry;//
						if (gridx >= 0 && gridx < outXSize && gridy >= 0 && gridy < outYSize)
						{
							int ndviSum = 0;
							short ndviCnt = 0;
							sumDs->GetRasterBand(1)->RasterIO(GF_Read, gridx, gridy, 1, 1, &ndviSum, 1, 1, GDT_Int32, 0, 0, 0);
							cntDs->GetRasterBand(1)->RasterIO(GF_Read, gridx, gridy, 1, 1, &ndviCnt, 1, 1, GDT_Int16, 0, 0, 0);
							if (tileBuffer[ix] == ocean)
							{
								if (ndviCnt == 0)
								{
									ndviSum = tileBuffer[ix];
								}
							}
							else {
								float fndvi = tileBuffer[ix] * scale + offset;
								float fndvi2 = fndvi*fndvi;
								float fndvi2sum = 0;
								stdevDs->GetRasterBand(1)->RasterIO(GF_Read, gridx, gridy, 1, 1, &fndvi2sum, 1, 1, GDT_Float32, 0, 0, 0);
								if (ndviCnt == 0)
								{
									ndviSum = tileBuffer[ix];
									fndvi2sum = fndvi2;
								}
								else {
									ndviSum += tileBuffer[ix];
									fndvi2sum += fndvi2;
								}
								++ndviCnt;
								stdevDs->GetRasterBand(1)->RasterIO(GF_Write, gridx, gridy, 1, 1, &fndvi2sum, 1, 1, GDT_Float32, 0, 0, 0);
							}
							sumDs->GetRasterBand(1)->RasterIO(GF_Write, gridx, gridy, 1, 1, &ndviSum, 1, 1, GDT_Int32, 0, 0, 0);
							cntDs->GetRasterBand(1)->RasterIO(GF_Write, gridx, gridy, 1, 1, &ndviCnt, 1, 1, GDT_Int16, 0, 0, 0);
						}
						else {
							std::cout << "A exception grid xy found " << gridx << " " << gridy << std::endl;
						}
					}
				}
				wft_term_progress(iy, tileYSize);
			}
			GDALClose(tileDs);
		}
	}
	delete[] tileBuffer; tileBuffer = 0;

	//compute average.
	std::cout << "Averaging ... ..." << std::endl;
	int* outBuffer = new int[outXSize];
	int* outBuffer2 = new int[outXSize];
	//float* outBufferNdvi = new float[outXSize];
	float* outBufferStdev = new float[outXSize];
	for (int iy = 0; iy < outYSize; ++iy)
	{
		sumDs->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, outXSize, 1, outBuffer, outXSize, 1, GDT_Int32, 0, 0, 0);
		cntDs->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, outXSize, 1, outBuffer2, outXSize, 1, GDT_Int32, 0, 0, 0);
		stdevDs->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, outXSize, 1, outBufferStdev, outXSize, 1, GDT_Float32, 0, 0, 0);
		for (int ix = 0; ix < outXSize; ++ix)
		{
			if (outBuffer2[ix] > 0)
			{
				double sum2 = outBuffer[ix] * 1.0 * scale + offset;
				sum2 = sum2 * sum2;//Ñù±¾ºÍµÄÆ½·½

				double ndvi2Sum = outBufferStdev[ix];//Ñù±¾Æ½·½µÄºÍ
				double stdev = sqrt(outBuffer2[ix] * ndvi2Sum - sum2) / outBuffer2[ix];

				outBuffer[ix] = outBuffer[ix] / outBuffer2[ix];
				//outBufferNdvi[ix] = outBuffer[ix] * scale + offset;
				outBufferStdev[ix] = (float)stdev;
			}
			else {
				//outBufferNdvi[ix] = -9;
				outBufferStdev[ix] = 0;
				if (outBuffer[ix] != ocean)
				{
					outBuffer[ix] = fill;
				}
			}
		}
		outDs->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, outXSize, 1, outBuffer, outXSize, 1, GDT_Int32, 0, 0, 0);
		//ndviDs->GetRasterBand(1)->RasterIO(GDALRWFlag::GF_Write, 0, iy, outXSize, 1, outBufferNdvi, outXSize, 1, GDALDataType::GDT_Float32, 0, 0, 0);
		stdevDs->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, outXSize, 1, outBufferStdev, outXSize, 1, GDT_Float32, 0, 0, 0);
		wft_term_progress(iy, outYSize);
	}

	delete[] outBuffer; outBuffer = 0;
	delete[] outBuffer2; outBuffer2 = 0;
	//delete[] outBufferNdvi; outBufferNdvi = 0;
	delete[] outBufferStdev; outBufferStdev = 0;

	GDALClose(cntDs);
	GDALClose(outDs);
	GDALClose(sumDs);
	//GDALClose(ndviDs);
	GDALClose(stdevDs);

	wft_remove_file(sumFile);

	return 100;
}

int main(int argc , char** argv )
{
	cout << "A program to convert fy3b ndvi into avhrr 0.05 grid.2017-12-02." << endl;
	cout << "V1.0  by wangfeng1@piesat.cn" << endl;

	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "fy3bham_to_grid_monitor startup.txt" << endl;

		cout << "*************** startup.txt example *************" << endl;
		cout<<"#indir"<<endl;
		cout<<"E:/testdata/fy3bvirrndvi/2017/"<<endl;
		cout<<"#outdir"<<endl;
		cout<<"d:/"<<endl;
		cout<<"#infixprefix"<<endl;
		cout<<"FY3B_VIRRX_"<<endl;
		cout<<"#infixtail"<<endl;
		cout<<"_AOTD_1000M_MS.HDF"<<endl;
		cout<<"#dsprefix"<<endl;
		cout<<"HDF5:\""<<endl;
		cout<<"#dstail"<<endl;
		cout<<"\"://1KM_10day_NDVI"<<endl;
		cout<<"#outprefix"<<endl;
		cout<<"fy3b_virrx_ndvi_"<<endl;
		cout<<"#outtail"<<endl;
		cout<<"_ten_grid005.tif"<<endl;
		cout<<"#ymdloc"<<endl;
		cout<<"31"<<endl;
		cout<<"#vhloc"<<endl;
		cout<<"11"<<endl;
		return 101;
	}
	string startup = argv[1];
	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	string outdir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);

	string inprefix = wft_getValueFromExtraParamsFile(startup, "#infixprefix", true);
	string intail = wft_getValueFromExtraParamsFile(startup, "#infixtail", true);

	string dsprefix = wft_getValueFromExtraParamsFile(startup, "#dsprefix", true);
	string dstail = wft_getValueFromExtraParamsFile(startup, "#dstail", true);

	string outprefix = wft_getValueFromExtraParamsFile(startup, "#outprefix", true);
	string outtail = wft_getValueFromExtraParamsFile(startup, "#outtail", true);

	int ymdloc, vhloc;
	string ymdlocstr = wft_getValueIntFromExtraParamsFile(startup, "#ymdloc", ymdloc , true);
	string vhlocstr = wft_getValueIntFromExtraParamsFile(startup, "#vhloc", vhloc ,  true);

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
		int newymd = newymdFromFy3bTileFileName(filename, ymdloc);
		string outname = outprefix + wft_int2str(newymd) + outtail ;
		string outpath = outdir + outname;
		if (wft_test_file_exists(outpath) == false)
		{
			cout << "making " << outname << endl;
			processOne(filepath, ymdloc, vhloc, dsprefix, dstail, outpath);
		}
	}

	return 0;

}

