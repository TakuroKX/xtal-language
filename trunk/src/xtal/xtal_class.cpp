#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_details.h"

namespace xtal{

namespace{
	Class* empty_inherited_classes[1] = {0};
};
	
InstanceVariables* InstanceVariables::create(ClassInfo* class_info){
	if(info_==&empty_class_info){
		// 空状態から一つだけインストールした状態へ移行
		InstanceVariables* ret = (InstanceVariables*)xmalloc(sizeof(InstanceVariables)+sizeof(AnyPtr)*class_info->instance_variable_size);
		ret->info_ = class_info;
		ret->sum_ = class_info->instance_variable_size;
		char* retbuf = (char*)(ret + 1);
		AnyPtr* retvalues = (AnyPtr*)retbuf;
		xmemset(retvalues, 0, class_info->instance_variable_size);
		return ret;
	}
	else if(info_){
		// 一つだけインストールした状態から複数インストールした状態へ移行
		char* buf = (char*)(this + 1);
		AnyPtr* values = (AnyPtr*)buf;

		int_t sum = info_->instance_variable_size+class_info->instance_variable_size;
		InstanceVariables* ret = (InstanceVariables*)xmalloc(
			sizeof(InstanceVariables) + sizeof(int_t) +
			sizeof(AnyPtr)*(2+sum));
		ret->info_ = 0;
		ret->sum_ = sum;

		char* retbuf = (char*)(ret + 1);
		int_t& retinstall_count = *(int_t*)retbuf; retbuf += sizeof(int_t);
		retinstall_count = 2;
		AnyPtr* retvalues = (AnyPtr*)retbuf;
		copy_any(retvalues[0], ImmediateValue(0, info_));
		copy_any(retvalues[1], ImmediateValue(info_->instance_variable_size, class_info));
		xmemcpy(retvalues+2, values, info_->instance_variable_size);
		xmemset(retvalues+2+info_->instance_variable_size, 0, class_info->instance_variable_size);	

		xfree(this, sizeof(InstanceVariables)+sizeof(AnyPtr)*info_->instance_variable_size);
		return ret;
	}
	else{
		char* buf = (char*)(this + 1);
		int_t install_count = *(int_t*)buf; buf += sizeof(int_t);
		AnyPtr* values = (AnyPtr*)buf;

		int_t oldsum = 0;
		for(int_t i=0; i<install_count; ++i){
			oldsum += ((ClassInfo*)rawvalue(values[i]).immediate_second_vpvalue())->instance_variable_size;
		}
		int_t sum = oldsum + class_info->instance_variable_size;

		InstanceVariables* ret = (InstanceVariables*)xmalloc(
			sizeof(InstanceVariables) + sizeof(int_t) +
			sizeof(AnyPtr)*(install_count+1+sum));
		ret->info_ = 0;
		ret->sum_ = sum;
	
		char* retbuf = (char*)(ret + 1);
		int_t& retinstall_count = *(int_t*)retbuf; retbuf += sizeof(int_t);
		retinstall_count = install_count+1;
		AnyPtr* retvalues = (AnyPtr*)retbuf;
		copy_any(retvalues[0], ImmediateValue(oldsum, class_info));
		xmemcpy(retvalues+1, values, install_count+oldsum);
		xmemset(retvalues+1+install_count+oldsum, 0, class_info->instance_variable_size);	

		xfree(this, sizeof(InstanceVariables)+sizeof(int_t)+sizeof(AnyPtr)*(oldsum+install_count));
		return ret;
	}
}

void InstanceVariables::destroy(){
	if(info_==&empty_class_info){

	}
	else if(info_){
		// 一つだけインストールした状態を削除
		char* buf = (char*)(this + 1);
		AnyPtr* values = (AnyPtr*)buf;
		for(int_t i=0, sz=sum_; i<sz; ++i){
			dec_ref_count_force(values[i]);
		}
		xfree(this, sizeof(InstanceVariables)+sizeof(AnyPtr)*sum_);
	}
	else{
		// 複数インストールした状態を削除
		char* buf = (char*)(this + 1);
		int_t install_count = *(int_t*)buf; buf += sizeof(int_t);
		AnyPtr* values = (AnyPtr*)buf;

		for(int_t i=0, sz=sum_; i<sz; ++i){
			dec_ref_count_force(values[install_count+i]);
		}

		xfree(this, sizeof(InstanceVariables)+sizeof(int_t)+sizeof(AnyPtr)*(sum_+install_count));
	}
}

void InstanceVariables::visit_members(Visitor& m){
	if(info_==&empty_class_info){

	}
	else if(info_){
		char* buf = (char*)(this + 1);
		AnyPtr* values = (AnyPtr*)buf;
		for(int_t i=0, sz=sum_; i<sz; ++i){
			m & values[i];
		}
	}
	else{
		char* buf = (char*)(this + 1);
		int_t install_count = *(int_t*)buf; buf += sizeof(int_t);
		AnyPtr* values = (AnyPtr*)buf;
		for(int_t i=0, sz=sum_; i<sz; ++i){
			m & values[install_count+i];
		}
	}
}

///////////////////////////////////////

Class::Class()
	:Frame(nul<Frame>(), nul<Code>(), 0){
	init();
}

Class::Class(const FramePtr& outer, const CodePtr& code, ClassInfo* info)
	:Frame(outer, code, info){
	init();
}

Class::Class(const IDPtr& name)
	:Frame(nul<Frame>(), nul<Code>(), 0){
	init();
	set_object_temporary_name(name);
}

Class::Class(cpp_class_t)
	:Frame(nul<Frame>(), nul<Code>(), 0){
	init();
	flags_ |= FLAG_NATIVE;
}

void Class::init(){
	options_ = 0;
	inherited_classes_ = empty_inherited_classes;
	object_force_ = 0;
	symbol_data_ = 0;
	set_initialized_members();
}

Class::~Class(){
	int_t count = 0;
	for(int_t i=0; inherited_classes_[i]; ++i){
		inherited_classes_[i]->dec_ref_count();
		count++;
	}

	if(count>0){
		xfree(inherited_classes_, sizeof(Class*)*(count+1));
	}

	if(options_){
		if(flags_&FLAG_OPTIONS){
			delete_object_xfree<Options2>((Options2*)options_);
		}
		else{
			delete_object_xfree<Options>(options_);
		}
	}
}

void Class::overwrite(const ClassPtr& p){
	if(!is_native() && !p->is_native()){
		
		for(int_t i=0; p->inherited_classes_[i]; ++i){
			inherit(p->inherited_classes_[i]);
		}
		
		if(p->buckets_){
			for(uint_t i=0; i<p->buckets_capa_; ++i){
				Node* node = p->buckets_[i];
				while(node!=0){
					if(node->flags&FLAG_NODE3){
						Node3* n = (Node3*)node;
						overwrite_member(n->primary_key, p->member_direct(n->num), n->secondary_key, n->accessibility());
					}
					else{
						Node2* n = (Node2*)node;
						overwrite_member(n->primary_key, p->member_direct(n->num), undefined, n->accessibility());
					}
					node = node->next;
				}	
			}
		}

		for(uint_t i=0; i<alive_object_count(); ++i){
			AnyPtr obj = alive_object(i);
			if(type(obj)==TYPE_BASE){
				if(obj->is(to_smartptr(this))){

					AnyPtr data = obj->save_instance_variables(to_smartptr(this));
					pvalue(obj)->init_instance_variables(p->info());
					obj->load_instance_variables(p, data);

					if(Node* it = find_node(Xid(reloaded), undefined)){
						const VMachinePtr& vm = setup_call(0);
						vm->set_arg_this(obj);
						member_direct(it->num)->rawcall(vm);
						vm->cleanup_call();
					}
				}
			}
		}

		set_outer(p->outer());
		set_code(p->code());
		set_info(p->info());
	}
}

void Class::inherit(const ClassPtr& cls){
	if(is_inherited(cls))
		return;

	XTAL_ASSERT(cls);

	cls->prebind();

	int_t count = 0;
	for(Class** pp=inherited_classes_; *pp; ++pp){
		count++;
	}

	Class** classes = (Class**)xmalloc(sizeof(Class*)*(count+2));
	*classes = cls.get();
	(*classes)->inc_ref_count();
	xmemcpy(classes+1, inherited_classes_, count+1);
	
	if(count>0){
		xfree(inherited_classes_, sizeof(Class*)*(count+1));
	}

	inherited_classes_ = classes;

	invalidate_cache_is();
}

void Class::inherit_first(const ClassPtr& cls){
	if(cls->is_singleton()){
		if(!is_singleton()){
			set_runtime_error(Xt0("XRE1031"));
			return;
		}
	}
	else{
		if(cls->is_native()){
			if(is_inherited_cpp_class()){
				set_runtime_error(Xt0("XRE1019"));
				return;
			}
		}
	}

	if(!symbol_data_){
		symbol_data_ = cls->symbol_data();
	}

	if(cls->is_final()){
		set_runtime_error(Xt1("XRE1028", name, cls->object_name()));
		return;
	}

	inherit(cls);
}

void Class::inherit_strict(const ClassPtr& cls){
	if(cls->is_native()){
		set_runtime_error(Xt1("XRE1029", name, cls->object_name()));
		return;
	}

	inherit_first(cls);
}

AnyPtr Class::inherited_classes(){
	return xnew<InheritedClassesIter>(to_smartptr(this));
}

const NativeFunPtr& Class::def_ctor(int_t type, const NativeFunPtr& ctor_func){
	set_option_ctor(type, ctor_func);
	flags_ |= (FLAG_LAST_DEFINED_CTOR);
	flags_ |= FLAG_NATIVE;
	return option_ctor(type);
}

const NativeFunPtr& Class::ctor(int_t type){
	prebind();

	const NativeFunPtr& ctor = option_ctor(type);

	if(ctor){
		return ctor;
	}

	for(int_t i=0; inherited_classes_[i]; ++i){
		if(const NativeFunPtr& ret = inherited_classes_[i]->ctor(type)){
			set_option_ctor(type, ret);
			return ret;
		}
	}

	return nul<NativeMethod>();
}

const NativeFunPtr& Class::def_ctor(const NativeFunPtr& ctor_func){
	return def_ctor(0, ctor_func);
}

const NativeFunPtr& Class::ctor(){
	return ctor(0);
}

const NativeFunPtr& Class::def_serial_ctor(const NativeFunPtr& ctor_func){
	return def_ctor(1, ctor_func);
}

const NativeFunPtr& Class::serial_ctor(){
	return ctor(1);
}

void Class::init_instance(const AnyPtr& self, const VMachinePtr& vm){
	for(int_t i=0; inherited_classes_[i]; ++i){
		inherited_classes_[i]->init_instance(self, vm);
	}
	
	if(info()->instance_variable_size){
		pvalue(self)->init_instance_variables(info());

		// 先頭のメソッドはインスタンス変数初期化関数
		if(member_direct(0)){
			vm->setup_call(0);
			vm->set_arg_this(self);
			member_direct(0)->rawcall(vm);
			vm->cleanup_call();
		}
	}
}

IDPtr Class::find_near_member2(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& minv){
	IDPtr minid = null;
	AnyPtr mem = get_class()->member(Xid(members_ancestors_too));

	if(is_undefined(mem)){
		return minid;
	}

	const VMachinePtr& vm = setup_call(1);
	vm->set_arg_this(ap(*this));
	mem->rawcall(vm);

	AnyPtr value = vm->result_and_cleanup_call();

	Xfor_cast(const ValuesPtr& v, value){
		IDPtr id = ptr_cast<ID>(v->at(0));
		int_t dist = edit_distance(primary_key, id);
		if(dist<minv){
			minv = dist;
			minid = id;
		}
	}

	return minid;
}

void Class::def_double_dispatch_method(const IDPtr& primary_key, int_t accessibility){
	on_def(primary_key, xtal::double_dispatch_method(primary_key), undefined, accessibility);
}

void Class::def_double_dispatch_fun(const IDPtr& primary_key, int_t accessibility){
	on_def(primary_key, xtal::double_dispatch_fun(to_smartptr(this), primary_key), undefined, accessibility);
}

const NativeFunPtr& Class::def_and_return(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t accessibility, const param_types_holder_n& pth, const void* val){
	if(is_singleton()){
		return unchecked_ptr_cast<NativeMethod>(def2(primary_key, xnew<NativeFun>(pth, val, to_smartptr(this)), secondary_key, accessibility));
	}
	else{
		return unchecked_ptr_cast<NativeMethod>(def2(primary_key, xnew<NativeMethod>(pth, val), secondary_key, accessibility));
	}
}

void Class::define(const LongLivedString& primary_key, const param_types_holder_n& pth){
	def(intern(primary_key), xnew<StatelessNativeMethod>(pth), undefined, KIND_PUBLIC);
}

void Class::define(const LongLivedString& primary_key, const AnyPtr& secondary_key, const param_types_holder_n& pth){
	def(intern(primary_key), xnew<StatelessNativeMethod>(pth), secondary_key, KIND_PUBLIC);
}

void Class::define_param(const LongLivedString& name, const AnyPtr& default_value){
	AnyPtr member;

	if(flags_&FLAG_LAST_DEFINED_CTOR){
		member = option_ctor();
	}
	else if(flags_&FLAG_LAST_DEFINED_CTOR2){
		member = option_serial_ctor();
	}
	else{
		member = members_.back();
	}

	if(type(member)==TYPE_STATELESS_NATIVE_METHOD){
		NativeFunPtr method = xnew<NativeMethod>(*rawvalue(member).pth());
		method->add_param(name, default_value);

		if(flags_&FLAG_LAST_DEFINED_CTOR){
			member = method;
		}
		else if(flags_&FLAG_LAST_DEFINED_CTOR2){
			member = method;
		}
		else{
			members_.set_at(members_.size()-1, method);
		}
	}
	else if(const NativeFunPtr& method = ptr_cast<NativeMethod>(member)){
		method->add_param(name, default_value);
	}
	else{
		XTAL_ASSERT(false);
	}
}

void Class::define(const LongLivedString& primary_key, const AnyPtr& value){
	def(intern(primary_key), value, undefined, KIND_PUBLIC);
}

void Class::define(const LongLivedString& primary_key, const AnyPtr& value, const AnyPtr& secondary_key){
	def(intern(primary_key), value, undefined, KIND_PUBLIC);
}

const AnyPtr& Class::def2(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	on_def(primary_key, value, secondary_key, accessibility);

	if(Node* it = find_node(primary_key, secondary_key)){
		return member_direct(it->num);
	}
	return null;
}

void Class::overwrite_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	if(Node* it= find_node(primary_key, secondary_key)){
		if(const ClassPtr& dest = ptr_cast<Class>(member_direct(it->num))){
			if(const ClassPtr& src = ptr_cast<Class>(value)){
				if(!dest->is_native() && !src->is_native()){
					dest->overwrite(src);
					invalidate_cache_member();
					return;
				}
			}
		}
		
		Frame::set_member_direct(it->num, value);
		value->set_object_parent(to_smartptr(this));
		invalidate_cache_member();
	}
	else{
		on_def(primary_key, value, secondary_key, accessibility);
	}
}

