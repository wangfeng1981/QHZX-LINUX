// ndvi_aio_monitor.cpp : 定义控制台应用程序的入口点。
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
string g_createvrt = "" ;

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
int computeSpaceValidAverageValue(string maskfile, string datafile ,
	 vector<AreaIndex>& result)
{
	cout<<"computing indices ... "<<endl ;
	double areaArr[255];
	short hasmask[255];
	memset(hasmask, 0, sizeof(short) * 255);
	memset(areaArr, 0, sizeof(double) * 255);

	int x0 = 0;
	int y0 = 0;

	GDALDataset* maskds = 0 ; 
	if( maskfile != "" )
	{
		maskds = (GDALDataset*)GDALOpen(maskfile.c_str(), GA_ReadOnly);
		if (  maskds == 0)
		{
			cout << "error : failed to open mask file " << maskfile << endl;
			return 10;
		}
		x0 = maskds->GetRasterXSize();
		y0 = maskds->GetRasterYSize();
	}
	GDALDataset* datads = (GDALDataset*)GDALOpen(datafile.c_str(), GA_ReadOnly);
	if( datads == 0 )
	{
		cout << "error : failed to open data file " << datafile << endl;
		if( maskds	) GDALClose(maskds);
		return 20;
	}
	
	int x1 = datads->GetRasterXSize();
	int y1 = datads->GetRasterYSize();
	if( maskds == 0 )
	{
		x0 = x1 ;
		y0 = y1 ;
	}

	if (x0 != x1 || y0 != y1)
	{
		cout << "error : data and mask size are different ." << x0 << "," << y0 << ";" << x1 << "," << y1 << endl;
		GDALClose(maskds);
		GDALClose(datads);
		return 30;
	}
	int bsize = x0  ;
	char* maskbuffer = new char[bsize];
	short* buffer = new short[bsize];
	if (maskbuffer == 0 || buffer == 0)
	{
		cout << "error : can not alloc memory." << endl;
		if( maskds ) GDALClose(maskds);
		if( datads ) GDALClose(datads);
		if (maskbuffer) delete[] maskbuffer;
		if (buffer) delete[] buffer;
		return 40;
	}
	int validCount = 0 ;
	for(int iy = 0 ; iy < y0 ; ++ iy )
	{
		if( maskds ) 
			maskds->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, x0, 1, maskbuffer, x0, 1, GDT_Byte, 0, 0, 0);
		datads->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, x0, 1, buffer, x0, 1, GDT_Int16, 0, 0, 0);
		for (int it = 0; it < x0 ; ++it)
		{
			int maskid = 1 ;
			if( maskds ) maskid = (int)maskbuffer[it];
			hasmask[maskid] = 1;
			if (maskid > 0 && buffer[it] >= -2000 && buffer[it] <= 10000 )
			{
				areaArr[maskid] += buffer[it] * 0.0001 ;
				++ validCount ;
			}
		}
	}
	if( maskds ) GDALClose(maskds);
	GDALClose(datads);
	delete[] maskbuffer;
	delete[] buffer;

	for (int i = 1; i < 255; ++i)
	{
		if ( hasmask[i] > 0 )
		{
			if( validCount == 0 )
			{
				AreaIndex ai;
				ai.maskid = i;
				ai.sumval = -9999 ;
				result.push_back(ai);
			}else 
			{
				AreaIndex ai;
				ai.maskid = i;
				ai.sumval = areaArr[i] / validCount;
				result.push_back(ai);
			}

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
	if( filepath != "" ) 
	{
		ofstream ofs(filepath.c_str());
		for (int i = 0; i < vec.size(); ++i)
		{
			ofs << pid << "\t" << vec[i].maskid << "\t" << vec[i].sumval << "\t" << ymd8 << endl;
			if( pid != "" ) g_db.insertxl("tb_xulie_data",pid,vec[i].maskid,vec[i].sumval,ymd8);
		}
		ofs.close();
	}
	else if( pid != "" )
	{
		for (int i = 0; i < vec.size(); ++i)
		{
			g_db.insertxl("tb_xulie_data",pid,vec[i].maskid,vec[i].sumval,ymd8);
		}
	}
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
	string outpath, bool useMax, bool useinputProjection )
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

	if( useinputProjection )
	{
		cout<<"use input projection."<<endl ;
		string dspath = selfiles[0];
		GDALDataset* tds = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
		double adfGeoTrans[6]  ;
		tds->GetGeoTransform(adfGeoTrans) ;
		outputDataset->SetGeoTransform(adfGeoTrans);
		outputDataset->SetProjection(tds->GetProjectionRef()) ;
		GDALClose(tds);
	}else 
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
					processCombineMaxOrAver(selectedfiles, outpath, true , false );
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
					processCombineMaxOrAver(selectedfiles, outpath1, false , false );
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
					processCombineMaxOrAver(selectedfiles, outpath, true , false );
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
					processCombineMaxOrAver(selectedfiles, outpath1, false , false );
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
					processCombineMaxOrAver(selectedfiles, outpath, true, false );
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
					processCombineMaxOrAver(selectedfiles, outpath1, false , false );
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
					processCombineMaxOrAver(selectedfiles, outpath, true , false );
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
					processCombineMaxOrAver(selectedfiles, outpath1, false , false );
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
void getfy3ymdRange( string filepath , int& ymd0 , int& ymd1  )
{
	int pos = filepath.rfind( "HAM_" ) ;
	int postd = filepath.rfind( "_AOTD_") ;
	int posmon = filepath.rfind("_AOAM_") ;
	string ymd8 = filepath.substr( pos + 4 , 8 ) ;
	ymd1 = wft_str2int(ymd8) ;
	int tyear = ymd1/10000 ;
	int rmon = (ymd1 % 10000)/100 ;
	int rday = ymd1%100 ;
	if( postd != string::npos )
	{//tendays
		if( rday == 10 )
		{
			rday = 1 ;
		}else if( rday == 20 )
		{
			rday = 11 ;
		}else 
		{
			rday = 21 ;
		}
		ymd0 = tyear * 10000 + rmon * 100 + rday ;
	}else 
	{//month
		ymd0 = tyear * 10000 + rmon * 100 + 1;
	}
}
void getmodymdRange( string filepath , int& ymd0 , int& ymd1  )
{
	int pos = filepath.rfind( ".A" ) ;
	int posmon = filepath.rfind("MOD13A3") ;
	string ymd7 = filepath.substr( pos + 2 , 7) ;
	int tymd1 = wft_str2int(ymd7) ;
	int tyear = tymd1 / 1000;
	int tday = tymd1 % 1000;
	int rmon, rday;
	wft_convertdayofyear2monthday(tyear, tday, rmon, rday);
	ymd0 = tyear * 10000 + rmon * 100 + rday;
	if( posmon == string::npos )
	{//16days
		if( rmon == 12 && rday >= 16 ) ymd1 = tyear*10000 + 1231 ;
		else 
			{
				int tday16 = tday + 15 ;
				wft_convertdayofyear2monthday(tyear, tday16, rmon, rday);
				ymd1 = tyear * 10000 + rmon * 100 + rday;
			}
	}else 
	{//month
		ymd0 = tyear * 10000 + rmon * 100 + 1;
		getmonStartEnd( ymd0 , ymd0 , ymd1 ) ;
	}
}

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


struct ProduceParams
{
	string pid ,xlpid, jppid ;
	string pcode ;
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
	string outrx , outry ;

	string fy3lonfile , fy3latfile ;
	double modisleft , modistop ;
	string pdtname ;
	int pixelres ;//250,500,1000
} ;

struct Fy3bCorrParam
{
	string maskfile ;
	double slope , inter ;
	int usemask ;
};
vector<Fy3bCorrParam> g_fy3bcorrParamsVec ;


int step1_mosaicModisFy3( 
	vector<string>& allfiles, 
	string mosfile ,
	ProduceParams& pp ,  
	string targetYmd7or8 
	)
{
	int tileXSize = 0 ;
	int tileYSize = 0 ;
	int ymdlen = targetYmd7or8.length() ;
	cout << "Mosaicing ... " << endl;
	if( allfiles.size()==0 )
	{
		cout<<"error allfiles size == 0 ."<<endl ;
		return 5 ;
	}
	{
		string temppath = pp.dsprefix + allfiles[0] + pp.dstail ;
		GDALDataset* ds = (GDALDataset*)GDALOpen( temppath.c_str() , 
			GA_ReadOnly ) ;
		if( ds == 0 ) 
		{
			cout<<"error failed to open file for getting data size "<<allfiles[0]<<endl ;
			return 10 ;
		}
		tileXSize = ds->GetRasterXSize() ;
		tileYSize = ds->GetRasterYSize() ;
		cout<<"tile size : "<<tileXSize <<","<<tileYSize<<endl ;
		GDALClose(ds) ;
	}
	int totalXSize = tileXSize * pp.usehtcnt ;
	int totalYSize = tileYSize * pp.usevtcnt ;
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outDataset = driver->Create(mosfile.c_str(),
		 totalXSize, totalYSize, 1, GDT_Int16 , 0);


	int* bufftile = new int[tileXSize*tileYSize];
	for (int ifile = 0; ifile < allfiles.size(); ++ifile)
	{
		string filepath = allfiles[ifile];
		string filename = wft_base_name(filepath);
		string tymd = filename.substr(pp.ymdloc, pp.ymdlen);
		if (tymd == targetYmd7or8)
		{
			int hi, vi;
			getTileIndex(filename, pp.htloc , pp.htlen , pp.vtloc , pp.vtlen , hi, vi);
			if (hi >= pp.useht0  && hi <= pp.useht0 + pp.usehtcnt-1 
				 && vi >= pp.usevt0 && vi <= pp.usevt0 + pp.usevtcnt - 1 )
			{
				string dspath = pp.dsprefix + allfiles[ifile] + pp.dstail ;
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
					cout<<"another tile size : "<<xsize1 <<","<<ysize1<<endl ;
					std::cout << "*** Error: " << allfiles[ifile] << " has a different xsize,ysize . out.";
					GDALClose(dsone);
					continue;
				}

				dsone->GetRasterBand(1)->RasterIO(GF_Read, 0, 0, tileXSize, tileYSize, 
					bufftile, tileXSize, tileYSize,
					GDT_Int32, 0, 0, 0);
				outDataset->GetRasterBand(1)->RasterIO(GF_Write, (hi - pp.useht0 )*tileXSize,
					 (vi - pp.usevt0 )*tileYSize, tileXSize, tileYSize,
					bufftile, tileXSize, tileYSize, GDT_Int32, 0, 0, 0);
				GDALClose(dsone);
				string filename1 = wft_base_name(allfiles[ifile]);
				std::cout << "mosic " << filename1 << " done." << endl;
			}
		}
		
	}
	delete[] bufftile; bufftile = 0;
	GDALClose(outDataset);
	return 100 ;
}

