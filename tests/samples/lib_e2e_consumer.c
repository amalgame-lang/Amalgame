#include <stdio.h>
#include "_runtime.h"

typedef struct _E2ELib_Calc E2ELib_Calc;
extern E2ELib_Calc* E2ELib_Calc_new(i64 b);
extern i64 E2ELib_Calc_Add(E2ELib_Calc* self, i64 x);
extern i64 E2ELib_Calc_Mul(E2ELib_Calc* self, i64 x);

int main(void) {
    code_runtime_init();
    E2ELib_Calc* c = E2ELib_Calc_new(10);
    printf("add=%lld mul=%lld\n",
           (long long)E2ELib_Calc_Add(c, 5),
           (long long)E2ELib_Calc_Mul(c, 3));
    return 0;
}
