#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{
        if (argc != 2) {
                fprintf(stderr, "Usage: %s <string>\n", argv[0]);
                return 1;
        }
        char cpy[60];

        strcpy(cpy, argv[1]);

        int c = 0;
        for(i = 0; str[i] != '\0'; i++)
        {
                if(str[i] == ',')
                        ++c;
        }

        if (c != 3) {
                fprintf(stderr, "Invalid input format. Expected: %s\n", cpy);
                return 1;
        }



        char *input = argv[1];
        char *command = strtok(input, ",");
        char *arg_1 = strtok(NULL, ",");
        char *arg_2 = strtok(NULL, ",");
        char *arg_3 = strtok(NULL, ",");

        if (command == NULL || arg_1 == NULL || arg_2 == NULL || arg_3 == NULL) {
                fprintf(stderr, "Invalid input format. Expected: %s\n", cpy);
                return 1;
        }

        printf("Command: %s\n", command);
        printf("Argument 1: %s\n", arg_1);
        printf("Argument 2: %s\n", arg_2);
        printf("Argument 3: %s\n", arg_3);

        return 0;
}