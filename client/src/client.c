#include "../include/client.h"

command commands[] = {
        {"len", "returns string size", len},
        {"capitalize", "returns capitalized string", capitalize},
        {"upper", "returns uppercased string", to_uppercase},
        {"lower", "returns lowercased string", to_lowercase},
        {NULL, NULL, NULL}
};

void len(char *str)
{
        printf("%ld\n", strlen(str));
}

void capitalize(char *str)
{
        for (int i = 0; str[i]; i++)
                if (str[i] >= 'A' && str[i] <= 'Z') 
                        str[i] += 'a' - 'A';
        str[0] -= 'a' - 'A';
        printf("%s\n", str);
}

void to_uppercase(char *str)
{
        for (int i = 0; str[i]; i++)
                if (str[i] >= 'a' && str[i] <= 'z') 
                        str[i] -= 'a' - 'A';
        printf("%s\n", str);
}

void to_lowercase(char *str)
{
        for (int i = 0; str[i]; i++)
                if (str[i] >= 'A' && str[i] <= 'Z') 
                        str[i] += 'a' - 'A';
        printf("%s\n", str);
}


bool validate(char *otp)
{
        int fd = open("/dev/otp-validator", O_RDWR);
        if (fd < 0) {
                perror("Failed to open\n");
                return false;
        }

        if (write(fd, otp, sizeof(otp)) < 0) {
                perror("Failed to write\n");
                close(fd);
                return false;
        }

        char result[32];
        read(fd, &result, sizeof(result));
        close(fd);
        int code = atoi(result);
        if (!code) {
                fprintf(stderr, "Invalid otp\n");
                return false;
        }
        return true;
}

void print_usage()
{
        fprintf(stderr, "Usage:\n");
        fprintf(stderr, "\tclient <otp> <cmd> <str>\n");
        fprintf(stderr, "\totp: Valid otp generated using OTP Manager Module\n");
        fprintf(stderr, "\tcmd: Command to do\n");
        fprintf(stderr, "\tstr: String that will be modified by the command\n");
        fprintf(stderr, "\n\tCommand list:\n");
        for (int i = 0; commands[i].name != NULL; i++)
                fprintf(stderr, "\t\t%s: %s\n", commands[i].name, commands[i].desc);
}

void execute_command(char *cmd, char *str)
{
        for (int i = 0; commands[i].name != NULL; i++) {
                if (strcmp(commands[i].name, cmd) == 0) {
                        commands[i].function(str);
                        return;
                }
        }
        fprintf(stderr, "Invalid command\n");
        print_usage();
}

int main(int argc, char *argv[])
{
        if (argc != 4) {
                print_usage();
                return 1;
        }

        if (!validate(argv[1]))
                return 2;
        execute_command(argv[2], argv[3]);
        // char cpy[60];

        // strcpy(cpy, argv[1]);

        // int c = 0;
        // for(i = 0; str[i] != '\0'; i++)
        // {
        //         if(str[i] == ',')
        //                 ++c;
        // }

        // if (c != 3) {
        //         fprintf(stderr, "Invalid input format. Expected: %s\n", cpy);
        //         return 1;
        // }



        // char *input = argv[1];
        // char *command = strtok(input, ",");
        // char *arg_1 = strtok(NULL, ",");
        // char *arg_2 = strtok(NULL, ",");
        // char *arg_3 = strtok(NULL, ",");

        // if (command == NULL || arg_1 == NULL || arg_2 == NULL || arg_3 == NULL) {
        //         fprintf(stderr, "Invalid input format. Expected: %s\n", cpy);
        //         return 1;
        // }

        // printf("Command: %s\n", command);
        // printf("Argument 1: %s\n", arg_1);
        // printf("Argument 2: %s\n", arg_2);
        // printf("Argument 3: %s\n", arg_3);

        return 0;
}