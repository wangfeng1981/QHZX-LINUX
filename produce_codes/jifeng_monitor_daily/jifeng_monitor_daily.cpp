// jifeng_monitor_daily.cpp : 定义控制台应用程序的入口点。
//季风日产品监测，绘图需要水汽总量和风场数据 2017-11-10
//能够叠加OLR

#include <iostream>
#include <ctime>
#include <fstream>
#include "../sharedcodes/wftools.h"
#include <vector>
#include <string>
#include "gdal_priv.h"
using namespace std;


#define INPUT_TYPE_TPW 0
#define INPUT_TYPE_OLR 1


struct POption {
	string indir ;
	string outdir ;
	string inprefix ;
	string intail ;
	string pid ;
	int ymdloc ;
	string sjcdlong ;
	string sjcdshort ;
	string plottem ;
	string valid0 ;
	string valid1 ;
} ;





string g_insert("/QHZX_DATA/produce_codes/insertdb/insertdb") ,  
	g_host("localhost") , g_user("htht") , g_pwd("htht123456") , g_db("qhzx_uus") , g_tb("tb_product_data")  ;


int processOneFile(
	string& imagefilepath  ,
	string& windfilepath ,
	string& outpngpath , 
	string& txtProgram,
	string& plotTemplate,
	string& plotProgram,
	string valid0str , 
	string valid1str ,
	string ymdstr , 
	string sjcd  ,
	string pid 
	)
{

	string temp_txtfile = outpngpath + ".xyz.tmp";
	string cmd4 = txtProgram + " -in " + imagefilepath + " -out " + temp_txtfile 
		+ " -valid0 " + valid0str  
		+ " -valid1 " + valid1str   
		+ " -scale 1 -offset 0.0 -nan NaN -x0 30 -x1 180 -y0 -20  -y1 50 ";
	cout << cmd4 << endl;
	int res4 = system(cmd4.c_str());
	cout << "image2xyz result:" << res4 << endl;
	if (wft_test_file_exists(temp_txtfile))
	{
		string ymdstr2 = wft_convert_ymd2y_m_d(ymdstr) ;

		vector<string> varvector, repvector;
		varvector.push_back("{{{INFILE1}}}");
		varvector.push_back("{{{INFILE2}}}");
		varvector.push_back("{{{OUTFILE}}}");
		varvector.push_back("{{{YMD}}}");
		varvector.push_back("{{{SJCD}}}");

		repvector.push_back(temp_txtfile);
		repvector.push_back(windfilepath);
		repvector.push_back(outpngpath);
		repvector.push_back(ymdstr2);
		repvector.push_back(sjcd);

		string temp_plotfile = outpngpath + ".plot";
		wft_create_file_by_template_with_replacement(temp_plotfile, plotTemplate, varvector, repvector);
		if (wft_test_file_exists(temp_plotfile))
		{
			string cmd6 = plotProgram + " " + temp_plotfile;
			int res6 = system(cmd6.c_str());
			cout << "plot result:" << res6 << endl;
			if (wft_test_file_exists(outpngpath))
			{
				//insert db.
				string cmd7 = g_insert + " -host " + g_host + " -user " + g_user
					+ " -pwd " + g_pwd + " -db " + g_db + " -tb " + g_tb
					+ " -datapath " + outpngpath
					+ " -dtloc -1 "  
					+ " -dtlen 0 "
					+ " -thumb " + outpngpath
					+ " -pid " + pid 
					+ " -startdate " + ymdstr2 
					+ " -enddate " + ymdstr2 ;

				cout << cmd7 << endl;
				int res7 = system(cmd7.c_str());
				cout << "insertdb result:" << res7 << endl;

				//delete temp files.
				wft_remove_file(temp_txtfile);

				return 100;
			}
			else {
				cout << "*** Error : failed to make png file " << outpngpath << endl;
				return 0;
			}
		}
		else {
			cout << "*** Error : failed to make plot file " << temp_plotfile << endl;
			return 0;
		}
	}
	else
	{
		cout << "*** Error : failed to make txt file  " << temp_txtfile << endl;
		return 0;
	}


	
	return 0;
}




void outputWindXyzfile(string outfilepath, int yearday, int year, int month, int day , int xsize , int ysize , float* buffer ,float* buffer1 )
{
	//-125.0f -- 160.0f
	ofstream ofs(outfilepath.c_str());
	ofs << "#x y uwnd vwnd yearday:"<<yearday<<" => year-month-day:"<<year<< "-"<<month<<"-"<<day << endl;
	float x0 = 0.0f;
	float y0 = 90.0;
	for (int iy = 0; iy < ysize; ++iy)
	{
		for (int ix = 0; ix < xsize; ++ix)
		{
			float x = x0 + ix * 2.5;
			x = x - 180;
			float y = y0 - iy * 2.5;
			if (buffer[iy*xsize + ix] >= -125.0f && buffer[iy*xsize + ix] <= 160.f)
			{
				ofs << x << " " << y << " " << buffer[iy*xsize + ix] << " " << buffer1[iy*xsize +ix] << endl;
			}
			else
			{
				ofs << x << " " << y << " NaN NaN " << endl;
			}

		}
		ofs << endl;
	}
	ofs.close();

}


