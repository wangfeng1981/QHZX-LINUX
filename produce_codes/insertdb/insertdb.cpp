#include "/usr/include/mysql/mysql.h"
#include <cstdio>
#include <iostream>
#include <string>
#include "../sharedcodes/wftools.h"  
#include <vector>
#include <cmath>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std ;

string programid = "insertdb" ;
string logdir = "" ;

//get count of some where clause.
//sample: wft_mysql_fetch_count( conn , "tb_product" , "filepath='/root/some.hdf'" );
int wft_mysql_fetch_count( 
                MYSQL* conn  ,
                string table ,  
                string whereContent )
{
        string sql = "SELECT count(*) FROM " ;
        sql += table + " WHERE " ;
        sql += whereContent ;
        if( mysql_query(conn , sql.c_str() ) )
        {
                wft_log( logdir , 
                      programid , 
                     sql + " query failed. out." ) ;
                mysql_close(conn);
                exit(21) ;
        } 
        MYSQL_RES * res = mysql_store_result(conn) ;
        int numf = mysql_num_fields(res) ;
        if( numf != 1 )
        {
                wft_log( logdir , 
                      programid , 
                     sql + " query result field count != 1. out." ) ;
                mysql_close(conn);
                exit(22) ;
        }

        MYSQL_ROW row = mysql_fetch_row(res) ;
        int count = atof(row[0]) ;
        mysql_free_result(res) ;
        res = NULL ;
        return count ;
}

//´Ófy4ÎÄ¼þÃüÖÐ»ñÈ¡Ê±¼ä ½á¹ûÎª2017-01-01 11:11:11
string wft_get_datetimestr_from_fy4filename( string fy4filename )
{
        //size_t pos0 = fy4filename.find("NOM_") ;//modified 2017-10-11
        size_t pos0 = fy4filename.rfind("NOM_") ;
        if( pos0 == string::npos )
        {
                return "" ;
        }else
        {
                string year = fy4filename.substr( pos0+4 , 4 ) ;
                string mon = fy4filename.substr( pos0+8 , 2 ) ;
                string day = fy4filename.substr( pos0+10 , 2 ) ;
                string hh = fy4filename.substr( pos0+12 , 2 ) ;
                string mm = fy4filename.substr( pos0+14 , 2 ) ;
                string ss = fy4filename.substr( pos0+16 , 2 ) ;
                string datetimeStr = year + "-" + mon + "-" + day + " " + hh + ":"+mm+":"+ss ;
                return datetimeStr ;
        }
}

bool isStringDigital( string str )
{
    int zero = (int)'0' ;
    int nine = (int)'9' ;
    for(int i = 0 ; i<str.length() ; ++ i )
    {
        if( str[i] < zero || str[i] > nine )
        {
            return false ;
        }
    }
    return true ;
}

string dtStringByFilename( string filename , int dtloc ,int dtlen )
{
    string year = "1900" ;
    string mon = "01" ;
    string day = "01" ;
    string hh = "00" ;
    string mm = "00" ;
    string ss = "00" ;
    if( dtlen >= 8 && filename.length() > dtloc + 8 )
    {
        year = filename.substr(dtloc , 4) ;
        mon = filename.substr(dtloc+4 , 2) ;
        day = filename.substr(dtloc+6 , 2) ;
    }
    if( dtlen >= 12 && filename.length() > dtloc + 12 )
    {
        hh = filename.substr(dtloc+8 , 2) ;
        if( isStringDigital(hh) == false ) hh = "00" ;
        mm = filename.substr(dtloc+10 , 2) ;
        if( isStringDigital(mm) == false ) mm = "00" ;
    }
    if( dtlen >= 14 && filename.length() > dtloc + 14 )
    {
        ss = filename.substr(dtloc+12 , 2) ;
        if( isStringDigital(ss) == false ) ss = "00" ;
    }
    string datetimeStr = year + "-" + mon + "-" + day + " " + hh + ":"+mm+":"+ss ;
    return datetimeStr ;

}


string make_fy4m15_insertsql( string filepath , string thumbfile , 
     string tablename , 
     string productid ,
     int    dtloc , 
     int    dtlen
     )
{
        string filename = wft_base_name(filepath) ;
        string dtstr = dtStringByFilename(filename,dtloc,dtlen) ;

        //modify 2017-10-11
        string sql = string("insert into ")+ tablename + "(product_id,data_filepath,filename,start_date,end_date,thumb,big_thumb) values ("  ;
        sql += productid + ","
                + "'" + filepath + "'"  + ","
                + "'" + filename + "'"  + ","   
                + "'" + dtstr + "'"  + "," 
                + "'" + dtstr + "'"  + "," 
                + "'" + thumbfile + "'"  + "," 
                + "'" + "" + "'"
                + " )" ;
        //insert into xxx (xx,xx) values(xx,xx) ;
        return sql ;
}

//0.3a
string make_fy4m15_insertsql2( string filepath , string thumbfile , 
     string tablename , 
     string productid ,
     string startdate  , 
     string enddate
     )
{
        string filename = wft_base_name(filepath) ;

        //modify 2017-10-11
        string sql = string("insert into ")+ tablename + "(product_id,data_filepath,filename,start_date,end_date,thumb,big_thumb) values ("  ;
        sql += productid + ","
                + "'" + filepath + "'"  + ","
                + "'" + filename + "'"  + ","   
                + "'" + startdate + "'"  + "," 
                + "'" + enddate + "'"  + "," 
                + "'" + thumbfile + "'"  + "," 
                + "'" + "" + "'"
                + " )" ;
        //insert into xxx (xx,xx) values(xx,xx) ;
        return sql ;
}


