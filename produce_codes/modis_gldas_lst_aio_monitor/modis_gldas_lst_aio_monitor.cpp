// modis_gldas_lst_aio_monitor.cpp : 定义控制台应用程序的入口点。
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




using namespace std;
using std::cout;

string g_gdaltranslate = "gdal_translate";
string g_gdalwarp = "gdalwarp";

string g_txt = "";
string g_gnuplot = "";



//pname


 
//xspace yspace
string g_xspace = "1";
string g_yspace = "1";

int g_from_ymd = 0;
int g_to_ymd = 0;



//mysql operation
wDb g_db;



struct RegionData
{
	string code;
	string displayName,displayName2;
	string plottem;
	string x0, x1, y0, y1;
	string xspace, yspace;
	string pid;
	string valid0, valid1;
	string scale, offset;
};


vector<RegionData> g_modisRegionVec , g_gldasRegionVec ;


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






void makeCinfofile(string outpath, int combcnt)
{
	char buff[10];
	sprintf(buff, "c%04d", combcnt);
	string outpath2 = outpath + "." + string(buff) + ".txt";
	ofstream ofs(outpath2.c_str());
	ofs << combcnt << endl;
	ofs.close();
}

void dowarpPlotDb(string inpath, string pngpath ,  int ymd0, int ymd1, 
	vector<RegionData>& regionarr , string* pidarr )
{
	if (wft_test_file_exists(inpath) == false) return;

	string inname = wft_base_name(inpath);
	string ymdstr = wft_convert_ymdi2y_m_d(ymd0);
	string ymdstr1 = wft_convert_ymdi2y_m_d(ymd1);

	if (ymd0 != ymd1)
	{
		ymdstr = "From " + ymdstr + " to " + ymdstr1;
	}

	for (int ir = 0; ir < regionarr.size() ; ++ir)
	{
		RegionData rd = regionarr[ir];
		
		string pngpath1 = pngpath +  "." + rd.code + ".png";
		if (ir == 0) pngpath1 = pngpath;
		string txtfile = pngpath1 + ".tmp";

		string cmdtxt1 = g_txt + " -in " + inpath + " -out " + txtfile
			+ " -valid0 " + rd.valid0
			+ " -valid1 " + rd.valid1
			+ " -xspace " + rd.xspace
			+ " -yspace " + rd.yspace
			+ " -scale " + rd.scale
			+ " -offset " + rd.offset
			+ " -x0 " + rd.x0
			+ " -x1 " + rd.x1
			+ " -y0 " + rd.y0
			+ " -y1 " + rd.y1;
		cout << cmdtxt1 << endl;
		system(cmdtxt1.c_str());

		if ( wft_test_file_exists(txtfile) )
		{
			vector<string> vec1, vec2;
			vec1.push_back("{{{INFILE}}}");
			vec1.push_back("{{{OUTFILE}}}");
			vec1.push_back("{{{YMD}}}");
			vec1.push_back("{{{YMD1}}}");
			vec1.push_back("{{{PNAME}}}");

			vec2.push_back(txtfile);
			vec2.push_back(pngpath1);
			vec2.push_back(ymdstr);
			vec2.push_back(ymdstr1);
			vec2.push_back(rd.displayName);

			string plot1file = pngpath1 + ".plot";
			wft_create_file_by_template_with_replacement(plot1file, rd.plottem , vec1, vec2);
			string cmdplot1 = g_gnuplot + " " + plot1file;
			system(cmdplot1.c_str());
		}

		if (wft_test_file_exists(pngpath1))
		{
			cout << "the pid : " << pidarr[ir] << endl;
			//insert db
			string fname = wft_base_name(inpath);
			//db
			g_db.insertimg("tb_product_data",
			pidarr[ir] ,
			inpath,
			fname,
			pngpath1,
			ymd0
			);
		}
	}
}


