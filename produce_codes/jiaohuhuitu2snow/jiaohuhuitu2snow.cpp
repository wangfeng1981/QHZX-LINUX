// jiaohuhuitu2snow.cpp : 定义控制台应用程序的入口点。
//积雪交互绘图输入时间和经纬度范围，和输出文件路径


#include <iostream>
#include <ctime>
#include <fstream>
#include "../sharedcodes/wftools.h"
#include "../sharedcodes/ajson5.h"
#include <vector>
#include <string>
#include "gdal_priv.h"
#include "ogr_spatialref.h"
using namespace std;
using std::cout;

string g_logfile = "temp.log";
string g_shengmaskfile ;
string g_spacetem  ;
string g_gnuplot;
double g_pixelarea = 16;

//string g_maskidarr[] = {"30","41","42","57","48","49","53","50","32","45","37","46","47","36","39","43","35","34","59","58","44","33","56","38","52","61","31","55","60","54","40","51"};
//string g_leftarr[] ={ "109.92 ","109.38 ","110.35 ","87.26 ","104.16 ","98.98 ","98.10 ","103.12 ","107.96 ","104.85 ","115.68 ","102.87 ","103.28 ","116.13 ","110.86 ","108.07 ","113.33 ","91.67 ","98.79 ","83.90 ","109.30 ","104.75 ","99.99 ","115.36 ","91.86 ","113.82 ","111.20 ","72.90 ","67.95 ","92.03 ","112.53 ","99.79"};
//string g_rightarr[] = {"123.00 ","125.14 ","126.22 ","114.21 ","122.80 ","117.56 ","115.09 ","123.31 ","125.35 ","122.14 ","140.59 ","121.63 ","119.75 ","136.81 ","127.40 ","123.98 ","131.29 ","131.58 ","113.15 ","108.56 ","128.20 ","120.05 ","116.75 ","127.40 ","114.03 ","130.08 ","123.56 ","104.61 ","101.88 ","111.69 ","128.45 ","115.69"};
//string g_toparr[] = {"46.56 ","40.15 ","33.81 ","48.29 ","31.02 ","31.89 ","34.72 ","25.67 ","48.12 ","41.87 ","59.06 ","38.78 ","35.63 ","51.80 ","40.63 ","35.58 ","48.99 ","58.83 ","44.88 ","44.84 ","43.91 ","46.24 ","45.08 ","37.35 ","39.81 ","31.45 ","45.74 ","41.99 ","54.68 ","34.75 ","36.68 ","37.71"};
//string g_bottomarr[] = {"33.94 ","23.90 ","18.07 ","27.09 ","14.72 ","15.40 ","19.13 ","0.82 ","30.55 ","25.88 ","37.92 ","23.54 ","19.13 ","35.36 ","25.26 ","18.99 ","33.22 ","31.91 ","29.74 ","26.04 ","28.89 ","29.09 ","26.21 ","25.19 ","20.55 ","16.25 ","33.07 ","21.35 ","28.83 ","15.64 ","21.64 ","22.66"};
struct DataType
{
	string pdtname;
	string pdtDisplayName;
	string dir;
	string inprefix;
	string intail;
	int ymdloc;
};

vector<string> g_maskidarr ;
vector<string> g_leftarr;
vector<string> g_rightarr;
vector<string> g_toparr ;
vector<string> g_bottomarr;
vector<DataType> g_dataTypeArray;






extern void getPixelRangeByLonLat(string filepath, double left, double right, double top, double bottom,
	int& x0, int& x1, int& y0, int& y1,
	int& xsize, int& ysize, GDALDataType& dt,
	double* newtrans);
extern 	void writelog(string str, int value);
extern 	void writelog(string str, double value);
extern 	void writelog(string line);
extern 	string makeYmdStr(string year, string mon, string day);
extern 	void writeresultinfo(string& filepath, int code, string description);
extern 	bool isGDALAccessable(string filepath);
extern 	void convertLonLatToOyAlbers(double lon, double lat, double& refx, double& refy);
extern 	bool findShengLonLatRect(string shengid, double* lrtbarr, string* lrtbstrarr);








