for f in /root/ncc-fy4-project/coming_files_pool/* 
do
  echo "processing... $f"

  #define related directory and path
  DIR1=/root/ncc-fy4-project/produce_codes/fy4_l2_sst_qc_procedure/
  OUTDIR1=/root/ncc-fy4-project/qc_output/demo_outputs/
  BNAME=$(basename $f)
  OUTROOT1=$OUTDIR1"$BNAME"
  OUTXML=$OUTROOT1".serverprovide.xml"
  WTPPATH=/root/ncc-fy4-project/wait_to_process/$BNAME".wtp.sh"
 
  #move file from coming pool into product dir
  PDTPATH=/root/ncc-fy4-project/input_product/fy4sst15min/$BNAME
  mv $f $PDTPATH

  #insert into data table
  echo "set names utf8;insert into tb_data(product_id,data_filepath) values(1,'$PDTPATH');">temp.sql
  mysql -h 10.10.31.229 -u htht -phtht123456 qhzx_uus -e "source temp.sql"

  #create sh file
  echo $DIR1"fy4sst_qc_onestep "$PDTPATH" "$DIR1"startup.dat "$OUTXML >  $WTPPATH

  #call the fy4lonlatlut do the calculation.
  sh $WTPPATH

  #save result into tb_qcresult(product_id=3)
  SCAPNG=$OUTROOT1".scatter.png"
  HISTPNG=$OUTROOT1".hist.png"
  HEATPNG=$OUTROOT1".heatmap.png"
  TIF1=$OUTROOT1".biasimg.tif"
  echo "set names utf8;insert into tb_qcresult(product_id,input_product_id,ref_product_id,input_data_id,ref_data_id,result_xml,biasdist_tif_file,dist_img_file,hist_img_file,scat_img_file) values(3,1,6,0,0,'$OUTXML','$TIF1','$HEATPNG','$HISTPNG','$SCAPNT');">temp1.sql
  mysql -h 10.10.31.229 -u htht -phtht123456 qhzx_uus -e "source temp1.sql"

done


#/root/ncc-fy4-project/wait_to_process/some.sh wait to process
