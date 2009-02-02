#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

void Array::visit_members(Visitor& m){
	Base::visit_members(m);
	for(uint_t i=0; i<size_; ++i){
		m & values_[i];
	}
}

Array::Array(uint_t size){
	capa_ = size + 3;
	size_ = size;
	values_ = (AnyPtr*)user_malloc(sizeof(AnyPtr)*capa_);
	std::memset(values_, 0, sizeof(AnyPtr)*size_);
}

Array::Array(const AnyPtr* first, const AnyPtr* end){
	int_t size = end-first;

	capa_ = size;
	size_ = size;
	values_ = (AnyPtr*)user_malloc(sizeof(AnyPtr)*capa_);

	for(int_t i=0; i<size; ++i){
		copy_innocence(values_[i], first[i]);
		inc_ref_count_force(values_[i]);
	}
}

Array::Array(const Array& v){
	size_ = capa_ = ((Array&)v).size();
	values_ = (AnyPtr*)user_malloc(sizeof(AnyPtr)*capa_);
	
	for(uint_t i=0; i<size_; ++i){
		copy_innocence(values_[i], v.values_[i]);
		inc_ref_count_force(values_[i]);
	}
}

Array& Array::operator =(const Array& v){
	Array temp(v);
	std::swap(values_, temp.values_);
	std::swap(size_, temp.size_);
	std::swap(capa_, temp.capa_);
	return *this;
}

Array::~Array(){
	for(uint_t i=0; i<size_; ++i){
		dec_ref_count_force(values_[i]);
	}
	user_free(values_);
}

void Array::clear(){
	for(uint_t i=0; i<size_; ++i){
		dec_ref_count_force(values_[i]);
	}
	size_ = 0;
}

void Array::resize(uint_t sz){
	if(sz<0) sz = 0;

	if(sz<size_){
		for(uint_t i=sz; i<size_; ++i){
			dec_ref_count_force(values_[i]);
		}
		size_ = sz;
	}
	else if(sz>size_){
		if(sz>capa_){
			uint_t newcapa = sz+capa_;
			AnyPtr* newp = (AnyPtr*)user_malloc(sizeof(AnyPtr)*newcapa);
			std::memcpy(newp, values_, sizeof(AnyPtr)*size_);
			std::memset(&newp[size_], 0, sizeof(AnyPtr)*(sz-size_));
			user_free(values_);
			values_ = newp;
			size_ = sz;
			capa_ = newcapa;
		}
		else{
			std::memset(&values_[size_], 0, sizeof(AnyPtr)*(sz-size_));
			size_ = sz;
		}
	}
}

const AnyPtr& Array::op_at(int_t i){
	return values_[calc_offset(i)];

}

void Array::op_set_at(int_t i, const AnyPtr& v){
	values_[calc_offset(i)] = v;
}

void Array::erase(int_t start, int_t n){
	int pos = calc_offset(start);
	if(n<0 || (uint_t)(n + pos)>size_){
		throw_index_error();
		return;
	}

	for(int_t i=0; i<n; ++i){
		dec_ref_count_force(values_[pos + i]);
	}

	size_ -= n;
	if(size_!=0){
		std::memmove(&values_[pos], &values_[pos+n], sizeof(AnyPtr)*(size_-pos));
	}
}

void Array::insert(int_t i, const AnyPtr& v){
	if(capa_==size_){
		resize(size_ + 1);
	}
	else{
		size_++;
	}
	int_t pos = calc_offset(i);
	std::memmove(&values_[pos+1], &values_[pos], sizeof(AnyPtr)*(size_-1-pos));
	copy_innocence(values_[pos], v);
	inc_ref_count_force(values_[pos]);
}

void Array::push_back(const AnyPtr& v){
	if(capa_==size_){
		resize(size_ + 1);
	}
	else{
		size_++;
	}
	copy_innocence(values_[size_-1], v);
	inc_ref_count_force(values_[size_-1]);
}

void Array::pop_back(){
	if(!empty()){
		dec_ref_count_force(values_[size_-1]);
		size_--;
	}
}

