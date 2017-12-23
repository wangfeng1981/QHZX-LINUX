load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_csm.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/contributed.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/shea_util.ncl"
load "/QHZX_DATA/produce_codes/NCL/GLDAS/LST_GLDAS_China.ncl"

begin

;ncPath = "E:/Qihou/gldas/DATA/GLDAS_CLM10SUBP_3H.A2017213.0000.001.2017324131509.pss.nc"
;cycle = "3h"
;date = "2017-08-01"
;pngPath = "E:/Qihou/gldas/OUTPUT/"

baseFunc = LST_GLDAS_China(ncPath,cycle,date,pngPath)

if(baseFunc)then
    print("Plotting is finished!")
end if

end
quit