void spacejhht(DataType datatype ,int ymd0,int ymd1,
	double ltx , double lty , double rbx , double rby , int maskid , string pngfile ,
	string* lrtb  )
{
	vector<string> allfiles;
	wft_get_allSelectedFiles(datatype.dir,
		datatype.inprefix,
		datatype.intail,
		-1,
		"",
		allfiles);

	vector<string> selectedFiles;
	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymdstr = filename.substr(datatype.ymdloc, 8);
		int ymdi = wft_str2int(ymdstr);
		if (ymdi >= ymd0 && ymdi <= ymd1)
		{
			selectedFiles.push_back(filepath);
		}
	}

	writelog("selected file count:", (int)selectedFiles.size());
	if (selectedFiles.size() == 0) return;

	//获取像素四角范围
	bool rectok = false;
	int x0, y0, x1, y1;
	int rxsize, rysize;
	double newtrans[6];
	{//获取像素四角范围
		for (int i = 0; i < selectedFiles.size(); ++i)
		{
			string filepath = selectedFiles[i];
			if (isGDALAccessable(filepath))
			{
				GDALDataset* ds = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
				rxsize = ds->GetRasterXSize();
				rysize = ds->GetRasterYSize();
				double trans[6];
				ds->GetGeoTransform(trans);
				x0 = (ltx - trans[0]) / trans[1];
				x1 = (rbx - trans[0]) / trans[1];
				y0 = (lty - trans[3]) / trans[5];
				y1 = (rby - trans[3]) / trans[5];
				x0 = max(0, x0);
				x1 = min(rxsize - 1, x1);
				y0 = max(0, y0);
				y1 = min(rysize - 1, y1);

				newtrans[0] = trans[0] + trans[1] * x0;
				newtrans[1] = trans[1];
				newtrans[2] = trans[2];
				newtrans[3] = trans[3] + trans[5] * y0;
				newtrans[4] = trans[4];
				newtrans[5] = trans[5];

				GDALClose(ds);
				rectok = true;
				break;
			}
		}
	}

	int xspace = 1;
	int yspace = 1;
	int subxsize = x1 - x0 + 1;
	int subysize = y1 - y0 + 1;
	if (subxsize > 2000) xspace = subxsize / 2000;
	if (subysize > 2000) yspace = subysize / 2000;

	if (rectok)
	{
		string txtfile = pngfile + ".txt";
		FILE* ofptr = fopen(txtfile.c_str(), "w");
		int allsize = subxsize * subysize;
		int allsizemb = allsize / 1000 / 1000;//estimate MB
		int valmin(1), valmax(-19999);
		int ngood = 0;
		if (allsizemb < 200)
		{
			short* sumbuff = new short[allsize];
			memset(sumbuff, 0, allsize * sizeof(short));
			char* buff = new char[allsize];
			char* maskbuff = new char[allsize];
			
			if (maskid > 0)
			{
				GDALDataset* maskds = (GDALDataset*)GDALOpen(g_shengmaskfile.c_str(), GA_ReadOnly);
				maskds->GetRasterBand(1)->RasterIO(GF_Read, x0, y0, subxsize, subysize, maskbuff,
					subxsize, subysize, GDT_Byte, 0, 0, 0);
				GDALClose(maskds);
			}

			for (int ifile = 0; ifile < selectedFiles.size(); ++ifile)
			{
				writelog("processing ", ifile);
				GDALDataset* ds = (GDALDataset*)GDALOpen(selectedFiles[ifile].c_str(), GA_ReadOnly);
				if (ds != 0)
				{
					++ngood;
					ds->GetRasterBand(1)->RasterIO(GF_Read, x0, y0, subxsize, subysize, buff,
						subxsize, subysize, GDT_Byte, 0, 0, 0);
					for (int iy = 0; iy < subysize; iy += yspace)
					{
						for (int ix = 0; ix < subxsize; ix += xspace)
						{
							int it = iy * subxsize + ix;
							if (buff[it] == 10 )
							{
								++sumbuff[it];
							}
						}
					}
					GDALClose(ds);
				}
			}

			for (int iy = 0; iy < subysize; iy += yspace)
			{
				double coory = newtrans[3] + newtrans[5] * (iy + 1.5);
				for (int ix = 0; ix < subxsize; ix += xspace)
				{
					int it = iy * subxsize + ix;
					double coorx = newtrans[0] + newtrans[1] * (ix + 1.5);

					if (sumbuff[it] > 0  ) {
						if ( maskid <= 0 || (maskid > 0 && maskbuff[it] == maskid) )
						{
							if (sumbuff[it] > valmax) valmax = sumbuff[it];
							if (sumbuff[it] < valmin) valmin = sumbuff[it];
							fprintf(ofptr, "%lf %lf %d\n", coorx, coory, sumbuff[it]);
						}
						else
						{
							fprintf(ofptr, "%lf %lf NaN\n", coorx, coory);
						}
					}
					else {
						fprintf(ofptr, "%lf %lf NaN\n", coorx, coory);
					}
				}
				fprintf(ofptr, "\n"); //ofs << endl;
			}
			
			delete[] buff;
			delete[] sumbuff;
			delete[] maskbuff;
		}
		else { //line by line
			char* buff = new char[subxsize];
			short* sumbuff = new short[subxsize];
			char* maskbuff = new char[subxsize];
			for (int iy = 0; iy < subysize; iy += yspace)
			{
				memset(sumbuff, 0, subxsize * sizeof(short));
				if (maskid > 0)
				{
					GDALDataset* maskds = (GDALDataset*)GDALOpen(g_shengmaskfile.c_str(), GA_ReadOnly);
					maskds->GetRasterBand(1)->RasterIO(GF_Read, 0, y0 + iy, subxsize, 1, maskbuff,
						subxsize, 1, GDT_Byte, 0, 0, 0);
					GDALClose(maskds);
				}

				for (int ifile = 0; ifile < selectedFiles.size() ; ++ifile)
				{
					GDALDataset* ds = (GDALDataset*)GDALOpen(selectedFiles[ifile].c_str(), GA_ReadOnly);
					if (ds != 0)
					{
						if (iy == 0) ++ngood;
						ds->GetRasterBand(1)->RasterIO(GF_Read, 0, y0 + iy, subxsize, 1, buff,
							subxsize, 1, GDT_Byte, 0, 0, 0);
						for (int ix = 0; ix < subxsize; ix += xspace)
						{
							if (buff[ix] == 10)
							{
								++sumbuff[ix];
							}
						}
						GDALClose(ds);
					}
				}
				double coory = newtrans[3] + newtrans[5] * (iy + 1.5);
				for (int ix = 0; ix < subxsize; ix += xspace)
				{
					double coorx = newtrans[0] + newtrans[1] * (ix + 1.5);
					if (sumbuff[ix] > 0) {
						if (maskid <= 0 || (maskid > 0 && maskbuff[ix] == maskid))
						{
							if (sumbuff[ix] > valmax) valmax = sumbuff[ix];
							if (sumbuff[ix] < valmin) valmin = sumbuff[ix];
							fprintf(ofptr, "%lf %lf %d\n", coorx, coory, sumbuff[ix]);
						}
						else
						{
							fprintf(ofptr, "%lf %lf NaN\n", coorx, coory);
						}
					}
					else
					{
						fprintf(ofptr, "%lf %lf NaN\n", coorx, coory);
					}
				}
				fprintf(ofptr, "\n");  
				wft_term_progress(iy, subysize);
			}
			delete[] buff;
			delete[] sumbuff;
			delete[] maskbuff;
		}
		fclose(ofptr); ofptr = 0;


		/////////////////////////////////////////////////////
		//
		//  PLOT
		//
		////////////////////////////////////////////////////
		vector<string> varVec;
		vector<string> repVec;
		string datetimeNow = wft_current_datetimestr();

		int outwid = 0;
		int outhei = 0;

		if (subxsize >= subysize)
		{
			outwid = 1000;
			outhei = subysize * 1.0 / subxsize * outwid;
		}
		else {
			outhei = 1000;
			outwid = subxsize * 1.0 / subysize * outhei;
			if (outwid < 500)
			{
				outwid = 500;
				outhei = subysize * 1.0 / subxsize * outwid;
			}
		}

		string dt0 = wft_convert_ymdi2y_m_d(ymd0);
		string dt1 = wft_convert_ymdi2y_m_d(ymd1);

		varVec.push_back("{{{INFILE}}}");
		varVec.push_back("{{{OUTFILE}}}");

		varVec.push_back("{{{DISPLAYNAME}}}");

		varVec.push_back("{{{DATE0}}}");
		varVec.push_back("{{{DATE1}}}");

		varVec.push_back("{{{LEFT}}}");
		varVec.push_back("{{{RIGHT}}}");
		varVec.push_back("{{{TOP}}}");
		varVec.push_back("{{{BOTTOM}}}");

		varVec.push_back("{{{OUTWID}}}");//png width
		varVec.push_back("{{{OUTHEI}}}");//png height

		varVec.push_back("{{{X0}}}");//xrange min
		varVec.push_back("{{{X1}}}");//xrange max
		varVec.push_back("{{{Y0}}}");//yrange min
		varVec.push_back("{{{Y1}}}");//yrange max

		varVec.push_back("{{{VMIN}}}");//cbmin
		varVec.push_back("{{{VMAX}}}");//cbmax

		varVec.push_back("{{{PTIME}}}");//produce time
		varVec.push_back("{{{CFC}}}");//combined files count

									  ///////////////////////////////
		repVec.push_back(txtfile);
		repVec.push_back(pngfile);

		repVec.push_back(datatype.pdtDisplayName);

		repVec.push_back(dt0);
		repVec.push_back(dt1);

		repVec.push_back(lrtb[0]);
		repVec.push_back(lrtb[1]);
		repVec.push_back(lrtb[2]);
		repVec.push_back(lrtb[3]);

		repVec.push_back(wft_int2str(outwid));
		repVec.push_back(wft_int2str(outhei));


		repVec.push_back(wft_double2str(ltx));
		repVec.push_back(wft_double2str(rbx));
		repVec.push_back(wft_double2str(rby));
		repVec.push_back(wft_double2str(lty));

		repVec.push_back(wft_double2str(valmin-0.5));
		repVec.push_back(wft_double2str(valmax+0.5));

		repVec.push_back(datetimeNow);
		repVec.push_back(wft_int2str(ngood));


		string plotfile = pngfile + ".plot";
		wft_create_file_by_template_with_replacement(plotfile, g_spacetem, varVec, repVec);

		string cmd2 = g_gnuplot + " " + plotfile;
		int res2 = system(cmd2.c_str());
		cout << "gnuplot return code " << res2 << endl;

		wft_remove_file(txtfile);
	}
}


