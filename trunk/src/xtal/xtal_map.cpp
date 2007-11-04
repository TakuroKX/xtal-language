#include "xtal.h"
#include "xtal_macro.h"

namespace xtal{

class MapIter : public Base{
	MapPtr map_;
	Map::iterator node_;
	int_t type_;

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & map_;
	}

public:

	MapIter(const MapPtr& m, int_t type)
		:map_(m), type_(type), node_(m->begin()){
	}
	
	void block_next(const VMachinePtr& vm){
		if(node_==map_->end()){
			vm->return_result(null, null);
			return;
		}
		
		switch(type_){
			case 0: vm->return_result(SmartPtr<MapIter>(this), node_->first, node_->second); break;
			case 1: vm->return_result(SmartPtr<MapIter>(this), node_->first); break;
			case 2: vm->return_result(SmartPtr<MapIter>(this), node_->second); break;
			case 3:
				for(;;){
					if(node_->second){
						vm->return_result(SmartPtr<MapIter>(this), node_->first);
						++node_;
						return;
					}

					++node_;

					if(node_==map_->end()){
						vm->return_result(null, null);
						return;
					}
				}
				break;
			default: vm->return_result(null, null); break;
		}

		++node_;
	}
};


void initialize_map(){
	{
		ClassPtr p = new_cpp_class<MapIter>("MapIter");
		p->inherit(Iterator());
		p->method("block_next", &MapIter::block_next);
	}

	{
		ClassPtr p = new_cpp_class<Map>("Map");
		p->inherit(Enumerator());
		
		p->def("new", ctor<Map>());
		p->method("to_s", &Map::to_s);
		p->method("to_m", &Map::to_m);
		p->method("size", &Map::size);
		p->method("length", &Map::length);
		p->method("insert", &Map::insert);
		p->method("each", &Map::pairs);
		p->method("pairs", &Map::pairs);
		p->method("keys", &Map::keys);
		p->method("values", &Map::values);
		p->method("clone", &Map::clone);
		p->method("erase", &Map::erase);
		p->method("empty", &Map::empty);

		p->method("op_at", &Map::at, get_cpp_class<Any>());
		p->method("op_set_at", &Map::set_at, get_cpp_class<Any>());
		p->method("op_eq", &Map::op_eq, get_cpp_class<Map>());
		p->method("op_cat", &Map::cat, get_cpp_class<Map>());
		p->method("op_cat_assign", &Map::cat_assign, get_cpp_class<Map>());
	}

	{
		ClassPtr p = new_cpp_class<Set>("Set");
		p->inherit(get_cpp_class<Map>());
		
		p->method("each", &Set::each);
	}

	builtin()->def("Map", get_cpp_class<Map>());
	builtin()->def("Set", get_cpp_class<Set>());
}

void Map::visit_members(Visitor& m){
	Base::visit_members(m);
	m & table_;
}	

const AnyPtr& Map::calc_key(const AnyPtr& key){
	if(type(key)==TYPE_BASE){
		if(const StringPtr& str = as<const StringPtr&>(key)){
			return str->intern();
		}
	}
	return key;
}
	
const AnyPtr& Map::at(const AnyPtr& akey){
	const AnyPtr& key = calc_key(akey);
	iterator it = table_.find(key);
	if(it!=end()){
		return it->second;
	}else{
		return default_value_;
	}
}

MapPtr Map::cat(const MapPtr& a){
	MapPtr ret(clone());
	ret->cat_assign(a);
	return ret;
}

MapPtr Map::cat_assign(const MapPtr& a){
	for(iterator p = a->begin(); p!=a->end(); ++p){
		set_at(p->first, p->second);
	}
	return MapPtr(this);
}

StringPtr Map::to_s(){
	if(empty())
		return xnew<String>("[:]");

	MemoryStreamPtr ms = xnew<MemoryStream>();
	ms->put_s("[");
	for(iterator p = begin(); p!=end(); ++p){
		if(p!=begin()){
			ms->put_s(",");
		}

		ms->put_s(p->first->to_s());
		ms->put_s(":");
		ms->put_s(p->second->to_s());
	}
	ms->put_s("]");
	return ms->to_s();
}

bool Map::op_eq(const MapPtr& a){
	if(size()!=a->size())
		return false;
	
	for(iterator p = a->begin(); p!=a->end(); ++p){
		if(at(p->first)!=p->second){
			return false;
		}
	}
	return true;
}
	
AnyPtr Map::pairs(){
	return xnew<MapIter>(MapPtr(this), 0);
}

AnyPtr Map::keys(){
	return xnew<MapIter>(MapPtr(this), 1);
}

AnyPtr Map::values(){
	return xnew<MapIter>(MapPtr(this), 2);
}

MapPtr Map::clone(){
	MapPtr ret(xnew<Map>());
	for(iterator p = begin(); p!=end(); ++p){
		ret->set_at(p->first, p->second);
	}	
	return ret;
}

void Map::push_all(const VMachinePtr& vm){
	for(iterator p = begin(); p!=end(); ++p){
		vm->push(p->first);
		vm->push(p->second);
	}	
}


AnyPtr Set::each(){
	return xnew<MapIter>(MapPtr(this), 3);
}


}
