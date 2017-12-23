// noaa_ndvi_aio_monitor.cpp : 定义控制台应用程序的入口点。
//
//


#include <iostream>
#include "ogr_spatialref.h"//17
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include "../sharedcodes/wftools.h"
#include "../sharedcodes/ajson5.h"
#include "../sharedcodes/wdb.h"

#define QHC_MAX 0
#define QHC_AVE 1
#define QHC_TEN 0
#define QHC_MON 1
#define QHC_SEA 2
#define QHC_YEA 3


using namespace std;
using std::cout;

string g_gdaltranslate = "gdal_translate";
string g_gdalwarp = "gdalwarp";

string g_txt = "";
string g_plottem_ndvi = "";
string g_plottem_juping = "";

string g_gnuplot = "";
 
double g_pixelarea = 16;//4km*4km
 

string g_shp_china = "";

//pname
string g_fy4_pname = "";
string g_rh_pname = "";
string g_ims_pname = "";

 
//xspace yspace
string g_xspace = "1";
string g_yspace = "1";

string g_ncdir = "";
string g_l2dir = "";
string g_l3dir = "";
string g_pngdir = "";
string g_ncprefix = "";
string g_nctail = "";
short g_noaa_qamask = 0;

int g_noaa_from_ymd = 0;
int g_noaa_to_ymd = 0;

string g_cmaxname = "";
string g_cavename = "";
string g_jpmaxname = "";
string g_jpavename = "";

int g_pidmaxarr[4];
int g_pidmaxarrjp[4];
int g_pidavearr[4];
int g_pidavearrjp[4];

string g_qhcdir[8];
int g_qhcymdloc[8];

//mysql operation
wDb g_db;


void extractFilesYmd(vector<string>& pathvec, vector<int>& ymdvec, int ymdloc)
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
	ymdarr[1] = (ymd % 10000) / 100;
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
	}
	else if (a[2] < 11)
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 6;
		ymd1 = a[0] * 10000 + a[1] * 100 + 10;
	}
	else if (a[2] < 16)
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 11;
		ymd1 = a[0] * 10000 + a[1] * 100 + 15;
	}
	else if (a[2] < 21)
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
		if (a[1] == 2)
		{
			ymd0 = a[0] * 10000 + a[1] * 100 + 21;
			ymd1 = a[0] * 10000 + a[1] * 100 + 28;
			if (wft_is_leapyear(a[0]))
			{
				ymd1 = a[0] * 10000 + a[1] * 100 + 29;
			}
		}
		else if (a[1] == 4 || a[1] == 6 || a[1] == 9 || a[1] == 11)
		{
			ymd0 = a[0] * 10000 + a[1] * 100 + 21;
			ymd1 = a[0] * 10000 + a[1] * 100 + 30;
		}
		else
		{
			ymd0 = a[0] * 10000 + a[1] * 100 + 21;
			ymd1 = a[0] * 10000 + a[1] * 100 + 31;
		}

	}
}

void getmonStartEnd(int ymdi, int & ymd0, int& ymd1)
{
	int a[3];
	sepYmd(ymdi, a);
	if (a[1] == 2)
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 1;
		ymd1 = a[0] * 10000 + a[1] * 100 + 28;
		if (wft_is_leapyear(a[0]))
		{
			ymd1 = a[0] * 10000 + a[1] * 100 + 29;
		}
	}
	else if (a[1] == 4 || a[1] == 6 || a[1] == 9 || a[1] == 11)
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 1;
		ymd1 = a[0] * 10000 + a[1] * 100 + 30;
	}
	else
	{
		ymd0 = a[0] * 10000 + a[1] * 100 + 1;
		ymd1 = a[0] * 10000 + a[1] * 100 + 31;
	}
}

void getseaStartEnd(int ymdi, int & ymd0, int& ymd1)
{
	int a[3];
	sepYmd(ymdi, a);
	if (a[1] < 3)
	{
		ymd0 = (a[0] - 1) * 10000 + 1201;
		ymd1 = a[0] * 10000 + 228;
		if (wft_is_leapyear(a[0]))
		{
			ymd1 = a[0] * 10000 + 229;
		}
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
		ymd0 = a[0] * 10000 + 901;
		ymd1 = a[0] * 10000 + 1130;
	}
	else {
		ymd0 = a[0] * 10000 + 1201;
		ymd1 = (a[0] + 1) * 10000 + 228;
		if (wft_is_leapyear(a[0]+1))
		{
			ymd1 = a[0] * 10000 + 229;
		}
	}
}

void getyeaStartEnd(int ymdi, int & ymd0, int& ymd1)
{
	int a[3];
	sepYmd(ymdi, a);
	ymd0 = a[0] * 10000 + 101;
	ymd1 = a[0] * 10000 + 1231;
}

struct AreaIndex
{
	inline AreaIndex() :maskid(0), sumval(0) {};
	int maskid;
	double sumval;
};


//计算积雪面积指数
int computeAreaIndexManyDays(string maskfile, string snonwfile,
	int ndays, vector<AreaIndex>& result)
{
	double areaArr[255];
	short hasmask[255];
	memset(hasmask, 0, sizeof(short) * 255);
	memset(areaArr, 0, sizeof(double) * 255);
	GDALDataset* maskds = (GDALDataset*)GDALOpen(maskfile.c_str(), GA_ReadOnly);
	if (maskds == 0)
	{
		cout << "error : failed to open mask file " << maskfile << endl;
		return 10;
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
			if (maskid > 0 && snowbuffer[it] == 10)
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
				areaArr[maskid] += g_pixelarea * snowbuffer[it];
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
			ai.sumval = areaArr[i] / ndays;
			result.push_back(ai);
		}
		else if (hasmask[i] > 0)
		{
			AreaIndex ai;
			ai.maskid = i;
			ai.sumval = 0;
			result.push_back(ai);
		}
	}
	return 100;
}

AreaIndex computeSumOfAreaIndex(vector<AreaIndex>& vec, int newmaskid)
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
void saveSnowAreaIndex(string filepath, string pid, vector<AreaIndex>& vec, int ymd8)
{
	
	ofstream ofs(filepath.c_str());
	for (int i = 0; i < vec.size(); ++i)
	{
		ofs << pid << "\t" << vec[i].maskid << "\t" << vec[i].sumval << "\t" << ymd8 << endl;
		//db g_db.insertxl("tb_xulie_data",pid,vec[i].maskid,vec[i].sumval,ymd8);
	}
	ofs.close();
}



void makeCinfofile(string outpath, int combcnt)
{
	char buff[10];
	sprintf(buff, "c%04d", combcnt);
	string outpath2 = outpath + "." + string(buff) + ".txt";
	ofstream ofs(outpath2.c_str());
	ofs << combcnt << endl;
	ofs.close();
}