//计算区域面积指数
double computeMjzs(vector<string>& selectedFiles, int x0, int y0, int subxsize, int subysize, int maskid)
{
	long allSnowDayCount = 0;
	int allsize = subxsize * subysize;
	int allsizemb = allsize / 1000 / 1000;//estimate MB
	int ngood = 0;
	int nf = selectedFiles.size();
	if (nf == 0) return -1;
	if (allsizemb < 200)
	{
		char* buff = new char[allsize];
		char* maskbuff = new char[allsize];
		if (maskid > 0)
		{
			GDALDataset* maskds = (GDALDataset*)GDALOpen(g_shengmaskfile.c_str(), GA_ReadOnly);
			maskds->GetRasterBand(1)->RasterIO(GF_Read, x0, y0, subxsize, subysize, maskbuff,
				subxsize, subysize, GDT_Byte, 0, 0, 0);
			GDALClose(maskds);
		}

		for (int ifile = 0; ifile < selectedFiles.size(); ++ifile)
		{
			writelog("processing "+ selectedFiles[ifile]);
			GDALDataset* ds = (GDALDataset*)GDALOpen(selectedFiles[ifile].c_str(), GA_ReadOnly);
			if (ds != 0)
			{
				++ngood;
				ds->GetRasterBand(1)->RasterIO(GF_Read, x0, y0, subxsize, subysize, buff,
					subxsize, subysize, GDT_Byte, 0, 0, 0);
				for (int it = 0; it < allsize; ++ it )
				{
					if (buff[it] == 10)
					{
						if (maskid <= 0 || (maskid > 0 && maskbuff[it] == maskid))
						{
							++allSnowDayCount ;
						}
					}
				}
				GDALClose(ds);
			}
		}
		delete[] buff;
		delete[] maskbuff;
	}
	else { //line by line
		char* buff = new char[subxsize];
		char* maskbuff = new char[subxsize];
		for (int iy = 0; iy < subysize; ++ iy)
		{
			if (maskid > 0)
			{
				GDALDataset* maskds = (GDALDataset*)GDALOpen(g_shengmaskfile.c_str(), GA_ReadOnly);
				maskds->GetRasterBand(1)->RasterIO(GF_Read, 0, y0 + iy, subxsize, 1, maskbuff,
					subxsize, 1, GDT_Byte, 0, 0, 0);
				GDALClose(maskds);
			}
			for (int ifile = 0; ifile < selectedFiles.size(); ++ifile)
			{
				GDALDataset* ds = (GDALDataset*)GDALOpen(selectedFiles[ifile].c_str(), GA_ReadOnly);
				if (ds != 0)
				{
					if (iy == 0) ++ngood;
					ds->GetRasterBand(1)->RasterIO(GF_Read, 0, y0 + iy, subxsize, 1, buff,
						subxsize, 1, GDT_Byte, 0, 0, 0);
					for (int ix = 0; ix < subxsize; ++ ix )
					{
						if (buff[ix] == 10)
						{
							if (maskid <= 0 || (maskid > 0 && maskbuff[ix] == maskid))
							{
								++allSnowDayCount;
							}
						}
					}
					GDALClose(ds);
				}
			}
			wft_term_progress(iy, subysize);
		}
		delete[] buff;
		delete[] maskbuff;
	}
	double sumMjzs = allSnowDayCount * g_pixelarea / nf;
	return sumMjzs;
}


