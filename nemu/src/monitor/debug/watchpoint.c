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

void show_used_wp(){
    if(head == NULL){
        printf("No watchpoints.\n");
        return ;
    }
    WP *temp = head;
    printf("Num     What\n");
    while(temp != NULL){
        printf("%-8d%s\n", temp->NO, temp->expression);
        temp = temp->next;
    }
}

bool check_wp(){
    WP *temp = head;
    bool ret = false;
    while(temp != NULL){
        bool success = false;
        uint32_t ans_new = expr(temp->expression, &success);
        if(success == true){
            if(ans_new == temp->value) {
                temp = temp->next;
                continue;
            }
            else{
                ret = true;
                printf("Watchpoint %d: %s\n\n", temp->NO, temp->expression);
                printf("Old value = 0x%08x\nNew value = 0x%08x\n", temp->value, ans_new);
                temp->value = ans_new;
            }
        }
        else{
            printf("Invalid Expression\n");
        }
        temp = temp->next;
    }
    return ret;
}
