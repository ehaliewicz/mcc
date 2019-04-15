#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>



// -------- UTILITY DEFINITIONS ------

char nl = 10;
int null = 0;

void print(char* s) {
  int i = 0;
  while(s[i] != 0) {
    putchar(s[i]);
    i = i+1;
  }
}

void printnl(char* s) {
  print(s);
  putchar(nl);
}

void puti_recur(int i, int base, int negative) {
  if(i == 0) {
    if(negative) {
      putchar(45); // negative sign
    }
  } else {
    int dig = i % base;
    int rem = i/base;
    puti_recur(rem, base, negative);
    if(dig > 9) {
      putchar(dig - 10 + 65); // count from A (for hex and other bases
			 // higher than 10)
    } else {
      putchar(dig+48);
    }
  }
}

void puti(int i) {
  int norm_i = i;
  if(norm_i < 0) {
    norm_i = -norm_i;
  }
  if(norm_i == 0) {
    putchar(48);
  } else {
    puti_recur(norm_i, 10, i < 0);  
  }
}

void puth(int i) {
  int norm_i = i;
  if(norm_i < 0) {
    norm_i = -norm_i;
  }
  if(norm_i == 0) {
    putchar(48);
  } else {
    puti_recur(norm_i, 16, i < 0);
  }
}

int slen(char* s) {
  int i = 0;
  int valid = 1;
  while(valid == 1) {
    if(s[i] == 0) {
      valid = 0;
    }
    i = i + 1;
  }

  return i;
}

char* sdup(char* str) {
  int bytes = slen(str);
  char* res = malloc(bytes+1);

  int i = 0;
  while(i < bytes) {
    res[i] = str[i];
    i = i+1;
    
  }
  str[i] = 0; // null terminator

  return res;
    
}


int streq(char* name, char* test) {
  int eq = 1;
  int valid = 1;

  while(valid == 1) {
    if(*name != *test) {
      valid = 0;
      eq = 0;
    } else if (*name == 0) {
      valid = 0;
    }
    name = name+1;
    test = test+1;
  }
  return eq;
}




int buf_sz = 1024*24; // max number of characters
char* buf = 0;        // buffer to read file into


int line = 1;
int col = 0;

void print_location() {
  puti(line);
  putchar(':');
  puti(col);
}

char look;
int ptr = 0;