void nianneiJhht(DataType datatype, int ymd0, int ymd1,
	double ltx, double lty, double rbx, double rby, int maskid, string jsonfile  )
{
	vector<string> allfiles;
	wft_get_allSelectedFiles(datatype.dir,
		datatype.inprefix,
		datatype.intail,
		-1,
		"",
		allfiles);

	vector<string> selectedFiles;
	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymdstr = filename.substr(datatype.ymdloc, 8);
		int ymdi = wft_str2int(ymdstr);
		if (ymdi >= ymd0 && ymdi <= ymd1)
		{
			selectedFiles.push_back(filepath);
		}
	}

	writelog("selected file count:", (int)selectedFiles.size());
	if (selectedFiles.size() == 0) return;

	//获取像素四角范围
	bool rectok = false;
	int x0, y0, x1, y1;
	{//获取像素四角范围
		for (int i = 0; i < selectedFiles.size(); ++i)
		{
			string filepath = selectedFiles[i];
			if (isGDALAccessable(filepath))
			{
				int rxsize, rysize;
				GDALDataset* ds = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
				rxsize = ds->GetRasterXSize();
				rysize = ds->GetRasterYSize();
				double trans[6];
				ds->GetGeoTransform(trans);
				x0 = (ltx - trans[0]) / trans[1];
				x1 = (rbx - trans[0]) / trans[1];
				y0 = (lty - trans[3]) / trans[5];
				y1 = (rby - trans[3]) / trans[5];
				x0 = max(0, x0);
				x1 = min(rxsize - 1, x1);
				y0 = max(0, y0);
				y1 = min(rysize - 1, y1);
				GDALClose(ds);
				rectok = true;
				break;
			}
		}
	}
	if (rectok == false)
	{
		writelog("Failed to compute albers coordinate from longitude and latitude.");
		return;
	}
	int subxsize = x1 - x0 + 1;
	int subysize = y1 - y0 + 1;
	vector<int> ymdvec;
	vector<double> mjzsvec;
	for (int ifile = 0; ifile < selectedFiles.size(); ++ifile)
	{
		vector<string> vec;
		vec.push_back(selectedFiles[ifile]);
		double mjzs = computeMjzs(vec, x0, y0, subxsize, subysize, maskid);
		string filepath = selectedFiles[ifile];
		string filename = wft_base_name(filepath);
		string ymdstr = filename.substr(datatype.ymdloc, 8);
		ymdvec.push_back(wft_str2int(ymdstr));
		mjzsvec.push_back(mjzs);
	}

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	root["product"] = datatype.pdtname ;
	root["product_display_name"] = datatype.pdtDisplayName;
	JsonArray& datearr = root.createNestedArray("date_arr");
	JsonArray& valarr = root.createNestedArray("val_arr");
	JsonArray& objarr = root.createNestedArray("obj_arr");
	for (int i = 0; i < mjzsvec.size(); ++i)
	{
		string ymdstr1 = wft_convert_ymdi2y_m_d(ymdvec[i]);
		datearr.add(ymdstr1);
		valarr.add(mjzsvec[i]);
		JsonObject& obj1 = objarr.createNestedObject();
		obj1["date"] = ymdstr1;
		obj1["value"] = mjzsvec[i];
	}
	size_t jsize = root.measurePrettyLength();
	char* jbuff = new char[jsize+1];
	if (jbuff)
	{
		root.prettyPrintTo(jbuff , jsize+1);
		ofstream ofs(jsonfile.c_str());//json here
		ofs << jbuff << endl;
		ofs.close();
		delete[] jbuff;
	}
	else
	{
		writelog("Failed to make json, bytes:", (int)jsize);
	}
}

