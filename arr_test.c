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

  int i = 0;
  int valid = 1;

  // print string
  while(valid == 1) {
    putchar(a[i]);
    if(a[i] == 0) {
      valid = 0;
    }
    i = i+1;
  }
}
