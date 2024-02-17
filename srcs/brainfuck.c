#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <strings.h>
#include <ctype.h>
#include <sys/stat.h>

#define MAX_PTR 1024
#define MAX_LOOP 128

void verbose(unsigned char *ptr) {
	for (int i = 0; i <= MAX_PTR; i++) {
		if (ptr[i]) {
			printf("[%p]\t[%d]\t=\t(%d)\t(%c)\n", &ptr[i], i, ptr[i], ptr[i]);
		}	
	}
}

static void	exitError(const char *str) {
	if (str && str[0]) {
		write(2, str, strlen(str));
	} else {
		perror("");
	}
	exit(1);
}

static bool is_valid_char(unsigned char c) {
	return c == '>' || c == '<' || c == '+' || c == '-' || \
			c == '[' || c == ']' || c == ',' || c == '.';
}

static size_t	read_from_file(const char *path) {
	int			c;
	size_t		size = 0;
	FILE		*file;
	struct stat	fileStat;

	
    if (stat(path, &fileStat) < 0) {
		exitError("");
	}

    if (!S_ISREG(fileStat.st_mode)) {
		exitError("Path must be a file");
    }

	file = fopen(path, "r");
	if (!file) {
		exitError("");
	}

	while ((c = fgetc(file)) != EOF) {
		if (is_valid_char((unsigned char)c)) {
			size++;
		} else if (!isspace(c)) {
			break ;
		}
	}

	if (!feof(file)) {
		clearerr(file);
		fclose(file);
		if (ferror(file)) {
			exitError("");
		} else {
			exitError("Unvalid char in list\n");
		}
	}

	fclose(file);
	return size;
}

static char	*convert_file_content(FILE *file, int read_bytes) {
	int		c, i = 0;
	char	*str;

	str  = malloc(sizeof(*str) * (read_bytes + 1));
	if (!str) {
		fclose(file);
		exitError("Fatal error\n");
	}

	while ((c = fgetc(file)) != EOF) {
		if (!isspace(c)) {
			str[i++] = c;
		}
	}
	str[i] = '\0';

	fclose(file);
	return str;
}

//note: what about overflow
static void	run(FILE *file, size_t read_bytes) {
	char			*instructions;
	unsigned char	*ptr;
	unsigned char	stack[MAX_LOOP] = {0};
	int				stack_ptr = -1;
	
	instructions = convert_file_content(file, read_bytes);
	ptr = calloc(MAX_PTR, sizeof(*ptr));
	
	for (int i = 0; instructions[i]; i++) {
		if (instructions[i] == '>') {
			ptr++;
		} else if (instructions[i] == '<') {
			ptr--;
		} else if (instructions[i] == '+') {
			(*ptr)++;
		} else if (instructions[i] == '-') {
			(*ptr)--;
		} else if (instructions[i] == '.') {
			putchar(*ptr);
		} else if (instructions[i] == ',') {
			(*ptr) = getchar();
		} else if (instructions[i] == '[') {
			stack[++stack_ptr] = i;
			if (!*ptr) {
				int nesting_level = 1;
				while (nesting_level > 0) {
					i++;
					if (instructions[i] == '[') {
						nesting_level++;
					} else if (instructions[i] == ']') {
						nesting_level--;
					}
				}
			}
		} else if (instructions[i] == ']') {
			if (*ptr) {
				i = stack[stack_ptr--] - 1;
			}
		}
	}

	free(instructions);
	//free(ptr);
}

static void	brainfuck(const char *file_path) {
	size_t	read_bytes = read_from_file(file_path);
	if (read_bytes == 0) {
		return ;
	}

	FILE	*file;
	file = fopen(file_path, "r");
	if (!file) {
		exitError("");
	}

	run(file, read_bytes);
}

int	main(int argc, char **argv) {
	if (argc != 2) {
		exitError("Wrong number of arguments\n");
	}

	brainfuck(argv[1]);
	
	return 0;
}
