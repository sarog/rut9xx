## Firmware Building And Installation

Note: This repo is based on firmware version 6.06.1.

### Prerequisites

Linux, BSD, OSX or WSL required for compilation. Cygwin will not work due to running on a case-insensitive filesystem.

### Building

1. Install the required packages on your host system:

Example commands for Ubuntu:

    $ sudo apt-get update
    $ sudo apt-get install -y build-essential curl devscripts gawk gcc-multilib gengetopt gettext git groff file flex \
        libncurses5-dev libssl-dev python2 subversion unzip zlib1g-dev wget

2. Extract the archive to an empty folder:

    $ mkdir RUT9XX_R_GPL
    $ tar -xf RUT9XX_R_GPL_00.XX.YY.tar -C RUT9XX_R_GPL

3. Update and install the package feeds:

    $ cd RUT9XX_R_GPL
    $ ./scripts/feeds update -a
    $ ./scripts/feeds install -a
    
4. Configure and build the image:

    $ make menuconfig
    $ make

After successful build you will get the image file "RUT9XX_R_GPL_00.XX.YY_WEBUI.bin" in "bin/ar71xx/tltFws/".

### Installation

Update the firmware via the web interface or through the CLI on your device.
