#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#define ZASYNC_IMPL
#include "zasync.h"

zdecl(session,
  zrettype(int),
  zparams(int, sock),
  int success;
  int bytes;
  char* buf;)
zasync(session, {
  zown(session, zarg(sock));
  z->buf = malloc(128);
  while (1) {
    zyield(zarg(sock), EV_READ, z->bytes = read(zarg(sock), z->buf, 128););
    if (z->bytes == 0) {
      close(zarg(sock));
      free(z->buf);
      break;
    }
    zyield(zarg(sock), EV_WRITE, write(zarg(sock), z->buf, z->bytes););
  }
})

zdecl(server_listen,
  zrettype(int),
  zparams(int, port),
  int s;
  int ls;
  socklen_t len;
  struct sockaddr_in addr;)
zasync(server_listen, {
  z->ls = socket(AF_INET, SOCK_STREAM, 0);
  z->addr.sin_family = AF_INET;
  z->addr.sin_addr.s_addr = INADDR_ANY;
  z->addr.sin_port = htons(zarg(port));
  int rc = bind(z->ls, (struct sockaddr *)&z->addr, sizeof(z->addr));
  if (rc < 0) {
    perror("error");
  }
  z->len = sizeof(z->addr);
  zown(server_listen, z->ls);
  listen(z->ls, 10);
  while(1) {
    printf("accepting...\n");
    zyield(z->ls, EV_READ, z->s = accept(z->ls, (struct sockaddr *)&z->addr, &z->len););
    printf("accepted\n");
    if (z->s <= 0) {
      continue;
    }
    zspawn(session, z->s);
  }
})

int main() {
  zinit();
  zspawn(server_listen, 1234);
  zrun();
}
