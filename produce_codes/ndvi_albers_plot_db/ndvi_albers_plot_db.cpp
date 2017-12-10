// ndvi_albers_plot_db.cpp : 定义控制台应用程序的入口点。
//


#include "gdal_priv.h"
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include "../sharedcodes/wftools.h"
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

int processOne(string infilepath, string outfilepath , string ymdstr , string totxt , string gnuplot , string plottem ,
	string valid0 , 
	string valid1 , 
	string scale , 
	string offset,
	string insert , 
	string host , 
	string user , 
	string pwd ,
	string db , 
	string tb ,
	string pid 
	)
{
	if (isGDALAccessable(infilepath) == false)
	{
		cout << "Error : can not open " << infilepath << endl;
		return 10;
	}


	GDALDataset * tempds = (GDALDataset *)GDALOpen(infilepath.c_str(), GA_ReadOnly);
	int axsize = tempds->GetRasterXSize();
	int aysize = tempds->GetRasterYSize();
	GDALClose(tempds);
	int xspace = 1;
	int yspace = 1;
	if (axsize > 2000)
	{
		xspace = axsize / 2000;
	}
	if (aysize > 2000)
	{
		yspace = aysize / 2000;
	}
	string txtfile = outfilepath + ".xyz.txt";
	string cmd3 = totxt + " -in " + infilepath + " -out " + txtfile + " -valid0 " + valid0 
		+ " -valid1 " + valid1
		+ " -scale " + scale
		+ " -offset " + offset
		+ " -xspace " + wft_int2str(xspace)
		+ " -yspace " + wft_int2str(yspace);

	int res3 = system(cmd3.c_str());
	cout << " image2xyz result : " << res3 << endl;
	if (wft_test_file_exists(txtfile) == false)
	{
		cout << "Error : failed to make " + txtfile << endl;
		return 206;
	}

	string ymd2 = wft_convert_ymd2y_m_d(ymdstr);

	vector<string> varVec, repVec;
	varVec.push_back("{{{OUTFILE}}}");
	varVec.push_back("{{{INFILE}}}");
	varVec.push_back("{{{TITLE}}}");
	varVec.push_back("{{{PTIME}}}");

	string pngfile = outfilepath;
	repVec.push_back(pngfile);
	repVec.push_back(txtfile);
	repVec.push_back(ymd2);
	repVec.push_back(wft_current_datetimestr());

	string plotfile = outfilepath + ".plot";
	wft_create_file_by_template_with_replacement(plotfile, plottem, varVec, repVec);
	if (wft_test_file_exists(plotfile) == false)
	{
		cout << "Error : failed to make " + plotfile << endl;
		return 207;
	}

	string cmd4 = gnuplot + " " + plotfile;
	int res4 = system(cmd4.c_str());
	cout << "gnuplot result : " << res4 << endl;

	if (wft_test_file_exists(pngfile) == false)
	{
		cout << "Error : failed to make " + pngfile << endl;
		return 208;
	}
	wft_remove_file(txtfile);


	if (insert != "")
	{
		//insert db.
		string cmd7 = insert + " -host " + host + " -user " + user
			+ " -pwd " + pwd + " -db " + db + " -tb " + tb
			+ " -datapath " + infilepath
			+ " -dtloc 0 "
			+ " -dtlen 0 "
			+ " -thumb " + pngfile
			+ " -pid " + pid
			+ " -startdate " + ymd2
			+ " -enddate " + ymd2;

		cout << cmd7 << endl;
		int res7 = system(cmd7.c_str());
		cout << "insertdb result:" << res7 << endl;
	}
	return 100;
}


