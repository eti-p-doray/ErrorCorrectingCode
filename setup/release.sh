#/bin/sh -f

mkdir release
cd release
wget https://github.com/eti-p-doray/FeCl/archive/master.zip -O FeCl.zip
unzip FeCl.zip
cp -rf ../Matlab/+fec/+bin/ FeCl-master/Matlab/+fec/+bin/
zip -r ../FeCl.zip FeCl-master

cd ..
rm -r release/
