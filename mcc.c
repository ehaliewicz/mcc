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

  while(1) {
    if(s[i] == 0) {
      break;
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

  res[bytes] = 0; // null terminator
  
  return res;
    
}


int streq(char* name, char* test) {
  int eq = 1;
  int valid = 1;

  int i = 0;
  while(valid == 1) {
    
    if(name[i] != test[i]) {
      eq = 0;
      break;
    }
    if (name[i] == 0) {
      break;
    }
    if(test[i] == 0) {
      break;
    }
    i = i+1;
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
  if(ptr > (buf_sz-1)) {
    look = -1;
  } else {
    look = buf[ptr];
    ptr = ptr+1;
    if(look == nl) {
      col = 1; line = line+1;
    } else {
      col = col+1;
    }
  }
}

void init() {
  consume();
}


void expected(char* str) {
  puti(line);
  putchar(':');
  puti(col);
  print(" - expected ");
  printnl(str);
  exit(1);
}

int is_space(char c) {
  if(c < 0) {
    return 0;
  }
  return (c < 33);
}

int is_alpha(char c) {
  if(c > 64) {
    if (c < 91) {
      return 1;
    }
    if(c > 96) {
      if (c < 123) {
	return 1;
      }
    }
  }

  return 0;
}

int is_digit(char c) {
  if(c > 47) {
    return (c < 58);
  }
  return 0;
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


int grow(int i){
  if(i < 8) {
    return 8;
  } else {
    return i * 2;
  }  
}



// stack machine opcodes

int LIT = 0; int DROP = 1;
int SWAP = 2; int POPENV = 3;
int PUSH_ENV = 4; int CALL = 5;
int RET = 6; int MKENV = 7;
int EXTEND_ENV = 8; int ADD = 9;
int SUB = 10; int NEG = 11;
int MUL = 12; int DIV = 13;
int MOD = 14; int IOR = 15;
int XOR = 16; int AND = 17;
int NOT = 18; int BRA = 19; 
int BNE = 20; int BEQ = 21; 
int CMP = 22; int CGT = 23; 
int CLT = 24;
int PUTCHAR = 25; int GETCHAR = 26;
int ALLOC = 26; int REALLOC = 27; int DEALLOC = 28;
int MSET = 29; int MGET = 30;
int OPEN = 31; int READ = 32;
int CLOSE = 33; int EXIT = 34;


char* opcode_names = "LIT_ DROP_ SWAP_ POPENV_ PUSH_ENV_ CALL_ RET_ MKENV_ EXTEND_ENV_ ADD_ SUB_ NEG_ MUL_ DIV_ MOD_ IOR_ XOR_ AND_ NOT_ BRA_ BNE_ BEQ_ CMP_ CGT_ CLT_ PUTCHAR_ GETCHAR_ ALLOC_ DEALLOC_ MSET_ MGET_ OPEN_ READ_ CLOSE_ EXIT_";

void print_opcode_name(int i) {

  int search_pos = 0;
  
  while(i > 0) {
    if(opcode_names[search_pos] == '_') {
      i = i-1;
      search_pos = search_pos + 1;
    } else {
      search_pos = search_pos + 1;
    }
  }
  
  while(opcode_names[search_pos] != '_') {
    putchar(opcode_names[search_pos]);
  }
}





int program_size = 0;
int *program_bytes = 0;
int program_capacity = 0;

int constants_size = 0;
int *constants_bytes = 0;
int constants_capacity = 0;

int emit_const(int i) {
  if(constants_size > (constants_capacity-1)) {
    constants_capacity = grow(constants_capacity);
    constants_bytes = realloc(constants_bytes, 4 * constants_capacity);
  }
}

int emit_int(int i) {
  if(program_size > (program_capacity-1)) {
    program_capacity = grow(program_capacity);
    program_bytes = realloc(program_bytes, 4 * program_capacity); 
  }

  int addr = program_size;

  program_bytes[program_size] = i;
  program_size = program_size + 1;
  
  return addr;
}

int emit_opcode(int o) {
  return emit_int(o);
}

int get_cur_addr() {
  return program_size;
}
int get_consts_addr() {
  return constants_size;
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
int TOK_CONTINUE = 32; int TOK_BREAK = 33;
int TOK_NEQ = 34; int TOK_RETURN = 35;
int TOK_EOF = 36; int TOK_COMMENT = 37;
int TOK_CHAR_CONST = 38; int TOK_STR_CONST = 39;



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
  if(is_digit(look)) {

    int val = 0;
    while(is_digit(look)) {
      val = val * 10;
      val = val + (look - '0');
      consume();
    }
    
    tok_val = val;
    return TOK_NUMBER;
    
  } else {
    expected("Integer");
  }
}


void add_char(char c) {
  if(tok_sz > (tok_capacity-1)) {
    tok_capacity = grow(tok_capacity);
    tok_sym = realloc(tok_sym, tok_capacity);
  }
  
  tok_sym[tok_sz] = c;
  tok_sz = tok_sz+1;
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




int str_pos(char* hay, char needle) {
  int i = 0;
  int res = -1;
  while(1) {
    
    if(hay[i] == 0) {
      break;
    }
    if(hay[i] == needle) {
      res = i;
      break;
    }
    i = i+1;
  }

  return res;
}

int is_punctuation(char c) {
  return str_pos(single_char_tok_str, c) != -1;
}


int is_single_quote(int token) {
  return token == 39;
}

int is_double_quote(int token) {
  return token == 34;
}


int get_lit_string() {
  tok_sz = 0;
  consume();

  tok_val = get_consts_addr();

  while(is_double_quote(look) == 0) {
    emit_const(look);
    consume();
  }
  
  emit_const(0);
  consume();
  
  
  return TOK_STR_CONST;
}

int get_sym_token() {

  if(is_alpha(look) == 0) {
    expected("Symbol");
  }
  
  tok_sz = 0;
  
  while(1) {
    if(is_space(look)) {
      break;
    }
    if(is_punctuation(look)) {
      break;
    }
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
  if(streq(tok_sym, "break")) {
    return TOK_BREAK;
  }
  
  return TOK_SYMBOL;
}

int type_to_pointer_type_token(int token) {
  if(token == TOK_INT) {
    return TOK_INT_PTR;
  }
  if (token == TOK_CHAR) {
    return TOK_CHAR_PTR;
  }
  expected("CHAR or INT");
  
}


int is_type_token(int token) {
  if(token == TOK_INT) {
    return 1;
  }
  if(token == TOK_CHAR) {
    return 1;
  }
  if(token == TOK_VOID) {
    return 1;
  }
  if(token == TOK_CHAR_PTR) {
    return 1;
  }
  if(token == TOK_INT_PTR) {
    return 1;
  }
  return 0;
}


int get_token() {
  skip_whitespace();
  int pos = str_pos(single_char_tok_str, look);
  int token;
  
  if(pos != -1) {
    consume();
    token = pos;
    if(token == TOK_ASSIGN) {
      if(look == '=') {
	consume();
	return TOK_EQ;
      } else {
	return TOK_ASSIGN;
      }
    }
    
    if (token == TOK_BOR) {
      if(look == '|') {
	consume();
	return TOK_BOR;
      } else {
	expected("|");
      }
    }
    if (token == TOK_DIV) {
      if(look == '/') {
	consume();
	return TOK_COMMENT;
      }
    } 
    return token;
    
    
  }
  if (look == -1) {
    return TOK_EOF;
  }
  if (is_digit(look)) {
    return get_num_token();
  }
  if (is_single_quote(look)) {
    return get_lit_char();
  }
  if (is_double_quote(look)) {
    return get_lit_string();
  }
  if (is_alpha(look)) {
    
    token = get_sym_token();
    // check if it's a pointer
    if(is_type_token(token)) {
      skip_whitespace();
      if(look == '*') {
	consume();
	return type_to_pointer_type_token(token);
      } 
    }
    return token;
  }
  if (look == '!') {
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
    putchar(nl);
    exit(1);
  }
}


// -----------------------------------




// --------- CODE GENERATION --------- 




void patch_int_at(int loc, int new_val) {
  if(loc > (program_size-1)) {
    printnl("Tried to patch outside of emitted program code.");
    exit(1);
  }
  program_bytes[loc] = new_val;
}

void print_operand(int operand) {
  print("0x");
  puth(operand);
  putchar('/');
  puti(operand);
  putchar(' ');
}

void print_program() {
  int pc = 0;
  
  putchar(nl);
  print("---- compiled output: ");
  puti(program_size);
  printnl(" bytes ----");
    
  while(pc < program_size) {
    print("0x");
    puth(pc);
    print(": ");

    int o = program_bytes[pc];
    pc = pc+1;
    print_opcode_name(o);
    putchar(' ');


    if(o == LIT) {
      print_operand(program_bytes[pc]);
      pc = pc+1;
    }
    if(o == POPENV) {
      print_operand(program_bytes[pc]);
      pc = pc+1;
    }
    if(o == PUSH_ENV) {
      print_operand(program_bytes[pc]);
      pc = pc+1;
    }
    if(o == CALL) {
      print_operand(program_bytes[pc]);
      pc = pc+1;	
    }
      
    putchar(nl);
      
  }
    
  printnl("----------------------------------");
  putchar(nl);
  
}


// ----RECURSIVE DESCENT PARSING------




int INT = 0;
int CHAR = 2;
int INT_PTR = 3;
int CHAR_PTR = 4;
int VOID = 4;


int type_token_to_type(int token) {
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
  int typ;
  int func_code_addr;
} sym;


struct sym_tab {
  int num_syms;
  int syms_cap;
  sym* syms;
};

int in_local_scope = 0;

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
      cur_sym_tab->syms_cap = grow(cur_sym_tab->syms_cap);
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
    putchar(' ');
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
  if(streq(name, "realloc")) {
    emit_opcode(REALLOC);
    check_args(name, num_args, 2);
    return 1;
  }
  if(streq(name, "free")) {
    emit_opcode(DEALLOC);
    check_args(name, num_args, 1);
    return 0;
  }
  if(streq(name, "exit")) {
    emit_opcode(EXIT);
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

    emit_opcode(PUSH_ENV);
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
    if(look_tok == TOK_STR_CONST) {
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
  emit_opcode(PUSH_ENV);
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

  in_local_scope = 1;
  block();
  in_local_scope = 0;
  
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
int cur_while_bail_addr;

void while_statement() {
  match_tok(TOK_WHILE, "while");
  match_tok(TOK_LPAREN, "(");

  
  int jmp_past_bail_loc = emit_opcode(BRA);
  int jmp_past_bail_target_loc = emit_int(0xDEAD);
  
  int bail_loc = emit_opcode(BRA);
  int bail_target_loc = emit_int(0xDEAD);

  int test_loc = get_cur_addr();

  patch_int_at(jmp_past_bail_target_loc, test_loc-jmp_past_bail_loc);
  
  expression();
  
  
  match_tok(TOK_RPAREN, ")");

  emit_opcode(BNE);
  int test_target_loc = emit_int(0xDEAD);

  
  
  int old_in_while = in_while; // to handle nesting
  int old_while_test_addr = cur_while_test_addr;
  int old_while_bail_addr = cur_while_bail_addr;
  in_while = 1;
  cur_while_test_addr = test_loc;
  cur_while_bail_addr = bail_loc;
  block();
  in_while = old_in_while;
  cur_while_test_addr = old_while_test_addr;
  cur_while_bail_addr = old_while_bail_addr;
  
  int jmp_top_loc = emit_opcode(BRA);
  emit_int(test_loc - jmp_top_loc);
  
  patch_int_at(test_target_loc, get_cur_addr() - test_loc);
  patch_int_at(bail_target_loc, get_cur_addr() - bail_loc);
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
    consume_tok();
  } else {
    print_location();
    printnl("'continue' invalid outside of while loop.");
    exit(1);
  }
}

void break_statement() {
  if(in_while) {
    int jmp_break_loc = emit_opcode(BRA);
    emit_int(cur_while_bail_addr - jmp_break_loc);
    consume_tok();
    match_tok(TOK_SEMICOL, ";");
  } else {
    print_location();
    printnl("'break' invalid outside of while loop.");
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
  } else if (look_tok == TOK_BREAK) {
    break_statement();
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
  int typ;
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
    int code = program_bytes[pc++];

    if(code == LIT) {
      if(sp == STACK_SZ) {
	putchar(nl);
	printnl("Stack overflow");
	exit(1);
      }
      stack[sp++].i = program_bytes[pc++];
      continue;
    }

    if(code == DROP) {
      sp--;
      continue;
    }
    
    if(code == SWAP) {
      cell a = stack[--sp];
      cell b = stack[--sp];
      stack[sp++] = a;
      stack[sp++] = b;
      continue;
    }

    if(code == POPENV) {
      int slot = program_bytes[pc++];
      cur_env->slots[slot] = stack[--sp];
      continue;
    }
    
    if (code == PUSH_ENV) {
      int slot = program_bytes[pc++];
      stack[sp++] = cur_env->slots[slot];
      continue;
    }
     
    if(code == CALL) {
      int abs_target = program_bytes[pc++];
      int ret = pc;
      pc = abs_target;
      if(rsp == STACK_SZ) {
	putchar(nl);
	printnl("Return stack overflow.");
	exit(1);
      }
      rstack[rsp++] = ret;
      continue;
    }
     
    if(code == RET) {
      pc = rstack[--rsp];
      if(cur_env->parent == NULL) {
	printnl("Attempt to return from global scope.");
	exit(1);
      }
      cur_env = cur_env->parent;
      continue;
    }
    
    if(code == MKENV) {
      int size = program_bytes[pc++];
      env* old_env = cur_env;
      int cap = grow(size);
      cur_env = malloc(sizeof(env) + (sizeof(cell) * cap));
      cur_env->parent = old_env;
      cur_env->cur_size = size;
      cur_env->cur_cap = cap;
      continue;
    }
      
    if(code == EXTEND_ENV) {
      int new_sz = ++cur_env->cur_size;
      if(new_sz >= cur_env->cur_cap) {
	cur_env->cur_cap = grow(cur_env->cur_cap);
	cur_env = realloc(cur_env, (sizeof(env) + (sizeof(cell) * cur_env->cur_cap)));
      }
      continue;
    }
    
    if(code == ADD) {
      int b = stack[--sp].i;
      int a = stack[--sp].i;
      stack[sp++].i = a + b;
      continue;
    }
    
    if(code == SUB) {
      int b = stack[--sp].i;
      int a = stack[--sp].i;
      stack[sp++].i = a - b;
      continue;
    }
     
    if(code == MUL) {
      int b = stack[--sp].i;
      int a = stack[--sp].i;
      stack[sp++].i = a * b;	
      continue;
    }

    if(code == DIV) {
      int b = stack[--sp].i;
      int a = stack[--sp].i;
      stack[sp++].i = a / b;
      continue;
    }
          
    if(code == MOD) {
      int b = stack[--sp].i;
	int a = stack[--sp].i;
	stack[sp++].i = a % b;
	continue;
    }

    if(code == IOR) {
      int b = stack[--sp].i;
      int a = stack[--sp].i;
      stack[sp++].i = b | a;
      continue;
    }

    if(code == NEG) {
      int i = stack[--sp].i;
      stack[sp++].i = -i;
      continue;
    }

    if(code == BRA) {
      int rel_off = program_bytes[pc++];
      pc = org_pc + rel_off;
      continue;
    }
     
    if(code == BNE) {
      int rel_off = program_bytes[pc++];
      if(stack[--sp].i == 0) {
	pc = org_pc + rel_off;
      }
      continue;
    }
    
    if(code == BEQ) {
      int rel_off = program_bytes[pc++];
      if(stack[--sp].i != 0) {
	pc = org_pc + rel_off;
      }
      continue;
    }

    if(code == CMP) {
      int val_b = stack[--sp].i;
      int val_a = stack[--sp].i;
      stack[sp++].i = (val_b == val_a) ? 1 : 0;
      continue;
    }

    if(code == CLT) {
      int val_b = stack[--sp].i;
      int val_a = stack[--sp].i;
      stack[sp++].i = (val_a < val_b) ? 1 : 0;
      continue;
    }      

    if(code == CGT) {
      int val_b = stack[--sp].i;
      int val_a = stack[--sp].i;
      stack[sp++].i = (val_a > val_b) ? 1 : 0;
      continue;
    }

    if(code == NOT) {
      int a = stack[--sp].i;
      stack[sp++].i = !a;
      continue;
    }

    if(code == PUTCHAR) {
      putchar(stack[--sp].c);
      continue;
    }

    if(code == GETCHAR) {
      stack[sp++].i = getchar();
      continue;
    }

    if(code == MSET) {
	int val = stack[--sp].i;
	int* addr = stack[--sp].ip;
	addr[0] = val;
	continue;
    }


    if(code == MGET) {
      int* val = stack[--sp].ip;
      stack[sp++].i = val[0];
      continue;
    }
      
    if(code == EXIT) {
      return stack[--sp].i;
    }
    
    if(code == ALLOC) {
      int bytes = stack[--sp].i; // number of bytes from stack
      int* allocated = calloc(bytes, 1); // allocate that number of bytes
      stack[sp++].ip = allocated;
      continue;
    }
    
    if(code == DEALLOC) {
      void* ptr = stack[--sp].vp;
      free(ptr);
      continue;
    }

    print("Unexpected opcode ");
    print_opcode_name(code);
    putchar(nl);
    exit(1);
    
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
    return execute_program();
  } else {
    print_program();
  }   

}
