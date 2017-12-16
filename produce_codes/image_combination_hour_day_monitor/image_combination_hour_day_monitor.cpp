// image_combination_hour_day_monitor.cpp
// 
//wangfeng1@piesat.cn 2017-12-05




#include <iostream>
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include "../sharedcodes/wftools.h"
using namespace std;
using std::cout;

#define METHOD_MAX 0
#define METHOD_AVER 1


void printCurrentTime() {
	//#include <ctime>
	time_t time0;
	time(&time0);
	std::cout << asctime(localtime(&time0)) << std::endl;
}



string makeOutFilename(string outprefix, string outtail, string timestr , string datetimeStr , string methodstr )
{
	//noaa-ndvi-20170101.c030.max.mon.tif
	string outname = outprefix + datetimeStr + "." + methodstr + "." + timestr + outtail;
	return outname;
}

void makeCinfofile(string outpath, int combcnt , vector<string>& filelist )
{
	char buff[10];
	sprintf(buff, "c%04d", combcnt);
	string outpath2 = outpath + "." + string(buff) + ".txt" ;
	ofstream ofs(outpath2.c_str());
	ofs << combcnt << endl;
	for(int i= 0 ; i<filelist.size() ; ++ i )
	{
		ofs<<i<<" "<<filelist[i]<<endl ;
	}
	ofs.close();
}

int selectedFilesByYmdhRange(vector<string>& allfiles, vector<int>& ymdhVec,
	 vector<string>& outfiles, int ymdh0, int ymdh1)
{
	int num = ymdhVec.size();
	int cnt = 0;
	for (int i = 0; i < num; ++i)
	{
		int cymdh = ymdhVec[i];
		if (cymdh >= ymdh0 && cymdh <= ymdh1)
		{
			outfiles.push_back(allfiles[i]);
			++cnt;
		}
	}
	return cnt;
}


