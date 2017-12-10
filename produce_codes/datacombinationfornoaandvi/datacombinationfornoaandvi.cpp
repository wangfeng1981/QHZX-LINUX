// datacombination.cpp : ¶¨Òå¿ØÖÆÌ¨Ó¦ÓÃ³ÌÐòµÄÈë¿Úµã¡£
//Êý¾Ý×éºÏ£¬ÀýÈçÃ¿ÌìµÄ15min·çËÄÊý¾Ý£¬×éºÏ³ÉÒ»ÌìµÄÊý¾Ý¡£×éºÏËã·¨°üÀ¨Æ½¾ù¡¢×îÐ¡Öµ¡¢×î´óÖµ
//wangfeng1@piesat.cn 2017-9-19
//Ôö¼Ó´ÓtxtÎÄ¼þÖÐ»ñÈ¡ÊäÈëÎÄ¼þÁÐ±íµÄ¹¦ÄÜ¡£v0.3a 2017-9-30´ÓÐÂ±àÒë
//v0.4a ÓÃÓÚ´¦Àínoaa-ndviµÄÊý¾ÝºÏ³É£¬Ôö¼ÓQAÊý¾Ý¼¯µÄÅÐ¶Ï¡£µ±QAµÈÓÚ128Ê±£¬ÈÏÎªndviÓÐÐ§¡£ 2017-10-9

#include <iostream>
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include <bitset>


void printCurrentTime() {
	//#include <ctime>
	time_t time0;
	time(&time0);
	std::cout << asctime(localtime(&time0)) << std::endl;
}

//Í¨¹ý±êÇ©»ñÈ¡ÃüÁîÐÐ²ÎÊý
bool wft_has_param(int argc, char** argv, char* key, std::string& value, bool mustWithValue)
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

bool isValidNoaaNdvi(double vmin, double vmax, double inval, short qaval )
{
	short mask = 14;
	short maskout = qaval & mask;
	//std::string qabinary = std::bitset<16>(qaval).to_string();
	// if (inputValidMin <= inval && inval <= inputValidMax && qaval == 128 )//ÏÞÖÆQAÖµÎª128±íÊ¾avhrrÎå¸ö²¨¶Î¶¼ÓÐÐ§ÇÒÆäËûÑÚÄ¤Öµ¶¼Îª0¡£2017-10-9.
	if (vmin <= inval && inval <= vmax && maskout == 0) return true; //ÏÞÖÆQAÖµ 1,2,3 bit ²»ÄÜÎª1.
	else
	{
		return false;
	}
}


