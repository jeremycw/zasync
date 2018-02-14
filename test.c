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
  zown(zarg(sock));
  z->buf = malloc(128);
  while (1) {
    zread(z->bytes, zarg(sock), z->buf, 128);
    if (z->bytes == 0) {
      close(zarg(sock));
      free(z->buf);
      break;
    }
    zwrite(z->bytes, zarg(sock), z->buf, z->bytes);
  }
})

void bind_localhost(int s, struct sockaddr_in* addr, int port) {
  addr->sin_family = AF_INET;
  addr->sin_addr.s_addr = INADDR_ANY;
  addr->sin_port = htons(port);
  int rc = bind(s, (struct sockaddr *)addr, sizeof(struct sockaddr_in));;
  if (rc < 0) {
    perror("error");
    exit(1);
  }
}

zdecl(server_listen,
  zrettype(int),
  zparams(int, port),
  int s;
  int ls;
  socklen_t len;
  struct sockaddr_in addr;)
zasync(server_listen, {
  z->ls = socket(AF_INET, SOCK_STREAM, 0);
  zown(z->ls);
  bind_localhost(z->ls, &z->addr, zarg(port));
  z->len = sizeof(z->addr);
  listen(z->ls, 10);
  while(1) {
    zaccept(z->s, z->ls, (struct sockaddr *)&z->addr, &z->len);
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
