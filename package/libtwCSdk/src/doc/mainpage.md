Introduction
------------

ThingWorx has introduced Software Development Kits (SDK) in various languages
in order to allow product companies to develop machine or device functionality
specific to their products and to easily connect those products to a ThingWorx
Platform Server. The SDKs can be implemented as a Gateway to several connected
products, or may be embedded directly in a product in a one-to-one basis.

All the ThingWorx SDKs share a common reference implementation and they provide
a secure communication conduit to the specified ThingWorx Platform, allowing a
device or machine to be a full participant in a ThingWorx solution.

The ThingWorx C SDK is a lightweight, but fully functional implementation of the
ThingWorx AlwaysOn™ binary protocol. It is designed to minimize memory
footprint while making it easy to integrate applications into the ThingWorx
distributed computing view of the Internet of Things. The goal of the C SDK is
to make creating applications that use it simple, but to also give the
developer enough flexibility to create very sophisticated applications.  For
example, the SDK contains a simple tasker that can be used to drive not only
the connectivity layer of the application, but the functionality of the
application itself.  However, it is not required to use the tasker at all. The
API is thread safe and can be used in a complex multithreaded environment as
well. Other examples of this flexibility are highlighted in this document.

Source Overview
---------------

The C SDK source code is organized into a number of directories:

 - src/api The main api functionality layer. See twApi.h for general
   functionality.
 - src/config The api configuration options. See twConfig.h for user
   definitions.
 - src/fileTransfer File transfer operations. See twFileManager.h for general
   functionality.
 - src/messaging Messaging and basic data type definitions.
 - src/porting OS-Specific definitions and options.
 - src/shapes Implementation of templates and shapes. Part of the Edge
   Extensions interface. See twShapes.h for general functionality.
 - src/simpleProps Default property handler. Part of the Edge Extensions
   interface. See twSimpleProps.h for general functionality.
 - src/thirdParty Third party libraries.
 - src/threadUtils Simple thread management. Part of the Edge Extensions
   interface. See twThreadUtils.h for general functionality.
 - src/tls TLS/SSL operations. See twTls.h for general functionality.
 - src/tunneling Tunneling operations. See twTunnelManager.h for general
   functionality.
 - src/utils Misc utilities including crypto library wrappers, JSON utilities,
   linked list functionality, string utilities, HTTP and NTLM proxy support,
logging, and tasking.
 - src/websocket Websockets operations. See twWebsocket.h for general
   functionality.

See <a href="files.html">the files page</a> for a list of all source files.
