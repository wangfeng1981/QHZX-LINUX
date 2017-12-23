// snow_combination.cpp : 定义控制台应用程序的入口点。
//
//不考虑中国区域单独投影了，统一使用欧亚大陆的albers投影。2017-12-15.


#include <iostream>
#include "ogr_spatialref.h"//17
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include "../sharedcodes/wftools.h"
#include "../sharedcodes/ajson5.h"
#include "../sharedcodes/wdb.h"

using namespace std;
using std::cout;

#define MJZS_DAY 0
#define MJZS_MON 1
#define MJZS_SEA 2
#define MJZS_YEA 3

string g_gdaltranslate = "gdal_translate";
string g_gdalwarp = "gdalwarp";

string g_txt = "";
string g_tem_day_asia = "";
string g_tem_day_china = "";
string g_tem_daycnt_asia = "";
string g_tem_daycnt_china = "";
string g_gnuplot = "";
string g_shengmask = "";
string g_threemask = "";
double g_pixelarea = 16;//4km*4km

string g_pid_fy4_day = "";
string g_pid_fy4_mon = "";
string g_pid_fy4_sea = "";
string g_pid_fy4_yea = "";

string g_pid_rnh_day = "";
string g_pid_rnh_mon = "";
string g_pid_rnh_sea = "";
string g_pid_rnh_yea = "";

string g_pid_ims_day = "";
string g_pid_ims_mon = "";
string g_pid_ims_sea = "";
string g_pid_ims_yea = "";


string g_shp_oyalbers_china = "";

//pname
string g_fy4_pname = "";
string g_rh_pname = "";
string g_ims_pname = "";

//mianjizhishu pid
string g_ims_pid_mjzs_day = "";
string g_ims_pid_mjzs_mon = "";

string g_fy4mjzs[4] ;
string g_rnhmjzs[4] ;
string g_imsmjzs[4] ;

int g_from_ymd = 0 ;
int g_to_ymd = 0 ;

//xspace yspace
string g_xspace = "1";
string g_yspace = "1";


//mysql operation
wDb g_db ; 


////////////////////////////////snow fusion//////////////////////////////////////////
double g_A = 2 * 6371.228 / 25;
double g_B = 3.1415926 / 4;
double g_C = 3.1415926 / 180;

string g_fusion_fy4lonfile = "" ;
string g_fusion_fy4latfile = "" ;
string g_fusion_fy4lonfile9 = "" ;
string g_fusion_fy4latfile9 = "" ;

struct StaSnow
{
	float lon, lat;
	bool snow;
	int next;
};

//70cols*40rows
short g_station2dgrid[2800];

void loadStationSnow(string filepath, int ymd , vector<StaSnow>& vec )
{
	for (int i = 0; i < 2800; ++i) g_station2dgrid[i] = -1;
	int year0 = ymd / 10000;
	int month0 = (ymd % 10000) / 100;
	int day0 = ymd % 100;

	ifstream  ifs(filepath.c_str());
	std::string line;
	std::getline(ifs, line); //desc line.
	int sid, y, m, d;
	float lon, lat, sd;
	int npp = 0;
	while (std::getline(ifs, line))
	{
		++npp;
		if (npp == 10000)
		{
			cout << ".";
			npp = 0;
		}
		std::istringstream iss(line);
		if (!(iss >> sid >> lat >> lon >> y >> m >> d >> sd))
		{
			//wrong data
		}
		else
		{
			if (year0 == y && month0 == m && day0 == d)
			{
				StaSnow ss;
				ss.next = -1;
				ss.lat = lat;
				ss.lon = lon;
				if (sd > 0.01f && sd < 30000 )
				{
					ss.snow = true;
				}
				else
				{
					ss.snow = false;
				}
				int gridcol = (ss.lon - 70);
				int gridrow = (ss.lat - 15);
				int gi = gridrow * 70 + gridcol;
				int vi = g_station2dgrid[gi];
				if ( vi < 0)
				{
					g_station2dgrid[gi] = vec.size();
				}
				else
				{
					while (vec[vi].next >= 0)
					{
						vi = vec[vi].next;
					}
					vec[vi].next = vec.size();
				}
				vec.push_back(ss);
			}
		}
	}
	cout<<"load station finished. "<< endl;
	ifs.close();
}


bool convertlonlat2fy3colrow(double lon, double lat, int& col, int& row)
{
	if (lat >= 0)
	{
		double colv2 = g_A * sinf(lon*g_C)*sinf(g_B - lat / 2 * g_C) + 360;
		double rowv2 = g_A * cosf(lon*g_C)*sinf(g_B - lat / 2 * g_C) + 360;
		col = ceil(colv2);
		row = ceil(rowv2);
	}
	else {
		double colSouthv2 = g_A * sinf(lon*g_C)*cosf(g_B - lat / 2 * g_C) + 360;
		double rowSouthv2 = -g_A * cosf(lon*g_C)*cosf(g_B - lat / 2 * g_C) + 360;
		col = ceil(colSouthv2);
		row = ceil(rowSouthv2);
	}
	if (col < 0 || col > 720 || row < 0 || row > 720)
	{
		return false;
	}
	else {
		return true;
	}
}

//-1 not found , 0 -nosnow , 1-snow
int stationHasSnow(float lon, float lat , vector<StaSnow>& ssvec )
{
	if (ssvec.size() == 0) return false;
	int n = ssvec.size();
	float mindist = 99999 ;
	bool hassnow = false;
	float lon1, lat1;

	int gridcol = lon - 70;
	int gridrow = lat - 15;

	int col0 = MAX(gridcol-1, 0);
	int col1 = MIN(gridcol + 1, 69);

	int row0 = MAX(gridrow - 1, 0);
	int row1 = MIN(gridrow + 1, 39);

	for (int irow = row0; irow <= row1; ++irow)
	{
		for (int icol = col0; icol <= col1; ++icol)
		{
			int igrid = irow * 70 + icol;
			int vi = g_station2dgrid[igrid];
			while ( vi >= 0 )
			{
				StaSnow ss = ssvec[vi];
				float a = ss.lon - lon;
				float b = ss.lat - lat;
				float d = a * a + b * b;
				if (d < mindist)
				{
					lon1 = ss.lon;
					lat1 = ss.lat;
					mindist = d;
					hassnow = ss.snow;
				}
				vi = ss.next;
			}
		}
	}
	if (mindist < 1)
	{
		if (hassnow) return 1;
		else return 0;
	}
	else
	{
		return -1;
	}
}