void Class::on_def(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	flags_ &= ~(FLAG_LAST_DEFINED_CTOR | FLAG_LAST_DEFINED_CTOR2);

	if(find_node(primary_key, secondary_key)){
		XTAL_SET_EXCEPT(cpp_class<RedefinedError>()->call(Xt2("XRE1011", object, this->object_name(), name, primary_key)));
	}
	else{
		Node* node = insert_node(primary_key, secondary_key);
		node->num = members_.size();
		node->flags |= accessibility;
		push_back_member(value);
		value->set_object_parent(to_smartptr(this));
		invalidate_cache_member();
	}
}

void Class::def(const char_t* primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	on_def(intern(primary_key), value, secondary_key, accessibility);
}

void Class::def(const IDPtr& primary_key, const AnyPtr& value){
	on_def(primary_key, value, undefined, KIND_PUBLIC);
}

void Class::def(const char_t* primary_key, const AnyPtr& value){
	on_def(intern(primary_key), value, undefined, KIND_PUBLIC);
}

const AnyPtr& Class::find_member(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility, bool& nocache){
	if(Node* it = find_node(primary_key, secondary_key)){
		accessibility = it->accessibility();
		nocache = it->nocache();
		return member_direct(it->num);
	}

	if(const ClassPtr& klass = ptr_cast<Class>(secondary_key)){
		for(int_t i=0; klass->inherited_classes_[i]; ++i){
			const AnyPtr& ret = find_member(primary_key,  klass->inherited_classes_[i], accessibility, nocache);
			if(!is_undefined(ret)){
				return ret;
			}
		}

		if(rawne(cpp_class<Any>(), klass)){
			const AnyPtr& ret = find_member(primary_key, cpp_class<Any>(), accessibility, nocache);
			if(!is_undefined(ret)){
				return ret;
			}
		}	
	}

	return undefined;
}
	
