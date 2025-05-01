#ifndef CLIENT_H
        #define CLIENT_H

        #include <stdio.h>
        #include <string.h>
        #include <stdlib.h>
        #include <sys/types.h>
        #include <sys/stat.h>
        #include <fcntl.h>
        #include <unistd.h>
        #include <stdbool.h>

        typedef struct {
                const char *name;
                const char *desc;
                void (*function)(char *);
        } command;

        void len(char *str);
        void capitalize(char *str);
        void to_uppercase(char *str);
        void to_lowercase(char *str);
#endif /* !CLIENT_H */