ArrayPtr Array::slice(int_t start, int_t n){
	if(n==0){
		return xnew<Array>(0);
	}

	int_t pos = calc_offset(start);
	if(n<0 || (uint_t)(n + pos)>size_){
		throw_index_error();
		return null;
	}

	return xnew<Array>(&values_[pos], &values_[pos+n]);
}

ArrayPtr Array::splice(int_t start, int_t n){
	ArrayPtr ret = slice(start, n);
	erase(start, n);
	return ret;
}

ArrayPtr Array::clone(){
	return xnew<Array>(*this);
}

ArrayPtr Array::cat(const ArrayPtr& a){
	ArrayPtr ret(clone());
	ret->cat_assign(a);
	return ret;
}

ArrayPtr Array::cat_assign(const ArrayPtr& a){
	for(uint_t i = 0, size = a->size(); i<size; ++i){
		push_back(a->at(i));
	}
	return from_this(this);
}
	
StringPtr Array::join(const StringPtr& sep){
	return ptr_cast<String>(send(Xid(join), sep));
}

bool Array::op_eq(const ArrayPtr& other){
	if(size()!=other->size())
		return false;

	const VMachinePtr& vm = vmachine();
	for(uint_t i=0, size=other->size(); i<size; ++i){
		if(rawne(at(i), other->at(i))){
			vm->setup_call(1, other->at(i));
			at(i)->rawsend(vm, Xid(op_eq));
			if(!vm->processed() || !vm->result()){
				vm->cleanup_call();
				return false;
			}
			vm->cleanup_call();
		}
	}
	return true;
}

AnyPtr Array::each(){
	return xnew<ArrayIter>(from_this(this));
}

AnyPtr Array::reverse(){
	return xnew<ArrayIter>(from_this(this), true);
}

void Array::assign(const AnyPtr& iterator){
	clear();

	uint_t i = 0;
	Xfor(v, iterator){
		if(i>=size_){
			resize(i+1);
		}

		set_at(i, v);
		++i;
	}
}

void Array::concat(const AnyPtr& iterator){
	Xfor(v, iterator){
		push_back(v);
	}
}

int_t Array::calc_offset(int_t i){
	if(i<0){
		i = size_ + i;
		if(i<0){
			throw_index_error();
			return 0;
		}
	}
	else{
		if((uint_t)i >= size_){
			throw_index_error();
			return 0;
		}
	}
	return i;
}

void Array::throw_index_error(){
	XTAL_THROW(RuntimeError()->call(Xt("Xtal Runtime Error 1020")), return);
}

//////////////////////////////////////////////////

ArrayIter::ArrayIter(const ArrayPtr& a, bool reverse)
	:array_(a), index_(-1), reverse_(reverse){
}
		
void ArrayIter::block_next(const VMachinePtr& vm){
	++index_;
	if(index_<(int_t)array_->size()){
		vm->return_result(from_this(this), array_->at(reverse_ ? array_->size()-1-index_ : index_));
	}
	else{
		vm->return_result(null, null);
	}
}
	
bool ArrayIter::block_next_direct(AnyPtr& ret){
	++index_;
	if(index_<(int_t)array_->size()){
		ret = array_->at(reverse_ ? array_->size()-1-index_ : index_);
		return true;
	}
	else{
		ret = null;
		return false;
	}
}

void ArrayIter::visit_members(Visitor& m){
	Base::visit_members(m);
	m & array_;
}

//////////////////////////////////////////////////

MultiValuePtr MultiValue::clone(){
	return xnew<MultiValue>(*this);
}

MultiValuePtr MultiValue::flatten_mv(){
	if(empty()){ return xnew<MultiValue>(); }

	MultiValuePtr ret = xnew<MultiValue>();
	ret->concat(from_this(this));
	
	for(;;){
		if(const MultiValuePtr& mv = ptr_as<MultiValue>(ret->back())){
			ret->pop_back();
			ret->concat(mv);
		}
		else{
			if(type(ret->back())==TYPE_UNDEFINED){
				ret->pop_back();
			}
			break;
		}
	}
	return ret;
}

