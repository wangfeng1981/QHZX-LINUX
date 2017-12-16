// image_combination_monitor.cpp : 定义控制台应用程序的入口点。
//


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

string makeOutFilename(string outprefix, string outtail, string timestr , int ymdstart, string methodstr )
{
	//noaa-ndvi-20170101.c030.max.mon.tif
	string outname = outprefix + wft_int2str(ymdstart) + "." + methodstr + "." + timestr + outtail;
	return outname;
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


int processOne( vector<string>& selfiles , string dsprefix , string dstail , int method , string outpath , double valid0 , double valid1 )
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
			// dstemp->GetRasterBand(1)->RasterIO(GF_Read, 0, y0Index, rasterXSize, useLines,
			//	dataBuffer, rasterXSize, bufferLines, GDT_Float64, 0, 0, 0); bug 2017-12-09
			dstemp->GetRasterBand(1)->RasterIO(GF_Read, 0, y0Index, rasterXSize, useLines,
				dataBuffer, rasterXSize, useLines, GDT_Float64, 0, 0, 0); //bugfixed 2017-12-09 !! useLines replace bufferLines.
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

	makeCinfofile(outpath, (int)selfiles.size() );
	return 100;
}



int main(int argc, char** argv)
{
	printCurrentTime();
	std::cout << "Description: A program to auto combine image into 5days(fiv),10days(ten),month(mon),season(sea),year(yea). " << std::endl;
	std::cout << "Version 0.1a . by wangfengdev@163.com 2017-11-29." << std::endl;
	std::cout << "Version 0.1.1a . bugfixed ." << std::endl;
	std::cout << "Version 0.1.3a . bugfixed write buffer with useLines ." << std::endl;
	std::cout << "Version 0.2a . bugfixed useLines and bufferlines 2017-12-09 ." << std::endl;
	if (argc == 1)
	{
		std::cout << "*** sample call: ***" << std::endl;
		std::cout << "image_combination_monitor startup.txt" << endl;
		std::cout << "*** startup.txt example ***" << std::endl;
		cout << "#indir" << endl;
		cout << "D:/avhrr_ndvi/combinations/qcdaily/" << endl;
		cout << "#outdir" << endl;
		cout << "D:/avhrr_ndvi/combinations/comb-level3/" << endl;
		cout << "#fnfixprefix" << endl;
		cout << "noaa-avhrr-ndvi-avh13c1-" << endl;
		cout << "#fnfixtail" << endl;
		cout << "-day.viqa.tif" << endl;
		cout << "#ymdloc" << endl;
		cout << "24" << endl;
		cout << "#outprefix" << endl;
		cout << "noaa-avhrr-ndiv-avh13c1-" << endl;
		cout << "#outtail" << endl;
		cout << ".tif" << endl;
		cout << "#dsprefix" << endl;
		cout << "" << endl;
		cout << "#dstail" << endl;
		cout << "" << endl;
		cout << "#fiv" << endl;
		cout << "1" << endl;
		cout << "#ten" << endl;
		cout << "1" << endl;
		cout << "#mon" << endl;
		cout << "1" << endl;
		cout << "#sea" << endl;
		cout << "1" << endl;
		cout << "#yea" << endl;
		cout << "1" << endl;
		cout << "#method" << endl;
		cout << "max/aver" << endl;
		cout << "#valid0" << endl;
		cout << "-2000" << endl;
		cout << "#valid1" << endl;
		cout << "10000" << endl;
		cout << "#fill" << endl;
		cout << "-9999" << endl;
		cout << "" << endl;
		cout << "" << endl;
		return 101;
	}

	std::cout << "processing..." << std::endl;
	string startup = argv[1];

	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	string outdir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);
	string inprefix = wft_getValueFromExtraParamsFile(startup, "#fnfixprefix", true);
	string intail = wft_getValueFromExtraParamsFile(startup, "#fnfixtail", true);

	string ymdlocstr = wft_getValueFromExtraParamsFile(startup, "#ymdloc", true);
	int ymdloc = (int)atof(ymdlocstr.c_str());
	string outprefix = wft_getValueFromExtraParamsFile(startup, "#outprefix", true);
	string outtail = wft_getValueFromExtraParamsFile(startup, "#outtail", true);

	string dsprefix = wft_getValueFromExtraParamsFile(startup, "#dsprefix", true);
	string dstail = wft_getValueFromExtraParamsFile(startup, "#dstail", true);

	string fivstr = wft_getValueFromExtraParamsFile(startup, "#fiv", true);
	string tenstr = wft_getValueFromExtraParamsFile(startup, "#ten", true);
	string monstr = wft_getValueFromExtraParamsFile(startup, "#mon", true);
	string seastr = wft_getValueFromExtraParamsFile(startup, "#sea", true);
	string yeastr = wft_getValueFromExtraParamsFile(startup, "#yea", true);

	string valid0str = wft_getValueFromExtraParamsFile(startup, "#valid0", true);
	string valid1str = wft_getValueFromExtraParamsFile(startup, "#valid1", true);
	string fillstr = wft_getValueFromExtraParamsFile(startup, "#fill", true);

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

	bool dofiv(false), doten(false), domon(false), dosea(false), doyea(false);
	if (fivstr == "1") dofiv = true;
	if (tenstr == "1") doten = true;
	if (monstr == "1") domon = true;
	if (seastr == "1") dosea = true;
	if (yeastr == "1") doyea = true;

	int todayYmd = wft_current_dateymd_int();

	vector<string> allfiles;
	vector<int> ymdVec;
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
		string ymdstr = filename.substr(ymdloc, 8);
		int ymdi = (int)atof(ymdstr.c_str());
		ymdVec.push_back(ymdi);
	}

	GDALAllRegister();

	for (int i = 0; i < numfiles; ++i)
	{
		int ymdi = ymdVec[i];
		if (dofiv)
		{
			int ymd0, ymd1;
			getfivdaysStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			string outname = makeOutFilename(outprefix, outtail, "fiv", ymd0, methodstr );
			string outpath = outdir + outname;
			if ( wft_test_file_exists(outpath) == false )
			{
				processOne(selectedfiles, dsprefix, dstail, method, outpath, valid0, valid1);
			}
		}
		if (doten)
		{
			int ymd0, ymd1;
			gettendaysStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			string outname = makeOutFilename(outprefix, outtail, "ten", ymd0, methodstr);
			string outpath = outdir + outname;
			if (wft_test_file_exists(outpath) == false)
			{
				processOne(selectedfiles, dsprefix, dstail, method, outpath, valid0, valid1);
			}
		}
		if (domon)
		{
			int ymd0, ymd1;
			getmonStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			string outname = makeOutFilename(outprefix, outtail, "mon", ymd0, methodstr);
			string outpath = outdir + outname;
			if (wft_test_file_exists(outpath) == false)
			{
				processOne(selectedfiles, dsprefix, dstail, method, outpath, valid0, valid1);
			}
		}
		if (dosea)
		{
			int ymd0, ymd1;
			getseaStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			string outname = makeOutFilename(outprefix, outtail, "sea", ymd0, methodstr);
			string outpath = outdir + outname;
			if (wft_test_file_exists(outpath) == false)
			{
				processOne(selectedfiles, dsprefix, dstail, method, outpath, valid0, valid1);
			}
		}
		if (doyea)
		{
			int ymd0, ymd1;
			getyeaStartEnd(ymdi, ymd0, ymd1);
			if (todayYmd <= ymd1) continue;
			vector<string> selectedfiles;
			int combcnt = selectedFilesByYmdRange(allfiles, ymdVec, selectedfiles, ymd0, ymd1);
			if (combcnt == 0) continue;
			string outname = makeOutFilename(outprefix, outtail, "yea", ymd0, methodstr);
			string outpath = outdir + outname;
			if (wft_test_file_exists(outpath) == false)
			{
				processOne(selectedfiles, dsprefix, dstail, method, outpath, valid0, valid1);
			}
		}
	}


	std::cout << "All done." << std::endl;
	printCurrentTime();
	return 0;
}

