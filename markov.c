#include<stdio.h>
#include<stdlib.h>
#include<string.h>

/* Current string */
char* string;

/* Rule structure */
typedef struct _rule {
	char* find; /* search for match */
	char* replace; /* replacement */
	size_t fl; /* find length */
	size_t rl; /* replace length */
	int final; /* is rule final */
	struct _rule* next;
} rule;

/* Pointer to the first rule */
rule* program;

int load_program(FILE* f) {
	char buf[1024];
	char* s;
	/* Pointer to the last rule (needed to attach the next rule read 
	 *  from a file) */
	rule** last = &program;
	program = NULL;
	
	/* Read program line by line into buf */
	while ( (s = fgets(buf, 1024, f)) ) {
		char* find, *repl;
		
		/* THere should be dot "." in the middle between find and replace
		 * to be considered final. */
		int final = strchr(s, '.') != NULL;
		
		/* Tokenize string to get 'find' and 'replace' strings */
		if ( (find = strsep(&s, "|.") ) && (repl = strsep(&s, "\n")) ) {
			*last = malloc(sizeof(rule));
			**last = (rule){strdup(find), strdup(repl),
				strlen(find), strlen(repl), final, NULL};
			/* Update pointer to the last 
			 * to be the pointer to the next field of the last rule
			 */
			last = &(*last)->next;
		} 
		/* otherwise, it looks like a comment -- skip it */
	}
	/* EOF */
	fclose(f);
	if ( !program ) {
		fprintf(stderr, "Program is empty!\n");
		return -1;
	}
	return 0;
}

int _interpret(rule *r, size_t sl) {
	char *s;
	if ( !r ) return -1; /* no more rules -- may be error? */
	/* Try to find find */
	if ( (s = strstr(string, r->find)) ) {
		/* Create a new string with replacement */
		char* ns = malloc(sl + r->rl - r->fl + 1);
		strncpy(ns, string, s-string); /* leave the pre-match string */
		strcpy(ns+(s-string), r->replace); /* put replacement */
		strcpy(ns+(s-string) +  r->rl, s + r->fl); /* put after-match string */
		free(string); /* old string no more needed */
		string = ns;
		return r->final ? 1 : 0;
	} 
	return _interpret(r->next, sl);
}

void print_string() {
	printf("String: %s\n", string);
}

int interpret() {
	int result = _interpret(program, strlen(string));
	print_string();
	if ( result == 0 ) /* continue interpretation */
		return interpret();
	return result;

}

void free_program() {
	while (program) {
		rule* next = program->next;
		free(program->find);
		free(program->replace);
		free(program);
		program = next;
	}
}

int load_program_and_interpret(FILE* f, char* initstr) {
	int result;
	if ( !load_program(f) ) {
		string = strdup(initstr);
		result = interpret();
		free(string);
		free_program();
		return result;
	} else {
		fprintf(stderr, "Cannot load program\n");
		return -1;
	}
}

int main(int argc, char** argv) {
	if ( argc > 2) {
		FILE* f = fopen(argv[1], "r");
		if ( !f ) {
			fprintf(stderr, "Cannot open file %s\n", argv[1]);
			exit(1);
		}
		load_program_and_interpret(f, argv[2]);
	} else {
		fprintf(stderr, "Usage: %s <program> <string>\n", argv[0]);
	}
	exit(0);
}
