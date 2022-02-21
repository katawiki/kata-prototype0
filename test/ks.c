/* test/ks.c - testing KataScript 
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/test.h>
#include <kata/ks.h>
#include <kata/vm.h>

int main(int argc, char** argv) {
    kinit(true);

    assert(kprintf(Kos_stdout, "ks.ast.name: %R\n", Ks_ast->name) >= 0);
    assert(kprintf(Kos_stdout, "ks.ast.docs: %R\n", Ks_ast->docs) >= 0);

    kstr filename = kstr_new(-1, "<test>");
    //kstr src = kstr_new(-1, "1 * 2 + a * b");
    //kstr src = kstr_new(-1, "1 + 2 + 3 + 4");
    kstr src = kstr_new(-1, "2 * 3 + 4 * 5");
    assert(kprintf(Kos_stdout, "filename: %R\n", filename) >= 0);
    assert(kprintf(Kos_stdout, "src: %R\n", src) >= 0);


    // parse the program
    s32 ntoks = 0;
    ks_tok* toks = NULL;
    ks_ast prog = ks_parse(filename, src, &ntoks, &toks);
    assert(prog != NULL);
    assert(kprintf(Kos_stdout, "ks_parse(filename, src):\n%R\n", prog) >= 0);

    kobj res = kvm_eval(NULL, NULL, prog);
    assert(res != NULL);
    assert(kprintf(Kos_stdout, "kvm_eval(prog): %R\n", res) >= 0);
    KOBJ_DECREF(res);

    KOBJ_DECREF(prog);
    
    KOBJ_DECREF(src);
    KOBJ_DECREF(filename); 

    usize i;
    for (i = 0; i < ntoks; ++i) {
        KOBJ_DECREF(toks[i]);
    }
    kmem_free(toks);

    return 0;
}