int processOne( vector<string>& selfiles , string dsprefix , string dstail ,
 int method , string outpath , double valid0 , double valid1 )
{
	cout << "making " << outpath << " ... " << endl;
	if (selfiles.size() == 0) return 0;

	//get output parameters.
	int rasterXSize = 0;
	int rasterYSize = 0;
	GDALDataType theDataType;
	double geotrans[6];//17
	char projection[2000];//17
	{
		string dspath = dsprefix + selfiles[0] + dstail;
		GDALDataset* tds = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
		if (tds == 0)
		{
			return 10;
		}
		tds->GetGeoTransform(geotrans);//17
		const char* proj = tds->GetProjectionRef();//17
		strcpy(projection, proj);//17
		rasterXSize = tds->GetRasterXSize();
		rasterYSize = tds->GetRasterYSize();
		theDataType = tds->GetRasterBand(1)->GetRasterDataType();
		GDALClose(tds);
	}


	std::string outputValidCountFilepath = outpath + ".validcnt.tif";
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outputDataset = driver->Create(outpath.c_str(), rasterXSize, rasterYSize, 1, theDataType, 0);
	GDALDataset* outputValidCountDataset = driver->Create(outputValidCountFilepath.c_str(), rasterXSize, rasterYSize, 1, GDT_Int16, 0);
	outputDataset->SetGeoTransform(geotrans);//17
	outputDataset->SetProjection(projection);//17
	int bufferLines = 100;
	double* dataBuffer = new double[rasterXSize*bufferLines];
	int*    countBuffer = new int[rasterXSize*bufferLines];
	double* outBuffer = new double[rasterXSize*bufferLines];
	int bufferSize = bufferLines * rasterXSize;

	if (dataBuffer == 0 || countBuffer == 0 || outBuffer == 0)
	{
		cout << "Error : can not alloc more memories databuffer , countbuffer , outbuffer." << endl;
		if (dataBuffer) delete[] dataBuffer;
		if (countBuffer) delete[] countBuffer;
		if (outBuffer) delete[] outBuffer;
		GDALClose(outputDataset);
		GDALClose(outputValidCountDataset);
		wft_remove_file(outpath);
		wft_remove_file(outputValidCountFilepath);
		return 30;
	}

	int numBlocks = (rasterYSize + bufferLines - 1) / bufferLines;

	int dsCount = (int)selfiles.size();
	for (int iby = 0; iby < numBlocks; ++iby)
	{
		int y0Index = iby * bufferLines;
		int useLines = MIN(bufferLines, rasterYSize - y0Index);

		for (int it = 0; it < bufferSize; ++it)
		{
			outBuffer[it] = 0;
			countBuffer[it] = 0;
		}

		for (int ids = 0; ids < dsCount; ++ids)
		{
			string dspath = dsprefix + selfiles[ids] + dstail;
			GDALDataset* dstemp = (GDALDataset*)GDALOpen(dspath.c_str(), GA_ReadOnly);
			if (dstemp == 0)
			{
				std::cout << "*** Error : can not open " << dspath << ". out." << std::endl;
				delete[] dataBuffer;
				delete[] countBuffer;
				delete[] outBuffer;
				GDALClose(outputDataset);
				GDALClose(outputValidCountDataset);
				wft_remove_file(outpath);
				wft_remove_file(outputValidCountFilepath);
				return 21;
			}
			int nx = dstemp->GetRasterXSize();
			int ny = dstemp->GetRasterYSize();
			if (nx != rasterXSize || ny != rasterYSize) {
				std::cout << "*** Error : " << dspath << " raster data size is not equal with others. out." << std::endl;
				GDALClose(dstemp);
				GDALClose(outputDataset);
				delete[] dataBuffer;
				delete[] countBuffer;
				delete[] outBuffer;
				wft_remove_file(outpath);
				wft_remove_file(outputValidCountFilepath);
				return 20;
			}
			//bug-20171210-1555 dstemp->GetRasterBand(1)->RasterIO(GF_Read, 0, y0Index, rasterXSize, useLines,
			//	dataBuffer, rasterXSize, bufferLines, GDT_Float64, 0, 0, 0);//bug bufferLines
			 dstemp->GetRasterBand(1)->RasterIO(GF_Read, 0, y0Index, rasterXSize, useLines,
				dataBuffer, rasterXSize, useLines, GDT_Float64, 0, 0, 0);//bugfixed use useLInes replace bufferlines 2017-12-10.
			if (method == METHOD_AVER)
			{//
				for (int iy = 0; iy < useLines; ++iy)
				{
					for (int ix = 0; ix < rasterXSize; ++ix)
					{
						int tindex = iy*rasterXSize + ix;
						double inval = dataBuffer[tindex];
						if (valid0 <= inval && inval <= valid1)
						{
							if (countBuffer[tindex] == 0) {
								outBuffer[tindex] = inval;
							}
							else {
								outBuffer[tindex] += inval;
							}
							++countBuffer[tindex];
						}
						else if (countBuffer[tindex] == 0) {
							outBuffer[tindex] = inval;//
						}
					}
				}
			}
			else //if (method == METHOD_MAX)
			{//max
				for (int iy = 0; iy < useLines; ++iy)
				{
					for (int ix = 0; ix < rasterXSize; ++ix)
					{

						int tindex = iy*rasterXSize + ix;
						double inval = dataBuffer[tindex];

						if (valid0 <= inval && inval <= valid1)
						{
							if (countBuffer[tindex] == 0) {
								outBuffer[tindex] = inval;
							}
							else if (outBuffer[tindex] < inval)
							{
								outBuffer[tindex] = inval;
							}
							++countBuffer[tindex];
						}
						else if (countBuffer[tindex] == 0) {
							outBuffer[tindex] = inval;//
						}
					}
				}
			}
			
			GDALClose(dstemp);
		}

		if (method == METHOD_AVER)
		{
			for (int iy = 0; iy < useLines; ++iy)
			{
				for (int ix = 0; ix < rasterXSize; ++ix)
				{
					int tindex = iy*rasterXSize + ix;
					if (countBuffer[tindex] > 0)
					{
						outBuffer[tindex] = outBuffer[tindex] / countBuffer[tindex];
					}
				}
			}
		}

		outputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, y0Index, rasterXSize, useLines,
			outBuffer, rasterXSize, useLines, GDT_Float64, 0, 0, 0);//bugfixed 2017-11-29.
		outputValidCountDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, y0Index, rasterXSize, useLines,
			countBuffer, rasterXSize, useLines, GDT_Int32, 0, 0, 0);//bugfixed 2017-11-29.

		wft_term_progress(iby , numBlocks) ;
	}

	delete[] dataBuffer; dataBuffer = 0;
	delete[] countBuffer; countBuffer = 0;
	delete[] outBuffer; outBuffer = 0;

	GDALClose(outputDataset);
	GDALClose(outputValidCountDataset);

	//2017-10-26
	wft_remove_file(outputValidCountFilepath);

	makeCinfofile(outpath, (int)selfiles.size() ,selfiles);
	return 100;
}



