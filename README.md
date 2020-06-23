# Firmware Building And Installation

Note: This repo is based on firmware version 6.06.1.

## Prerequisites

    Linux, *BSD, OSX or WSL required for compilation. Cygwin will not work due to running on a case-insensitive filesystem.

## Building


    1. Install required packages

        $ sudo apt-get update
        $ sudo apt-get install -y build-essential curl devscripts gawk gcc-multilib gengetopt gettext git groff file flex \
            libncurses5-dev libssl-dev python2.7 subversion unzip vim-common zlib1g-dev wget

    2. Extract the archive to an empty folder:

        $ mkdir RUT9XX_R_GPL
        $ tar -xf RUT9XX_R_GPL_00.XX.YY.tar -C RUT9XX_R_GPL

    4. Build the image

        $ cd RUT9XX_R_GPL
        $ make

        After successful build you will get the image file "RUT9XX_R_GPL_00.XX.YY_WEBUI.bin" in "bin/ar71xx/tltFws/".

## Installation

    Update the new firmware via the web interface on your device.