void doModisPlotDb(string inpath1,string inpath2, string datapath , string pngdir , int ymd0, int ymd1,
	vector<RegionData>& regionarr, string* pidarr)
{
	if (wft_test_file_exists(inpath1) == false) return;
	if (wft_test_file_exists(inpath2) == false) return;

	string ymdstr = wft_convert_ymdi2y_m_d(ymd0);
	string ymdstr1 = wft_convert_ymdi2y_m_d(ymd1);

	if (ymd1 != ymd0)
	{
		ymdstr = "From " + ymdstr + " to " + ymdstr1;
	}

	for (int ir = 0; ir < regionarr.size(); ++ir)
	{
		string inname = wft_base_name(datapath);
		RegionData rd = regionarr[ir];
		string pngpath = pngdir + inname + "." + rd.code + ".png";
		if (wft_test_file_exists(pngpath)) continue;
		string txtfile1 = pngpath + ".1.tmp";
		string txtfile2 = pngpath + ".2.tmp";
		string cmdtxt1 = g_txt + " -in " + inpath1 + " -out " + txtfile1
			+ " -valid0 " + rd.valid0
			+ " -valid1 " + rd.valid1
			+ " -xspace " + rd.xspace
			+ " -yspace " + rd.yspace
			+ " -scale " + rd.scale
			+ " -offset " + rd.offset
			+ " -x0 " + rd.x0
			+ " -x1 " + rd.x1
			+ " -y0 " + rd.y0
			+ " -y1 " + rd.y1;
		cout << cmdtxt1 << endl;
		system(cmdtxt1.c_str());

		string cmdtxt2 = g_txt + " -in " + inpath2 + " -out " + txtfile2
			+ " -valid0 " + rd.valid0
			+ " -valid1 " + rd.valid1
			+ " -xspace " + rd.xspace
			+ " -yspace " + rd.yspace
			+ " -scale " + rd.scale
			+ " -offset " + rd.offset
			+ " -x0 " + rd.x0
			+ " -x1 " + rd.x1
			+ " -y0 " + rd.y0
			+ " -y1 " + rd.y1;
		cout << cmdtxt2 << endl;
		system(cmdtxt2.c_str());


		if (wft_test_file_exists(txtfile1) && wft_test_file_exists(txtfile2))
		{
			vector<string> vec1, vec2;
			vec1.push_back("{{{INFILE}}}");
			vec1.push_back("{{{OUTFILE}}}");
			vec1.push_back("{{{YMD}}}");
			vec1.push_back("{{{YMD1}}}");
			vec1.push_back("{{{PNAME}}}");
			vec1.push_back("{{{PNAME2}}}");
			vec1.push_back("{{{YMD2}}}");
			vec1.push_back("{{{INFILE2}}}");

			//PNAME2 YMD2 INFILE2

			vec2.push_back(txtfile1);
			vec2.push_back(pngpath);
			vec2.push_back(ymdstr);
			vec2.push_back(ymdstr1);
			vec2.push_back(rd.displayName);
			vec2.push_back(rd.displayName2);
			vec2.push_back(ymdstr);
			vec2.push_back(txtfile2);

			string plot1file = pngpath + ".plot";
			wft_create_file_by_template_with_replacement(plot1file, rd.plottem, vec1, vec2);
			string cmdplot1 = g_gnuplot + " " + plot1file;
			system(cmdplot1.c_str());
		}

		if (wft_test_file_exists(pngpath))
		{
			cout << "the pid : " << pidarr[ir] << endl;
			//insert db
			string fname = wft_base_name(inpath1);
			//db
			g_db.insertimg("tb_product_data",
			pidarr[ir],
			inpath1,
			fname,
			pngpath,
			ymd0
			);
		}
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




string modisymd(string & filename , int& ymd )
{
	ymd = 19000101;
	int pos = filename.rfind(".A");
	if (pos == string::npos) return "19000101";
	else
	{
		pos = pos + 2;
		int yd7 = wft_str2int(filename.substr(pos, 7));
		int year = yd7 / 1000;
		int doy = yd7 % 1000;
		int month, day;
		wft_convertdayofyear2monthday(year, doy, month , day);
		ymd = year * 10000 + month * 100 + day;
		return wft_int2str(ymd);
	}
}





//处理多天加和数据
int processCombineAver(vector<string>& selfiles,
	string outpath, int minval , int maxval , bool isgldas )
{
	int todayYmd = wft_current_dateymd_int();
	cout << "making average " << outpath << " ... " << endl;
	if (selfiles.size() == 0) return 0;

	//get output parameters.
	int rasterXSize = 0;
	int rasterYSize = 0;
	string dspath = selfiles[0];
	GDALDataset* tds = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
	if (tds == 0)
	{
		return 10;
	}
	rasterXSize = tds->GetRasterXSize();
	rasterYSize = tds->GetRasterYSize();

	GDALDataType tdt = GDT_Int16;
	if (isgldas) tdt = GDT_Float32;

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outputDataset = driver->Create(outpath.c_str(), rasterXSize, rasterYSize, 1, tdt, 0);
	if (outputDataset == 0)
	{
		cout << "Error : failed create " << outpath << endl;
		return 10;
	}
	if( isgldas )
	{
		double adfGeoTrans[6] = { -180 , 1 , 0 , 90 , 0 , -1 };
		adfGeoTrans[0] += adfGeoTrans[1] * 0.5 ;
		adfGeoTrans[3] += adfGeoTrans[5] * 0.5 ;
		OGRSpatialReference osrs;
		char* pszSRS_WKT = 0;
		outputDataset->SetGeoTransform(adfGeoTrans);
		osrs.SetWellKnownGeogCS("EPSG:4326");
		osrs.exportToWkt(&pszSRS_WKT);
		outputDataset->SetProjection(pszSRS_WKT);
		CPLFree(pszSRS_WKT);
	}
	else
	{
		double adfGeoTrans[6];
		tds->GetGeoTransform(adfGeoTrans);
		outputDataset->SetGeoTransform(adfGeoTrans);
		outputDataset->SetProjection(tds->GetProjectionRef());
	}
	GDALClose(tds);
	double* buffer = new double[rasterXSize];
	double* outBuffer = new double[rasterXSize];
	short* cntBuffer = cntBuffer = new short[rasterXSize];
	for (int iy = 0; iy < rasterYSize; ++iy)
	{
		memset(cntBuffer, 0, sizeof(short) * rasterXSize);
		memset(outBuffer, 0, sizeof(double) * rasterXSize);
		for (int ifile = 0; ifile < selfiles.size(); ++ifile)
		{
			GDALDataset* ds = (GDALDataset*)GDALOpen(selfiles[ifile].c_str(), GA_ReadOnly);
			if (ds != 0)
			{
				ds->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, rasterXSize, 1,
					buffer, rasterXSize, 1, GDT_Float64, 0, 0, 0);
				for (int ix = 0; ix < rasterXSize; ++ix)
				{
					if (buffer[ix] >= minval && buffer[ix] <= maxval )
					{
						if (cntBuffer[ix] == 0)
						{
							outBuffer[ix] = buffer[ix];
							cntBuffer[ix] = 1;
						}
						else
						{
							outBuffer[ix] += buffer[ix];
							cntBuffer[ix] += 1;
						}
					}
				}
				GDALClose(ds);
			}
		}
		for (int ix = 0; ix < rasterXSize; ++ix)
		{
			if (cntBuffer[ix] > 0)
			{
				outBuffer[ix] = outBuffer[ix] / cntBuffer[ix];
			}
		}
		outputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, rasterXSize, 1,
			outBuffer, rasterXSize, 1, GDT_Float64, 0, 0, 0);
	}
	delete[] buffer;
	delete[] outBuffer;
	delete[] cntBuffer;
	GDALClose(outputDataset);

	makeCinfofile(outpath, (int)selfiles.size());
	if (wft_test_file_exists(outpath) == false)
	{
		cout << "error : failed to make " << outpath << endl;
		return 40;
	}
	return 100;
}


