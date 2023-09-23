#include <criterion/criterion.h>
#include <criterion/logging.h>

#include "watcher_table.h"
#include "str_link_list.h"
#include "ticker.h"

Test(table_suite, table_init_allocate_test) {
    watcher_table *table = table_init();
    cr_assert_not_null(table, "Table allocated incorrectly!");
}

Test(table_suite, table_free_test_1) {
    watcher_table *table = table_init();
    cr_assert_not_null(table, "Table allocated incorrectly!");
    destroy_table(&table);
    cr_assert_null(table, "Table was not successfully destroyed!");
}

Test(table_suite, table_insert_test_1) {
    watcher_table *table = table_init();
    cr_assert_not_null(table, "Table allocated incorrectly!");

    char *argv[] = {"hi", "hi", NULL};
    char *argc[] = {"bye", "bye", NULL};
    sll_t *new_l = init_sll();
    int i = 0;
    while(argc[i] != NULL) {
        insert_into_sll(new_l, argc[i], strlen(argc[i]));
        i++;
    }
    char **stra = sll_to_stra(new_l);

    add_new_entry(table, "joe", 12321, 12, 12, &watcher_types[0], false, argv, stra);
    new_l = init_sll();
    i = 0;
    while(argc[i] != NULL) {
        insert_into_sll(new_l, argc[i], strlen(argc[i]));
        i++;
    }
    stra = sll_to_stra(new_l);
    add_new_entry(table, "joe", 12321, 12, 12, &watcher_types[0], false, argv, stra);
    new_l = init_sll();
    i = 0;
    while(argc[i] != NULL) {
        insert_into_sll(new_l, argc[i], strlen(argc[i]));
        i++;
    }
    stra = sll_to_stra(new_l);
    add_new_entry(table, "joe", 12321, 12, 12, &watcher_types[0], false, argv, stra);
    new_l = init_sll();
    i = 0;
    while(argc[i] != NULL) {
        insert_into_sll(new_l, argc[i], strlen(argc[i]));
        i++;
    }
    stra = sll_to_stra(new_l);
    add_new_entry(table, "joe", 12321, 12, 12, &watcher_types[0], false, argv, stra);
    new_l = init_sll();
    i = 0;
    while(argc[i] != NULL) {
        insert_into_sll(new_l, argc[i], strlen(argc[i]));
        i++;
    }
    stra = sll_to_stra(new_l);
    add_new_entry(table, "joe", 12321, 12, 12, &watcher_types[0], false, argv, stra);

    cr_assert_eq(table->len, 5, "Size was not set correctly!");
    destroy_table(&table);
}

Test(table_suite, table_insert_test_2) {
    watcher_table *table = table_init();
    cr_assert_not_null(table, "Table allocated incorrectly!");

    char *argv[] = {"hi", "hi", NULL};
    char *argc[] = {"bye", "bye", NULL};

    for(int i = 0; i < 1024; ++i) {
        sll_t *new_l = init_sll();
        int j = 0;
        while(argc[j] != NULL) {
            insert_into_sll(new_l, argc[j], strlen(argc[j]));
            j++;
        }
        char **stra = sll_to_stra(new_l);
        add_new_entry(table, "joe", 12321, 12, 12, &watcher_types[0], false,argv, stra);
    }
    cr_assert_eq(table->len, 1024, "Size was not set correctly!");
    for(int i = 0; i < 1024; ++i) {
        cr_assert_eq(table->table_arr[i].idx, i, "Index was incorrectly set!");
        cr_assert_eq(table->table_arr[i].proc_conn_info.name, "joe", "Name was incorrectly set!");
    }
    destroy_table(&table);
}

Test(table_suite, table_remove_test_1) {
    watcher_table *table = table_init();
    cr_assert_not_null(table, "Table allocated incorrectly!");

    char *argv[] = {"hi", "hi", NULL};
    char *argc[] = {"bye", "bye", NULL};

    sll_t *new_l = init_sll();
    int i = 0;
    while(argc[i] != NULL) {
        insert_into_sll(new_l, argc[i], strlen(argc[i]));
        i++;
    }
    char **stra = sll_to_stra(new_l);

    add_new_entry(table, "joe", 12321, 12, 12, &watcher_types[0], false,argv, stra);
    new_l = init_sll();
    int j = 0;
    while(argc[j] != NULL) {
        insert_into_sll(new_l, argc[j], strlen(argc[j]));
        j++;
    }
    stra = sll_to_stra(new_l);
    add_new_entry(table, "joe", 12321, 12, 12,&watcher_types[0], false, argv, stra);
    new_l = init_sll();
    j = 0;
    while(argc[j] != NULL) {
        insert_into_sll(new_l, argc[j], strlen(argc[j]));
        j++;
    }
    stra = sll_to_stra(new_l);
    add_new_entry(table, "joe", 12321, 12, 12,&watcher_types[0], false, argv, stra);
    new_l = init_sll();
    j = 0;
    while(argc[j] != NULL) {
        insert_into_sll(new_l, argc[j], strlen(argc[j]));
        j++;
    }
    stra = sll_to_stra(new_l);
    add_new_entry(table, "joe", 12321, 12, 12,&watcher_types[0], false, argv, stra);
    new_l = init_sll();
    j = 0;
    while(argc[j] != NULL) {
        insert_into_sll(new_l, argc[j], strlen(argc[j]));
        j++;
    }
    stra = sll_to_stra(new_l);
    add_new_entry(table, "joe", 12321, 12, 12,&watcher_types[0], false, argv, stra);
    print_watcher_table(table);

    cr_assert_eq(table->len, 5, "Size was not set correctly!");

    remove_entry(table, 2);
    remove_entry(table, 2);
    remove_entry(table, 2);

    cr_assert_eq(table->len, 4, "Size was not set correctly!");

    remove_entry(table, 0);

    cr_assert_eq(table->len, 3, "Size was not set correctly!");
    destroy_table(&table);
}

Test(table_suite, table_remove_test_2) {
    watcher_table *table = table_init();
    cr_assert_not_null(table, "Table allocated incorrectly!");

    char *argv[] = {"hi", "hi", NULL};
    char *argc[] = {"bye", "bye", NULL};

    for(int i = 0; i < 1024; ++i) {
        sll_t *new_l = init_sll();
        int j = 0;
        while(argc[j] != NULL) {
            insert_into_sll(new_l, argc[j], strlen(argc[j]));
            j++;
        }
        char **stra = sll_to_stra(new_l);
        add_new_entry(table, "joe", 12321, 12, 12, &watcher_types[0], false,argv, stra);
    }
    cr_assert_eq(table->len, 1024, "Size was not set correctly!");

    remove_entry(table, 605);
    remove_entry(table, 605);
    remove_entry(table, 605);

    cr_assert_eq(table->table_arr[605].idx, 0, "wasn't successfully removed!");
    cr_assert_eq(table->table_arr[605].argv, 0, "wasn't successfully removed!");
    cr_assert_eq(table->len, 1023, "Size was not set correctly!");
    destroy_table(&table);
}
