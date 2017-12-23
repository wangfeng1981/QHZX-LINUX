// fy4_aio_monitor.cpp : all in one 处理 
//fy4 sst qa product, fy4 lpw (low,mid,hig,tot) , fy4 olr , fy4 lst
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

#define QHC_DAY 0
#define QHC_FIV 1
#define QHC_TEN 2
#define QHC_MON 3
#define QHC_SEA 4
#define QHC_YEA 5


using namespace std;
using std::cout;

string g_gdaltranslate = "gdal_translate";
string g_gdalwarp = "gdalwarp";
string g_txt = "";
string g_gnuplot = "";
string g_createvrt = "" ;

int g_from_ymd = 0;
int g_to_ymd = 0;
string g_not_deletetmp = "" ;

//mysql operation
wDb g_db;



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

void getDateRange( int ymdi , string type  , int& ymd0 , int& ymd1 ) 
{
	if( type == "fiv" )
	{
		getfivdaysStartEnd(ymdi , ymd0 , ymd1 ) ;
	}else if( type == "ten" )
	{
		gettendaysStartEnd(ymdi , ymd0 , ymd1 ) ;
	}else if( type == "mon" )
	{
		getmonStartEnd(ymdi , ymd0 , ymd1 ) ;
	}else if( type == "sea" )
	{
		getseaStartEnd(ymdi , ymd0 , ymd1 ) ;
	}else if( type == "yea" )
	{
		getyeaStartEnd(ymdi , ymd0 , ymd1 ) ;
	}else
	{
		ymd0 = ymdi ;
		ymd1 = ymdi ;
	} 
}

struct AreaIndex
{
	inline AreaIndex() :maskid(0), sumval(0) {};
	int maskid;
	double sumval;
};