int step2_warpandcutWgs84( string mosfile , string wgs84file , ProduceParams& pp )
{
	string tempwgs84file = wgs84file + ".tmp.tif" ;
	if ( pp.ymdlen == 8 )
	{//fy3
		string cmd1 = g_createvrt + " -type llfile -in " + mosfile + " -lonfile " + pp.fy3lonfile
			+ " -latfile " + pp.fy3latfile
			+ " -temproot " + tempwgs84file
			+ " -llnodata -999 "
			+ " -pdtnodata -9999 "
			+ " -pdtlut " + "-3001:-9999,-3000:-3000,10000:10000,10001:-9999"
			+ " -outtif " + tempwgs84file
			+ " -gdalwarp " + g_gdalwarp
			+ " -orx " + pp.outrx 
			+ " -ory " + pp.outry ;//use 0.01

		int res1 = system(cmd1.c_str());
		cout << "fy3 warp result:" << res1 << endl;
	}
	else {
		//modis
		//250m ->231.6563583
		//500m ->463.3127165
		//1km  ->926.6254331
		double geotrans[6] = {0,0,0,0,0,0};
		geotrans[0] = pp.modisleft;
		geotrans[3] = pp.modistop;
		if ( pp.pixelres == 1000 )
		{
			geotrans[1] = 926.6254331;
			geotrans[5] = -926.6254331;
		}
		else if ( pp.pixelres == 500 )
		{
			geotrans[1] = 463.3127165;
			geotrans[5] = -463.3127165;
		}
		else {
			geotrans[1] = 231.6563583;
			geotrans[5] = -231.6563583;
		}
		GDALDataset* ds =(GDALDataset*) GDALOpen(mosfile.c_str() , GA_Update ) ;
		ds->SetGeoTransform(geotrans);
		{
			OGRSpatialReference osrs;
			osrs.importFromProj4("+proj=sinu +lon_0=0 +x_0=0 +y_0=0 +a=6371007.181 +b=6371007.181 +units=m +no_defs");
			char* sinuProj = 0;
			osrs.exportToWkt(&sinuProj);
			ds->SetProjection(sinuProj);
			CPLFree(sinuProj);
		}
		GDALClose(ds) ;
		string tr = "";
		tr = " -tr " + pp.outrx + " " + pp.outry + " " ;//1km use 0.01 , 0.01
		string cmd1 = g_gdalwarp + " -srcnodata \"-3000\" -dstnodata \"-9999\" -t_srs EPSG:4326  " 
			+ tr + mosfile + "  " + tempwgs84file;
		int res1 = system(cmd1.c_str());
		cout << "modis warp result:" << res1 << endl;
	}

	//shp cut
	string cmd2 = g_gdalwarp + " -overwrite "
		+ " -crop_to_cutline -q -cutline " + g_shp_china 
		+ " -srcnodata -9999 "
		+ " -dstnodata -9999 "
		+ " -of GTiff "
		+ tempwgs84file
		+ " "
		+ wgs84file;
	int res2 = system(cmd2.c_str() ) ;
	cout<<"shp cut :"<<res2<<endl ;
	wft_remove_file(tempwgs84file) ;
	if( wft_test_file_exists(wgs84file) )
	{
		return 100 ;
	}else 
	{
		cout<<"error failed to cut wgs84 file "<< tempwgs84file<<endl ;
		return 30 ;
	}
}


