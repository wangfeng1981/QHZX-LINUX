//image_space_average 计算时间序列
//wangfeng1@piesat.cn 2017-11-07

#include "/usr/include/mysql/mysql.h"
#include <iostream>
#include "gdal_priv.h"
#include <ctime>
#include <fstream>
#include "../sharedcodes/wftools.h"
#include <vector>
#include <string>
using namespace std;
using std::cout;

//整形转字符串 2017-11-29
std::string double2str(double val)
{
	std::stringstream ss;
	ss << val;
	return ss.str();
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


string insert_record_sql(
	string tb,
	string productid,
	int mask,
	double value,
	string ymd
	)
{
	string ymd2 = wft_convert_ymd2y_m_d(ymd);
	string sql = string("insert into ") + tb + "(product_id,mask,value,start_date) values (";
	sql += productid + ","
		+ "'" + wft_int2str(mask) + "'" + ","
		+ "'" + double2str(value) + "'" + ","
		+ "'" + ymd2 + "'"
		+ " )";
	//insert into xxx (xx,xx) values(xx,xx) ;
	return sql;
}

int wft_mysql_fetch_count(
	MYSQL* conn,
	string table,
	string productid , 
	string ymd )
{
	string ymd2 = wft_convert_ymd2y_m_d(ymd);
	string sql = "SELECT count(*) FROM ";
	sql += table + " WHERE ";
	sql = sql + " product_id=" + productid + " AND start_date='" + ymd2 + "' ";
	if (  mysql_query(conn, sql.c_str()) )
	{
		cout << "Error : MySQL query failed." << sql << endl;
		mysql_close(conn);
		return -1;
	}
	MYSQL_RES * res = mysql_store_result(conn);
	int numf = mysql_num_fields(res);
	if (numf != 1)
	{
		cout << "Error : MySQL query result field count != 1." << sql << endl;
		mysql_close(conn);
		return -1;
	}

	MYSQL_ROW row = mysql_fetch_row(res);
	int count = atof(row[0]);
	mysql_free_result(res);
	res = NULL;
	return count;
}


bool wft_insert_sql(MYSQL* conn, string sql)
{
	mysql_query(conn, sql.c_str());
	int id = mysql_insert_id(conn);
	if (id > 0) return true;
	else return false;
}



int processOne(string filepath, string outpath, string ymd, string maskfile, double valid0, double valid1 , 
	string tb , 
	string pid  ,
	MYSQL* conn , 
	double scale , 
	double offset )
{
	if (isGDALAccessable(filepath)==false)
	{
		cout << " Error : can not open " << filepath << endl;
		return 10;
	}

	if ( maskfile != "" && isGDALAccessable(maskfile) == false)
	{
		cout << " Error : can not open " << maskfile << endl;
		return 10;
	}


	GDALDataset* inds = (GDALDataset*)GDALOpen(filepath.c_str(), GA_ReadOnly);
	int xsize = inds->GetRasterXSize();
	int ysize = inds->GetRasterYSize();
	GDALDataset* maskds = 0;
	if (maskfile != "")
	{
		maskds = (GDALDataset*)GDALOpen(maskfile.c_str(), GA_ReadOnly);

		int x1size = maskds->GetRasterXSize();
		int y1size = maskds->GetRasterYSize();

		if (xsize != x1size || ysize != y1size)
		{
			cout << "Error : inds size " << xsize << " " << ysize << " is not equal with mask size " << x1size << " " << y1size << "." << endl;
			GDALClose(inds);
			if (maskds) GDALClose(maskds);
			return 30;
		}
	}
	



	double* buff = new double[xsize];
	if (buff == 0)
	{
		cout << "Error : can not alloc memory." << endl;
		return 40;
	}
	int * maskbuff = new int[xsize];
	if (maskbuff == 0)
	{
		cout << "Error : can not alloc memory." << endl;
		return 50;
	}

	double maskSum[255] ;
	int maskCount[255] ;
	bool masked[255];
	for (int i = 0; i < 255; ++i)
	{
		maskSum[i] = 0;
		maskCount[i] = 0;
		masked[i] = false ;
	}

	if (maskds == 0)
	{
		masked[1] = true ;
		for (int iy = 0; iy < ysize; ++iy)
		{
			inds->GetRasterBand(1)->RasterIO( GF_Read, 0, iy, xsize, 1,
				buff, xsize, 1, GDT_Float64, 0, 0, 0);
			for (int ix = 0; ix < xsize; ++ix)
			{
				if (buff[ix] >= valid0 && buff[ix] <= valid1)
				{
					maskSum[1] += buff[ix];
					++ maskCount[1];
				}
			}
		}
	}
	else
	{
		for (int iy = 0; iy < ysize; ++iy)
		{
			maskds->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, xsize, 1,
				maskbuff, xsize, 1, GDT_Int32, 0, 0, 0);
			inds->GetRasterBand(1)->RasterIO(GF_Read, 0, iy, xsize, 1,
				buff, xsize, 1, GDT_Float64, 0, 0, 0);
			for (int ix = 0; ix < xsize; ++ix)
			{
				if (maskbuff[ix] > 0 && maskbuff[ix] < 255)
				{
					masked[maskbuff[ix]] = true;
					if (buff[ix] >= valid0 && buff[ix] <= valid1)
					{
						maskSum[maskbuff[ix]] += buff[ix];
						++maskCount[maskbuff[ix]] ;
					}

				}
			}
		}
	}

	GDALClose(inds);
	GDALClose(maskds);

	delete[] buff;
	delete[] maskbuff;

	ofstream ofs(outpath.c_str());
	for (int i = 0; i < 255; ++i)
	{
		if (masked[i])
		{
			double aver = -99999.0;
			if (maskCount[i] > 0)
			{
				aver = (maskSum[i] / maskCount[i]) * scale + offset ;
			}
			ofs << i << "," << aver << endl;
			string sql = insert_record_sql( tb , pid ,
										i ,
										aver ,
										ymd ) ;
			bool isok = wft_insert_sql(conn, sql) ;
			cout<<isok << " for " << sql<<endl ;
		}
	}
	ofs.close();
	return 100;
}







