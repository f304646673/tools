sudo apt update && sudo apt upgrade -y
sudo apt install perl
sudo apt install doxygen-doxyparse
sudo apt install cpanminus
sudo cpanm File::ShareDir::Install
sudo cpanm FindBin::libs
sudo cpanm App::Cmd::Setup
sudo cpanm Class::Inspector
sudo cpanm Env::Path
sudo cpanm Class::Accessor::Fast
sudo cpanm Graph
sudo cpanm Graph::Writer::Dot
sudo cpanm YAML::XS
sudo cpanm Statistics::Descriptive
sudo cpanm File::HomeDir
sudo cpanm CHI
sudo cpanm File::Copy::Recursive
git clone https://git.launchpad.net/ubuntu/+source/analizo
cd analizo
perl Makefile.PL
make
sudo make install