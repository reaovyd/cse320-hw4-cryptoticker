#include <criterion/criterion.h>
#include <criterion/logging.h>

#include "parser.h"

Test(parser_tests, parser_tests_1) {
    char test_arr[] = "la la la la\n";
    int t1 = is_valid_input(test_arr);  
    cr_assert_eq(t1, -1, "Expected invalid input!");
}

Test(parser_tests, parser_tests_2) {
    char test_arr[] = "stop 1\n";
    int t1 = is_valid_input(test_arr);  
    cr_assert_eq(t1, 3, "Bad input!");
}


Test(parser_tests, parser_tests_3) {
    char test_arr[] = "stop start stop stop start\n";
    int t1 = is_valid_input(test_arr);  
    cr_assert_eq(t1, -1, "Bad input!");
}


Test(parser_tests, parser_tests_4) {
    char test_arr[] = "\n";
    int t1 = is_valid_input(test_arr);  
    cr_assert_eq(t1, -1, "Bad input!");
}

Test(parser_tests, parser_tests_5) {
    char test_arr[] = "quit\n";
    int t1 = is_valid_input(test_arr);  
    cr_assert_eq(t1, 1, "Bad input!");
}

Test(parser_tests, parser_tests_6) {
    char test_arr[] = "watchers\n";
    int t1 = is_valid_input(test_arr);  
    cr_assert_eq(t1, 2, "Bad input!");
}

Test(parser_tests, parser_tests_7) {
    char test_arr[] = "trace 2\n";
    int t1 = is_valid_input(test_arr);  
    cr_assert_eq(t1, 4, "Bad input!");
}

Test(parser_tests, parser_tests_8) {
    char test_arr[] = "untrace 2\n";
    int t1 = is_valid_input(test_arr);  
    cr_assert_eq(t1, 5, "Bad input!");
}

Test(parser_tests, parser_tests_9) {
    char test_arr[] = "show bitcoin\n";
    int t1 = is_valid_input(test_arr);  
    cr_assert_eq(t1, 6, "Bad input! Got %d", t1);
}

Test(parser_tests, parser_tests_10) {
    //is_valid_server_msg(char *txt);
    char test_arr[] = "Server message: joenuts\n";
    cr_assert_null(is_valid_server_msg(test_arr));
}

Test(parser_tests, parser_tests_11) {
    //is_valid_server_msg(char *txt);
    char test_arr[] = "joe";
    cr_assert_null(is_valid_server_msg(test_arr));
}

Test(parser_tests, parser_tests_12) {
    //is_valid_server_msg(char *txt);
    char test_arr[] = "Server joe:";
    cr_assert_null(is_valid_server_msg(test_arr));
}

Test(parser_tests, parser_tests_13) {
    //is_valid_server_msg(char *txt);
    char test_arr[] = "Server message: '{}'\n";
    cr_assert_null(is_valid_server_msg(test_arr));
}