void nianjianJhht(DataType datatype, int ymd0, int ymd1,
	double ltx, double lty, double rbx, double rby, int maskid, string jsonfile)
{
	vector<string> allfiles;
	wft_get_allSelectedFiles(datatype.dir,
		datatype.inprefix,
		datatype.intail,
		-1,
		"",
		allfiles);

	int yearmin = 0;
	int yearmax = 0;
	vector<string> selectedFiles;
	vector<int> selectedYears ;

	int yyyy0 = ymd0 / 10000;
	int yyyy1 = ymd1 / 10000;

	int mmdd0 = ymd0 % 10000;
	int mmdd1 = ymd1 % 10000;
	if (yyyy1 > yyyy0) mmdd1 + 10000;

	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymdstr = filename.substr(datatype.ymdloc, 8);
		int ymdi = wft_str2int(ymdstr);
		int mmddi = ymdi % 10000;
		if (mmddi >= mmdd0 && mmddi <= mmdd1 )
		{
			selectedFiles.push_back(filepath);
			int yeari = ymdi / 10000;
			if (yyyy1 > yyyy0)
			{
				if ( mmdd0 <= mmddi && mmddi <= 1231 )
				{
					selectedYears.push_back(yeari);
				}
				else if( 0 < mmddi && mmddi < mmdd1  ) {
					selectedYears.push_back(yeari - 1);
				}
			}
			else
			{
				selectedYears.push_back(yeari);
			}
			if (yearmin == 0)
			{
				yearmin = yeari;
				yearmax = yeari;
			}
			else
			{
				if (yearmin > yeari)
				{
					yearmin = yeari;
				}
				if (yearmax < yeari)
				{
					yearmax = yeari;
				}
			}
		}
	}
	writelog("from year:", yearmin );
	writelog("to year:", yearmax );
	writelog("selected file count:", (int)selectedFiles.size());
	if (selectedFiles.size() == 0) return;

	//获取像素四角范围
	bool rectok = false;
	int x0, y0, x1, y1;
	{//获取像素四角范围
		for (int i = 0; i < selectedFiles.size(); ++i)
		{
			string filepath = selectedFiles[i];
			if (isGDALAccessable(filepath))
			{
				int rxsize, rysize;
				GDALDataset* ds = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
				rxsize = ds->GetRasterXSize();
				rysize = ds->GetRasterYSize();
				double trans[6];
				ds->GetGeoTransform(trans);
				x0 = (ltx - trans[0]) / trans[1];
				x1 = (rbx - trans[0]) / trans[1];
				y0 = (lty - trans[3]) / trans[5];
				y1 = (rby - trans[3]) / trans[5];
				x0 = max(0, x0);
				x1 = min(rxsize - 1, x1);
				y0 = max(0, y0);
				y1 = min(rysize - 1, y1);
				GDALClose(ds);
				rectok = true;
				break;
			}
		}
	}
	if (rectok == false)
	{
		writelog("Failed to compute albers coordinate from longitude and latitude.");
		return;
	}
	int subxsize = x1 - x0 + 1;
	int subysize = y1 - y0 + 1;
	vector<int> yearvec ;
	vector<double> mjzsvec;
	for (int iyear = yearmin ; iyear <= yearmax ; ++iyear)
	{
		vector<string> vec;
		writelog("prepare for year ", iyear);
		for (int isel = 0; isel < selectedYears.size(); ++isel)
		{
			if (selectedYears[isel] == iyear)
			{
				vec.push_back(selectedFiles[isel]);
				writelog(selectedFiles[isel]);
			}
		}
		writelog("end prepare for year ", iyear);
		if (vec.size() > 0)
		{
			double mjzs = computeMjzs(vec, x0, y0, subxsize, subysize, maskid);
			mjzsvec.push_back(mjzs);
			yearvec.push_back(iyear);
		}

	}

	DynamicJsonBuffer jsonBuffer;
	JsonObject& root = jsonBuffer.createObject();
	root["product"] = datatype.pdtname;
	root["product_display_name"] = datatype.pdtDisplayName;
	JsonArray& datearr = root.createNestedArray("date_arr");
	JsonArray& valarr = root.createNestedArray("val_arr");
	JsonArray& objarr = root.createNestedArray("obj_arr");
	for (int i = 0; i < mjzsvec.size(); ++i)
	{
		datearr.add(yearvec[i]);
		valarr.add(mjzsvec[i]);
		JsonObject& obj1 = objarr.createNestedObject();
		obj1["date"] = yearvec[i];
		obj1["value"] = mjzsvec[i];
	}
	size_t jsize = root.measurePrettyLength();
	char* jbuff = new char[jsize + 1];
	if (jbuff)
	{
		root.prettyPrintTo(jbuff, jsize + 1);
		ofstream ofs(jsonfile.c_str());//json here
		ofs << jbuff << endl;
		ofs.close();
		delete[] jbuff;
	}
	else
	{
		writelog("Failed to make json, bytes:", (int)jsize);
	}
}