int processOneModisProduct(string filepath, string outfilepath , 
	string prefix , string tail )
{
	string dspath = prefix + filepath + tail ; //in window
	GDALDataset* ds = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);

	if (ds == 0  )
	{
		if (ds) GDALClose(ds);
		cout << "Error : can not open " << dspath  << endl;
		return 10;
	}

	int rasterXSize = ds->GetRasterXSize();
	int rasterYSize = ds->GetRasterYSize();
	GDALDataType theDataType = ds->GetRasterBand(1)->GetRasterDataType();

	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(outfilepath.c_str(), rasterXSize, rasterYSize, 1, theDataType, 0);

	{
		double adfGeoTrans[6] = { -180 , 0.05 , 0 , 90 , 0 , -0.05 };
		adfGeoTrans[0] += 0.075;
		adfGeoTrans[3] += -0.075;
		OGRSpatialReference osrs;
		char* pszSRS_WKT = 0;
		outds->SetGeoTransform(adfGeoTrans);
		osrs.SetWellKnownGeogCS("EPSG:4326");
		osrs.exportToWkt(&pszSRS_WKT);
		outds->SetProjection(pszSRS_WKT);
		CPLFree(pszSRS_WKT);
	}

	int* buffer = new int[rasterXSize];
	for (int iy = 0; iy < rasterYSize; ++iy)
	{
		ds->GetRasterBand(1)->RasterIO(GF_Read, 0, iy , rasterXSize, 1,
			buffer , rasterXSize, 1, GDT_Int32, 0, 0, 0);
		outds->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, rasterXSize, 1,
			buffer, rasterXSize, 1, GDT_Int32, 0, 0, 0);
	}
	if (buffer) delete[] buffer;
	GDALClose(ds);
	GDALClose(outds);


	return 100;
}