void dowarpPlotDb(string inpath, string outpng ,  int ymd0, int ymd1, 
	string chinapid, string mjzspid 
	, string pname , string  plottem , bool isjuping )
{
	string inname = wft_base_name(inpath);

	cout << "shp cutting ..." << endl;
	string cutchinafile = outpng + ".cn.tif";
	//shapecut
	string cmdcut = g_gdalwarp + " -overwrite -srcnodata -9999 -dstnodata -9999  -cutline " + g_shp_china + " -crop_to_cutline -tr 0.05 0.05 -of GTiff " + inpath + " " + cutchinafile;
	cout << cmdcut << endl;
	system(cmdcut.c_str());


	cout << "converting cuted wgs84 -> albers..." << endl;
	string albersfile = cutchinafile + ".albers.tif";
	string cmd3 = g_gdalwarp + " -overwrite -t_srs \"+proj=aea +ellps=krass +lon_0=105 +lat_1=25 +lat_2=47\" -tr 5000 5000 "
		+ cutchinafile + " " + albersfile;
	cout << cmd3 << endl;
	system(cmd3.c_str());

	string ymdstr = wft_convert_ymdi2y_m_d(ymd0);
	string ymdstr1 = wft_convert_ymdi2y_m_d(ymd1);
	if (g_txt != "" )
	{
		cout << "ploting ..." << endl;
		string asiatxtfile = albersfile + ".tmp";
		string cmdtxt1 = "";
		if (isjuping)
		{
			cmdtxt1 = g_txt + " -in " + albersfile + " -out " + asiatxtfile + " -valid0 -9990 -valid1 10000 -xspace "
				+ g_xspace + " -yspace " + g_yspace + " -scale 0.0001 -offset 0 ";
		}
		else
		{
			cmdtxt1 = g_txt + " -in " + albersfile + " -out " + asiatxtfile + " -valid0 -2000 -valid1 10000 -xspace "
				+ g_xspace + " -yspace " + g_yspace + " -scale 0.0001 -offset 0 ";
		}
		system(cmdtxt1.c_str());
		vector<string> vec1, vec2;
		vec1.push_back("{{{INFILE}}}");
		vec1.push_back("{{{OUTFILE}}}");
		vec1.push_back("{{{YMD}}}");
		vec1.push_back("{{{YMD1}}}");
		vec1.push_back("{{{PNAME}}}");
		vec1.push_back("{{{PTIME}}}");

		string png1file = outpng ;
		vec2.push_back(asiatxtfile);
		vec2.push_back(png1file);
		vec2.push_back(ymdstr);
		vec2.push_back(ymdstr1);
		vec2.push_back(pname);
		vec2.push_back(wft_current_datetimestr());

		string plot1file = png1file + ".plot";
		wft_create_file_by_template_with_replacement(plot1file, plottem, vec1, vec2);

		string cmdplot1 = g_gnuplot + " " + plot1file;
		system(cmdplot1.c_str());

		if (chinapid != "")
		{
			//insert db
			string fname = wft_base_name(albersfile);
			g_db.insertimg("tb_product_data",
				chinapid ,
				albersfile,
				fname,
				png1file,
				ymd0
			);
		}

	}


	//计算积雪面积指数
	if (mjzspid != "")
	{
		/*
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
		saveSnowAreaIndex(mjzsfile, mjzspid, snowAreaIndices, ymd0);
		*/
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




string ncymd(string & filename , int& ymd )
{
	ymd = 19000101;
	int pos = filename.rfind("_");
	if (pos == string::npos) return "19000101";
	else
	{
		ymd = wft_str2int(filename.substr(pos - 8, 8));
		return filename.substr(pos - 8, 8);
	}
}


bool isValidNoaaNdvi(int ndvi, short qaval)
{
	if (ndvi >= -2000 && ndvi <= 10000)
	{
		short maskout = qaval & g_noaa_qamask ;
		if (maskout == 0) return true;
		else return false;
	}
	else
	{
		return false;
	}
}


//处理多天加和数据
int processCombineMaxOrAver(vector<string>& selfiles,
	string outpath, bool useMax)
{
	int todayYmd = wft_current_dateymd_int();
	cout << "making max " << outpath << " ... " << endl;
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
	{
		double adfGeoTrans[6] = { -180 , 0.05 , 0 , 90 , 0 , -0.05 };
		OGRSpatialReference osrs;
		char* pszSRS_WKT = 0;
		outputDataset->SetGeoTransform(adfGeoTrans);
		osrs.SetWellKnownGeogCS("EPSG:4326");
		osrs.exportToWkt(&pszSRS_WKT);
		outputDataset->SetProjection(pszSRS_WKT);
		CPLFree(pszSRS_WKT);
	}


	short* buffer = new short[rasterXSize*rasterYSize];
	int* outBuffer = new int[rasterXSize*rasterYSize];
	
	short* cntBuffer = 0;


	int bufferSize = rasterXSize * rasterYSize;
	if (useMax == false)
	{
		cntBuffer = new short[bufferSize];
		memset(cntBuffer, 0, sizeof(short)*bufferSize);
	}

	for (int i = 0; i < bufferSize; ++i) outBuffer[i] = -9999;

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
		if (useMax)
		{
			for (int it = 0; it < bufferSize; ++it)
			{
				if (buffer[it] >= -2000 && buffer[it] <= 10000)
				{
					if (outBuffer[it] < buffer[it]) outBuffer[it] = buffer[it];
				}
			}
		}
		else
		{
			for (int it = 0; it < bufferSize; ++it)
			{
				if (buffer[it] >= -2000 && buffer[it] <= 10000)
				{
					if (cntBuffer[it] == 0)
					{
						outBuffer[it] = buffer[it];
						cntBuffer[it] = 1;
					}
					else {
						outBuffer[it] += buffer[it];
						cntBuffer[it] += 1;
					}
				}
			}
		}
		GDALClose(dstemp);
	}
	if (useMax==false)
	{
		for (int it = 0; it < bufferSize; ++it)
		{
			if (cntBuffer[it] > 0 )
			{
				outBuffer[it] = outBuffer[it]/cntBuffer[it] ;//bugfixed 2017-12-18
			}
		}
	}
	outputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, rasterXSize, rasterYSize,
		outBuffer, rasterXSize, rasterYSize, GDT_Int32, 0, 0, 0);

	delete[] buffer; buffer = 0;
	delete[] outBuffer; outBuffer = 0;
	if (cntBuffer) delete[]cntBuffer; cntBuffer = 0;

	GDALClose(outputDataset);

	makeCinfofile(outpath, (int)selfiles.size());


	if (wft_test_file_exists(outpath) == false)
	{
		cout << "error : failed to make " << outpath << endl;
		return 40;
	}

	return 100;
}


int processOneDayQaProduct(string filepath, string outfilepath)
{
	string vipath = "HDF5:" + filepath + "://NDVI"; //in window
	string qapath = "HDF5:" + filepath + "://QA";   //in window
	// string vipath = "NETCDF:\"" + filepath + "\":NDVI"; //in linux use netcdf
	// string qapath = "NETCDF:\"" + filepath + "\":QA"; //in linux use netcdf

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
		GDALClose(outds);
		GDALClose(dsvi);
		GDALClose(dsqa);
		cout << "Error : can not alloc memory for ndvi or qa " << bufferSize << endl;
		return 20;
	}

	dsvi->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, rasterXSize, rasterYSize,
		dataBufferNdvi, rasterXSize, rasterYSize, GDT_Int32, 0, 0, 0);
	dsqa->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, rasterXSize, rasterYSize,
		dataBufferQA, rasterXSize, rasterYSize, GDT_Int16, 0, 0, 0);
	GDALClose(dsvi);
	GDALClose(dsqa);
	for (int i = 0; i < bufferSize; ++i)
	{
		short qaval = dataBufferQA[i];
		if (isValidNoaaNdvi(dataBufferNdvi[i], qaval))
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

//max0ave1 t0m1s2y3
string findQhc(int max0ave1, int t0m1s2y3, int ymd)
{
	string dir = "";
	int loc = 0;
	if (max0ave1 == 0)
	{
		dir = g_qhcdir[t0m1s2y3];
		loc = g_qhcymdloc[t0m1s2y3];
	}
	else
	{
		dir = g_qhcdir[4 + t0m1s2y3];
		loc = g_qhcymdloc[4 + t0m1s2y3];
	}
	vector<string> allfiles;
	wft_get_allfiles(dir, allfiles);
	int dt0 = ymd % 10000;
	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		int ymdi = wft_str2int(filename.substr(loc, 8));
		int dti = ymdi % 10000;
		if (dti == dt0)
		{
			return filepath;
		}
	}
	return "";
}


