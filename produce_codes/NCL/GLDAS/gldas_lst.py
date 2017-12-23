# coding:utf-8
import sys
import datetime
import os
import commands
import re


#根据各自业务判断是否需要处理,跟输出目录比较是否处理过也可在此处进行
def is_need_process(file_path):
    is_need_process = False
    if file_path.endswith(".tif"):
        is_need_process = True

    return is_need_process

#获取需要处理的文件
def get_files(input_path):
    in_file_list = os.listdir(input_path)
    file_list = []
    for i in range(len(in_file_list)-1,-1,-1):
        file_path = os.path.join(input_path, in_file_list[i])  
        if os.path.isfile(file_path) and is_need_process(file_path):
            file_list.append(file_path)
    
    return file_list

#绘图
def draw_PNG(path_ncl, pngPath, tif_file, dsName):
    print "processing.... ",tif_file+"\r"
    pattern = 'gldas_lst_(\d{8}).*(fiv|ten|mon|sea)'
    nc_name_obj = re.search(pattern, tif_file)
    if nc_name_obj == None or len(nc_name_obj.regs) <3:
        return
    else:
        nc_name = nc_name_obj.group()
        date = nc_name_obj.group(1)
        cycle = nc_name_obj.group(2)
    if cycle == "fiv":
        cycle = "five days"
    elif cycle == "ten":
        cycle = "ten days"
    elif cycle == "mon":
        cycle = "one month"
    elif cycle == "sea":
        cycle = "one season"
    ncPath = os.path.join(pngPath,nc_name+".nc")
    
    gdal_str ="gdal_translate -of netCDF " + tif_file + " " + ncPath
    #print "gdal_str:   ", gdal_str
    commands.getstatusoutput(gdal_str)
     
    shell_str = "ncl -Q " + path_ncl +" ncPath='\"" + ncPath + "\"'" +" cycle='\"" + cycle + "\"'"+" date='\"" + date + "\"'" +" pngPath='\""+pngPath+"\"'" 
    #print "shell_str:   ", shell_str
    status,output=commands.getstatusoutput(shell_str)
    
    #删除nc文件
    os.remove(ncPath)
    
if __name__ == "__main__":
    reload(sys)
    sys.setdefaultencoding('utf-8')
    #     input_path = sys.argv[1]
    #     pngPath = sys.argv[2]


    startTime = datetime.datetime.now()
     
    input_path = "/QHZX_DATA/input_product/gldas/level3"
    #input_path = "C:\\Users\\yu\\Desktop\\ncl\\GLDAS\\tif"
    path_ncl_global = "/QHZX_DATA/produce_codes/NCL/GLDAS/LST_GLDAS_Global_Main.ncl"
    path_ncl_asian = "/QHZX_DATA/produce_codes/NCL/GLDAS/LST_GLDAS_Asian_Main.ncl"
    path_ncl_china = "/QHZX_DATA/produce_codes/NCL/GLDAS/LST_GLDAS_China_Main.ncl"
    pngPath = "/QHZX_DATA/product/gldas/"
    dsName = "Band1"
    
    file_list = get_files(input_path)
    for file in file_list:
        #全球
        draw_PNG(path_ncl_global, pngPath, file, dsName)
        #亚洲
        draw_PNG(path_ncl_asian, pngPath, file, dsName)
        #中国
        draw_PNG(path_ncl_china, pngPath, file, dsName)

    
    # 耗时
    spendTime = datetime.datetime.now() - startTime
    print 'running_time', spendTime