void processModisDaily(string hdfdir , string l2dir , string pngdir , vector<string>& pidvec)
{
	vector<string> allfiles;
	vector<int> ymdVec;

	wft_get_allSelectedFiles(hdfdir, "MOD11C1." , ".hdf" , -1, "", allfiles);
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
		string ymdstr = modisymd(filename, ymdi);
		ymdVec.push_back(ymdi);
	}
	int todayYmd = wft_current_dateymd_int();
	
	string dsprefix[2] = { "HDF4_EOS:EOS_GRID:\"", "HDF4_EOS:EOS_GRID:\"" };
	string dstail[2] = { "\":MODIS_CMG_3MIN_LST:LST_Day_CMG", "\":MODIS_CMG_3MIN_LST:LST_Night_CMG" };
	string pidarr[3];
	pidarr[0] = pidvec[0];
	pidarr[1] = pidvec[4];
	pidarr[2] = pidvec[8];

	for (int i = 0; i < numfiles; ++i)
	{
		string inpath = allfiles[i];
		int ymd0 = ymdVec[i];

		if (ymd0 >= g_from_ymd && ymd0 <= g_to_ymd)
		{
			string outname1 = "modis_lst_mod11c1_" + wft_int2str(ymd0) + ".dt.day.tif";
			string outname2 = "modis_lst_mod11c1_" + wft_int2str(ymd0) + ".nt.day.tif";

			string outpath1 = l2dir + outname1;
			string outpath2 = l2dir + outname2;

			if (wft_test_file_exists(outpath1) == false || wft_test_file_exists(outpath2) == false)
			{
				cout << "make modis lst day and night product " << outname1 <<","<< outname2 << endl;
				processOneModisProduct(inpath, outpath1, dsprefix[0], dstail[0]);
				processOneModisProduct(inpath, outpath2, dsprefix[1], dstail[1]);

				if (wft_test_file_exists(outpath1) && wft_test_file_exists(outpath2))
				{
					cout << "plot and insert ..."<< endl;
					doModisPlotDb(outpath1,outpath2, inpath, pngdir, ymd0, ymd0, g_modisRegionVec, pidarr);
				}
			}
		}
		cout << i << "/" << numfiles << endl;
	}
}

