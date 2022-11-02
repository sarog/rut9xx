/***************************************
 *  Copyright 2016, PTC, Inc.
 ***************************************/

/**
 * \file simpleshape.h
 * \brief Provides a simple thing shape example which provides two properties and a service.
 * \author bill.reichardt@thingworx.com
 *
 * Provides a simple thing shape example which provides two properties and a service. This header is only required
 * if this library is going to be statically linked to your application. It exposes the library initalization function
 * so that when you application starts, you can load this shape into your list of shapes which can be used when
 * you construct your thing using the twCreateThingFromTemplate() function.
*/
#ifndef TW_C_SDK_SIMPLESHAPE_H
#define TW_C_SDK_SIMPLESHAPE_H

int init_libsimpleext();
#endif //TW_C_SDK_SIMPLESHAPE_H