void arraySum( float* arrsum , float* arr, int* cntarr , int size )
{
	for(int i = 0 ; i<size ; ++ i )
	{
		if ( arr[i] >= -125.0f && arr[i] <= 160.f)
		{
			arrsum[i] += arr[i] ;
			++ cntarr[i] ;
		}
	}
}

void arrayAver( float* arrSumAver , int* cntarr , int size )
{
	for(int i = 0 ; i<size ; ++ i )
	{
		if ( cntarr[i] > 0 )
		{
			arrSumAver[i] = arrSumAver[i]/cntarr[i] ;
		}
	}
}


void processOneWindPairFile(string uncfilepath , string vncfilepath , string outdir )
{
	int numLevels = 17;
	int levelIndex = 2;
	string levelName = "850";
	string ncfilename = wft_base_name(uncfilepath);
	string fileYearStr = ncfilename.substr(5, 4);
	int fileYear = (int)atof(fileYearStr.c_str());

	string udspath = string("NETCDF:\"") + uncfilepath + "\":uwnd" ;
	string vdspath = string("NETCDF:\"") + vncfilepath + "\":vwnd" ;

	GDALDataset* uds = (GDALDataset*)GDALOpen(udspath.c_str(), GA_ReadOnly);
	GDALDataset* vds = (GDALDataset*)GDALOpen(vdspath.c_str(), GA_ReadOnly);
	const int rasterXSize = uds->GetRasterXSize();
	const int rasterYSize = uds->GetRasterYSize();
	int nband = uds->GetRasterCount();
	int nband1 = vds->GetRasterCount();
	if (nband == nband1)
	{
		cout << "x , y , nband  " << endl;
		cout << rasterXSize << " " << rasterYSize << " " << nband << endl;

		float* buffer = new float[rasterXSize*rasterYSize];
		float* buffer1 = new float[rasterXSize*rasterYSize];


		
		int allsize = rasterXSize* rasterYSize ;
		float* monSumArray1 = new float[allsize] ;
		float* monSumArray2 = new float[allsize] ;
		float* fivSumArray1 = new float[allsize] ;
		float* fivSumArray2 = new float[allsize] ;
		int* monCntArray1 = new int[allsize] ;
		int* monCntArray2 = new int[allsize] ;
		int* fivCntArray1 = new int[allsize] ;
		int* fivCntArray2 = new int[allsize] ;

		int numberOfDays = (int)(nband / numLevels);
		cout << "number of days in ncfile:" << numberOfDays << endl;

		int theMonth = 0 ;
		int theFiveDays = 0 ;
 

		char fnamebuffer[2048];
		for (int iday = 0; iday < numberOfDays; ++iday)
		{
			int dayOfYear = iday + 1;
			int month, day;
			wft_convertdayofyear2monthday(fileYear, dayOfYear, month, day);
			sprintf(fnamebuffer, "%sncepwind.%d%02d%02d.%s.xyuv.day.txt", outdir.c_str(), fileYear, month, day, levelName.c_str());
			string xyzfilepath = string(fnamebuffer);

			int bandindex = iday * numLevels + levelIndex + 1;
			uds->GetRasterBand(bandindex)->RasterIO(GF_Read, 0, 0, rasterXSize, rasterYSize,
					buffer, rasterXSize, rasterYSize, GDT_Float32, 0, 0, 0);
			vds->GetRasterBand(bandindex)->RasterIO(GF_Read, 0, 0, rasterXSize, rasterYSize,
					buffer1, rasterXSize, rasterYSize, GDT_Float32, 0, 0, 0);

			if (wft_test_file_exists(xyzfilepath) == false)
			{
				cout << "it is making " << xyzfilepath << endl;
				
				outputWindXyzfile(xyzfilepath, dayOfYear, fileYear, month, day, rasterXSize, rasterYSize, buffer, buffer1 );
			}


			{//five days.
				if( theFiveDays != day && (day==1||day==6||day==11||day==16||day==21||day==26) )
				{
					memset( fivSumArray1 , 0 , sizeof(float) * allsize ) ;
					memset( fivSumArray2 , 0 , sizeof(float) * allsize ) ;
					memset( fivCntArray1 , 0 , sizeof(int) * allsize ) ;
					memset( fivCntArray2 , 0 , sizeof(int) * allsize ) ;
					theFiveDays = day ;
				}

				char fnamebuffer2[1024] ;
				sprintf(fnamebuffer2, "%sncepwind.%d%02d%02d.%s.xyuv.fiv.txt", outdir.c_str(), fileYear,
										 month, theFiveDays , levelName.c_str());
				string xyzfilepath2 = string(fnamebuffer2);
				if (wft_test_file_exists(xyzfilepath2) == false)
				{
					
					arraySum( fivSumArray1 , buffer , fivCntArray1 , allsize) ;
					arraySum( fivSumArray2 , buffer1 , fivCntArray2 , allsize) ;

					int dayOfYear2 = dayOfYear + 1 ;
					int month2 , day2 ;
					wft_convertdayofyear2monthday(fileYear, dayOfYear2, month2, day2);
					if( ( theFiveDays!=26 && day2 >= theFiveDays + 5) || (theFiveDays==26 && month2 > month ) )
					{
						arrayAver( fivSumArray1 , fivCntArray1 , allsize ) ;
						arrayAver( fivSumArray2 , fivCntArray2 , allsize ) ;

						cout << "it is making five-day file " << xyzfilepath2 << endl;
						outputWindXyzfile(xyzfilepath2, dayOfYear, fileYear, month, theFiveDays
							, rasterXSize, rasterYSize, fivSumArray1, fivSumArray2 );
						
					}
				}
			
			}

			//do month average.
			{
				if( theMonth != month )
				{
					memset( monSumArray1 , 0 , sizeof(float) * allsize ) ;
					memset( monSumArray2 , 0 , sizeof(float) * allsize ) ;
					memset( monCntArray1 , 0 , sizeof(int) * allsize ) ;
					memset( monCntArray2 , 0 , sizeof(int) * allsize ) ;
					theMonth = month ;
				}

				char fnamebuffer2[1024] ;
				sprintf(fnamebuffer2, "%sncepwind.%d%02d%02d.%s.xyuv.mon.txt", outdir.c_str(), fileYear, month, 1, levelName.c_str());
				string xyzfilepath2 = string(fnamebuffer2);
				if (wft_test_file_exists(xyzfilepath2) == false)
				{
					
					arraySum( monSumArray1 , buffer , monCntArray1 , allsize) ;
					arraySum( monSumArray2 , buffer1 , monCntArray2 , allsize) ;

					int dayOfYear2 = dayOfYear + 1 ;
					int month2 , day2 ;
					wft_convertdayofyear2monthday(fileYear, dayOfYear2, month2, day2);
					if( month2 != month )
					{
						arrayAver( monSumArray1 , monCntArray1 , allsize ) ;
						arrayAver( monSumArray2 , monCntArray2 , allsize ) ;
						cout << "it is making month file " << xyzfilepath2 << endl;
						outputWindXyzfile(xyzfilepath2, dayOfYear, fileYear, month, 1, rasterXSize, rasterYSize, monSumArray1, monSumArray2 );
					}
				}
				
			}


			

		}

		delete[] monSumArray1 ;
		delete[] monSumArray2 ;

		delete[] fivSumArray1 ;
		delete[] fivSumArray2 ;

		delete[] monCntArray1 ;
		delete[] monCntArray2 ;

		delete[] fivCntArray1 ;
		delete[] fivCntArray2 ;


		delete[] buffer; buffer = 0;
		delete[] buffer1; buffer1 = 0;

		GDALClose(uds);
		GDALClose(vds);
		//change nc file name 
		string ymd = wft_current_dateymd();
		string uncfilepathback = uncfilepath + "." + ymd + ".backup";
		string vncfilepathback = vncfilepath + "." + ymd + ".backup";
		rename(uncfilepath.c_str(), uncfilepathback.c_str());
		rename(vncfilepath.c_str(), vncfilepathback.c_str());
	}
	else {
		cout << "Error: uwnd band " << nband << " is not equal vwnd band " << nband1 << endl;
		GDALClose(uds);
		GDALClose(vds);
	}
}

