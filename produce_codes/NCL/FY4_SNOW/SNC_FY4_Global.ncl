load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_code.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/gsn_csm.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/contributed.ncl"
load "$NCARG_ROOT/lib/ncarg/nclscripts/csm/shea_util.ncl"



undef("SNC_FY4_Global")
function SNC_FY4_Global(ncPath:string,\        ;�������FY4���ݼ��ļ������·��
                                            pngPath:string)       ;FY4��ƷͶӰ��ʽ                                                            
local ncPath,pngPath

begin

;��������ļ������ڷ���False ���
re = False

strs = str_split(ncPath,"/")
n_title = product(dimsizes(strs))
Year = str_get_cols(strs(n_title-1),31,34)
Month = str_get_cols(strs(n_title-1),35,36)
Day = str_get_cols(strs(n_title-1),37,38)
filename=str_get_cols(strs(n_title-1),0,strlen(strs(n_title-1))-4)
;print(filename)

ncFile  = addfile(ncPath,"r")
iniDataset = ncFile->$"Band1"$
dataset=where(iniDataset .eq. 0,-999,iniDataset)
dataset = dataset(::-1,:)
dataset@_FillValue = -999
printMinMax(dataset,False)

LatLonPath="/QHZX_DATA/produce_codes/NCL/config/FullMask_Grid_4000_1047E_BSQ.nc"
LatLonTable = addfile(LatLonPath,"r")
   
lat2d =LatLonTable->LAT(:,:)
lon2d=LatLonTable->LON(:,:)      

lat2d@_FillValue=-999
lon2d@_FillValue=-999
   
lat2d@units = "degrees_north"
lon2d@units = "degrees_east"

setvalues NhlGetWorkspaceObjectId()
    "wsMaximumSize" : 1000000000
   end setvalues
;print(lon)

;=================PLOT===============

;========��ȡ��ͼ�Ĺ����ռ�
type = "png"
type@wkWidth = 3100
type@wkHeight = 3000

wks=gsn_open_wks(type,pngPath+filename)
gsn_define_colormap(wks,"SNC_QHZX")
colors=gsn_retrieve_colormap(wks)

res = True
res@gsnDraw = False
res@gsnFrame = False
res@gsnAddCyclic = False
res@txFuncCode = "~"

txtPath = "/QHZX_DATA/produce_codes/NCL/config/SNC.txt"
levelValues = asciiread(txtPath,-1,"float")
strPath = "/QHZX_DATA/produce_codes/NCL/config/SNClbStrings.txt"
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

res@mpProjection = "Orthographic"
res@mpPerimOn = False
res@mpCenterLonF = 105
res@mpCenterLatF = 0

res@trGridType = "TriangularMesh"
res@sfXArray = lon2d
res@sfYArray = lat2d

;====res for colorbar===================
res@lbAutoManage = False
res@lbOrientation = "Vertical"
res@lbLabelStrings = lbStrings
res@lbLabelFontHeightF = 0.012
res@lbLabelFont = 26
res@lbLabelOffsetF = 0.035
res@lbLabelJust = "CenterLeft"
res@lbBoxSeparatorLinesOn = False
res@lbBoxLineThicknessF = 3.0
res@lbLeftMarginF = -0.01
res@lbRightMarginF = 0.1
res@lbTopMarginF = 0.07
res@lbBottomMarginF = 0
res@lbLabelAlignment = "BoxCenters"

res@mpOceanFillColor = "white"
res@mpInlandWaterFillColor = "white"
;res@mpLandFillColor = "#b3b3b3"
res@lbBoxCount = dimsizes(levelValues)

res@tiMainString = "FY4 Combined Snow Cover "+Year+"-"+Month+"-"+Day+" Daily 4km"
;res@tiMainOffsetXF = 0.04
res@tiMainFontHeightF = 0.020;   
res@tiMainFuncCode ="~"
res@tiMainFontThicknessF=2.0
res@tiMainFont=26
res@tiMainOffsetYF = 0.03


plot = gsn_csm_contour_map(wks,dataset,res)

;;===================== DRAW country/province Lines======================
shp_province = "/QHZX_DATA/produce_codes/NCL/config/shp/province2.shp"
shp_country = "/QHZX_DATA/produce_codes/NCL/config/shp/country2.shp"

lnres1 = True
lnres1@gsLineColor = "black"
lnres1@gsLineThicknessF = 2.0
lnres1@gsLineDashPattern = 1
ploy1=gsn_add_shapefile_polylines(wks,plot,shp_province,lnres1)
delete(lnres1)

lnres2 = True
lnres2@gsLineColor = "black"
lnres2@gsLineThicknessF =2.0
ploy2=gsn_add_shapefile_polylines(wks,plot,shp_country,lnres2)
delete(lnres2)

draw(plot)
frame(wks)

re = True
re@plot = plot
re@wks = wks
delete(wks)
return(re)
end