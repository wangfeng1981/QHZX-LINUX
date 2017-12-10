#include "/usr/include/mysql/mysql.h"
#include <cstdio>
#include <iostream>
#include <string>
#include "wftools.h"
#include <vector>




int main(int argc ,char** argv)
{
  std::cout<<"hello."<<std::endl ;
  std::cout<<mysql_get_client_info()<<std::endl ;
  
  std::vector<std::string> files ;
  wft_linux_get_dir( std::string("/root/ncc-fy4-project/")  , files ) ;

  std::cout<<"number of files:" << files.size() <<std::endl ;
  
  std::string host = "localhost" ;
  std::string user = "htht" ;
  std::stirng   





  return 0 ;
}
