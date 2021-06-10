#include <inc/args.h>
#include <inc/string.h>

void
argstart(int *argc, char **argv, struct Argstate *args)
{
	args->argc = argc;
	args->argv = (const char **) argv;
	args->curarg = (*argc > 1 && argv ? "" : 0);
	args->argvalue = 0;
}

int
argnext(struct Argstate *args)
{
	int arg;

	args->argvalue = 0;

	// Done processing arguments if args->curarg == 0
	if (args->curarg == 0)
		return -1;

	if (!*args->curarg) {  // args->curarg是空字符
		// Need to process the next argument
		// Check for end of argument list
		if (*args->argc == 1   // 无参数
		    || args->argv[1][0] != '-' // 一开始就不是-x这种格式
		    || args->argv[1][1] == '\0') // 唯一参数就是一个'-'
			goto endofargs;
		// Shift arguments down one
		args->curarg = args->argv[1] + 1;
		memmove(args->argv + 1, args->argv + 2, sizeof(const char *) * (*args->argc - 1));
		(*args->argc)--;
		// Check for "--": end of argument list
		if (args->curarg[0] == '-' && args->curarg[1] == '\0')
			goto endofargs;
	}

	arg = (unsigned char) *args->curarg;
	args->curarg++;
	return arg;

    endofargs:
	args->curarg = 0;
	return -1;
}

char *
argvalue(struct Argstate *args)
{
	return (char*) (args->argvalue ? args->argvalue : argnextvalue(args));
}

char *
argnextvalue(struct Argstate *args)
{
	if (!args->curarg)
		return 0;
	if (*args->curarg) {
		args->argvalue = args->curarg;
		args->curarg = "";
	} else if (*args->argc > 1) {
		args->argvalue = args->argv[1];
		memmove(args->argv + 1, args->argv + 2, sizeof(const char *) * (*args->argc - 1));
		(*args->argc)--;
	} else {
		args->argvalue = 0;
		args->curarg = 0;
	}
	return (char*) args->argvalue;
}

