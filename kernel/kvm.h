#ifndef __KVM_H_
#define __KVM_H_

#include "pte.h"
#include "pg.h"


void kvm_init(pte_t *kp);
void kvm_install(pte_t *kp);





#endif // __KVM_H_
