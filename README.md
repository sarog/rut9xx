## Firmware Building And Installation

Note: This repo is based on firmware version 7.02.7.

### Prerequisites

Linux, BSD, OSX or WSL required for compilation. Cygwin will not work due to running on a case-insensitive filesystem.

### Building

1. Install the required packages on your host system:
```
## Example commands for Ubuntu:
$ sudo apt-get update
$ sudo apt install -y build-essential ccache ecj fastjar file g++ gawk gettext git java-propose-classpath libelf-dev libncurses5-dev libncursesw5-dev libssl1.0-dev python python3 unzip wget python3-distutils python3-setuptools rsync subversion swig time libffi-dev libtool xsltproc zlib1g-dev u-boot-tools nodejs nodejs-dev node-gyp npm jq

```

2. Extract the archive to an empty folder:
```
$ mkdir RUT9XX_R_GPL
$ tar -xf RUT9XX_R_GPL_00.XX.YY.tar -C RUT9XX_R_GPL
```
3. Update and install the package feeds:
```
$ cd RUT9XX_R_GPL
$ ./scripts/feeds update -a
```    
4. Configure and build the image:
```
$ make menuconfig
$ make
```

After successful build you will get the image file "RUT9XX_R_GPL_00.XX.YY_WEBUI.bin" in "bin/ar71xx/tltFws/".

### Installation

Update the firmware via the web interface or through the CLI on your device.