int processOneFileV3(
	string fy4file, 
	string fy4lonfile, 
	string fy4latfile,
	string fy3file,
	string stationfile ,  
	string outputfile , 
	int ymd 
)
{
	vector<StaSnow> ssvec;
	//
	if (stationfile != "" )
	{
		cout << "Load station snows ... " << endl;
		loadStationSnow(stationfile, ymd, ssvec);
		cout << "station snow records num:" << ssvec.size() << endl;
	}
	else
	{
		cout << "no station snow files." << endl;
	}
	

	//fy4 data
	GDALDataset* fy4ds = (GDALDataset*)GDALOpen(fy4file.c_str(), GA_ReadOnly);
	GDALDataset* fy4londs = (GDALDataset*)GDALOpen(fy4lonfile.c_str(), GA_ReadOnly);
	GDALDataset* fy4latds = (GDALDataset*)GDALOpen(fy4latfile.c_str(), GA_ReadOnly);
	const int fy4XSize = fy4ds->GetRasterXSize();
	const int fy4YSize = fy4ds->GetRasterYSize();

	//out data
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(outputfile.c_str(), fy4XSize, fy4YSize, 1, GDT_Byte, 0);

	//fy3 snow depth data
	const int fy3size = 721 * 721;
	short* fy3northArr = new short[fy3size];
	short* fy3southArr = new short[fy3size];
	{
		short* fy3north0 = new short[fy3size];
		short* fy3north1 = new short[fy3size];
		short* fy3south0 = new short[fy3size];
		short* fy3south1 = new short[fy3size];
		string fy3northfile = "HDF5:\"" + fy3file + "\"://SD_Northern_Daily";
		string fy3southfile = "HDF5:\"" + fy3file + "\"://SD_Southern_Daily";
		GDALDataset* fy3nds = (GDALDataset*)GDALOpen(fy3northfile.c_str(), GA_ReadOnly);
		GDALDataset* fy3sds = (GDALDataset*)GDALOpen(fy3southfile.c_str(), GA_ReadOnly);
		fy3nds->GetRasterBand(1)->RasterIO(GF_Read,
			0, 0, 721, 721, fy3north0, 721, 721, GDT_Int16, 0, 0, 0);
		fy3nds->GetRasterBand(2)->RasterIO(GF_Read,
			0, 0, 721, 721, fy3north1, 721, 721, GDT_Int16, 0, 0, 0);

		fy3sds->GetRasterBand(1)->RasterIO(GF_Read,
			0, 0, 721, 721, fy3south0, 721, 721, GDT_Int16, 0, 0, 0);
		fy3sds->GetRasterBand(2)->RasterIO(GF_Read,
			0, 0, 721, 721, fy3south1, 721, 721, GDT_Int16, 0, 0, 0);
		GDALClose(fy3nds);
		GDALClose(fy3sds);
		//计算上轨和下轨的平均值
		for (int i = 0; i < fy3size; ++i)
		{
			if (0 <= fy3north0[i] && fy3north0[i] <= 1000 &&
				0 <= fy3north0[i] && fy3north0[i] <= 1000
				)
			{
				fy3northArr[i] = (short)round(fy3north0[i] / 2.0 + fy3north1[i] / 2.0);
			}
			else if (0 <= fy3north0[i] && fy3north0[i] <= 1000) {
				fy3northArr[i] = fy3north0[i];
			}
			else {
				fy3northArr[i] = fy3north1[i];
			}

			if (0 <= fy3south0[i] && fy3south0[i] <= 1000 &&
				0 <= fy3south1[i] && fy3south1[i] <= 1000
				)
			{
				fy3southArr[i] = (short)round(fy3south0[i] / 2.0 + fy3south1[i] / 2.0);
			}
			else if (0 <= fy3south0[i] && fy3south0[i] <= 1000) {
				fy3southArr[i] = fy3south0[i];
			}
			else {
				fy3southArr[i] = fy3south1[i];
			}
		}
		delete[] fy3north0;
		delete[] fy3north1;
		delete[] fy3south0;
		delete[] fy3south1;
	}

	int asize = fy4XSize * fy4YSize;
	short* fy4buffer = new short[asize];
	float* fy4lonbuffer = new float[asize];
	float* fy4latbuffer = new float[asize];

	fy4ds->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, fy4XSize, fy4YSize,
		fy4buffer, fy4XSize, fy4YSize, GDT_Int16, 0, 0, 0);
	fy4londs->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, fy4XSize, fy4YSize,
		fy4lonbuffer, fy4XSize, fy4YSize, GDT_Float32, 0, 0, 0);
	fy4latds->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, fy4XSize, fy4YSize,
		fy4latbuffer, fy4XSize, fy4YSize, GDT_Float32, 0, 0, 0);

	GDALClose(fy4londs);
	GDALClose(fy4latds);
	GDALClose(fy4ds);


	for (int it = 0; it < asize; ++it )
	{
			float f4lon = fy4lonbuffer[it];
			float f4lat = fy4latbuffer[it];

			if (f4lon < -181 || f4lon > 361)
			{
				fy4buffer[0] = 1;
				continue;
			}
			if (f4lat < -91 || f4lat > 91)
			{
				fy4buffer[0] = 1;
				continue;
			}

			if (f4lon > 180.f)
			{
				f4lon = f4lon - 360.f;//bugfixed.
			}

			//FY4为雪的时候 snow : fy4 default 200, baizhaofeng 10
			if (fy4buffer[it] == 6)
			{//cloud
				if (f4lat >= 0)
				{//北半球
					int fy3col, fy3row;
					bool find = convertlonlat2fy3colrow(f4lon, f4lat, fy3col, fy3row);
					if (find)
					{
						int ify3 = fy3row * 721 + fy3col;
						short fy3val = fy3northArr[ify3];
						if (fy3val <= 10)
						{
							//风四判别为云，风三判别为无雪，那么进行站点判断
							int ss1 = stationHasSnow(f4lon, f4lat, ssvec);
							if ( ss1 == 1 )
							{
								fy4buffer[it] = 10 ;
							}
							else if( ss1 == 0 )
							{
								fy4buffer[it] = 8;
							}
							else
							{
								fy4buffer[it] = 8;
							}
						}
						else if (fy3val > 10 && fy3val <= 1000) {
							fy4buffer[it] = 10;
						}
						else {
							//nochange
						}
					}
					else {
						//FY4有云，但是FY3没有定位到有效像元 no change
					}
				}
				else
				{//南半球
					int fy3col, fy3row;
					bool find = convertlonlat2fy3colrow(f4lon, f4lat, fy3col, fy3row);
					if (find)
					{
						int ify3 = fy3row * 721 + fy3col;
						short fy3val = fy3southArr[ify3];//bugfixed
						if (fy3val <= 10)
						{
							//风四判别为云，风三判别为无雪，那么进行站点判断
							int ss1 = stationHasSnow(f4lon, f4lat, ssvec);
							if (ss1 == 1)
							{
								fy4buffer[it] = 10;
							}
							else if (ss1 == 0)
							{
								fy4buffer[it] = 8;
							}
							else
							{
								fy4buffer[it] = 8;
							}
						}
						else if (fy3val > 10 && fy3val <= 1000) {
							fy4buffer[it] = 10;
						}
						else {
							//nochange
						}
					}
					else {
						//FY4有云，但是FY3没有定位到有效像元 no change 
					}
				}
			}
		wft_term_progress(it, asize);
	}
	outds->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, fy4XSize, fy4YSize,
		fy4buffer, fy4XSize, fy4YSize, GDT_Int16, 0, 0, 0);

	delete[] fy4buffer; fy4buffer = 0;
	delete[] fy4lonbuffer; fy4lonbuffer = 0;
	delete[] fy4latbuffer; fy4latbuffer = 0;

	delete[] fy3northArr; fy3northArr = 0;
	delete[] fy3southArr; fy3southArr = 0;

	GDALClose(outds);


	return 100;
}


void snowfusionProcess( string fy4l2v2daydir , int ymdloc , int lonloc , string outdir , string fy3dir, string stationSnowfile
 )
{
	vector<string> allfy4files ;
	wft_get_allSelectedFiles(fy4l2v2daydir, "fy4a_snc_l2v2_", ".day.tif", -1, "", allfy4files);
	for (int i = 0; i < allfy4files.size(); ++i)
	{
		string filepath = allfy4files[i];
		string filename = wft_base_name(filepath);

		//FY3C_MWRIX_GBAL_L2_SWE_MLT_ESD_20170204_POAD_025KM_MS.HDF
		string ymd = filename.substr(ymdloc, 8);
		string lonstr = filename.substr(lonloc, 4);
		string fy3name = string("FY3C_MWRIX_GBAL_L2_SWE_MLT_ESD_") + ymd + "_POAD_025KM_MS.HDF";
		string fy3path = fy3dir + fy3name;
		if (wft_test_file_exists(fy3path))
		{
			string outfilename = "fy4a_rnh_v03_"+lonstr + "_" + ymd + ".day.tif";
			string outpath = outdir + outfilename;
			if (wft_test_file_exists(outpath) == false )
			{
				cout << "find pair : " << filepath << " , " << fy3path << endl;
				string lonfile = g_fusion_fy4lonfile;
				string latfile = g_fusion_fy4latfile ;
				if (lonstr != "1047")
				{
					lonfile = g_fusion_fy4lonfile9;
					latfile = g_fusion_fy4latfile9;
				}
				int res = processOneFileV3(filepath, lonfile, latfile, fy3path , stationSnowfile , outpath , wft_str2int(ymd) );
				if (res == 100)
				{
					cout<<"snow fusion ok "<<outpath<<endl ;
				}else 
				{
					cout<<"Failed to make fusion "<<outpath<<endl ;
				}
			}
		}
	}
}


	













void extractFilesYmd(vector<string>& pathvec, vector<int>& ymdvec , int ymdloc )
{
	int n = pathvec.size();
	for (int i = 0; i < n; ++i)
	{
		string filepath = pathvec[i];
		string filename = wft_base_name(filepath);
		string ymdstr = filename.substr(ymdloc, 8);
		ymdvec.push_back(wft_str2int(ymdstr));
	}
}

void printCurrentTime() {
	//#include <ctime>
	time_t time0;
	time(&time0);
	std::cout << asctime(localtime(&time0)) << std::endl;
}

