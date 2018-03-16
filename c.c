#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*-------- UTILITY DEFINITIONS ------*/

int buf_sz = 0;
char* buf = NULL;

void read_file(char* file) {
    
  FILE *f = fopen(file, "rb");
  fseek(f, 0, SEEK_END);
  long fsize = ftell(f);
  fseek(f, 0, SEEK_SET);

  buf = malloc(fsize + 1);
  fread(buf, fsize, 1, f);
  buf_sz = fsize;
  
  fclose(f);
  buf[fsize] = 0;
}

int line = 1;
int col = 0;

char look;
int ptr = 0;

void consume() {
  if(ptr >= buf_sz) {
    look = -1;
  } else {
    look = buf[ptr++];
    if(look == '\n') {
      col = 1; line++;
    } else {
      col++;
    }
    
  }
}

void init() {
  consume();
}

void expected(char* str) __attribute__((noreturn));

void expected(char* str) {
  printf("%i:%i - expected %s\n", line, col, str);
  exit(1);
}

void skip_whitespace() {
  while(isspace(look)) {
    consume();
  }
}


int streq(char* name, char* test) {
  return strcmp(name, test) == 0;
}


/*-----------------------------------*/

/*----------- TOKENIZATION ----------*/

#define TOKENS					\
  X(LPAREN) X(RPAREN)				\
  X(LBRACE) X(RBRACE)				\
  X(LBRACK) X(RBRACK)				\
  X(SEMICOL) X(COMMA)				\
  X(PLUS) X(MINUS)				\
  X(DIV) X(MULT) X(MOD)				\
  X(GT) X(LT) X(EQ)				\
  X(BOR) X(BAND) X(BXOR) X(BNOT)		\
  X(ASSIGN) X(SYMBOL) X(NUMBER)			\
  X(INT) X(CHAR)				\
  X(IF) X(ELSE) X(WHILE)			\
  X(NEQ) X(RETURN) X(EOF)



typedef enum {
#define X(nm) TOK_ ## nm ,
  TOKENS
  #undef X
} tok;

char* token_names[] = {
#define X(nm) #nm,
  TOKENS
  #undef X
};

tok look_tok;


int tok_capacity = 0;
int tok_sz = 0;
char* tok_sym;
int tok_val;


tok get_num_token() {
  if(!isdigit(look)) {
    expected("Integer");
  }

  int val = 0;
  while(isdigit(look)) {
    val *= 10;
    val += look - '0';
    consume();
  }

  tok_val = val;
  return TOK_NUMBER;
}

#define GROW(val) ((val) < 8 ? 8 : (1.5 * val))

void add_char(char c) {
  if(tok_sz >= tok_capacity) {
    tok_capacity = GROW(tok_capacity);
    tok_sym = realloc(tok_sym, tok_capacity);
  }
  
  tok_sym[tok_sz++] = c;
}



char* single_char_tok_str = "(){}[];,+-/*%><=|&^~";

int is_punctuation(char c) {
  return strchr(single_char_tok_str, c) != NULL;
}


tok get_sym_token() {
  
  
  if(!isalpha(look)) {
    expected("Symbol");
  }
  
  tok_sz = 0;

  while(!isspace(look) && !is_punctuation(look)) {
    add_char(look);
    consume();
  }
  
  add_char('\0');
  tok types[] = {TOK_INT, TOK_CHAR, TOK_IF, TOK_ELSE, TOK_WHILE};
  char* strs[] = {"int", "char","if", "else", "while", "return"};
  for(unsigned int i = 0; i < (sizeof(strs)/sizeof(char*)); i++) {
    if(streq(tok_sym, strs[i])) {
      return types[i];
    }
  }
  return TOK_SYMBOL;
}


