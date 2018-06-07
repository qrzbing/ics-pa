#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#define EVAL_FAULT 0x80000000
enum {
    TK_NOTYPE = 256,    // to skip ascii 256 
    TK_OR, TK_AND, TK_NOT, 
    TK_EQ, TK_NEQ,
    TK_PLUS, TK_SUB, TK_MUL, TK_DIV,    // Operayions
    TK_DEC, TK_HEX,     // Number
    TK_LPARE, TK_RPARE, // Parenthesis
    TK_NEGA, TK_DEREF,  // Symbol
    TK_REG,             // Register 
};

static struct rule {
  char *regex;
  int token_type;
} rules[] = {

    {" +", TK_NOTYPE},                  // spaces
    {"\\+", TK_PLUS},                   // plus
    {"!=", TK_NEQ},                     // not equal 
    {"==", TK_EQ},                      // equal
    {"!", TK_NOT},                      // not
    {"\\*", TK_MUL},                    // multiply
    {"\\-", TK_SUB},                    // subtraction
    {"\\(", TK_LPARE},                  // left Parenthesis
    {"\\)", TK_RPARE},                  // right Parenthesis 
    {"/", TK_DIV},                      // devision
    {"\\|\\|", TK_OR},                  // or
    {"&&", TK_AND},                     // and
    {"\\$[A-Za-z]+", TK_REG},           // register 
    {"0x[A-Fa-f0-9]+", TK_HEX},         // Hex Number 
    {"[0-9]+", TK_DEC},                 // Dec Number 
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
            case TK_MUL: {
                if(nr_token == 0 || (tokens[nr_token - 1].type != TK_DEC && tokens[nr_token - 1].type != TK_HEX && tokens[nr_token - 1].type != TK_REG && tokens[nr_token - 1].type != TK_RPARE)){
                    tokens[nr_token].type = TK_DEREF;
                }
                else{
                    tokens[nr_token].type = TK_MUL;
                }
                strncpy(tokens[nr_token].str, substr_start, substr_len);
                tokens[nr_token].str[substr_len] = '\0';
                ++nr_token;
                break;
            }
            case TK_OR: case TK_AND: case TK_NOT:
            case TK_EQ: case TK_NEQ:
            case TK_PLUS: case TK_DIV:
            case TK_LPARE: case TK_RPARE: 
            case TK_DEC: case TK_HEX: case TK_REG: {
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
    int S = 0;
    if(p > q){
        printf("Wrong expression.\n");
        return EVAL_FAULT;
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
            printf("Wrong Register.\n");
            return EVAL_FAULT;
        }
        else{
            printf("Wrong Number.\n");
            return EVAL_FAULT;
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
            bool flag = false;
            if(tokens[temp_count].type == TK_OR){
                if(S != 0) continue; 
                flag = true;
            }
            else if(tokens[temp_count].type == TK_AND){
                if(S != 0 || op_type < TK_AND) continue;
                flag = true;
            }
            else if(tokens[temp_count].type == TK_EQ || tokens[temp_count].type == TK_NEQ){
                if(S != 0 || op_type < TK_EQ) continue;
                flag = true;
            }
            else if(tokens[temp_count].type == TK_PLUS || tokens[temp_count].type == TK_SUB){
                if(S != 0 || op_type < TK_PLUS) continue;
                flag = true;
            }
            else if(tokens[temp_count].type == TK_MUL || tokens[temp_count].type == TK_DIV){
                if(S != 0 || op_type < TK_MUL) continue;
                flag = true;
            }
            else if(tokens[temp_count].type == TK_LPARE){
                ++S;
                continue;
            }
            else if(tokens[temp_count].type == TK_RPARE){
                if(S == 0){
                    printf("Wrong Expression");
                    return EVAL_FAULT;
                }
                --S;
                continue;
            }
            if(flag == true){    
                op = temp_count;
                op_type = tokens[temp_count].type;
            }
        }
        if(op == 0){
            if(tokens[p].type == TK_NEGA){
                return 0 - eval(p + 1, q);
            }
            else if(tokens[p].type == TK_NOT){
                return !(eval(p + 1, q));
            }
            else if(tokens[p].type == TK_DEREF){
                // default watch 4 bytes of memory
                return vaddr_read(eval(p + 1, q), 4);
            }
            else{
                printf("Wrong Expression\n");
                return EVAL_FAULT;
            }
        }
        uint32_t val1 = eval(p, op - 1);
        uint32_t val2 = eval(op + 1, q);
        
        if(val1 == EVAL_FAULT || val2 == EVAL_FAULT) return EVAL_FAULT;

        switch (op_type) {
            case TK_PLUS: return val1 + val2;
            case TK_SUB: return val1 - val2;
            case TK_MUL: return val1 * val2;
            case TK_DIV: return val1 / val2;
            case TK_EQ: return val1 == val2;
            case TK_NEQ: return val1 != val2;
            case TK_AND: return val1 && val2;
            case TK_OR: return val1 || val2;
            default: {
                printf("Wrong op_type\n");
                return EVAL_FAULT;
            }
        }
    }
    return EVAL_FAULT;
}

uint32_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
  }

  /* TODO: Insert codes to evaluate the expression. */
    *success = true;
    int ans = eval(0, nr_token - 1);
    if(ans == 0x80000000) *success = false;
    return ans;
}
