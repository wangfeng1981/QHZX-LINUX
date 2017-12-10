// datacombination.cpp : ¶¨Òå¿ØÖÆÌ¨Ó¦ÓÃ³ÌÐòµÄÈë¿Úµã¡£
//Êý¾Ý×éºÏ£¬ÀýÈçÃ¿ÌìµÄ15min·çËÄÊý¾Ý£¬×éºÏ³ÉÒ»ÌìµÄÊý¾Ý¡£×éºÏËã·¨°üÀ¨Æ½¾ù¡¢×îÐ¡Öµ¡¢×î´óÖµ
//wangfeng1@piesat.cn 2017-9-19
//Ôö¼Ó´ÓtxtÎÄ¼þÖÐ»ñÈ¡ÊäÈëÎÄ¼þÁÐ±íµÄ¹¦ÄÜ¡£v0.3a 2017-9-30´ÓÐÂ±àÒë

#include <iostream>
#include "gdal_priv.h"
#include <ctime>
#include <fstream>



void printCurrentTime() {
	//#include <ctime>
	time_t time0;
	time(&time0);
	std::cout << asctime(localtime(&time0)) << std::endl;
}

//Í¨¹ý±êÇ©»ñÈ¡ÃüÁîÐÐ²ÎÊý
bool wft_has_param(int argc, char** argv,const char* key, std::string& value, bool mustWithValue)
{
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], key) == 0) {
			if (i != argc - 1)
			{
				value = std::string(argv[i + 1]);
				return true;
			}
			else {
				if (mustWithValue) {
					std::cout << "Error: can not find value for key :" << key << ". out." << std::endl;
					exit(99);
				}
				return true;
			}
		}
	}
	if (mustWithValue) {
		std::cout << "Error: can not find value for key :" << key << ". out." << std::endl;
		exit(99);
	}
	return false;
}

//2017-10-26
int wft_remove_file(std::string filepath)
{
	if (remove(filepath.c_str()) != 0)
	{
		std::cout << "Error deleting file:" << filepath << std::endl;
		return 101;
	}
	else
	{
		std::cout << "Successfully deleting file:" << filepath << std::endl;
		return 0;
	}
}