int imageASubtractB(string apath, string bpath, string outpath)
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
		cout << "Error : a size " << xsize << " " << ysize << " is different with b size " << bxsize << " " << bysize << "." << endl;
		GDALClose(ads);
		GDALClose(bds);
		return 20;
	}
	
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(outpath.c_str(), xsize, ysize, 1, GDT_Int16 , 0);

	if (outds == 0)
	{
		cout << "Error : can not create " << outpath << endl;
		GDALClose(ads);
		GDALClose(bds);
		return 20;
	}

	double trans[6];
	ads->GetGeoTransform(trans);
	outds->SetGeoTransform(trans);
	outds->SetProjection(ads->GetProjectionRef());

	int asize = xsize * ysize;
	short* abuff = new short[asize];
	if (abuff == 0)
	{
		GDALClose(ads);
		GDALClose(bds);
		GDALClose(outds);
		cout << "Error : can not alloc memory . " << endl;
		return 30;
	}
	short* bbuff = new short[asize];
	if (bbuff == 0)
	{
		GDALClose(ads);
		GDALClose(bds);
		GDALClose(outds);
		delete[] abuff;
		cout << "Error : can not alloc memory . " << endl;
		return 31;
	}

	ads->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, xsize, ysize, abuff, xsize, ysize, GDT_Int16, 0, 0, 0);
	bds->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, xsize, ysize, bbuff, xsize, ysize, GDT_Int16, 0, 0, 0);

	for (int it = 0; it < asize ; ++it)
	{
		if (abuff[it] >= -2000  && abuff[it] <= 10000  && bbuff[it] >= -2000 && bbuff[it] <= 10000 )
		{
			abuff[it] = abuff[it] - bbuff[it] ;
		}
		else {
			abuff[it] = -9999 ;
		}
	}

	outds->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, xsize, ysize , abuff, xsize, ysize , GDT_Int16, 0, 0, 0);
	delete[] abuff;
	delete[] bbuff;
	GDALClose(ads);
	GDALClose(bds);
	GDALClose(outds);
	return 100;
}
 


void processNoaaNcDaily()
{

	vector<string> allfiles;
	vector<int> ymdVec;

	wft_get_allSelectedFiles( g_ncdir, g_ncprefix , g_nctail , -1, "", allfiles);
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
		int ymdi = 0;
		string ymdstr = ncymd(filename, ymdi);
		ymdVec.push_back(ymdi);
	}
	int todayYmd = wft_current_dateymd_int();
	
	char qabuff[8];
	sprintf(qabuff, "qa%05d", g_noaa_qamask);
	string qastr(qabuff);

	for (int i = 0; i < numfiles; ++i)
	{
		string inpath = allfiles[i];
		int ymd0 = ymdVec[i];
		string outname = "noaa_ndvi_" + wft_int2str(ymd0) + "." + qastr + ".day.tif";
		string outpath = g_l2dir + outname;
		if (ymd0 >= g_noaa_from_ymd && ymd0 <= g_noaa_to_ymd)
		{
			if (wft_test_file_exists(outpath) == false)
			{
				cout << "process qa product " << outname << endl;
				processOneDayQaProduct(inpath, outpath);
			}
		}

	}
}