void sepYmd(int ymd, int* ymdarr)
{
	ymdarr[0] = ymd / 10000;
	ymdarr[1] = (ymd % 10000 )/ 100;
	ymdarr[2] = ymd % 100;
}

void getfivdaysStartEnd(int ymdi, int & ymd0, int& ymd1)
{
	int a[3];
	sepYmd(ymdi, a);
	if (a[2] < 6)
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 1;
		ymd1 = a[0] * 10000 + a[1] * 100 + 5;
	}else if (a[2] < 11)
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 6;
		ymd1 = a[0] * 10000 + a[1] * 100 + 10;
	}
	else if (a[2] < 16)
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 11;
		ymd1 = a[0] * 10000 + a[1] * 100 + 15;
	}
	else if (a[2] < 21 )
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 16;
		ymd1 = a[0] * 10000 + a[1] * 100 + 20;
	}
	else if (a[2] < 26)
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 21;
		ymd1 = a[0] * 10000 + a[1] * 100 + 25;
	}
	else 
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 26;
		ymd1 = a[0] * 10000 + a[1] * 100 + 31;
	}
}

void gettendaysStartEnd(int ymdi, int & ymd0, int& ymd1)
{
	int a[3];
	sepYmd(ymdi, a);
	if (a[2] < 11)
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 1;
		ymd1 = a[0] * 10000 + a[1] * 100 + 10;
	}
	else if (a[2] < 21)
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 11;
		ymd1 = a[0] * 10000 + a[1] * 100 + 20;
	}
	else
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 21;
		ymd1 = a[0] * 10000 + a[1] * 100 + 31;
	}
}

void getmonStartEnd(int ymdi, int & ymd0, int& ymd1)
{
	int a[3];
	sepYmd(ymdi, a);
	ymd0 = a[0] * 10000 + a[1] * 100 + 1;
	ymd1 = a[0] * 10000 + a[1] * 100 + 31;
}

void getseaStartEnd(int ymdi, int & ymd0, int& ymd1)
{
	int a[3];
	sepYmd(ymdi, a);
	if (a[1] < 3)
	{
		ymd0 = (a[0]-1) * 10000 + 1201;
		ymd1 = a[0] * 10000 + 229;
	}
	else if (a[1] < 6)
	{
		ymd0 = a[0] * 10000 + 301;
		ymd1 = a[0] * 10000 + 531;
	}
	else if (a[1] < 9)
	{
		ymd0 = a[0] * 10000 + 601;
		ymd1 = a[0] * 10000 + 831;
	}
	else if (a[1] < 12)
	{
		ymd0 = a[0] * 10000 + 901 ;
		ymd1 = a[0] * 10000 + 1130;
	}
	else {
		ymd0 = a[0] * 10000 + 1201;
		ymd1 = (a[0]+1) * 10000 + 229;
	}
}

void getyeaStartEnd(int ymdi, int & ymd0, int& ymd1)
{
	int a[3];
	sepYmd(ymdi, a);
	ymd0 = a[0] * 10000 + 101 ;
	ymd1 = a[0] * 10000 + 1231;
}

struct AreaIndex
{
	inline AreaIndex() :maskid(0), sumval(0) {};
	int maskid;
	double sumval;
};


//计算积雪面积指数
int computeAreaIndexManyDays(string maskfile, string snonwfile ,
	int ndays , vector<AreaIndex>& result )
{
	double areaArr[255]; 
	short hasmask[255];
	memset(hasmask, 0, sizeof(short) * 255);
	memset(areaArr, 0, sizeof(double) * 255);
	GDALDataset* maskds = (GDALDataset*)GDALOpen(maskfile.c_str(), GA_ReadOnly);
	if (maskds == 0)
	{
		cout << "error : failed to open mask file " << maskfile << endl;
		return 10 ;
	} 
	int x0 = maskds->GetRasterXSize();
	int y0 = maskds->GetRasterYSize();
	GDALDataset* snowds = (GDALDataset*)GDALOpen(snonwfile.c_str(), GA_ReadOnly);
	if (snowds == 0)
	{
		cout << "error : failed to open snow file " << snonwfile << endl;
		GDALClose(maskds);
		return 20;
	} 
	int x1 = snowds->GetRasterXSize();
	int y1 = snowds->GetRasterYSize();

	if (x0 != x1 || y0 != y1)
	{
		cout << "error : snow and mask size are different ." << x0 << "," << y0 << ";" << x1 << "," << y1 << endl;
		GDALClose(maskds);
		GDALClose(snowds);
		return 30;
	}
	int bsize = x0 * y0;
	char* maskbuffer = new char[bsize];
	short* snowbuffer = new short[bsize];
	if (maskbuffer == 0 || snowbuffer == 0)
	{
		cout << "error : can not alloc memory." << endl;
		GDALClose(maskds);
		GDALClose(snowds);
		if (maskbuffer) delete[] maskbuffer;
		if (snowbuffer) delete[] snowbuffer;
		return 40;
	}
	maskds->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, x0, y0, maskbuffer, x0, y0, GDT_Byte, 0, 0, 0);
	snowds->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, x0, y0, snowbuffer, x0, y0, GDT_Int16, 0, 0, 0);
	GDALClose(maskds);
	GDALClose(snowds);

	if (ndays == 1)
	{
		for (int it = 0; it < bsize; ++it)
		{
			int maskid = (int)maskbuffer[it];
			hasmask[maskid] = 1;
			if (maskid > 0 && snowbuffer[it] == 10 )
			{
				areaArr[maskid] += g_pixelarea;
			}
		}
	}
	else {
		for (int it = 0; it < bsize; ++it)
		{
			int maskid = (int)maskbuffer[it];
			hasmask[maskid] = 1;
			if (maskid > 0)
			{
				areaArr[maskid] += g_pixelarea * snowbuffer[it] ;
			}
		}
	}


	delete[] maskbuffer;
	delete[] snowbuffer;

	for (int i = 1; i < 255; ++i)
	{
		if (areaArr[i] > 0.001)
		{
			AreaIndex ai;
			ai.maskid = i;
			ai.sumval = areaArr[i] / ndays ;
			result.push_back(ai);
		}
		else if (hasmask[i] > 0)
		{
			AreaIndex ai;
			ai.maskid = i;
			ai.sumval = 0 ;
			result.push_back(ai);
		}
	}
	return 100;
}

AreaIndex computeSumOfAreaIndex(vector<AreaIndex>& vec, int newmaskid )
{
	double sum = 0;
	for (int i = 0; i < vec.size(); ++i)
	{
		sum += vec[i].sumval;
	}
	AreaIndex ai;
	ai.maskid = newmaskid;
	ai.sumval = sum;
	return ai;
}


//保存积雪面积指数
void saveSnowAreaIndex(string filepath, string pid, vector<AreaIndex>& vec , int ymd8)
{
	ofstream ofs(filepath.c_str());
	for (int i = 0; i < vec.size(); ++i)
	{
		ofs << pid << "\t" << vec[i].maskid << "\t" << vec[i].sumval <<"\t"<<ymd8 << endl;
		g_db.insertxl( "tb_xulie_data" , 
			pid , 
			vec[i].maskid , 
			vec[i].sumval , 
			ymd8 ) ;
	}
	ofs.close();
}



void makeCinfofile(string outpath, int combcnt)
{
	char buff[10];
	sprintf(buff, "c%04d", combcnt);
	string outpath2 = outpath + "." + string(buff) + ".txt" ;
	ofstream ofs(outpath2.c_str());
	ofs << combcnt << endl;
	ofs.close();
}

