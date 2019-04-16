

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


int fib(int cnt) {
    if(cnt < 2) {
        return cnt;
    } else {
        return fib(cnt-2) + fib(cnt-1);
    }
}

int fib_iter(int cnt) {
    int a = 0;
    int b = 1;
    while(cnt > 0) {
        int c = a+b;
	a = b;
        b = c;
        cnt = cnt-1;
    }
    return a;
}

int main() {
  int nl = 10;
  exit(1);
  int i = getchar();
  
  int a = fib(i-'0');

  puti(a);
  putchar(nl);


  a = fib_iter(i);
  puti(a);
  putchar(nl);
}