int main(int argc , char** argv )
{
	printCurrentTime();
	std::cout << "processing..." << std::endl;
	if (argc == 1 )
	{
		std::cout << "Description: A program to combine multi-data files into one. " << std::endl;
		std::cout << "Version 0.1a . by wangfengdev@163.com 2017-9-19." << std::endl;
		std::cout << "Version 0.3a . by wangfengdev@163.com 2017-9-30." << std::endl;
		std::cout << "Version 0.3a for linux. 2017-10-20 modified for linux." << std::endl;
		std::cout << "Version 0.3.1a . by wangfengdev@163.com 2017-9-30. remove validcount.tif file after combine." << std::endl;

		std::cout << "*** sample call: ***" << std::endl;
		std::cout << "1:\t datacombination [-method aver/min/max] -vmin -20 -vmax 50 -out outfile.nc [-inprefix HDF:] [-intail :dsname]"
			" -infiles infile0.nc infile1.nc infile2.nc ... " << std::endl;
		std::cout << "2:\t datacombination [-method aver/min/max] -vmin -20 -vmax 50 -out outfile.nc [-inprefix HDF:] [-intail :dsname]"
			" -infilestxt infiles.txt " << std::endl;
		std::cout << "no enough parameters. out." << std::endl;
		return 101;
	}
	
	int method = 0;
	{
		std::string inputMethodStr = "aver";
		{
			wft_has_param(argc, argv, "-method", inputMethodStr, false);
			if (inputMethodStr == "min") {
				method = 1;
			}
			else if (inputMethodStr == "max") {
				method = 2;
			}
		}
	}

	std::string valid0str, valid1str;
	wft_has_param(argc, argv, "-vmin", valid0str, true);
	wft_has_param(argc, argv, "-vmax", valid1str, true);
	double inputValidMin = atof(valid0str.c_str());
	double inputValidMax = atof(valid1str.c_str());

	std::string inputOutputFilepath;
	wft_has_param(argc, argv, "-out", inputOutputFilepath, true);

	std::string inPrefix, inTail;
	wft_has_param(argc, argv, "-inprefix", inPrefix, false);
	wft_has_param(argc, argv, "-intail", inTail, false);

	bool hasInputFileList = false;
	std::string filename0;
	std::vector<std::string> inputDatasetPathVector;
	if (wft_has_param(argc, argv, "-infiles", filename0, false))
	{
		hasInputFileList = true;
		int infilesIndex = -1;
		for (int i = 0; i < argc; ++i) {
			if (strcmp(argv[i], "-infiles") == 0) {
				infilesIndex = i + 1;
				break;
			}
		}
		for (int iIn = infilesIndex; iIn < argc; ++iIn)
		{
			std::string tempInDsPath = inPrefix + std::string(argv[iIn]) + inTail;
			inputDatasetPathVector.push_back(tempInDsPath);
		}
	}

	if (hasInputFileList == false)
	{
		std::string txtfile;
		if (wft_has_param(argc, argv, "-infilestxt", txtfile, false))
		{
			hasInputFileList = true;
			std::ifstream tfs(txtfile.c_str());
			std::string line;
			while (std::getline(tfs, line))
			{
				if (line.length() > 1)
				{
					std::string tempInDsPath = inPrefix + line + inTail;
					inputDatasetPathVector.push_back(tempInDsPath);
				}
			}
			tfs.close();
		}
		else {
			std::cout << "Error: no -infiles or -infilestxt parameters. out." << std::endl;
			exit(105);
		}
	}

	GDALAllRegister();

	//get output parameters.
	GDALDataset* tempInputDataset = (GDALDataset*)GDALOpen(inputDatasetPathVector[0].c_str(), GA_ReadOnly);
	const int rasterXSize = tempInputDataset->GetRasterXSize();
	const int rasterYSize = tempInputDataset->GetRasterYSize();
	const GDALDataType theDataType = tempInputDataset->GetRasterBand(1)->GetRasterDataType();
	

	//create output
	//GDALDriver* ncDriver = GetGDALDriverManager()->GetDriverByName("netCDF");
	//std::string outputValidCountFilepath = inputOutputFilepath + ".validcnt.nc";
	std::string outputValidCountFilepath = inputOutputFilepath + ".validcnt.tif";
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outputDataset = driver->Create(inputOutputFilepath.c_str(), rasterXSize, rasterYSize, 1, theDataType , 0);
	GDALDataset* outputValidCountDataset = driver->Create(outputValidCountFilepath.c_str(), rasterXSize, rasterYSize, 1, GDT_Int16, 0);
	GDALClose(tempInputDataset);

	const int bufferLines = 100;
	double* dataBuffer = new double[rasterXSize*bufferLines];
	int*    countBuffer = new int[rasterXSize*bufferLines];
	double* outBuffer = new double[rasterXSize*bufferLines];
	const int bufferSize = bufferLines * rasterXSize;
	
	int numBlocks = (rasterYSize + bufferLines-1) / bufferLines;

	const int dsCount = (int) inputDatasetPathVector.size();
	for (int iby = 0; iby < numBlocks; ++iby )
	{
		int y0Index = iby * bufferLines;
		int useLines = MIN(bufferLines, rasterYSize - y0Index);

		for (int it = 0; it < bufferSize; ++it)
		{
			outBuffer[it] = 0 ;
			countBuffer[it] = 0;
		}

		for (int ids = 0; ids < dsCount; ++ids)
		{
			GDALDataset* dstemp = (GDALDataset*)GDALOpen(inputDatasetPathVector[ids].c_str(), GA_ReadOnly);
			int nx = dstemp->GetRasterXSize();
			int ny = dstemp->GetRasterYSize();
			if (nx != rasterXSize || ny != rasterYSize) {
				std::cout << "*** Error : " << inputDatasetPathVector[ids] << " raster data size is not equal with others. out." << std::endl;
				GDALClose(dstemp);
				GDALClose(outputDataset);
				exit(102);
			}
			dstemp->GetRasterBand(1)->RasterIO(GF_Read, 0, y0Index , rasterXSize, useLines ,
				dataBuffer, rasterXSize, bufferLines , GDT_Float64, 0, 0, 0);
			if (method == 0)
			{//¼ÓºÍºóÃæÈ¡Æ½¾ùÖµ
				for (int iy = 0; iy < useLines; ++iy)
				{
					for (int ix = 0; ix < rasterXSize; ++ix)
					{
						int tindex = iy*rasterXSize + ix;
						double inval = dataBuffer[tindex];
						if (inputValidMin <= inval && inval <= inputValidMax)
						{
							if (countBuffer[tindex] == 0) {
								outBuffer[tindex] = inval;
							}
							else {
								outBuffer[tindex] += inval;
							}
							++countBuffer[tindex];
						}
						else if( countBuffer[tindex] == 0 ) {
							outBuffer[tindex] = inval;//Èç¹ûµÚÒ»´Î·ÃÎÊÊä³öÏñËØÊ±£¬Óöµ½µÄÊäÈëÓ°ÏñÎªÎÞÐ§Öµ£¬ÔòÖ±½Ó½«Õâ¸öÖµ¿½±´µ½Êä³öÊý¾ÝÖÐ¡£
						}
					}
				}
			}
			else if (method == 1)
			{//È¡×îÐ¡Öµ
				for (int iy = 0; iy < useLines; ++iy)
				{
					for (int ix = 0; ix < rasterXSize; ++ix)
					{
						int tindex = iy*rasterXSize + ix;
						double inval = dataBuffer[tindex];
						if (inputValidMin <= inval && inval <= inputValidMax)
						{
							if (countBuffer[tindex] == 0) {
								outBuffer[tindex] = inval;
							}else if (outBuffer[tindex] > inval)
							{
								outBuffer[tindex] = inval;
							}
							++countBuffer[iy*rasterXSize + ix];
						}
						else if (countBuffer[tindex] == 0) {
							outBuffer[tindex] = inval;//Èç¹ûµÚÒ»´Î·ÃÎÊÊä³öÏñËØÊ±£¬Óöµ½µÄÊäÈëÓ°ÏñÎªÎÞÐ§Öµ£¬ÔòÖ±½Ó½«Õâ¸öÖµ¿½±´µ½Êä³öÊý¾ÝÖÐ¡£
						}
					}
				}
			}
			else if (method == 2)
			{//È¡×î´óÖµ
				for (int iy = 0; iy < useLines; ++iy)
				{
					for (int ix = 0; ix < rasterXSize; ++ix)
					{
						int tindex = iy*rasterXSize + ix;
						double inval = dataBuffer[tindex];
						if (inputValidMin <= inval && inval <= inputValidMax)
						{
							if (countBuffer[tindex] == 0) {
								outBuffer[tindex] = inval;
							}
							else if (outBuffer[tindex] < inval)
							{
								outBuffer[tindex] = inval;
							}
							++countBuffer[iy*rasterXSize + ix];
						}
						else if (countBuffer[tindex] == 0) {
							outBuffer[tindex] = inval;//Èç¹ûµÚÒ»´Î·ÃÎÊÊä³öÏñËØÊ±£¬Óöµ½µÄÊäÈëÓ°ÏñÎªÎÞÐ§Öµ£¬ÔòÖ±½Ó½«Õâ¸öÖµ¿½±´µ½Êä³öÊý¾ÝÖÐ¡£
						}
					}
				}
			}
			GDALClose(dstemp);
		}


		//¼ÆËãÆ½¾ùÖµ
		if (method == 0)
		{
			for (int iy = 0; iy < useLines; ++iy)
			{
				for (int ix = 0; ix < rasterXSize; ++ix)
				{
					int tindex = iy*rasterXSize + ix;
					if (countBuffer[tindex] > 0)
					{
						outBuffer[tindex] = outBuffer[tindex] / countBuffer[tindex] ;
					}
				}
			}
		}

		//Ð´ÈëÊý¾Ý¼¯
		outputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, y0Index, rasterXSize, useLines,
			outBuffer, rasterXSize, bufferLines, GDT_Float64, 0, 0, 0);
		outputValidCountDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, y0Index, rasterXSize, useLines,
			countBuffer, rasterXSize, bufferLines, GDT_Int32 , 0, 0, 0);

		std::cout << "Block " << iby <<"/" << numBlocks  << " done." << std::endl;
	}
	
	delete [] dataBuffer; dataBuffer = 0;
	delete [] countBuffer; countBuffer = 0;
	delete[] outBuffer; outBuffer = 0;

	GDALClose(outputDataset);
	GDALClose(outputValidCountDataset);
	
	//2017-10-26
	wft_remove_file(outputValidCountFilepath);

	std::cout << "All done." << std::endl;
	printCurrentTime();
    return 0;
}