tok get_token() {
  skip_whitespace();
  
  char* pos = strchr(single_char_tok_str, look);

  
  
  
  if(pos != NULL) {
    consume();
    tok token = pos - single_char_tok_str;
    if(token == TOK_LT) {
      if(look == '-') {
	consume();
	return TOK_ASSIGN;
      } else {
	return TOK_LT;
      }
    } else if (token == TOK_BOR) {
      if(look == '|') {
	consume();
	return TOK_BOR;
      } else {
	expected("|");
      }
      
    } else {
      return token;
    }
    
  } else if (look == -1) {
    return TOK_EOF;
  } else if (isdigit(look)) {
    return get_num_token();
  } else if (isalpha(look)) {
    return get_sym_token();
  } else if (look == '!') {
    consume();
    if(look == '=') {
      consume();
      return TOK_NEQ;
    } else {
      expected("'=' in '!='");
    }
  } else {
    expected("Number or Symbol");
  }
  
}

void consume_tok() {
  look_tok = get_token();
}

void init_tok() {
  consume_tok();
}

int is_addop(tok t) {
  return (t == TOK_PLUS || t == TOK_MINUS);
}

int is_mulop(tok t) {
  return (t == TOK_MOD || t == TOK_MULT || t == TOK_DIV);
}

int is_boolop(tok t) {
  return (t == TOK_BOR || t == TOK_BAND || t == TOK_BNOT);
}

int is_relop(tok t) {
  return (t == TOK_EQ || t == TOK_NEQ || t == TOK_GT || t == TOK_LT);
}


void match_tok(tok t) {
  if (look_tok == t) {
    consume_tok();
  } else {
    printf("%i:%i - Expected %s\n", line, col, token_names[t]);
    exit(1);
  }
}


/*-----------------------------------*/




/*--------- code generation ---------*/


typedef enum {
  INT,
  CHAR
} runtime_type;


#define OPCODES						\
  X(LIT, 1) X(DROP, 0)					\
  X(POPENV, 1) X(PUSHENV, 1)				\
  X(CALL, 1) X(RET, 0)					\
  X(MKENV, 1) X(EXTEND_ENV, 0)				\
  X(POP_EXTEND_ENV, 0)					\
  X(CHKTYPE_POPENV, 2) X(DROPENV, 0)			\
  X(ADD, 0) X(SUB, 0)   X(NEG, 0)			\
  X(MUL, 0) X(DIV, 0)   X(MOD, 0)			\
  X(IOR, 0) X(XOR, 0)   X(AND, 0)			\
  X(NOT, 0) X(BRA, 1)   X(BEQ, 1)			\
  X(CMP, 0) X(TST, 0) X(BGT, 1)				\
  X(PRINT, 0) X(PUTC, 0)				\
  X(READ, 0) X(READC, 0)				\
  X(HALT, 0)





typedef enum {
#define X(nm, op) nm,
  OPCODES
#undef X
} opcode;

char* opcode_names[] = {
#define X(nm, op) #nm,
  OPCODES
#undef X
};

int has_operand[] = {
#define X(nm, op) op,
  OPCODES
#undef X
};


int grow(int i){
  return (i < 8 ? 8 : (i * 1.5));
}

int program_size = 0;
int *program_bytes = NULL;

int emit_int(int i) {
  static int program_capacity = 0;
  if(program_size >= program_capacity) {
    program_capacity = grow(program_capacity);
    program_bytes = realloc(program_bytes, sizeof(int) * program_capacity); 
  }

  int addr = program_size;

  program_bytes[program_size++] = i;
  
  return addr;
}

int emit_opcode(opcode o) {
  return emit_int((int)o);
}

int get_cur_addr() {
  return program_size;
}

void patch_int_at(int loc, int new_val) {
  if(loc >= program_size) {
    printf("Tried to patch outside of emitted program code.\n");
    exit(1);
  }
  program_bytes[loc] = new_val;
}

void print_program(int disassemble) {
  
  
  int pc = 0;
  if(disassemble) {

    printf("\n---- compiled output: %i bytes ----\n", program_size);

    while(pc < program_size) {
      
      printf("0x%04x: ", pc);
      opcode o = program_bytes[pc++];
      printf("%s ", opcode_names[o]);

      for(int i = 0; i < has_operand[o]; i++) {
	int operand = program_bytes[pc++];
	printf("0x%04x/%i ", operand, operand);
      }
      printf("\n");
      
    }
    printf("----------------------------------\n\n");
    
  } else {
    printf("writing\n");
    fwrite(program_bytes, sizeof(int), program_size, stdout);

  }
}


/*----RECURSIVE DESCENT PARSING------*/


