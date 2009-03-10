
#include "xtal.h"
#include "xtal_lib/xtal_winthread.h"
#include "xtal_lib/xtal_cstdiostream.h"
#include "xtal_lib/xtal_winfilesystem.h"
#include "xtal_lib/xtal_chcode.h"
#include "xtal_lib/xtal_errormessage.h"

using namespace xtal;

#ifndef XTAL_NO_PARSER

static int kcode = 'u';

static void print_usage(){
	fprintf(stdout,
		"usage: xtal [options] [script [args]].\n"
		"Available options are:\n"
		"  -Ks      set kcode sjis\n"
		"  -Ku      set kcode utf-8\n"
		"  -Ke      set kcode euc\n"
		"  -v       show version information\n"
	);
}

static void handle_argv(char** argv){
	int i;
	for(i=1; argv[i]!=0; i++){
		if(argv[i][0]!='-')
			break;

		switch(argv[i][1]){
		case 'v':
			fprintf(stdout, "xtal %d.%d.%d.%d\n", VERSION1, VERSION2, VERSION3, VERSION4);
			break;

		case 'K':
			kcode = argv[i][2];
			break;

		default:
			print_usage();
			return;
		}
	
	}

	if(argv[i]!=0){
		ArrayPtr arg_list(xnew<Array>());
		const char *filename = argv[i++];
		for(i=1; argv[i]!=0; i++){
			arg_list->push_back(argv[i]);
		}
		builtin()->def("argv", arg_list);
		load(filename);
	}
}

int main(int argc, char** argv){

	CStdioStreamLib cstd_stream_lib;
	WinThreadLib win_thread_lib;
	WinFilesystemLib win_filesystem_lib;
	SJISChCodeLib sjis_chcode_lib;
	UTF8ChCodeLib utf8_chcode_lib;
	EUCChCodeLib euc_chcode_lib;

	Setting setting;
	setting.thread_lib = &win_thread_lib;
	setting.stream_lib = &cstd_stream_lib;
	setting.filesystem_lib = &win_filesystem_lib;
	
	switch(kcode){
	case 's': setting.chcode_lib = &sjis_chcode_lib; break;
	case 'u': setting.chcode_lib = &utf8_chcode_lib; break;
	case 'e': setting.chcode_lib = &euc_chcode_lib; break;
	}

	initialize(setting);
	bind_error_message();

	handle_argv(argv);
	
	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
	}

	uninitialize();

	return 0;
}

#else

int main(int argc, char** argv){
	puts("xtal no parser");
	return 0;
}

#endif
