
#ifndef W_DB_H
#define W_DB_H

#include "/usr/include/mysql/mysql.h"
#include <cstdio>
#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <algorithm>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

using namespace std ;
using std::cout;

class wDb
{
public:

	inline wDb():conn(0){} ;

	void connect( string host , 
		string user , 
		string pwd , 
		string db ) ;

	void insertimg(string tb,string pid,string fpath,string fname,string thumb,int ymd) ;
	void insertimg(string tb,string pid,string fpath,string fname,string thumb,string ymd8str) ;

	void insertxl(string tb,string pid,string maskid,
		string value,string ymd8str);
	void insertxl(string tb,string pid,int maskid,
		double value,int ymd);

	inline ~wDb(){ mysql_close(conn); conn=0; } ;

	string ymd8toy_m_d(string ymd8) ;

private:
	MYSQL* conn ;


} ;

string wDb::ymd8toy_m_d(string ymd8) 
{
	string y = ymd8.substr(0,4) ;
	string m = ymd8.substr(4,2) ;
	string d = ymd8.substr(6,2) ;
	return y+ "-" + m + "-" + d ;
}

void wDb::connect(string host , 
		string user , 
		string pwd , 
		string db ) 
{
	if( conn == 0 )
	{
		conn = mysql_init(0) ;
	    if( mysql_real_connect(conn , 
	                            host.c_str() , 
	                            user.c_str() , 
	                            pwd.c_str() , 
	                            db.c_str() , 
	                            0 , 0 , 0 )
	                    == NULL )
	    { 
	    	cout<<"Error: mysql failed to connect."<<endl ;
	    }
	}
}



void wDb::insertimg(string tb,string pid,string fpath,string fname,string thumb,int ymd) 
{
	char buff[16] ;
	sprintf(buff , "%d" , ymd ) ;
	string ymdstr(buff) ;
	this->insertimg(tb,pid,fpath,fname,thumb,ymdstr) ;
}
void wDb::insertimg(string tb,string pid,string fpath,string fname,string thumb,string ymd8str) 
{
	if( conn == 0 )
	{
		cout<<"Error : no connection."<<endl ;
		return ;
	}

	string ymdstr = this->ymd8toy_m_d(ymd8str) ;

	bool hasone = false ;
	{
		string sql = "SELECT count(*) FROM  " + tb  ;
		sql += " WHERE " ;
		sql += " product_id='" + pid + "' " ;
		sql += " AND start_date='" + ymdstr + "' " ;

		if( mysql_query(conn , sql.c_str() ) )
		{
			cout<<"Error : select count query failed."<<endl ;
		    return ;
		} 
		MYSQL_RES * res = mysql_store_result(conn) ;
		MYSQL_ROW row = mysql_fetch_row(res) ;
		int count = atof(row[0]) ;
		mysql_free_result(res) ;
		res = NULL ;
		if( count > 0 ) hasone = true ;
	}
	
	if( hasone == false )
	{
        string sql = string("insert into ")+tb+" (product_id,data_filepath,filename,start_date,end_date,thumb,big_thumb) values ("  ;
        sql += pid + ","
                + "'" +  fpath  + "',"
                + "'" +  fname  + "',"
                + "'" +  ymdstr  + "',"
                + "'" +  ymdstr  + "',"
                + "'" +  thumb  + "',"
                + "'' " 
                + " )" ;
        mysql_query(conn , sql.c_str()) ;
        int id = mysql_insert_id(conn) ;
        if( id > 0 )
        {
        	cout<<"insert ok:"<<id<<" , "
			<<pid<<","<<fname<<","<<thumb<<","<<ymdstr<<endl;
        }else 
        {
        	cout<<"insert failed:"
			<<sql<<endl ; 
        }
    }
    else 
	{
		cout<<"Already in db:"
			<<pid<<","<<fname<<","<<thumb<<","<<ymdstr<<endl;
	}
}


void wDb::insertxl(string tb,string pid,int maskid,
	double value,int ymd)
{

	char buf[16] ;
	sprintf(buf , "%d" , maskid ) ;
	string maskidstr(buf) ;

	char buff[16] ;
	sprintf(buff , "%d" , ymd ) ;
	string ymdstr(buff) ;

	char buff2[20] ;
	sprintf(buff2 , "%lf" , value ) ;
	string valstr(buff2) ;

	this->insertxl(tb,pid,maskidstr,valstr,ymdstr) ;
}


void wDb::insertxl(string tb,string pid,string maskid,
	string value,string ymd8str)
{
	if( conn == 0 )
	{
		cout<<"Error : no connection."<<endl ;
		return ;
	}

	string ymdstr = this->ymd8toy_m_d(ymd8str) ;

	bool hasone = false ;
	{
		string sql = "SELECT count(*) FROM  " + tb  ;
		sql += " WHERE " ;
		sql += " product_id='" + pid + "' " ;
		sql += " AND mask='" + maskid + "' " ;
		sql += " AND value='" + value + "' " ;
		sql += " AND start_date='" + ymdstr + "' " ;

		if( mysql_query(conn , sql.c_str() ) )
		{
			cout<<"Error : select count query failed."<<endl ;
		    return ;
		} 
		MYSQL_RES * res = mysql_store_result(conn) ;
		MYSQL_ROW row = mysql_fetch_row(res) ;
		int count = atof(row[0]) ;
		mysql_free_result(res) ;
		res = NULL ;
		if( count > 0 ) hasone = true ;
	}
	
	if( hasone == false )
	{
        string sql = string("insert into ")+tb+" (product_id,mask,value,start_date) values ("  ;
        sql += pid + ","
                +  maskid  + ","
                +  value   + ","   
                + "'" + ymdstr + "'" 
                + " )" ;
        mysql_query(conn , sql.c_str()) ;
        int id = mysql_insert_id(conn) ;
        if( id > 0 )
        {
        	cout<<"insert ok:"<<id<<" , "
			<<pid<<","<<maskid<<","<<value<<","<<ymdstr<<endl;
        }else 
        {
        	cout<<"insert failed:"<<
			sql<<endl ;
        }
    }
    else 
	{
		cout<<"Already in db:"
			<<pid<<","<<maskid<<","<<value<<","<<ymdstr<<endl;
	}
}


#endif