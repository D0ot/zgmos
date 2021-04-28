#include <stdio.h>

int add(int a, int b) {
  return a + b;
}

int main(void) {
  int a = add(1, 2);
  if(a == 1) {
    add(1, 2);
  }
  while(1);
  return 0;
}
