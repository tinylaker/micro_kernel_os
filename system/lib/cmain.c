#include <cmain.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

static char _cmd[1024] = { 0 };
static int _off_cmd = 0;

static char* read_cmain_arg(void) {
	char* p = NULL;
	uint8_t quotes = 0;

	while(_cmd[_off_cmd] != 0) {
		char c = _cmd[_off_cmd];
		_off_cmd++;
		if(quotes) { //read whole quotes content.
			if(c == '"') {
				_cmd[_off_cmd-1] = 0;
				return p;
			}
			continue;
		}
		if(c == ' ') { //read next arg.
			if(p == NULL) //skip begin spaces.
				continue;
			_cmd[_off_cmd-1] = 0;
			break;
		}
		else if(p == NULL) {
			if(c == '"') { //if start of quotes.
				quotes = 1;
				_off_cmd++;
			}
			p = _cmd + _off_cmd - 1;
		}
	}
	return p;
}

void init_cmd(void) {
}

#define ARG_MAX 16

void _start(void) {
	char* argv[ARG_MAX];
	int32_t argc = 0;

	init_cmd();

	while(argc < ARG_MAX) {
		char* arg = read_cmain_arg(); 
		if(arg == NULL || arg[0] == 0)
			break;
		argv[argc++] = arg;
	}

	int ret = main(argc, argv);
	exit(ret);
}