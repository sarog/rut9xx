CoovaChilli is a feature rich software access controller that provides a
captive portal / walled-garden environment and uses RADIUS or a HTTP protocol
for access provisioning and accounting.
Released under the GNU General Public License (GPL).

Visit website for documentation and archived content

[https://coova.github.io/](https://coova.github.io/)

Please use the [Github issues](https://github.com/coova/coova-chilli/issues) section for bug reports only.

There are now mailing lists for general support / discussion as well as an
announcement list for notices.

[General discussion / support mailing list](https://www.brightonchilli.org.uk/mailman/listinfo/coovachilli)

[Announcement mailing list](https://www.brightonchilli.org.uk/mailman/listinfo/coovachilli-announce)

[![Build Status](https://travis-ci.org/coova/coova-chilli.svg?branch=master)](https://travis-ci.org/coova/coova-chilli)

To get started after cloning git repository:

  `sh bootstrap`
  
  `./configure` 
  
  `make`

More details about the build process and dependencies are covered in the [INSTALL file](/INSTALL)

## UBUS Methods

The following methods are exposed over `ubus`:

**Object: chilli**

Path | Procedure | Signature | Description
--- | --- | --- | ---
chilli | list | `{"ip":"String","mac":"String","sessionid":"String"}` | List sessions. Params: `ip` - ip address (optional), mac - mac address (optional), sessionid - session ID (optional).
chilli | logout | `{"ip":"String","mac":"String","sessionid":"String"}` | Logoff active user. Params: `ip` - ip address (optional), mac - mac address (optional), sessionid - session ID (optional). At least one parameter required.