int step3_wgs84toalbers( string wgs84file , string albersfile )
{
	string cmd2 = g_gdalwarp + " -overwrite "
		+ " -t_srs \"+proj=aea +ellps=krass +lon_0=105 +lat_1=25 +lat_2=47\" "
		+ " -srcnodata -9999 "
		+ " -dstnodata -9999 "
		+ " -of GTiff "
		+ wgs84file
		+ " "
		+ albersfile;
	int res2 = system(cmd2.c_str());
	cout << "warp albers result:" << res2 << endl;
	if( wft_test_file_exists(albersfile) )
	{
		cout<<"wgs84 to albers ok."<<endl ;
		return 100 ;
	}else 
	{
		cout<<"error wgs84 to albers "<<albersfile<<endl ;
		return 10 ;
	}
}


void step4_plotAlbersTifAndDb( string albersfile , string pngfile 
	, string pdtname 
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
		string cmd3 = g_txt + " -in " + albersfile  
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
			return ;
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
			return ;
		}

		string cmd4 = g_gnuplot + " " + plotfile;
		int res4 = system(cmd4.c_str());
		cout << "gnuplot result : " << res4 << endl;
		if (wft_test_file_exists(pngfile) == false)
		{
			cout << "Error : failed to make " + pngfile << endl;
			return ;
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

void step11_makewgs84grid( string wgs84file , string gridfile )
{
	cout<<"making grid "<<gridfile<<endl ;
	string cmd = g_gdalwarp + " -overwrite -te -180 -90 180 90 -tr 0.05 0.05 "
			+ " -srcnodata -9999 "
			+ wgs84file + " " + gridfile ;
	system( cmd.c_str() ) ;
}

int step12_linearcorr( string datafile , string corrfile , string maskfile ,
		int usemask , double slope , double inter ,
		double valid0 , double valid1 )
{
	GDALDataset* inDs = (GDALDataset*)GDALOpen(datafile.c_str(), GA_ReadOnly);

	if (inDs == 0)
	{
		cout << "Error: can not open " << datafile << endl;
		return 10;
	}
	GDALDataset* maskds = 0 ;
	if( maskfile != "" )
	{
		maskds = (GDALDataset*)GDALOpen(maskfile.c_str(), GA_ReadOnly);
		if (maskds == 0)
		{
			cout << "Error: can not open " << maskfile << endl;
			return 20;
		}
	}
	
	int xSize = inDs->GetRasterXSize();
	int ySize = inDs->GetRasterYSize();
	GDALDataType dtype = inDs->GetRasterBand(1)->GetRasterDataType() ;
	if( maskds != 0 )
	{
		if (xSize != maskds->GetRasterXSize() || 
			ySize != maskds->GetRasterYSize())
		{
			std::cout << "Error : sizes are not equal between infile and fit type postion. out." << std::endl;
			GDALClose(inDs);
			GDALClose(maskds);
			return 10;
		}
	}
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(corrfile.c_str(), 
		xSize, ySize, 1, dtype, 0);
	{
		double trans[6] ;
		inDs->GetGeoTransform(trans) ;
		outds->SetGeoTransform(trans) ;
		outds->SetProjection(inDs->GetProjectionRef()) ;
	}

	double* inbuffer = new double[xSize];
	int* maskbuffer = new int[xSize];
	for (int iy = 0; iy < ySize; ++iy)
	{
		inDs->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, xSize, 1,
			inbuffer, xSize, 1, GDT_Float64, 0, 0, 0);
		if( maskds != 0 ) 
			maskds->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, xSize, 1,
				maskbuffer , xSize, 1, GDT_Int32, 0, 0, 0);
		for (int ix = 0; ix < xSize; ++ix)
		{
			if (inbuffer[ix] >= valid0 && inbuffer[ix] <= valid1)
			{
				if( maskds != 0 )
				{
					if( usemask != maskbuffer[ix] ) continue ;
				}
				double newval = inbuffer[ix] * slope + inter;
				inbuffer[ix] = newval;
			}
		}
		outds->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, xSize, 1,
			inbuffer, xSize, 1, GDT_Float64, 0, 0, 0);
		wft_term_progress(iy, ySize);
	}
	delete[] inbuffer; inbuffer = 0;
	delete[] maskbuffer; maskbuffer = 0;
	GDALClose(inDs);
	GDALClose(outds);
	if( maskds ) GDALClose(maskds);
	return 100;
}