const AnyPtr& Class::find_member_from_inherited_classes(const IDPtr& primary_key, const AnyPtr& secondary_key, int_t& accessibility, bool& nocache){
	for(int_t i=0; inherited_classes_[i]; ++i){
		const AnyPtr& ret = inherited_classes_[i]->find_member(primary_key, secondary_key, true, accessibility, nocache);
		if(!is_undefined(ret)){
			return ret;
		}
	}
	return undefined;
}

const AnyPtr& Class::find_member(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	const AnyPtr& ret = find_member(primary_key, secondary_key, accessibility, nocache);
	if(!is_undefined(ret)){
		return ret;
	}

	for(int_t i=0; i<CppClassSymbolData::BIND; ++i){
		if(bind(i)){
			const AnyPtr& ret = find_member(primary_key, secondary_key, accessibility, nocache);
			if(!is_undefined(ret)){
				return ret;
			}
		}
	}

	if(inherited_too){
		const AnyPtr& ret = find_member_from_inherited_classes(primary_key, secondary_key, accessibility, nocache);
		if(!is_undefined(ret)){
			return ret;
		}
	}

	return undefined;
}

const AnyPtr& Class::on_rawmember(const IDPtr& primary_key, const AnyPtr& secondary_key, bool inherited_too, int_t& accessibility, bool& nocache){
	const AnyPtr& ret = find_member(primary_key, secondary_key, inherited_too, accessibility, nocache);
	if(!is_undefined(ret)){
		return ret;
	}

	if(inherited_too){
		return cpp_class<Any>()->find_member(primary_key, secondary_key, false, accessibility, nocache);
	}

	return undefined;
}