int main(int argc, char** argv)
{
	printCurrentTime();
	std::cout << "Description: A program to auto combine image into hourly(hou),daily(ten). " << std::endl;
	std::cout << "Version 0.1a . by wangfengdev@163.com 2017-12-05." << std::endl;
	std::cout << "Version 1.1a . add support LPW .2017-12-07." << std::endl;
	std::cout << "Version 1.2a . bugfixed bug-20171210-1555. 2017-12-10." << std::endl;
	
	if (argc == 1)
	{
		std::cout << "*** sample call: ***" << std::endl;
		std::cout << "image_combination_hour_day_monitor startup.txt" << endl;
		std::cout << "*** startup.txt example ***" << std::endl;
		cout << "#indir" << endl;
		cout << "D:/avhrr_ndvi/combinations/qcdaily/" << endl;
		cout << "#outhoudir" << endl;
		cout << "/.../hour/" << endl;
		cout << "#outdaydir" << endl;
		cout << "/.../day/" << endl;
		cout << "#fnfixprefix" << endl;
		cout << "FY4A_..._" << endl;
		cout << "#fnfixtail" << endl;
		cout << ".NC" << endl;
		cout << "#ymdloc" << endl;
		cout << "24" << endl;
		cout << "#outprefix" << endl;
		cout << "fy4_agri_ndisk_1047_" << endl;
		cout << "#outtail" << endl;
		cout << ".tif" << endl;
		cout << "#dsprefix" << endl;
		cout << "" << endl;
		cout << "#dstail" << endl;
		cout << "" << endl;
		cout << "#hou" << endl;
		cout << "1" << endl;
		cout << "#day" << endl;
		cout << "1" << endl;
		cout << "#method" << endl;
		cout << "max/aver" << endl;
		cout << "#valid0" << endl;
		cout << "40" << endl;
		cout << "#valid1" << endl;
		cout << "450" << endl;
		cout << "#fill" << endl;
		cout << "-9999" << endl;
		cout << "#islpw[optional]" << endl;
		cout << "0" << endl;
		cout << "" << endl;
		return 101;
	}

	std::cout << "processing..." << std::endl;
	string startup = argv[1];

	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	string outhoudir = wft_getValueFromExtraParamsFile(startup, "#outhoudir", true);
	string outdaydir = wft_getValueFromExtraParamsFile(startup, "#outdaydir", true);
	string inprefix = wft_getValueFromExtraParamsFile(startup, "#fnfixprefix", true);
	string intail = wft_getValueFromExtraParamsFile(startup, "#fnfixtail", true);

	string ymdlocstr = wft_getValueFromExtraParamsFile(startup, "#ymdloc", true);
	int ymdloc = (int)atof(ymdlocstr.c_str());
	string outprefix = wft_getValueFromExtraParamsFile(startup, "#outprefix", true);
	string outtail = wft_getValueFromExtraParamsFile(startup, "#outtail", true);

	string dsprefix = wft_getValueFromExtraParamsFile(startup, "#dsprefix", true);
	string dstail = wft_getValueFromExtraParamsFile(startup, "#dstail", true);

	string houstr = wft_getValueFromExtraParamsFile(startup, "#hou", true);
	string daystr = wft_getValueFromExtraParamsFile(startup, "#day", true);

	string valid0str = wft_getValueFromExtraParamsFile(startup, "#valid0", true);
	string valid1str = wft_getValueFromExtraParamsFile(startup, "#valid1", true);
	string fillstr = wft_getValueFromExtraParamsFile(startup, "#fill", true);

	string islpw = wft_getValueFromExtraParamsFile(startup, "#islpw", false);

	double valid0 = atof(valid0str.c_str());
	double valid1 = atof(valid1str.c_str());
	double fill = atof(fillstr.c_str());

	string methodstr = wft_getValueFromExtraParamsFile(startup, "#method", true);
	methodstr = methodstr.substr(0, 3);

	int method = METHOD_MAX;
	if (methodstr != "max")
	{
		method = METHOD_AVER;
	}

	bool dohou(false), doday(false) ;
	if (houstr == "1") dohou = true;
	if (daystr == "1") doday = true;


	int hhmm = 0 ;
	int todayYmd0 = wft_current_dateymd_int2(hhmm);
	int todayYmdh = (todayYmd0 % 1000000) * 100 + (hhmm/100) ;
	cout<<"today ymd "<<todayYmd0<<endl ;
	cout<<"current hhmm "<<hhmm ;
	cout<<"current yyMMddhh "<<todayYmdh<<endl ;

	vector<string> allfiles; 
	vector<int> ymdhVec;//yyMMddhh
	wft_get_allSelectedFiles(indir, inprefix, intail, -1, "", allfiles);
	int numfiles = allfiles.size();
	if (numfiles == 0)
	{
		cout << "Warning : no files is need to process." << endl;
		return 0;
	}
	for (int i = 0; i < numfiles; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymdhstr = filename.substr(ymdloc+2, 8);
		int ymdhi = (int)atof(ymdhstr.c_str());
		ymdhVec.push_back(ymdhi);
	}

	GDALAllRegister();

	for (int i = 0; i < numfiles; ++i)
	{
		int ymdhi = ymdhVec[i];
		if (dohou)
		{
			int ymdh0 = ymdhi ;
			int ymdh1 = ymdhi ; 
			if (todayYmdh <= ymdh1+2 ) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdhRange(allfiles, ymdhVec, selectedfiles, ymdh0, ymdh1);
			if (combcnt == 0) continue;

			string filepath = allfiles[i] ;
			string filename = wft_base_name(filepath) ;

			string datetimeStr1 = filename.substr(ymdloc,10) ;
			string outname = makeOutFilename(outprefix, outtail, "hou", datetimeStr1, methodstr );

			string ymd8 = filename.substr(ymdloc,8) ;

			string outdir1 = outhoudir + ymd8 + "/" ;
			mkdir(outdir1.c_str() , 0777) ;
		
			string outpath = outhoudir + ymd8 + "/" + outname;//2017-12-05
			if ( wft_test_file_exists(outpath) == false )
			{
				processOne(selectedfiles, dsprefix, dstail, method, outpath, valid0, valid1);
			}
		}
		if (doday)
		{
			int ymdh0 = (ymdhi/100) * 100  ;//17120500
			int ymdh1 = (ymdhi/100) * 100 + 24 ;//17120524
			if (todayYmdh <= ymdh1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdhRange(allfiles, ymdhVec, selectedfiles, ymdh0, ymdh1);
			if (combcnt == 0) continue;

			string filepath = allfiles[i] ;
			string filename = wft_base_name(filepath) ;

			string datetimeStr1 = filename.substr(ymdloc,8) ;

			if( islpw == "1" )
			{
				string lpwsubname[] = {"tpw_tpw_","lyr_hig_","lyr_mid_","lyr_low_"} ;
				string lpwdstail[] = {"\"://TPW" , "\"://LPW_HIGH" , "\"://LPW_MID" , "\"://LPW_LOW" } ;
				for(int ilyr = 0 ; ilyr < 4 ; ++ ilyr )
				{
					string outname = makeOutFilename(outprefix + lpwsubname[ilyr] 
						, outtail, "day", datetimeStr1, methodstr);
					string outpath = outdaydir + outname;
					if (wft_test_file_exists(outpath) == false)
					{
						processOne(selectedfiles, "HDF5:\"" , lpwdstail[ilyr]
							, method, outpath, valid0, valid1);
					}
				}
			}else {
				string outname = makeOutFilename(outprefix, outtail, "day", datetimeStr1, methodstr);
				string outpath = outdaydir + outname;
				if (wft_test_file_exists(outpath) == false)
				{
					processOne(selectedfiles, dsprefix, dstail, method, outpath, valid0, valid1);
				}
			}

		}
		cout<<i<<"/"<<numfiles<<endl ;
	}


	std::cout << "All done." << std::endl;
	printCurrentTime();
	return 0;
}