typedef struct sym_tab sym_tab;

typedef enum {
  FUNC,
  VAR
} binding_type;

typedef struct {
  char* name;
  int env_idx;
  binding_type func_or_var;
  int func_num_args;
  tok type;
  int func_code_addr;
  int num_resolve_addrs;
  int *resolve_addrs;
} sym;


struct sym_tab {
  int num_syms;
  int syms_cap;
  sym* syms;
};


sym_tab* fresh_sym_tab() {
  sym_tab* res = malloc(sizeof(sym_tab));
  res->num_syms = 0;
  res->syms_cap = 0;
  res->syms = NULL;
  return res;
}

sym_tab *global_sym_tab;
sym_tab *cur_sym_tab;



// returns NULL if does not exist
sym* find_sym(char* name) {
  for(int i = 0; i < cur_sym_tab->num_syms; i++) {
    if(streq(cur_sym_tab->syms[i].name, name)) {
      return &cur_sym_tab->syms[i];
    }
  }
  return NULL;
}


void add_sym(char* name, binding_type func_or_var,
	     int func_num_args,
	     tok type, int code_addr) {
  
  if(find_sym(name) != NULL) {
    printf("Variable '%s' declared more than once.\n", name);
    exit(1);
  }
  
  if(cur_sym_tab->num_syms >= cur_sym_tab->syms_cap) {
    cur_sym_tab->syms_cap = GROW(cur_sym_tab->syms_cap);
    cur_sym_tab->syms     = realloc(cur_sym_tab->syms,     
				    sizeof(sym) * cur_sym_tab->syms_cap);
  }

  sym* s = &(cur_sym_tab->syms[cur_sym_tab->num_syms]);
  
  s->name = name;
  s->func_or_var = func_or_var;
  s->env_idx = cur_sym_tab->num_syms;
  s->type = type;
  s->func_num_args = func_num_args;
  
  s->func_code_addr = code_addr;
  s->num_resolve_addrs = 0;
  s->resolve_addrs = NULL;

  cur_sym_tab->num_syms++;
}

void add_var(char* name, tok tok_type) {
  add_sym(name, VAR, -1, tok_type, -1);
}

void add_func(char* name, int num_args, tok tok_type, int code_addr) {
  add_sym(name, FUNC, num_args, tok_type, code_addr);
}

sym* find_existing_sym(char* name) {
  sym* s = find_sym(name);
  if(s == NULL) {
    printf("'%s' is not defined.\n", name);
    exit(1);
  }
  return s;
}

void expression();


void func_call(char* name) {
  match_tok(TOK_LPAREN);
  int num_args = 0;
    
  while(look_tok != TOK_RPAREN) {
    expression();
    num_args++;
    if(look_tok == TOK_RPAREN) {
      break;
    }
    match_tok(TOK_COMMA);
  }

  match_tok(TOK_RPAREN);
  
  if(streq(name, "print") || streq(name, "putc")) {
    
    if(num_args != 1) {
      printf("Tried to apply %s() to more than one argument.\n", name);
      exit(1);
    }
    if(streq(name, "print")) {
      emit_opcode(PRINT);
    } else {
      emit_opcode(PUTC);
    }
  } else if (streq(name, "read")  || streq(name, "readc")) {
    if(num_args != 0) {
      printf("Tried to apply %s() to one or more arguments.\n", name);
      exit(1);
    }
    if (streq(name, "read")) {
      emit_opcode(READ);
    } else {
      emit_opcode(READC);
    }
  } else {
    sym* s = find_existing_sym(name);
    if(s->func_or_var != FUNC) {
      printf("Attempted to call variable '%s'.\n", name);
      exit(1);
    }
    if(num_args != s->func_num_args) {
      printf("%s expects %i parameter(s) but only got %i\n", 
	     name, s->func_num_args, num_args);
      exit(1);
    }
    
    emit_opcode(CALL);
    if(s->func_code_addr != -1) {
      emit_int(s->func_code_addr);
    } else {
      int patch_addr = emit_int(0xDEAD);
      s->resolve_addrs = realloc(s->resolve_addrs, sizeof(int) *
				 s->num_resolve_addrs+1);
      s->resolve_addrs[s->num_resolve_addrs++] = patch_addr;
    }
            
  }
}

