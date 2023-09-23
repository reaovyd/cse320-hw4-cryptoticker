#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <string.h>

#include "str_link_list.h"

Test(sll_suite, sll_test_1) {
    sll_t *new_sll = init_sll();
    cr_assert_not_null(new_sll, "Found NULL!");
}

Test(sll_suite, sll_test_2) {
    sll_t *new_sll = init_sll();
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    print_lst(new_sll);
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    char **new_stra = sll_to_stra(new_sll);
    char **to_free_stra = new_stra;
    while(*new_stra != NULL) {
        cr_assert_eq(strcmp(*new_stra, "joenuts") == 0, true, "WRong joe: %s", *new_stra);
        new_stra++;
    }
    destroy_sll_stra(to_free_stra);
    cr_assert_eq(new_sll->len, 4, "WRong len!");
}

Test(sll_suite, sll_test_3) {
    sll_t *new_sll = init_sll();
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    print_lst(new_sll);
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    print_lst(new_sll);
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    cr_assert_eq(new_sll->head->token->len, strlen("joenuts"), "WRong len!");
}

Test(sll_suite, sll_test_4) {
    sll_t *new_sll = init_sll();
    destroy_sll(&new_sll);
    cr_assert_null(new_sll, "wasn't properly delted!");
}

Test(sll_suite, sll_test_5) {
    sll_t *new_sll = init_sll();
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    insert_into_sll(new_sll, "joenuts", strlen("joenuts"));
    print_lst(new_sll);
    destroy_sll(&new_sll);
    cr_assert_null(new_sll, "wasn't properly delted!");
}
