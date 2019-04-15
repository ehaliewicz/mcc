#include <stdio.h>

void puti_recur(int i) {
  if(i == 0) {
    
  } else {
    int dig = i % 10;
    int rem = i/10;
    puti_recur(rem);
    putchar(dig+48);
  }
}

void puti(int i) {
  puti_recur(i);  
  i = i+i+i;
}


int fizz() {
  int a = 1;
      
  while(a < 100) {
    int modcheck = 0;
    if(a % 3 == 0) {
      modcheck = 1;
      putchar(102); putchar(105); putchar(122); putchar(122);
    }

    if(a % 5 == 0) {
      modcheck = 1;
      putchar(98); putchar(117); putchar(122); putchar(122);
    }

    if(modcheck == 0) {
      puti(a);
    }
    
    putchar(10);
    a = a+1;
  }
  return a;
}

int main() {
  fizz();
}