bool Class::set_member(const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key){
	if(Node* it = find_node(primary_key, secondary_key)){
		Frame::set_member_direct(it->num, value);
		value->set_object_parent(to_smartptr(this));
		invalidate_cache_member();
		return true;
	}
	else{
		set_runtime_error(Xid(undefined));
		return false;
	}
}

void Class::set_member_direct(int_t i, const IDPtr& primary_key, const AnyPtr& value, const AnyPtr& secondary_key, int_t accessibility){
	Frame::set_member_direct(i, value);
	
	Node* it = insert_node(primary_key, secondary_key);
	it->num = i;
	it->flags |= accessibility;
	value->set_object_parent(to_smartptr(this));
	invalidate_cache_member();
}

void Class::on_set_object_parent(const ClassPtr& parent){
	if(object_force_<parent->object_force()){
		object_force_ = parent->object_force()-1;
		HaveParentBase::on_set_object_parent(parent);

		for(uint_t i=0; i<buckets_capa_; ++i){
			Node* node = buckets_[i];
			while(node!=0){
				member_direct(node->num)->set_object_parent(to_smartptr(this));
				node =  node->next;
			}	
		}
	}
}

ValuesPtr Class::child_object_name(const AnyPtr& a){
	for(uint_t i=0; i<buckets_capa_; ++i){
		Node* node = buckets_[i];
		while(node!=0){
			if(raweq(member_direct(node->num), a)){
				if(node->flags&FLAG_NODE3){
					return mv(((Node3*)node)->primary_key, ((Node3*)node)->secondary_key);
				}
				else{
					return mv(((Node2*)node)->primary_key, undefined);
				}
			}
			node =  node->next;
		}	
	}
	return nul<Values>();
}