bool loadConfigs(string configfile)
{
	std::ifstream ifs(configfile.c_str());
	ArduinoJson::DynamicJsonBuffer jsonBuffer;
	ArduinoJson::JsonVariant root = jsonBuffer.parseObject(ifs);
	ifs.close();

	JsonArray& jmaskidarr = root["maskidarr"].asArray() ;
	JsonArray& jleftarr = root["leftarr"].asArray();
	JsonArray& jrightarr = root["rightarr"].asArray();
	JsonArray& jtoparr = root["toparr"].asArray();
	JsonArray& jbottomarr = root["bottomarr"].asArray();

	JsonArray& dtarr = root["datatypearr"].asArray();

	int n1 = jmaskidarr.size();
	for (int i = 0; i < n1; ++i)
	{
		g_maskidarr.push_back(jmaskidarr[i]);
		g_leftarr.push_back(jleftarr[i]);
		g_rightarr.push_back(jrightarr[i]);
		g_toparr.push_back(jtoparr[i]);
		g_bottomarr.push_back(jbottomarr[i]);
	}

	int n2 = dtarr.size();
	for (int i = 0; i < n2; ++i)
	{
		JsonObject& dt1 = dtarr[i];
		DataType dt2;
		dt2.pdtname = dt1["pdtname"].asString() ;
		dt2.pdtDisplayName = dt1["pdtDisplayName"].asString();
		dt2.dir = dt1["dir"].asString();
		dt2.inprefix = dt1["inprefix"].asString();
		dt2.intail = dt1["intail"].asString();
		dt2.ymdloc = dt1["ymdloc"].as<double>() ;
		g_dataTypeArray.push_back(dt2);
	}

	g_shengmaskfile = root["shengmask"].asString() ;
	g_spacetem = root["plottem"].asString() ;
	g_gnuplot = root["gnuplot"].asString();
	g_pixelarea = root["pixelarea"].as<double>();

	return true;
}



