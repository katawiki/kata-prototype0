/* kata/vm.h - KataVM, the virtual machine interface for running code
 *
 *
 *
 * @author: Cade Brown <me@cade.site>
 */

#pragma once
#ifndef KATA_VM_H
#define KATA_VM_H

#ifndef KATA_API_H
  #include <kata/api.h>
#endif

////////////////////////////////////////////////////////////////////////////////

// KataVM - a virtual machine type for parsing, optimizing, compiling, and executing
//            kata code, modules, and packages
//
// this is basically a structure that manages program state and internal structures
typedef struct kvm {




}* kvm;


// compute 'fn(*args)' and return it
KATA_API kobj
kvm_call(struct kvm* vm, kobj fn, usize nargs, kobj* vargs);

// evaluates 'node' (which can be an AST or value), in a given scope/context
KATA_API kobj
kvm_eval(struct kvm* vm, kobj scope, kobj node);


////////////////////////////////////////////////////////////////////////////////

KATA_API ktype
Kvm
;

#endif // KATA_KS_H