int main(int argc , char** argv )
{
	std::cout << "Compute image space average save into txt." << std::endl;
	std::cout << "Version 0.1a . by wangfengdev@163.com 2017-11-30." << std::endl;
	std::cout << "Version 0.2a . add scale and offset 2017-12-02." << std::endl;

	std::cout << "processing..." << std::endl;
	if (argc == 1 )
	{
		
		std::cout << "*** sample call: ***" << std::endl;
		std::cout << "image_space_average startup.txt" << std::endl;

		cout<<"*********** startup ***************"<<endl ;
		cout<<"#indir"<<endl;
		cout<<"/QHZX_DATA/input_product/fy3b-ndvi-comb-level3/"<<endl;
		cout<<"#outdir"<<endl;
		cout<<"/QHZX_DATA/input_product/fy3b-ndvi-comb-level3/"<<endl;
		cout<<"#infixprefix"<<endl;
		cout<<"fy3b_ndvi_aoam."<<endl;
		cout<<"#infixtail"<<endl;
		cout<<".max.sea.comb.albers.tif"<<endl;
		cout<<"#outprefix"<<endl;
		cout<<"fy3b_ndvi_aoam."<<endl;
		cout<<"#outtail"<<endl;
		cout<<".max.sea.comb.xulie.txt"<<endl;
		cout<<"#ymdloc"<<endl;
		cout<<"15"<<endl;
		cout<<"#valid0"<<endl;
		cout<<"-2000"<<endl;
		cout<<"#valid1"<<endl;
		cout<<"10000"<<endl;
		cout<<"#mask"<<endl;
		cout<<""<<endl;
		cout<<"#host"<<endl;
		cout<<"localhost"<<endl;
		cout<<"#user"<<endl;
		cout<<"htht"<<endl;
		cout<<"#pwd"<<endl;
		cout<<"htht123456"<<endl;
		cout<<"#db"<<endl;
		cout<<"qhzx_uus"<<endl;
		cout<<"#tb"<<endl;
		cout<<"tb_xulie_data"<<endl;
		cout<<"#pid"<<endl;
		cout<<"116"<<endl;
		cout<<"#scale"<<endl;//2017-12-2
		cout<<"0.0001"<<endl;
		cout<<"#offset"<<endl;//2017-12-2
		cout<<"0"<<endl;
		cout<<""<<endl;

		std::cout << "no enough parameters. out." << std::endl;
		return 101;
	}
	
	string startup = argv[1];
	string indir = wft_getValueFromExtraParamsFile(startup, "#indir", true);
	string outdir = wft_getValueFromExtraParamsFile(startup, "#outdir", true);
	string fixprefix = wft_getValueFromExtraParamsFile(startup, "#infixprefix", true);
	string fixtail = wft_getValueFromExtraParamsFile(startup, "#infixtail", true);
	string outprefix = wft_getValueFromExtraParamsFile(startup, "#outprefix", true);
	string outtail = wft_getValueFromExtraParamsFile(startup, "#outtail", true);
	string ymdlocstr = wft_getValueFromExtraParamsFile(startup, "#ymdloc", true);

	string valid0str = wft_getValueFromExtraParamsFile(startup, "#valid0", true);
	string valid1str = wft_getValueFromExtraParamsFile(startup, "#valid1", true);
	string maskfile = wft_getValueFromExtraParamsFile(startup, "#mask", true);
	//mask 0为无效值，从1开始进行统计 输出的时候mask值作为一个输出值数据，如果mask为空那么输出的mask值默认为1

	string host = wft_getValueFromExtraParamsFile(startup, "#host", true);
	string user = wft_getValueFromExtraParamsFile(startup, "#user", true);
	string pwd = wft_getValueFromExtraParamsFile(startup, "#pwd", true);
	string db = wft_getValueFromExtraParamsFile(startup, "#db", true);
	string tb = wft_getValueFromExtraParamsFile(startup, "#tb", true);
	string pid = wft_getValueFromExtraParamsFile(startup, "#pid", true);

	double scale , offset ; //2017-12-02
	string scalestr = wft_getValueDoubleFromExtraParamsFile(startup, "#scale", scale , true);
	string offsetstr = wft_getValueDoubleFromExtraParamsFile(startup, "#offset", offset, true);


	MYSQL* conn = mysql_init(NULL) ;
    if( mysql_real_connect(conn , 
                            host.c_str() , 
                            user.c_str() , 
                            pwd.c_str() , 
                            db.c_str() , 
                            0 , NULL , 0 )
                    == NULL )
    { 
            cout<< "Error mysql connection failed. out." <<endl ;
    }


	int ymdloc = wft_str2int(ymdlocstr);
	double valid0 = atof(valid0str.c_str());
	double valid1 = atof(valid1str.c_str());
	
	vector<string> allfiles;
	wft_get_allSelectedFiles(indir, fixprefix, fixtail, -1, "", allfiles);

	int todayYmd = wft_current_dateymd_int();
	cout << "Today " << todayYmd << endl;

	GDALAllRegister();

	for (int i = 0; i < allfiles.size(); ++i)
	{
		string filepath = allfiles[i];
		string filename = wft_base_name(filepath);
		string ymd = filename.substr(ymdloc, 8);
		string outname = outprefix + ymd + outtail;
		string outpath = outdir + outname;

		int isInDb = wft_mysql_fetch_count(  conn, tb ,  pid, ymd ) ;
		if ( (isInDb < 0 && wft_test_file_exists(outpath) == false ) || isInDb==0 )
		{
			cout << "processing " << outname << " ... " << endl;
			processOne(filepath, outpath, ymd, maskfile , valid0, valid1 , 
				 tb , pid , conn , scale , offset );
		}
	}

		


	
	std::cout << "All done." << std::endl;

    return 0;
}