int main(int argc , char** argv )
{
	cout << "A program to plot some data with combined and subseted data. 2017-12-04. wangfeng1@piesat.cn" << endl;
	cout << "Version 2.0 2017-12-04." << endl;
	cout << "Version 2.0 . modify json val -> value 2017-12-18." << endl;
	if (argc != 3)
	{
		cout << "sample call:" << endl;
		cout << "jiaohuhuitu2snow config.json params.txt" << endl;

		cout << "********* example params.txt *************" << endl;


		exit(101);
	}
	
	//log信息自动保存在 params.txt.log文件中。

	string configfile = argv[1];
	string paramsfile = argv[2];

	g_spacetem = "";
	g_gnuplot = "";
	g_shengmaskfile = "";
	g_pixelarea = 0;

	g_logfile = paramsfile + ".log";
	string outpng = paramsfile + ".png";
	string outjson = paramsfile + ".mjzs.json";

	writelog("start processing ...");

	///////////// fix params ////////////////
	loadConfigs(configfile);



	string pdtname = wft_getValueFromExtraParamsFile(paramsfile,"#pdtname", true);
	string jtype = wft_getValueFromExtraParamsFile(paramsfile,"#jtype", true);
	string timetype = wft_getValueFromExtraParamsFile(paramsfile,"#timetype", true);
	string areatype = wft_getValueFromExtraParamsFile(paramsfile,"#areatype", true);
	string maskidstr = wft_getValueFromExtraParamsFile(paramsfile,"#maskid", true);

	string leftstr = wft_getValueFromExtraParamsFile(paramsfile,"#left", true);
	string rightstr = wft_getValueFromExtraParamsFile(paramsfile,"#right", true);
	string topstr = wft_getValueFromExtraParamsFile(paramsfile,"#top", true);
	string bottomstr = wft_getValueFromExtraParamsFile(paramsfile,"#bottom", true);
	string ymd0str = wft_getValueFromExtraParamsFile(paramsfile,"#ymd0", true);
	string ymd1str = wft_getValueFromExtraParamsFile(paramsfile,"#ymd1", true);

	string region[4]  ;
	double left, right, top, bottom;

	int maskid = 0;
	if (areatype == "rect")
	{
		region[0] = leftstr;
		region[1] = rightstr;
		region[2] = topstr;
		region[3] = bottomstr;

		left = atof(leftstr.c_str());
		right = atof(rightstr.c_str());
		top = atof(topstr.c_str());
		bottom = atof(bottomstr.c_str());

	}
	else {
		maskid = wft_str2int(maskidstr);
		double ll[4];
		bool findmask = findShengLonLatRect(maskidstr, ll, region);
		left = ll[0];
		right = ll[1];
		top = ll[2];
		bottom = ll[3];
		if (findmask == false)
		{
			writelog("Can not find maskid : ", maskid);
			return 11;
		}
	}

	int ymd0 = wft_str2int(ymd0str);
	int ymd1 = wft_str2int(ymd1str);

	bool findDt = false;
	DataType pickedDt;
	for (int i = 0; i < g_dataTypeArray.size(); ++i)
	{
		if (g_dataTypeArray[i].pdtname == pdtname)
		{
			pickedDt = g_dataTypeArray[i];
			findDt = true;
			break;
		}
	}
	if (findDt == false)
	{
		writelog("Can not find configurations for " + pdtname);
		return 20;
	}
	

	GDALAllRegister();
	//convert long lati -> albers x,y
	double ltx, lty, rbx, rby;
	convertLonLatToOyAlbers(left, top, ltx, lty);
	convertLonLatToOyAlbers(right, bottom, rbx, rby);
	
	if (jtype == "space")
	{
		spacejhht( pickedDt, ymd0, ymd1, ltx, lty, rbx, rby , maskid ,outpng, region );
	}
	else
	{
		if (timetype == "nn")
		{
			nianneiJhht(pickedDt, ymd0, ymd1, ltx, lty, rbx, rby, maskid, outjson);
		}
		else
		{
			nianjianJhht(pickedDt, ymd0, ymd1, ltx, lty, rbx, rby, maskid, outjson);
		}
	}




    return 0;
}

















/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////







