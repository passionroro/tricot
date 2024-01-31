#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <strings.h>

#define MAX_PTR 1024

static void	exitError(const char *str) {
	if (str) {
		write(2, str, strlen(str));
	}
	exit(1);
}

static bool is_valid_char(unsigned char c) {
	return c == '>' || c == '<' || c == '+' || c == '-' ||
			c == '[' || c == ']' || c == ',' || c == '.';
}

static size_t	read_from_file(FILE *file) {
	int		c;
	size_t	size = 0;

	while ((c = fgetc(file)) != EOF) {
		// casting may lead to problems
		if (!is_valid_char((unsigned char)c)) {
			break ;
		}
		size++;
	}
	
	if (!feof(file)) {
		clearerr(file);
		fclose(file);
		if (ferror(file)) {
			perror("");
			return 0;
		} else {
			exitError("Unvalid char in list\n");
		}
	}

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
		str[i++] = c;
	}

	fclose(file);
	return str;
}

static void	run(FILE *file, size_t read_bytes) {
	char			*instructions;
	unsigned char	*ptr;
	
	instructions = convert_file_content(file, read_bytes);
	ptr = calloc(MAX_PTR, sizeof(*ptr));

	while (instructions++) {
		if (*instructions == '>') {
			ptr++;
		} else if (*instructions == '<') {
			ptr--;
		} else if (*instructions == '+') {
			++(*ptr);
		} else if (*instructions == '-') {
			--(*ptr);
		} else if (*instructions == '.') {
			putchar(*ptr);
		} else if (*instructions == ',') {
			(*ptr) = getchar();
		} else if (*instructions == '[') {
		} else if (*instructions == ']') {
		}
	}

	free(instructions);
}

int	main(int argc, char **argv) {
	if (argc != 2) {
		exitError("Wrong number of arguments\n");
	}
	
	FILE *file = fopen(argv[1], "r");
	if (!file) {
		perror("");
		return 1;
	}

	size_t	read_bytes = read_from_file(file);
	if (read_bytes == 0) {
		return 2;
	}

	run(file, read_bytes);

	return 0;
}
