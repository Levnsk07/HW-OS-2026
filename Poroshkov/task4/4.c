#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <errno.h>

typedef struct Node {
    char *str;
    struct Node *next;
} Node;

static struct termios saved;
static volatile sig_atomic_t terminal_active = 0;
static int echo_input = 0;

static void restore_terminal(void) {
    if (terminal_active) {
        tcsetattr(STDIN_FILENO, TCSANOW, &saved);
        terminal_active = 0;
    }
}

static void stop_handler(int sig) {
    restore_terminal();
    _exit(128 + sig);
}

static int prepare_input(void) {
    if (!isatty(STDIN_FILENO))
        return 0;
    if (tcgetattr(STDIN_FILENO, &saved) == -1)
        return -1;
    if (atexit(restore_terminal) != 0)
        return -1;
    struct sigaction sa;
    sa.sa_handler = stop_handler;
    sigfillset(&sa.sa_mask);
    sa.sa_flags = 0;
    int signals[] = {SIGINT, SIGQUIT, SIGTERM, SIGHUP};
    for (size_t i = 0; i < sizeof(signals) / sizeof(signals[0]); ++i) {
        if (sigaction(signals[i], &sa, NULL) == -1)
            return -1;
    }
    struct termios mode = saved;
    mode.c_lflag &= ~(ICANON | ECHO | ECHONL | IEXTEN);
    mode.c_cc[VMIN] = 1;
    mode.c_cc[VTIME] = 0;
    terminal_active = 1;
    if (tcsetattr(STDIN_FILENO, TCSANOW, &mode) == -1)
        return -1;
    echo_input = isatty(STDERR_FILENO);
    return 0;
}

static int read_key(void) {
    int state = 0;
    for (;;) {
        int c = getchar();
        if (c == EOF)
            return EOF;
        if (state == 2) {
            if (c >= 32 && c < 127) {
                if (c >= '@' && c <= '~' && c != '[')
                    state = 0;
                continue;
            }
            state = 0;
        }
        if (state == 1) {
            state = 0;
            if (c == '[' || c == 'O') {
                state = 2;
                continue;
            }
        }
        if (c == 27) {
            state = 1;
            continue;
        }
        return c;
    }
}

static void erase_last(char *line, size_t *length) {
    if (*length == 0)
        return;
    --*length;
    while (*length > 0 && ((unsigned char)line[*length] & 0xc0) == 0x80)
        --*length;
    size_t width = 1;
    if (line[*length] == '\t') {
        size_t column = 0;
        for (size_t i = 0; i < *length; ++i) {
            if (line[i] == '\t')
                column += 8 - column % 8;
            else if (((unsigned char)line[i] & 0xc0) != 0x80)
                ++column;
        }
        width = 8 - column % 8;
    }
    line[*length] = '\0';
    if (echo_input) {
        while (width-- > 0)
            fputs("\b \b", stderr);
    }
}

static int read_line(char **line) {
    size_t length = 0, capacity = 128;
    *line = malloc(capacity);
    if (*line == NULL)
        return -1;
    (*line)[0] = '\0';
    for (;;) {
        int c = read_key();
        if (c == EOF || (terminal_active && c == saved.c_cc[VEOF])) {
            if (c == EOF && ferror(stdin))
                goto error;
            if (length == 0) {
                free(*line);
                *line = NULL;
                return 0;
            }
            break;
        }
        if (c == '\n' || (terminal_active && c == '\r'))
            break;
        if (c == '\b' || c == 127 || (terminal_active && c == saved.c_cc[VERASE])) {
            erase_last(*line, &length);
            continue;
        }
        if (c == 21 || (terminal_active && c == saved.c_cc[VKILL])) {
            while (length > 0)
                erase_last(*line, &length);
            continue;
        }
        if (c < 32 && c != '\t')
            continue;
        if (length + 1 == capacity) {
            if (capacity > SIZE_MAX / 2) {
                errno = ENOMEM;
                goto error;
            }
            char *grown = realloc(*line, capacity * 2);
            if (grown == NULL)
                goto error;
            *line = grown;
            capacity *= 2;
        }
        (*line)[length++] = (char)c;
        (*line)[length] = '\0';
        if (echo_input)
            fputc(c, stderr);
    }
    if (echo_input)
        fputc('\n', stderr);
    return 1;
error:
    free(*line);
    *line = NULL;
    return -1;
}

static void free_list(Node *head) {
    while (head != NULL) {
        Node *next = head->next;
        free(head->str);
        free(head);
        head = next;
    }
}

int main(void) {
    if (prepare_input() == -1) {
        perror("Настройка ввода");
        return EXIT_FAILURE;
    }
    Node *head = NULL, *tail = NULL;
    char *line;
    int result;
    while ((result = read_line(&line)) == 1) {
        if (line[0] == '.') {
            free(line);
            break;
        }
        Node *node = malloc(sizeof(*node));
        if (node == NULL) {
            perror("malloc");
            free(line);
            free_list(head);
            return EXIT_FAILURE;
        }
        node->str = line;
        node->next = NULL;
        if (tail == NULL)
            head = node;
        else
            tail->next = node;
        tail = node;
    }
    restore_terminal();
    if (result == -1) {
        perror("Чтение строки");
        free_list(head);
        return EXIT_FAILURE;
    }
    for (Node *cur = head; cur != NULL; cur = cur->next)
        puts(cur->str);
    free_list(head);
    return fflush(stdout) == EOF ? EXIT_FAILURE : EXIT_SUCCESS;
}
