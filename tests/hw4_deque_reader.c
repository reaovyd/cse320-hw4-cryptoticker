#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <string.h>

#include "deque_reader.h"

Test(deque_reader_suite, deque_reader_1) {
    deque_reader *dr = dr_init(); 
    cr_assert_not_null(dr);
    cr_assert_eq(dr->len, 0);
    cr_assert_eq(dr->cap, 4096);
    cr_assert_not_null(dr->buf);
    cr_assert_eq(dr->input_ready, 0);
}

Test(deque_reader_suite, deque_reader_2) {
    deque_reader *dr = dr_init(); 
    for(int i = 0; i < 8192; ++i) {
        dr_write_char(dr, 'a');
    }

    for(int i = 0; i < 8192; ++i) {
        cr_assert_eq(dr->buf[i], 'a');
    }
}

Test(deque_reader_suite, deque_reader_3) {
    deque_reader *dr = dr_init(); 
    char *input = dr_get_avail_input(dr);

    cr_assert_null(input);
}

Test(deque_reader_suite, deque_reader_4) {
    deque_reader *dr = dr_init(); 
    dr_write_char(dr, 'c');
    dr_write_char(dr, 'o');
    dr_write_char(dr, 'r');
    dr_write_char(dr, 'r');
    dr_write_char(dr, 'e');
    dr_write_char(dr, 's');
    dr_write_char(dr, 'p');
    dr_write_char(dr, 'o');
    dr_write_char(dr, '\n');
    char *input = dr_get_avail_input(dr);

    cr_assert_eq(strcmp(input, "correspo\n") == 0, 1);

    dr_write_char(dr, 'j');
    dr_write_char(dr, 'o');
    dr_write_char(dr, 'e');
    dr_write_char(dr, 'b');
    dr_write_char(dr, 'r');
    dr_write_char(dr, 'o');
    dr_write_char(dr, 'f');
    dr_write_char(dr, 'l');
    dr_write_char(dr, '\n');

    input = dr_get_avail_input(dr);
    cr_assert_eq(strcmp(input, "joebrofl\n") == 0, 1);

    dr_write_char(dr, 'j');
    dr_write_char(dr, 'o');
    dr_write_char(dr, 'e');
    dr_write_char(dr, 'b');

    input = dr_get_avail_input(dr);
    cr_assert_null(input);
}

Test(deque_reader_suite, deque_reader_5) {
    deque_reader *dr = dr_init(); 
    dr_write_char(dr, 'c');
    dr_write_char(dr, 'o');
    dr_write_char(dr, 'r');
    dr_write_char(dr, 'r');
    dr_write_char(dr, 'e');
    dr_write_char(dr, 's');
    dr_write_char(dr, 'p');
    dr_write_char(dr, 'o');
    dr_write_char(dr, '\n');
    char *input = dr_get_avail_input(dr);

    cr_assert_eq(strcmp(input, "correspo\n") == 0, 1);

    dr_write_char(dr, 'j');
    dr_write_char(dr, 'o');
    dr_write_char(dr, 'e');
    dr_write_char(dr, 'b');
    dr_write_char(dr, 'r');
    dr_write_char(dr, 'o');
    dr_write_char(dr, 'f');
    dr_write_char(dr, 'l');
    dr_write_char(dr, '\n');

    input = dr_get_avail_input(dr);
    cr_assert_eq(strcmp(input, "joebrofl\n") == 0, 1);

    dr_write_char(dr, 'j');
    dr_write_char(dr, 'o');
    dr_write_char(dr, 'e');
    dr_write_char(dr, 'b');

    input = dr_get_avail_input(dr);
    cr_assert_null(input);

    dr_destroy(&dr);

    cr_assert_null(dr);
}
