load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_csm.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/contributed.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/shea_util.ncl"

undef("LST_MODIS_Asian")
function LST_MODIS_Asian(ncPath:string,\        ;所需绘制FY4数据集文件的相对路径
                                            dsName:string,\          ;辅助数据集文件的相对路径
                                            cycle:string,\	 ;产品绘制结果图输出绝对的路径，不包括后缀名称
                                            date:string,\     ;所需绘制数据集的名称，区分大小写
                                            pngPath:string)       ;FY4产品投影方式
                                                             
local ncPath,dsName,cycle,date,pngPath

begin

re = False
;如果数据文件不存在返回False 结果
if(.not. isfilepresent(ncPath))then
    re@log="File in" +ncPath+ "is not exist!"
    print("ERROR: File in" +ncPath+" is not exist!")
    return(re)
end if

strs = str_split(ncPath,"/")
n_title = product(dimsizes(strs))
n_str = str_split(dsName,"_")
dsName_title = n_str(1)
print(dsName_title)
Title_Year=str_get_cols(strs(n_title-1),9,12)
Title_Year=str_get_cols(strs(n_title-1),9,12)
filename=str_get_cols(strs(n_title-1),0,strlen(strs(n_title-1))-5)
;print(filename)

ncFile = addfile(ncPath,"r")
iniDataset = ncFile->$dsName$
dataset = iniDataset*0.02
dataset = where(dataset .eq. 0, -999,dataset)
dataset@_FillValue = -999
printMinMax(dataset,False)
;printVarSummary(dataset)

lat = fspan(90,-89.95,3600)
lat!0 = "lat"
lat@units = "degree_north"
lon = fspan(-180,179.95,7200)
lon!0 = "lon"
lon@units = "degree_east"

dataset!0 = "lat"
dataset!1 = "lon"
dataset&lat = lat
dataset&lon =lon

setvalues NhlGetWorkspaceObjectId()
    "wsMaximumSize" : 1000000000
   end setvalues
;print(lon)

;=================PLOT===============
lonMin = 20
lonMax = 160
latMin = 0
latMax = 70

type = "png"
type@wkWidth = 3200
type@wkHeight = 3200

wks = gsn_open_wks(type,pngPath+filename+"_"+dsName+"_Asian.png")
gsn_define_colormap(wks,"LST")
colors= gsn_retrieve_colormap(wks)

res = True
res@gsnDraw = False
res@gsnFrame = False
res@gsnAddCyclic = False
res@txFuncCode = "~"

; =======define X&Y axis===================
res@pmTickMarkDisplayMode = "Always"
;res@mpCenterLonF = 100
;res@mpCenterLatF=(latMax-latMin)/2
;res@mpLimitMode = "LatLon"
res@mpMinLonF = lonMin
res@mpMaxLonF = lonMax
res@mpMinLatF = latMin
res@mpMaxLatF = latMax

res@tiXAxisString = "Longitude"
res@tiXAxisOffsetYF = -0.01
res@tiXAxisFontHeightF = 0.012
res@tiXAxisFont = 26

res@tiYAxisString = "Latitude"
res@tiYAxisOffsetXF = 0.005
res@tiYAxisFontHeightF = 0.012
res@tiYAxisFont = 26

res@tmXBLabelFontHeightF = 0.01
res@tmYLLabelFontHeightF = 0.01
res@tmXBLabelFont = 26
res@tmYLLabelFont = 26
res@tmXBMinorOn = False
res@tmYLMinorOn = False
res@tmXBMajorLengthF = 0.005
res@tmYLMajorLengthF = 0.005
res@gsnMajorLatSpacing = 20
res@gsnMajorLonSpacing = 20

txtPath = "/QHZX_DATA/produce_codes/NCL/config/LST.txt"
levelValues = asciiread(txtPath,-1,"float")
strPath = "/QHZX_DATA/produce_codes/NCL/config/LSTlbStrings.txt"
lbStrings = asciiread(strPath,-1,"string")

res@cnFillOn = True
res@cnLineLabelsOn = False
res@cnInfoLabelOn = False
res@cnConstFLabelOn = False
res@cnLinesOn = False
res@cnFillMode = "RasterFill"
res@cnLevelSelectionMode = "ExplicitLevels"
res@cnLevels = levelValues
res@cnFillColors = colors(2:,:)


;====res for colorbar===================
res@lbAutoManage = False
res@lbOrientation = "Vertical"
res@lbLabelStrings = lbStrings
res@lbLabelFontHeightF = 0.01
res@lbLabelFont = 26
res@lbLabelOffsetF = 0.035
res@lbLabelJust = "CenterLeft"
res@lbBoxSeparatorLinesOn = False
res@lbBoxLineThicknessF = 3.0
res@lbLeftMarginF = -0.01
res@lbRightMarginF = 0.1
res@lbTopMarginF = 0.01
res@lbBottomMarginF = 0

res@lbLabelStride = 10
;res@mpOceanFillColor = "#01008a"
   
res@tiMainString = "MODIS LST "+dsName_title+" "+date+" "+cycle+" 0.05~S~o~N~ Asian"
;res@tiMainOffsetXF = 0.04
res@tiMainFontHeightF = 0.018;   
res@tiMainFuncCode ="~"
res@tiMainFontThicknessF=2.0
res@tiMainFont=26

plot = gsn_csm_contour_map(wks,dataset,res)

;======add country boundaries==============================
shp_filename1 = "/QHZX_DATA/produce_codes/NCL/config/shp/province2.shp"
shp_filename2 = "/QHZX_DATA/produce_codes/NCL/config/shp/country2.shp"


lnres1 = True
lnres1@gsLineColor = "black"
lnres1@gsLineThicknessF = 1.0
lnres1@gsLineDashPattern = 1
ploy1=gsn_add_shapefile_polylines(wks,plot,shp_filename1,lnres1)

lnres2 = True
lnres2@gsLineColor = "black"
lnres2@gsLineThicknessF =1.0
ploy2=gsn_add_shapefile_polylines(wks,plot,shp_filename2,lnres2)

 ;=========draw text on plot===============
tx1res = True
tx1res@txPerimOn = False
tx1res@txFontHeightF = 0.012
tx1res@txFuncCode = "~"

pngUnit1="~F22~Unit: K"
txid1 = gsn_create_text(wks,pngUnit1,tx1res)

am1res = True
am1res@amParallelPosF = 0.57
am1res@amOrthogonalPosF = -0.525
am1res@amJust = "CenterLeft"
annoid1 = gsn_add_annotation(plot,txid1,am1res)

draw(plot)
frame(wks)

re = True
re@plot = plot
re@wks = wks
delete(wks)
return(re)
end