
#include "xtal.h"
#include "xtal_macro.h"

using namespace xtal;

static void print_usage(){
	fprintf(stderr,
		"usage: xtal [options] [script [args]].\n"
		"Available options are:\n"
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


#include "xtal_peg.h"

void debug_line(const SmartPtr<debug::Info>& info){
	//std::cout << Xf("kind=%d, line=%s, file=%s, fun=%s\n")(info->kind(), info->line(), info->file_name(), info->fun_name());

	/*if(info->local_variables()){
		Xfor2(key, value, info->local_variables()->members()){
			std::cout << Xf("key=%s, value=%s\n")(key, value);
		}
	}*/
}

//#include <crtdbg.h>

struct PointSelf : public Base{
	AnyPtr self;

	PointSelf(){
		self = AnyPtr::from_this(this);
	}

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & self;
	}
};

int main(int argc, char** argv){

	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | /*_CRTDBG_CHECK_ALWAYS_DF |*/ _CRTDBG_DELAY_FREE_MEM_DF);

	try{

		initialize();

		xnew<PointSelf>();

#if 1
		//debug::enable();
		//debug::set_line_hook(fun(&debug_line));
		//debug::set_call_hook(fun(&debug_line));
		//debug::set_return_hook(fun(&debug_line));

		{
			StringPtr path(argv[0]);

#ifdef WIN32
			StringPtr sep("\\");
#else
			StringPtr sep("/");
#endif

			ArrayPtr temp = cast<ArrayPtr>(path->split(sep)->send("to_a"));
			temp->pop_back();
#ifdef WIN32
			temp->push_back("message.xtal");
#else
			temp->push_back("message_en.xtal");
#endif
			path = temp->join(sep)->to_s();
			add_get_text_map(cast<MapPtr>(load(path)));
		}
	

AnyPtr ret = Xsrc
((

 //split("::" | "," | ".");
	
 compile("for(i:0; i<10; i=i+1)i.p;").inspect.p;
for(i:0; i<10; i=i+1)i.p;


test: fun(aaaa, b, aaaaa){ aaaa.p; b.p; }.p;
test(5, 6, 7);

))();

		//handle_argv(argv);
		
int c;

		/*		
		c = clock();
		load("../bench/vec.xtal");
		printf("vec %g\n", (clock()-c)/1000.0f);		
		
		c = clock();
		load("../bench/inst.xtal");
		printf("inst %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		full_gc();
		printf("full_gc %g\n\n", (clock()-c)/1000.0f);

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

		load("../test/test_empty.xtal");
		load("../test/test_array.xtal");
		load("../test/test_float.xtal");
		load("../test/test_class.xtal");
		load("../test/test_multiple_values.xtal");
		load("../test/test_except.xtal");
		load("../test/test_fiber.xtal");
		load("../test/test_fun.xtal");
		load("../test/test_iter.xtal");
		load("../test/test_fib.xtal");
		load("../test/test_calc.xtal");
		load("../test/test_for.xtal");
		load("../test/test_if.xtal");
		load("../test/test_nested_loops.xtal");
		load("../test/test_assign.xtal");
		load("../test/test_op_assign.xtal");
		load("../test/test_inc.xtal");
		load("../test/test_toplevel.xtal");
		//load("../test/test_serialize.xtal");
		
		//*/
#endif

	}catch(AnyPtr e){
		fprintf(stderr, "%s\n", e->to_s()->c_str());
	}

	vmachine().get()->print_info();
	print_result_of_cache();

	uninitialize();

	return 0;
}
