# python-env-sh
Python provides many code libraries for the convenience of developers. However, in the simultaneous development of multiple projects, the versions of the code libraries that different projects depend on may be different. If we maintain these projects in the same environment, the versions of dependent libraries will be confused. In order to solve this problem, we introduce a virtual environment for project isolation.
The script introduced in this article provides the following methods:

 - init: initialize and create the environment. Including installing python-venv and creating a virtual environment (placed in the directory .env folder)
 - del: Delete the virtual environment. If you are currently in a virtual environment, you need to submit the force command again to exit the virtual environment and then delete the virtual environment (.env folder).
 - Input: Enter the virtual environment.
 - quit: Exit the virtual environment.
 - import: Import dependent code libraries from requirements.txt in the current directory.
 - export: Export the code base of the current virtual environment to requirements.txt. If the requirements.txt file exists, you need to submit an additional force parameter to force overwriting.
 - Installation: Install the code base via pip3.
 - Uninstall: Remove the code base via pip3.
 - Help: Provide help information.
   
# init

```bash
source env.sh init
```
## Python-venv is not installed

> python version number: Python3.10
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
Note, selecting 'python3.10-venv' for glob 'Python3.10-venv'
The following NEW packages will be installed:
  python3.10-venv
0 upgraded, 1 newly installed, 0 to remove and 6 not upgraded.
Need to get 0 B/5,716 B of archives.
After this operation, 28.7 kB of additional disk space will be used.
Selecting previously unselected package python3.10-venv.
(Reading database ... 81266 files and directories currently installed.)
Preparing to unpack .../python3.10-venv_3.10.12-1~22.04.3_amd64.deb ...
Unpacking python3.10-venv (3.10.12-1~22.04.3) ...
Setting up python3.10-venv (3.10.12-1~22.04.3) ...
Scanning processes...                                                                                                                                                                            
Scanning candidates...                                                                                                                                                                           
Scanning linux images...                                                                                                                                                                         
Running kernel seems to be up-to-date.
Restarting services...
Service restarts being deferred:
 /etc/needrestart/restart.d/dbus.service
 systemctl restart networkd-dispatcher.service
 systemctl restart systemd-logind.service
 systemctl restart unattended-upgrades.service
 systemctl restart user@1000.service
No containers need to be restarted.
No user sessions are running outdated binaries.
No VM guests are running outdated hypervisor (qemu) binaries on this host.
Create virtual environment
## Python-venv installed
> python version number: Python3.10
Reading package lists... Done
Building dependency tree... Done
Reading state information... Done
Note, selecting 'python3.10-venv' for glob 'Python3.10-venv'
python3.10-venv is already the newest version (3.10.12-1~22.04.3).
0 upgraded, 0 newly installed, 0 to remove and 6 not upgraded.
The virtual environment already exists. If you need to create a new virtual environment, please execute the source env.sh del command to delete the virtual environment.
# del

```bash
source env.sh del
```
## Not entering the virtual environment

> Delete virtual environment

## Entered the virtual environment
### Optional

> Currently in the virtual environment. Failed to delete the virtual environment. if you need to delete the virtual environment, please exit the virtual environment and then execute the source env.sh del command to delete the virtual environment. Or execute the source env.sh del force command to force delete the virtual environment.
### force
```bash
source env.sh del force
```
> Quit the virtual environment
Delete virtual environment

# enter
```bash
source env.sh enter
```
## Not entering the virtual environment

> Enter the virtual environment

## Entered the virtual environment

> Currently in the virtual environment.

# quit

```bash
source env.sh quit
```
## Not entering the virtual environment

> Currently not in the virtual environment.

## Entered the virtual environment

> Quit the virtual environment

# import
```bash
source env.sh import
```
## Not entering the virtual environment

> Currently not in the virtual environment.
## Entered the virtual environment
### requirements.txt file does not exist