void ident() {
  char* name = strdup(tok_sym);

  match_tok(TOK_SYMBOL);
  
  if(look_tok == TOK_LPAREN) {
    func_call(name);
  } else {
    sym* s = find_existing_sym(name);
    if(s->func_or_var == FUNC) {
      printf("Attempted to use reference to function '%s' in expression.\n", name);
      exit(1);
    }

    emit_opcode(PUSHENV);
    emit_int(s->env_idx);
  }
  free(name);
}

void factor() {
  if(look_tok == TOK_LPAREN) {
    match_tok(TOK_LPAREN);
    expression();
    match_tok(TOK_RPAREN);
  } else if (look_tok == TOK_SYMBOL) {
    ident();
  } else {
    emit_opcode(LIT);
    emit_opcode(tok_val);
    
    match_tok(TOK_NUMBER);
    
  }
}

void signed_factor() {
  if(look_tok == TOK_PLUS) {
    match_tok(TOK_PLUS);
  } else if (look_tok == TOK_MINUS) {
    match_tok(TOK_MINUS);
    if(look_tok == TOK_NUMBER) {
      emit_opcode(LIT);
      emit_int(-tok_val);
      match_tok(TOK_NUMBER);
    } else {
      factor();
      emit_opcode(NEG);
    }
  } else {
    factor();
  }
}

void multiply() {
  match_tok(TOK_MULT);
  factor();
  emit_opcode(MUL);
}

void divide() {
  match_tok(TOK_DIV);
  factor();
  emit_opcode(DIV);
}

void mod() {
  match_tok(TOK_MOD);
  factor();
  emit_opcode(MOD);
}

void term1() {
  while (is_mulop(look_tok)) {
    switch(look_tok) {
    case TOK_MULT:
      multiply();
      break;
    case TOK_DIV:
      divide();
      break;
    case TOK_MOD:
      mod();
      break;
    default:
      expected("Mulop");
      break;
    }
  }
}

void term() {
  factor();
  term1();
}


void first_term() {
  signed_factor();
  term1();
}

void add() {
  match_tok(TOK_PLUS);
  term();
  emit_opcode(ADD);
}

void sub() {
  match_tok(TOK_MINUS);
  term();
  emit_opcode(SUB);
}


void addexpr() {
  first_term();
  
  while (is_addop(look_tok)) {
    switch(look_tok) {
    case TOK_PLUS:
      add();
      break;
    case TOK_MINUS:
      sub();
      break;
    default:
      expected("Addop");
      break;
    }
  }
}


void cmp_func(tok type, opcode branch_inst, int cc_is_true) {
  match_tok(type);
  addexpr();
  emit_opcode(CMP);
  int bne_loc = emit_opcode(branch_inst);
  int bne_target_loc = emit_int(0xDEAD);
  emit_opcode(LIT);
  emit_int(cc_is_true ? 0 : 1);
  int bra_loc = emit_opcode(BRA);
  int bra_target_loc = emit_opcode(0xDEAD);
  int bne_target = emit_opcode(LIT);
  emit_int(cc_is_true ? 1 : 0);
  int bra_target = get_cur_addr();
  
  patch_int_at(bne_target_loc, bne_target-bne_loc);
  patch_int_at(bra_target_loc, bra_target-bra_loc);
  
}

void eq() {
  cmp_func(TOK_EQ, BEQ, 1);
}

void neq() {
  cmp_func(TOK_NEQ, BEQ, 0);
}

void gt() {
  cmp_func(TOK_GT, BGT, 1);
}

void lt() {
  cmp_func(TOK_LT, BGT, 0);
}

void relexpr() {
  addexpr();

  while(is_relop(look_tok)) {
    switch(look_tok) {
    case TOK_EQ:
      eq();
      break;
    case TOK_NEQ:
      neq();
      break;
    case TOK_GT:
      gt();
      break;
    case TOK_LT:
      lt();
      break;
    default:
      expected("Relop");
      break;
    }
  }
}


void bor() {
  match_tok(TOK_BOR);
  relexpr();
  emit_opcode(IOR);
}

void band() {
  match_tok(TOK_BAND);
  relexpr();
  emit_opcode(AND);
}

