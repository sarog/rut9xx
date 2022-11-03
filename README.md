## RUTOS 7.x Firmware Building and Installation

### Prerequisites

A supported Linux distribution is recommended. Teltonika suggests using Ubuntu 20.04 LTS.

### Building the firmware

1. Install the required dependencies:
```
$ sudo apt update
$ sudo apt install build-essential ccache ecj fastjar file g++ gawk \
gettext git java-propose-classpath jq libelf-dev libffi-dev \
libncurses5-dev libncursesw5-dev libssl1.0-dev libtool \ 
node-gyp nodejs nodejs-dev npm python python2.7-dev python3 \
python3-distutils python3-setuptools rsync subversion swig \
time u-boot-tools unzip wget xsltproc zlib1g-dev 
```

2. Extract the GPL archive to an empty folder:
```
$ mkdir RUT9_R_GPL_00.07.02.7
$ tar -xzf ~/Downloads/RUT9_R_GPL_00.07.02.7.tar.gz -C RUT9_R_GPL_00.07.02.7
```

3. Copy the default `.config` file by Teltonika:
```
$ cp .config.default .config
```

4. Update the package feeds:
```
$ cd RUT9_R_GPL_00.07.02.7
$ ./scripts/feeds update -a
```

5. Build the image: this will take some time. You can append `-jX` after the `make` command to utilize more CPU cores (replace X with the number of cores in your build system).
```
$ make
```
or to utilize 4 cores on a system:
```
$ make -j4
```

After a successful build, the firmware file will be available under `RUT9_R_GPL_00.07.02.7/bin/targets/ath79/generic/tltFws` with a filename similar to `RUT9_R_GPL_00.07.XX.X_WEBUI.bin`.

### Customization

Before customizing the firmware, it's essential to run a stock build to verify if the build system is properly configured.

To customize RUTOS, simply run:
```
$ make menuconfig
```
and then save the custom configuration file as `.config`.

Note that the target device may be limited on storage (flash), so enabling all the bells & whistles may not be feasible.

### Installation

The new firmware can be deployed on a RUT9XX device through the web interface by visiting `Administration -> Firmware` and uploading the file.
