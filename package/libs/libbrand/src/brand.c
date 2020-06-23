#include "brand.h"
#include "config.h"
#include <stdio.h>
#include <string.h>

void print_var(char* string){
  printf("%s", string);
}

char *brand2(char* arg){

  if (strcmp(arg, "company_name") == 0)
    return strings[1];
  else
    return strings[0];

  return NULL;
}

char *brand3(int arg){
  char buffer [50];
  int i = 0;

  while (strings[i] != NULL) {
    i++;
  }

  if ( arg > i-1){
    arg = 0;
  }

  return strings[arg];
}
