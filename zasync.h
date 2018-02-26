#ifndef ZASYNC_H
#define ZASYNC_H

#include <fcntl.h>
#include <ev.h>
#include <errno.h>
#include <stdlib.h>

struct zio_s {
  ev_io io;
  struct zio_s* next;
};

typedef struct {
  int fd;
  int ev_type;
} zyield_t;

typedef struct {
  void* childstack;
  zyield_t waiting_for;
  int state;
  struct zio_s* io;
  void(*cb)(struct ev_loop*, ev_io*, int);
  void(*sem_cb)(void*);
} zstate_t;

typedef struct {
  char count;
  void* stack;
} zsemaphore;

#define ZNO_FD_OP -2
#define ZSEM_EV 1 

#define zdecl(name, ret, params, rest) \
  struct name##_stack_s { \
    zstate_t zz; \
    union { \
      params \
    } p; \
    ret \
    rest \
  }; \
  zyield_t name(struct name##_stack_s* z); \
  void name##_io_cb(EV_P_ ev_io *w, int revents); \
  void name##_sem_cb(void*);

#define zcallback(name, stack, fdes, ev_check) \
  zyield_t wf = stack->zz.waiting_for; \
  if (wf.fd == fdes && ev_check) { \
    zyield_t r = name(stack); \
    stack->zz.waiting_for = r; \
    if (r.fd == -1) { \
      zclean((void*)stack); \
    } \
  }

#define zasync(name, block) \
  zyield_t name(struct name##_stack_s* z) { \
    switch (z->zz.state) { \
    case 0: \
      block \
    } \
    zyield_t r; \
    r.fd = -1; \
    return r; \
  } \
  void name##_io_cb(EV_P_ ev_io *w, int revents) { \
    struct name##_stack_s* stack = (struct name##_stack_s*)w->data; \
    zcallback(name, stack, w->fd, revents & stack->zz.waiting_for.ev_type) \
  } \
  void name##_sem_cb(void* data) { \
    struct name##_stack_s* stack = (struct name##_stack_s*)data; \
    zcallback(name, stack, ZNO_FD_OP, stack->zz.waiting_for.ev_type == ZSEM_EV) \
  }

#define zyield(fdes, ev, code) \
  case __LINE__: \
    { code } \
    if (errno == EWOULDBLOCK) { \
      errno = 0; \
      z->zz.state = __LINE__; \
      zyield_t r; \
      r.ev_type = ev; \
      r.fd = fdes; \
      return r; \
    }

#define zreturn(value) \
{ \
  z->zretv = value; \
  zyield_t r; \
  r.fd = -1; \
  return r; \
}

#define zawait(name, ret, ...) \
  { \
    struct name##_stack_s* childstack = malloc(sizeof(struct name##_stack_s)); \
    childstack->zz.state = 0; \
    zassignparams(childstack, __VA_ARGS__) \
    z->zz.childstack = (void*)childstack; \
  } \
  case __LINE__: \
  { \
    struct name##_stack_s* childstack = (struct name##_stack_s*)z->zz.childstack; \
    zyield_t r = name(childstack); \
    if (r.fd != -1) { \
      z->zz.state = __LINE__; \
      return r; \
    } \
    z->ret = childstack->zretv; \
    free(childstack); \
  }

#define zrettype(type) \
  type zretv;

#define _zparams(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, NAME, ...) NAME

#define zparams(...) _zparams(__VA_ARGS__, \
    zparams5, _err, zparams4, _err, zparams3, _err, zparams2, _err, zparams1)(__VA_ARGS__)

#define zparams1(pt1, pn1) \
  struct { pt1 pn1; }; \
  struct { pt1 p1; }; \

#define zparams2(pt1, pn1, pt2, pn2) \
  struct { pt1 pn1; pt2 pn2; }; \
  struct { pt1 p1; pt2 p2; }; \

#define zparams3(pt1, pn1, pt2, pn2, pt3, pn3) \
  struct { pt1 pn1; pt2 pn2; pt3 pn3; }; \
  struct { pt1 p1; pt2 p2; pt3 p3; }; \

