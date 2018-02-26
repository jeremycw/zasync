#include <stdio.h>
#define ZASYNC_IMPL
#include "zasync.h"

zdecl(printer,
  zrettype(int),
  zparams(zsemaphore*, print, zsemaphore*, inc,  int*, count),
  int i;)
zasync(printer, {
  for (z->i = 0; z->i < 100; z->i++) {
    zwait(zarg(print));
    printf("count: %d\n", *zarg(count));
    zsignal(zarg(inc));
  }
})

zdecl(incrementer,
  zrettype(int),
  zparams(zsemaphore*, print, zsemaphore*, inc,  int*, count),
  int i;)
zasync(incrementer, {
  for (z->i = 0; z->i < 100; z->i++) {
    zsignal(zarg(print));
    *zarg(count) += 1;
    zwait(zarg(inc));
  }
})

int main() {
  int count = 0;
  zsemaphore print;
  zsemaphore inc;
  inc.count = 0;
  print.count = 0;
  print.stack = NULL;
  inc.stack = NULL;
  zspawn(printer, &print, &inc, &count);
  zspawn(incrementer, &print, &inc, &count);
}