int main(int argc, char** argv)
{
	std::cout << "A programe to plot fy3 modis ndvi albers and insert db." << std::endl;
	std::cout << "V1.0 . 2017-11-29." << std::endl;
	if (argc == 1)
	{
		std::cout << "*** Sample call: *** " << std::endl;
		std::cout << " ndvi_albers_plot_db startup.txt " << std::endl;
		cout << "**************** example startup.txt *********************" << endl;
		cout << "#indir" << endl;
		cout << "D:/avhrr_ndvi/" << endl;
		cout << "#outdir" << endl;
		cout << "D:/avhrr_ndvi/" << endl;
		cout << "#ymdloc" << endl;
		cout << "10" << endl;
		cout << "#valid0" << endl;
		cout << "-2000" << endl;
		cout << "#valid1" << endl;
		cout << "10000" << endl;
		cout << "#scale" << endl;
		cout << "0.0001" << endl;
		cout << "#offset" << endl;
		cout << "0" << endl;
		cout << "#infixprefix" << endl;
		cout << "some" << endl;
		cout << "#infixtail" << endl;
		cout << "some" << endl;
		cout << "#totxt" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/image2xyz" << endl;
		cout << "#plottem" << endl;
		cout << "E:/coding/fy4qhzx-project/extras/noaa-ndvi-daily.plot" << endl;
		cout << "#gnuplot" << endl;
		cout << "gnuplot" << endl;
		cout << "#insert" << endl;
		cout << "/QHZX_DATA/produce_codes/insertdb/insertdb" << endl;
		cout << "#host" << endl;
		cout << "localhost" << endl;
		cout << "#user" << endl;
		cout << "htht" << endl;
		cout << "#pwd" << endl;
		cout << "htht123456" << endl;
		cout << "#db" << endl;
		cout << "qhzx_uus" << endl;
		cout << "#tb" << endl;
		cout << "tb_product_data" << endl;
		cout << "#pid" << endl;
		cout << "000" << endl;
		cout << "" << endl;
		cout << "" << endl;
		cout << "**************** **************** *********************" << endl;
		std::cout << "*** No enough parameters, out.***" << std::endl;
		exit(101);
	}

	string startup = argv[1];
	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	string outdir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);
	string ymdlocstr = wft_getValueFromExtraParamsFile(startup, "#ymdloc", true);
	int ymdloc = wft_str2int(ymdlocstr);

	string inprefix = wft_getValueFromExtraParamsFile(startup, "#infixprefix", true);
	string intail = wft_getValueFromExtraParamsFile(startup, "#infixtail", true);

	string totxt = wft_getValueFromExtraParamsFile(startup, "#totxt", true);
	string gnuplot = wft_getValueFromExtraParamsFile(startup, "#gnuplot", true);
	string plottem = wft_getValueFromExtraParamsFile(startup, "#plottem", true);

	string valid0 = wft_getValueFromExtraParamsFile(startup, "#valid0", true);
	string valid1 = wft_getValueFromExtraParamsFile(startup, "#valid1", true);
	string scale = wft_getValueFromExtraParamsFile(startup, "#scale", true);
	string offset = wft_getValueFromExtraParamsFile(startup, "#offset", true);

	string insert = wft_getValueFromExtraParamsFile(startup, "#insert", true);
	string host = wft_getValueFromExtraParamsFile(startup, "#host", true);
	string user = wft_getValueFromExtraParamsFile(startup, "#user", true);
	string pwd = wft_getValueFromExtraParamsFile(startup, "#pwd", true);
	string db = wft_getValueFromExtraParamsFile(startup, "#db", true);
	string tb = wft_getValueFromExtraParamsFile(startup, "#tb", true);
	string pid = wft_getValueFromExtraParamsFile(startup, "#pid", true);


	vector<string> allfiles;
	wft_get_allSelectedFiles(indir, inprefix, intail, -1, "", allfiles);

	GDALAllRegister();

	int numfiles = allfiles.size();
	for (int i = 0; i < numfiles; ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string outname = filename + ".png";
		string outpath = outdir + outname;
		if (wft_test_file_exists(outpath) == false)
		{
			cout << "processing " << outname << " ... " << endl;
			string ymdstr = filename.substr(ymdloc, 8);
			processOne(filepath,
				outpath,
				ymdstr,
				totxt,
				gnuplot,
				plottem,
				valid0,
				valid1,
				scale,
				offset,
				insert,
				host,
				user,
				pwd,
				db,
				tb,
				pid);
		}
	}

    return 0;
}

