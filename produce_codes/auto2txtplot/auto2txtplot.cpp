// auto2txt.cpp : ¶¨Òå¿ØÖÆÌ¨Ó¦ÓÃ³ÌÐòµÄÈë¿Úµã¡£
//É¨ÃèÌØ¶¨ÎÄ¼þ¼Ð£¬ÕÒµ½·ûºÏÒªÇóµÄÎÄ¼þ£¬ÖÆ×÷llv.tmpÎÄ±¾ÎÄ¼þ¡£
//wangfeng1@piesat.cn 2017-10-20
 

//#define UNICODE
//#include "../../sharedcodes/tinydir.h"
#include <vector>
#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <cmath>
#include "../sharedcodes/wftools.h"
using namespace std; 
using std::cout ;



struct RegionPlot
{
	string code ;
	string tem ;
} ;



//2017-10-26


string g_insert = "/QHZX_DATA/produce_codes/insertdb/insertdb" ;
string g_host = "localhost" ;
string g_user = "htht" ;
string g_pwd = "htht123456" ;
string g_db = "qhzx_uus" ;
string g_tb = "tb_product_data" ;

vector<string> g_pcodeVec ;
vector<string> g_pidVec ;
vector<RegionPlot> g_regionPlotVec ;




string getPid( string pname , string regioncode , string ttype )
{
	
	int num= g_pidVec.size() ;

	string pcode1 = pname + regioncode + ttype ;
	for(int i = 0 ; i<num ; ++ i )
	{
		if( g_pcodeVec[i] == pcode1 )
		{
			return g_pidVec[i] ;
		}
	}
	return "0" ;
}



int doInsertDb(  string datapath , string thumbpath , string ymdstr , string pid )
{

	string cmd = g_insert 
		+ " -host "+ g_host 
		+ " -user "+ g_user 
		+ " -pwd " + g_pwd 
		+ " -db " + g_db
		+ " -tb " + g_tb 
		+ " -datapath " + datapath 
		+ " -dtloc -1 "  
		+ " -dtlen 0 "  
		+ " -pid " + pid 
		+ " -thumb " + thumbpath 
		+ " -startdate \"" + ymdstr + "\"" 
		+ " -enddate \"" + ymdstr + "\"" ;

	cout<<cmd<<endl ;

	int ret = system( cmd.c_str() ) ;
	cout<<"insertdb return code:"<<ret <<endl ;
	return ret ;
}



int makePlotAndInsert(string llvfilepath , string pngfilepath ,string plotTem 
	, string plotProgram 
	, string datestr, string ttypelong , string ttypeshort 
	, string pname , string datapath   )
{
	if (wft_test_file_exists(llvfilepath) == true )
	{
		string ptime = wft_current_datetimestr() ;

		vector<string> vec1;
		vec1.push_back("{{{DATE}}}");
		vec1.push_back("{{{TTYPE}}}");
		vec1.push_back("{{{INFILE}}}");
		vec1.push_back("{{{OUTFILE}}}");
		vec1.push_back("{{{PTIME}}}");

		vector<string> vec2;
		vec2.push_back(datestr);
		vec2.push_back(ttypelong);
		vec2.push_back(llvfilepath);
		vec2.push_back(pngfilepath);
		vec2.push_back(ptime);

		string tempPlotFile = pngfilepath + ".plot";
		wft_create_file_by_template_with_replacement(tempPlotFile, plotTem, vec1, vec2);

		string cmd = plotProgram + " " + tempPlotFile;
		cout << cmd << endl;
		int ret = system(cmd.c_str());
		cout << "gnuplot return code :" << ret << endl;

		if (wft_test_file_exists(pngfilepath))
		{
			//insert db 
			string pid = getPid( pname , "" ,  ttypeshort ) ;
			int retdb = doInsertDb( datapath , pngfilepath , datestr , pid ) ;

			if( g_regionPlotVec.size() > 0 )
			{
				for(int i = 0 ; i<g_regionPlotVec.size() ; ++ i )
				{
					RegionPlot rp1 = g_regionPlotVec[i] ;
					
					string pid1 = getPid( pname , rp1.code ,  ttypeshort ) ;
					cout<<"ploting region for "<<rp1.code<<" , pid " << pid1 << endl ;

					string pngfile1 = pngfilepath + "." + rp1.code + ".png" ;
					string plotfile1 = pngfile1 + ".plot";
					vec2[3] = pngfile1 ;//bugfixed.2017-12-07.
					wft_create_file_by_template_with_replacement(plotfile1, rp1.tem , vec1, vec2);
					string cmd1 = plotProgram + " " + plotfile1;
					cout << cmd1 << endl;
					int ret1 = system(cmd1.c_str());
					cout << "gnuplot return code :" << ret1 << endl;

					if( wft_test_file_exists(pngfile1) )
					{
						int retdb1 = doInsertDb( datapath , pngfile1 , datestr , pid1 ) ;
					}
				}
			}

			wft_remove_file(llvfilepath);
			return 0;
		}
		else
		{
			cout << "Error failed to make png : " << pngfilepath << endl;
			return 104;
		}
	}
	else {
		cout << "Error can not find file : " << llvfilepath << endl;
		return 103;
	}
}





