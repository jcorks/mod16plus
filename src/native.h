#ifndef SES_NATIVE_INCLUDED
#define SES_NATIVE_INCLUDED

#include "matte/src/matte_vm.h"
#include "matte/src/matte_heap.h"


void ses_native_commit_rom();
int ses_native_main_loop(matte_t *);
// normally called for you
// returns whether to continue;
int ses_native_update(matte_t * m);



#endif
