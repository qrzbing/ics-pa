#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */
    char expression[80];
    uint32_t value;
} WP;

void init_wp_pool();
WP *new_WP();
void free_wp(int n);
#endif