int main(int argc , char** argv )
{
	printCurrentTime();
	std::cout << "Description: A program to combine multi-data(NOAA-NDVI) files into one. " << std::endl;
	std::cout << "Version 0.1a . by wangfengdev@163.com 2017-9-19." << std::endl;
	std::cout << "Version 0.3a . by wangfengdev@163.com 2017-9-30." << std::endl;
	std::cout << "Version 0.4a . by wangfengdev@163.com 2017-10-9. only for noaa-ndvi AVH13C1 ." << std::endl;
	std::cout << "Version 0.4.1a . by wangfengdev@163.com 2017-10-9. add QA filter." << std::endl;
	std::cout << "Version 0.4.2a . by wangfengdev@163.com 2017-10-9 23:30. add progress output." << std::endl;
	std::cout << "Version 0.4.3a . by wangfengdev@163.com 2017-10-26. use QA or not." << std::endl;
	std::cout << "Version 0.4.4a . by wangfengdev@163.com 2017-10-26. display if use qa when running." << std::endl;
	std::cout << "processing..." << std::endl;
	if (argc == 1 )
	{

		std::cout << "*** sample call: ***" << std::endl;
		std::cout << "1:\t datacombinationfornoaandvi [-method aver/min/max] -vmin -20 -vmax 50 -out outfile.tif [-qa 0/1] "
			" -infiles infile0.nc infile1.nc infile2.nc ... " << std::endl;
		std::cout << "2:\t datacombinationfornoaandvi [-method aver/min/max] -vmin -20 -vmax 50 -out outfile.tif [-qa 0/1] "
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

	bool useQA = true;
	{
		std::string tstr1;
		if (wft_has_param(argc, argv, "-qa", tstr1, false))
		{
			if (tstr1 == "1")
			{
				useQA = true;
			}
			else {
				useQA = false;
			}
		}
	}
	if (useQA)
	{
		std::cout << " Using QA." << std::endl;
	}
	else
	{
		std::cout << " Not using QA." << std::endl;
	}


	std::string inputOutputFilepath;
	wft_has_param(argc, argv, "-out", inputOutputFilepath, true);

	bool hasInputFileList = false;
	std::string filename0;
	std::vector<std::string> inputNdviDatasetPathVector , inputQADatasetPathVector ;
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
			std::string tempInDsPath = "HDF5:" + std::string(argv[iIn]) + "://NDVI" ;
			inputNdviDatasetPathVector.push_back(tempInDsPath);
			std::string tempInDsPath2 = "HDF5:" + std::string(argv[iIn]) + "://QA";
			inputQADatasetPathVector.push_back(tempInDsPath2);
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
					std::string tempInDsPath = "HDF5:" + line + "://NDVI";
					inputNdviDatasetPathVector.push_back(tempInDsPath);
					std::string tempInDsPath2 = "HDF5:" + line + "://QA";
					inputQADatasetPathVector.push_back(tempInDsPath2);

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
	GDALDataset* tempInputNdviDataset = (GDALDataset*)GDALOpen(inputNdviDatasetPathVector[0].c_str(), GA_ReadOnly);
	const int rasterXSize = tempInputNdviDataset->GetRasterXSize();
	const int rasterYSize = tempInputNdviDataset->GetRasterYSize();
	const GDALDataType theDataType = tempInputNdviDataset->GetRasterBand(1)->GetRasterDataType();
	

	//create output
	//GDALDriver* ncDriver = GetGDALDriverManager()->GetDriverByName("netCDF");
	//std::string outputValidCountFilepath = inputOutputFilepath + ".validcnt.nc";
	std::string outputValidCountFilepath = inputOutputFilepath + ".validcnt.tif";
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outputDataset = driver->Create(inputOutputFilepath.c_str(), rasterXSize, rasterYSize, 1, theDataType , 0);
	GDALDataset* outputValidCountDataset = driver->Create(outputValidCountFilepath.c_str(), rasterXSize, rasterYSize, 1, GDT_Int16, 0);
	GDALClose(tempInputNdviDataset);

	const int bufferLines = 100;
	double* dataBufferNdvi = new double[rasterXSize*bufferLines];
	short* dataBufferQA = new short[rasterXSize*bufferLines];
	int*    countBuffer = new int[rasterXSize*bufferLines];
	double* outBuffer = new double[rasterXSize*bufferLines];
	const int bufferSize = bufferLines * rasterXSize;
	
	int numBlocks = (rasterYSize + bufferLines-1) / bufferLines;

	const int dsCount = (int)inputNdviDatasetPathVector.size();
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
			std::cout << ids << "/" << dsCount << " ";
			GDALDataset* dstempNdvi = (GDALDataset*)GDALOpen(inputNdviDatasetPathVector[ids].c_str(), GA_ReadOnly);
			GDALDataset* dstempQA = (GDALDataset*)GDALOpen(inputQADatasetPathVector[ids].c_str(), GA_ReadOnly);
			int nx = dstempNdvi->GetRasterXSize();
			int ny = dstempNdvi->GetRasterYSize();
			if (nx != rasterXSize || ny != rasterYSize) {
				std::cout << "*** Error : " << inputNdviDatasetPathVector[ids] << " raster data size is not equal with others. out." << std::endl;
				GDALClose(dstempNdvi);
				GDALClose(dstempQA);
				GDALClose(outputDataset);
				exit(102);
			}
			dstempNdvi->GetRasterBand(1)->RasterIO(GF_Read, 0, y0Index , rasterXSize, useLines ,
				dataBufferNdvi, rasterXSize, bufferLines , GDT_Float64, 0, 0, 0);
			dstempQA->GetRasterBand(1)->RasterIO(GF_Read, 0, y0Index, rasterXSize, useLines,
				dataBufferQA, rasterXSize, bufferLines, GDT_Int16, 0, 0, 0);
			if (method == 0)
			{//¼ÓºÍºóÃæÈ¡Æ½¾ùÖµ
				for (int iy = 0; iy < useLines; ++iy)
				{
					for (int ix = 0; ix < rasterXSize; ++ix)
					{
						int tindex = iy*rasterXSize + ix;
						double inval = dataBufferNdvi[tindex];
						short qaval = dataBufferQA[tindex];
						if (useQA == false) qaval = 0;//2017-10-26
						if ( isValidNoaaNdvi(inputValidMin, inputValidMax, inval, qaval))//ÏÞÖÆQAÖµ 1,2,3 bit ²»ÄÜÎª1.
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
							outBuffer[tindex] = -9999;//Èç¹ûµÚÒ»´Î·ÃÎÊÊä³öÏñËØÊ±£¬Óöµ½µÄÊäÈëÓ°ÏñÎªÎÞÐ§Öµ£¬ÔòÖ±½ÓÊ¹ÓÃÎÞÐ§Öµ-9999¡£2017-10-9.
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
						double inval = dataBufferNdvi[tindex];
						short qaval = dataBufferQA[tindex];
						if (isValidNoaaNdvi(inputValidMin, inputValidMax, inval, qaval))//ÏÞÖÆQAÖµ 1,2,3 bit ²»ÄÜÎª1.
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
							outBuffer[tindex] = -9999 ;//
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
						double inval = dataBufferNdvi[tindex];
						short qaval = dataBufferQA[tindex];
						if ( isValidNoaaNdvi(inputValidMin,inputValidMax,inval,qaval) )//ÏÞÖÆQAÖµ 1,2,3 bit ²»ÄÜÎª1.
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
							outBuffer[tindex] = -9999;//
						}
					}
				}
			}
			GDALClose(dstempNdvi);
			GDALClose(dstempQA);
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

		std::cout << std::endl;

		//Ð´ÈëÊý¾Ý¼¯
		outputDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, y0Index, rasterXSize, useLines,
			outBuffer, rasterXSize, bufferLines, GDT_Float64, 0, 0, 0);
		outputValidCountDataset->GetRasterBand(1)->RasterIO(GF_Write, 0, y0Index, rasterXSize, useLines,
			countBuffer, rasterXSize, bufferLines, GDT_Int32 , 0, 0, 0);

		std::cout << "Block " << iby <<"/" << numBlocks  << " done." << std::endl;
	}
	
	delete [] dataBufferNdvi; dataBufferNdvi = 0;
	delete[] dataBufferQA; dataBufferQA = 0;
	delete [] countBuffer; countBuffer = 0;
	delete[] outBuffer; outBuffer = 0;

	GDALClose(outputDataset);
	GDALClose(outputValidCountDataset);
	
	if (remove(outputValidCountFilepath.c_str()) != 0)
		std::cout << "Error deleting file " << outputValidCountFilepath << std::endl;
	else
		std::cout << "Successfully deleting file " << outputValidCountFilepath << std::endl;
	
	std::cout << "All done." << std::endl;
	printCurrentTime();
    return 0;
}