void dowarpPlotDb(string inpath , double lon , int ymd0 , int ymd1 , string asiatem , string chinatem , 
	string asiapid , string chinapid , string mjzspid , bool wantChina 
	, string pname )
{
	string albersfile = inpath ;
	if (lon > -900)
	{
		cout << "converting wgs84 -> albers..." << endl;
		albersfile = inpath + ".asia.albers.tif";
		//translate geos
		string geosfile = inpath + ".geos.tif";
		char lonbuff[20];
		sprintf(lonbuff, "%.1f", lon);
		string lonstr(lonbuff);
		string cmd1 = g_gdaltranslate
			+ "  -a_srs \"+proj=geos +h=35785863 +a=6378137.0 +b=6356752.3 +lon_0=" + lonstr
			+ " +no_defs\" -a_ullr -5496000 5496000 5496000 -5496000 " + inpath
			+ " " + geosfile;
		cout << cmd1 << endl;
		system(cmd1.c_str());

		//wgs84
		string wgs84file = inpath + ".wgs84.tif";
		string cmd2 = g_gdalwarp + " -overwrite -t_srs \"+proj=latlong +ellps=WGS84 +pm=0\" -tr 0.04 0.04 "
			+ " -te 20 -0 180 85 " + geosfile + " " + wgs84file;
		cout << cmd2 << endl;
		system(cmd2.c_str());

		wft_remove_file(geosfile);

		//albers 亚洲 +lon_0=95 +lat_1=15 +lat_2=65 
		//gdalwarp -overwrite -t_srs "+proj=aea +ellps=krass +lon_0=105 +lat_1=25 +lat_2=47" 
		//-srcnodata -9999 -dstnodata -9999 ndvichina.tif alberschina.tif 
		string cmd3 = g_gdalwarp + " -overwrite -t_srs \"+proj=aea +ellps=krass +lon_0=95  +lat_1=15 +lat_2=65\" -tr 4000 4000 "
			+ wgs84file + " " + albersfile;
		cout << cmd3 << endl;
		system(cmd3.c_str());
	}
	else {
		cout << "useing inpath (albers) directly..." << endl;
	}

	string ymdstr = wft_convert_ymdi2y_m_d(ymd0);
	string ymdstr1 = wft_convert_ymdi2y_m_d(ymd1);
	if( g_txt != "" && g_tem_day_asia!= "" )
	{
		cout << "ploting euro-asia..." << endl;
		string asiatxtfile = albersfile + ".tmp";

		string cmdtxt1 = g_txt + " -in " + albersfile + " -out " + asiatxtfile + " -valid0 1 -valid1 10000 -xspace "
			+ g_xspace +  " -yspace " + g_yspace ;
		system(cmdtxt1.c_str());
		vector<string> vec1, vec2;
		vec1.push_back("{{{INFILE}}}");
		vec1.push_back("{{{OUTFILE}}}");
		vec1.push_back("{{{YMD}}}");
		vec1.push_back("{{{YMD1}}}");
		vec1.push_back("{{{PNAME}}}");

		string png1file = albersfile + ".ea.png";
		vec2.push_back(asiatxtfile);
		vec2.push_back(png1file);
		vec2.push_back(ymdstr);
		vec2.push_back(ymdstr1);
		vec2.push_back(pname);

		string plot1file = png1file + ".plot";
		wft_create_file_by_template_with_replacement(plot1file, asiatem , vec1, vec2);

		string cmdplot1 = g_gnuplot + " " + plot1file;
		system(cmdplot1.c_str());

		if (asiapid !=  "" )
		{
			//insert db
			string fname = wft_base_name(albersfile) ;
			g_db.insertimg("tb_product_data" ,
				asiapid , 
				albersfile , 
				fname , 
				png1file , 
				ymd0
				) ;
		}

	}
	if (g_txt != "" && g_tem_day_china != "" && wantChina )
	{
		cout << "ploting china..." << endl;
		string cutchinafile = inpath + ".cn.tif";
		//shapecut
		string cmdcut = g_gdalwarp + " -cutline " + g_shp_oyalbers_china + " -crop_to_cutline -tr 4000 4000 -of GTiff " + albersfile + " " + cutchinafile;
		cout << cmdcut << endl;
		system(cmdcut.c_str());

		string chinatxtfile = cutchinafile + ".tmp";
		string cmdtxt1 = g_txt + " -in " + cutchinafile + " -out " + chinatxtfile + " -valid0 1 -valid1 10000 -xspace "+g_xspace
			+" -yspace  " + g_yspace 
			+ " -x0 -1900000 -x1 2800000 -y0 1970000 -y1 6570000 " ;//debug
		system(cmdtxt1.c_str());
		vector<string> vec1, vec2;
		vec1.push_back("{{{INFILE}}}");
		vec1.push_back("{{{OUTFILE}}}");
		vec1.push_back("{{{YMD}}}");
		vec1.push_back("{{{YMD1}}}");
		vec1.push_back("{{{PNAME}}}");

		string png1file = cutchinafile + ".png";
		vec2.push_back(chinatxtfile);
		vec2.push_back(png1file);
		vec2.push_back(ymdstr);
		vec2.push_back(ymdstr1);
		vec2.push_back(pname);

		string plot1file = png1file + ".plot";
		wft_create_file_by_template_with_replacement(plot1file, chinatem, vec1, vec2);

		string cmdplot1 = g_gnuplot + " " + plot1file;
		system(cmdplot1.c_str());

		if (chinapid  != "")
		{
			//insert db
			string fname = wft_base_name(cutchinafile) ;
			g_db.insertimg("tb_product_data" ,
				//bug20171219-0757 asiapid , 
				chinapid , //bugfixed 20171219-0757
				cutchinafile , 
				fname , 
				png1file , 
				ymd0
				) ;
		}
	}

	//计算积雪面积指数
	if (mjzspid != "")
	{
		int ndays = wft_get_days_between(ymd0, ymd1);
		vector<AreaIndex> snowAreaIndices;
		if (g_shengmask != "")
		{
			computeAreaIndexManyDays(g_shengmask, albersfile, ndays, snowAreaIndices);//计算各省面积指数
			AreaIndex cnAi = computeSumOfAreaIndex(snowAreaIndices, 1);
			snowAreaIndices.push_back(cnAi);
		}
		if (g_threemask != "")
		{
			computeAreaIndexManyDays(g_threemask, albersfile, ndays, snowAreaIndices);//计算三大区面积指数
		}
		//积雪面积指数入库或者保存到文件
		string mjzsfile = albersfile + ".mjzs.txt";
		saveSnowAreaIndex(mjzsfile, mjzspid, snowAreaIndices, ymd0 );
	}
	
}

int selectedFilesByYmdRange(vector<string>& allfiles, vector<int>& ymdVec, vector<string>& outfiles, int ymd0, int ymd1)
{
	int num = ymdVec.size();
	int cnt = 0;
	for (int i = 0; i < num; ++i)
	{
		int cymd = ymdVec[i];
		if (cymd >= ymd0 && cymd <= ymd1)
		{
			outfiles.push_back(allfiles[i]);
			++cnt;
		}
	}
	return cnt;
}


//for fy4 l2 snc product 0300-0600 combined into daily.
int processOneFy4_3_6_to_day(vector<string>& selfiles,
	string outpath , double lon , int ymd )
{
	cout << "making daily " << outpath << " ... " << endl;
	if (selfiles.size() == 0) return 0;

	//get output parameters.
	int rasterXSize = 0;
	int rasterYSize = 0;

	{
		string dspath = selfiles[0] ;
		GDALDataset* tds = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
		if (tds == 0)
		{
			return 10;
		}
		rasterXSize = tds->GetRasterXSize();
		rasterYSize = tds->GetRasterYSize();
		GDALClose(tds);
	}

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outputDataset = driver->Create(outpath.c_str(), rasterXSize, rasterYSize, 1, GDT_Byte, 0);

	short* buffer = new short[rasterXSize*rasterYSize];
	short* outBuffer = new short[rasterXSize*rasterYSize];
	int bufferSize = rasterXSize * rasterYSize;

	memset(outBuffer, 0, sizeof(short)*bufferSize);

	if (buffer == 0 || outBuffer == 0 )
	{
		cout << "Error : can not alloc more memories databuffer , countbuffer , outbuffer." << endl;
		if (buffer) delete[] buffer;
		if (outBuffer) delete[] outBuffer;

		GDALClose(outputDataset);
		wft_remove_file(outpath);
		return 30;
	}

	int dsCount = selfiles.size();
	for (int ids = 0; ids < dsCount; ++ids)
	{
		string dspath =  selfiles[ids]  ;
		GDALDataset* dstemp = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
		if (dstemp == 0)
		{
			std::cout << "*** Error : can not open " << dspath << ". out." << std::endl;
			continue;
		}
		int nx = dstemp->GetRasterXSize();
		int ny = dstemp->GetRasterYSize();
		if (nx != rasterXSize || ny != rasterYSize) {
			std::cout << "*** Error : rastersize is wrong " << rasterXSize<<","
				<<rasterYSize<<";"<<nx<<","<<ny << ". out." << std::endl;
			GDALClose(dstemp);
			continue ;
		}
		dstemp->GetRasterBand(1)->RasterIO( GF_Read, 0, 0, rasterXSize, rasterYSize,
			buffer, rasterXSize, rasterYSize, GDT_Int16, 0, 0, 0);
		for (int it = 0 ; it < bufferSize ; ++ it )
		{
			if (outBuffer[it] < buffer[it]) outBuffer[it] = buffer[it];
		}
		GDALClose(dstemp);
	}
	outputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, rasterXSize, rasterYSize,
			outBuffer, rasterXSize, rasterYSize, GDT_Int16, 0, 0, 0);
		
	
	delete[] buffer; buffer = 0;
	delete[] outBuffer; outBuffer = 0;

	GDALClose(outputDataset);

	makeCinfofile(outpath, (int)selfiles.size());


	if (wft_test_file_exists(outpath) == false)
	{
		cout << "error : failed to make " << outpath << endl;
		return 40;
	}

	return 100;
}