void processModisLevel3(string l2dir , string l3dir , string pngdir , vector<string>& pidvec)
{
	int ymdloc = 18 ;

	string tailarr[2] = { ".dt.day.tif" , ".nt.day.tif" };
	string gcode[2] = { "dt","nt" };
	for(int ig = 0 ; ig < 2 ; ++ ig )
	{
		vector<string> allfiles;
		vector<int> ymdVec;
		string inprefix = "modis_lst_";
		string intail = tailarr[ig];
		wft_get_allSelectedFiles(l2dir, inprefix, intail, -1, "", allfiles);
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
			if (ymdi < g_from_ymd) continue;
			if (ymdi > g_to_ymd) continue;

			string dttail[] = { ".fiv.tif",".ten.tif",".mon.tif" };

			for(int idt = 0 ; idt < 3 ; ++ idt )
			{
				int ymd0, ymd1;
				if (idt == 0)
				{
					getfivdaysStartEnd(ymdi, ymd0, ymd1);
				}
				else if (idt == 1)
				{
					gettendaysStartEnd(ymdi, ymd0, ymd1);
				}
				else
				{
					getmonStartEnd(ymdi, ymd0, ymd1);
				}
				if (todayYmd <= ymd1) continue;
				vector<string> selectedfiles;
				int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
				if (combcnt == 0) continue;
				{
					string outname = "modis_lst_mod11c1_" + wft_int2str(ymd0) + "." + gcode[ig] + dttail[idt];
					string outpath = l3dir + outname;
					if (wft_test_file_exists(outpath) == false)
					{
						processCombineAver(selectedfiles, outpath, 7500 , 65535 , false  );
					}
				}
			}
			cout << i << "/" << numfiles << endl;
		}
	}

	string l3tailarr[] = { ".dt.fiv.tif" , ".dt.ten.tif" , ".dt.mon.tif" };
	for (int idt = 0; idt < 3; ++idt)
	{
		vector<string> l3files;
		wft_get_allSelectedFiles(l3dir, "modis_lst_", l3tailarr[idt], -1, "", l3files);
		if (l3files.size() == 0) continue;
		for (int ifile = 0; ifile < l3files.size(); ++ifile)
		{
			string filepath1 = l3files[ifile];
			string filename1 = wft_base_name(filepath1);
			string filename2 = wft_replaceString(filename1, ".dt.", ".nt.");
			string filepath2 = l3dir + filename2;
			string ymdstr = filename1.substr(ymdloc, 8);
			int ymdi = (int)atof(ymdstr.c_str());
			if (wft_test_file_exists(filepath1) && wft_test_file_exists(filepath2))
			{
				int ymd0, ymd1;
				string pidarr[3];
				if (idt == 0)
				{
					getfivdaysStartEnd(ymdi, ymd0, ymd1);
					pidarr[0] = pidvec[1];
					pidarr[1] = pidvec[5];
					pidarr[2] = pidvec[9];
				}
				else if (idt == 1)
				{
					gettendaysStartEnd(ymdi, ymd0, ymd1);
					pidarr[0] = pidvec[2];
					pidarr[1] = pidvec[6];
					pidarr[2] = pidvec[10];
				}
				else
				{
					getmonStartEnd(ymdi, ymd0, ymd1);
					pidarr[0] = pidvec[3];
					pidarr[1] = pidvec[7];
					pidarr[2] = pidvec[11];
				}
				doModisPlotDb(filepath1, filepath2, filepath1, pngdir, ymd0, ymd1, g_modisRegionVec , pidarr);
			}
			cout << ifile << "/" << l3files.size() << endl;
		}
	}


}


RegionData makeRegionDataByJsonNode(JsonObject& obj)
{
	RegionData rd;
	rd.code = obj["code"].asString() ;
	rd.displayName = obj["display-name"].asString() ;
	rd.displayName2 = obj["display-name2"].asString();
	rd.plottem = obj["plot-tem"].asString();
	rd.x0 = obj["x0"].asString();
	rd.x1 = obj["x1"].asString();
	rd.y0 = obj["y0"].asString();
	rd.y1 = obj["y1"].asString();
	rd.xspace = obj["xspace"].asString();
	rd.yspace = obj["yspace"].asString();
	rd.valid0 = obj["valid0"].asString();
	rd.valid1 = obj["valid1"].asString();
	rd.scale = obj["scale"].asString();
	rd.offset = obj["offset"].asString();
	return rd;
}