int getfy3bCorrIndex( int ymd )
{
	int mon = (ymd%10000)/100 ;
	if( mon < 3 )
	{
		return 3 ;
	}else if( mon < 6 )
	{
		return 0 ;
	}else if( mon < 9 )
	{
		return 1 ;
	}else if( mon < 12 )
	{
		return 2 ;
	}else 
	{
		return 3 ;
	}
}



void processModishdf( vector<ProduceParams>& ppVec )
{

	for(int idir = 0 ; idir < ppVec.size() ; ++ idir )
	{
		ProduceParams pp = ppVec[idir] ;
		vector<string> selectedfiles ;
		wft_get_allSelectedFiles( pp.indir , pp.inprefix , 
			pp.intail , -1 , "" , selectedfiles ) ;
		if( selectedfiles.size() == 0 ) continue ;
		vector<int> ymdVec , ymdVec1 ;
		for(int ifile = 0 ; ifile < selectedfiles.size() ; ++ ifile )
		{
			string filepath = selectedfiles[ifile];
			string filename = wft_base_name(filepath);
			int ymd0 , ymd1 ;
			if( pp.ymdlen==8 )
			{
				getfy3ymdRange( filename , ymd0 , ymd1) ;
			}else 
			{
				getmodymdRange( filename , ymd0 , ymd1 ) ;
			}
			ymdVec.push_back(ymd0) ;
			ymdVec1.push_back(ymd1) ;
		}
		for (int ifile = 0; ifile < selectedfiles.size() ; ++ ifile )
		{
			string filepath = selectedfiles[ifile];
			string filename = wft_base_name(filepath);
			int ymd0 = ymdVec[ifile] ;
			int ymd1 = ymdVec1[ifile] ;
			int hi, vi;
			getTileIndex(filename, pp.htloc , pp.htlen ,
			 pp.vtloc, pp.vtlen , hi, vi);
			if (hi >= pp.useht0 && hi <= pp.useht0+pp.usehtcnt-1
				 && vi >= pp.usevt0  && vi <= pp.usevt0+pp.usevtcnt-1 )
			{
				string wgs84path = pp.wgs84dir + pp.outprefix
				 + wft_int2str(ymd0) + pp.wgs84tail ;
				
				if (wft_test_file_exists(wgs84path) == false)
				{
					string mosfile = wgs84path + ".mos.tif" ;
					cout << "making " << mosfile << endl;
					string targetymd7or8 = filename.substr( pp.ymdloc , pp.ymdlen ) ;
					step1_mosaicModisFy3( selectedfiles , 
						mosfile , 
						pp , 
						targetymd7or8
						) ;

					if( wft_test_file_exists(mosfile) )
					{
						cout << "making " << wgs84path << endl;
						step2_warpandcutWgs84( mosfile , wgs84path , pp ) ;
					}
					
				}
				if( pp.jppid != ""  )
				{
					string gridfile = wgs84path + ".grid.tif" ;
					if( wft_test_file_exists(wgs84path) && wft_test_file_exists(gridfile)==false )
					{
						step11_makewgs84grid( wgs84path , gridfile ) ;
					} 
					string corrfile = gridfile + ".corr.tif" ;
					if( wft_test_file_exists(gridfile)  && wft_test_file_exists(corrfile)==false)
					{
						int corri = getfy3bCorrIndex(ymd0) ;
						cout<<"fy3b "<<ymd0<<" use corr "<<corri<<endl ;
						step12_linearcorr( gridfile , corrfile ,
							g_fy3bcorrParamsVec[corri].maskfile , 
							g_fy3bcorrParamsVec[corri].usemask , 
							g_fy3bcorrParamsVec[corri].slope , 
							g_fy3bcorrParamsVec[corri].inter , 
							-2000 , 
							10000 
							 ) ;
					}
					string jpfile = corrfile + ".jp.tif" ;
					if( wft_test_file_exists(corrfile) && wft_test_file_exists(jpfile) == false)
					{
						int qhctype = QHC_TEN ;
						if( pp.pcode.find("sea") != string::npos )
						{
							qhctype = QHC_SEA ;
						}else if( pp.pcode.find("mon") != string::npos )
						{
							qhctype = QHC_MON ;
						}else if( pp.pcode.find("yea") != string::npos )
						{
							qhctype = QHC_YEA ;
						}
						string qhc = findQhc(QHC_MAX, qhctype, ymd0);//check
						if (wft_test_file_exists(qhc))
						{
							imageASubtractB(corrfile, qhc, jpfile);
						}
						else
						{
							cout << "Error : not found qhc for :" << corrfile << endl;
						}
					}
					string jppngfile = jpfile + ".png" ;
					if (wft_test_file_exists(jpfile) && wft_test_file_exists(jppngfile) == false)
					{	
						cout<<"ploting and insert juping..."<<endl ;
						dowarpPlotDb(jpfile, jppngfile, 
							ymd0, ymd1, 
							pp.jppid , "0", pp.pdtname + " Anomalies" , g_plottem_juping , true  );
					}
				}
				
				//xulie
				if( pp.xlpid != "" )
				{
					cout<<"compute xulie..."<<endl ;
					if (wft_test_file_exists(wgs84path) && 
						g_db.hasxl( pp.xlpid , "1" , ymd0 )==false )
					{
						vector<AreaIndex> aivec ;
						computeSpaceValidAverageValue( "" , wgs84path , aivec ) ;//
						string xuliefile = wgs84path + ".xl.txt" ;
						saveSnowAreaIndex(xuliefile, pp.xlpid , aivec,   ymd0) ;
					}else
					{
						cout<<"---- ---- ---- no need update xulie data."<<endl ;
					}
				}
				

				string alberspath = pp.albersdir + pp.outprefix
				 + wft_int2str(ymd0) + pp.alberstail ;
				if( wft_test_file_exists(wgs84path) && wft_test_file_exists(alberspath)==false )
				{
					cout << "making " << alberspath << endl;
					step3_wgs84toalbers( wgs84path , alberspath ) ;
				}

				string pngfile = pp.pngdir + pp.outprefix 
								 + wft_int2str(ymd0) + pp.alberstail + ".png" ;
				if( wft_test_file_exists(pngfile)==false &&  wft_test_file_exists(alberspath)  )
				{
					cout << "making " << pngfile << endl;
					step4_plotAlbersTifAndDb( alberspath , pngfile , pp.pdtname,
						wft_convert_ymdi2y_m_d(ymd0) , 
						wft_convert_ymdi2y_m_d(ymdVec1[ifile]) , 
						pp.pid , 
						ymd0 , 
						g_plottem_ndvi ) ;	
				}

				if( g_db.hasimg(pp.pid , ymd0) == false && wft_test_file_exists(pngfile) ) 
				{
					//insert db
					string fname = wft_base_name(alberspath);
					g_db.insertimg("tb_product_data",
						pp.pid  ,
						alberspath,
						fname,
						pngfile,
						ymd0
					);
				}
			}
			cout<<ifile<<"/"<<selectedfiles.size() <<endl ;
		}
	}

}
void processModisLevel3( vector<ProduceParams>& ppVec )
{
	for( int ipp = 0 ; ipp < ppVec.size() ; ++ ipp )
	{
		ProduceParams pp = ppVec[ipp] ;
		int ymdloc = pp.ymdloc  ;
		vector<string> allfiles;
		vector<int> ymdVec;
		string inprefix = pp.inprefix;
		string intail = pp.intail ;
		wft_get_allSelectedFiles( pp.indir , inprefix, intail, -1, "", allfiles);
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
			int ymd0, ymd1;
			int pos1 = pp.wgs84tail.find(".sea.") ;
			if( pos1 != string::npos )
			{
				getseaStartEnd(ymdi, ymd0, ymd1);
			}else 
			{
				pos1 = pp.wgs84tail.find(".yea.") ;
				if( pos1 != string::npos )
				{
					getyeaStartEnd(ymdi, ymd0, ymd1);
				}else 
				{
					cout<<"error out tail not contain .sea. nor .yea. continue."<<endl ;
					continue ;
				}
			}
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;

			string wgs84file = pp.wgs84dir + pp.outprefix + wft_int2str(ymd0) + pp.wgs84tail ;
			if( wft_test_file_exists(wgs84file) == false )
			{
				processCombineMaxOrAver(selectedfiles, wgs84file, true , true );
			}
			if( pp.jppid != ""  )
			{
				string gridfile = wgs84file + ".grid.tif" ;
				if( wft_test_file_exists(wgs84file) && wft_test_file_exists(gridfile)==false )
				{
					step11_makewgs84grid( wgs84file , gridfile ) ;
				} 
				string corrfile = gridfile + ".corr.tif" ;
				if( wft_test_file_exists(gridfile)  && wft_test_file_exists(corrfile)==false)
				{
					int corri = getfy3bCorrIndex(ymd0) ;
					cout<<"fy3b "<<ymd0<<" use corr "<<corri<<endl ;
					step12_linearcorr( gridfile , corrfile ,
						g_fy3bcorrParamsVec[corri].maskfile , 
						g_fy3bcorrParamsVec[corri].usemask , 
						g_fy3bcorrParamsVec[corri].slope , 
						g_fy3bcorrParamsVec[corri].inter , 
						-2000 , 
						10000 
						 ) ;
				}
				string jpfile = corrfile + ".jp.tif" ;
				if( wft_test_file_exists(corrfile) && wft_test_file_exists(jpfile) == false)
				{
					int qhctype = QHC_TEN ;
					if( pp.pcode.find("sea") != string::npos )
					{
						qhctype = QHC_SEA ;
					}else if( pp.pcode.find("mon") != string::npos )
					{
						qhctype = QHC_MON ;
					}else if( pp.pcode.find("yea") != string::npos )
					{
						qhctype = QHC_YEA ;
					}
					string qhc = findQhc(QHC_MAX, qhctype, ymd0);//check
					if (wft_test_file_exists(qhc))
					{
						imageASubtractB(corrfile, qhc, jpfile);
					}
					else
					{
						cout << "Error : not found qhc for :" << corrfile << endl;
					}
				}
				string jppngfile = jpfile + ".png" ;
				if (wft_test_file_exists(jpfile) && wft_test_file_exists(jppngfile) == false)
				{	
					cout<<"ploting and insert juping..."<<endl ;
					dowarpPlotDb(jpfile, jppngfile, 
						ymd0, ymd1, 
						pp.jppid , "0", pp.pdtname + " Anomalies" , g_plottem_juping , true  );
				}
			}
			
			if( wft_test_file_exists(wgs84file)  && g_db.hasxl(pp.xlpid , "1" , ymd0) == false )
			{
				cout<<"compute xulie..."<<endl ;
				vector<AreaIndex> aivec ;
				computeSpaceValidAverageValue( "" , wgs84file , aivec ) ;//
				string xuliefile = wgs84file + ".xl.txt" ;
				saveSnowAreaIndex(xuliefile, pp.xlpid , aivec,   ymd0) ;
			}else{
				cout<<"---- ---- ----- no need update xulie."<<endl ;
			}
			string albersfile = pp.albersdir + pp.outprefix +  wft_int2str(ymd0) + pp.alberstail ;
			if( wft_test_file_exists(albersfile) == false &&  wft_test_file_exists(wgs84file) )
			{
				step3_wgs84toalbers( wgs84file , albersfile ) ;
			}
			string pngfile = pp.pngdir + pp.outprefix + wft_int2str(ymd0) + pp.alberstail + ".png" ;
			if( wft_test_file_exists(albersfile)  &&  wft_test_file_exists(pngfile)==false )
			{
				step4_plotAlbersTifAndDb( albersfile , pngfile , pp.pdtname,
							wft_convert_ymdi2y_m_d(ymd0) , 
							wft_convert_ymdi2y_m_d(ymd1) , 
							pp.pid , 
							ymd0 , 
							g_plottem_ndvi ) ;
			}
			if( g_db.hasimg(pp.pid , ymd0) == false && wft_test_file_exists(pngfile) ) 
			{
				//insert db
				string fname = wft_base_name(albersfile);
				g_db.insertimg("tb_product_data",
					pp.pid  ,
					albersfile,
					fname,
					pngfile,
					ymd0
				);
			}
			cout<<"modfy3 level3 "<<i<<"/"<<numfiles<<endl ;
		}
	}
}



