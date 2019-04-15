char nl = 10;
int null = 0;

void print(char* s) {
  char* t = s;
  while(*t != 0) {
    putchar(*t);
    t++;
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
    if(dig >= 10) {
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
    if(*s == 0) {
      valid = 0;
    }
    i = i + 1;
    s = s + 1;
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