//处理多天加和数据
int processOneSum(vector<string>& selfiles,
	string outpath, double lon, int ymd0 , int ymd1 )
{
	int todayYmd = wft_current_dateymd_int();
	if (todayYmd <= ymd1) return 0;
	cout << "making sum " << outpath << " ... " << endl;
	if (selfiles.size() == 0) return 0;

	//get output parameters.
	int rasterXSize = 0;
	int rasterYSize = 0;

	{
		string dspath = selfiles[0];
		GDALDataset* tds = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
		if (tds == 0)
		{
			return 10;
		}
		rasterXSize = tds->GetRasterXSize();
		rasterYSize = tds->GetRasterYSize();
		GDALClose(tds);
	}

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outputDataset = driver->Create(outpath.c_str(), rasterXSize, rasterYSize, 1, GDT_Int16, 0);
	if (outputDataset == 0)
	{
		cout << "Error : failed create " << outpath << endl;
		return 10;
	}

	short* buffer = new short[rasterXSize*rasterYSize];
	short* outBuffer = new short[rasterXSize*rasterYSize];
	int bufferSize = rasterXSize * rasterYSize;

	memset(outBuffer, 0, sizeof(short)*bufferSize);

	if (buffer == 0 || outBuffer == 0)
	{
		cout << "Error : can not alloc more memories databuffer , countbuffer , outbuffer." << endl;
		if (buffer) delete[] buffer;
		if (outBuffer) delete[] outBuffer;

		GDALClose(outputDataset);
		wft_remove_file(outpath);
		return 30;
	}

	int dsCount = selfiles.size();
	for (int ids = 0; ids < dsCount; ++ids)
	{
		string dspath = selfiles[ids];
		GDALDataset* dstemp = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
		if (dstemp == 0)
		{
			std::cout << "*** Error : can not open " << dspath << ". out." << std::endl;
			continue;
		}
		int nx = dstemp->GetRasterXSize();
		int ny = dstemp->GetRasterYSize();
		if (nx != rasterXSize || ny != rasterYSize) {
			std::cout << "*** Error : raster size is wrong " << rasterXSize << ","
				<< rasterYSize << ";" << nx << "," << ny << ". out." << std::endl;
			GDALClose(dstemp);
			continue;
		}
		dstemp->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, rasterXSize, rasterYSize,
			buffer, rasterXSize, rasterYSize, GDT_Int16, 0, 0, 0);
		for (int it = 0; it < bufferSize; ++it)
		{
			if (10 == buffer[it]) ++ outBuffer[it]  ;
		}
		GDALClose(dstemp);
	}
	outputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, rasterXSize, rasterYSize,
		outBuffer, rasterXSize, rasterYSize, GDT_Int16, 0, 0, 0);


	delete[] buffer; buffer = 0;
	delete[] outBuffer; outBuffer = 0;

	if( lon < -900 )
	{
		string dspath = selfiles[0];
		GDALDataset* tds = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
		double trans[6];
		tds->GetGeoTransform(trans);
		outputDataset->SetGeoTransform(trans);
		outputDataset->SetProjection(tds->GetProjectionRef());
		GDALClose(tds);
	}

	GDALClose(outputDataset);

	makeCinfofile(outpath, (int)selfiles.size());


	if (wft_test_file_exists(outpath) == false)
	{
		cout << "error : failed to make " << outpath << endl;
		return 40;
	}
	
	return 100;
}


void processOneDayIms(string inpath, string outpath , int ymd )
{
	//"gdalwarp -overwrite -t_srs "+proj=aea +ellps=krass +lon_0=95  +lat_1=15 +lat_2=65"
	//-tr 4000 4000 -te -8397691.5699528586119413 -792.7805328145623207 9246308.4300471413880587 10215207.2194671854376793  test.stere.tif test.asia.albers.tif
	string cmd1 = g_gdalwarp + " -overwrite -t_srs \"+proj=aea +ellps=krass +lon_0=95  +lat_1=15 +lat_2=65\" "
		+ " -tr 4000 4000 -te -8397691.5699528586119413 -792.7805328145623207 9246308.4300471413880587 10215207.2194671854376793 "
		+ " " + inpath + " " + outpath ;
	system(cmd1.c_str());

	if (wft_test_file_exists(outpath))
	{
		GDALDataset* ds = (GDALDataset*)GDALOpen(outpath.c_str() , GA_Update );
		if (ds != 0)
		{
			int xsize = ds->GetRasterXSize();
			int ysize = ds->GetRasterYSize();
			int asize = xsize * ysize;
			char* buffer = new char[asize];
			ds->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, xsize, ysize, buffer, xsize, ysize, GDT_Byte, 0, 0, 0);
			for (int it = 0; it < asize; ++it)
			{
				switch (buffer[it])
				{
				case 1: buffer[it] = 7; break;
				case 2:buffer[it] = 8; break;
				case 3:buffer[it] = 9; break;
				case 4:buffer[it] = 10; break;
				default:
					buffer[it] = 0;
					break;
				}
			}
			ds->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, xsize, ysize, buffer, xsize, ysize, GDT_Byte, 0, 0, 0);
			GDALClose(ds);
		}
		else {
			cout << "Error : failed to open " << outpath << endl;
		}
	}
	else {
		cout << "Error : failed to make " << outpath << endl;
	}

}
void processImsDaily( string imsdir , string l2dir , int ymdloc )
{
	string inprefix = "ims";
	string intail = ".tif";
	vector<string> allfiles;
	wft_get_allSelectedFiles(imsdir, inprefix, intail, -1, "", allfiles);
	int n = allfiles.size();
	for (int i = 0; i < n ; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymd7 = filename.substr(ymdloc, 7);
		string yearstr = ymd7.substr(0, 4);
		string doystr = ymd7.substr(4, 3);
		int year = wft_str2int(yearstr);
		int doy = wft_str2int(doystr);
		int mon, day;
		wft_convertdayofyear2monthday(year, doy, mon, day);
		int ymd8 = year * 10000 + mon * 100 + day;
		if( ymd8 < g_from_ymd || ymd8 > g_to_ymd ) continue ;
		string ymd8str = wft_int2str(ymd8);
		string newname = "ims_" + ymd8str + "_4km_gis.ea.day.tif";
		string newpath = l2dir + newname;
		if (wft_test_file_exists(newpath) == false)
		{
			processOneDayIms(filepath, newpath , ymd8 );
		}
		if( g_db.hasimg(g_pid_ims_day, ymd8)==false && wft_test_file_exists(newpath) ) 
		{
			dowarpPlotDb(newpath, -999, ymd8, ymd8, g_tem_day_asia, "" , g_pid_ims_day
				, "" , g_imsmjzs[MJZS_DAY]  , false , g_ims_pname );
		}
		cout << i << "/" << n << endl;
	}
}
void processImsLevel3(string l2dir, string l3dir )
{
	int ymdloc = 4;
	string inprefix = "ims";
	string intail = ".ea.day.tif";
	vector<string> allfiles;
	wft_get_allSelectedFiles(l2dir, inprefix, intail, -1, "", allfiles);
	vector<int> ymdvec;
	extractFilesYmd(allfiles, ymdvec, ymdloc);
	int n = allfiles.size();
	for (int i = 0; i < n; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymdstr = filename.substr(ymdloc, 8);
		int ymd = wft_str2int(ymdstr);
		if( ymd < g_from_ymd || ymd > g_to_ymd ) continue ;
		{//monthly
			int ymd0, ymd1;
			getmonStartEnd(ymd, ymd0, ymd1);
			string ymd0str = wft_int2str(ymd0);
			vector<string> selfiles;
			selectedFilesByYmdRange(allfiles, ymdvec, selfiles, ymd0, ymd1);
			string newname = "ims_" + ymd0str + "_4km_gis.ea.mon.tif";
			string newpath = l3dir + newname;
			if (wft_test_file_exists(newpath) == false)
			{
				cout<<"making sum ..."<<endl ;
				if (processOneSum(selfiles, newpath, -999, ymd0, ymd1) == 100)
				{

				}
			}
			if( wft_test_file_exists(newpath) &&
				 g_db.hasimg(g_pid_ims_mon,ymd0) == false )
			{
				dowarpPlotDb(newpath, -999, ymd0, ymd1, g_tem_daycnt_asia, g_tem_daycnt_china,
						g_pid_ims_mon, "",  g_imsmjzs[MJZS_MON] , false, g_ims_pname);
			}
		}
		{//season
			int ymd0, ymd1;
			getseaStartEnd(ymd, ymd0, ymd1);
			string ymd0str = wft_int2str(ymd0);
			vector<string> selfiles;
			selectedFilesByYmdRange(allfiles, ymdvec, selfiles, ymd0, ymd1);
			string newname = "ims_" + ymd0str + "_4km_gis.ea.sea.tif";
			string newpath = l3dir + newname;
			if (wft_test_file_exists(newpath) == false)
			{
				cout<<"making sum ..."<<endl ;
				if (processOneSum(selfiles, newpath, -999, ymd0, ymd1) == 100)
				{
					
				}
			}
			if( wft_test_file_exists(newpath) &&
				 g_db.hasimg(g_pid_ims_sea,ymd0) == false )
			{
				dowarpPlotDb(newpath, -999, ymd0, ymd1, g_tem_daycnt_asia, g_tem_daycnt_china,
						g_pid_ims_sea, "",  g_imsmjzs[MJZS_SEA] , false, g_ims_pname);
			}
		}


		{//year
			int ymd0, ymd1;
			getyeaStartEnd(ymd, ymd0, ymd1);
			string ymd0str = wft_int2str(ymd0);
			vector<string> selfiles;
			selectedFilesByYmdRange(allfiles, ymdvec, selfiles, ymd0, ymd1);
			string newname = "ims_" + ymd0str + "_4km_gis.ea.yea.tif";
			string newpath = l3dir + newname;
			if (wft_test_file_exists(newpath) == false)
			{
				cout<<"making sum ..."<<endl ;
				if (processOneSum(selfiles, newpath, -999, ymd0, ymd1) == 100)
				{

				}
			}
			if( wft_test_file_exists(newpath) &&
				 g_db.hasimg(g_pid_ims_yea,ymd0) == false )
			{//cout<<"making sum ..."<<endl ;
				dowarpPlotDb(newpath, -999, ymd0, ymd1, g_tem_daycnt_asia, g_tem_daycnt_china,
						g_pid_ims_yea, "",  g_imsmjzs[MJZS_YEA] , false, g_ims_pname);
			}
		}
		cout << i << "/" << n << endl;
	}
}