void expression() {
  relexpr();

  while(is_boolop(look_tok)) {
    switch(look_tok) {
    case TOK_BOR:
      
      bor();
      break;
    case TOK_BAND:
      band();
      break;
    default:
      expected("Boolop");
      break;
    }
  }
}


void var_decl(char* var_name, tok tok_type) {
  
  add_var(var_name, tok_type);
  
  if(look_tok == TOK_SEMICOL) {
    match_tok(TOK_SEMICOL);
    emit_opcode(EXTEND_ENV);
  } else {
    match_tok(TOK_EQ);
    expression();
    match_tok(TOK_SEMICOL);
    emit_opcode(POP_EXTEND_ENV);
  }
  
}

void var_assign(char* var_name) {
  match_tok(TOK_ASSIGN);
  expression();
  match_tok(TOK_SEMICOL);
  
  sym* s = find_existing_sym(var_name);

  if(s->func_or_var == FUNC) {
    printf("Attempted to assign to function.\n");
    exit(1);
  }

  emit_opcode(POPENV);
  emit_int(s->env_idx);
}


tok param() {
  tok type = look_tok;
  if(type == TOK_INT || type == TOK_CHAR) {
    consume_tok();
    char* name = strdup(tok_sym);
    match_tok(TOK_SYMBOL);
    add_var(name, type);
  } else {
    expected("type");
  }
  return type;
}

void statement();

void block() {
  match_tok(TOK_LBRACE);
  while(look_tok != TOK_RBRACE) {
    statement();
  }
  match_tok(TOK_RBRACE);
}

void func_decl(char* name, tok type) {

  match_tok(TOK_LPAREN);

  if(cur_sym_tab != global_sym_tab) {
    printf("Cannot declare a function inside a local scope.\n");
    exit(1);
  }
  
  cur_sym_tab = fresh_sym_tab();

  
  int jmp_loc = emit_opcode(BRA);
  int jmp_target_loc = emit_int(0xDEAD);


  int num_args = 0;
  tok* types = NULL;
  
  
  while(look_tok != TOK_EOF) {
    tok type = param();

    types = realloc(types, sizeof(tok) * num_args+1);
    types[num_args++] = type;
    
    if(look_tok == TOK_RPAREN) {
      break;
    }
    match_tok(TOK_COMMA);
  }
  
  match_tok(TOK_RPAREN);

  // create new stack frame (aka environment frame)
  int func_addr = emit_opcode(MKENV);
  emit_int(num_args);

  // initialize parameter bindings and check types of arguments
  for(int i = num_args-1; i >= 0; i--) {
    emit_int(CHKTYPE_POPENV);
    emit_int(types[i] == TOK_INT ? INT : CHAR);
    emit_int(i);
  }
  
  free(types);

  // this allows recursion
  add_func(name, num_args, type, func_addr);

  block();
  
  // drop environment frame and return
  emit_opcode(DROPENV);
  emit_opcode(RET);
  
  int jmp_target = get_cur_addr();

  patch_int_at(jmp_target_loc, jmp_target-jmp_loc);
  
  cur_sym_tab = global_sym_tab;

  add_func(name, num_args, type, func_addr);

  emit_opcode(EXTEND_ENV);
  
}

void while_statement() {
  match_tok(TOK_WHILE);
  match_tok(TOK_LPAREN);

  int top_loc = get_cur_addr();
  expression();
  
  match_tok(TOK_RPAREN);
  
  emit_opcode(TST);
  int test_loc = emit_opcode(BEQ);
  int test_target_loc = emit_int(0xDEAD);

  block();

  int jmp_top_loc = emit_opcode(BRA);
  emit_int(top_loc - jmp_top_loc);

  patch_int_at(test_target_loc, get_cur_addr() - test_loc);
}