> The requirements.txt file does not exist. If you need to import the requirements.txt file, please execute the source env.sh export command to export the requirements.txt file from other virtual environment.

### requirements.txt file already exists
Take the content in requirements.txt as numpy==1.26.3 as an example.
> Collecting numpy==1.26.3
  Using cached numpy-1.26.3-cp310-cp310-manylinux_2_17_x86_64.manylinux2014_x86_64.whl (18.2 MB)
Installing collected packages: numpy
Successfully installed numpy-1.26.3
Import virtual environment

# export
## Not entering the virtual environment

> Currently not in the virtual environment.

## Entered the virtual environment
### requirements.txt file does not exist

> Export virtual environment

### requirements.txt file already exists
#### Optional coverage
> The requirements.txt file already exists. If you need to update the requirements.txt file, remove requirements.txt and then execute the source env.sh export command to update the requirements.txt file.
#### Force coverage

```bash
source env.sh export force
```

> Export virtual environment
# install

```bash
source env.sh install numpy scipy
```
## Not entering the virtual environment

> source env.sh install numpy scipy

## Entered the virtual environment

> Collecting numpy
  Using cached numpy-1.26.3-cp310-cp310-manylinux_2_17_x86_64.manylinux2014_x86_64.whl (18.2 MB)
Collecting scipy
  Using cached scipy-1.11.4-cp310-cp310-manylinux_2_17_x86_64.manylinux2014_x86_64.whl (36.4 MB)
Installing collected packages: numpy, scipy
Successfully installed numpy-1.26.3 scipy-1.11.4
Install libraries numpy scipy

# uninstall
```bash
source env.sh install numpy scipy
```
## Not entering the virtual environment

> Currently not in the virtual environment.

## Entered the virtual environment

> Found existing installation: numpy 1.26.3
Uninstalling numpy-1.26.3:
  Would remove:
    /home/fangliang/numpy-example/.env/bin/f2py
    /home/fangliang/numpy-example/.env/lib/python3.10/site-packages/numpy-1.26.3.dist-info/*
    /home/fangliang/numpy-example/.env/lib/python3.10/site-packages/numpy.libs/libgfortran-040039e1.so.5.0.0
    /home/fangliang/numpy-example/.env/lib/python3.10/site-packages/numpy.libs/libopenblas64_p-r0-0cf96a72.3.23.dev.so
    /home/fangliang/numpy-example/.env/lib/python3.10/site-packages/numpy.libs/libquadmath-96973f99.so.0.0.0
    /home/fangliang/numpy-example/.env/lib/python3.10/site-packages/numpy/*
Proceed (Y/n)? Y
  Successfully uninstalled numpy-1.26.3
Found existing installation: scipy 1.11.4
Uninstalling scipy-1.11.4:
  Would remove:
    /home/fangliang/numpy-example/.env/lib/python3.10/site-packages/scipy-1.11.4.dist-info/*
    /home/fangliang/numpy-example/.env/lib/python3.10/site-packages/scipy.libs/libgfortran-040039e1.so.5.0.0
    /home/fangliang/numpy-example/.env/lib/python3.10/site-packages/scipy.libs/libopenblasp-r0-23e5df77.3.21.dev.so
    /home/fangliang/numpy-example/.env/lib/python3.10/site-packages/scipy.libs/libquadmath-96973f99.so.0.0.0
    /home/fangliang/numpy-example/.env/lib/python3.10/site-packages/scipy/*
Proceed (Y/n)? Y
  Successfully uninstalled scipy-1.11.4
Uninstall libraries numpy scipy

# help

```bash
source env.sh help
```

> init: create virtual environment
enter: enter virtual environment
quit: quit virtual environment
del (force): delete virtual environment. If the force parameter is added, the virtual environment will be deleted forcibly
export (force): export virtual environment. If the force parameter is added, the requirements.txt file will be overwritten
import: import virtual environment
install: install libraries
uninstall : uninstall libraries
help: view help