//计算积雪面积指数
//used
int step21_computeSpaceValidAverageValue(
	string maskfile, string datafile ,
	double lon0 , double lon1 , double lat0 , double lat1 ,
	double valid0 , double valid1 ,
	double scale , double offset,
	 vector<AreaIndex>& result)
{
	cout<<"computing space average ... "<<endl ;
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

	double trans[6] ;
	datads->GetGeoTransform(trans) ;
	
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
	double* buffer = new double[bsize];
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
		datads->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, x0, 1, buffer, x0, 1, GDT_Float64, 0, 0, 0);
		double xlat = trans[3] + trans[5] * iy ;
		//这段代码可以进一步优化，直接通过经纬度范围反算出读取数据的范围，然后计算平均值。
		if( xlat >= lat0 && xlat <= lat1 )
		{
			for (int it = 0; it < x0 ; ++it)
			{
				double xlon = trans[0] + trans[1] * it ;
				if( xlon>=lon0 && xlon<=lon1 )
				{
					int maskid = 1 ;
					if( maskds ) maskid = (int)maskbuffer[it];
					hasmask[maskid] = 1;
					if (maskid > 0 && buffer[it] >= valid0 && buffer[it] <= valid1 )
					{
						areaArr[maskid] += buffer[it] * scale + offset ;
						++ validCount ;
					}
				}
				
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
//used
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




//used
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




//处理多天加和数据 这个函数完了！
//used
int stepall_processCombineAver(vector<string>& selfiles,
	string outpath , string dsprefix , string dstail , double valid0, double valid1 )
{
	int todayYmd = wft_current_dateymd_int();
	cout << "making average combined product " << outpath << " ... " << endl;
	if (selfiles.size() == 0) return 0;

	//get output parameters.
	int rasterXSize = 0;
	int rasterYSize = 0;
	GDALDataType datatype  ;
	{
		string dspath = dsprefix + selfiles[0] + dstail ;
		GDALDataset* tds = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
		if (tds == 0)
		{
			return 10;
		}
		rasterXSize = tds->GetRasterXSize();
		rasterYSize = tds->GetRasterYSize();
		datatype = tds->GetRasterBand(1)->GetRasterDataType() ;
		GDALClose(tds);
	}

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(outpath.c_str(), rasterXSize, rasterYSize, 1, datatype, 0);
	if (outds == 0)
	{
		cout << "Error : failed create " << outpath << endl;
		return 10;
	}

	{
		string dspath = dsprefix + selfiles[0] + dstail ;
		GDALDataset* tds = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
		double adfGeoTrans[6]  ;
		tds->GetGeoTransform(adfGeoTrans) ;
		outds->SetGeoTransform(adfGeoTrans);
		outds->SetProjection(tds->GetProjectionRef()) ;
		GDALClose(tds);
	}

	double* buffer = new double[rasterXSize];
	double* outBuffer = new double[rasterXSize];
	short* cntBuffer = new short[rasterXSize];

	if (buffer == 0 || outBuffer == 0 || cntBuffer == 0 )
	{
		cout << "Error : can not alloc more memories ." << endl;
		if (buffer) delete[] buffer;
		if (outBuffer) delete[] outBuffer;
		if (cntBuffer) delete[] cntBuffer;
		GDALClose(outds);
		wft_remove_file(outpath);
		return 30;
	}

	int dsCount = selfiles.size();
	for(int iy = 0 ; iy < rasterYSize ; ++ iy )
	{
		memset( cntBuffer , 0 , sizeof(short) * rasterXSize ) ;
		for (int ids = 0; ids < dsCount; ++ids)
		{
			string dspath = dsprefix + selfiles[ids] + dstail ;
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
			dstemp->GetRasterBand(1)->RasterIO(GF_Read, 0, iy , rasterXSize, 1,
				buffer, rasterXSize, 1, GDT_Float64, 0, 0, 0);
			for (int it = 0; it < rasterXSize; ++it)
			{
				if (buffer[it] >= valid0 && buffer[it] <= valid1 )
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
				}else if( cntBuffer[it] == 0 )
				{
					outBuffer[it] = buffer[it];
				}
			}
			GDALClose(dstemp);
		}
		for (int it = 0; it < rasterXSize ; ++it)
		{
			if (cntBuffer[it] > 0 )
			{
				outBuffer[it] = outBuffer[it]/cntBuffer[it] ;//bugfixed 2017-12-18
			}
		}
		outds->GetRasterBand(1)->RasterIO(GF_Write, 0, iy , rasterXSize, 1,
				outBuffer, rasterXSize, 1, GDT_Float64, 0, 0, 0);
		wft_term_progress(iy , rasterYSize) ;
	}

	delete[] buffer; buffer = 0;
	delete[] outBuffer; outBuffer = 0;
	delete[] cntBuffer; cntBuffer = 0;
	GDALClose(outds);
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
	/*
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
	*/
	return 100;
}





struct AreaParams
{
	string outprefix ;
	string outtail ;
	string areapid ;
	string plottem ;
	string outdir  ;
	string regional ;// "1" used.
	string lon0 , lon1 , lat0 , lat1 ;
	string valid0,valid1,scale,offset ;
	string pdtname ;
	string xspace , yspace ;
} ;


struct Fy4ProduceParams
{
	string enabled ;//"1"
	string pid ,xlpid  ;
	string pcode ;
	string daytype ;//day , fiv , ten , mon , sea , yea
	string indir ;
	string inprefix ;
	string intail ;
	string dsprefix , dstail ;
	int ymdloc , ymdlen , lonloc ;
	string outdir , outtail ;//combined file out dir .
	string wgs84dir;
	string outprefix , wgs84tail  ;
	string outrx , outry ;
	string srcnodata , dstnodata ;
	string pdtname ;
	vector<AreaParams> areaVec ;

	//used for space average value.for xlpid
	double lon0, lon1 , lat0 , lat1 , valid0 , valid1 , scale , offset ;
} ;



//used
int step2_warpWgs84( string infile , string wgs84file , Fy4ProduceParams& pp )
{
	string filename = wft_base_name(infile) ;
	string lonstr = "104.7" ;
	if( filename.find("995") != string::npos )
	{
		lonstr = "99.5" ;
	}

	cout << "converting  -> geos ..." << endl;
	string geosfile = wgs84file + ".geos.tif";
	string cmd1 = g_gdaltranslate
		+ " -a_srs \"+proj=geos +h=35785863 +a=6378137.0 +b=6356752.3 +lon_0=" + lonstr
		+ " +no_defs\" -a_ullr -5496000 5496000 5496000 -5496000 " + infile
		+ " " + geosfile;
	cout << cmd1 << endl;
	system(cmd1.c_str());

	if( wft_test_file_exists(geosfile) == false )
	{
		cout<<"failed to make "<<geosfile<<endl ;
		return 10 ;
	}
	//wgs84
	string cmd2 = g_gdalwarp + " -overwrite -t_srs \"+proj=latlong +ellps=WGS84 +pm=0\" -tr 0.04 0.04 "
		+ " -tr "+pp.outrx + " " + pp.outry 
		+ " -srcnodata " + pp.srcnodata
		+ " -dstnodata " + pp.dstnodata 
		+ " -r near "
		+ " -te 0 -89 180 89 " + geosfile + " " + wgs84file;
	cout << cmd2 << endl;
	system(cmd2.c_str());
	wft_remove_file(geosfile);

	if( wft_test_file_exists(wgs84file) )
	{
		return 100 ;
	}else 
	{
		cout<<"error failed to cut wgs84 file "<< wgs84file<<endl ;
		return 30 ;
	}
}



//used
void step4_plot( string wgs84file , string pngfile 
	, int ymd0 , int ymd1 
	, AreaParams& ap  )
{
	string ymd0str = wft_convert_ymdi2y_m_d(ymd0) ;
	string ymd1str = wft_convert_ymdi2y_m_d(ymd1) ;

	if (g_txt != "" )
	{
		GDALDataset * tempds =(GDALDataset *) GDALOpen( wgs84file.c_str() ,
			 GA_ReadOnly);
		int axsize = tempds->GetRasterXSize();
		int aysize = tempds->GetRasterYSize();
		GDALClose(tempds);
		string txtfile = pngfile + ".tmp";
		string cmd3 = g_txt + " -in " + wgs84file  
			+ " -out " + txtfile 
			+ " -valid0 " + ap.valid0 
			+ " -valid1 " + ap.valid1 
			+ " -scale " + ap.scale 
			+ " -offset " + ap.offset
			+ " -xspace " + ap.xspace
			+ " -yspace " + ap.yspace ;
		if( ap.regional == "1" )
		{
			cmd3 += " -x0 " + ap.lon0 
			+ " -x1 " + ap.lon1 
			+ " -y0 " + ap.lat0
			+ " -y1 " + ap.lat1  ;
		}
			
		int res3 = system(cmd3.c_str());
		cout << " image2xyz result : " << res3 << endl;
		if (wft_test_file_exists(txtfile) == false)
		{
			cout << "Error : failed to make " + txtfile << endl;
			return ;
		}

		string ymd01str = ymd0str ;
		if( ymd0 != ymd1 ) {
			ymd01str = "From " + ymd0str + " to " + ymd1str ;
		}

		vector<string> varVec, repVec;
		varVec.push_back("{{{OUTFILE}}}");
		varVec.push_back("{{{INFILE}}}");
		varVec.push_back("{{{PNAME}}}");
		varVec.push_back("{{{YMD}}}");
		varVec.push_back("{{{YMD1}}}");
		varVec.push_back("{{{YMD01}}}");
		varVec.push_back("{{{PTIME}}}");

		repVec.push_back(pngfile);
		repVec.push_back(txtfile);
		repVec.push_back(ap.pdtname);
		repVec.push_back(ymd0str);
		repVec.push_back(ymd1str);
		repVec.push_back(ymd01str);
		repVec.push_back(wft_current_datetimestr());

		string plotfile = pngfile + ".plot";
		wft_create_file_by_template_with_replacement(plotfile, ap.plottem
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
		if( g_not_deletetmp!="1" ) wft_remove_file(txtfile);
	}
}


//used
void processFy4OneProduct( Fy4ProduceParams& pp )
{
	if( pp.enabled != "1" )
	{
		cout<<pp.pdtname<<" is not enabled ."<<endl ;
		return ;
	}
	vector<string> selectedfiles ;
	wft_get_allSelectedFiles( pp.indir , pp.inprefix , 
		pp.intail , -1 , "" , selectedfiles ) ;
	if( selectedfiles.size() == 0 )
	{
		cout<<"product "<<pp.pcode<<" no files for producing."<<endl ;
		return ;
	} 
	vector<int> ymdVec ;
	for(int ifile = 0 ; ifile < selectedfiles.size() ; ++ ifile )
	{
		string filepath = selectedfiles[ifile];
		string filename = wft_base_name(filepath);
		int ymdx = wft_str2int( filename.substr( pp.ymdloc , 8 ) ) ;
		ymdVec.push_back(ymdx) ;
	}
	for (int ifile = 0; ifile < selectedfiles.size() ; ++ ifile )
	{
		string filepath = selectedfiles[ifile];
		string filename = wft_base_name(filepath);
		int ymd0 , ymd1 ;
		int ymdi = ymdVec[ifile] ;
		getDateRange( ymdi , pp.daytype , ymd0 , ymd1 ) ;
		if( ymd0 < g_from_ymd || ymd1 > g_to_ymd ) continue ;
		vector<string> pickedfiles ;
		int combcnt = selectedFilesByYmdRange(selectedfiles, ymdVec, pickedfiles, ymd0, ymd1);
		if( combcnt == 0 )
		{
			cout<<"no files from "<<ymd0<<" to "<<ymd1<<endl ;
			continue ;
		}
		string lonstr = "" ; 
		if( pp.lonloc >= 0 )
		{
			lonstr = filename.substr(pp.lonloc , 4 ) + "_" ;
		}
		string combfile = pp.outdir + pp.outprefix + lonstr + wft_int2str(ymdi) + pp.outtail ;
		if( wft_test_file_exists(combfile) == false )
		{
			cout<<"making "<<combfile<<endl ;
			stepall_processCombineAver( pickedfiles , 
				 combfile ,
				 pp.dsprefix , 
				 pp.dstail , 
				 pp.valid0 , 
				 pp.valid1 ) ;
		}

		if( pp.wgs84dir != "" )
		{//坐标变换wgs84

			string wgs84file = pp.wgs84dir + pp.outprefix +  wft_int2str(ymd0) + pp.wgs84tail  ;
			if( wft_test_file_exists(combfile) &&
			 wft_test_file_exists(wgs84file) == false )
			{
				step2_warpWgs84( combfile , wgs84file , pp ) ;
			}

			if( pp.xlpid != "" )
			{//计算区域平均值
				if (wft_test_file_exists(wgs84file) && 
					g_db.hasxl( pp.xlpid , "1" , ymd0 )==false )
				{
					vector<AreaIndex> aivec ;
					step21_computeSpaceValidAverageValue( "" , wgs84file ,
						pp.lon0 , pp.lon1 , pp.lat0 , pp.lat1 ,
						pp.valid0 , pp.valid1 , pp.scale , pp.offset , 
						aivec ) ;
					string xuliefile = wgs84file + ".xl.txt" ;
					saveSnowAreaIndex(xuliefile, pp.xlpid , aivec,   ymd0) ;
				}
			}

			for( int ia = 0 ; ia < pp.areaVec.size() ; ++ ia )
			{
				AreaParams ap = pp.areaVec[ia] ;
				string pngfile = ap.outdir + ap.outprefix + wft_int2str(ymd0) + ap.outtail ;
				if( wft_test_file_exists(wgs84file) && 
					wft_test_file_exists(pngfile)==false )
				{
					step4_plot( 
						wgs84file , 
						pngfile , 
						ymd0 , ymd1 ,
						ap
						) ;
				}
				if( wft_test_file_exists(pngfile) && 
					g_db.hasimg(ap.areapid,ymd0) ==false )
				{
					//入库
					string fname = wft_base_name(wgs84file);
					g_db.insertimg("tb_product_data",
						ap.areapid  ,
						wgs84file ,
						fname,
						pngfile,
						ymd0
					);
				}else
				{
					cout<<"png image already in db."<<endl ;
				}
			}
		}
	}

}

vector<AreaParams> loadAreaParams( JsonArray& arr)
{
	vector<AreaParams> vec ;
	for(int i = 0 ; i<arr.size() ; ++ i )
	{
		AreaParams ap ;
		JsonObject& obj = arr[i]  ;
		cout<<"loading area "<<obj["pdtname"]<<endl ;
		ap.pdtname = obj["pdtname"].asString() ;
		ap.outprefix = obj["outprefix"].asString() ;
		ap.outtail = obj["outtail"].asString() ;
		ap.areapid = obj["areapid"].asString() ;
		ap.plottem = obj["plottem"].asString() ;
		ap.outdir = obj["outdir"].asString() ;
		ap.regional = obj["regional"].asString() ;
		ap.lon0 = obj["lon0"].asString() ;
		ap.lon1 = obj["lon1"].asString() ;
		ap.lat0 = obj["lat0"].asString() ;
		ap.lat1 = obj["lat1"].asString() ;
		ap.valid0 = obj["valid0"].asString() ;
		ap.valid1 = obj["valid1"].asString() ;
		ap.scale = obj["scale"].asString() ;
		ap.offset = obj["offset"].asString() ;
		ap.xspace = obj["xspace"].asString() ;
		ap.yspace = obj["yspace"].asString() ;

		vec.push_back(ap) ;
	}
	return vec ;
}

Fy4ProduceParams buildPPbyJsonObj( JsonObject& obj )
{
	Fy4ProduceParams pp ;
	cout<<"loading "<<obj["pdtname"]<<endl ;
	pp.pdtname= obj["pdtname"].asString() ; 
	pp.enabled = obj["enabled"].asString() ;
	pp.pid = obj["pid"].asString() ;
	pp.xlpid = obj["xlpid"].asString() ;
	pp.pcode = obj["pcode"].asString() ;
	pp.daytype = obj["daytype"].asString() ;//day fiv ten mon sea yea 
	pp.indir = obj["indir"].asString() ;
	pp.inprefix = obj["inprefix"].asString() ;
	pp.intail = obj["intail"].asString() ;
	pp.dsprefix = obj["dsprefix"].asString() ;
	pp.dstail = obj["dstail"].asString() ;
	pp.ymdloc = obj["ymdloc"].as<int>() ;
	pp.ymdlen = obj["ymdlen"].as<int>() ;
	pp.lonloc = obj["lonloc"].as<int>() ;
	pp.outdir = obj["outdir"].asString() ;
	pp.outtail= obj["outtail"].asString() ; 
	pp.wgs84dir= obj["wgs84dir"].asString() ; 
	pp.outprefix = obj["outprefix"].asString() ; 
	pp.wgs84tail = obj["wgs84tail"].asString() ; 
	pp.outrx = obj["outrx"].asString() ; 
	pp.outry= obj["outry"].asString() ; 
	
	pp.srcnodata =  obj["srcnodata"].asString() ; 
	pp.dstnodata =  obj["dstnodata"].asString() ; 

	pp.areaVec = loadAreaParams( obj["area_array"].asArray() ) ;
	pp.lon0= obj["lon0"].as<double>() ; 
	pp.lon1 = obj["lon1"].as<double>() ; 
	pp.lat0 = obj["lat0"].as<double>() ; 
	pp.lat1 = obj["lat1"].as<double>() ; 
	pp.valid0= obj["valid0"].as<double>() ;  
	pp.valid1 = obj["valid1"].as<double>() ; 
	pp.scale = obj["scale"].as<double>() ; 
	pp.offset = obj["offset"].as<double>() ; 

	return pp ;
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

	cout<<"read json files. begin analyze..."<<endl ;

	///////////////////////////////////////////////////////////////////////
	//
	//
	//  Initializing parameters.
	//
	///////////////////////////////////////////////////////////////////////
	g_gdaltranslate = jsonRoot["gdaltranslate"].asString() ;
	g_gdalwarp = jsonRoot["gdalwarp"].asString();

	cout<<"gdalwarp "<<g_gdalwarp<<endl ;
 
	g_txt = jsonRoot["totxt"].asString();
	g_gnuplot = jsonRoot["gnuplot"].asString();
	g_createvrt = jsonRoot["createvrt"].asString();
	
	cout<<"g_txt "<<g_txt<<endl ;

	g_from_ymd = jsonRoot["fromymd"].as<int>();
	g_to_ymd = jsonRoot["toymd"].as<int>();
	g_not_deletetmp = jsonRoot["not_deletetmp"].asString();

	cout<<"g_not_deletetmp "<<g_not_deletetmp<<endl ;
	
	//db staff
	string host = jsonRoot["host"];
	string user = jsonRoot["user"];
	string pwd = jsonRoot["pwd"];
	string db = jsonRoot["db"];

	cout<<"host "<<host<<endl ;
	g_db.connect(host, user, pwd, db);


	///////////////////////////////////////////////////////////////////////
	//
	//
	//  all fy4 product
	//
	///////////////////////////////////////////////////////////////////////
	vector<Fy4ProduceParams> ppVec  ;

	JsonArray& pparr = jsonRoot["product_array"].asArray() ;
	for( int i = 0 ; i<pparr.size() ; ++ i )
	{
		Fy4ProduceParams pp = buildPPbyJsonObj( pparr[i] ) ;
		ppVec.push_back(pp) ;
		cout<<"load producing params for "<<pp.pdtname<<endl ;
	}

	cout<<"begin processing..."<<endl ;
	for(int ify4 = 0 ; ify4 < ppVec.size() ; ++ ify4 )
	{
		processFy4OneProduct( ppVec[ify4] ) ;
	}

	std::cout << "All done." << std::endl;
	printCurrentTime();
	return 0;
}
