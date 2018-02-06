#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);

/* We use the `readline' library to provide more flexibility to read from stdin. */
char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}

static int cmd_q(char *args) {
  return -1;
}

static int cmd_help(char *args);

static int cmd_si(char *args);

static int cmd_info(char *args);

static int cmd_x(char *args);

static int cmd_p(char *args);

static int cmd_w(char *args);

static int cmd_d(char *args);

static int cmd_math(char *args);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
    { "help", "help [cmmmand] , Display informations about all supported commands", cmd_help },
    { "c", "c, Continue the execution of the program", cmd_c },
    { "q", "q, Exit NEMU", cmd_q },
    { "si", "si [N], Step command", cmd_si },
    { "info", "info SUBCMD, Print register", cmd_info },
    { "x", "x N EXPR, Scan memory", cmd_x },
    { "p", "p EXPR, Show infomation", cmd_p },
    { "w", "w EXPR, Watch ", cmd_w },
    { "d", "d N, Delete Watchpoint", cmd_d },
    { "math", "math EXPR, Arithmetic evaluation", cmd_math },
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

static int cmd_si(char *args) {
    char *arg = strtok(NULL," ");

    if(arg == NULL) {
        /* no argument given */
        cpu_exec(1); 
    }
    else {
        int i = atoi(arg);
        if(i == 0) {
            /* illegal command */
            printf("Illegal command '%s'\n", arg);
            return -1;
        }
        cpu_exec(i);
    }
    
    return 0;
}


static int cmd_info(char *args){
    char *arg = strtok(NULL," ");

    if(arg==NULL) {
        printf("Invalid option.\n");
    }
    else{
        if(strcmp(arg,"r")==0){
            int temp_count=0;
            for(temp_count = 0; temp_count < 8; ++temp_count){
                printf("%s:    0x%x\n", regsl[temp_count], reg_l(temp_count));
            }
            printf("eip:    0x%x\n", cpu.eip);
            for(temp_count = 0; temp_count < 8; ++temp_count){
                printf("%s:     0x%x\n", regsw[temp_count], reg_w(temp_count));
            }
            for(temp_count = 0; temp_count < 8; ++temp_count){
                printf("%s:     0x%x\n", regsb[temp_count], reg_b(temp_count));
            }
        }
        else{
            printf("default\n");
        }
    }
    return 0;
}


static int cmd_x(char *args){
    char *arg1=strtok(NULL," ");
    char line_cmd[80]="";
    while(true){
        char *arg2=strtok(NULL," ");
        if(arg2==NULL) break;
        strcat(line_cmd,arg2);
    }
    bool success;
    uint32_t ans = expr(line_cmd, &success);
    if(success == true) {
        uint32_t addr = ans;
        int temp_sum = atoi(arg1);
        int temp_count = 0;
        for(; temp_count < temp_sum; ++temp_count){
            if(temp_count % 2 == 0){
                printf("0x%x: ", addr);
            }
            printf("0x%08x ", vaddr_read(addr, 8));
            addr += 4;
            if(temp_count % 2 == 1){
                printf("\n");
            }
        }
        if(temp_count % 2 == 1){
            printf("\n");
        }
        return 0;
    }
    else {
        printf("Invalid Command.\n");
        return -1;
    }
}

static int cmd_p(char *args){
    char line_cmd[80] = "\0";
    while(true){
        char *arg = strtok(NULL, " ");
        if(arg == NULL) break;
        if(strlen(arg) + strlen(line_cmd) > 80){
            panic("Buffer Overflow.");
        }
        strcat(line_cmd, arg);
    }
    bool success;
    uint32_t ans = expr(line_cmd, &success);
    if(success == true){
        printf("%s = 0x%x\n", line_cmd, ans);
        return 0;
    }
    else{
        printf("Invalid Command.\n");
        return -1;
    }
}

static int cmd_w(char *args){
    return -1;
}

static int cmd_d(char *args){
    return -1;
}

static int cmd_math(char *args){
    char line_cmd[80]="\0";
    while(true){
        char *arg=strtok(NULL, " ");
        if(arg == NULL) break;
        if(strlen(arg) + strlen(line_cmd) > 80){
            panic("Buffer Overflow");
        }
        strcat(line_cmd, arg);
    }
    bool success;
    uint32_t ans = expr(line_cmd, &success);
    if(success == true){
        printf("result = %d", ans);
        return 0;
    }
    else printf("Invalid Command.\n");
    return -1;
}

void ui_mainloop(int is_batch_mode) {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  while (1) {
    char *str = rl_gets();
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}
