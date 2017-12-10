echo "set names utf8; insert into tb_data(product_id,data_filepath)values(1,'somethingxxx');">temp.sql
mysql -h 10.10.31.229 -u htht -phtht123456 qhzx_uus -e "source temp.sql"