IDPtr Class::object_temporary_name(){
	IDPtr name = option_name();
	if(name && name->data_size()!=0){
		return name;
	}

	if(symbol_data_ && (symbol_data_->flags&CppClassSymbolData::FLAG_NAME)){
		return XTAL_LONG_LIVED_STRING(symbol_data_->name);
	}

	if(code() && info()){
		return code()->identifier(info()->name_number);
	}

	return empty_id;
}

void Class::set_object_temporary_name(const IDPtr& name){
	set_option_name(name);
}

void Class::make_options(){
	if(!options_){
		options_ = new_object_xmalloc<Options>();
	}
}

void Class::make_options_wide(){
	if(!options_){
		options_ = new_object_xmalloc<Options2>();
	}
	else if(flags_&FLAG_OPTIONS){
		Options2* p = new_object_xmalloc<Options2>();
		p->ctor = options_->ctor;
		delete_object_xfree<Options>(options_);
		options_ = p;
	}
	flags_ |= FLAG_OPTIONS;
}

const IDPtr& Class::option_name(){
	if(options_ && (flags_&FLAG_OPTIONS)){
		return ((Options2*)options_)->name;
	}
	return empty_id;
}

void Class::set_option_name(const IDPtr& name){
	make_options_wide();
	((Options2*)options_)->name = name;
}