void processFy4Daily(string fy4l2v2dir, string fy4l2v2daydir, int ymdloc, int lonloc )
{
	
	vector<string> allfiles;
	vector<int> ymdVec;
	string inprefix = "fy4a_snc_l2v2_";
	string intail = ".tif";
	wft_get_allSelectedFiles(fy4l2v2dir, inprefix, intail, -1, "", allfiles);
	int numfiles = allfiles.size();
	if (numfiles == 0)
	{
		cout << "Warning : no files is need to process." << endl;
		return ;
	}
	for (int i = 0; i < numfiles; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymdstr = filename.substr(ymdloc, 8);
		int ymdi = (int)atof(ymdstr.c_str());
		ymdVec.push_back(ymdi);
	}
	int todayYmd = wft_current_dateymd_int();
	for (int i = 0; i < numfiles; ++i)
	{
		int ymdi = ymdVec[i];
		int ymd0, ymd1;
		ymd0 = ymdi;
		ymd1 = ymdi;
		if (todayYmd <= ymd1) continue;
		vector<string> selectedfiles;
		int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
		if (combcnt == 0) continue;
		string infilename = wft_base_name(selectedfiles[0]);
		string lonstr = infilename.substr(lonloc, 4);
		double lonval = atof(lonstr.c_str())*0.1;
		string outname = "fy4a_snc_l2v2_" + lonstr + "_" + wft_int2str(ymd0) + ".day.tif";
		string outpath = fy4l2v2daydir + outname;
		if (wft_test_file_exists(outpath) == false)
		{
			if (processOneFy4_3_6_to_day(selectedfiles, outpath, lonval, ymd0) == 100)
			{
				//translate geos
				
			}
		}
		if(wft_test_file_exists(outpath)&&
			g_db.hasimg(g_pid_fy4_day , ymd0)==false )
		{
			dowarpPlotDb(outpath, lonval, ymd0, ymd0, g_tem_day_asia, g_tem_day_china, 
					g_pid_fy4_day, "", g_fy4mjzs[MJZS_DAY] , false, g_fy4_pname);
		}
	}
}

void processFy4Level3(string fy4l2v2daydir, string fy4l2v2level3dir )
{
	int ymdloc = 19;
	int lonloc = 14;
	vector<string> allfiles;
	vector<int> ymdVec;
	string inprefix = "fy4a_snc_l2v2_";
	string intail = ".day.tif";
	wft_get_allSelectedFiles(fy4l2v2daydir, inprefix, intail, -1, "", allfiles);
	int numfiles = allfiles.size();
	if (numfiles == 0)
	{
		cout << "Warning : no files is need to process." << endl;
	}
	for (int i = 0; i < numfiles; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymdstr = filename.substr(ymdloc, 8);
		int ymdi = (int)atof(ymdstr.c_str());
		ymdVec.push_back(ymdi);
	}
	int todayYmd = wft_current_dateymd_int();
	for (int i = 0; i < numfiles; ++i)
	{
		int ymdi = ymdVec[i];
		string infilename = wft_base_name(allfiles[i]);
		string lonstr = infilename.substr(lonloc, 4);
		double lonval = atof(lonstr.c_str())*0.1;
		//monthly
		{
			int ymd0, ymd1;
			getmonStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			string outname = "fy4a_snc_l2v2_" + lonstr + "_" + wft_int2str(ymd0) + ".mon.tif";
			string outpath = fy4l2v2level3dir + outname;
			if (wft_test_file_exists(outpath) == false)
			{
				if (processOneSum(selectedfiles, outpath, lonval, ymd0, ymd1) == 100)
				{

				}
			}
			if(wft_test_file_exists(outpath)&&
				g_db.hasimg(g_pid_fy4_mon , ymd0)==false )
			{
				dowarpPlotDb(outpath, lonval, ymd0, ymd1, g_tem_daycnt_asia,
						g_tem_daycnt_china, g_pid_fy4_mon, "", g_fy4mjzs[MJZS_MON] ,
						false, g_fy4_pname);
			}

		}
		//seasonly
		{
			int ymd0, ymd1;
			getseaStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			string outname = "fy4a_snc_l2v2_" + lonstr + "_" + wft_int2str(ymd0) + ".sea.tif";
			string outpath = fy4l2v2level3dir + outname;
			if (wft_test_file_exists(outpath) == false)
			{
				if (processOneSum(selectedfiles, outpath, lonval, ymd0, ymd1) == 100)
				{

				}
			}
			if(wft_test_file_exists(outpath)&&
				g_db.hasimg(g_pid_fy4_sea , ymd0)==false )
			{
				dowarpPlotDb(outpath, lonval, ymd0, ymd1, g_tem_daycnt_asia,
						g_tem_daycnt_china, g_pid_fy4_sea, "", g_fy4mjzs[MJZS_SEA] ,
						false, g_fy4_pname);
			}
		}
		//yearly
		{
			int ymd0, ymd1;
			getyeaStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			string outname = "fy4a_snc_l2v2_" + lonstr + "_" + wft_int2str(ymd0) + ".yea.tif";
			string outpath = fy4l2v2level3dir + outname;
			if (wft_test_file_exists(outpath) == false)
			{
				if (processOneSum(selectedfiles, outpath, lonval, ymd0, ymd1) == 100)
				{
					
				}
			}
			if(wft_test_file_exists(outpath)&&
				g_db.hasimg(g_pid_fy4_yea , ymd0)==false )
			{
				dowarpPlotDb(outpath, lonval, ymd0, ymd1, g_tem_daycnt_asia,
						g_tem_daycnt_china, g_pid_fy4_yea, "", g_fy4mjzs[MJZS_YEA] ,
						false, g_fy4_pname);
			}
		}
	}
	

}