void if_statement() {
  match_tok(TOK_IF);
  
  match_tok(TOK_LPAREN);
  expression();
  match_tok(TOK_RPAREN);
  
  emit_opcode(TST);
  int jmp_else_loc = emit_opcode(BEQ);
  int jmp_else_target_loc = emit_int(0xDEAD);

  block();
    
  int end_target_loc;
  int end_jmp_loc;
  int needs_end_target = 0;

  
  if(look_tok == TOK_ELSE) {
    match_tok(TOK_ELSE);

    end_jmp_loc = emit_opcode(BRA);
    end_target_loc = emit_int(0xDEAD);
    needs_end_target = 1;
    
    int else_target = get_cur_addr();
    patch_int_at(jmp_else_target_loc, else_target-jmp_else_loc);
  
    block();
    
  } else {
    int else_target = get_cur_addr();
    patch_int_at(jmp_else_target_loc, else_target-jmp_else_loc);
  }

  if(needs_end_target) {
    int end_target = get_cur_addr();
    patch_int_at(end_target_loc, end_target - end_jmp_loc);
  }
  
}

void statement() {
  
  if(look_tok == TOK_SYMBOL) {
    
    char* sym_name = strdup(tok_sym);
    match_tok(TOK_SYMBOL);
    // either a function call
    // if statement
    // or variable assignment
    
    if (look_tok == TOK_LPAREN) {
      func_call(sym_name);
      match_tok(TOK_SEMICOL);
    } else if (look_tok == TOK_ASSIGN) {
      var_assign(sym_name);
    } else {
      expected("Statement\n");
    }
  } else if (look_tok == TOK_WHILE) {

    while_statement();
    
  } else if (look_tok == TOK_IF) {
    
    if_statement();

      
  } else if (look_tok == TOK_INT || look_tok == TOK_CHAR) {
    // either a variable declaration or a function declaration
    tok tok_type = look_tok;
    match_tok(tok_type);
    char* name = strdup(tok_sym);
    match_tok(TOK_SYMBOL);
    
    if(look_tok == TOK_LPAREN) {
      func_decl(name, tok_type);
    } else {
      var_decl(name, tok_type);
    }
  } else {
    expected("Statement");
  }

}

void program() {

  emit_opcode(MKENV);
  emit_int(0);
  
  while(look_tok != TOK_EOF) {
    statement();
  }

  emit_opcode(HALT);
  
}


typedef struct {
  runtime_type typ;
  union {
    int i;
    char c;
  };
} cell;

#define STACK_SZ 4096

cell stack[STACK_SZ];
int rstack[STACK_SZ];

int sp = 0;
int rsp = 0;

typedef struct env env;

struct env {
  env* parent;
  int cur_size;
  int cur_cap;
  cell slots[];
};

env *cur_env = NULL;