const NativeFunPtr& Class::option_ctor(){
	if(options_){
		return ((Options2*)options_)->ctor;
	}
	return nul<NativeMethod>();
}

void Class::set_option_ctor(const NativeFunPtr& fun){
	make_options();
	((Options2*)options_)->ctor = fun;
}

const NativeFunPtr& Class::option_serial_ctor(){
	if(options_ && (flags_&FLAG_OPTIONS)){
		return ((Options2*)options_)->serial_ctor;
	}
	return nul<NativeMethod>();
}

void Class::set_option_serial_ctor(const NativeFunPtr& fun){
	make_options_wide();
	((Options2*)options_)->serial_ctor = fun;
}

const NativeFunPtr& Class::option_ctor(uint_t n){
	if(n==0){ return option_ctor(); }
	else{ return option_serial_ctor(); }
}

void Class::set_option_ctor(uint_t n, const NativeFunPtr& fun){
	if(n==0){ set_option_ctor(fun); }
	else{ set_option_serial_ctor(fun); }
}

bool Class::is_inherited(const AnyPtr& v){
	if(this==pvalue(v)){
		return true;
	}

	if(raweq(v, cpp_class<Any>())){
		return true;
	}

	prebind();

	for(int_t i=0; inherited_classes_[i]; ++i){
		if(inherited_classes_[i]->is_inherited(v)){
			return true;
		}
	}

	return false;
}

bool Class::is_inherited_cpp_class(){
	if(is_native()){
		return true;
	}

	prebind();

	for(int_t i=0; inherited_classes_[i]; ++i){
		if(inherited_classes_[i]->is_inherited_cpp_class()){
			return true;
		}
	}

	return false;
}

