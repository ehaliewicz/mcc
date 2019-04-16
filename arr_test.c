int main() {
  int a = malloc(12);
  a[0] = 'a';
  a[1] = 'b';
  a[2] = 'c';
  a[3] = 10;
  a[4] = 'd';
  a[5] = 'e';
  a[6] = 'f';
  a[7] = 10;
  a[8] = 0;
  int i = 0;
  

  // print string
  while(i < 12) {
    if(a[i] == 0) {
      break;
    }
    putchar(a[i]);
    i = i+1;
  }
}
