wget https://www.gson.org/egypt/download/egypt-1.11.tar.gz .
tar xzf egypt-1.11.tar.gz
rm egypt-1.11.tar.gz
cd egypt-1.11
perl Makefile.PL
make
sudo make install
cd -