string getTtypeStrByFilename( string filename  , string& typeshort )
{
	int posl2 = filename.find( "_L2-_" ) ;
	string extname = filename.substr( filename.length()-3 , 3) ;

	if( filename.find(".hou.") != string::npos )
	{
		typeshort = "hou" ;
		return "Hourly" ;
	}
	else if( filename.find(".day.") != string::npos )
	{
		typeshort = "day" ;
		return "Daily" ;
	}else if( filename.find(".fiv.") != string::npos )
	{
		typeshort = "fiv" ;
		return "Five-Days" ;
	}else if( filename.find(".ten.") != string::npos )
	{
		typeshort = "ten" ;
		return "Ten-Days" ;
	}else if( filename.find(".mon.") != string::npos )
	{
		typeshort = "mon" ;
		return "Monthly" ;
	}else if( filename.find(".sea.") != string::npos )
	{
		typeshort = "sea" ;
		return "Seasonly" ;
	}else if( filename.find(".yea.") != string::npos )
	{
		typeshort = "yea" ;
		return "Yearly" ;
	}else if( extname == ".NC" && posl2 != string::npos )
	{//realtime data
		typeshort = "min" ;
		return "Near Realtime" ;
	}
	typeshort = "" ;
	return "" ;
}


void loadRegionPlots( string startup , vector<RegionPlot>& regionPlotVec )
{
	int num = 100 ;
	for(int i = 0 ; i<100 ; ++ i )
	{
		string key1 = "#region" + wft_int2str(i) + "-code" ;
		string key2 = "#region" + wft_int2str(i) + "-plottem" ;
		string code = wft_getValueFromExtraParamsFile( startup , key1 , false ) ;
		string tem = wft_getValueFromExtraParamsFile( startup , key2 , false ) ;
		if( code != "" && tem != "" )
		{
			cout<<"region code,tem "<<code<<" , "<<tem <<endl ;
			RegionPlot rp ;
			rp.code = code ;
			rp.tem = tem ;
			regionPlotVec.push_back(rp) ;
		}
	}
}


void loadPcodeAndPid( string configfile , vector<string>& pcodevec, vector<string>& pidvec )
{
	std::ifstream infs(configfile.c_str());
	std::string line;
	while (std::getline(infs, line))
	{
		if ((int)line[line.length() - 1] == 0 || (int)line[line.length() - 1] == 13)
        {
        	line = line.substr(0, line.length() - 1);
        }
		if( line.length()==0 )
		{

		}else {
			if( line[0] =='#' ) continue ;
			pcodevec.push_back(line) ;
			cout<<line<< "-" ;
			std::getline(infs, line) ;
			if ((int)line[line.length() - 1] == 0 || (int)line[line.length() - 1] == 13)
	        {
	        	line = line.substr(0, line.length() - 1);
	        }
			pidvec.push_back(line) ;
			cout<<line<<endl ;
		}
	}
}


