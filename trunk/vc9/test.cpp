
#include "../src/xtal/xtal.h"
#include "../src/xtal/xtal_macro.h"

#include "../src/xtal/xtal_lib/xtal_winthread.h"
#include "../src/xtal/xtal_lib/xtal_cstdiostream.h"
#include "../src/xtal/xtal_lib/xtal_winfilesystem.h"
#include "../src/xtal/xtal_lib/xtal_chcode.h"
#include "../src/xtal/xtal_lib/xtal_errormessage.h"

#include "time.h"

#include <iostream>

using namespace xtal;

class Foo{
public:
    float x, y;
    Foo(): x(0), y(0) {}
};

XTAL_PREBIND(Foo){
    it->def_ctor( ctor<Foo>() );
}

XTAL_BIND(Foo){
   it->def_getter(Xid(x), &Foo::x);
   it->def_setter(Xid(set_x), &Foo::x);
}

void test(){
    lib()->def(Xid(Foo), cpp_class<Foo>());

	if(CodePtr code = Xsrc((
		foo: lib::Foo();
		foo.x = 0.5;
		foo.x.p;
	))){
		code->call();
	}
} 

int main2(int argc, char** argv){
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | /*_CRTDBG_CHECK_ALWAYS_DF |*/ _CRTDBG_DELAY_FREE_MEM_DF);
	
	using namespace std;

	//enable_debug();
	//debug()->set_throw_hook(fun(&debug_throw));

	test();
			
	if(CodePtr code = Xsrc((
		//check_implicit_lookup();
	
	))){
		code->call();
	}


	//load("../struct.xtal");

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

	//compile_file("../bench/ao.xtal")->inspect()->p();
	
	if(0){
		int c = clock();
		load("../bench/ao.xtal");
		printf("ao %g\n\n", (clock()-c)/1000.0f);		
	}

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}


#if 1

	int c;

	/*		
	c = clock();
	load("../bench/vec.xtal");
	printf("vec %g\n\n", (clock()-c)/1000.0f);		
	
	c = clock();
	load("../bench/inst.xtal");
	printf("inst %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/gc.xtal");
	printf("gc %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/loop.xtal");
	printf("loop %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/nested_loops.xtal");
	printf("nested_loops %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/fib.xtal");
	printf("fib %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/loop_iter.xtal");
	printf("loop_iter %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/array_for.xtal");
	printf("array_for %g\n\n", (clock()-c)/1000.0f);

	c = clock();
	load("../bench/array_each.xtal");
	printf("array_each %g\n\n", (clock()-c)/1000.0f);

	//*/

	//*
	CodePtr code = compile_file("../test/test.xtal_");
	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

	code->call();

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

	lib()->member("test")->send("run_dir", "../test");
	
	//*/
#endif

	XTAL_CATCH_EXCEPT(e){
		stderr_stream()->println(e);
		return 1;
	}

	return 0;
}

int main(int argc, char** argv){
	CStdioStdStreamLib cstd_std_stream_lib;
	WinThreadLib win_thread_lib;
	WinFilesystemLib win_filesystem_lib;
	SJISChCodeLib sjis_ch_code_lib;

	Setting setting;
	setting.thread_lib = &win_thread_lib;
	setting.std_stream_lib = &cstd_std_stream_lib;
	setting.filesystem_lib = &win_filesystem_lib;
	setting.ch_code_lib = &sjis_ch_code_lib;

	initialize(setting);

	int ret = 1;
	XTAL_PROTECT{
		bind_error_message();

		ret = main2(argc, argv);

		vmachine()->print_info();
		uninitialize();
	}
	XTAL_OUT_OF_MEMORY{
		puts("out of memory");
	}

	return ret;
}