void processGldasDaily(string ncdir, string l2dir, string pngdir, vector<string>& pidvec)
{
	int ymdloc = 20;
	vector<string> allfiles;
	vector<int> ymdVec;
	wft_get_allSelectedFiles(ncdir, "GLDAS_CLM10SUBP_3H.", ".pss.nc", -1, "", allfiles);
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
		string ymdstr = modisymd(filename, ymdi);
		ymdVec.push_back(ymdi);
	}
	int todayYmd = wft_current_dateymd_int();

	string pidarr[3];
	pidarr[0] = pidvec[0];
	pidarr[1] = pidvec[4];
	pidarr[2] = pidvec[8];

	for (int i = 0; i < numfiles; ++i)
	{
		string inpath = allfiles[i];
		int ymd0 = ymdVec[i];

		if (ymd0 >= g_from_ymd && ymd0 <= g_to_ymd)
		{
			string outname1 = "gldas_lst_" + wft_int2str(ymd0) + ".day.tif";
			string outpath1 = l2dir + outname1;
			if (wft_test_file_exists(outpath1) == false )
			{
				vector<string> selfiles;
				selectedFilesByYmdRange(allfiles, ymdVec, selfiles, ymd0, ymd0);
				if (selfiles.size() == 0) continue;
				cout << "make gldas lst day product " << outname1  << endl;
				processCombineAver(selfiles, outpath1, 200, 500, true);

			}
			string pngpath = pngdir + outname1 + ".png";
			if (wft_test_file_exists(outpath1) && wft_test_file_exists(pngpath) == false)
			{
				cout << "plot and insert ..."<<pngpath << endl;
				dowarpPlotDb(outpath1, pngpath, ymd0, ymd0, g_gldasRegionVec, pidarr);
			}
		}
		cout << i << "/" << numfiles << endl;
	}
}


void processGldasLevel3(string l2dir, string l3dir, string pngdir, vector<string>& pidvec)
{
	int ymdloc = 10;

	vector<string> allfiles;
	vector<int> ymdVec;
	string inprefix = "gldas_lst_";
	string intail = ".day.tif";
	wft_get_allSelectedFiles(l2dir, inprefix, intail, -1, "", allfiles);
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
		if (ymdi < g_from_ymd) continue;
		if (ymdi > g_to_ymd) continue;

		string dttail[] = { ".fiv.tif",".ten.tif",".mon.tif" };

		for (int idt = 0; idt < 3; ++idt)
		{
			int ymd0, ymd1;
			if (idt == 0)
			{
				getfivdaysStartEnd(ymdi, ymd0, ymd1);
			}
			else if (idt == 1)
			{
				gettendaysStartEnd(ymdi, ymd0, ymd1);
			}
			else
			{
				getmonStartEnd(ymdi, ymd0, ymd1);
			}
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			{
				string outname = "gldas_lst_" + wft_int2str(ymd0) + dttail[idt];
				string outpath = l3dir + outname;
				if (wft_test_file_exists(outpath) == false)
				{
					processCombineAver(selectedfiles, outpath, 200, 500, true );
				}
				string pngpath = pngdir + outname + ".png";

				if (wft_test_file_exists(outpath) && wft_test_file_exists(pngpath) == false)
				{
					string pidarr[3];
					if (idt == 0)
					{
						getfivdaysStartEnd(ymdi, ymd0, ymd1);
						pidarr[0] = pidvec[1];
						pidarr[1] = pidvec[5];
						pidarr[2] = pidvec[9];
					}
					else if (idt == 1)
					{
						gettendaysStartEnd(ymdi, ymd0, ymd1);
						pidarr[0] = pidvec[2];
						pidarr[1] = pidvec[6];
						pidarr[2] = pidvec[10];
					}
					else
					{
						getmonStartEnd(ymdi, ymd0, ymd1);
						pidarr[0] = pidvec[3];
						pidarr[1] = pidvec[7];
						pidarr[2] = pidvec[11];
					}
					cout << "plot and insert ..." << pngpath << endl;
					dowarpPlotDb(outpath, pngpath, ymd0, ymd1, g_gldasRegionVec, pidarr);
				}
			}
		}
		cout << i << "/" << numfiles << endl;
	}


}