void execute_program() {
  int pc = 0;

  int eq_flag = 0;
  int gt_flag = 0;

  
  while(pc < program_size) {
    opcode code = program_bytes[pc++];

    switch(code) {
    case LIT:
      if(sp == STACK_SZ) {
	printf("\nStack overflow\n");
	exit(1);
      }
      stack[sp++].i = program_bytes[pc++];
      break;
    case DROP:
      sp--;
      break;
    case POPENV:
      do {
	int slot = program_bytes[pc++];
	cur_env->slots[slot] = stack[--sp];
      } while(0);
      break;
    case PUSHENV:
      do {
	int slot = program_bytes[pc++];
	stack[sp++] = cur_env->slots[slot];
      } while(0);
      break;
    case CALL:
      do {
	int abs_target = program_bytes[pc++];
	int ret = pc;
	pc = abs_target;
	if(rsp == STACK_SZ) {
	  printf("\nReturn stack overflow.\n");
	  exit(1);
	}
	rstack[rsp++] = ret;
      } while(0);
      break;
    case RET:
      pc = rstack[--rsp];
      break;
    case MKENV:
      do {
	int size = program_bytes[pc++];
	env* old_env = cur_env;
	int cap = GROW(size);
	cur_env = malloc(sizeof(env) + (sizeof(cell) * cap));
	cur_env->parent = old_env;
	cur_env->cur_size = size;
	cur_env->cur_cap = cap;
      } while(0);
      break;
      
    case EXTEND_ENV:
      do {
	int new_sz = ++cur_env->cur_size;
	if(new_sz >= cur_env->cur_cap) {
	  cur_env->cur_cap = GROW(cur_env->cur_cap);
	  cur_env = realloc(cur_env, (sizeof(env) + (sizeof(cell) * cur_env->cur_cap)));
	}
      } while(0);
      break;

    case POP_EXTEND_ENV:
      do {
	int new_sz = ++cur_env->cur_size;
	if(new_sz >= cur_env->cur_cap) {
	  cur_env->cur_cap = GROW(cur_env->cur_cap);
	  cur_env = realloc(cur_env, (sizeof(env) + (sizeof(cell) * cur_env->cur_cap)));
	}
	cur_env->slots[new_sz-1] = stack[--sp];
      } while(0);
      break;
            
    case CHKTYPE_POPENV:
      do {
	pc++; // skip type operand
	//runtime_type type = program_bytes[pc++];
	int slot = program_bytes[pc++];
	cur_env->slots[slot] = stack[--sp];
      } while(0);
      break;
      
    case DROPENV:
      do {
	if(cur_env == NULL) {
	  printf("Attempted to drop a null environment.\n");
	  exit(1);
	}
	cur_env = cur_env->parent;
      } while(0);
      break;

      case ADD:
	do {
	  int b = stack[--sp].i;
	  int a = stack[--sp].i;
	  stack[sp++].i = a + b;
	} while(0);
	break;

    case SUB:
      do {
	int b = stack[--sp].i;
	int a = stack[--sp].i;
	stack[sp++].i = a - b;
      } while(0);
      break;

    case IOR:
      do {
	int b = stack[--sp].i;
	int a = stack[--sp].i;
	stack[sp++].i = b | a;
      } while(0);
      break;

    case NEG:
      do {
	int i = stack[--sp].i;
	stack[sp++].i = -i;
      } while(0);

    case BRA:
      do {
	int base = pc-1;
      	int rel_off = program_bytes[pc++];
	pc = base + rel_off;
	
      } while(0);
      break;

    case BGT:
      do {
	int branch_val = gt_flag;
	int base = pc-1;
	int rel_off = program_bytes[pc++];
	if(branch_val) {
	  pc = base + rel_off;
	}
      } while(0);
      break;
      
    case BEQ:
      do {
	int branch_val = eq_flag;
      	int base = pc-1;
	int rel_off = program_bytes[pc++];
	if(branch_val) {
	  pc = base + rel_off;
	}
      } while(0);
      break;

    case CMP:
      do {
	int val_b = stack[--sp].i;
	int val_a = stack[--sp].i;
	eq_flag = (val_a == val_b);
	gt_flag = (val_a > val_b);
      } while(0);
      break;

    case TST:
      do {
	int val = stack[--sp].i;
	eq_flag = (val == 0);
	gt_flag = (val > 0);
      } while(0);
      break;
      
    case PRINT:
      printf("%i", stack[--sp].i);
      break;

    case PUTC:
      printf("%c", stack[--sp].i);
      break;

    case READ:
      do {
	int val;
	scanf("%i", &val);
	stack[sp++].i = val;
      } while(0);
      break;
      
    case READC:
      stack[sp++].i = getchar();
      break;
      
    case HALT:
      return;
      break;

    default:
      printf("Unexpected opcode %s\n", opcode_names[code]);
      exit(1);
    }
  }
}


int main(int argc, char** argv) {
  global_sym_tab = fresh_sym_tab();
  cur_sym_tab = global_sym_tab;
  
  // -g/-d -> disassembled output
  // -b    -> binary output


  if(argc < 2) {
    printf("usage: c input_file [-d/--disassemble] [-e/--execute]\n");
    exit(1);
  }
  
  char* file = argv[1];

  int disassemble = 0;
  int execute = 0;
  
  for(int i = 2; i < argc; i++) {
    char* str = argv[i];
    if(streq(str, "-d") || streq(str, "--disassemble")) {
      disassemble = 1;
    } else if (streq(str, "-e") || streq(str, "--execute")) {
      execute = 1;
    } else {
      printf("Unrecognized option '%s'\n", str);
      exit(1);
    }
  }

  
  read_file(file);
  
  init();
  init_tok();  
  program();
  
  

  if(execute) {
    if(disassemble) {
      printf("Cannot execute and show disassembled output at the same time.\n");
      exit(1);
    }
    execute_program();
  } else {
    print_program(disassemble);
  }
    
}