void processNoaaLevel3()
{

	int ymdloc = 10 ;
	int qaloc = 19 ;
	vector<string> allfiles;
	vector<int> ymdVec;
	string inprefix = "noaa_ndvi_";
	string intail = ".day.tif";
	wft_get_allSelectedFiles(g_l2dir , inprefix, intail, -1, "", allfiles);
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
		
		if (ymdi < g_noaa_from_ymd) continue;
		if (ymdi > g_noaa_to_ymd) continue;

		//ten-days
		{
			int ymd0, ymd1;
			gettendaysStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			
			//最大实况
			{
				string outname = "noaa_ndvi_" + wft_int2str(ymd0) + ".max.ten.tif";
				string outpath = g_l3dir + outname;
				string outpng = g_pngdir + outname + ".png";
				if (wft_test_file_exists(outpath) == false)
				{
					processCombineMaxOrAver(selectedfiles, outpath, true);
				}
				if (wft_test_file_exists(outpath) && wft_test_file_exists(outpng) == false)
				{
					dowarpPlotDb(outpath, outpng, ymd0, ymd1, wft_int2str(g_pidmaxarr[0]), "0", g_cmaxname , g_plottem_ndvi ,false);
				}
				//最大距平
				string jpname = "noaa_ndvi_" + wft_int2str(ymd0) + ".max.ten.jp.tif";
				string jppath = g_l3dir + jpname;
				string jppngpath = g_pngdir + jpname + ".png";
				if (wft_test_file_exists(jppath) == false)
				{
					string qhc = findQhc(0, 0, ymd0);
					if( wft_test_file_exists(qhc) )
					{
						imageASubtractB(outpath, qhc, jppath);
					}
					else
					{
						cout << "Error : not found qhc for :" << outname << endl;
					}
				}
				if (wft_test_file_exists(jppath) && wft_test_file_exists(jppngpath) == false)
				{
					dowarpPlotDb(jppath, jppngpath, ymd0, ymd1, wft_int2str(g_pidmaxarrjp[0]), "0", g_jpmaxname , g_plottem_juping , true  );
				}

			}
			
			//平均实况
			{
				string outname1 = "noaa_ndvi_" + wft_int2str(ymd0) + ".ave.ten.tif";
				string outpath1 = g_l3dir + outname1;
				string outpng1 = g_pngdir + outname1 + ".png";
				if (wft_test_file_exists(outpath1) == false)
				{
					processCombineMaxOrAver(selectedfiles, outpath1, false);
				}
				if (wft_test_file_exists(outpath1) && wft_test_file_exists(outpng1) == false)
				{
					dowarpPlotDb(outpath1, outpng1, ymd0, ymd1, wft_int2str(g_pidavearr[0]), "0", g_cavename , g_plottem_ndvi ,false );
				}

				//距平
				//                                                   *** ***
				string jpname = "noaa_ndvi_" + wft_int2str(ymd0) + ".ave.ten.jp.tif";//check
				string jppath = g_l3dir + jpname;
				string jppngpath = g_pngdir + jpname +  ".png";
				if (wft_test_file_exists(jppath) == false)
				{
					//                    *******   *******
					string qhc = findQhc( QHC_AVE , QHC_TEN , ymd0);//check
					if (wft_test_file_exists(qhc))
					{
						imageASubtractB(outpath1, qhc, jppath);
					}
					else
					{
						cout << "Error : not found qhc for :" << outname1 << endl;
					}
				}
				if (wft_test_file_exists(jppath) && wft_test_file_exists(jppngpath) == false)
				{
					//                                                      ****************                                       ****
					dowarpPlotDb(jppath, jppngpath, ymd0, ymd1, wft_int2str(g_pidavearrjp[0]), "0", g_jpavename, g_plottem_juping, true);//check here!!!
				}
			}
			
		}

		//monthly
		{
			int ymd0, ymd1;
			getmonStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;

			{
				string outname = "noaa_ndvi_" + wft_int2str(ymd0) + ".max.mon.tif";
				string outpath = g_l3dir + outname;
				string outpng = g_pngdir + outname + ".png";
				if (wft_test_file_exists(outpath) == false)
				{
					processCombineMaxOrAver(selectedfiles, outpath, true);
				}
				if (wft_test_file_exists(outpath) && wft_test_file_exists(outpng) == false)
				{
					dowarpPlotDb(outpath, outpng, ymd0, ymd1, wft_int2str(g_pidmaxarr[1]), "0", g_cmaxname, g_plottem_ndvi,false);
				}

				//最大距平
				//                                                   *** ***
				string jpname = "noaa_ndvi_" + wft_int2str(ymd0) + ".max.mon.jp.tif";//check
				string jppath = g_l3dir + jpname;
				string jppngpath = g_pngdir + jpname + ".png";
				if (wft_test_file_exists(jppath) == false)
				{
					//                    *******   *******
					string qhc = findQhc(QHC_MAX, QHC_MON, ymd0);//check
					if (wft_test_file_exists(qhc))
					{
						imageASubtractB(outpath, qhc, jppath);
					}
					else
					{
						cout << "Error : not found qhc for :" << outname << endl;
					}
				}
				if (wft_test_file_exists(jppath) && wft_test_file_exists(jppngpath) == false)
				{
					//                                                      ****************                                       ****
					dowarpPlotDb(jppath, jppngpath, ymd0, ymd1, wft_int2str(g_pidmaxarrjp[1]), "0", g_jpavename, g_plottem_juping, true);//check here!!!
				}

			}


			{
				string outname1 = "noaa_ndvi_" + wft_int2str(ymd0) + ".ave.mon.tif";
				string outpath1 = g_l3dir + outname1;
				string outpng1 = g_pngdir + outname1 + ".png";
				if (wft_test_file_exists(outpath1) == false)
				{
					processCombineMaxOrAver(selectedfiles, outpath1, false);
				}
				if (wft_test_file_exists(outpath1) && wft_test_file_exists(outpng1) == false)
				{
					dowarpPlotDb(outpath1, outpng1, ymd0, ymd1, wft_int2str(g_pidavearr[1]), "0", g_cavename, g_plottem_ndvi,false);
				}
				//平均距平
				//                                                   *** ***
				string jpname = "noaa_ndvi_" + wft_int2str(ymd0) + ".ave.mon.jp.tif";//check
				string jppath = g_l3dir + jpname;
				string jppngpath = g_pngdir + jpname + ".png";
				if (wft_test_file_exists(jppath) == false)
				{
					//                    *******   *******
					string qhc = findQhc(QHC_AVE, QHC_MON, ymd0);//check
					if (wft_test_file_exists(qhc))
					{
						imageASubtractB(outpath1, qhc, jppath);
					}
					else
					{
						cout << "Error : not found qhc for :" << outname1 << endl;
					}
				}
				if (wft_test_file_exists(jppath) && wft_test_file_exists(jppngpath) == false)
				{
					//                                                      ****************                                       ****
					dowarpPlotDb(jppath, jppngpath, ymd0, ymd1, wft_int2str(g_pidavearrjp[1]), "0", g_jpavename, g_plottem_juping, true);//check here!!!
				}
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

			{
				string outname = "noaa_ndvi_" + wft_int2str(ymd0) + ".max.sea.tif";
				string outpath = g_l3dir + outname;
				string outpng = g_pngdir + outname + ".png";
				if (wft_test_file_exists(outpath) == false)
				{
					processCombineMaxOrAver(selectedfiles, outpath, true);
				}
				if (wft_test_file_exists(outpath) && wft_test_file_exists(outpng) == false)
				{
					dowarpPlotDb(outpath, outpng, ymd0, ymd1, wft_int2str(g_pidmaxarr[2]), "0", g_cmaxname, g_plottem_ndvi,false);
				}
				//最大距平
				//                                                   *** ***
				string jpname = "noaa_ndvi_" + wft_int2str(ymd0) + ".max.sea.jp.tif";//check
				string jppath = g_l3dir + jpname;
				string jppngpath = g_pngdir + jpname + ".png";
				if (wft_test_file_exists(jppath) == false)
				{
					//                    *******   *******
					string qhc = findQhc(QHC_MAX, QHC_SEA, ymd0);//check
					if (wft_test_file_exists(qhc))
					{
						imageASubtractB(outpath, qhc, jppath);
					}
					else
					{
						cout << "Error : not found qhc for :" << outname << endl;
					}
				}
				if (wft_test_file_exists(jppath) && wft_test_file_exists(jppngpath) == false)
				{
					//                                                      ****************                                       ****
					dowarpPlotDb(jppath, jppngpath, ymd0, ymd1, wft_int2str(g_pidmaxarrjp[2]), "0", g_jpavename, g_plottem_juping, true);//check here!!!
				}
			}


			{
				string outname1 = "noaa_ndvi_" + wft_int2str(ymd0) + ".ave.sea.tif";
				string outpath1 = g_l3dir + outname1;
				string outpng1 = g_pngdir + outname1 + ".png";
				if (wft_test_file_exists(outpath1) == false)
				{
					processCombineMaxOrAver(selectedfiles, outpath1, false);
				}
				if (wft_test_file_exists(outpath1) && wft_test_file_exists(outpng1) == false)
				{
					dowarpPlotDb(outpath1, outpng1, ymd0, ymd1, wft_int2str(g_pidavearr[2]), "0", g_cavename, g_plottem_ndvi,false);
				}

				//平均距平
				//                                                   *** ***
				string jpname = "noaa_ndvi_" + wft_int2str(ymd0) + ".ave.sea.jp.tif";//check
				string jppath = g_l3dir + jpname;
				string jppngpath = g_pngdir + jpname + ".png";
				if (wft_test_file_exists(jppath) == false)
				{
					//                    *******   *******
					string qhc = findQhc(QHC_AVE, QHC_SEA, ymd0);//check
					if (wft_test_file_exists(qhc))
					{
						imageASubtractB(outpath1, qhc, jppath);
					}
					else
					{
						cout << "Error : not found qhc for :" << outname1 << endl;
					}
				}
				if (wft_test_file_exists(jppath) && wft_test_file_exists(jppngpath) == false)
				{
					//                                                      ****************                                       ****
					dowarpPlotDb(jppath, jppngpath, ymd0, ymd1, wft_int2str(g_pidavearrjp[2]), "0", g_jpavename, g_plottem_juping, true);//check here!!!
				}
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

			{
				string outname = "noaa_ndvi_" + wft_int2str(ymd0) + ".max.yea.tif";
				string outpath = g_l3dir + outname;
				string outpng = g_pngdir + outname + ".png";
				if (wft_test_file_exists(outpath) == false)
				{
					processCombineMaxOrAver(selectedfiles, outpath, true);
				}
				if (wft_test_file_exists(outpath) && wft_test_file_exists(outpng) == false)
				{
					dowarpPlotDb(outpath, outpng, ymd0, ymd1, wft_int2str(g_pidmaxarr[3]), "0", g_cmaxname, g_plottem_ndvi,false);
				}
				//最大距平
				//                                                   *** ***
				string jpname = "noaa_ndvi_" + wft_int2str(ymd0) + ".max.yea.jp.tif";//check
				string jppath = g_l3dir + jpname;
				string jppngpath = g_pngdir + jpname + ".png";
				if (wft_test_file_exists(jppath) == false)
				{
					//                    *******   *******
					string qhc = findQhc(QHC_MAX, QHC_YEA, ymd0);//check
					if (wft_test_file_exists(qhc))
					{
						imageASubtractB(outpath, qhc, jppath);
					}
					else
					{
						cout << "Error : not found qhc for :" << outname << endl;
					}
				}
				if (wft_test_file_exists(jppath) && wft_test_file_exists(jppngpath) == false)
				{
					//                                                      ****************                                       ****
					dowarpPlotDb(jppath, jppngpath, ymd0, ymd1, wft_int2str(g_pidmaxarrjp[3]), "0", g_jpavename, g_plottem_juping, true);//check here!!!
				}
			}


			{
				string outname1 = "noaa_ndvi_" + wft_int2str(ymd0) + ".ave.yea.tif";
				string outpath1 = g_l3dir + outname1;
				string outpng1 = g_pngdir + outname1 + ".png";
				if (wft_test_file_exists(outpath1) == false)
				{
					processCombineMaxOrAver(selectedfiles, outpath1, false);
				}
				if (wft_test_file_exists(outpath1) && wft_test_file_exists(outpng1) == false)
				{
					dowarpPlotDb(outpath1, outpng1, ymd0, ymd1, wft_int2str(g_pidavearr[3]), "0", g_cavename, g_plottem_ndvi,false);
				}
				//平均距平
				//                                                   *** ***
				string jpname = "noaa_ndvi_" + wft_int2str(ymd0) + ".ave.yea.jp.tif";//check
				string jppath = g_l3dir + jpname;
				string jppngpath = g_pngdir + jpname + ".png";
				if (wft_test_file_exists(jppath) == false)
				{
					//                    *******   *******
					string qhc = findQhc(QHC_AVE, QHC_YEA, ymd0);//check
					if (wft_test_file_exists(qhc))
					{
						imageASubtractB(outpath1, qhc, jppath);
					}
					else
					{
						cout << "Error : not found qhc for :" << outname1 << endl;
					}
				}
				if (wft_test_file_exists(jppath) && wft_test_file_exists(jppngpath) == false)
				{
					//                                                      ****************                                       ****
					dowarpPlotDb(jppath, jppngpath, ymd0, ymd1, wft_int2str(g_pidavearrjp[3]), "0", g_jpavename, g_plottem_juping, true);//check here!!!
				}
			}
		}
	}
}

///////////////////////////////////// fy3 modis

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


int getHTileIndex(string tname)
{
	if (tname.length() == 3)
	{
		return (int)atof(tname.substr(1, 2).c_str());
	}
	else {
		for (int i = 0; i < 36; ++i)
		{
			if (tname == fyHoriFenFuHaoArray[i])
			{
				return i;
			}
		}
	}
	return -1;
}

int getVTileIndex(string tname)
{
	if (tname.length() == 3)
	{
		return (int)atof(tname.substr(1, 2).c_str());
	}
	else {
		for (int i = 0; i < 18; ++i)
		{
			if (tname == fyVertFenFuHaoArray[i])
			{
				return i;
			}
		}
	}
	return -1;
}

void getTileIndex( string filename , int hloc , int hlen , int vloc , int vlen , int& hindex , int& vindex )
{
	if (hlen == 3)
	{
		hindex = (int)atof(filename.substr(hloc + 1, 2).c_str());
		vindex = (int)atof(filename.substr(vloc + 1, 2).c_str());
	}
	else {
		string hstr = filename.substr(hloc, hlen);
		string vstr = filename.substr(vloc, vlen);
		hindex = -1;
		vindex = -1;
		for (int i = 0; i < 18; ++i)
		{
			if (vstr == fyVertFenFuHaoArray[i])
			{
				vindex = i;
				break;
			}
		}
		for (int i = 0; i < 36; ++i)
		{
			if (hstr == fyHoriFenFuHaoArray[i])
			{
				hindex = i;
				break;
			}
		}
	}
}


struct DirInfo
{
	string indir ;
	string inprefix ;
	string intail ;
	string dsprefix , dstail ;
	int ymdloc , ymdlen ;
	int htloc,htlen,vtloc,vtlen ;
	int useht0 , usevt0 , usehtcnt , usevtcnt ;
	string wgs84dir , albersdir ;
	string pngdir ;
	string outprefix , wgs84tail , alberstail ;
} ;

int mosaicWarpCutModisFy3HDF( vector<string>& infiles , 
	string& wgs84path ,
	string& alberspath , 
	string dsPrefix , string dsTail , 
	int numHortTiles , int numVertTiles , 
	string targetYmd ,
	int htloc ,int htlen , int vtloc , int vtlen ,
	int ymdloc , int ymdlen , int htile0 , int htile1 , 
	int vtile0 , int vtile1 , double fill ,
	int tileXSize , int tileYSize , GDALDataType dataType , 
	string lonfile , string latfile , 
	double tileWid , double tileHei , double valid0 , double valid1 ,
	string gdalwarp , string createvrt , 
	string orx , string ory , 
	string cutshpfile , 
	double modisleft , 
	double modistop 
	)
{

	bool isfy3 = false;
	if (htlen == 2) isfy3 = true;

	if (tileXSize == 0 || tileYSize == 0) {
		std::cout << "*** Error: Invalide tileXSize or tileYSize. out.";
		return 104;
	}

	int totalXSize = tileXSize * numHortTiles;
	int totalYSize = tileYSize * numVertTiles;

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");

	string outputMos = wgs84path + ".mos.tif";
	GDALDataset* outDataset = driver->Create(outputMos.c_str(), totalXSize, totalYSize, 1, dataType, 0);

	if (isfy3 == false)
	{
		if (modisleft == 0 || modistop == 0)
		{
			cout << "Error : modis left or top is zero. out." << endl;
			GDALClose(outDataset);
			return 105;
		}
		//250m ->231.6563583
		//500m ->463.3127165
		//1km  ->926.6254331
		double geotrans[6] = {0,0,0,0,0,0};
		geotrans[0] = modisleft;
		geotrans[3] = modistop;
		if (tileXSize == 1200)
		{
			geotrans[1] = 926.6254331;
			geotrans[5] = -926.6254331;
		}
		else if (tileXSize == 2400)
		{
			geotrans[1] = 463.3127165;
			geotrans[5] = -463.3127165;
		}
		else {
			geotrans[1] = 231.6563583;
			geotrans[5] = -231.6563583;
		}
		outDataset->SetGeoTransform(geotrans);
		{
			OGRSpatialReference osrs;
			osrs.importFromProj4("+proj=sinu +lon_0=0 +x_0=0 +y_0=0 +a=6371007.181 +b=6371007.181 +units=m +no_defs");
			char* sinuProj = 0;
			osrs.exportToWkt(&sinuProj);
			outDataset->SetProjection(sinuProj);
			CPLFree(sinuProj);
		}
	}

	GDALDataset* londs = 0;
	GDALDataset* latds = 0;

	bool needDeleteLonFile = false;
	bool needDeleteLatFile = false;
	if (lonfile == "" && isfy3 )
	{
		needDeleteLonFile = true;
		lonfile = outpath + ".lon.tif";
		londs = driver->Create(lonfile.c_str(), totalXSize, totalYSize, 1, GDT_Float32 , 0);
	}
	if (latfile == "" && isfy3 )
	{
		needDeleteLatFile = true;
		latfile = outpath + ".lat.tif";
		latds = driver->Create(latfile.c_str(), totalXSize, totalYSize, 1, GDT_Float32, 0);
	}


	int* buff = new int[totalXSize];
	float* llbuff = new float[totalXSize];
	for (int i = 0; i < totalXSize; ++i) {
		buff[i] = fill;
		llbuff[i] = -999.f ;
	}
	cout << "filling output datafiles..." << endl;
	for (int iy = 0; iy < totalYSize; ++iy)
	{
		outDataset->GetRasterBand(1)->RasterIO(GF_Write, 0 , iy , totalXSize, 1,
			buff, totalXSize, 1, GDT_Int32, 0, 0, 0);
		if( londs ) 
			londs->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, totalXSize, 1,
			llbuff, totalXSize, 1, GDT_Float32, 0, 0, 0);
		if( latds ) 
			latds->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, totalXSize, 1,
			llbuff, totalXSize, 1, GDT_Float32, 0, 0, 0);
		wft_term_progress(iy, totalYSize);
	}
	delete[] buff; buff = 0;
	delete[] llbuff; llbuff = 0;

	double irx = tileWid / tileXSize;
	double iry = tileHei / tileYSize;
	if (isfy3)
	{
		irx = 1002.228;
		iry = 1002.228; 
	}

	cout << "Mosaicing ... " << endl;
	int* bufftile = new int[tileXSize*tileYSize];
	float* lonbuff = new float[tileXSize * tileYSize];
	float* latbuff = new float[tileXSize * tileYSize];
	for (int ifile = 0; ifile < infiles.size(); ++ifile)
	{
		string filepath = infiles[ifile];
		string filename = wft_base_name(filepath);
		string tymd = filename.substr(ymdloc, ymdlen);
		if (tymd == targetYmd)
		{
			int hi, vi;
			getTileIndex(filename, htloc, htlen, vtloc, vtlen, hi, vi);
			if (hi >= htile0 && hi <= htile1 && vi >= vtile0&& vi <= vtile1)
			{
				string dspath = dsPrefix + infiles[ifile] + dsTail;
				GDALDataset* dsone = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
				if (dsone == 0)
				{
					cout << "Error : can not open " << dspath << " . go next." << endl;
					continue;
				}
				int xsize1 = dsone->GetRasterXSize();
				int ysize1 = dsone->GetRasterYSize();
				if (xsize1 != tileXSize || ysize1 != tileYSize)
				{
					std::cout << "*** Error: " << infiles[ifile] << " has a differenct xsize,ysize . out.";
					GDALClose(dsone);
					continue;
				}

				dsone->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, tileXSize, tileYSize, bufftile, tileXSize, tileYSize,
					GDT_Int32, 0, 0, 0);
				outDataset->GetRasterBand(1)->RasterIO(GF_Write, (hi - htile0)*tileXSize, (vi - vtile0)*tileYSize, tileXSize, tileYSize,
					bufftile, tileXSize, tileYSize, GDT_Int32, 0, 0, 0);
				GDALClose(dsone);

				if (londs || latds )
				{
					cout << "Making lon lat for tile ... " << endl;
					int allsize = tileXSize * tileYSize;
					for (int i = 0; i < allsize; ++i)
					{
						lonbuff[i] = -999.f;
						latbuff[i] = -999.f;
					}


					if (isfy3)
					{
						float ffLeftBottomX0 = ( hi - 18)*tileXSize*irx + irx / 2;
						float ffLeftBottomY0 = (8 - vi )*tileYSize*iry + iry / 2;

						for (int iy = 0; iy < tileYSize; ++iy)
						{
							for (int ix = 0; ix < tileXSize; ++ix)
							{
								int i1d = iy*tileXSize + ix;
								if (bufftile[i1d] >= valid0 && bufftile[i1d] <= valid1)
								{
									double mapx = ffLeftBottomX0 + ix * irx;
									double mapy = ffLeftBottomY0 + (tileYSize - 1 - iy)*iry;

									double mapx2 = mapx / earthRadius;
									double mapy2 = mapy / earthRadius;
									//=SQRT(1-(O2/4)*(O2/4)-(O3/2)*(O3/2))
									double tempZ = sqrt(1.0 - (mapx2 / 4)*(mapx2 / 4) - (mapy2 / 2)*(mapy2 / 2));
									//= 2 * ATAN(O5*O2 / 2 / (2 * O5*O5 - 1)) * 180 / 3.14;
									double templon = 2 * atan(tempZ*mapx2 / 2 / (2 * tempZ*tempZ - 1)) * 180 / M_PI;
									//=ASIN(O5*O3)*180/3.14
									double templat = asin(tempZ*mapy2) * 180 / M_PI;
									if (londs) {
										lonbuff[i1d] = templon;
									}
									if (latds)
									{
										latbuff[i1d]=templat;
									}
								}
							}
							wft_term_progress(iy, tileYSize);
						}

					}
					else {
						for (int iy = 0; iy < tileYSize; ++iy)
						{
							for (int ix = 0; ix < tileXSize; ++ix)
							{
								int i1d = iy*tileXSize + ix;
								if (bufftile[i1d] >= valid0 && bufftile[i1d] <= valid1)
								{
									double templat = 90.0 - ( vi * tileYSize + iy - 0.5) * iry;
									double templon = ((hi - 18) * tileXSize + ix + 1.5) * irx / cos( templat * M_PI / 180.0);
									//这个地方modis校正后的数据总是和矢量数据相比偏西边一点。


									if (londs) {
										lonbuff[i1d] = templon;
									}
									if (latds)
									{
										latbuff[i1d] = templat;
									}
								}
							}
							wft_term_progress(iy, tileYSize);
						}
					}
					if( londs ) londs->GetRasterBand(1)->RasterIO(GF_Write, (hi - htile0)*tileXSize, (vi - vtile0)*tileYSize, tileXSize, tileYSize,
						lonbuff, tileXSize, tileYSize, GDT_Float32, 0, 0, 0);
					if( latds) latds->GetRasterBand(1)->RasterIO(GF_Write, (hi - htile0)*tileXSize, (vi - vtile0)*tileYSize, tileXSize, tileYSize,
						latbuff, tileXSize, tileYSize, GDT_Float32, 0, 0, 0);
				}
				string filename1 = wft_base_name(infiles[ifile]);
				std::cout << "mosic " << filename1 << " done." << endl;
			}
		}
		
	}
	delete[] bufftile; bufftile = 0;
	delete[] lonbuff; lonbuff = 0;
	delete[] latbuff; latbuff = 0;

	GDALClose(outDataset);
	if( londs != 0 ) GDALClose(londs);
	if (latds != 0) GDALClose(latds);

	if (isfy3 && wft_test_file_exists(latfile)==false )
	{
		cout << "Error : lat file is not exist " << latfile << endl;
		return 201;
	}
	if (isfy3 && wft_test_file_exists(lonfile)==false )
	{
		cout << "Error : lon file is not exist " << lonfile << endl;
		return 202;
	}

	if (wft_test_file_exists(outputMos)==false )
	{
		cout << "Error : output mosaic file is not exist " << outputMos << endl;
		return 203;
	}

	//warping
	string lut = wft_int2str(valid0 - 1) + ":-9999," + wft_int2str(valid0) + ":" + wft_int2str(valid0) + ","  + wft_int2str(valid1) + ":" + wft_int2str(valid1) + "," + wft_int2str(valid1 + 1) + ":-9999";
	cout << "Warping wgs84 ... " << endl;
	if (isfy3)
	{
		string cmd1 = createvrt + " -type llfile -in " + outputMos + " -lonfile " + lonfile
			+ " -latfile " + latfile
			+ " -temproot " + outputMos
			+ " -llnodata -999 "
			+ " -pdtnodata -9999"
			+ " -pdtlut " + lut
			+ " -outtif " + outpath
			+ " -gdalwarp " + gdalwarp
			+ " -orx " + orx
			+ " -ory " + ory;

		int res1 = system(cmd1.c_str());
		cout << "fy3 warp result:" << res1 << endl;
	}
	else {
		string tr = "";
		tr = " -tr 0.01 0.01 ";
		string cmd1 = gdalwarp + " -srcnodata \"-3000\" -dstnodata \"-3000\" -t_srs EPSG:4326  " + tr + outputMos + "  " + outpath;
		int res1 = system(cmd1.c_str());
		cout << "modis warp result:" << res1 << endl;
	}

	if (wft_test_file_exists(outpath)==false )
	{
		cout << "Error : output  file is not exist " << outpath << endl;
		return 204;
	}

	if (needDeleteLonFile) wft_remove_file(lonfile);
	if (needDeleteLatFile) wft_remove_file(latfile);

	string usecut = "";
	if (cutshpfile != "")
	{
		usecut = " -crop_to_cutline -q -cutline " + cutshpfile;
	}
	//gdalwarp - overwrite - t_srs "+proj=aea +ellps=krass +lon_0=105 +lat_1=25 +lat_2=47" - srcnodata - 9999 - dstnodata - 9999 ndvichina.tif alberschina.tif
	string outputalbers = alberspath ;
	string cmd2 = gdalwarp + " -overwrite "
		+ " -t_srs \"+proj=aea +ellps=krass +lon_0=105 +lat_1=25 +lat_2=47\" "
		+ " -srcnodata -9999 "
		+ " -dstnodata -9999 "
		+ usecut 
		+ " -of GTiff "
		+ outpath
		+ " "
		+ outputalbers;
	int res2 = system(cmd2.c_str());
	cout << "warp albers result:" << res2 << endl;

	if (wft_test_file_exists(outputalbers)==false )
	{
		cout << "Error : output albers  file is not exist " << outputalbers << endl;
		return 205;
	}
	return 100;
}