void loadOptions( string startup , vector<POption> & opVec )
{
	int num = 20 ;
	for(int i = 0 ; i<num ; ++ i )
	{
		string tag = "#op" + wft_int2str(i)  ;
		POption op ;
		op.indir = wft_getValueFromExtraParamsFile( startup , tag+"-indir" , false) ;
		if( op.indir != "" )
		{
			op.outdir = wft_getValueFromExtraParamsFile( startup , tag+"-outdir" , true) ;
			op.inprefix = wft_getValueFromExtraParamsFile( startup , tag+"-inprefix" , true) ;
			op.intail = wft_getValueFromExtraParamsFile( startup , tag+"-intail" , true) ;
			op.pid = wft_getValueFromExtraParamsFile( startup , tag+"-pid"  , true) ;
			string ymdlocstr = 
			      wft_getValueIntFromExtraParamsFile( startup , tag+"-ymdloc", op.ymdloc , true) ;
			op.sjcdlong = wft_getValueFromExtraParamsFile( startup , tag+"-sjcdlong" , true) ;
			op.sjcdshort = wft_getValueFromExtraParamsFile( startup , tag+"-sjcdshort" , true) ;
			op.valid0 = wft_getValueFromExtraParamsFile( startup , tag+"-valid0" , true) ;
			op.valid1 = wft_getValueFromExtraParamsFile( startup , tag+"-valid1" , true) ;
			op.plottem = wft_getValueFromExtraParamsFile( startup , tag+"-plottem" , true) ;
			opVec.push_back(op) ;
			cout<<"add op"<<i<<endl ;
		}
	}
}