bool findShengLonLatRect(string shengid, double* lrtbarr, string* lrtbstrarr)
{

	for (int i = 0; i < 32; ++i)
	{
		if (g_maskidarr[i] == shengid)
		{
			lrtbarr[0] = atof(g_leftarr[i].c_str());
			lrtbarr[1] = atof(g_rightarr[i].c_str());
			lrtbarr[2] = atof(g_toparr[i].c_str());
			lrtbarr[3] = atof(g_bottomarr[i].c_str());

			lrtbstrarr[0] = g_leftarr[i];
			lrtbstrarr[1] = g_rightarr[i];
			lrtbstrarr[2] = g_toparr[i];
			lrtbstrarr[3] = g_bottomarr[i];

			return true;
		}
	}
	return false;
}


void convertLonLatToOyAlbers(double lon, double lat, double& refx, double& refy)
{

	OGRSpatialReference wgs84;
	wgs84.SetWellKnownGeogCS("WGS84");

	char* buf = new char[200];
	strcpy(buf, "+proj=aea +ellps=krass +lon_0=95  +lat_1=15 +lat_2=65");
	char abuf[] = "+proj=aea +ellps=krass +lon_0=95  +lat_1=15 +lat_2=65";
	OGRSpatialReference oyAlers;
	oyAlers.importFromProj4(abuf);
	delete[] buf;

	OGRCoordinateTransformation * pConverter = OGRCreateCoordinateTransformation(&wgs84,
		&oyAlers);
	double x = lon;
	double y = lat;
	pConverter->Transform(1, &x, &y);
	refx = x;
	refy = y;

	OCTDestroyCoordinateTransformation(pConverter);
}


bool isGDALAccessable(string filepath)
{
	GDALDataset* ds = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	if (ds == 0)
	{
		return false;
	}
	else {
		GDALClose(ds);
		return true;
	}

}

void writeresultinfo(string& filepath, int code, string description)
{
	ofstream ofs(filepath.c_str());
	ofs << code << endl;
	ofs << description << endl;
	ofs.close();
}

string makeYmdStr(string year, string mon, string day)
{
	int y = (int)atof(year.c_str());
	int m = (int)atof(mon.c_str());
	int d = (int)atof(day.c_str());
	return wft_int2str(y * 10000 + m * 100 + d);
}




void writelog(string line)
{
	cout << line << endl;
	if (g_logfile != "")
	{
		ofstream ofs(g_logfile.c_str(), ifstream::app);
		if (ofs)
		{
			string timestr = wft_current_datetimestr();
			ofs << timestr << " " << line << endl;
			ofs.close();
		}
		else
		{
			cout << "Error : can not open logfile " << g_logfile << endl;
		}
	}
}

void writelog(string str, double value)
{
	cout << str << " " << value << endl;
	if (g_logfile != "")
	{
		ofstream ofs(g_logfile.c_str(), ifstream::app);
		if (ofs)
		{
			string timestr = wft_current_datetimestr();
			ofs << timestr << " " << str << " " << value << endl;
			ofs.close();
		}
		else
		{
			cout << "Error : can not open logfile " << g_logfile << endl;
		}
	}
}

void writelog(string str, int value)
{
	cout << str << " " << value << endl;
	if (g_logfile != "")
	{
		ofstream ofs(g_logfile.c_str(), ifstream::app);
		if (ofs)
		{
			string timestr = wft_current_datetimestr();
			ofs << timestr << " " << str << " " << value << endl;
			ofs.close();
		}
		else
		{
			cout << "Error : can not open logfile " << g_logfile << endl;
		}
	}
}


void getPixelRangeByLonLat(string filepath, double left, double right, double top, double bottom,
	int& x0, int& x1, int& y0, int& y1,
	int& xsize, int& ysize, GDALDataType& dt,
	double* newtrans)
{
	GDALDataset* ds = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	if (ds == 0)
	{
		x0 = 0;
		x1 = 0;
		y0 = 0;
		y1 = 0;
		xsize = 0;
		ysize = 0;
	}
	else {
		double t[6];
		xsize = ds->GetRasterXSize();
		ysize = ds->GetRasterYSize();
		dt = ds->GetRasterBand(1)->GetRasterDataType();
		ds->GetGeoTransform(t);
		x0 = (int)((left - t[0]) / t[1]) - 1;
		x1 = (int)((right - t[0]) / t[1]) + 1;
		y0 = (int)((top - t[3]) / t[5]) - 1;
		y1 = (int)((bottom - t[3]) / t[5]) + 1;
		x0 = MAX(0, x0);
		x1 = MIN(xsize - 1, x1);
		y0 = MAX(0, y0);
		y1 = MIN(ysize - 1, y1);
		GDALClose(ds);

		newtrans[0] = t[0] + t[1] * x0;
		newtrans[1] = t[1];
		newtrans[2] = t[2];
		newtrans[3] = t[3] + t[5] * y0;
		newtrans[4] = t[4];
		newtrans[5] = t[5];

	}
}

