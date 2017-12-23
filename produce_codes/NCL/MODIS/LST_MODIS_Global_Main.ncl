load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_csm.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/contributed.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/shea_util.ncl"
load "/QHZX_DATA/produce_codes/NCL/MODIS/LST_MODIS_Global.ncl"

begin

;ncPath = "/QHZX_DATA/input_product/MODIS_LST/MOD11C1.A2017002.006.2017006134848.hdf"
;dsName = "LST_Day_CMG"
;dsName = "LST_Night_CMG"
;cycle = "Daily"
;date = "2017-01-02"
;pngPath = "/QHZX_DATA/produce_codes/NCL/config/output/"

baseFunc = LST_MODIS_Global(ncPath,dsName,cycle,date,pngPath)

if(baseFunc)then
    print("Plotting is finished!")
end if

end
quit