MultiValuePtr MultiValue::flatten_all_mv(){
	if(empty()){ return xnew<MultiValue>(); }

	MultiValuePtr ret = xnew<MultiValue>();
	MultiValuePtr temp(from_this(this));
	
	for(uint_t i=0; i<temp->size(); ++i){
		if(const MultiValuePtr& mv = ptr_as<MultiValue>(temp->at(i))){
			ret->concat(mv->flatten_all_mv());
		}
		else{
			if(type(ret->back())!=TYPE_UNDEFINED){
				ret->push_back(temp->at(i));
			}
		}
	}
	return ret;
}

//////////////////////////////////////////////

void initialize_array(){
	
	{
		ClassPtr p = new_cpp_class<ArrayIter>(Xid(ArrayIter));
		p->inherit(Iterator());
		p->method(Xid(block_next), &ArrayIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<Array>(Xid(Array));
		p->inherit(Iterable());

		p->def(Xid(new), ctor<Array, int_t>()->params(Xid(size), 0));
		
		p->method_static<uint_t (Array::*)(), &Array::size>(Xid(size));
		p->method_static<uint_t (Array::*)(), &Array::size>(Xid(length));
		p->method_static<void (Array::*)(uint_t), &Array::resize>(Xid(resize));
		p->method_static<bool (Array::*)(), &Array::empty>(Xid(empty));
		p->method_static<ArrayPtr (Array::*)(int_t, int_t), &Array::slice>(Xid(slice))->params(Xid(i), null, Xid(n), 1);
		p->method_static<ArrayPtr (Array::*)(int_t, int_t), &Array::splice>(Xid(splice))->params(Xid(i), null, Xid(n), 1);
		p->method_static<void (Array::*)(), &Array::pop_back>(Xid(pop_back));
		p->method_static<void (Array::*)(), &Array::pop_front>(Xid(pop_front));
		p->method_static<void (Array::*)(const AnyPtr&), &Array::push_back>(Xid(push_back));
		p->method_static<void (Array::*)(const AnyPtr&), &Array::push_front>(Xid(push_front));

		p->method(Xid(erase), &Array::erase)->params(Xid(i), null, Xid(n), 1);
		p->method(Xid(insert), &Array::insert);
		p->method(Xid(to_a), &Array::to_a);
		p->method(Xid(each), &Array::each);
		p->method(Xid(clone), &Array::clone);
		p->method(Xid(front), &Array::front);
		p->method(Xid(back), &Array::back);
		p->method(Xid(clear), &Array::clear);
		p->method(Xid(reverse), &Array::reverse);
		p->method(Xid(assign), &Array::assign);
		p->method(Xid(concat), &Array::concat);

		p->method(Xid(op_cat), &Array::cat, get_cpp_class<Array>());
		p->method(Xid(op_cat_assign), &Array::cat_assign, get_cpp_class<Array>());
		p->method(Xid(op_at), &Array::op_at, get_cpp_class<Int>());
		p->method(Xid(op_set_at), &Array::op_set_at, get_cpp_class<Int>());
		p->method(Xid(op_eq), &Array::op_eq, get_cpp_class<Array>());
	}

	{
		ClassPtr p = new_cpp_class<MultiValue>(Xid(MultiValue));
		p->inherit(get_cpp_class<Array>());

		p->def(Xid(new), ctor<MultiValue, int_t>()->params(Xid(size), 0));
		p->method(Xid(clone), &MultiValue::clone);
		p->method(Xid(to_a), &Array::clone);
		p->method(Xid(to_mv), &Any::self);
		p->method(Xid(flatten_mv), &MultiValue::flatten_mv);
		p->method(Xid(flatten_all_mv), &MultiValue::flatten_all_mv);
	}

	{
		ClassPtr p = new_cpp_class<Expr>(Xid(Expr));
		p->inherit(get_cpp_class<Array>());		
	}

	builtin()->def(Xid(Array), get_cpp_class<Array>());
	builtin()->def(Xid(MultiValue), get_cpp_class<MultiValue>());
}

void initialize_array_script(){
Xemb((
	Array::block_first: method this.each.block_first;

Array::join: method(sep: ""){
	sep = sep.to_s;
	ms: MemoryStream();
	if(sep===""){
		this.each{
			ms.put_s(it.to_s);
		}
	}
	else{
		this.each{
			if(!first_step){
				ms.put_s(sep);
			}
			ms.put_s(it.to_s);
		}
	}
	return ms.to_s;
}

Array::to_s: method{
	return %f([%s])(this.join(", "));
}
MultiValue::to_s: method{
	return %f((%s))(this.join(", "));
}

),
"\x78\x74\x61\x6c\x01\x00\x00\x00\x00\x00\x01\x8e\x39\x00\x01\x89\x00\x01\x00\x0f\x0b\x2a\x00\x02\x2f\x00\x00\x01\x01\x00\x03\x25\x00\x37\x00\x03\x39\x00\x01\x89\x00\x02\x01\x2a\x53\x00\x00\x07\x28\x00\x01\x20\x00\x3f\x00\x00\x1d\x01\x2a\x00\x06\x20\x01\x39"
"\x00\x07\x2e\x00\x00\x01\x00\x20\x00\x1d\x01\x28\x00\x01\x4c\x00\x73\x13\x00\x70\x3f\x00\x02\x0b\x2a\x00\x02\x2f\x00\x00\x02\x00\x00\x03\x20\x00\x20\x01\x42\x00\x01\x1d\x01\x13\x00\x25\x1d\x00\x2a\x00\x06\x1d\x02\x2f\x01\x00\x00\x00\x00\x0a\x1d\x01\x2f\x00"
"\x00\x02\x00\x00\x0b\x20\x00\x20\x01\x1d\x01\x13\x00\x05\x15\xff\xdf\x44\x00\x2a\x43\x42\x00\x02\x3f\x00\x05\x20\x00\x1d\x00\x1d\x02\x31\x01\x00\x01\x00\x00\x0d\x14\x00\x05\x1d\x00\x8b\x41\x00\x05\x43\x1d\x01\x31\x00\x00\x00\x00\x00\x0e\x45\x41\x00\x02\x15"
"\x00\x86\x3f\x00\x07\x0b\x2a\x00\x02\x2f\x00\x00\x02\x00\x00\x03\x20\x00\x20\x01\x42\x00\x03\x1d\x01\x13\x00\x3e\x1d\x00\x2a\x00\x06\x1d\x02\x2f\x01\x00\x00\x00\x00\x0a\x1d\x01\x2f\x00\x00\x02\x00\x00\x0b\x20\x00\x20\x01\x1d\x01\x13\x00\x1e\x1d\x03\x1d\x02"
"\x2f\x01\x00\x00\x00\x00\x0a\x1d\x00\x2a\x00\x06\x1d\x02\x2f\x01\x00\x00\x00\x00\x0a\x15\xff\xd4\x44\x00\x2a\x43\x42\x00\x04\x3f\x00\x0c\x20\x00\x1d\x00\x1d\x02\x31\x01\x00\x01\x00\x00\x0d\x14\x00\x05\x1d\x00\x8b\x41\x00\x0c\x43\x1d\x01\x31\x00\x00\x00\x00"
"\x00\x0e\x45\x41\x00\x07\x1d\x00\x2f\x00\x00\x01\x01\x00\x06\x41\x00\x00\x25\x00\x37\x00\x12\x39\x00\x01\x89\x00\x03\x00\x17\x28\x00\x02\x0b\x2f\x01\x00\x01\x00\x00\x12\x28\x00\x03\x2e\x01\x00\x01\x01\x25\x00\x37\x00\x06\x39\x00\x13\x89\x00\x04\x00\x17\x28"
"\x00\x02\x0b\x2f\x01\x00\x01\x00\x00\x12\x28\x00\x04\x2e\x01\x00\x01\x01\x25\x00\x37\x00\x06\x25\x00\x8b\x00\x0f\x29\x00\x00\x00\x00\x01\x05\x00\x01\x00\x12\x00\x48\x00\x00\x00\x00\x01\x08\x00\x00\x00\x12\x00\x48\x00\x00\x00\x00\x01\x08\x00\x02\x00\x12\x00"
"\x5d\x00\x00\x00\x00\x01\x0a\x00\x00\x00\x12\x00\x62\x00\x00\x00\x00\x01\x0a\x00\x00\x00\x12\x00\x8c\x00\x00\x00\x00\x01\x0c\x00\x01\x00\x12\x00\xb6\x00\x00\x00\x00\x01\x0f\x00\x00\x00\x12\x00\xb6\x00\x00\x00\x00\x01\x0f\x00\x02\x00\x12\x00\xcb\x00\x00\x00"
"\x00\x01\x11\x00\x00\x00\x12\x00\xd0\x00\x00\x00\x00\x01\x11\x00\x00\x00\x12\x00\xf0\x00\x00\x00\x00\x01\x11\x00\x00\x00\x12\x00\xf0\x00\x00\x00\x00\x01\x11\x00\x00\x00\x12\x00\x13\x01\x00\x00\x00\x01\x11\x00\x01\x00\x12\x00\x53\x01\x00\x00\x00\x00\x13\x00"
"\x00\x00\x12\x00\x73\x01\x00\x00\x00\x00\x14\x00\x00\x00\x12\x00\x00\x00\x00\x05\x00\x00\x00\x00\x03\x06\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x08\x00\x00\x00\x05\x00\x02\x00\x00\x00\x00\x00\x00\x01\x00\x00\x20\x00\x00\x00\x05\x00\x04\x00\x01\x00\x00\x00"
"\x00\x01\x00\x01\x53\x01\x00\x00\x05\x00\x13\x00\x00\x00\x00\x00\x00\x01\x00\x00\x73\x01\x00\x00\x05\x00\x14\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x05\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x89\x00\x00\x00\xa6\x00\x00\x00\xb0\x00\x00\x00\x00\x00"
"\x00\x00\xa6\x00\x00\x00\xa6\x00\x00\x00\x10\x01\x00\x00\x2d\x01\x00\x00\x37\x01\x00\x00\x00\x00\x00\x00\x2d\x01\x00\x00\x2d\x01\x00\x00\x00\x2f\x00\x00\x00\x00\x1e\x00\x00\x00\x00\x00\x00\x00\x02\x00\x00\x00\x18\x00\x00\x00\x15\x00\x00\x00\x18\x00\x00\x00"
"\x04\x00\x00\x00\x1b\x00\x00\x00\x15\x00\x00\x00\x2c\x00\x00\x00\x05\x00\x00\x00\x33\x00\x00\x00\x06\x00\x00\x00\x3d\x00\x00\x00\x13\x00\x00\x00\x3d\x00\x00\x00\x07\x00\x00\x00\x48\x00\x00\x00\x0b\x00\x00\x00\x4b\x00\x00\x00\x08\x00\x00\x00\x5a\x00\x00\x00"
"\x0b\x00\x00\x00\x5d\x00\x00\x00\x08\x00\x00\x00\x62\x00\x00\x00\x0a\x00\x00\x00\x62\x00\x00\x00\x09\x00\x00\x00\x70\x00\x00\x00\x0a\x00\x00\x00\x70\x00\x00\x00\x08\x00\x00\x00\x85\x00\x00\x00\x0b\x00\x00\x00\xb6\x00\x00\x00\x13\x00\x00\x00\xb9\x00\x00\x00"
"\x0d\x00\x00\x00\xc8\x00\x00\x00\x13\x00\x00\x00\xcb\x00\x00\x00\x0d\x00\x00\x00\xd0\x00\x00\x00\x12\x00\x00\x00\xd0\x00\x00\x00\x10\x00\x00\x00\xd0\x00\x00\x00\x11\x00\x00\x00\xde\x00\x00\x00\x12\x00\x00\x00\xde\x00\x00\x00\x0d\x00\x00\x00\xf0\x00\x00\x00"
"\x12\x00\x00\x00\xf0\x00\x00\x00\x10\x00\x00\x00\xf0\x00\x00\x00\x0f\x00\x00\x00\xfb\x00\x00\x00\x10\x00\x00\x00\xfb\x00\x00\x00\x11\x00\x00\x00\x09\x01\x00\x00\x12\x00\x00\x00\x0c\x01\x00\x00\x13\x00\x00\x00\x3a\x01\x00\x00\x14\x00\x00\x00\x46\x01\x00\x00"
"\x15\x00\x00\x00\x4b\x01\x00\x00\x19\x00\x00\x00\x4b\x01\x00\x00\x17\x00\x00\x00\x4e\x01\x00\x00\x19\x00\x00\x00\x53\x01\x00\x00\x18\x00\x00\x00\x66\x01\x00\x00\x19\x00\x00\x00\x6b\x01\x00\x00\x1c\x00\x00\x00\x6b\x01\x00\x00\x1a\x00\x00\x00\x6e\x01\x00\x00"
"\x1c\x00\x00\x00\x73\x01\x00\x00\x1b\x00\x00\x00\x86\x01\x00\x00\x1c\x00\x00\x00\x8b\x01\x00\x00\x1e\x00\x00\x00\x00\x01\x0b\x00\x00\x00\x03\x09\x00\x00\x00\x06\x73\x6f\x75\x72\x63\x65\x09\x00\x00\x00\x11\x74\x6f\x6f\x6c\x2f\x74\x65\x6d\x70\x2f\x69\x6e\x2e"
"\x78\x74\x61\x6c\x09\x00\x00\x00\x0b\x69\x64\x65\x6e\x74\x69\x66\x69\x65\x72\x73\x0a\x00\x00\x00\x14\x09\x00\x00\x00\x00\x09\x00\x00\x00\x05\x41\x72\x72\x61\x79\x09\x00\x00\x00\x04\x65\x61\x63\x68\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x66\x69\x72\x73"
"\x74\x09\x00\x00\x00\x03\x73\x65\x70\x09\x00\x00\x00\x02\x6d\x73\x09\x00\x00\x00\x04\x74\x6f\x5f\x73\x09\x00\x00\x00\x0c\x4d\x65\x6d\x6f\x72\x79\x53\x74\x72\x65\x61\x6d\x09\x00\x00\x00\x08\x69\x74\x65\x72\x61\x74\x6f\x72\x09\x00\x00\x00\x02\x69\x74\x09\x00"
"\x00\x00\x05\x70\x75\x74\x5f\x73\x09\x00\x00\x00\x0a\x62\x6c\x6f\x63\x6b\x5f\x6e\x65\x78\x74\x09\x00\x00\x00\x01\x65\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x63\x61\x74\x63\x68\x09\x00\x00\x00\x0b\x62\x6c\x6f\x63\x6b\x5f\x62\x72\x65\x61\x6b\x02\x00\x00"
"\x00\x0e\x02\x00\x00\x00\x0f\x02\x00\x00\x00\x12\x09\x00\x00\x00\x04\x6a\x6f\x69\x6e\x09\x00\x00\x00\x0a\x4d\x75\x6c\x74\x69\x56\x61\x6c\x75\x65\x09\x00\x00\x00\x06\x76\x61\x6c\x75\x65\x73\x0a\x00\x00\x00\x05\x03\x02\x00\x00\x00\x06\x09\x00\x00\x00\x02\x2c"
"\x20\x00\x09\x00\x00\x00\x14\x6c\x69\x62\x3a\x3a\x62\x75\x69\x6c\x74\x69\x6e\x3a\x3a\x46\x6f\x72\x6d\x61\x74\x0b\x00\x00\x00\x02\x09\x00\x00\x00\x11\x6c\x69\x62\x3a\x3a\x62\x75\x69\x6c\x74\x69\x6e\x3a\x3a\x41\x6e\x79\x03\x02\x00\x00\x00\x1b\x09\x00\x00\x00"
"\x04\x5b\x25\x73\x5d\x00\x02\x00\x00\x00\x1b\x0b\x00\x00\x00\x02\x02\x00\x00\x00\x1d\x03\x02\x00\x00\x00\x1b\x09\x00\x00\x00\x04\x28\x25\x73\x29"
)->call();

}

}