void processFy3hdf( )
{

}
void processFy3Level3( )
{

}




ProduceParams buildPPbyJsonObj( JsonObject& obj )
{
	ProduceParams pp ;
	pp.pcode = obj["pcode"].asString() ;
	pp.pid = obj["pid"].asString() ;
	pp.xlpid = obj["xlpid"].asString() ;
	pp.jppid = obj["jppid"].asString() ;
	pp.indir = obj["indir"].asString() ;
	pp.inprefix = obj["inprefix"].asString() ;


	pp.intail = obj["intail"].asString() ;
	pp.dsprefix = obj["dsprefix"].asString() ;
	pp.dstail = obj["dstail"].asString() ;

	pp.ymdloc = obj["ymdloc"].as<int>() ;
	pp.ymdlen = obj["ymdlen"].as<int>() ;
	pp.htloc = obj["htloc"].as<int>() ;

	pp.htlen = obj["htlen"].as<int>() ;
	pp.vtloc = obj["vtloc"].as<int>() ;
	pp.vtlen = obj["vtlen"].as<int>() ;


	pp.useht0 = obj["useht0"].as<int>() ;
	pp.usevt0 = obj["usevt0"].as<int>() ;
	pp.usehtcnt = obj["usehtcnt"].as<int>() ;

	pp.usevtcnt = obj["usevtcnt"].as<int>() ;


	pp.wgs84dir = obj["wgs84dir"].asString() ;
	pp.albersdir = obj["albersdir"].asString() ;

	pp.pngdir = obj["pngdir"].asString() ;
	pp.outprefix = obj["outprefix"].asString() ;

	pp.wgs84tail = obj["wgs84tail"].asString() ;
	pp.alberstail = obj["alberstail"].asString() ;

	pp.outrx = obj["outrx"].asString() ;
	pp.outry = obj["outry"].asString() ;

	pp.fy3lonfile = obj["fy3lonfile"].asString() ;
	pp.fy3latfile = obj["fy3latfile"].asString() ;

	pp.modisleft = obj["modisleft"].as<double>() ;
	pp.modistop = obj["modistop"].as<double>() ;

	pp.pdtname = obj["pdtname"].asString() ;
	pp.pixelres = obj["pixelres"].as<int>() ;


	return pp ;
}

