#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

char *history[80];
int his_count = 0;
pid_t current_child = -1;


void add_hist(char *cmd) {
    if (strlen(cmd) == 0) {
        return;
    }
    if (his_count == 80) {
        free(history[0]);
        int a = 1;
        while (a < 80) {
            history[a - 1] = history[a];
            a++;
        }
        his_count--;
    }
    history[his_count++] = strdup(cmd);
}

void show_hist() {
    printf("Histories:\n");
    int a = 0;
    while (a < his_count) {
        printf("%d: %s\n", a + 1, history[a]);
        a++;
    }
}


char *read_inp() {
        char *inp = NULL;
        size_t buffer = 0;

        while (1) {
                ssize_t input = getline(&inp, &buffer, stdin);
                if (input == -1) {
                        if (feof(stdin)) {
                                printf("exit\n");
                                exit(EXIT_SUCCESS);
                        } else if (errno == EINTR) {
                                clearerr(stdin);  
                                free(inp);
                                inp = NULL;
                                buffer = 0;
                                printf("\n%s", "Shell_Input>");
                                fflush(stdout);
                                continue;
                        } else {
                                perror("Input reading issue");
                                exit(EXIT_FAILURE);
                        }
                }

                size_t lengthOfInp = strlen(inp);
                if (lengthOfInp > 0 && inp[lengthOfInp - 1] == '\n') {
                        inp[lengthOfInp - 1] = '\0';
                }

                return inp;
        }
}



void signal_handle(int sig) {
        if (sig == SIGINT) {
                if (current_child > 0) {
                        kill(current_child, SIGINT);
                } else {
                        printf("\n%s", "Shell_Input");
                        fflush(stdout);
                }
        }
}

void handlers() {
        struct sigaction s;
        s.sa_handler = signal_handle;
        sigemptyset(&s.sa_mask);
        s.sa_flags = 0;
        sigaction(SIGINT, &s, NULL);
}


char **inpTokenise(char *inp, const char *sep) {
        char *token;
        int pos = 0;
        token = strtok(inp, sep);
        char **tokenMem = malloc(90 * sizeof(char*));
        if (!tokenMem) {
                perror("malloc function issue");
                exit(EXIT_FAILURE);
        }
        while (token != NULL) {
                tokenMem[pos] = strdup(token);
                pos++;
                if (pos >= 100) {
                        fprintf(stderr, "You have given too many tokens\n");
                        exit(EXIT_FAILURE);
                }
                token = strtok(NULL, sep);
        }
        tokenMem[pos] = NULL;
        return tokenMem;
}

void free_alloc_tokens(char **t) {
        for (int i = 0; t[i] != NULL; i++) {
                free(t[i]);
        }
        free(t);
}

int num_of_tokens(char **tokens) {
        int c = 0;
        while (tokens[c] != NULL) {
                c++;
        }
        return c;
}

int exec_cmd(char **arguments) {
        if (arguments[0] == NULL) {
                return -1;
        }

        if (strcmp(arguments[0], "exit") == 0) {
                return 0;
        } else if (strcmp(arguments[0], "cd") == 0) {
                if (arguments[1] == NULL) {
                        fprintf(stderr, "Expected argument to \"cd\"\n");
                } else {
                        if (chdir(arguments[1]) != 0) {
                                perror("cd error");
                        }
                }
                return 1;
        } else if (strcmp(arguments[0], "history") == 0) {
                show_hist();
                return 1;
        }

        pid_t pid = fork();

        if (pid == 0) {
                if (execvp(arguments[0], arguments) == -1) {
                        perror("Could Not execute command");
                        exit(EXIT_FAILURE);
                }
        } else if (pid < 0) {
                perror("Could Not Fork");
        } else {
                current_child = pid;
                int status;
                waitpid(pid, &status, 0);
                current_child = -1;

                if (WIFEXITED(status)) {
                        return WEXITSTATUS(status) == 0 ? 1 : -1;
                } else {
                        return -1;
                }
        }

        return 1;
}

int piping_commands(char ***cmds, int n) {
    int pipes[n - 1][2];
    pid_t pids[n];
    int status;

    int a = 0;
    while (a < n - 1) {
        if (pipe(pipes[a]) == -1) {
            perror("Pipe failed");
            return 1;
        }
        a++;
    }

    a = 0;
    while (a < n) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("Fork failed");
            return -1;
        } else if (pid == 0) {
            if (a > 0) {
                if (dup2(pipes[a - 1][0], STDIN_FILENO) == -1) {
                    perror("dup2 failed");
                    exit(EXIT_FAILURE);
                }
            }
            if (a < n - 1) {
                if (dup2(pipes[a][1], STDOUT_FILENO) == -1) {
                    perror("dup2 failed ");
                    exit(EXIT_FAILURE);
                }
            }

            int b = 0;
            while (b < n - 1) {
                close(pipes[b][0]);
                close(pipes[b][1]);
                b++;
            }

            if (execvp(cmds[a][0], cmds[a]) == -1) {
                perror("Could Not execute command");
                exit(EXIT_FAILURE);
            }
        } else {
            pids[a] = pid;
        }
        a++;
    }

    a = 0;
    while (a < n - 1) {
        close(pipes[a][0]);
        close(pipes[a][1]);
        a++;
    }

    a = 0;
    while (a < n) {
        waitpid(pids[a], &status, 0);
        a++;
    }

    return 1;
}

