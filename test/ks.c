/* test/ks.c - testing KataScript 
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/test.h>
#include <kata/ks.h>

int main(int argc, char** argv) {
    kinit(true);

    assert(kprintf(Kos_stdout, "ks.ast.name: %R\n", Ks_ast->name) >= 0);
    assert(kprintf(Kos_stdout, "ks.ast.docs: %R\n", Ks_ast->docs) >= 0);

    kstr filename = kstr_new(-1, "<test>");
    kstr src = kstr_new(-1, "1 + 2");
    assert(kprintf(Kos_stdout, "filename: %R\n", filename) >= 0);
    assert(kprintf(Kos_stdout, "src: %R\n", src) >= 0);

    KOBJ_DECREF(src);
    KOBJ_DECREF(filename);
    return 0;

    // parse the program
    ks_ast prog = ks_parse(filename, src, -1, NULL);
    assert(kprintf(Kos_stdout, "ks_parse(filename, src):\n%R\n", prog) >= 0);

    KOBJ_DECREF(prog);
    

    return 0;
}

