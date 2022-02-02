/* test/str.c - testing 'kprintf'
 *
 * @author: Cade Brown <me@cade.site>
 */

#include <kata/test.h>


int main(int argc, char** argv) {
    kinit(true);

    kstr xyz = kstr_new(3, "xyz");
    assert(xyz != NULL);
    assert(xyz->lenb == 3);
    assert(xyz->lenc == 3);
    assert(strcmp(xyz->data, "xyz") == 0);
    assert(xyz->data[3] == 0x00);

    assert(kprintf(Ksys_stdout, "hello, world: %S\n", (kobj)xyz) >= 0);
    KOBJ_DECREF(xyz);

    return 0;
}