void consume() {
  if(ptr >= buf_sz) {
    look = -1;
  } else {
    look = buf[ptr++];
    if(look == nl) {
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
  puti(line);
  putchar(':');
  puti(col);
  print(" - expected ");
  printnl(str);
  exit(1);
}

int is_space(char c) {
  return (c <= 32) && (c != EOF);
}

int is_alpha(char c) {
  return ((c >= 65 && c <= 91) ||
	  (c >= 97 && c <= 122));
}

int is_digit(char c) {
  return (c >= 48 && c <= 57); 
}

void skip_whitespace() {
  while(is_space(look)) {
    consume();
  }
}

void read_until_next_line() {
  while(look != nl) {
    consume();
  }
  consume();
}


// -----------------------------------

// ----------- TOKENIZATION ----------

int TOK_LPAREN = 0; int TOK_RPAREN = 1;
int TOK_LBRACE = 2; int TOK_RBRACE = 3;
int TOK_LBRACK = 4; int TOK_RBRACK = 5;
int TOK_SEMICOL = 6; int TOK_COMMA = 7;
int TOK_PLUS = 8; int TOK_MINUS = 9;
int TOK_DIV = 10; int TOK_MULT = 11;
int TOK_MOD = 12; int TOK_GT = 13;
int TOK_LT = 14; int TOK_ASSIGN = 15;
int TOK_BOR = 16; int TOK_BAND = 17;
int TOK_BXOR = 18; int TOK_BNOT = 19;
int TOK_HASH = 20; int TOK_EQ = 21;
int TOK_SYMBOL = 22; int TOK_NUMBER = 23;
int TOK_INT = 24; int TOK_CHAR = 25;
int TOK_VOID = 26; int TOK_CHAR_PTR = 27;
int TOK_INT_PTR = 28; int TOK_IF = 29;
int TOK_ELSE = 30; int TOK_WHILE = 31;
int TOK_CONTINUE = 32; int TOK_NEQ = 33;
int TOK_RETURN = 34; int TOK_EOF = 35;
int TOK_COMMENT = 36; int TOK_CHAR_CONST = 36;



char* single_char_tok_str = "(){}[];,+-/*%><=|&^~#";


//char* token_names[] = {
//#define X(nm) #nm,
//  TOKENS
//  #undef X
//};

int look_tok;


int tok_capacity = 0;
int tok_sz = 0;
char* tok_sym;
int tok_val;


int get_num_token() {
  if(!is_digit(look)) {
    expected("Integer");
  }

  int val = 0;
  while(is_digit(look)) {
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

int get_lit_char() {
  consume();
  int char_val = look;
  consume();
  if(look != 39) {
    expected("quote ending character constant.");
  }
  consume();
  tok_val = char_val;
  return TOK_CHAR_CONST;
}




char* str_pos(char* hay, char needle) {
  int valid = 1;
  char* ret = NULL;
  while(valid == 1) {
    if(*hay == 0) {
      valid = 0;
    } else if(*hay == needle) {
      valid = 0;
      ret = hay;
    }
    hay = hay+1;
  }
  return ret;
}

int is_punctuation(char c) {

  return str_pos(single_char_tok_str, c) != NULL;
}


int get_sym_token() {
  
  if(!is_alpha(look)) {
    expected("Symbol");
  }
  
  tok_sz = 0;

  while(!is_space(look) && !is_punctuation(look)) {
    add_char(look);
    consume();
  }
  
  add_char(0);
  
  if(streq(tok_sym, "int")) {
    return TOK_INT; 
  }
  if(streq(tok_sym, "char")) {
    return TOK_CHAR;
  }
  if(streq(tok_sym, "void")) {
    return TOK_VOID;
  }
  if(streq(tok_sym, "if")) {
    return TOK_IF;
  }
  if(streq(tok_sym, "else")) {
    return TOK_ELSE;
  }
  if(streq(tok_sym, "while")) {
    return TOK_WHILE;
  }
  if(streq(tok_sym, "return")) {
    return TOK_RETURN;
  }
  if(streq(tok_sym, "continue")) {
    return TOK_CONTINUE;
  }
  
  return TOK_SYMBOL;
}

int type_to_pointer_type_token(int token) {
  if(token == TOK_INT) {
    return TOK_INT_PTR;
  } else if (token == TOK_CHAR) {
    return TOK_CHAR_PTR;
  } else {
    expected("CHAR or INT");
  }
}


int is_type_token(int token) {
  return (token == TOK_INT || token == TOK_CHAR || token == TOK_VOID || token == TOK_CHAR_PTR || token == TOK_INT_PTR);  
}

int is_single_quote(int token) {
  return token == 39;
}

void consume_tok();


int get_token() {
  skip_whitespace();
  char* pos = str_pos(single_char_tok_str, look);

  if(pos != NULL) {
    consume();
    int token = pos - single_char_tok_str;
    if(token == TOK_ASSIGN) {
      if(look == '=') {
	consume();
	return TOK_EQ;
      } else {
	return TOK_ASSIGN;
      }
    } else if (token == TOK_BOR) {
      if(look == '|') {
	consume();
	return TOK_BOR;
      } else {
	expected("|");
      }
    } else if (token == TOK_DIV && look == '/') {
      consume();
      return TOK_COMMENT;
    } else {
      return token;
    }
    
  } else if (look == -1) {
    return TOK_EOF;
  } else if (is_digit(look)) {
    return get_num_token();
  } else if (is_single_quote(look)) {
    return get_lit_char();
  } else if (is_alpha(look)) {
    int token = get_sym_token();
    // check if it's a pointer
    if(is_type_token(token)) {
      skip_whitespace();
      if(look == '*') {
	consume();
	return type_to_pointer_type_token(token);
      } 
    }
    return token;
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

int is_addop(int t) {
  return (t == TOK_PLUS || t == TOK_MINUS);
}

int is_mulop(int t) {
  return (t == TOK_MOD || t == TOK_MULT || t == TOK_DIV);
}

int is_boolop(int t) {
  return (t == TOK_BOR || t == TOK_BAND || t == TOK_BNOT);
}

int is_relop(int t) {
  return (t == TOK_EQ || t == TOK_NEQ || t == TOK_GT || t == TOK_LT);
}


void match_tok(int t, char* token_name) {
  if (look_tok == t) {
    consume_tok();
  } else {
    print_location();
    print(" - Expected ");
    print(token_name);
    putchar('\n');
    exit(1);
  }
}


// -----------------------------------




// --------- CODE GENERATION --------- 


typedef enum {
  INT,
  CHAR,
  INT_PTR,
  CHAR_PTR,
  VOID
} type;


// stack machine opcodes
#define OPCODES						\
  X(LIT, 1) X(DROP, 0) X(SWAP, 0)			\
  X(POPENV, 1) X(PUSHENV, 1)				\
  X(CALL, 1) X(RET, 0)					\
  X(MKENV, 1) X(EXTEND_ENV, 0)				\
  X(POP_EXTEND_ENV, 0)					\
  X(ADD, 0) X(SUB, 0)   X(NEG, 0)			\
  X(MUL, 0) X(DIV, 0)   X(MOD, 0)			\
  X(IOR, 0) X(XOR, 0)   X(AND, 0)			\
  X(NOT, 0)						\
  X(BRA, 1) X(BNE, 1) X(BEQ, 1)				\
  X(CMP, 0) X(CGT, 0) X(CLT, 0)				\
  X(PUTCHAR, 0) X(GETCHAR, 0)				\
  X(ALLOC, 0) X(DEALLOC, 0)				\
  X(MSET, 0) X(MGET, 0)					\
  X(OPEN, 0) X(READ, 0) X(CLOSE, 0)			\
  X(EXIT, 0)





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
    printnl("Tried to patch outside of emitted program code.");
    exit(1);
  }
  program_bytes[loc] = new_val;
}

void print_program(int disassemble) {
  
  
  int pc = 0;
  if(disassemble) {
    
    putchar(nl);
    print("---- compiled output: ");
    puti(program_size);
    printnl(" bytes ----");
    
    while(pc < program_size) {
      print("0x");
      puth(pc);
      print(": ");

      opcode o = program_bytes[pc++];
      print(opcode_names[o]);
      putchar(' ');
      
      for(int i = 0; i < has_operand[o]; i++) {
	int operand = program_bytes[pc++];
	print("0x");
	puth(operand);
	putchar('/');
	puti(operand);
	putchar(' ');
      }
      putchar(nl);
      
    }
    
    printnl("----------------------------------");
    putchar(nl);
    
  } else {

    fwrite(program_bytes, sizeof(int), program_size, stdout);

  }
}


// ----RECURSIVE DESCENT PARSING------



type type_token_to_type(int token) {
  if(token == TOK_INT) {
    return INT;
  }
  if(token == TOK_CHAR) {
    return CHAR;
  }
  if(token == TOK_VOID) {
    return VOID;
  }
  if(token == TOK_INT_PTR) {
    return INT_PTR;
  }
  if(token == TOK_CHAR_PTR) {
    return CHAR_PTR;
  }

  print_location();
  printnl("Invalid type token.");
  exit(1);
}



typedef struct sym_tab sym_tab;

int FUNC_SYM = 0;
int VAR_SYM = 1;


typedef struct {
  char* name;
  int env_idx;
  int func_or_var;
  int func_num_args;
  type typ;
  int func_code_addr;
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

//char* global_sym_names;
//int* global_sym_env_idxs;
//int* global_sym_func_or_vars;


//int global_syms_cap;
//int global_num_syms;

sym_tab *cur_sym_tab;
//char* global_sym



// returns NULL if does not exist
sym* find_sym(char* name) {
  
  for(int i = 0; i < cur_sym_tab->num_syms; i++) {
    if(streq(cur_sym_tab->syms[i].name, name)) {
      return &cur_sym_tab->syms[i];
    }
  }
  for(int i = 0; i < global_sym_tab->num_syms; i++) {
    if(streq(global_sym_tab->syms[i].name, name)) {
      return &global_sym_tab->syms[i];
    }
  }
  return NULL;
}

sym* find_sym_in_cur_env(char* name) {
  for(int i = 0; i < cur_sym_tab->num_syms; i++) {
    if(streq(cur_sym_tab->syms[i].name, name)) {
      return &cur_sym_tab->syms[i];
    }
  }
  return NULL;
}

void func_error(char* func_name, char* err_str) {
  print_location();
  print(" Function ");
  print(func_name);
  putchar(' ');
  printnl(err_str);
  exit(1);
}

sym* add_sym(char* name, int func_or_var,
	     int func_num_args,
	     int typ, int code_addr) {
  sym* old_def_sym = find_sym_in_cur_env(name);
  if(old_def_sym != NULL && old_def_sym->func_or_var == VAR_SYM) {
    print_location();
    print(" Variable '");
    print(name);
    printnl("' declared more than once.");
    exit(1);
  }


  if(old_def_sym != NULL) {
    if(old_def_sym->func_or_var != func_or_var) {
      func_error(name, "redefined as variable.");
    }
    if(old_def_sym->func_num_args != func_num_args) {
      func_error(name, "redefined with different number of arguments.");
    }
    
    old_def_sym->func_code_addr = code_addr;
    
    return old_def_sym;
  } else {
    if(cur_sym_tab->num_syms >= cur_sym_tab->syms_cap) {
      cur_sym_tab->syms_cap = GROW(cur_sym_tab->syms_cap);
      cur_sym_tab->syms     = realloc(cur_sym_tab->syms,     
				      sizeof(sym) * cur_sym_tab->syms_cap);
    }

    sym* s = &(cur_sym_tab->syms[cur_sym_tab->num_syms]);
  
    s->name = name;
    s->func_or_var = func_or_var;
    s->env_idx = cur_sym_tab->num_syms;
    s->typ = type_token_to_type(typ);
    s->func_num_args = func_num_args;
  
    s->func_code_addr = code_addr;

    cur_sym_tab->num_syms++;
    return s;
  }
  
}

sym* add_var(char* name, int tok_type) {
  return add_sym(name, VAR_SYM, -1, tok_type, -1);
}

sym* add_func(char* name, int num_args, int tok_type, int code_addr) {
  return add_sym(name, FUNC_SYM, num_args, tok_type, code_addr);
}

sym* find_existing_sym(char* name) {
  sym* s = find_sym(name);
  if(s == NULL) {
    print_location();
    print("- '");
    print(name);
    printnl("' is not defined.");
    exit(1);
  }
  return s;
}

void expression();

int gen_func_call(char* name, int num_args) {
  
    sym* s = find_existing_sym(name);
    if(s->func_or_var != FUNC_SYM) {
      print("Attempted to call variable ");
      print(name);
      printnl(".");
      exit(1);
    }
    if(num_args != s->func_num_args) {
      print(name);
      print(" expects ");
      puti(s->func_num_args);
      print(" parameter(s) but only got ");
      puti(num_args);
      printnl(".");
      exit(1);
    }
    
    emit_opcode(CALL);
    if(s->func_code_addr != -1) {
      emit_int(s->func_code_addr);
    } else {
      printnl("Called function that hasn't been declared.");
      exit(1);
    }

    return (s->typ == VOID ? 0 : 1);
}

void check_args(char* name, int num_args, int num_expected_args) {
  if(num_args != num_expected_args) {
    print_location();
    print(name);
    print("() expected ");
    puti(num_expected_args);
    print(" args, but got ");
    puti(num_args);
    printnl(".");
    exit(1);
  }
}

// returns 1 if this function has a return value
int func_call(char* name) {
  match_tok(TOK_LPAREN, "(");
  int num_args = 0;
    
  while(look_tok != TOK_RPAREN) {
    expression();
    num_args++;
    if(look_tok == TOK_RPAREN) {
      break;
    }
    match_tok(TOK_COMMA, ",");
  }

  match_tok(TOK_RPAREN, ")");
  

  if(streq(name, "putchar")) {
    emit_opcode(PUTCHAR);
    check_args(name, num_args, 1);
    return 0;
  }
  if(streq(name, "getchar")) {
    emit_opcode(GETCHAR);
    check_args(name, num_args, 0);
    return 1;
  }
  if(streq(name, "read")) {
    emit_opcode(READ);
    check_args(name, num_args, 3);
    return 1;
  }
  if(streq(name, "open")) {
    emit_opcode(OPEN);
    check_args(name, num_args, 2);
    return 1;
  }
  if(streq(name, "close")) {
    emit_opcode(CLOSE);
    check_args(name, num_args, 1);
    return 1;
  }
  if(streq(name, "malloc")) {
    emit_opcode(ALLOC);
    check_args(name, num_args, 1);
    return 1;
  }
  if(streq(name, "free")) {
    emit_opcode(DEALLOC);
    check_args(name, num_args, 1);
    return 0;
  }

  return gen_func_call(name, num_args);
  
}

void ident() {
  char* name = sdup(tok_sym);

  match_tok(TOK_SYMBOL, "<symbol>");
  
  if(look_tok == TOK_LPAREN) {
    func_call(name);
  } else {
    sym* s = find_existing_sym(name);
    if(s->func_or_var == FUNC_SYM) {
      print("Attempted to use reference to function '");
      print(name);
      printnl("' in expression.");
      exit(1);
    }

    emit_opcode(PUSHENV);
    emit_int(s->env_idx);
  }
  free(name);
}

void factor() {
  if(look_tok == TOK_LPAREN) {
    match_tok(TOK_LPAREN, "(");
    expression();
    match_tok(TOK_RPAREN, ")");
  } else if (look_tok == TOK_SYMBOL) {
    ident();
  } else {
    emit_opcode(LIT);
    emit_opcode(tok_val);

    if(look_tok == TOK_NUMBER) {
      consume_tok();
      return;
    } 
    if(look_tok == TOK_CHAR_CONST) {
      consume_tok();
      return;
    }
    expected("number or character constant");
    
    
  }
}

void signed_factor() {
  if(look_tok == TOK_PLUS) {
    match_tok(TOK_PLUS, "+");
  } else if (look_tok == TOK_MINUS) {
    match_tok(TOK_MINUS, "-");
    if(look_tok == TOK_NUMBER) {
      emit_opcode(LIT);
      emit_int(-tok_val);
      match_tok(TOK_NUMBER, "<number>");
    } else {
      factor();
      emit_opcode(NEG);
    }
  } else {
    factor();
  }
}

void multiply() {
  match_tok(TOK_MULT, "*");
  factor();
  emit_opcode(MUL);
}

void divide() {
  match_tok(TOK_DIV, "/");
  factor();
  emit_opcode(DIV);
}

void mod() {
  match_tok(TOK_MOD, "%");
  factor();
  emit_opcode(MOD);
}

void term1() {
  while (is_mulop(look_tok)) {
    if(look_tok == TOK_MULT) {
      multiply();
      continue;
    }
    if(look_tok == TOK_DIV) {
      divide();
      continue;
    }
    if(look_tok == TOK_MOD) {
      mod();
      continue;
    }
    expected("Mulop");
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
  match_tok(TOK_PLUS, "+");
  term();
  emit_opcode(ADD);
}

void sub() {
  match_tok(TOK_MINUS, "-");
  term();
  emit_opcode(SUB);
}


void addexpr() {
  first_term();
  
  while(is_addop(look_tok)) {
    if(look_tok == TOK_PLUS) {
      add();
      continue;
    }
    if(look_tok == TOK_MINUS) {
      sub();
      continue;
    }
    expected("Addop");
    
  }
}

void arr_idx() {
  match_tok(TOK_LBRACK, "[");
  addexpr();
  
  emit_opcode(LIT); 
  emit_opcode(4); 
  emit_opcode(MUL);
  match_tok(TOK_RBRACK, "]");
}

void indexpr() {
  addexpr();
  
  if(look_tok == TOK_LBRACK) {
    arr_idx();
    emit_opcode(ADD);
    emit_opcode(MGET);
  }
}


void eq() {
  match_tok(TOK_EQ, "==");
  indexpr();
  emit_opcode(CMP);
}

void neq() {
  match_tok(TOK_NEQ, "!=");
  indexpr();
  emit_opcode(CMP);
  emit_opcode(NOT);
}

void gt() {
  match_tok(TOK_GT, ">");
  indexpr();
  emit_opcode(CGT);
}

void lt() {
  match_tok(TOK_LT, "<");
  indexpr();
  emit_opcode(CLT);
}

void relexpr() {

  indexpr();

  while(is_relop(look_tok)) {
    if(look_tok == TOK_EQ) {
      eq();
      continue;
    }
    if(look_tok == TOK_NEQ) {
      neq();
      continue;
    }
    if(look_tok == TOK_GT) {
      gt();
      continue;
    }
    if(look_tok == TOK_LT) {
      lt();
      continue;
    }
    expected("Relop");
  }
}


void bor() {
  match_tok(TOK_BOR, "|");
  relexpr();
  emit_opcode(IOR);
}

void band() {
  match_tok(TOK_BAND, "&");
  relexpr();
  emit_opcode(AND);
}

void expression() {
  relexpr();
  
  while(is_boolop(look_tok)) {
    if(look_tok == TOK_BOR) {
      bor();
      continue;
    }
    if(look_tok == TOK_BAND) {
      band();
      continue;
    }
    expected("Boolop");
  }
}


void var_decl(char* var_name, int tok_type) {
  
  sym* s = add_var(var_name, tok_type);
  
  if(look_tok == TOK_SEMICOL) {
    match_tok(TOK_SEMICOL, ";");
    emit_opcode(EXTEND_ENV);
  } else {
    match_tok(TOK_ASSIGN, "=");
    emit_opcode(EXTEND_ENV);
    expression();
    match_tok(TOK_SEMICOL, ";");
    emit_opcode(POPENV);
    emit_int(s->env_idx);
  }
  
}

void var_assign(char* var_name) {
  match_tok(TOK_ASSIGN, "=");
  expression();
  match_tok(TOK_SEMICOL, ";");
  
  sym* s = find_existing_sym(var_name);

  if(s->func_or_var == FUNC_SYM) {
    printnl("Attempted to assign to function.");
    exit(1);
  }

  emit_opcode(POPENV);
  emit_int(s->env_idx);
}

void arr_assign(char* var_name) {
  sym* s = find_existing_sym(var_name);
  if(s->func_or_var == FUNC_SYM) {
    printnl("Attempted to assign to function.");
  }
  emit_opcode(PUSHENV);
  emit_int(s->env_idx); // grab value from array

  arr_idx(); // get array index
  emit_opcode(ADD);
  
  match_tok(TOK_ASSIGN, "=");
  expression();
  
  match_tok(TOK_SEMICOL, ";");
  emit_opcode(MSET);
}


int param() {
  int type = look_tok;
  if(is_type_token(type)) {
    consume_tok();
    char* name = sdup(tok_sym);
    match_tok(TOK_SYMBOL, "<symbol>");
    add_var(name, type);
  } else {
    expected("type");
  }
  return type;
}

void statement();
void statement_with_returns();


void block() {
  match_tok(TOK_LBRACE, "{");
  while(look_tok != TOK_RBRACE) {
    if(cur_sym_tab != global_sym_tab) {
      statement_with_returns();
    } else {
      statement();
    }
  }
  match_tok(TOK_RBRACE, "}");
}

void func_decl(char* name, int type) {
  match_tok(TOK_LPAREN, "(");
  
  if(cur_sym_tab != global_sym_tab) {
    printnl("Cannot declare a function inside a local scope.");
    exit(1);
  }
  

  
  int jmp_loc = emit_opcode(BRA);
  int jmp_target_loc = emit_int(0xDEAD);


  int num_args = 0;

  cur_sym_tab = fresh_sym_tab();
  
  if(look_tok != TOK_RPAREN) {
        
    while(look_tok != TOK_EOF) {
    
      // no type checking at all!
      int type = param();
      num_args++;
      if(look_tok == TOK_RPAREN) {
	break;
      }
      
      match_tok(TOK_COMMA, ",");
    }
  }

  
  match_tok(TOK_RPAREN, ")");

  
  
  // create new stack frame (aka environment frame)
  int func_addr = emit_opcode(MKENV);
  emit_int(num_args);
  
  // initialize parameter bindings and check types of arguments
  for(int i = num_args-1; i >= 0; i--) {
    
    emit_opcode(POPENV);
    emit_int(i);
  }
  
  
  // this allows recursion
  add_func(name, num_args, type, func_addr);
  
  block();
  
  // drop environment frame and return
  emit_opcode(RET);
  
  
  int jmp_target = get_cur_addr();
  
  patch_int_at(jmp_target_loc, jmp_target-jmp_loc);
  
  cur_sym_tab = global_sym_tab;
  
  
  add_func(name, num_args, type, func_addr);

  emit_opcode(EXTEND_ENV);
  
}

int in_while = 0;
int cur_while_test_addr;

void while_statement() {
  match_tok(TOK_WHILE, "while");
  match_tok(TOK_LPAREN, "(");

  int top_loc = get_cur_addr();
  expression();
  
  
  match_tok(TOK_RPAREN, ")");

  
  int test_loc = emit_opcode(BNE);
  int test_target_loc = emit_int(0xDEAD);
  
  int old_in_while = in_while; // to handle nesting
  int old_while_test_addr = cur_while_test_addr;
  in_while = 1;
  cur_while_test_addr = test_loc;
  block();
  in_while = old_in_while;
  cur_while_test_addr = old_while_test_addr;
  
  
  int jmp_top_loc = emit_opcode(BRA);
  emit_int(top_loc - jmp_top_loc);
  
  patch_int_at(test_target_loc, get_cur_addr() - test_loc);
}

void if_statement() {
  match_tok(TOK_IF, "if");
  
  match_tok(TOK_LPAREN, "(");
  expression();
  match_tok(TOK_RPAREN, ")");
  
  int jmp_else_loc = emit_opcode(BNE);
  int jmp_else_target_loc = emit_int(0xDEAD);

  block();
    
  int end_target_loc;
  int end_jmp_loc;
  int needs_end_target = 0;

  
  if(look_tok == TOK_ELSE) {
    match_tok(TOK_ELSE, "else");

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

void return_statement() {
  match_tok(TOK_RETURN, "return");
  expression();
  match_tok(TOK_SEMICOL, ";");
}

void statement_with_returns() {
  if(look_tok == TOK_RETURN) {
    return_statement();
  } else {
    statement();
  }
}

void continue_statement() {
  if(in_while) {
    int jmp_top_loc = emit_opcode(BRA);
    emit_int(cur_while_test_addr - jmp_top_loc);
  } else {
    print_location();
    printnl("'continue' invalid outside of while loop.");
    exit(1);
  }
}



void statement() {
  if(look_tok == TOK_COMMENT) {
    read_until_next_line();
    consume_tok();
  } else if (look_tok == TOK_HASH) {
    read_until_next_line();
    consume_tok();
  } else if (look_tok == TOK_CONTINUE) {
    continue_statement();
  } else if(look_tok == TOK_SYMBOL) {

    char* sym_name = sdup(tok_sym);
    match_tok(TOK_SYMBOL, "<symbol>");
    // either a function call
    // if statement
    // or variable assignment
    
    if (look_tok == TOK_LPAREN) {
      int returns_val = func_call(sym_name);
      if(returns_val) {
	emit_opcode(DROP);
      }
      match_tok(TOK_SEMICOL, ";");
    } else if (look_tok == TOK_ASSIGN) {
      var_assign(sym_name);
    } else if (look_tok == TOK_LBRACK) {
      arr_assign(sym_name);
      
    } else {
      expected("Statement");
    }
  } else if (look_tok == TOK_WHILE) {
    while_statement();
    
  } else if (look_tok == TOK_IF) {
    if_statement();

  } else if (is_type_token(look_tok)) {
    // either a variable declaration or a function declaration
    int tok_type = look_tok;
    consume_tok();
    char* name = sdup(tok_sym);
    match_tok(TOK_SYMBOL, "<symbol>");

    
    
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

  emit_opcode(CALL);
  int patch_addr = emit_int(0xDEAD);
  add_func("main", 0, TOK_INT, -1);
  
  emit_opcode(LIT);
  emit_int(0);
  emit_opcode(EXIT);  
  
  while(look_tok != TOK_EOF) {
    statement();
  }
  
  sym* s = find_existing_sym("main");
  if(s->func_code_addr == -1) {
    printnl("No main function defined");
    exit(1);
  }
  patch_int_at(patch_addr, s->func_code_addr);
  
}


// -----------------------------------

// ---- RUNTIME & VIRTUAL MACHINE ----


typedef struct {
  type typ;
  union {
    int i;
    char c;
    char* cp;
    int* ip;
    void* vp;
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


int execute_program() {
  int pc = 0;

  while(pc < program_size) {
    int org_pc = pc; // pc of this instruction
    opcode code = program_bytes[pc++];

    switch(code) {
    case LIT:
      if(sp == STACK_SZ) {
	putchar(nl);
	printnl("Stack overflow");
	exit(1);
      }
      stack[sp++].i = program_bytes[pc++];
      break;
    case DROP:
      sp--;
      break;
    case SWAP:
      do {
	cell a = stack[--sp];
	cell b = stack[--sp];
	stack[sp++] = a;
	stack[sp++] = b;
      } while(0);
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
	  putchar(nl);
	  printnl("Return stack overflow.");
	  exit(1);
	}
	rstack[rsp++] = ret;
      } while(0);
      break;
    case RET:
      pc = rstack[--rsp];
      if(cur_env->parent == NULL) {
	printnl("Attempt to return from global scope.");
	exit(1);
      }
      cur_env = cur_env->parent;
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

    case MUL:
      do {
	int b = stack[--sp].i;
	int a = stack[--sp].i;
	stack[sp++].i = a * b;	
      } while(0);
      break;

    case DIV:
      do {
	int b = stack[--sp].i;
	int a = stack[--sp].i;
	stack[sp++].i = a / b;
      } while(0);
      break;
      
    case MOD:
      do {
	int b = stack[--sp].i;
	int a = stack[--sp].i;
	stack[sp++].i = a % b;
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
	int rel_off = program_bytes[pc++];
	pc = org_pc + rel_off;
	
      } while(0);
      break;

    case BNE:
      do {
	int rel_off = program_bytes[pc++];
	if(stack[--sp].i == 0) {
	  pc = org_pc + rel_off;
	}
      } while(0);
      break;

    case BEQ:
      do {
	int rel_off = program_bytes[pc++];
	if(stack[--sp].i != 0) {
	  pc = org_pc + rel_off;
	}
      } while(0);
      break;

    case CMP:
      do {
	int val_b = stack[--sp].i;
	int val_a = stack[--sp].i;
	stack[sp++].i = (val_b == val_a) ? 1 : 0;
      } while(0);
      break;

    case CLT:
      do {
	int val_b = stack[--sp].i;
	int val_a = stack[--sp].i;
	stack[sp++].i = (val_a < val_b) ? 1 : 0;
      } while(0);
      break;

      

    case CGT:
      do {
	int val_b = stack[--sp].i;
	int val_a = stack[--sp].i;
	stack[sp++].i = (val_a > val_b) ? 1 : 0;
      } while(0);
      break;  

    case NOT:
      do {
	int a = stack[--sp].i;
	stack[sp++].i = !a;
      } while(0);
      break;

    case PUTCHAR:
      putchar(stack[--sp].c);
      break;

    case GETCHAR:
      stack[sp++].i = getchar();
      break;

    case MSET: do {
	int val = stack[--sp].i;
	int* addr = stack[--sp].ip;
	addr[0] = val;
      } while(0);
      break;

    case MGET: do {
	int* val = stack[--sp].ip;
	stack[sp++].i = val[0];
      } while(0);
      break;
      
    case EXIT:
      return stack[--sp].i;
      break;

    case ALLOC: do {
	int bytes = stack[--sp].i; // number of bytes from stack
	int* allocated = calloc(bytes, 1); // allocate that number of bytes
	stack[sp++].ip = allocated;
	
      } while(0);
      break;
    case DEALLOC: do {
	void* ptr = stack[--sp].vp;
	free(ptr);
      } while(0);
      break;
    default:
      print("Unexpected opcode ");
      printnl(opcode_names[code]);
      exit(1);
    }
  }

  return 0;
}


int main(int argc, char** argv) {
  global_sym_tab = fresh_sym_tab();
  cur_sym_tab = global_sym_tab;
  
  // -g/-d -> disassembled output
  // -b    -> binary output

  if(argc < 2) {
    printnl("usage: c input_file [-d/--disassemble] [-e/--execute]");
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
      print("Unrecognized option '");
      print(str);
      printnl("'.");
      exit(1);
    }
  }

  int fd = open(file, 0);
  if(fd < 0) {
    print("Error opening file '");
    print(file);
    print("'\n");
    exit(1);
  }

  buf = malloc(buf_sz);
  int read_bytes = read(fd, buf, buf_sz-1);
  buf[read_bytes] = 0;
  close(fd);

  
  init();
  init_tok();  
  program();

  if(execute) {
    if(disassemble) {
      printnl("Cannot execute and show disassembled output at the same time.");
      exit(1);
    }
    return execute_program();
  } else {
    print_program(disassemble);
  }   

}