void processRongheDaily(string rhdir2 , int ymdloc, int lonloc)
{
	vector<string> allfiles;
	vector<int> ymdVec;
	string inprefix = "fy4a_rnh_";
	string intail = ".day.tif";
	wft_get_allSelectedFiles(rhdir2, inprefix, intail, -1, "", allfiles);
	int numfiles = allfiles.size();
	if (numfiles == 0)
	{
		cout << "Warning : no files is need to process." << endl;
		return;
	}
	for (int i = 0; i < numfiles; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymdstr = filename.substr(ymdloc, 8);
		int ymdi = (int)atof(ymdstr.c_str());
		ymdVec.push_back(ymdi);
	}
	int todayYmd = wft_current_dateymd_int();
	for (int i = 0; i < numfiles; ++i)
	{
		int ymdi = ymdVec[i];
		if (todayYmd <= ymdi) continue;
		string outpath = allfiles[i]  + ".cn.tif.png" ;
		if (wft_test_file_exists(outpath) == false ||
				g_db.hasimg( g_pid_rnh_day , ymdi )==false  )
		{
			string filename = wft_base_name(allfiles[i]);
			string lonstr = filename.substr(lonloc, 4);
			double lonval = atof(lonstr.c_str())*0.1;
			//translate geos
			dowarpPlotDb(allfiles[i], lonval, ymdi, ymdi, g_tem_day_asia, g_tem_day_china,
					"", g_pid_rnh_day , g_rnhmjzs[MJZS_DAY] , true , g_rh_pname );
		}
	}
}

void processRongheLevel3(string rhdir2, string rhdir3 , int ymdloc , int lonloc )
{
	vector<string> allfiles;
	vector<int> ymdVec;
	string inprefix = "fy4a_rnh_";
	string intail = ".day.tif";
	wft_get_allSelectedFiles(rhdir2, inprefix, intail, -1, "", allfiles);
	int numfiles = allfiles.size();
	if (numfiles == 0)
	{
		cout << "Warning : no files is need to process." << endl;
	}
	for (int i = 0; i < numfiles; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymdstr = filename.substr(ymdloc, 8);
		int ymdi = (int)atof(ymdstr.c_str());
		ymdVec.push_back(ymdi);
	}
	int todayYmd = wft_current_dateymd_int();
	for (int i = 0; i < numfiles; ++i)
	{
		int ymdi = ymdVec[i];
		string infilename = wft_base_name(allfiles[i]);
		string lonstr = infilename.substr(lonloc, 4);
		double lonval = atof(lonstr.c_str())*0.1;
		string prefix = infilename.substr(0,ymdloc);

		//monthly
		{
			int ymd0, ymd1;
			getmonStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			int pos = infilename.rfind(".day.tif");
			string ymd0str = wft_int2str(ymd0);
			string outname = prefix + ymd0str + ".mon.tif";
			string outpath = rhdir3 + outname;
			if (wft_test_file_exists(outpath) == false)
			{
				if (processOneSum(selectedfiles, outpath, lonval, ymd0, ymd1) == 100)
				{
					
				}
			}
			if(wft_test_file_exists(outpath) &&
				g_db.hasimg(g_pid_rnh_mon,ymd0) ==false ) 
			{
				dowarpPlotDb(outpath, lonval, ymd0, ymd1, g_tem_daycnt_asia,
						g_tem_daycnt_china, "" , g_pid_rnh_mon , g_rnhmjzs[MJZS_MON] ,
						true , g_rh_pname );
			}
		}
		//seasonly
		{
			int ymd0, ymd1;
			getseaStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			int pos = infilename.rfind(".day.tif");
			string ymd0str = wft_int2str(ymd0);
			string outname = prefix + ymd0str + ".sea.tif";
			string outpath = rhdir3 + outname;
			if (wft_test_file_exists(outpath) == false)
			{
				if (processOneSum(selectedfiles, outpath, lonval, ymd0, ymd1) == 100)
				{
					
				}
			}
			if(wft_test_file_exists(outpath) &&
				g_db.hasimg(g_pid_rnh_sea,ymd0) ==false ) 
			{
				dowarpPlotDb(outpath, lonval, ymd0, ymd1, g_tem_daycnt_asia,
						g_tem_daycnt_china, "", g_pid_rnh_sea, g_rnhmjzs[MJZS_SEA],
						true, g_rh_pname);
			}
		}
		//yearly
		{
			int ymd0, ymd1;
			getyeaStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			int pos = infilename.rfind(".day.tif");
			string ymd0str = wft_int2str(ymd0);
			string outname = prefix + ymd0str + ".yea.tif";
			string outpath = rhdir3 + outname;
			if (wft_test_file_exists(outpath) == false)
			{
				if (processOneSum(selectedfiles, outpath, lonval, ymd0, ymd1) == 100)
				{
					
				}
			}
			if(wft_test_file_exists(outpath) &&
				g_db.hasimg(g_pid_rnh_yea,ymd0) ==false ) 
			{
				dowarpPlotDb(outpath, lonval, ymd0, ymd1, g_tem_daycnt_asia,
						g_tem_daycnt_china, "", g_pid_rnh_yea, g_rnhmjzs[MJZS_YEA] , 
						true, g_rh_pname);
			}
		}
	}


}








