#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
enum {
    TK_NOTYPE = 256, TK_EQ,
    TK_PLUS, TK_SUB, TK_MUL, TK_DIV,    // Operayions
    TK_DEC, TK_HEX,     // Number
    TK_LPARE, TK_RPARE, // Parenthesis
    TK_NEGA,            // Symbol
    TK_REG,             // Register

};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

    {" +",           TK_NOTYPE},                 // spaces
    {"\\+",          TK_PLUS},                   // plus
    {"==",           TK_EQ},                     // equal
    {"\\*",          TK_MUL},                    // multiply
    {"\\-",          TK_SUB},                    // Subtraction
    {"\\(",          TK_LPARE},                  // Left Parenthesis
    {"\\)",          TK_RPARE},                  // Right Parenthesis 
    {"/",            TK_DIV},                    // Devision
    {"\\$[A-Za-z]+", TK_REG},                    // Register 
    {"0x[A-Fa-f0-9]+",  TK_HEX},            // Hex Number 
    {"[0-9]+",  TK_DEC},                    // Dec Number 
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

Token tokens[32];
int nr_token;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        /*
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        */
        position += substr_len; 
        switch (rules[i].token_type) {
            case TK_NOTYPE: {
                printf("pause\n");
                break;
            }
            case TK_SUB: {
                if(nr_token == 0 || tokens[nr_token - 1].type != TK_DEC) {
                    tokens[nr_token].type = TK_NEGA;
                }
                else {
                    tokens[nr_token].type = TK_SUB;
                }
                strncpy(tokens[nr_token].str, substr_start, substr_len);
                tokens[nr_token].str[substr_len] = '\0';
                ++nr_token;
                break;
            }
            case TK_PLUS: case TK_EQ: case TK_MUL: case TK_DIV:
            case TK_LPARE: case TK_RPARE: 
            case TK_DEC: case TK_HEX:
            case TK_REG: {
                tokens[nr_token].type = rules[i].token_type;
                strncpy(tokens[nr_token].str, substr_start, substr_len);
                tokens[nr_token].str[substr_len] = '\0';
                ++nr_token;
                break;
            }
            
          default: TODO();
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

#define stacksize 31

typedef struct qrz_stack{
    char stack[stacksize];
    int top;
}MyStack;

#define isEmpty(S) (S.top == 0?true:false)

#define push(S, ch) if(S.top == stacksize){ \
                        panic("Buffer Overflow"); \
                    } \
                    S.stack[S.top]=ch; \
                    ++S.top

#define pop(S)  if(isEmpty(S) == true){ \
                    panic("Stack Empty"); \
                } \
                --S.top

#define test_output(p, q) \
            int temp_count = p; \
            for(; temp_count <= q; ++temp_count) \
                printf("%s", tokens[temp_count].str); \
            printf("\n")

static bool check_parentheses(uint32_t p,uint32_t q){
    int S = 0;
    if(tokens[p].type != TK_LPARE) return false;
    int temp_count=p;
    for(; temp_count <= q; ++temp_count){
        if(tokens[temp_count].type == TK_LPARE){
            ++S;
        }
        else if(tokens[temp_count].type == TK_RPARE){
            if(S == 0){
                panic("Wrong Expression");
                return false;       // Wrong Expression
            }
            --S;
            if(S == 0){
                if(temp_count == q) return true;
                return false;       // True Expression
            }
        }
    }
    return false;
}

#include<stdlib.h>

uint32_t eval(uint32_t p,uint32_t q){
    //test_output(p,q);
    int S = 0;
    if(p > q){
        panic("Wrong expression.");
    }
    else if (p == q){
        if(tokens[p].type == TK_DEC){
            return atoi(tokens[p].str);
        }
        else if(tokens[p].type == TK_HEX){
            return strtol(tokens[p].str, NULL, 16);
        }
        else if(tokens[p].type == TK_REG){
            int temp_count = 0;
            char temp_str[10];
            for(temp_count = 1; temp_count < strlen(tokens[p].str); ++temp_count){
                temp_str[temp_count - 1] = tokens[p].str[temp_count];
            }
            temp_str[temp_count - 1] = '\0';
            for(temp_count = 0; temp_count < 8; ++temp_count){
                if(strcmp(temp_str, regsl[temp_count]) == 0){
                    return reg_l(temp_count);
                }
                else if(strcmp(temp_str, regsw[temp_count]) == 0){
                    return reg_w(temp_count);
                }
                else if(strcmp(temp_str, regsb[temp_count]) == 0){
                    return reg_b(temp_count);
                }
            }
            if(strcmp(temp_str, "eip") == 0){
                return cpu.eip;
            }
            panic("Wrong Register.");        }
        else{
            panic("Wrong Number.");
        }
    }
    else if (check_parentheses(p, q) == true){
        return eval(p + 1, q - 1);
    }
    else {
        int op_type = 0x7fffffff;
        int op = 0;
        int temp_count = p;
        for(; temp_count <= q; ++temp_count){
            switch(tokens[temp_count].type){
                case TK_PLUS: case TK_SUB: {
                    if(S != 0) break;
                    op = temp_count;
                    op_type = tokens[temp_count].type;
                    break;
                }
                case TK_MUL: case TK_DIV: {
                    if(S != 0) break;
                    if(op_type < TK_MUL){
                        break;
                    }
                    op = temp_count;
                    op_type = tokens[temp_count].type;
                    break;
                }
                case TK_LPARE: {
                    ++S;
                    break;
                }
                case TK_RPARE: {
                    if(S == 0){
                        panic("Wrong Expression");
                    }
                    --S;
                    break;
                }
                default: break;
            }
        }
        if(op == 0){
            if(tokens[p].type == TK_NEGA){
                uint32_t ret = 0 - eval(p + 1, q);
                return ret;
            }
            else{
                panic("What 's Wrong???");
            }
        }
        uint32_t val1 = eval(p, op - 1);
        uint32_t val2 = eval(op + 1, q);

        switch (op_type) {
            case TK_PLUS: return val1 + val2;
            case TK_SUB:  return val1 - val2;
            case TK_MUL:  return val1 * val2;
            case TK_DIV:  return val1 / val2;
            default: {
                printf("Wrong op_type\n");
                assert(0);
            }
        }
    }
    return -1;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
    *success = true;
    return eval(0,nr_token - 1);
    uint32_t ans1=eval(0,nr_token-1);
    printf("%d\n", ans1);
    *success = true;
  //TODO();

  return 0;
}