int main(int argc, char** argv)
{
	cout << "A program to plot jifeng(TPW/OLR + Wind) and insert db." << endl;
	cout << "Version 0.1a by wangfeng1@piesat.cn 2017-11-10." << endl;
	cout << "Version 0.1.1a bugfixed for finding tpw file. 2017-11-10." << endl;
	cout << "Version 0.2a add support of OLR. 2017-11-15." << endl;//2017-11-15
	cout << "Version 2a do it all in one. 2017-12-08." << endl;

	if (argc == 1)
	{
		cout << "sample call:" << endl;
		cout << "jifeng_monitor_daily startup.txt" << endl;
		cout << "******** startup.txt sample ********" << endl;
		
		cout << "**** **** **** **** ****" << endl;

		exit(101);
	}

	string startupfile = argv[1];

	////////////////////////////////////
	
	GDALAllRegister();
	string windoutdir = wft_getValueFromExtraParamsFile(startupfile, "#windoutdir", true);
	string windindir = wft_getValueFromExtraParamsFile(startupfile, "#windindir", true);
	string txtProgram = wft_getValueFromExtraParamsFile(startupfile, "#totxt", true);
	string plotProgram = wft_getValueFromExtraParamsFile(startupfile, "#gnuplot", true);

	g_insert = wft_getValueFromExtraParamsFile(startupfile, "#insert", true);
	g_host = wft_getValueFromExtraParamsFile(startupfile, "#host", true);
	g_user = wft_getValueFromExtraParamsFile(startupfile, "#user", true);
	g_pwd = wft_getValueFromExtraParamsFile(startupfile, "#pwd", true);
	g_db = wft_getValueFromExtraParamsFile(startupfile, "#db", true);
	g_tb = wft_getValueFromExtraParamsFile(startupfile, "#tb", true);

	cout<<"extracting uwnd and vwnd into xyz file."<<endl ;
	for (int i = 2010; i < 3000 ; ++i)
	{
		string yearStr = wft_int2str(i);
		string uwndfilepath = windindir + "uwnd." + yearStr + ".nc";
		string vwndfilepath = windindir + "vwnd." + yearStr + ".nc";
		if (wft_test_file_exists(uwndfilepath) && wft_test_file_exists(vwndfilepath))
		{
			processOneWindPairFile(uwndfilepath , vwndfilepath , windoutdir);
		}

	}
	cout << "extract done." << endl;

	

	vector<POption> optionVec ;
	loadOptions( startupfile , optionVec ) ;

	for(int iop = 0 ; iop < optionVec.size() ; ++ iop )
	{
		POption op1 = optionVec[iop]  ;
		vector<string> selectedfiles ;
		wft_get_allSelectedFiles( op1.indir , op1.inprefix , op1.intail , -1 , "" , selectedfiles ) ;
		int numfiles = selectedfiles.size() ;
		for( int ifile = 0 ; ifile < numfiles ; ++ ifile )
		{
			cout<<iop<<"/"<<ifile<<"/"<<numfiles<<endl ;
			string filepath = selectedfiles[ifile] ;
			string filename = wft_base_name(filepath) ;

			string outname = filename + ".monsoons.png" ;
			string outpath = op1.outdir + outname ;

			if( wft_test_file_exists(outpath)  ==  false )
			{
				cout<<"making "<<outpath<<endl ;
				string ymdstr = filename.substr( op1.ymdloc , 8 ) ;
				string windfilename = "ncepwind." + ymdstr + ".850.xyuv." + op1.sjcdshort + ".txt" ;
				string windpath = windoutdir + windfilename ;
				if( wft_test_file_exists(windpath) == false )
				{
					cout<<"Warning no wind file "<<windpath<<endl ;
					continue ;
				}else {
					int ret = processOneFile( 
						filepath ,
						windpath , 
						outpath , 
						txtProgram , 
						op1.plottem , 
						plotProgram , 
						op1.valid0  , 
						op1.valid1 , 
						ymdstr , 
						op1.sjcdlong , 
						op1.pid 
						);
					if (ret == 100)
					{
						//go on!
					}

				}

			}
		}
	}



	cout << "done." << endl;
	return 0;
}