void plotAlbersTifAndDb( string albersfile , string pngfile 
	, pdtname 
	, string ymd0str , string ymd1str , string pid , int ymd0 
	, string plottem  )
{
	if (g_txt != "" )
	{
		GDALDataset * tempds =(GDALDataset *) GDALOpen( albersfile.c_str() ,
			 GA_ReadOnly);
		int axsize = tempds->GetRasterXSize();
		int aysize = tempds->GetRasterYSize();
		GDALClose(tempds);
		int xspace = 1;
		int yspace = 1;
		if (axsize > 2000)
		{
			xspace = axsize / 2000;
		}
		if (aysize > 2000)
		{
			yspace = aysize / 2000;
		}
		string txtfile = pngfile + ".tmp";
		string cmd3 = g_txt + " -in " + outputalbers  
			+ " -out " + txtfile + " -valid0 -2000 " 
			+ " -valid1 10000 "
			+ " -scale 0.0001 " 
			+ " -offset 0 "
			+ " -xspace " + wft_int2str(xspace)
			+ " -yspace " + wft_int2str(yspace);

		int res3 = system(cmd3.c_str());
		cout << " image2xyz result : " << res3 << endl;
		if (wft_test_file_exists(txtfile) == false)
		{
			cout << "Error : failed to make " + txtfile << endl;
			return 206;
		}

		vector<string> varVec, repVec;
		varVec.push_back("{{{OUTFILE}}}");
		varVec.push_back("{{{INFILE}}}");
		varVec.push_back("{{{PNAME}}}");
		varVec.push_back("{{{YMD}}}");
		varVec.push_back("{{{YMD1}}}");
		varVec.push_back("{{{PTIME}}}");

		repVec.push_back(pngfile);
		repVec.push_back(txtfile);
		repVec.push_back(pdtname);
		repVec.push_back(ymd0str);
		repVec.push_back(ymd1str);
		repVec.push_back(wft_current_datetimestr());

		string plotfile = pngfile + ".plot";
		wft_create_file_by_template_with_replacement(plotfile, plottem
			, varVec, repVec);
		if (wft_test_file_exists(plotfile) == false)
		{
			cout << "Error : failed to make " + plotfile << endl;
			return 207;
		}

		string cmd4 = g_gnuplot + " " + plotfile;
		int res4 = system(cmd4.c_str());
		cout << "gnuplot result : " << res4 << endl;
		if (wft_test_file_exists(pngfile) == false)
		{
			cout << "Error : failed to make " + pngfile << endl;
			return 208;
		}
		wft_remove_file(txtfile);


		if (pid != "")
		{
			//insert db
			string fname = wft_base_name(albersfile);
			g_db.insertimg("tb_product_data",
				pid  ,
				albersfile,
				fname,
				pngfile,
				ymd0
			);
		}
	}
}


