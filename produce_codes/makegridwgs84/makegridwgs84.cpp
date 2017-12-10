// makegridwgs84.cpp : ¶¨Òå¿ØÖÆÌ¨Ó¦ÓÃ³ÌÐòµÄÈë¿Úµã¡£
//ÓÃÓÚ½«µÈ¾­Î³Íø¸ñÊý¾Ý×ª³É´ý×ø±êwgs84µÄtifÎÄ¼þ

#include "gdal_priv.h"
#include "ogr_spatialref.h"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdio>
#include "../sharedcodes/wftools.h"



int main(int argc , char** argv )
{
	std::cout << "A program to copy source dataset and give it a wgs84 coordinate." << std::endl;
	std::cout << "Version 0.1a . by wangfengdev@163.com 2017-10-13." << std::endl;

	std::cout << "processing..." << std::endl;
	if (argc == 1)
	{
		std::cout << "sample call:" << std::endl;
		std::cout << "makegridwgs84 -in HDF5:\"xxxx.nc\"://NDVI -out ndvi.wgs84.tif -left -180 -right 180 -top 90 -bottom -90 " << std::endl;
		std::cout << "no enough parameters. out." << std::endl;
		return 101;
	}


	std::string infile , outfile ;
	double left, right, top, bottom;
	wft_has_param(argc, argv, "-in", infile, true);
	wft_has_param(argc, argv, "-out", outfile, true);
	{
		std::string temp;
		wft_has_param(argc, argv, "-left", temp, true);
		left = atof(temp.c_str());
	}
	{
		std::string temp;
		wft_has_param(argc, argv, "-right", temp, true);
		right = atof(temp.c_str());
	}
	{
		std::string temp;
		wft_has_param(argc, argv, "-top", temp, true);
		top = atof(temp.c_str());
	}
	{
		std::string temp;
		wft_has_param(argc, argv, "-bottom", temp, true);
		bottom = atof(temp.c_str());
	}



	GDALAllRegister();

	//get output parameters.
	GDALDataset* inds = (GDALDataset*)GDALOpen(infile.c_str(), GA_ReadOnly);
	const int inXSize = inds->GetRasterXSize();
	const int inYSize = inds->GetRasterYSize();
	const GDALDataType theDataType = inds->GetRasterBand(1)->GetRasterDataType();

	//create output
	GDALDriver* driver = GetGDALDriverManager()->GetDriverByName("GTiff");
	GDALDataset* outds = driver->Create(outfile.c_str(), inXSize, inYSize, 1, theDataType, 0);

	double trans[6];
	trans[0] = left;
	trans[1] = (right - left) / inXSize;
	trans[2] = 0;
	trans[3] = top;
	trans[4] = 0;
	trans[5] = (bottom - top) / inYSize;

	{
		OGRSpatialReference osrs;
		char* pszSRS_WKT = 0;
		outds->SetGeoTransform(trans);
		osrs.SetWellKnownGeogCS("EPSG:4326");
		osrs.exportToWkt(&pszSRS_WKT);
		outds->SetProjection(pszSRS_WKT);
		CPLFree(pszSRS_WKT);
	}

	double * buffer = new double[inXSize];
	
	for (int iby = 0; iby < inYSize; ++iby)
	{
		inds->GetRasterBand(1)->RasterIO(GF_Read, 0, iby, inXSize, 1,
			buffer, inXSize, 1, GDT_Float64, 0, 0, 0);
		
		outds->GetRasterBand(1)->RasterIO(GF_Write, 0, iby, inXSize, 1,
			buffer, inXSize, 1, GDT_Float64, 0, 0, 0);

		wft_term_progress(iby, inYSize);
	}

	delete[] buffer; buffer = 0;

	GDALClose(inds);
	GDALClose(outds);

	std::cout << "done." << std::endl;
    return 0;
}

