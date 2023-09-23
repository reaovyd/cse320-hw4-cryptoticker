#ifndef DEQUE_READER_H
#define DEQUE_READER_H

typedef struct deque_reader {
    char *buf;
    size_t len;
    size_t cap;
    int input_ready;
} deque_reader;

// set to probably a static
extern void dr_write_char(deque_reader *dr, char c);

// user must ensure to free if successful
extern char *dr_get_avail_input(deque_reader *dr);
extern deque_reader *dr_init();
extern void dr_destroy(deque_reader **dr);

#endif /* DEQUE_READER_H */