void checkVecOk( vector<string>& vec , int n , string tag )
{
	if( vec.size() == n ){
		return ;
	}else {
		cout<<tag<<" vector size " << vec.size() << " is not equal with "<<n<<endl ;
		exit(2) ;
	}
}




int main(int argc , char** argv )
{
	cout << "A program to auto make lon lat value txt file." << endl;
	cout << "Version 0.1a .2017-10-20" << endl;
	cout << "Version 0.1.1a linux. outfile path bugfixed 2017-10-20" << endl;
	cout << "Version 0.2a . accept multi fix tail string separated by ;" << endl;
	cout << "Version 0.3a linux. make llv if no png found, after plot delete llv." << endl;
	cout << "Version 0.4a linux. call insert db after plot." << endl;
	cout << "Version 0.4.1a linux. add default pid." << endl;
	cout << "Version 1.0a do until all finish. 2017-12-06." << endl;
	cout << "Version 1.1a add support for NC and Hourly(hou). 2017-12-07." << endl;
	cout << "Version 1.2a bugfixed for hour and realtime date time put into db. 2017-12-07." << endl;
	cout << "Version 1.3a add support for multi product in one. 2017-12-07 18:30 ." << endl;
	if (argc != 3)
	{
		cout << "sample call: " << endl;
		cout << "auto2txtplot config.txt params.startup" << endl;

		cout << "***************** example params.txt ********************" << endl;

		cout<<"#indir"<<endl;
		cout<<"/dir1/;/dir2/;/dir3/"<<endl;
		cout<<"#outdir"<<endl;
		cout<<"/out1/;/out2/;/out3/"<<endl;
		cout<<"#inprefix"<<endl;
		cout<<"prefix1;prefix2;prefix3"<<endl;
		cout<<"#intail"<<endl;
		cout<<"tail1;tail2;tail3"<<endl;
		cout<<"#dsprefix"<<endl;
		cout<<";;"<<endl;
		cout<<"#dstail"<<endl;
		cout<<";;"<<endl;
		cout<<"#valid0"<<endl;
		cout<<"0;0;0"<<endl;
		cout<<"#valid1"<<endl;
		cout<<"10;10;10"<<endl;
		cout<<"#scale"<<endl;
		cout<<"1;1;1"<<endl;
		cout<<"#offset"<<endl;
		cout<<"0;0;0"<<endl;
		cout<<"#totxt"<<endl;
		cout<<"/QHZX_DATA/produce_codes/fy4totxt/fy4totxt"<<endl;
		cout<<"#gnuplot"<<endl;
		cout<<"/usr/local/bin/gnuplot"<<endl;
		cout<<"#plottem"<<endl;
		cout<<"plottem1;plottem2;plottem3"<<endl;
		cout<<"#lltype"<<endl;
		cout<<"llfiles"<<endl;
		cout<<"#lonfile"<<endl;
		cout<<"/QHZX_DATA/extras/fy4lon.tif"<<endl;
		cout<<"#latfile"<<endl;
		cout<<"/QHZX_DATA/extras/fy4lat.tif"<<endl;
		cout<<"#pname"<<endl;
		cout<<"fy4lpwtpw;fy4lpwlow;fy4lpwmid"<<endl;
		cout<<"#ymdloc"<<endl;
		cout<<"25;25;25"<<endl;
		cout<<"#insert"<<endl;
		cout<<"/QHZX_DATA/produce_codes/insertdb/insertdb"<<endl;
		cout<<"#host"<<endl;
		cout<<"localhost"<<endl;
		cout<<"#user"<<endl;
		cout<<"htht"<<endl;
		cout<<"#pwd"<<endl;
		cout<<"htht123456"<<endl;
		cout<<"#db"<<endl;
		cout<<"qhzx_uus"<<endl;
		cout<<"#tb"<<endl;
		cout<<"tb_product_data"<<endl;
		cout<<""<<endl;
		cout<<"#### MAX Region Num 100 [OPTIONAL] ###"<<endl;
		cout<<"#region1-code"<<endl;
		cout<<"asia"<<endl;
		cout<<"#region1-plottem"<<endl;
		cout<<"fy4olr-asia.plot"<<endl;
		cout<<""<<endl;


		cout << "" << endl; 

		exit(101);
	}
 	string configfile = argv[1] ;
 	loadPcodeAndPid( configfile , g_pcodeVec , g_pidVec ) ;


	string startupFile(argv[2]); 
	string templine ;

	templine = wft_getValueFromExtraParamsFile(startupFile, "#indir", true  ); 
	vector<string> indirvec = wft_string_split2(templine , ";") ;
	templine = wft_getValueFromExtraParamsFile(startupFile, "#outdir", true  ); 
	vector<string> outdirvec = wft_string_split2(templine , ";") ;

	templine = wft_getValueFromExtraParamsFile(startupFile, "#inprefix", true  ); 
	vector<string> inprefixvec = wft_string_split2(templine , ";") ;
	templine = wft_getValueFromExtraParamsFile(startupFile, "#intail", true  );
	vector<string> intailvec = wft_string_split2(templine , ";") ;


	templine = wft_getValueFromExtraParamsFile(startupFile, "#dsprefix", true  ); 
	vector<string> dsprefixvec = wft_string_split2(templine , ";") ;
	templine = wft_getValueFromExtraParamsFile(startupFile, "#dstail", true  );
	vector<string> dstailvec = wft_string_split2(templine , ";") ;

	templine = wft_getValueFromExtraParamsFile(startupFile, "#valid0", true  );
	vector<string> valid0vec = wft_string_split2(templine , ";") ;
	templine = wft_getValueFromExtraParamsFile(startupFile, "#valid1", true  );
	vector<string> valid1vec = wft_string_split2(templine , ";") ;
	templine = wft_getValueFromExtraParamsFile(startupFile, "#scale", true  );
	vector<string> scalevec = wft_string_split2(templine , ";") ;
	templine = wft_getValueFromExtraParamsFile(startupFile, "#offset", true  );
	vector<string> offsetvec = wft_string_split2(templine , ";") ;

	templine = wft_getValueFromExtraParamsFile(startupFile, "#pname", true  );
	vector<string> pnamevec = wft_string_split2(templine , ";") ;
	templine = wft_getValueFromExtraParamsFile(startupFile, "#plottem", true  );
	vector<string> plottemvec = wft_string_split2(templine , ";") ;

	templine = wft_getValueFromExtraParamsFile(startupFile, "#ymdloc"  , true  );
	vector<string> ymdlocvec =  wft_string_split2(templine , ";") ;

	string gnuplot = wft_getValueFromExtraParamsFile(startupFile, "#gnuplot", true  );
	string totxt = wft_getValueFromExtraParamsFile(startupFile, "#totxt", true  );

	string llType = wft_getValueFromExtraParamsFile(startupFile, "#lltype", true  );
	string lonfile = wft_getValueFromExtraParamsFile(startupFile, "#lonfile", true  );
	string latfile = wft_getValueFromExtraParamsFile(startupFile, "#latfile", true  );

	//db
	g_insert = wft_getValueFromExtraParamsFile(startupFile, "#insert", true  ); 
	g_host = wft_getValueFromExtraParamsFile(startupFile, "#host", true  ); 
	g_user = wft_getValueFromExtraParamsFile(startupFile, "#user", true  ); 
	g_pwd = wft_getValueFromExtraParamsFile(startupFile, "#pwd", true  ); 
	g_db = wft_getValueFromExtraParamsFile(startupFile, "#db", true  ); 
	g_tb = wft_getValueFromExtraParamsFile(startupFile, "#tb", true  ); 


	int numOfSets = indirvec.size() ;
	cout<<"number of sets:"<<numOfSets<<endl ;
	checkVecOk(outdirvec , numOfSets , "outdirvec") ;

	checkVecOk(inprefixvec , numOfSets, "inprefixvec") ;
	checkVecOk(intailvec , numOfSets, "intailvec") ;

	checkVecOk(dsprefixvec , numOfSets, "dsprefixvec") ;
	checkVecOk(dstailvec , numOfSets, "dstailvec") ;

	checkVecOk(valid0vec , numOfSets, "valid0vec") ;
	checkVecOk(valid1vec , numOfSets, "valid1vec") ;
	checkVecOk(scalevec , numOfSets, "scalevec") ;
	checkVecOk(offsetvec , numOfSets, "offsetvec") ;

	checkVecOk(plottemvec , numOfSets, "plottemvec") ;
	checkVecOk(pnamevec , numOfSets, "pnamevec") ;

	loadRegionPlots( startupFile , g_regionPlotVec ) ;

	for(int iv = 0 ;iv < indirvec.size() ; ++ iv )
	{
		string indir = indirvec[iv] ; 
		string outdir = outdirvec[iv]  ;
		string inprefix = inprefixvec[iv] ;
		string intail = intailvec[iv] ;
		string dsPrefix = dsprefixvec[iv] ;
		string dsTail = dstailvec[iv] ;

		string valid0str = valid0vec[iv] ;
		string valid1str = valid1vec[iv] ;
		string scalestr =  scalevec[iv] ;
		string offsetstr = offsetvec[iv] ;

		string plottem = plottemvec[iv] ;
		string pname = pnamevec[iv] ;

		int ymdloc = wft_str2int( ymdlocvec[iv] ) ;

		vector<string> allFiles;
		wft_get_allSelectedFiles(indir, inprefix, intail, -1, "", allFiles);
		int allnum = allFiles.size() ;
		for (size_t ifile = 0; ifile < allnum ; ++ifile)
		{
			string filepath = allFiles[ifile] ;
			string filename = wft_base_name(filepath) ;

			string outpngpath = outdir + filename + ".png" ;
			if (  wft_test_file_exists(outpngpath)  == false )
			{
				cout<<"making ("<<ifile<<"/"<<allnum<< ")" <<outpngpath<<endl ;
				string txtfile = outpngpath + ".tmp" ;

				string dsPath = dsPrefix + filepath + dsTail ;
				string cmd = totxt + " -in " + dsPath;
				cmd = cmd + " -out " + txtfile  ;
				cmd = cmd + " -type " + llType ;
				cmd = cmd + " -lon " + lonfile;
				cmd = cmd + " -lat " + latfile;

				cmd = cmd + " -valid0 " + valid0str;
				cmd = cmd + " -valid1 " + valid1str;

				cmd = cmd + " -scale " + scalestr;
				cmd = cmd + " -offset " + offsetstr;

				cout << cmd << endl;
				int ret = system(cmd.c_str());
				cout << "totxt return code :" << ret << endl;

				if( wft_test_file_exists( txtfile ) )
				{
					string tsht = "" ;
					string ttype = getTtypeStrByFilename(filename , tsht ) ;

					string ymdstr = "" ;  
					string ymdstr2 = "" ; 
					if( tsht == "min" )
					{
						ymdstr =  filename.substr( ymdloc , 14 ) ;
						ymdstr2 = wft_convert_ymd2y_m_d_hms(ymdstr) ;
					}else if( tsht == "hou" )
					{
						ymdstr =  filename.substr( ymdloc , 10 ) ;
						ymdstr2 = wft_convert_ymd2y_m_d_hms(ymdstr) ;
					}else {
						ymdstr =  filename.substr( ymdloc , 8 ) ;
						ymdstr2 = wft_convert_ymd2y_m_d(ymdstr) ;
					}
					
					int retplot = makePlotAndInsert(txtfile, outpngpath, plottem, 
						gnuplot , ymdstr2 , ttype
						,tsht , pname ,  filepath );
					
				}else {
					cout<<"Error : failed to make "<<txtfile<<endl ;
				}
			}
			cout<<iv<<"/"<<ifile<<"/"<<allnum<<endl ;
		}
	}

	cout << "done." << endl;

    return 0;
}