void processModishdf( vector<DirInfo>& dirVec )
{

	for(int idir = 0 ; idir < dirVec.size() ; ++ idir )
	{
		DirInfo di = dirVec[idir] ;
		vector<string> selectedfiles ;
		wft_get_allSelectedFiles( di.indir , di.inprefix , 
			di.intail , -1 , "" , selectedfiles ) ;
		if( selectedfiles.size() == 0 ) continue ;
		vector<int> ymdVec ;
		for(int ifile = 0 ; ifile < selectedfiles.size() ; ++ ifile )
		{
			string filepath = selectedfiles[ifile];
			string filename = wft_base_name(filepath);
			string ymd0 = filename.substr(di.ymdloc, di.ymdlen );
			int ymd1 = (int)atof(ymd0.c_str());
			if (di.ymdlen == 7)
			{
				int tyear = ymd1 / 1000;
				int tday = ymd1 % 1000;
				int rmon, rday;
				wft_convertdayofyear2monthday(tyear, tday, rmon, rday);
				ymd1 = tyear * 10000 + rmon * 100 + rday;
			}
			ymdVec.push_back(ymd1) ;
		}


		for (int ifile = 0; ifile < selectedfiles.size() ; ++ ifile )
		{
			string filepath = selectedfiles[ifile];
			string filename = wft_base_name(filepath);
			int ymd1 = ymdVec[ifile] ;
			int hi, vi;
			getTileIndex(filename, di.htloc , di.htlen ,
			 di.vtloc, di.vtlen , hi, vi);
			if (hi >= di.useht0 && hi <= di.useht0+di.usehtcnt-1
				 && vi >= di.usevt0  && vi <= di.usevt0+di.usevtcnt-1 )
			{
				string wgs84path = di.wgs84dir + di.outprefix
				 + wft_int2str(ymd1) + di.wgs84tail ;
				 string alberspath = di.albersdir + di.outprefix
				 + wft_int2str(ymd1) + di.alberstail ;

				if (wft_test_file_exists(wgs84path) == false)
				{
					cout << "making " << wgs84path << endl;
					int tileXSize, tileYSize;
					GDALDataType dataType;
					{
						string ds0path = di.dsprefix + filepath + di.dstail ;
						GDALDataset* tempDs = 
							(GDALDataset*)GDALOpen(ds0path.c_str(), GA_ReadOnly);
						if (tempDs == 0)
						{
							cout << "Error : opening " << filepath << endl;
							continue;
						}
						tileXSize = tempDs->GetRasterXSize();
						tileYSize = tempDs->GetRasterYSize();
						dataType = tempDs->GetRasterBand(1)->GetRasterDataType();
						GDALClose(tempDs);
					}
					string displayYmd = wft_ymd_int2str(ymd1);
					int res = mosaicWarpCutModisFy3HDF(
						selectedfiles , 


						);
					cout << " result: " << res << endl;
				}//for1
			}

			cout<<ifile<<"/"<<allfiles.size() <<endl ;
		}





	}

	
}
void processModisLevel3( )
{

}



