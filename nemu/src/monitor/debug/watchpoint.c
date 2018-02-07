#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

WP* new_WP(){
    if(free_ == NULL) return NULL;
    WP *temp_watchpoint = free_;
    free_ = free_->next;
    if(head == NULL){
        head = temp_watchpoint;
    }
    else{
        WP *temp_w = head;
        while(temp_w->next != NULL) temp_w = temp_w->next;
        temp_w->next = temp_watchpoint;
    }
    temp_watchpoint->next = NULL;
    return temp_watchpoint;
}

void free_wp(int n){
    WP *temp = head;
    WP *wp = NULL;
    while(temp != NULL){
        if(temp->NO == n){
            wp = temp;
            break;
        }
        temp = temp->next;
    }
    if(wp == NULL){
        printf("Invalid Number\n");
        return ;
    }
    if(head == wp){
        head = wp->next;
    } 
    else{
        WP *temp_w = head;
        while(temp_w->next != wp) temp_w = temp_w->next;
        temp_w->next = wp->next;
        if(free_ == NULL){
            free_ = wp;
            wp->next = NULL;
        }
        else{
            WP *temp_f = free_;
            while(temp_f->next != NULL) temp_f = temp_f->next;
            temp_f->next = wp;
            wp->next = NULL;
        }
    }
}