int main(int argc, char** argv)
{
	printCurrentTime();
	std::cout << "Description: do modis gldas lst all in one. " << std::endl;
	std::cout << "Version 1a . by wangfengdev@163.com 2017-12-19." << std::endl;
	std::cout << "Version 1.1a . add linux db staff." << std::endl;
	if (argc == 1)
	{
		std::cout << "*** sample call: ***" << std::endl;
		std::cout << "modis_gldas_lst_aio_monitor startup.json "<< endl;
		std::cout << "*** startup.json example ***" << std::endl;
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

	g_txt = jsonRoot["totxt"].asString();
	g_gnuplot = jsonRoot["gnuplot"].asString();


	

	//
	g_xspace = jsonRoot["xspace"].asString();
	g_yspace = jsonRoot["yspace"].asString();

	g_from_ymd = jsonRoot["from_ymd"].as<int>();
	g_to_ymd = jsonRoot["to_ymd"].as<int>();



	string host = jsonRoot["host"].asString() ;
 	string user = jsonRoot["user"].asString() ;
	string pwd = jsonRoot["pwd"].asString() ;
	string db = jsonRoot["db"].asString() ;
	string tbimg = jsonRoot["tbimg"].asString() ;
	string tbxl = jsonRoot["tbxl"].asString() ;
	g_db.connect(host, user, pwd, db);

	int domodis = jsonRoot["domodis"].as<int>() ;
	int dogldas = jsonRoot["dogldas"].as<int>() ;


	///////////////////////////////////////////////////////////////////////
	//
	//
	//  modis
	//
	///////////////////////////////////////////////////////////////////////
	string modishdfdir = jsonRoot["modishdfdir"].asString();
	string modisl2dir = jsonRoot["modisl2dir"].asString();
	string modisl3dir = jsonRoot["modisl3dir"].asString();
	string modispngdir = jsonRoot["modispngdir"].asString();

	JsonArray& mpidarr = jsonRoot["modis-pids"].asArray();
	vector<string> modispidvec;
	for (int it = 0; it < mpidarr.size(); ++it)
	{
		modispidvec.push_back(mpidarr[it].asString());
	}


	JsonArray& jregions = jsonRoot["modis-regions"].asArray() ;
	for (int ir = 0; ir < jregions.size(); ++ir)
	{
		JsonObject& obj = jregions[ir].asObject();
		g_modisRegionVec.push_back(makeRegionDataByJsonNode(obj));
	}
	cout << "processing modis level2..." << endl;
	if( domodis == 1 ) processModisDaily( modishdfdir , modisl2dir , modispngdir , modispidvec );
	cout << "processing modis level3..." << endl;
	if( domodis == 1 ) processModisLevel3(modisl2dir, modisl3dir , modispngdir, modispidvec);



	///////////////////////////////////////////////////////////////////////
	//
	//
	//  GLDAS
	//
	///////////////////////////////////////////////////////////////////////
	string gncdir = jsonRoot["gldasncdir"].asString();
	string gl2dir = jsonRoot["gldasl2dir"].asString();
	string gl3dir = jsonRoot["gldasl3dir"].asString();
	string gpngdir = jsonRoot["gldaspngdir"].asString();
	
	JsonArray& gregions = jsonRoot["gldas-regions"].asArray();
	for (int ir = 0; ir < gregions.size(); ++ir)
	{
		JsonObject& obj = gregions[ir].asObject();
		g_gldasRegionVec.push_back(makeRegionDataByJsonNode(obj));
	}

	vector<string> gpidvec;
	JsonArray& gpidarr = jsonRoot["gldas-pids"].asArray();
	for (int i = 0; i < gpidarr.size(); ++i)
	{
		gpidvec.push_back(gpidarr[i]);
	}
	cout << "processing gldas daily ..." << endl;
	if( dogldas == 1 ) processGldasDaily(gncdir, gl2dir, gpngdir, gpidvec);
	cout << "processing gldas level3 ..." << endl;
	if( dogldas == 1 ) processGldasLevel3(gl2dir, gl3dir, gpngdir, gpidvec);





	std::cout << "All done." << std::endl;
	printCurrentTime();
	return 0;
}