void processFy3hdf( )
{

}
void processFy3Level3( )
{

}







int main(int argc, char** argv)
{
	printCurrentTime();
	std::cout << "Description: do noaa ndvi monitor all in one. " << std::endl;
	std::cout << "Version 1a . by wangfengdev@163.com 2017-12-18." << std::endl;
	std::cout << "Version 1.1a . add database staff ." << std::endl;
	std::cout << "Version 1.2a . month ymd0 bugfixed." << std::endl;

	if (argc == 1)
	{
		std::cout << "*** sample call: ***" << std::endl;
		std::cout << "noaa_ndvi_aio_monitor startup.txt" << endl;
		std::cout << "*** startup.txt example ***" << std::endl;
		cout << "" << endl;
		cout << "" << endl;
		cout << "" << endl;
		return 101;
	}

	std::cout << "processing..." << std::endl;
	string startup = argv[1];
	GDALAllRegister();

	std::ifstream ifs(startup.c_str());
	DynamicJsonBuffer jsonBuffer;
	JsonVariant jsonRoot = jsonBuffer.parseObject(ifs);
	ifs.close();

	///////////////////////////////////////////////////////////////////////
	//
	//
	//  Initializing parameters.
	//
	///////////////////////////////////////////////////////////////////////
	g_gdaltranslate = jsonRoot["gdaltranslate"].asString() ;
	g_gdalwarp = jsonRoot["gdalwarp"].asString();

	g_shp_china = jsonRoot["wgs84-china-shp"].asString();

	g_plottem_ndvi = jsonRoot["ndvi-plot-tem"].asString();
	g_plottem_juping = jsonRoot["jp-plot-tem"].asString();

	g_txt = jsonRoot["totxt"].asString();
	g_gnuplot = jsonRoot["gnuplot"].asString();

	

	//
	g_xspace = jsonRoot["noaa"]["xspace"].asString();
	g_yspace = jsonRoot["noaa"]["yspace"].asString();

	g_ncdir = jsonRoot["noaa"]["ncdir"].asString();
	g_l2dir = jsonRoot["noaa"]["l2daydir"].asString();
	g_l3dir = jsonRoot["noaa"]["l3dir"].asString();
	g_pngdir = jsonRoot["noaa"]["pngdir"].asString();
	g_ncprefix = jsonRoot["noaa"]["ncprefix"].asString();
	g_nctail = jsonRoot["noaa"]["nctail"].asString();
	g_pixelarea = jsonRoot["noaa"]["pixelarea"].as<double>();
	g_noaa_qamask = jsonRoot["noaa"]["qamask"].as<short>();

	g_noaa_from_ymd = jsonRoot["noaa"]["fromymd"].as<int>();
	g_noaa_to_ymd = jsonRoot["noaa"]["toymd"].as<int>();

	g_cmaxname = jsonRoot["noaa"]["cmaxname"].asString();
	g_cavename = jsonRoot["noaa"]["cavename"].asString();
	g_jpmaxname = jsonRoot["noaa"]["jpmaxname"].asString();
	g_jpavename = jsonRoot["noaa"]["jpavename"].asString();

	g_pidmaxarr[0] = jsonRoot["noaa"]["pidmaxten"].as<int>(); 
	g_pidmaxarr[1] = jsonRoot["noaa"]["pidmaxmon"].as<int>();
	g_pidmaxarr[2] = jsonRoot["noaa"]["pidmaxsea"].as<int>();
	g_pidmaxarr[3] = jsonRoot["noaa"]["pidmaxyea"].as<int>();

	g_pidmaxarrjp[0] = jsonRoot["noaa"]["pidmaxtenjp"].as<int>();
	g_pidmaxarrjp[1] = jsonRoot["noaa"]["pidmaxmonjp"].as<int>();
	g_pidmaxarrjp[2] = jsonRoot["noaa"]["pidmaxseajp"].as<int>();
	g_pidmaxarrjp[3] = jsonRoot["noaa"]["pidmaxyeajp"].as<int>();

	g_pidavearr[0] = jsonRoot["noaa"]["pidaveten"].as<int>();
	g_pidavearr[1] = jsonRoot["noaa"]["pidavemon"].as<int>();
	g_pidavearr[2] = jsonRoot["noaa"]["pidavesea"].as<int>();
	g_pidavearr[3] = jsonRoot["noaa"]["pidaveyea"].as<int>();

	g_pidavearrjp[0] = jsonRoot["noaa"]["pidavetenjp"].as<int>();
	g_pidavearrjp[1] = jsonRoot["noaa"]["pidavemonjp"].as<int>();
	g_pidavearrjp[2] = jsonRoot["noaa"]["pidaveseajp"].as<int>();
	g_pidavearrjp[3] = jsonRoot["noaa"]["pidaveyeajp"].as<int>();

	{
		JsonArray& qhcarr = jsonRoot["qhcdir"];
		JsonArray& qhcloc = jsonRoot["qhcymdloc"];
		for (int i = 0; i < 8; ++i)
		{
			g_qhcdir[i] = qhcarr[i].asString();
			g_qhcymdloc[i] = qhcloc[i].as<int>();
		}

	}


	//db staff
	string host = jsonRoot["host"];
	string user = jsonRoot["user"];
	string pwd = jsonRoot["pwd"];
	string db = jsonRoot["db"];

	g_db.connect(host, user, pwd, db);



	///////////////////////////////////////////////////////////////////////
	//
	//
	//  noaa ndvi dir
	//
	///////////////////////////////////////////////////////////////////////
	processNoaaNcDaily( );
	processNoaaLevel3( );



	///////////////////////////////////////////////////////////////////////
	//
	//
	//  modis ndvi dir
	//
	///////////////////////////////////////////////////////////////////////
	processModishdf( );
	processModisLevel3( );



	///////////////////////////////////////////////////////////////////////
	//
	//
	//  fy3 ndvi dir
	//
	///////////////////////////////////////////////////////////////////////
	processFy3hdf( );
	processFy3Level3( );



	std::cout << "All done." << std::endl;
	printCurrentTime();
	return 0;
}




/*

*/