void redirection(char **arguments) {
        int i = 0;
        int fd_in = -1, fd_out = -1;
        int append = 0;
        char **new_args = malloc(100 * sizeof(char*));
        int new_i = 0;

        while (arguments[i] != NULL) {
                if (strcmp(arguments[i], "<") == 0) {
                        if (arguments[i+1] == NULL) {
                                fprintf(stderr, "No input file specified\n");
                                free(new_args);
                                return;
                        }
                        fd_in = open(arguments[i+1], O_RDONLY);
                        if (fd_in == -1) {
                                perror("Input redirection failed");
                                free(new_args);
                                return;
                        }
                        i += 2;
                } else if (strcmp(arguments[i], ">") == 0) {
                        if (arguments[i+1] == NULL) {
                                fprintf(stderr, "No output file specified\n");
                                free(new_args);
                                return;
                        }
                        fd_out = open(arguments[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
                        if (fd_out == -1) {
                                perror("Output redirection failed");
                                free(new_args);
                                return;
                        }
                        i += 2;
                } else if (strcmp(arguments[i], ">>") == 0) {
                        if (arguments[i+1] == NULL) {
                                fprintf(stderr, "No output file specified\n");
                                free(new_args);
                                return;
                        }
                        fd_out = open(arguments[i+1], O_WRONLY | O_CREAT | O_APPEND, 0644);
                        if (fd_out == -1) {
                                perror("Output redirection failed");
                                free(new_args);
                                return;
                        }
                        i += 2;
                } else {
                        new_args[new_i++] = strdup(arguments[i]);
                        i++;
                }
        }
        new_args[new_i] = NULL;

        pid_t pid = fork();

        if (pid == 0) {
                if (fd_in != -1) {
                        dup2(fd_in, STDIN_FILENO);
                        close(fd_in);
                }
                if (fd_out != -1) {
                        dup2(fd_out, STDOUT_FILENO);
                        close(fd_out);
                }
                if (execvp(new_args[0], new_args) == -1) {
                        perror("Command execution failed");
                        exit(EXIT_FAILURE);
                }
        } else if (pid < 0) {
                perror("Fork failed");
        } else {
                current_child = pid;
                int status;
                waitpid(pid, &status, 0);
                current_child = -1;
        }

        for (i = 0; i < new_i; i++) {
                free(new_args[i]);
        }
        free(new_args);

        if (fd_in != -1) close(fd_in);
        if (fd_out != -1) close(fd_out);
}

int multiple_cmds(char *input) {
    char **semicolon = inpTokenise(input, ";");
    int a = 0;

    while (semicolon[a] != NULL) {
        char **andCommands = inpTokenise(semicolon[a], "&&");
        int b = 0;
        int succ = 1;

        while (andCommands[b] != NULL && succ > 0) {
            char **p_parts = inpTokenise(andCommands[b], "|");
            int p_count = num_of_tokens(p_parts);

            if (p_count > 1) {
                char ***cmds = malloc(p_count * sizeof(char**));
                int c = 0;
                while (c < p_count) {
                    char *trim = p_parts[c];
                    while (*trim == ' ') trim++;
                    cmds[c] = inpTokenise(trim, " \t");
                    c++;
                }

                succ = piping_commands(cmds, p_count);

                c = 0;
                while (c < p_count) {
                    free_alloc_tokens(cmds[c]);
                    c++;
                }
                free(cmds);
            } else {
                char **argus = inpTokenise(p_parts[0], " \t");
                int redirection_flag = 0;
                int c = 0;

                while (argus[c] != NULL) {
                    if (strcmp(argus[c], "<") == 0 || 
                        strcmp(argus[c], ">") == 0 || 
                        strcmp(argus[c], ">>") == 0) {
                        redirection_flag = 1;
                        break;
                    }
                    c++;
                }

                if (redirection_flag) {
                    redirection(argus);
                } else {
                    succ = exec_cmd(argus);
                    if (succ == 0) {
                        free_alloc_tokens(argus);
                        free_alloc_tokens(p_parts);
                        free_alloc_tokens(andCommands);
                        free_alloc_tokens(semicolon);
                        return 0;
                    }
                }

                free_alloc_tokens(argus);
            }

            free_alloc_tokens(p_parts);
            b++;
        }

        free_alloc_tokens(andCommands);
        a++;
    }

    free_alloc_tokens(semicolon);
    return 1;
}


int main() {
        handlers();
        while (1) {
                printf("%s", "Shell_Input>");
                fflush(stdout);
                char *inp = read_inp();
                add_hist(inp);
                int resume = multiple_cmds(inp);
                free(inp);
                if (resume == 0) {
                        break;
                }
        }
        return 0;
}
