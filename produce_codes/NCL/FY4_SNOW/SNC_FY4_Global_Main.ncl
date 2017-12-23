load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_csm.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/contributed.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/shea_util.ncl"
load "/QHZX_DATA/produce_codes/NCL/FY4_SNOW/SNC_FY4_Global.ncl"

begin

;ncPath="/QHZX_DATA/produce_codes/NCL/FY4_SNOW/2-N_DISK_1047E_L3-_SNC-_MULT_NOM_20170830000000_20170830235959_4000M_V0001.nc"
;pngPath="/QHZX_DATA/produce_codes/NCL/FY4_SNOW/"

baseFunc = SNC_FY4_Global(ncPath,pngPath)

if(baseFunc)then
    print("Plotting is finished!")
end if

end
quit