Fy3bCorrParam buildFy3bCorrByJsonObj( JsonObject& obj )
{
	Fy3bCorrParam cp ;
	cp.maskfile = obj["maskfile"].asString() ;
	cp.usemask = obj["usemask"].as<int>() ;
	cp.slope = obj["slope"].as<double>() ;
	cp.inter = obj["inter"].as<double>() ;
	return cp ;
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
	g_createvrt = jsonRoot["createvrt"].asString();
	


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


	int donoaa = jsonRoot["donoaa"].as<int>() ;
	int domodis = jsonRoot["domodis"].as<int>() ;
	int dofy3 = jsonRoot["dofy3"].as<int>() ;




	///////////////////////////////////////////////////////////////////////
	//
	//
	//  noaa ndvi dir
	//
	///////////////////////////////////////////////////////////////////////
	if( donoaa==1 ) processNoaaNcDaily( );
	if( donoaa==1 ) processNoaaLevel3( );



	///////////////////////////////////////////////////////////////////////
	//
	//
	//  modis ndvi dir and fy3 ndvi dir in one 
	//
	///////////////////////////////////////////////////////////////////////
	vector<ProduceParams> ppVec , pp3Vec ;

	JsonArray& pparr = jsonRoot["modfy3_pparr"].asArray() ;
	for( int i = 0 ; i<pparr.size() ; ++ i )
	{
		ProduceParams pp = buildPPbyJsonObj( pparr[i] ) ;
		ppVec.push_back(pp) ;
		cout<<pp.pdtname<<endl ;
	}




	JsonArray& pparrl3 = jsonRoot["modfy3_level3_pparr"].asArray() ;
	for( int i = 0 ; i<pparrl3.size() ; ++ i )
	{
		ProduceParams pp = buildPPbyJsonObj( pparrl3[i] ) ;
		pp3Vec.push_back(pp) ;
		cout<<pp.pdtname<<endl ;
	}

	JsonArray& cpArr = jsonRoot["fy3b_corr_params"].asArray() ;
	for(int i =0 ; i<cpArr.size() ; ++ i )
	{
		Fy3bCorrParam cp = buildFy3bCorrByJsonObj( cpArr[i])  ;
		cout<<"fy3b corr mask file : "<<cp.maskfile<<endl ;
		g_fy3bcorrParamsVec.push_back( cp ) ;
	}



	if( domodis==1 ) processModishdf( ppVec );
	if( domodis==1 ) processModisLevel3( pp3Vec );




	std::cout << "All done." << std::endl;
	printCurrentTime();
	return 0;
}




/*

*/


