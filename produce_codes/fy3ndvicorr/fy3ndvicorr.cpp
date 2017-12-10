// fy3ndvicorr.cpp : 定义控制台应用程序的入口点。
//根据不同的拟合公式类型fit type position使用对应的线性拟合公式对fy3植被指数数据进行修正。2017-10-16


#include <iostream>
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include "../sharedcodes/wftools.h"
using namespace std;
using std::cout;

void readCorrEquations(string file, std::vector<double>& slopeVector, std::vector<double>& interVector)
{
	cout << "loading fitting slope and inter:" << endl;
	ifstream fs1(file.c_str());
	string line;
	while (std::getline(fs1, line))
	{
		if (line.length() > 1 && line[0] != '#')
		{
			stringstream ss(line);
			double v1, v2;
			ss >> v1 >> v2;
			slopeVector.push_back(v1);
			interVector.push_back(v2);
			cout << v1 << " " << v2 << endl;
		}
	}
	fs1.close();
}


int processOne( string filepath , string outpath , string cepfile , string locfile,
	double valid0 , double valid1 )
{
	vector<double> slopevec, intervec;
	readCorrEquations(cepfile, slopevec, intervec);
	int fitcount = slopevec.size();

	GDALDataset* inDs = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	if (inDs == 0)
	{
		cout << "Error: can not open " << filepath << endl;
		return 10;
	}
	GDALDataset* ftpDs = (GDALDataset*)GDALOpen(locfile.c_str(), GA_ReadOnly);
	if (ftpDs == 0)
	{
		cout << "Error: can not open " << locfile << endl;
		return 20;
	}
	int xSize = inDs->GetRasterXSize();
	int ySize = inDs->GetRasterYSize();
	{
		if (xSize != ftpDs->GetRasterXSize() || ySize != ftpDs->GetRasterYSize())
		{
			std::cout << "Error : sizes are not equal between infile and fit type postion. out." << std::endl;
			GDALClose(inDs);
			GDALClose(ftpDs);
			return 10;
		}
	}
	GDALDataType theDataType = inDs->GetRasterBand(1)->GetRasterDataType();


	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outDs = driver->Create(outpath.c_str(), xSize, ySize, 1, theDataType, 0);
	double trans[6];
	inDs->GetGeoTransform(trans);
	outDs->SetGeoTransform(trans);
	outDs->SetProjection(inDs->GetProjectionRef());

	double* inbuffer = new double[xSize];
	int* ftpbuffer = new int[xSize];
	double* outbuffer = new double[xSize];

	for (int iy = 0; iy < ySize; ++iy)
	{
		inDs->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, xSize, 1,
			inbuffer, xSize, 1, GDT_Float64, 0, 0, 0);
		ftpDs->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, xSize, 1,
			ftpbuffer, xSize, 1, GDT_Int32, 0, 0, 0);
		for (int ix = 0; ix < xSize; ++ix)
		{
			if (inbuffer[ix] >= valid0 && inbuffer[ix] <= valid1)
			{
				int ifit = ftpbuffer[ix];
				double slope = 1.0;
				double inter = 0.0;
				if (ifit >= 0 && ifit < fitcount)
				{
					slope = slopevec[ifit];
					inter = intervec[ifit];
				}
				double newval = inbuffer[ix] * slope + inter;
				outbuffer[ix] = newval;
			}
			else
			{
				outbuffer[ix] = inbuffer[ix];
			}
		}
		outDs->GetRasterBand(1)->RasterIO(GF_Write, 0, iy, xSize, 1,
			outbuffer, xSize, 1, GDT_Float64, 0, 0, 0);
		wft_term_progress(iy, ySize);
	}

	delete[] inbuffer; inbuffer = 0;
	delete[] ftpbuffer; ftpbuffer = 0;
	delete[] outbuffer; outbuffer = 0;

	GDALClose(inDs);
	GDALClose(outDs);
	GDALClose(ftpDs);
	return 100;
}




