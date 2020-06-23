/* 
 * A simple pure C client for Graphite allows to send metrics to Graphite/Carbon using Graphite plaintext protocol
 * http://vdevos.github.io/graphite-c-client/
 */

#ifndef GRAPHITE_CLIENT_H
#define GRAPHITE_CLIENT_H

#include <sys/types.h>

int graphite_init( const char *host, const int port );
void graphite_finalize( void );

/** 
  This function allows you to send metrics to Graphite/Carbon using the plaintext protocol
  @param path       this is the metric path - example: server.process.task.load, server.process.task.count, etc.
  @param value      this is the metric value - example: 1, 12.4, 113.0, etc.
  @param timestamp  this is your metrics timestamp (UNIX Epoch) - Warning: timestamp == 0 is also accepted!

  WARNING: Make sure you use graphite_init on port: 2003 (only this port allows the plaintext protocol)
*/
void graphite_send_plain( const char* path, float value, unsigned long timestamp );

#endif
