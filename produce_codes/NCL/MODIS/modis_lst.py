# coding:utf-8
import sys
import time,datetime
import os
import commands
import re


#根据各自业务判断是否需要处理,跟输出目录比较是否处理过也可在此处进行
def is_need_process(file_path):
    is_need_process = False
    if file_path.endswith(".hdf"):
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
def draw_PNG(path_ncl, pngPath, ncPath, cycle, dsName):
    print "processing.....  ",ncPath+"\r"
    pattern = 'MOD11C1.A(\d{7}).006'
    nc_name_obj = re.search(pattern, ncPath)
    if nc_name_obj == None or len(nc_name_obj.regs) <2:
        return
    else:
        day_of_year = nc_name_obj.group(1)
      
    tem_date = time.strptime(day_of_year, '%Y%j')
    date = time.strftime('%Y-%m-%d', tem_date)
    
    shell_str = "ncl -Q " + path_ncl +" ncPath='\"" + ncPath + "\"'" +" dsName='\"" + dsName + "\"'" +" cycle='\"" + cycle + "\"'" +" date='\"" + date + "\"'" +" pngPath='\""+pngPath+"\"'"
    status,output=commands.getstatusoutput(shell_str)
    
    
if __name__ == "__main__":
    reload(sys)
    sys.setdefaultencoding('utf-8')
    #     input_path = sys.argv[1]
    #     pngPath = sys.argv[2]


    startTime = datetime.datetime.now()
    print 'start modis lst ',startTime
    input_path = "/QHZX_DATA/input_product/MODIS_LST/"
    #input_path = "C:\\Users\\yu\\Desktop\\ncl\\modis"
    pngPath = "/QHZX_DATA/product/modis/lst/"
    path_ncl_global = "/QHZX_DATA/produce_codes/NCL/MODIS/LST_MODIS_Global_Main.ncl"
    path_ncl_asian = "/QHZX_DATA/produce_codes/NCL/MODIS/LST_MODIS_Asian_Main.ncl"
    path_ncl_china = "/QHZX_DATA/produce_codes/NCL/MODIS/LST_MODIS_China_Main.ncl"
    dsName_day = "LST_Day_CMG"
    dsName_night = "LST_Night_CMG"
    cycle = 'daily'
    file_list = get_files(input_path)
    for file in file_list:
        #全球
        draw_PNG(path_ncl_global, pngPath, file, cycle, dsName_day)
        draw_PNG(path_ncl_global, pngPath, file, cycle, dsName_night)
        #亚洲
        draw_PNG(path_ncl_asian, pngPath, file, cycle, dsName_day)
        draw_PNG(path_ncl_asian, pngPath, file, cycle, dsName_night)
        #中国
        draw_PNG(path_ncl_china, pngPath, file, cycle, dsName_day)
        draw_PNG(path_ncl_china, pngPath, file, cycle, dsName_night)

    
    # 耗时
    spendTime = datetime.datetime.now() - startTime
    print 'running_time', spendTime