int main(int argc, char** argv)
{
	printCurrentTime();
	std::cout << "Description: snow combination for  daily(day),month(mon),season(sea),year(yea). " << std::endl;
	std::cout << "Version 1a . by wangfengdev@163.com 2017-12-16." << std::endl;
	std::cout << "Version 2a . add snow fusion v3. by wangfengdev@163.com 2017-12-18." << std::endl;
	std::cout << "Version 2.0.1a . bugfixed 20171219-0757 ." << std::endl;
	std::cout << "Version 2.1.1a . do or not do  ." << std::endl;
	std::cout << "Version 2.2a . add mjzs for season and year. 2017-12-21  ." << std::endl;
	if (argc == 1)
	{
		std::cout << "*** sample call: ***" << std::endl;
		std::cout << "snow_combination startup.txt" << endl;
		std::cout << "*** startup.txt example ***" << std::endl;
		cout<<""<<endl;

		return 101;
	}

	std::cout << "processing..." << std::endl;
	string startup = argv[1];
	GDALAllRegister();


	///////////////////////////////////////////////////////////////////////
	//
	//
	//  Initializing parameters.
	//
	///////////////////////////////////////////////////////////////////////
	g_gdaltranslate = wft_getValueFromExtraParamsFile(startup, "#gdaltranslate", true);
	g_gdalwarp = wft_getValueFromExtraParamsFile(startup, "#gdalwarp", true);

	g_shp_oyalbers_china = wft_getValueFromExtraParamsFile(startup, "#shp-oyalbers-china", true);

	g_txt = wft_getValueFromExtraParamsFile(startup, "#txt", true);
	g_gnuplot = wft_getValueFromExtraParamsFile(startup, "#gnuplot", true);
	g_tem_day_asia = wft_getValueFromExtraParamsFile(startup, "#plottem-day-asia", true);
	g_tem_day_china = wft_getValueFromExtraParamsFile(startup, "#plottem-day-china", true);
	g_tem_daycnt_asia = wft_getValueFromExtraParamsFile(startup, "#plottem-daycnt-asia", true);
	g_tem_daycnt_china = wft_getValueFromExtraParamsFile(startup, "#plottem-daycnt-china", true);

	g_shengmask = wft_getValueFromExtraParamsFile(startup, "#sheng-mask", true);
	g_threemask = wft_getValueFromExtraParamsFile(startup, "#three-mask", true);

	g_pixelarea = 16;//4km*4km
	wft_getValueDoubleFromExtraParamsFile(startup, "#pixelarea", g_pixelarea, true);


	//space monitor product ids
	g_pid_fy4_day = wft_getValueFromExtraParamsFile(startup, "#pid-fy4-day", true);
	g_pid_fy4_mon = wft_getValueFromExtraParamsFile(startup, "#pid-fy4-mon", true);
	g_pid_fy4_sea = wft_getValueFromExtraParamsFile(startup, "#pid-fy4-sea", true);
	g_pid_fy4_yea = wft_getValueFromExtraParamsFile(startup, "#pid-fy4-yea", true);

	g_pid_rnh_day = wft_getValueFromExtraParamsFile(startup, "#pid-rnh-day", true);
	g_pid_rnh_mon = wft_getValueFromExtraParamsFile(startup, "#pid-rnh-mon", true);
	g_pid_rnh_sea = wft_getValueFromExtraParamsFile(startup, "#pid-rnh-sea", true);
	g_pid_rnh_yea = wft_getValueFromExtraParamsFile(startup, "#pid-rnh-yea", true);

	g_pid_ims_day = wft_getValueFromExtraParamsFile(startup, "#pid-ims-day", true);
	g_pid_ims_mon = wft_getValueFromExtraParamsFile(startup, "#pid-ims-mon", true);
	g_pid_ims_sea = wft_getValueFromExtraParamsFile(startup, "#pid-ims-sea", true);
	g_pid_ims_yea = wft_getValueFromExtraParamsFile(startup, "#pid-ims-yea", true);


	//pname
	g_fy4_pname = wft_getValueFromExtraParamsFile(startup, "#fy4-pname", true);
	g_rh_pname = wft_getValueFromExtraParamsFile(startup, "#rnh-pname", true);
	g_ims_pname = wft_getValueFromExtraParamsFile(startup, "#ims-pname", true);

	//mianjizhishu pid

	g_fy4mjzs[MJZS_DAY] = wft_getValueFromExtraParamsFile(startup, "#pid-fy4-mjzs-day", true);
	g_fy4mjzs[MJZS_MON] = wft_getValueFromExtraParamsFile(startup, "#pid-fy4-mjzs-mon", true);
	g_fy4mjzs[MJZS_SEA] = wft_getValueFromExtraParamsFile(startup, "#pid-fy4-mjzs-sea", true);
	g_fy4mjzs[MJZS_YEA] = wft_getValueFromExtraParamsFile(startup, "#pid-fy4-mjzs-yea", true);

	g_rnhmjzs[MJZS_DAY] = wft_getValueFromExtraParamsFile(startup, "#pid-rnh-mjzs-day", true);
	g_rnhmjzs[MJZS_MON]  = wft_getValueFromExtraParamsFile(startup, "#pid-rnh-mjzs-mon", true);
	g_rnhmjzs[MJZS_SEA]  = wft_getValueFromExtraParamsFile(startup, "#pid-rnh-mjzs-sea", true);
	g_rnhmjzs[MJZS_YEA]  = wft_getValueFromExtraParamsFile(startup, "#pid-rnh-mjzs-yea", true);

	g_imsmjzs[MJZS_DAY] = wft_getValueFromExtraParamsFile(startup, "#pid-ims-mjzs-day", true);
	g_imsmjzs[MJZS_MON] = wft_getValueFromExtraParamsFile(startup, "#pid-ims-mjzs-mon", true);
	g_imsmjzs[MJZS_SEA] = wft_getValueFromExtraParamsFile(startup, "#pid-ims-mjzs-sea", true);
	g_imsmjzs[MJZS_YEA] = wft_getValueFromExtraParamsFile(startup, "#pid-ims-mjzs-yea", true);

	//
	g_xspace = wft_getValueFromExtraParamsFile(startup, "#xspace", true);
	g_yspace = wft_getValueFromExtraParamsFile(startup, "#yspace", true);


	string host = wft_getValueFromExtraParamsFile(startup, "#host", true);
	string user = wft_getValueFromExtraParamsFile(startup, "#user", true);
	string pwd = wft_getValueFromExtraParamsFile(startup, "#pwd", true);
	string db = wft_getValueFromExtraParamsFile(startup, "#db", true);
	g_db.connect(host ,user , pwd , db ) ;


	string dofy4 = wft_getValueFromExtraParamsFile(startup, "#dofy4", true);
	string dornh = wft_getValueFromExtraParamsFile(startup, "#dornh", true);
	string doims = wft_getValueFromExtraParamsFile(startup, "#doims", true);


	string fromymdstr = wft_getValueIntFromExtraParamsFile(startup, "#fromymd" , g_from_ymd , true);
	string toymdstr = wft_getValueIntFromExtraParamsFile(startup, "#toymd",g_to_ymd ,  true);
	cout<<"from "<<g_from_ymd<<" to "<<g_to_ymd<<endl ;
	///////////////////////////////////////////////////////////////////////
	//
	//
	//  FY4 snow
	//
	///////////////////////////////////////////////////////////////////////
	string dir2 = wft_getValueFromExtraParamsFile(startup, "#fy4l2v2dir", true);
	string dir3 = wft_getValueFromExtraParamsFile(startup, "#fy4l2v2daydir", true);
	string dir4 = wft_getValueFromExtraParamsFile(startup, "#fy4l2v2level3dir", true);
	string fy4ymdlocstr = wft_getValueFromExtraParamsFile(startup, "#fy4l2v2ymdloc", true);
	string fy4lonlocstr = wft_getValueFromExtraParamsFile(startup, "#fy4l2v2lonloc", true);
	int fy4ymdloc = wft_str2int(fy4ymdlocstr);
	int fy4lonloc = wft_str2int(fy4lonlocstr);
	if( dofy4=="1" ) processFy4Daily(dir2, dir3, fy4ymdloc ,fy4lonloc );
	if( dofy4=="1" ) processFy4Level3(dir3, dir4);


	///////////////////////////////////////////////////////////////////////
	//
	//
	//  Rong he snow
	//
	///////////////////////////////////////////////////////////////////////
	g_fusion_fy4lonfile = wft_getValueFromExtraParamsFile(startup, "#fy4lon", true);
	g_fusion_fy4latfile = wft_getValueFromExtraParamsFile(startup, "#fy4lat", true);
	g_fusion_fy4lonfile9 = wft_getValueFromExtraParamsFile(startup, "#fy4lon995", true);
	g_fusion_fy4latfile9 = wft_getValueFromExtraParamsFile(startup, "#fy4lat995", true);

	string fy3swedir = wft_getValueFromExtraParamsFile(startup, "#fy3swedir", true);
	string stationfile = wft_getValueFromExtraParamsFile(startup, "#stationsnowfile", true);

	string rhdir2 = wft_getValueFromExtraParamsFile(startup, "#rnhdaydir", true);
	string rhdir3 = wft_getValueFromExtraParamsFile(startup, "#rnhlevel3dir", true);
	int rnhymdloc, rnhlonloc;
	string rnhymdlocstr = wft_getValueIntFromExtraParamsFile(startup, "#rnhymdloc", rnhymdloc ,true);
	string rnhlonlocstr = wft_getValueIntFromExtraParamsFile(startup, "#rnhlonloc", rnhlonloc, true);
	if( dornh=="1" )snowfusionProcess(dir3 , 19 , 14 , rhdir2  , fy3swedir , stationfile ) ;

	if( dornh=="1" )processRongheDaily(rhdir2, rnhymdloc, rnhlonloc);
	if( dornh=="1" )processRongheLevel3(rhdir2, rhdir3, rnhymdloc, rnhlonloc);



	///////////////////////////////////////////////////////////////////////
	//
	//
	//  IMS Processing
	//
	///////////////////////////////////////////////////////////////////////
	string imsdir = wft_getValueFromExtraParamsFile(startup, "#imsdaydir", true);
	string imsl2dir = wft_getValueFromExtraParamsFile(startup, "#imsv2daydir", true);
	string imsl3dir = wft_getValueFromExtraParamsFile(startup, "#imsv2level3dir", true);
	string imsymdlocstr = wft_getValueFromExtraParamsFile(startup, "#imsymdloc", true);
	int imsYmdloc = wft_str2int(imsymdlocstr);
	cout << "begin process ims..." << endl;
	if( doims=="1" )processImsDaily(imsdir, imsl2dir , imsYmdloc);
	cout << "begin suming ..." << endl;
	if( doims=="1" )processImsLevel3(imsl2dir, imsl3dir);


	std::cout << "All done." << std::endl;
	printCurrentTime();
	return 0;
}