int main(int argc , char** argv )
{


	std::cout << "Description: A program to do correction for fy3b ndvi. " << std::endl;
	std::cout << "Version 0.1a . wangfeng1@piesat.cn 2017-10-16. " << std::endl;
	std::cout << "Version 0.2a . add valid range -valid0 -valid1. " << std::endl;
	std::cout << "Version 1a . for monitor use. " << std::endl;
	if (argc == 1)
	{

		std::cout << "*** sample call: ***" << std::endl;
		std::cout << "fy3ndvicorr startup.txt" << endl;
		std::cout << "a sample corr EQuation file:" << std::endl;
		std::cout << "#the equation was computed by fy3bndvi(May) vs AVHRRndvi(May) 2017-10-11\n"
			"#slope inter\n"
			"1	0\n"
			"0.7846	0.2745\n"
			"1.1056 -0.0207\n"
			"0.9206 -0.1258\n"
			<< std::endl;
		cout << "************** startup example **************" << endl;
		cout << "#indir" << endl;
		cout << "..." << endl;
		cout << "#infixprefix" << endl;
		cout << "..." << endl;
		cout << "#infixtail" << endl;
		cout << "..." << endl;
		cout << "#monloc" << endl;
		cout << "10" << endl;
		cout << "#fitfile03" << endl;
		cout << "..." << endl;
		cout << "#fitfile06" << endl;
		cout << "..." << endl;
		cout << "#fitfile09" << endl;
		cout << "..." << endl;
		cout << "#fitfile12" << endl;
		cout << "..." << endl;
		cout << "#locfile03" << endl;
		cout << "..." << endl;
		cout << "#locfile06" << endl;
		cout << "..." << endl;
		cout << "#locfile09" << endl;
		cout << "..." << endl;
		cout << "#locfile12" << endl;
		cout << "..." << endl;
		cout << "#valid0" << endl;
		cout << "-2000" << endl;
		cout << "#valid1" << endl;
		cout << "10000" << endl;
		cout << "#outtail" << endl;
		cout << ".corr.tif" << endl;

		std::cout << "no enough parameters. out." << std::endl;
		return 101;
	}

	string startup = argv[1];

	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);

	string inprefix = wft_getValueFromExtraParamsFile(startup, "#infixprefix", true);
	string intail = wft_getValueFromExtraParamsFile(startup, "#infixtail", true);
	int monloc;
	string monlocstr = wft_getValueIntFromExtraParamsFile(startup, "#monloc", monloc, true);

	string fitfile[4], locfile[4];
	fitfile[0] = wft_getValueFromExtraParamsFile(startup, "#fitfile03", true);//3-5
	fitfile[1] = wft_getValueFromExtraParamsFile(startup, "#fitfile06", true);//6-8
	fitfile[2] = wft_getValueFromExtraParamsFile(startup, "#fitfile09", true);//9-11
	fitfile[3] = wft_getValueFromExtraParamsFile(startup, "#fitfile12", true);//12,1,2

	locfile[0] = wft_getValueFromExtraParamsFile(startup, "#locfile03", true);
	locfile[1] = wft_getValueFromExtraParamsFile(startup, "#locfile06", true);
	locfile[2] = wft_getValueFromExtraParamsFile(startup, "#locfile09", true);
	locfile[3] = wft_getValueFromExtraParamsFile(startup, "#locfile12", true);

	double valid0, valid1;
	string valid0str = wft_getValueDoubleFromExtraParamsFile(startup, "#valid0", valid0, true);
	string valid1str = wft_getValueDoubleFromExtraParamsFile(startup, "#valid1", valid1 ,true);
	string outtail = wft_getValueFromExtraParamsFile(startup, "#outtail", true);



	GDALAllRegister();
	
	vector<string> allfiles;
	wft_get_allSelectedFiles(indir, inprefix, intail, -1, "", allfiles);
	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath = allfiles[i];
		string outpath = filepath + outtail;
		if (wft_test_file_exists(outpath) == false )
		{
			string filename = wft_base_name(filepath);
			string monstr = filename.substr(monloc, 2);
			int mon = wft_str2int(monstr);
			int ifit = 0;
			if (mon < 3) ifit = 3;
			else if (mon < 6) ifit = 0;
			else if (mon < 9) ifit = 1;
			else if (mon < 12) ifit = 2;
			else ifit = 3;
			cout << "correcting " << filename << " with ifit : " << ifit << endl;
			processOne(filepath, outpath, fitfile[ifit], locfile[ifit], valid0, valid1);

		}
	}


	cout << "done." << endl;

    return 0;
}




