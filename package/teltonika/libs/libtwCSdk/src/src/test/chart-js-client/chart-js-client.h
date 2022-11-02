//
// Created by William Reichardt on 12/9/16.
//

#ifndef TW_C_SDK_CHAR_JS_CLIENT_H
#define TW_C_SDK_CHAR_JS_CLIENT_H
void chartjs_write_graph(const char* name,const char* x,const char* xunits,const char* y,const char* yunits);
void chartjs_send_plain( const char* path, float value, unsigned long timestamp );
void chartjs_write_file();
#endif //TW_C_SDK_CHAR_JS_CLIENT_H
