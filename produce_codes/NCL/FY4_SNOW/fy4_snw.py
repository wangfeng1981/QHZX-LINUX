# coding:utf-8
import sys
import datetime
import os
import commands
import re

#获取需要处理的文件
def get_files(input_path):
    in_file_list = os.listdir(input_path)
    file_list = []
    for i in range(len(in_file_list)-1,-1,-1):
        filepath = os.path.join(input_path, in_file_list[i])  
        if os.path.isfile(filepath):
            file_list.append(filepath)
    
    return file_list

#绘图
def draw_PNG(path_ncl, pngPath, tif_file):
    #tif_file = "/QHZX_DATA/input_product/ronghe-snc/resultFY4A-_AGRI--_N_DISK_1047E_L3-_SNC-_MULT_NOM_20170822000000_20170831235959_4000M_V0001.tif"
    pattern = 'N_DISK.*._V0001'
    nc_name_obj = re.search(pattern, tif_file)
    if nc_name_obj == None:
        return
    else:
        nc_name = nc_name_obj.group()
    ncPath = os.path.join(pngPath,nc_name+".nc")
    gdal_str ="gdal_translate -of netCDF " + tif_file + " " + ncPath
    print "gdal_str:   ", gdal_str
    commands.getstatusoutput(gdal_str)
     
    shell_str = "ncl -Q " + path_ncl +" ncPath='\"" + ncPath + "\"'" +" pngPath='\""+pngPath+"\"'"
    print "shell_str:   ", shell_str
    status,output=commands.getstatusoutput(shell_str)
    
    #删除nc文件
    os.remove(ncPath)
    
if __name__ == "__main__":
    reload(sys)
    sys.setdefaultencoding('utf-8')
    #     input_path = sys.argv[1]
    #     pngPath = sys.argv[2]


    startTime = datetime.datetime.now()
     
    input_path = "/QHZX_DATA/input_product/ronghe-snc/"
    #input_path = "C:\\Users\\yu\\Desktop\\ncl\\FY4\\tif"
    path_ncl = "/QHZX_DATA/produce_codes/NCL/FY4_SNOW/SNC_FY4_Global_Main.ncl"
    pngPath = "/QHZX_DATA/product/fy4/snc/"
    
    file_list = get_files(input_path)
    for file in file_list:
        draw_PNG(path_ncl, pngPath, file)

    # 耗时
    spendTime = datetime.datetime.now() - startTime
    print 'running_time', spendTime
