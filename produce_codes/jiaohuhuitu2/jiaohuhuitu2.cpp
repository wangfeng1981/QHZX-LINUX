// jiaohuhuitu2.cpp : 定义控制台应用程序的入口点。
//交互绘图第二版，输入时间和经纬度范围，和输出文件路径


#include <iostream>
#include <ctime>
#include <fstream>
#include "../sharedcodes/wftools.h"
#include <vector>
#include <string>
#include "gdal_priv.h"
using namespace std;
using std::cout;

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


string g_logfile = "temp.log";

void writelog(string line)
{
	cout << line << endl;
	if (g_logfile != "")
	{
		ofstream ofs(g_logfile.c_str(), ifstream::app);
		if (ofs)
		{
			string timestr = wft_current_datetimestr();
			ofs << timestr<< " " <<  line << endl;
			ofs.close();
		}
		else
		{
			cout << "Error : can not open logfile " << g_logfile << endl;
		}
	}
}

void writelog(string str , double value)
{
	cout << str <<" "  << value << endl;
	if (g_logfile != "")
	{
		ofstream ofs(g_logfile.c_str(), ifstream::app);
		if (ofs)
		{
			string timestr = wft_current_datetimestr();
			ofs << timestr << " " << str << " " << value  << endl;
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
	int& x0, int& x1, int& y0, int& y1 ,
	int& xsize , int& ysize , GDALDataType& dt ,
	double* newtrans )
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
		x0 = (int)( (left - t[0]) / t[1] ) - 1;
		x1 = (int)((right - t[0]) / t[1]) + 1;
		y0 = (int)((top - t[3]) / t[5]) -1 ;
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

struct WindData
{
	WindData():lon(0),lat(0),uval(0),vval(0),num(0){} ;
	float lon , lat , uval , vval ;
	int num ;
} ;


void makeWindAverage( string winddir , int ymd0, int ymd1 , string outwindfilepath )
{
	if( winddir == "" ) return ;
	vector<WindData> vec ;
	for(int iymd = ymd0 ; iymd <= ymd1 ; ++ iymd )
	{
		string file1 = winddir + "ncepwind." + wft_int2str(iymd) + ".850.xyuv.day.txt" ;
		if( wft_test_file_exists(file1) == false ) continue ;
		cout<<"averaging "<<file1<<endl ;
		std::ifstream infs(file1.c_str());
		std::string line;
		int iline = 0 ;
		while (std::getline(infs, line))
		{
			if( line.length() >= 1 && line[0] == '#' ) continue ;
			if( line.length() > 6 )
			{
				std::istringstream iss(line);
				float lon ,lat , u1 , v1 ;
				iss >> lon >> lat >> u1 >> v1 ;
				{
					if( iline == vec.size() )
					{
						WindData wd ;
						wd.lon = lon ; wd.lat = lat ;
						wd.uval = u1 ; wd.vval = v1 ; wd.num = 1 ;
						vec.push_back(wd) ;
						++ iline ;
					}else if( iline < vec.size() )
					{
						if( vec[iline].num <= 0 )
						{
							cout<<"Error : empty line match data line. bad wind files ."<<endl ;
							infs.close() ;
							return ;
						}
						vec[iline].num += 1 ;
						vec[iline].uval += u1 ;
						vec[iline].vval += v1 ;
					}else 
					{
						cout<<"Error : iline GT vec.size(). bad wind files ."<<endl ;
						infs.close() ;
						return ;
					}
					
				} 
			}else {
				if( iline == vec.size() )
				{
					WindData wd ;
					wd.num = -1 ;
					vec.push_back(wd) ;
					++ iline ;
				}else if( iline < vec.size() )
				{
					if( vec[iline].num > 0 )
					{
						cout<<"Error : empty line get data line. bad wind files ."<<endl ;
						infs.close() ;
						return ;
					}
				}
			}
		}
		infs.close() ;
	}

	if( vec.size() > 0 )
	{
		ofstream ofs( outwindfilepath.c_str() ) ;
		for( int i = 0 ; i<vec.size() ; ++ i )
		{
			if( vec[i].num <= 0 )
			{
				ofs<<endl ;
			}else {
				ofs<<vec[i].lon<<" "<<vec[i].lat<<" "<<(vec[i].uval/vec[i].num)<<" "<<(vec[i].vval/vec[i].num)<<endl ;
			}
		}
		ofs.close() ;
	}

}



#define CTMAX 0
#define CTAVE 1
#define CTMIN 2

int main(int argc , char** argv )
{
	cout << "A program to plot some data with combined and subseted data. 2017-12-04. wangfeng1@piesat.cn" << endl;
	cout << "Version 2.0 2017-12-04." << endl;
	cout << "Version 2.1 . not delete txt file. txt file has extname of tmp . 2017-12-05." << endl;
	cout << "Version 2.2 . add wind overlay. 2017-12-08." << endl;
	if (argc != 3)
	{
		cout << "sample call:" << endl;
		cout << "jiaohuhuitu2 config.txt params.txt" << endl;

		cout << "output to plot include:" << endl;
		cout << "{{{INFILE}}},{{{OUTFILE}}},{{{DISPLAYNAME}}},{{{DATE0}}},{{{DATE1}}}" << endl;
		cout << "Geo:{{{LEFT}}},{{{RIGHT}}},{{{TOP}}},{{{BOTTOM}}}" << endl;
		cout << "Output PNG Size:{{{OUTWID}}},{{{OUTHEI}}}" << endl;
		cout << "Plot xrange:{{{X0}}},{{{X1}}}" << endl;
		cout << "Plot yrange:{{{Y0}}},{{{Y1}}}" << endl;
		cout << "Value min,max:{{{VMIN}}},{{{VMAX}}}" << endl;
		cout << "Produce time:{{{PTIME}}}" << endl;
		cout << "Combined file count:{{{CFC}}}" << endl;


		cout << "********* example params.txt *************" << endl;
		cout << "#product" << endl;
		cout << "modis-ndvi-max" << endl;
		cout << "#left" << endl;
		cout << "-180.0" << endl;
		cout << "#right" << endl;
		cout << "180" << endl;
		cout << "#top" << endl;
		cout << "90" << endl;
		cout << "#bottom" << endl;
		cout << "-90" << endl;
		cout << "#yyyymmdd0" << endl;
		cout << "20170101" << endl;
		cout << "#yyyymmdd1" << endl;
		cout << "20171231" << endl;
		cout << "**************** end *************" << endl;

		exit(101);
	}
	
	//log信息自动保存在 params.txt.log文件中。

	string configfile = argv[1];
	string paramsfile = argv[2];
	g_logfile = paramsfile + ".log";
	string outpng = paramsfile + ".png";

	writelog("start processing ...");

	string product =  wft_getValueFromExtraParamsFile(paramsfile, "#product", true);
	double left, right, top, bottom;
	string leftstr = wft_getValueDoubleFromExtraParamsFile(paramsfile, "#left" , left , true);
	string rightstr = wft_getValueDoubleFromExtraParamsFile(paramsfile, "#right" , right , true);
	string topstr = wft_getValueDoubleFromExtraParamsFile(paramsfile, "#top",top , true);
	string bottomstr = wft_getValueDoubleFromExtraParamsFile(paramsfile, "#bottom",bottom , true);

	int ymd0, ymd1;
	string ymd0str = wft_getValueIntFromExtraParamsFile(paramsfile, "#yyyymmdd0",ymd0, true);
	string ymd1str = wft_getValueIntFromExtraParamsFile(paramsfile, "#yyyymmdd1",ymd1, true);
	

	string gnuplot = wft_getValueFromExtraParamsFile(configfile,  "#gnuplot", true);

	string productkey = "#" + product + "-" ;
	string indir = wft_getValueFromExtraParamsFile(configfile, productkey+"indir", true);
	string inprefix = wft_getValueFromExtraParamsFile(configfile, productkey + "inprefix", true);
	string intail = wft_getValueFromExtraParamsFile(configfile, productkey + "intail", true);
	double valid0, valid1 , scale , offset ;
	string valid0str = wft_getValueDoubleFromExtraParamsFile(configfile, productkey + "valid0", valid0 , true);
	string valid1str = wft_getValueDoubleFromExtraParamsFile(configfile, productkey + "valid1", valid1 , true);
	string scalestr = wft_getValueDoubleFromExtraParamsFile(configfile, productkey + "scale", scale , true);
	string offsetstr = wft_getValueDoubleFromExtraParamsFile(configfile, productkey + "offset",offset, true);
	string lltype = wft_getValueFromExtraParamsFile(configfile, productkey + "lltype", true);
	int ymdloc;
	string ymdlocstr = wft_getValueIntFromExtraParamsFile(configfile, productkey + "ymdloc" , ymdloc , true);
	string plottem = wft_getValueFromExtraParamsFile(configfile, productkey + "plottem", true);
	string displayname = wft_getValueFromExtraParamsFile(configfile, productkey + "displayname", true);
	string combtype = wft_getValueFromExtraParamsFile(configfile, productkey + "combtype", false);//max/aver
	string winddir = wft_getValueFromExtraParamsFile(configfile, productkey + "winddir", false);//2017-12-08

	string averWindfilepath = outpng + ".wind.tmp" ;
	if( winddir != "" )
	{
		cout<<"Overlay with wind."<<endl ;
		makeWindAverage( winddir , ymd0 , ymd1 , averWindfilepath ) ;
	}else {
		cout<<"No need wind."<<endl ;
	}

	if (combtype == "") combtype = "aver";
	int combtypei = CTAVE;
	if (combtype[1] == 'a') combtypei = CTMAX;
	else if (combtype[1] == 'v') combtypei = CTAVE;

	string txtfile = outpng + ".tmp";

	vector<string> allfiles;
	wft_get_allSelectedFiles(indir, inprefix, intail, -1, "", allfiles);
	int nf = allfiles.size();
	if (nf == 0)
	{
		writelog("No files available.");
		return 10;
	}
	else
	{
		writelog("All files count:", nf);
	}



	GDALAllRegister();


	//pick files from ymd0 to ymd1.
	int numf2 = 0;
	vector<string> selectedFiles;
	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymd = filename.substr(ymdloc, 8);
		int ymdi = wft_str2int(ymd);
		if (ymdi >= ymd0 && ymdi <= ymd1)
		{
			++numf2;
			if (isGDALAccessable(filepath))
			{
				selectedFiles.push_back(filepath);
			}
		}
	}
	int snf = selectedFiles.size();
	writelog("Number of files exists from " + ymd0str + " to " + ymd1str + ":" + wft_int2str(numf2) );
	if (snf == 0)
	{
		writelog("No files is accessable from "+ymd0str + " to " + ymd1str  + ".");
		return 20;
	}
	else {
		writelog("Accessable files count " + wft_int2str(snf) + " from " + ymd0str + " to " + ymd1str + ".");
	}

	string outtif = outpng + ".cb.tif";

	int x0, x1, y0, y1;
	int xsize, ysize;
	double newtrans[6];
	GDALDataType datatype;
	getPixelRangeByLonLat(selectedFiles[0], left, right, top, bottom, x0, x1, y0, y1 , xsize , ysize , datatype , newtrans );
	writelog("pixel range x0,x1,y0,y1:" + wft_int2str(x0) + "," + wft_int2str(x1) + "," + wft_int2str(y0) + "," + wft_int2str(y1));
	int subxsize = x1 - x0 + 1;
	int subysize = y1 - y0 + 1;

	//GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");

	//GDALDataset* outds = driver->Create(outtif.c_str(), subxsize, subysize, 1, datatype, 0);
	//outds->SetGeoTransform(newtrans);
	//{
	//	GDALDataset* ds0 = (GDALDataset*)GDALOpen(selectedFiles[0].c_str(), GDALAccess::GA_ReadOnly);
	//	outds->SetProjection(ds0->GetProjectionRef());
	//	GDALClose(ds0);
	//}

	//totxt
	int xspace = 1;
	int yspace = 1;
	if (subxsize > 1000)
	{
		xspace = subxsize / 1000;
	}
	if (subysize > 1000)
	{
		yspace = subysize / 1000;
	}

	writelog("xspace ", xspace);
	writelog("yspace ", yspace);

	//ofstream ofs(txtfile.c_str() );
	//string txtfile2 = txtfile + ".2.txt";
	FILE* ofptr = fopen(txtfile.c_str(), "w");
	int allsize = subxsize * subysize  ;
	int allsizemb = allsize * 4 / 1000 / 1000;//estimate MB
	double valmin (999999) , valmax (-999999) ;
	if ( allsizemb < 200 )
	{
		short* cntbuff = new short[allsize];
		memset(cntbuff, 0, allsize* sizeof(short));
		float* valbuff = new float[allsize];
		float* sumbuff = new float[allsize];
		for (int i = 0; i < allsize; ++i) sumbuff[i] = valid0 - 1 ;
		for (int ifile = 0; ifile < snf; ++ifile)
		{
			writelog("processing ", ifile);
			GDALDataset* ds = (GDALDataset*)GDALOpen(selectedFiles[ifile].c_str(), GA_ReadOnly);
			if (ds != 0)
			{
				ds->GetRasterBand(1)->RasterIO(GF_Read, x0, y0, subxsize, subysize, valbuff, subxsize, subysize, GDT_Float32, 0, 0, 0);
				for (int iy = 0; iy < subysize; iy += yspace  )
				{
					for (int ix = 0; ix < subxsize; ix += xspace)
					{
						int it = iy*subxsize + ix;
						if (valbuff[it] >= valid0 && valbuff[it] <= valid1)
						{
							if (combtypei == CTMAX) {
								if (sumbuff[it] < valbuff[it])
									sumbuff[it] = valbuff[it];
							}
							else {
								sumbuff[it] += valbuff[it];
							}
							++cntbuff[it];
						}
					}
				}
				GDALClose(ds);
			}
		}
		if (combtypei == CTAVE)
		{
			for (int iy = 0; iy < subysize; iy += yspace )
			{
				float coory = newtrans[3] + newtrans[5] * (iy+1.5);
				for (int ix = 0; ix < subxsize; ix += xspace )
				{
					int it = iy * subxsize + ix;
					float coorx = newtrans[0] + newtrans[1] * (ix+1.5);
					if (cntbuff[it] > 0) {
						sumbuff[it] = sumbuff[it] / cntbuff[it];
						double val = sumbuff[it] * scale + offset;
						if (val > valmax) valmax = val;
						if (val < valmin) valmin = val;
						fprintf(ofptr, "%f %f %f\n", coorx, coory, val );//ofs << coorx << " " << coory << " " << (sumbuff[it]*scale+offset) << endl;
					}
					else {
						fprintf(ofptr, "%f %f NaN\n", coorx, coory);//ofs << coorx << " " << coory << " NaN" << endl;
					}
				}
				fprintf(ofptr, "\n"); //ofs << endl;
			}
		}
		else
		{
			for (int iy = 0; iy < subysize; iy += yspace )
			{
				float coory = newtrans[3] + newtrans[5] * (iy+1.5);
				for (int ix = 0; ix < subxsize; ix += xspace )
				{
					int it = iy * subxsize + ix;
					float coorx = newtrans[0] + newtrans[1] * (ix+1.5);
					if (cntbuff[it] > 0) {
						//ofs << coorx << " " << coory << " " << (sumbuff[it] * scale + offset) << endl;
						double val = sumbuff[it] * scale + offset ;
						if (val > valmax) valmax = val;
						if (val < valmin) valmin = val;
						fprintf(ofptr, "%f %f %f\n", coorx, coory, val );//
					}
					else {
						//ofs << coorx << " " << coory << " NaN" << endl;
						fprintf(ofptr, "%f %f NaN\n", coorx, coory );//
					}
				}
				//ofs << endl;
				fprintf(ofptr, "\n"   );
			}
		}
		//outds->GetRasterBand(1)->RasterIO(GF_Write, 0, 0, subxsize, subysize, sumbuff, subxsize, subysize, GDT_Float32, 0, 0, 0);
		delete[] cntbuff;
		delete[] valbuff;
		delete[] sumbuff;
		//GDALClose(outds);
	}
	else { //line by line
		short* cntbuff = new short[subxsize];
		float* valbuff = new float[subxsize];
		float* sumbuff = new float[subxsize];
		for (int iy = 0; iy < subysize; iy += yspace )
		{
			memset(cntbuff, 0, subxsize* sizeof(short));
			for (int i = 0; i < subxsize; ++i) sumbuff[i] = valid0 - 1;
			for (int ifile = 0; ifile < snf; ++ifile)
			{
				GDALDataset* ds = (GDALDataset*)GDALOpen(selectedFiles[ifile].c_str(), GA_ReadOnly);
				if (ds != 0)
				{
					ds->GetRasterBand(1)->RasterIO(GF_Read, 0, y0 + iy, subxsize, 1, valbuff, subxsize, 1, GDT_Float32, 0, 0, 0);
					for (int ix = 0; ix < subxsize; ix += xspace )
					{
						if (valbuff[ix] >= valid0 && valbuff[ix] <= valid1)
						{
							if (combtypei == CTMAX) {
								if (sumbuff[ix] < valbuff[ix])
									sumbuff[ix] = valbuff[ix];
							}
							else {
								sumbuff[ix] += valbuff[ix];
							}
							++cntbuff[ix];
						}
					}
					GDALClose(ds);
				}
			}
			float coory = newtrans[3] + newtrans[5] * (iy+1.5);
			if (combtypei == CTAVE)
			{
				for (int ix = 0; ix < subxsize; ix+=xspace )
				{
					float coorx = newtrans[0] + newtrans[1] *( ix+1.5);
					if (cntbuff[ix] > 0) {
						sumbuff[ix] = sumbuff[ix] / cntbuff[ix];
						double val = sumbuff[ix] * scale + offset;
						if (val > valmax) valmax = val;
						if (val < valmin) valmin = val;
						fprintf(ofptr, "%f %f %f\n", coorx, coory,  val );//ofs << coorx << " " << coory << " " << (sumbuff[ix]*scale+offset) << endl;
					}
					else
					{
						fprintf(ofptr, "%f %f NaN\n", coorx, coory);//ofs << coorx << " " << coory << " NaN" << endl;
					}
				}
			}
			else {
				for (int ix = 0; ix < subxsize; ix += xspace)
				{
					float coorx = newtrans[0] + newtrans[1] * (ix+1.5);
					if (cntbuff[ix] > 0) {
						double val = sumbuff[ix] * scale + offset;
						if (val > valmax) valmax = val;
						if (val < valmin) valmin = val;
						fprintf(ofptr, "%f %f %f\n", coorx, coory, val );//ofs << coorx << " " << coory << " " <<( sumbuff[ix]*scale+offset) << endl;
					}
					else
					{
						fprintf(ofptr, "%f %f NaN\n", coorx, coory);//ofs << coorx << " " << coory << " NaN" << endl;
					}
				}
			}
			fprintf(ofptr, "\n"); //ofs << endl;
			//outds->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, subxsize, 1, sumbuff, subxsize, 1, GDT_Float32, 0, 0, 0);
			wft_term_progress(iy, subysize);
		}

		delete[] cntbuff;
		delete[] valbuff;
		delete[] sumbuff;
		//GDALClose(outds);
	}
	fclose(ofptr); ofptr = 0;
	//ofs.close();

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
		outhei = subysize*1.0 / subxsize * outwid;
	}
	else {
		outhei = 1000;
		outwid = subxsize*1.0 / subysize * outhei;
	}

	string dt0 = wft_convert_ymdi2y_m_d(ymd0);
	string dt1 = wft_convert_ymdi2y_m_d(ymd1);

	//string title1 = displayname + ". Date Range from " + wft_convert_ymdi2y_m_d(ymd0) + " to " + wft_convert_ymdi2y_m_d(ymd1)  ;
	//string title2 = "Region left " + leftstr + " right " + rightstr + " top " + topstr + " bottom " + bottomstr;
	//string ptime = "Produced:" + datetimeNow + "; Combined files count:" + wft_int2str(snf) + "." ;
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
	repVec.push_back(outpng);

	repVec.push_back(displayname);

	repVec.push_back(dt0);
	repVec.push_back(dt1);

	repVec.push_back(leftstr);
	repVec.push_back(rightstr);
	repVec.push_back(topstr);
	repVec.push_back(bottomstr);

	repVec.push_back(wft_int2str(outwid));
	repVec.push_back(wft_int2str(outhei));

	repVec.push_back(leftstr);
	repVec.push_back(rightstr);
	repVec.push_back(bottomstr);
	repVec.push_back(topstr);

	repVec.push_back(wft_double2str(valmin));
	repVec.push_back(wft_double2str(valmax));

	repVec.push_back(datetimeNow);
	repVec.push_back(wft_int2str(snf));


	string plotfile = outpng + ".plot";
	wft_create_file_by_template_with_replacement(  plotfile , plottem , varVec, repVec);

	string cmd2 = gnuplot + " " + plotfile;
	int res2 = system(cmd2.c_str());
	cout << "gnuplot return code " << res2 << endl;
	
	//wft_remove_file(txtfile);

    return 0;
}