bool wft_insert_sql( MYSQL* conn , string sql )
{
        mysql_query(conn , sql.c_str()) ;
        int id = mysql_insert_id(conn) ;
        if( id > 0 ) return true ;
        else return false ;
}



int main(int argc ,char** argv)
{
    std::cout<<"version 0.1a . by wangfengdev@163.com 2017-10-26. "<<std::endl ;
    std::cout<<"version 0.2a . command line params add pid . "<<std::endl ;
    std::cout<<"version 0.3a . input add startdate , enddate. 2017-11-13 . "<<std::endl ;
    std::cout<<"version 1.a . add dtlen condition for startdate , enddate. 2017-11-13 . "<<std::endl ;
    if( argc == 1 )
    {
        cout<<"sample call:"<<endl ;
        cout<<"insertdb "
        " -host localhost "
        " -user somebody "
        " -pwd password "
        " -db dbname "
        " -tb tablename "
        " -datapath datapath.nc/tif "
        " -dtloc 44 "
        " -dtlen 8/12/14 "
        " -thumb thumbfile.png "
        " -pid 10 "
        " [-startdate 20170101] " //0.3a
        " [-enddate 20170101] " //0.3a
        <<endl ;

        cout<<""<<endl ;
        exit(101) ;
    }

    //make log dir.
    {
        string nowYmd = wft_current_dateymd() ;
        string nowYear = nowYmd.substr(0,4) ;
        string testlogdir = string("./log/") + nowYear ;
        struct stat st = {0} ;
        if( stat("./log",&st) == -1 )
        {
            mkdir("./log", 0700) ;
            mkdir(testlogdir.c_str(),0700) ;
            cout<<"make dir "<<testlogdir<<endl ;
        }else if( stat(testlogdir.c_str(),&st) ==-1 ) {
            mkdir(testlogdir.c_str(),0700) ;
            cout<<"make2 dir "<<testlogdir<<endl ;
        }
        logdir = testlogdir + "/" ;
    }

    std::cout<<mysql_get_client_info()<<std::endl ;

    //connect mysql.
    string host ,user , pwd , dbName , tbName ;
    wft_has_param(argc , argv , "-host" , host , true ) ;
    wft_has_param(argc , argv , "-user" , user , true ) ;
    wft_has_param(argc , argv , "-pwd" , pwd , true ) ;
    wft_has_param(argc , argv , "-db" , dbName , true ) ;
    wft_has_param(argc , argv , "-tb" , tbName , true ) ;

    string dataFile , thumbFile , dtLocStr , dtLenStr , pidStr  ;
    wft_has_param(argc , argv , "-datapath" , dataFile , true ) ;
    wft_has_param(argc , argv , "-dtloc" , dtLocStr , true ) ;
    wft_has_param(argc , argv , "-dtlen" , dtLenStr , true ) ;
    wft_has_param(argc , argv , "-thumb" , thumbFile , true ) ;
    wft_has_param(argc , argv , "-pid" , pidStr , true ) ;

    string startdate , enddate ;//0.3a
    int dtloc = (int)atof(dtLocStr.c_str()) ;
    int dtlen = (int)atof(dtLenStr.c_str()) ;
    if( dtloc < 0 || dtlen == 0 ) // add more conditions if( dtloc < 0 ) 2017-12-02
    {
        wft_has_param(argc , argv , "-startdate" , startdate , true ) ;
        wft_has_param(argc , argv , "-enddate" , enddate , true ) ;
    }
    
    MYSQL* conn = mysql_init(NULL) ;
    if( mysql_real_connect(conn , 
                            host.c_str() , 
                            user.c_str() , 
                            pwd.c_str() , 
                            dbName.c_str() , 
                            0 , NULL , 0 )
                    == NULL )
    { 
            wft_log( logdir , 
                  programid , 
                 "error connection failed. out." ) ;
            mysql_close(conn);
            exit(11) ;
    }

    string filename = wft_base_name(dataFile) ;
    string wherestr = "filename='" ;
    wherestr += filename + "'" ;
    if( wft_mysql_fetch_count(
                            conn , 
                            tbName.c_str() , 
                            wherestr
                            )
                    > 0 )
    {
        cout<<"Already in db. no need insert. "<<endl ;
    }else
    {  
        string insertSql;
        if( dtloc >= 0 && dtlen > 0 )//bugfixed. 2017-12-02
        {
            insertSql = make_fy4m15_insertsql(dataFile,
                thumbFile , tbName , pidStr , dtloc , dtlen  );
        }else {
            insertSql = make_fy4m15_insertsql2( dataFile , thumbFile , tbName , pidStr , startdate , enddate ) ;
        }

        if( wft_insert_sql(conn , insertSql) )
        {
                wft_log( logdir , 
                 programid , 
                filename + " inserted." ) ;
                wft_log( logdir , 
                 programid , 
                wft_base_name(thumbFile)
                + " inserted." ) ;
        }else{
                //cout<<insertSql<<endl ;
                wft_log( logdir , 
                 programid , 
                filename + " insert failed." ) ;
                wft_log( logdir , 
                 programid , 
                wft_base_name(thumbFile)
                + " insert failed." ) ;
        }
    }

    mysql_close(conn);
    cout<<"insertdb done."<<endl ;
    return 0 ;
}
