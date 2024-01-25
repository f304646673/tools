# Installation

You have more than one choice of install, see below.

## Debian package

### Newer versions

Analizo 1.26.0 (not released yet) and later versions are oficially on Debian then all you need to
install is (as root):

```console
apt install analizo
```

### Versions before 1.26.0

Analizo is readily available as a Debian package. This package might work with Ubuntu 18.04 or upper versions as well. For Ubuntu 16.04 version see this [section](#running-on-ubuntu-1604).
Installing the Debian package has the follwing advantages:

  1. you do not have to compile anything
  2. all dependencies are installed automatically for you
  3. new versions will be automatically available for upgrading the version
     on your system.

You will find the instructions to install Analizo Debian package below. All of
the steps must be performed as =root= user:

1) Create a file /etc/apt/sources.list.d/analizo.list file with the following
contents:

```console
deb http://analizo.org/download/ ./
deb-src http://analizo.org/download/ ./
```

2) Add the repository signing key to your list of trusted keys:

```console
wget -O - http://analizo.org/download/signing-key.asc | apt-key add -
```

3) Update your package lists:

```console
apt-get update
```

4) Install analizo:

```console
apt-get install analizo
```

## From CPAN

Install [cpanminus](https://metacpan.org/pod/App::cpanminus) and building
dependencies (see "Installing dependencies on non-Debian systems" on
HACKING.md) then run `cpanm`:

```console
cpanm Analizo
```

## From sources

Download the analizo tarball linked from
<span class='repository'><a href="http://analizo.org/download.html">the download page</a></span>,
extract it and run the following commands inside the analizo-x.y.z directory:

```console
perl Makefile.PL
make
sudo make install
```

See the HACKING.md file for instructions on how to install Analizo dependencies.

You need to install the dependencies before installing Analizo from sources.

## Running on Ubuntu 16.04

As reported in this [issue](https://github.com/analizo/analizo/issues/149) Analizo __.deb__
package had some problems during installation on Ubuntu Xenial versions. This problem is caused by an incompatible version of Perl. So, to workaround this follow those steps.

1) Install [perlbrew](https://perlbrew.pl/). Perlbrew is a management tool to install diferent versions of Perl without mixing out with your local enviroment. Install and check if the instalation was sucessufull:
```console
sudo apt install perlbrew
perlbrew --version
```

2) Install a newest version of Perl:
```console
perlbrew init
perlbrew install perl-5.26.1
perlbrew switch perl-5.26.1
```

3) This step will change you to an enviroment with the Perl you just installed. Install [cpanminus](https://metacpan.org/pod/App::cpanminus):
```console
cpan App::cpanminus
```

4) It's important before you install Analizo that you have this following dependencies:
```console
sudo apt install libssl-dev libmagic-dev libzmq-dev libexpat1-dev gnuplot git
```

5) Then you can install Analizo
```console
cpanm Analizo
```

## Using Docker

You can run your development environment without installing any dependency, using only the last docker image released.

```bash

docker run --rm \
       -v $LOCAL_REPO_PATH:/home/analizo/ \
       -v $FOLDER_TO_ANALIZE:/src/ \
       analizo/stable:1.22.0 \
       bash -c "cd /src && analizo $1  $2  $3  $4"

```

This first volume will map the your local development Analizo to the path of Analizo inside the container. Making it run your local version instead of the original 1.22.0 code.  
The second volume will map your folder to be analized and put it inside the /src of the container to be analized by the new analizo receiving the $n parameters. If you need more than 4 parameters, some $5 or more should be added.  

To make it easier to use, you can create a bash function configured for your local pc, inside your __~/.bashrc__, as such:

```bash

danalizo(){
path_to_local_repo=/absolute/path/to/dev_analizo/
docker run --rm -v $path_to_local_repo:/home/analizo/ -v $PWD:/src/ analizo/stable:1.22.0 bash -c "cd /src && analizo $1  $2  $3  $4"
}
```
Now you can use your local analizo as:
```bash
danalizo <command> <file or folder>

danalizo metrics goodcode.c

danalizo graph --modules ./myproject/
```
**Warning** : We are using __$PWD__ to copy the files to be analyzed to the container, so you can only analyze files and folders inside your current folder, do NOT use absolute paths or like: "danalizo metrics ../../file.c" or "danalizo graph /home/user/file.java".