void Class::on_rawcall(const VMachinePtr& vm){
	prebind();

	if(is_singleton()){
		return Any::rawsend(vm, Xid(op_call));
	}

	if(is_native()){
		if(const NativeFunPtr& ret = ctor()){
			ret->rawcall(vm);
			if(vm->except()){
				return;
			}
			init_instance(vm->result(), vm);
		}
		else{
			set_runtime_error(Xt1("XRE1013", object, object_name()), vm);
		}
	}
	else{
		AnyPtr instance;
		if(const NativeFunPtr& newfun = ctor()){
			instance = newfun->call();
		}
		else{
			instance = XNew<Base>();
		}

		if(type(instance)==TYPE_BASE){
			pvalue(instance)->set_class(to_smartptr(this));
		}

		init_instance(instance, vm);

		XTAL_CHECK_EXCEPT(e){ return; }
		
		if(const AnyPtr& ret = member(Xid(initialize), undefined)){
			int_t n = vm->need_result_count();
			vm->set_arg_this(instance);
			ret->rawcall(vm);
			if(n!=0){
				vm->replace_result(instance);
			}
		}
		else{
			vm->return_result(instance);
		}
	}
}

void Class::s_new(const VMachinePtr& vm){
	if(is_singleton()){
		set_runtime_error(Xt1("XRE1013", object, object_name()), vm);
		return;
	}

	if(is_native()){
		if(const AnyPtr& ret = serial_ctor()){
			ret->rawcall(vm);
			init_instance(vm->result(), vm);
		}
		else{
			set_runtime_error(Xt1("XRE1013", object, object_name()), vm);
		}
	}
	else{
		AnyPtr instance;
		if(const AnyPtr& newfun = serial_ctor()){
			instance = newfun->call();
		}
		else{
			instance = XNew<Base>();
		}

		if(type(instance)==TYPE_BASE){
			pvalue(instance)->set_class(to_smartptr(this));
			init_instance(instance, vm);
		}

		vm->return_result(instance);
	}
}

AnyPtr Class::ancestors(){
	if(raweq(to_smartptr(this), cpp_class<Any>())){
		return null;
	}			
	
	ArrayPtr ret = xnew<Array>();

	for(int_t i=0; inherited_classes_[i]; ++i){
		ret->push_back(inherited_classes_[i]);

		Xfor(it2, inherited_classes_[i]->ancestors()){
			ret->push_back(it2);
		}
	}

	ret->push_back(cpp_class<Any>());
	return ret;
}

void Class::prebind(){
	if((flags_ & FLAG_PREBINDED)==0){
		flags_ |= FLAG_PREBINDED;
		if(symbol_data_ && (symbol_data_->flags&CppClassSymbolData::FLAG_BIND0)){
			symbol_data_->prebind(this);
		}
	}
}

bool Class::bind(int_t n){
	if(n<0){
		bool ret = false;
		for(int_t i=0; i<CppClassSymbolData::BIND; ++i){
			if(bind(i)){
				ret = true;
			}
		}
		return ret;
	}

	if((flags_ & (FLAG_BINDED<<n))==0){
		flags_ |= (FLAG_BINDED<<n);
		prebind();
		if(symbol_data_ && (symbol_data_->flags&(CppClassSymbolData::FLAG_BIND1<<n))){
			symbol_data_->bind[n](this);
			return true;
		}
	}

	return false;
}

void Class::set_singleton(){
	if((flags_ & FLAG_SINGLETON)==0){
		flags_ |= FLAG_SINGLETON;
		inherit(get_class());
		Base::set_class(to_smartptr(this));
	}
}

void Class::init_singleton(const VMachinePtr& vm){
	init_instance(to_smartptr(this), vm);
	
	if(const AnyPtr& ret = member(Xid(initialize), undefined)){
		vm->setup_call(0);
		vm->set_arg_this(to_smartptr(this));
		ret->rawcall(vm);
		vm->cleanup_call();
	}
}


InheritedClassesIter::InheritedClassesIter(const ClassPtr& a)
	:frame_(a), it_(0){
}

void InheritedClassesIter::on_visit_members(Visitor& m){
	Base::on_visit_members(m);
	m & frame_;
}

void InheritedClassesIter::block_next(const VMachinePtr& vm){
	if(frame_->inherited_classes_[it_]){
		vm->return_result(this, frame_->inherited_classes_[it_]);
		++it_;
	}
	else{
		vm->return_result(null, null);
	}
}


}