#define zparams4(pt1, pn1, pt2, pn2, pt3, pn3, pt4, pn4) \
  struct { pt1 pn1; pt2 pn2; pt3 pn3; pt4 pn4; }; \
  struct { pt1 p1; pt2 p2; pt3 p3; pt4 p4; }; \

#define zparams5(p1, p2, p3, p4, p5) \
  struct { pt1 pn1; pt2 pn2; pt3 pn3; pt4 pn4; pt5 pn5; }; \
  struct { pt1 p1; pt2 p2; pt3 p3; pt4 p4; pt5 p5; }; \

#define zarg(name) \
  z->p.name

#define _zassignparams(_1, _2, _3, _4, _5, NAME, ...) NAME

#define zassignparams(name, ...) _zassignparams(__VA_ARGS__, \
  zassignparams5, zassignparams4, zassignparams3, zassignparams2, zassignparams1)(name, __VA_ARGS__)

#define zassignparams1(name, pn1) \
  name->p.p1 = pn1;

#define zassignparams2(name, pn1, pn2) \
  name->p.p1 = pn1; \
  name->p.p2 = pn2;

#define zassignparams3(name, pn1, pn2, pn3) \
  name->p.p1 = pn1; \
  name->p.p2 = pn2; \
  name->p.p3 = pn3;

#define zassignparams4(name, pn1, pn2, pn3, pn4) \
  name->p.p1 = pn1; \
  name->p.p2 = pn2; \
  name->p.p3 = pn3; \
  name->p.p4 = pn4;

#define zassignparams5(name, pn1, pn2, pn3, pn4, pn5) \
  name->p.p1 = pn1; \
  name->p.p2 = pn2; \
  name->p.p3 = pn3; \
  name->p.p4 = pn4; \
  name->p.p5 = pn5;

#define zspawn(name, ...) \
{ \
  struct name##_stack_s* zstack = malloc(sizeof(struct name##_stack_s)); \
  memset(&zstack->zz, 0, sizeof(zstack->zz)); \
  zassignparams(zstack, __VA_ARGS__) \
  zstack->zz.cb = name##_io_cb; \
  zstack->zz.sem_cb = name##_sem_cb; \
  zstack->zz.waiting_for = name(zstack); \
}

#define zown(fd) \
  zownfn(fd, &z->zz, (void*)z);

#define zread(ret, fd, buf, size) \
  zyield(fd, EV_READ, ret = read(fd, buf, size);)

#define zwrite(ret, fd, buf, size) \
  zyield(fd, EV_WRITE, ret = write(fd, buf, size);)

#define zaccept(ret, fd, addr, addrlen) \
  zyield(fd, EV_READ, ret = accept(fd, addr, addrlen);)

#define zwait(sem) \
  case __LINE__: \
  if (sem->count > 0) { \
    sem->count--; \
    sem->stack = NULL; \
  } else { \
    sem->stack = (void*)z; \
    z->zz.state = __LINE__; \
    zyield_t r; \
    r.fd = ZNO_FD_OP; \
    r.ev_type = ZSEM_EV; \
    return r; \
  } \

#define zsignal(sem) \
{ \
  sem->count++; \
  zstate_t* state = (zstate_t*)sem->stack; \
  if (state) { \
    state->sem_cb(sem->stack); \
  } \
} \

void zownfn(int fd, zstate_t* zz, void*);
void zclean(void* stack);
void zinit();
void zrun();
extern struct ev_loop *zloop;

#endif

#ifdef ZASYNC_IMPL

struct ev_loop *zloop;

void zinit() {
  zloop = EV_DEFAULT;
}

void zrun() {
  ev_run(zloop, 0);
}

void zownfn(int fd, zstate_t* zz, void* data) {
  struct zio_s* w = malloc(sizeof(struct zio_s));
  w->io.data = data;
  struct zio_s* head = zz->io;
  zz->io = w;
  w->next = head;
  int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
  ev_io_init((ev_io*)w, zz->cb, fd, EV_READ | EV_WRITE);
  ev_io_start(zloop, (ev_io*)w);
}

void zclean(void* stack) {
  //XXX free/stop all watchers
  //ev_io_stop(zloop, w);
  free(stack);
}

#endif
