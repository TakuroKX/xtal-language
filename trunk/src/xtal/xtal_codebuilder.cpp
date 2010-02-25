#include "xtal.h"
#include "xtal_macro.h"

#include "xtal_codebuilder.h"

#ifndef XTAL_NO_PARSER

namespace xtal{
	
CodeBuilder::CodeBuilder(){
	implicit_ref_map_ = XNew<Map>();
}

CodeBuilder::~CodeBuilder(){}

void CodeBuilder::error(const AnyPtr& message){
	parser_.executor_->error(message, lineno());
}

CodePtr CodeBuilder::compile(const StreamPtr& stream, const StringPtr& source_file_name){
	xpeg::ExecutorPtr exec = xnew<xpeg::Executor>(stream, source_file_name);
	parser_.parse(exec);

	if(!stream->eos()){
		error(Xt("XCE1001"));
	}

	if(exec->has_error()){
		return nul<Code>();
	}

	prev_inst_op_ = -1;
	eval_ = false;

	return compile_toplevel(ep(exec->tree_back()), source_file_name);
}

CodePtr CodeBuilder::compile(const xpeg::ExecutorPtr& executor, const StringPtr& source_file_name){
	parser_.parse(executor);

	if(executor->has_error()){
		return nul<Code>();
	}

	prev_inst_op_ = -1;
	eval_ = false;
	return compile_toplevel(ep(executor->tree_back()), source_file_name);
}

CodePtr CodeBuilder::eval_compile(const xpeg::ExecutorPtr& executor){
	parser_.parse_eval(executor);
	
	if(executor->has_error()){
		return nul<Code>();
	}

	ExprPtr e = ep(executor->tree_back());

	if(e->itag()==EXPR_DEFINE){
		if(e->bin_lhs()->itag()==EXPR_LVAR){
			eb_.push(Xid(filelocal));
			eb_.splice(EXPR_LVAR, 1);
			eb_.push(e->bin_lhs()->lvar_name());
			eb_.push(null);
			eb_.splice(EXPR_MEMBER, 3);
			e->set_bin_lhs(ep(eb_.pop()));
		}
	}
	else if(e->itag()==EXPR_MDEFINE){
		ExprPtr lhs = e->mdefine_lhs_exprs();
		for(uint_t i=0; i<lhs->size(); ++i){
			ExprPtr e2 = ep(lhs->at(i));
			if(e2->itag()==EXPR_LVAR){
				eb_.push(Xid(filelocal));
				eb_.splice(EXPR_LVAR, 1);
				eb_.push(e2->lvar_name());
				eb_.push(null);
				eb_.splice(EXPR_MEMBER, 3);
				lhs->set_at(i, ep(eb_.pop()));
			}
		}
	}
	else{
		eb_.push(e);
		eb_.splice(EXPR_LIST, 1);
		eb_.splice(EXPR_RETURN, 1);
		e = ep(eb_.pop());
	}

	prev_inst_op_ = -1;
	eval_ = true;
	return compile_eval_toplevel(e, "<eval>");
}

CodePtr CodeBuilder::compile_eval_toplevel(const ExprPtr& e, const StringPtr& source_file_name){
	result_ = XNew<Code>();
	result_->source_file_name_ = source_file_name;
	result_->except_info_table_.push_back(ExceptInfo());

	identifier_map_ = XNew<Map>();
	register_identifier(empty_id);

	value_map_ = XNew<Map>();
	register_value(null);
	
	result_->once_table_.push_back(undefined);

	linenos_.push(0);

	// 関数フレームを作成する
	fun_frames_.push();	
	ff().labels.clear();
	ff().loops.clear();
	ff().finallies.clear();
	ff().var_frame_count = var_frames_.size();
	ff().stack_count = 0;
	ff().max_stack_count = 0;
	ff().variable_count = 1;
	ff().max_variable_count = 1;
	ff().extendable_param = true;

	// 変数フレームを作成して、コマンドライン引数を登録する
	var_begin(VarFrame::FUN);

	// コマンドライン引数
	var_define(Xid(arg));

	// 関数コアを作成
	FunInfo info;
	info.pc = 0;
	info.kind = KIND_METHOD;
	info.min_param_count = 0;
	info.max_param_count = 0;
	info.name_number = register_identifier(Xid(toplevel));
	info.flags = FunInfo::FLAG_EXTENDABLE_PARAM;

	// 引数の名前を識別子テーブルに順番に乗せる
	info.variable_size = vf().entries.size();
	info.variable_identifier_offset = result_->identifier_table_.size();
	for(uint_t i=0; i<vf().entries.size(); ++i){
		vf().entries[i].initialized = true;
		result_->identifier_table_.push_back(vf().entries[i].name);
	}

	int_t fun_info_table_number = 0;
	result_->xfun_info_table_.push_back(info);

	//var_begin(VarFrame::FRAME);
	//scope_begin();	

	// 関数本体を処理する
	compile_stmt(e);

	//scope_end();
	//var_end();
	
	break_off(ff().var_frame_count+1);

	put_inst(InstReturn(0));
	
	result_->set_lineno_info(result_->final_lineno()+1);

	put_inst(InstThrow());

	process_labels();
	
	if(vf().scope_chain){
		result_->xfun_info_table_[fun_info_table_number].flags |= FunInfo::FLAG_SCOPE_CHAIN;
	}

	// 関数フレームをポップする
	fun_frames_.downsize(1);

	// 変数フレームをポップする
	var_end();

	Xfor2(k, v, implicit_ref_map_){
		if(v){
			Code::ImplcitInfo info = {k->to_i(), v->to_i()};
			result_->implicit_table_.push_back(info);
		}
	}

	if(!parser_.executor_->errors()){
		opt_jump();
		result_->generated();
		return result_;
	}
	else{
		result_ = null;
		return nul<Code>();
	}
}

CodePtr CodeBuilder::compile_toplevel(const ExprPtr& e, const StringPtr& source_file_name){
	result_ = XNew<Code>();

	result_->source_file_name_ = source_file_name;
	result_->except_info_table_.push_back(ExceptInfo());

	identifier_map_ = XNew<Map>();
	register_identifier(empty_id);

	value_map_ = XNew<Map>();
	register_value(null);
	
	result_->once_table_.push_back(undefined);

	linenos_.push(0);

	// 関数フレームを作成する
	fun_frames_.push();	
	ff().labels.clear();
	ff().loops.clear();
	ff().finallies.clear();
	ff().var_frame_count = var_frames_.size();
	ff().stack_count = 0;
	ff().max_stack_count = 0;
	ff().variable_count = 1;
	ff().max_variable_count = 1;
	ff().extendable_param = true;

	// 変数フレームを作成して、コマンドライン引数を登録する
	var_begin(VarFrame::FUN);

	// コマンドライン引数
	var_define(Xid(arg));

	// 関数コアを作成
	FunInfo info;
	info.pc = 0;
	info.kind = KIND_FUN;
	info.min_param_count = 0;
	info.max_param_count = 0;
	info.name_number = register_identifier(Xid(toplevel));
	info.flags = FunInfo::FLAG_EXTENDABLE_PARAM;

	// 引数の名前を識別子テーブルに順番に乗せる
	info.variable_size = vf().entries.size();
	info.variable_identifier_offset = result_->identifier_table_.size();
	for(uint_t i=0; i<vf().entries.size(); ++i){
		vf().entries[i].initialized = true;
		result_->identifier_table_.push_back(vf().entries[i].name);
	}

	int_t fun_info_table_number = 0;
	result_->xfun_info_table_.push_back(info);

	//var_begin(VarFrame::FRAME);
	//scope_begin();	

	// 関数本体を処理する
	compile_stmt(e);

	//scope_end();
	//var_end();
	
	break_off(ff().var_frame_count+1);

	eb_.set_lineno(result_->final_lineno());
	eb_.push(Xid(filelocal));
	eb_.splice(EXPR_LVAR, 1);
	eb_.splice(0, 1);
	eb_.splice(EXPR_RETURN, 1);
	compile_stmt(eb_.pop());
	
	result_->set_lineno_info(result_->final_lineno()+1);

	put_inst(InstThrow());

	process_labels();
	
	if(vf().scope_chain){
		result_->xfun_info_table_[fun_info_table_number].flags |= FunInfo::FLAG_SCOPE_CHAIN;
	}

	// 関数フレームをポップする
	fun_frames_.downsize(1);

	// 変数フレームをポップする
	var_end();

	Xfor2(k, v, implicit_ref_map_){
		if(v){
			Code::ImplcitInfo info = {k->to_i(), v->to_i()};
			result_->implicit_table_.push_back(info);
		}
	}

	if(!parser_.executor_->errors()){
		opt_jump();
		result_->generated();
		return result_;
	}
	else{
		result_ = null;
		return null;
	}
}

inst_address_t CodeBuilder::calc_address(const inst_t* pc, inst_address_t address){
	const inst_t* pc2 = pc + address;
	if(*pc2==InstGoto::NUMBER){
		InstGoto& inst2 = *(InstGoto*)pc2;
		return pc2+inst2.address - pc;
	}

	return address;
}

void CodeBuilder::opt_jump(){
	const inst_t* begin = result_->data();
	inst_t* pc = (inst_t*)begin;
	const inst_t* end = result_->data() + result_->size(); 

	while(pc<end){
		switch(*pc){
			XTAL_DEFAULT{}

			XTAL_CASE(InstIf::NUMBER){
				InstIf& inst = *(InstIf*)pc;
				inst.address_true = calc_address(pc, inst.address_true);
				inst.address_false = calc_address(pc, inst.address_false);
			}

			XTAL_CASE(InstGoto::NUMBER){
				InstGoto& inst = *(InstGoto*)pc;
				inst.address = calc_address(pc, inst.address);
			}
		}

		int_t size = inst_size(*pc);
		if(size==0){
			break;
		}

		pc += inst_size(*pc);
	}
}

AnyPtr CodeBuilder::errors(){
	return parser_.executor_->errors();
}

void CodeBuilder::put_inst2(const Inst& t, uint_t sz){
	if(t.op==255){
		error(Xt("XCE1027"));
	}

	prev_inst_op_ = t.op;

	size_t cur = result_->code_.size();
	result_->code_.resize(cur+sz/sizeof(inst_t));
	std::memcpy(&result_->code_[cur], &t, sz);
}

int_t CodeBuilder::register_identifier(const IDPtr& v){
	if(!v){ return 0; }
	if(const AnyPtr& pos = identifier_map_->at(v)){ return pos->to_i(); }
	result_->identifier_table_.push_back(v);
	identifier_map_->set_at(v, result_->identifier_table_.size()-1);
	return result_->identifier_table_.size()-1;
}

int_t CodeBuilder::register_value(const AnyPtr& v){
	if(const AnyPtr& pos = value_map_->at(v)){ return pos->to_i(); }
	result_->value_table_.push_back(v);
	value_map_->set_at(v, result_->value_table_.size()-1);
	return result_->value_table_.size()-1;
}

int_t CodeBuilder::append_identifier(const IDPtr& identifier){
	result_->identifier_table_.push_back(identifier);
	return result_->identifier_table_.size()-1;
}

int_t CodeBuilder::append_value(const AnyPtr& v){
	result_->value_table_.push_back(v);
	return result_->value_table_.size()-1;
}

int_t CodeBuilder::lookup_instance_variable(const IDPtr& key, bool must){
	if(!class_frames_.empty()){
		int ret = 0;
		ClassFrame& cf = class_frames_.top();
		for(size_t i = 0, last = cf.entries.size(); i<last; ++i){
			if(raweq(cf.entries[i].name, key)){
				return ret;
			}
			ret++;
		}
	}

	if(must || !eval_){
		error(Xt("XCE1023")->call(Named(Xid(name), Xid(_)->cat(key))));
		return 0;
	}

	return -1;
}

void CodeBuilder::put_set_instance_variable_code(int_t value, const IDPtr& var){
	int_t n = lookup_instance_variable(var, false);
	if(n<0){
		put_inst(InstSetInstanceVariableByName(value, register_identifier(var)));
	}
	else{
		put_inst(InstSetInstanceVariable(value, lookup_instance_variable(var), class_info_num()));
	}
}

void CodeBuilder::put_instance_variable_code(int_t result, const IDPtr& var){
	int_t n = lookup_instance_variable(var, false);
	if(n<0){
		put_inst(InstInstanceVariableByName(result, register_identifier(var)));
	}
	else{
		put_inst(InstInstanceVariable(result, lookup_instance_variable(var), class_info_num()));
	}
}

int_t CodeBuilder::reserve_label(){
	ff().labels.resize(fun_frames_.top().labels.size()+1);
	ff().labels[ff().labels.size()-1].pos = -1;
	return ff().labels.size()-1;
}

void CodeBuilder::set_label(int_t labelno){
	prev_inst_op_ = -1;
	ff().labels[labelno].pos = code_size();
}

void CodeBuilder::set_jump(int_t offset, int_t labelno){
	FunFrame::Label::From f;
	f.lineno = linenos_.top();
	f.pos = code_size();
	f.set_pos = f.pos + offset/sizeof(inst_t);
	ff().labels[labelno].froms.push_back(f);
}

void CodeBuilder::process_labels(){
	for(size_t i = 0; i<ff().labels.size(); ++i){
		FunFrame::Label &l = ff().labels[i];
		XTAL_ASSERT(l.pos!=-1);
		for(size_t j = 0; j<l.froms.size(); ++j){
			FunFrame::Label::From &f = l.froms[j];
			inst_address_t& buf = *(inst_address_t*)&result_->code_[f.set_pos];
			buf = l.pos - f.pos; //l.pos - f.set_pos;
		}
	}
	ff().labels.clear();
}

void CodeBuilder::break_off(int_t n){
	for(uint_t scope_count = var_frames_.size(); scope_count!=(uint_t)n; scope_count--){
		for(uint_t k = 0; k<ff().finallies.size(); ++k){
			if((uint_t)ff().finallies[k].frame_count==scope_count){
				int_t label = reserve_label();
				set_jump(InstPushGoto::OFFSET_address, label);
				put_inst(InstPushGoto());
				put_inst(InstTryEnd());
				set_label(label);
			}
		}

		VarFrame& vf = var_frames_[var_frames_.size()-scope_count];
		if(vf.real_entry_num!=0 && vf.kind!=VarFrame::TOPLEVEL && vf.kind==VarFrame::FRAME){
			put_inst(InstScopeEnd());
		}
	}
}

bool CodeBuilder::is_comp_bin(const ExprPtr& e){
	if(e && EXPR_EQ<=e->itag() && e->itag()<=EXPR_NIS){
		return true;
	}	
	return false;
}

void CodeBuilder::scope_chain(int_t var_frame_size){
	for(int_t i=0; i<var_frame_size; ++i){
		var_frames_[i].scope_chain = true;
	}
}

CodeBuilder::LVarInfo CodeBuilder::var_find(const IDPtr& key, bool define, bool traceless, int_t number){
	LVarInfo ret = {0, 0, 0, 0, 0, false, false};
	for(size_t i = 0, last = var_frames_.size(); i<last; ++i){
		VarFrame& vf = var_frames_[i];

		for(size_t j = 0, jlast = vf.entries.size(); j<jlast; ++j){
			VarFrame::Entry& entry = vf.entries[vf.entries.size()-1-j];

			if(raweq(entry.name, key) && (number<0 || entry.number<0 || number==entry.number)){
				bool out_of_fun = (uint_t)vf.fun_frames_size!=fun_frames_.size();

				if(out_of_fun || entry.initialized || define){
					ret.out_of_fun = out_of_fun;
					ret.var_frame = var_frames_.size()-1-i;
					ret.entry = vf.entries.size()-1-j;

					if(!traceless){
						if(define){ entry.initialized = true; }
						if(vf.kind!=VarFrame::TOPLEVEL){
							scope_chain(i);
						}
					}

					for(size_t k = 0, klast = vf.entries.size()-1-j; k<klast; ++k){
						//VarFrame::Entry& entry = vf.entries[k];
						ret.pos++;
					}

					if(!define && ret.depth==0 && vf.kind==VarFrame::CLASS){
						ret.out_of_fun = true;
					}

					ret.vpos--;

					if(vf.kind==VarFrame::TOPLEVEL){
						ret.toplevel = true;
						ret.vpos = ret.entry;
						ret.pos = -1;
					}

					return ret;
				}
			}

			if(vf.kind!=VarFrame::TOPLEVEL){
				ret.vpos--;
			}
		}

		if(vf.kind==VarFrame::FRAME){
			for(size_t k = 0, klast = vf.entries.size(); k<klast; ++k){
				//VarFrame::Entry& entry = vf.entries[k];
				ret.depth++;
				break;
			}
		}
		else if(vf.kind==VarFrame::TOPLEVEL){

		}
		else{
			ret.depth++;
		}
	}

	ret.pos = -1;
	return ret;
}

void CodeBuilder::var_begin(int_t kind){
	var_frames_.push();
	vf().entries.clear();
	vf().scope_info_num = 0;
	vf().kind = kind;
	vf().fun_frames_size = fun_frames_.size();
	vf().scope_chain = false;
}

void CodeBuilder::var_define_stmts(const ExprPtr& stmts){
	Xfor(v, stmts){
		var_define_stmt(v);
	}
}

void CodeBuilder::var_define_stmt(const AnyPtr& stmt){
	if(ExprPtr v = ptr_cast<Expr>(stmt)){
		if(v->itag()==EXPR_DEFINE){
			if(v->bin_lhs()->itag()==EXPR_LVAR){
				var_define(v->bin_lhs()->lvar_name(), v->bin_rhs());

				if(v->bin_rhs()->itag()==EXPR_CLASS){
					v->bin_rhs()->set_class_name(v->bin_lhs()->lvar_name());
				}
				else if(v->bin_rhs()->itag()==EXPR_FUN){
					v->bin_rhs()->set_fun_name(v->bin_lhs()->lvar_name());
				}
			}
		}
		else if(v->itag()==EXPR_MDEFINE){
			Xfor(v1, v->mdefine_lhs_exprs()){
				ExprPtr vv = ep(v1);
				if(vv->itag()==EXPR_LVAR){
					var_define(vv->lvar_name());
				}			
			}
		}
	}
}

void CodeBuilder::var_define(const IDPtr& name, const ExprPtr& expr, int_t accessibility, bool define, bool constant, bool assign, int_t number){
	if(number<0){
		for(size_t j = 0, jlast = vf().entries.size(); j<jlast; ++j){
			if(raweq(vf().entries[vf().entries.size()-1-j].name, name)){
				error(Xt("XCE1026")->call(Named(Xid(name), name)));
				return;
			}
		}
	}

	VarFrame::Entry entry;
	entry.name = name;
	entry.expr = expr;
	entry.value = undefined;
	entry.constant = constant;
	entry.initialized = define;
	entry.accessibility = accessibility;
	entry.assigned = assign;
	entry.referenced = false;
	entry.number = number;
	vf().entries.push_back(entry);
}

void CodeBuilder::var_end(){
	var_frames_.downsize(1);
}

void CodeBuilder::scope_begin(){
	int_t scope_info_num = result_->scope_info_table_.size();

	ScopeInfo info;
	info.pc = code_size();

	ff().variable_count += vf().entries.size();
	if(ff().max_variable_count<ff().variable_count){
		ff().max_variable_count = ff().variable_count;
	}

	int_t real_entry_num = vf().entries.size();

	vf().real_entry_num = real_entry_num;
	vf().scope_info_num = scope_info_num;

	info.variable_size = real_entry_num;
	info.variable_identifier_offset = result_->identifier_table_.size();
	for(uint_t i=0; i<vf().entries.size(); ++i){
		VarFrame::Entry& entry = vf().entries[i];
		result_->identifier_table_.push_back(entry.name);
	}

	if(vf().real_entry_num!=0 && vf().kind!=VarFrame::TOPLEVEL){
		put_inst(InstScopeBegin(scope_info_num));
	}

	ff().variable_count += vf().real_entry_num;

	result_->scope_info_table_.push_back(info);
}

void CodeBuilder::scope_end(){
	
	ff().variable_count -= vf().real_entry_num;

	if(vf().real_entry_num!=0 && vf().kind!=VarFrame::TOPLEVEL){
		put_inst(InstScopeEnd());
	}
		
	if(vf().scope_chain){
		result_->scope_info_table_[vf().scope_info_num].flags |= FunInfo::FLAG_SCOPE_CHAIN;
	}
}

int_t CodeBuilder::code_size(){
	return result_->code_.size();
}

void CodeBuilder::compile_comp_bin_assert(const AnyPtr& f, const ExprPtr& e, const ExprPtr& str, const ExprPtr& mes, int_t label, int_t stack_top){
	if(is_comp_bin(e->bin_lhs()) || is_comp_bin(e->bin_rhs())){
		error(Xt("XCE1025"));
	}

	int_t target = stack_top++;
	int_t lhs = compile_expr(e->bin_lhs(), stack_top);
	int_t rhs = compile_expr(e->bin_rhs(), stack_top);
	
	{
		int_t label_false = reserve_label();
		put_if_code(e->itag(), target, lhs, rhs, label, label_false, stack_top);

		int_t label_true = reserve_label();
		put_inst(InstLoadConstant(target, LOAD_TRUE));

		set_jump(InstGoto::OFFSET_address, label_true);
		put_inst(InstGoto());

		set_label(label_false);
		put_inst(InstLoadValue(target, LOAD_FALSE));
		set_label(label_true);
	}

	int_t label_true = reserve_label();
	set_jump(InstIf::OFFSET_address_false, label_true);
	set_jump(InstIf::OFFSET_address_true, label);
	put_inst(InstIf(target, 0, 0));
	set_label(label_true);

	int_t vart = stack_top++;
	put_inst(InstLoadConstant(vart, register_value(f)));

	int_t strt = stack_top++;
	if(str){ compile_expr(str, stack_top, strt); }
	else{ put_inst(InstLoadConstant(strt, register_value(empty_string))); }
	
	int_t lhst = stack_top++;
	put_inst(InstCopy(lhst, lhs));

	int_t rhst = stack_top++;
	put_inst(InstCopy(rhst, rhs));

	int_t mest = stack_top++;
	if(mes){ compile_expr(mes, stack_top, mest); }
	else{ put_inst(InstLoadConstant(mest, register_value(empty_string))); }

	put_inst(InstCall(target, 1, vart, 0, vart+1, 4, 0, 0));
	put_inst(InstAssert(target));
}	

void CodeBuilder::put_bin(int_t result, const ExprPtr& e, int_t a, int_t b, int_t stack_top){
	int_t tag = e->itag();
	if(tag>=EXPR_ADD && tag<=EXPR_MOD){	
		put_inst(InstArith(result, ARITH_ADD + tag - EXPR_ADD, a, b, stack_top));
		return;
	}

	if(tag>=EXPR_ADD_ASSIGN && tag<=EXPR_MOD_ASSIGN){	
		put_inst(InstArith(result, ARITH_ADD_ASSIGN + tag - EXPR_ADD_ASSIGN, a, b, stack_top));
		return;
	}

	if(tag>=EXPR_AND && tag<=EXPR_USHR){	
		put_inst(InstBitwise(result, BITWISE_AND + tag - EXPR_AND, a, b, stack_top));
		return;
	}

	if(tag>=EXPR_AND_ASSIGN && tag<=EXPR_USHR_ASSIGN){	
		put_inst(InstBitwise(result, BITWISE_AND_ASSIGN + tag - EXPR_AND_ASSIGN, a, b, stack_top));
		return;
	}
}

void CodeBuilder::compile_op_assign(const ExprPtr& e, int_t stack_top){
	ExprPtr lhs = e->bin_lhs();
	ExprPtr rhs = e->bin_rhs();

	if(lhs->itag()==EXPR_LVAR){
		int_t a = compile_expr(lhs, stack_top);
		int_t b = compile_expr(rhs, stack_top);
		if(a<0){
			put_bin(a, e, a, b, stack_top);
		}
		else{
			put_bin(a, e, a, b, stack_top);
			compile_lassign(a, lhs->lvar_name());
		}
	}
	else if(lhs->itag()==EXPR_IVAR){
		int_t a = compile_expr(lhs, stack_top);
		int_t b = compile_expr(rhs, stack_top);
		int_t target = stack_top++;
		put_bin(target, e, a, b, stack_top);
		put_set_instance_variable_code(target, lhs->ivar_name());
	}
	else if(lhs->itag()==EXPR_PROPERTY){
		AnyPtr sec;

		int_t term = compile_expr(lhs->property_term(), stack_top);

		if(lhs->property_ns()){
			sec = compile_expr(lhs->property_ns(), stack_top);
		}

		int_t value = compile_expr(rhs, stack_top);
		int_t target = stack_top++;

		compile_property(term, lhs->property_name(), sec, 0, stack_top, target);
		put_bin(target, e, target, value, stack_top);
		compile_set_property(term, lhs->property_name(), sec, target, 0, stack_top, target, 0);
	}
	else if(lhs->itag()==EXPR_PROPERTY_Q){
		AnyPtr sec;

		int_t term = compile_expr(lhs->property_term(), stack_top);

		if(lhs->property_ns()){
			sec = compile_expr(lhs->property_ns(), stack_top);
		}

		int_t value = compile_expr(rhs, stack_top);
		int_t target = stack_top++;

		compile_property(term, lhs->property_name(), sec, MEMBER_FLAG_Q_BIT, stack_top, target);
		put_bin(target, e, target, value, stack_top);
		compile_set_property(term, lhs->property_name(), sec, target, MEMBER_FLAG_Q_BIT, stack_top, target, 0);
	}
	else if(lhs->itag()==EXPR_AT){
		int_t var = compile_expr(lhs->bin_lhs(), stack_top);
		int_t key = compile_expr(lhs->bin_rhs(), stack_top);
		int_t value = compile_expr(rhs, stack_top);
		int_t target = stack_top++;

		put_inst(InstAt(target, var, key, stack_top));
		put_bin(value, e, target, value, stack_top);
		//put_inst(InstArith(value, ARITH_ADD_ASSIGN + e->itag() - EXPR_ADD_ASSIGN, target, value, stack_top));
		put_inst(InstSetAt(var, key, value, stack_top));
	}

}

void CodeBuilder::put_incdec(const ExprPtr& e, int_t ret, int_t target, int_t stack_top){
	if(e->itag()==EXPR_INC)
		put_inst(InstUna(ret, UNA_INC, target, stack_top));
	else
		put_inst(InstUna(ret, UNA_DEC, target, stack_top));
}

void CodeBuilder::compile_incdec(const ExprPtr& e, int_t stack_top){
	ExprPtr term = e->una_term();

	if(term->itag()==EXPR_LVAR){
		int_t a = compile_expr(term, stack_top);
		if(a<0){
			put_incdec(e, a, a, stack_top);
		}
		else{
			put_incdec(e, a, a, stack_top);
			compile_lassign(a, term->lvar_name());
		}
	}
	else if(term->itag()==EXPR_IVAR){
		int_t a = compile_expr(term, stack_top);
		put_incdec(e, a, a, stack_top);
		put_set_instance_variable_code(a, term->ivar_name());
	}
	else if(term->itag()==EXPR_PROPERTY){
		AnyPtr sec;

		int_t nterm = compile_expr(term->property_term(), stack_top);

		if(term->property_ns()){
			sec = compile_expr(term->property_ns(), stack_top);
		}

		int_t target = stack_top++;

		compile_property(nterm, term->property_name(), sec, 0, stack_top, target);
		put_incdec(e, target, target, stack_top);
		compile_set_property(nterm, term->property_name(), sec, target, 0, stack_top, target, 0);
	}
	else if(term->itag()==EXPR_PROPERTY_Q){
		AnyPtr sec;

		int_t nterm = compile_expr(term->property_term(), stack_top);

		if(term->property_ns()){
			sec = compile_expr(term->property_ns(), stack_top);
		}

		int_t target = stack_top++;

		compile_property(nterm, term->property_name(), sec, MEMBER_FLAG_Q_BIT, stack_top, target);
		put_incdec(e, target, target, stack_top);
		compile_set_property(nterm, term->property_name(), sec, target, MEMBER_FLAG_Q_BIT, stack_top, target, 0);
	}
	else if(term->itag()==EXPR_AT){
		int_t var = compile_expr(term->bin_lhs(), stack_top);
		int_t key = compile_expr(term->bin_rhs(), stack_top);
		int_t target = stack_top++;

		put_inst(InstAt(target, var, key, stack_top));
		put_incdec(e, target, target, stack_top);
		put_inst(InstSetAt(var, key, target, stack_top));
	}
}

void CodeBuilder::compile_loop_control_statement(const ExprPtr& e){
	IDPtr label;
	int_t label_kind = 0;

	if(e->itag()==EXPR_BREAK){
		label = e->break_label();
		label_kind = 0;
	}
	else if(e->itag()==EXPR_CONTINUE){
		label = e->continue_label();
		label_kind = 1;
	}

	if(ff().loops.empty()){
		error(Xt("XCE1006"));
	}
	else{
		if(label){
			bool found = false;
			for(int_t i = 0, last = ff().loops.size(); i<last; ++i){
				if(raweq(ff().loops[i].label, label)){
					break_off(ff().loops[i].frame_count);
					set_jump(InstGoto::OFFSET_address, ff().loops[i].control_statement_label[label_kind]);
					put_inst(InstGoto());
					found = true;
					break;
				}
			}

			if(!found){
				error(Xt("XCE1006"));
			}
		}
		else{
			bool found = false;
			for(size_t i = 0, last = ff().loops.size(); i<last; ++i){
				if(!ff().loops[i].have_label){
					break_off(ff().loops[i].frame_count);
					set_jump(InstGoto::OFFSET_address, ff().loops[i].control_statement_label[label_kind]);
					put_inst(InstGoto());		
					found = true;
					break;
				}
			}

			if(!found){
				error(Xt("XCE1006"));
			}
		}
	}
}

void CodeBuilder::compile_class(const ExprPtr& e, int_t stack_top, int_t result){
	{
		fun_frames_.push();	
		ff().labels.clear();
		ff().loops.clear();
		ff().finallies.clear();
		ff().var_frame_count = var_frames_.size();
		ff().stack_count = 0;
		ff().max_stack_count = 0;
		ff().variable_count = 0;
		ff().max_variable_count = 0;
		ff().extendable_param = 0;
	}

	// インスタンス変数を暗黙的初期化するメソッドを定義する

	{
		int_t method_kind = e->class_kind()==KIND_SINGLETON ? KIND_FUN : KIND_METHOD;
		ExprPtr stmts = xnew<Expr>();
		MapPtr ivar_map = xnew<Map>();
		bool auto_initialize = false;
		Xfor_cast(const ExprPtr& v, e->class_stmts()->clone()){
			if(v->itag()==EXPR_CDEFINE_IVAR){
				if(v->cdefine_ivar_term()){
					eb_.push(v->cdefine_ivar_name());
					eb_.splice(EXPR_IVAR, 1);
					eb_.push(v->cdefine_ivar_term());
					eb_.splice(EXPR_ASSIGN, 2);
					stmts->push_back(eb_.pop());
					auto_initialize = true;
				}

				// 可触性が付いているので、アクセッサを定義する
				if(v->cdefine_ivar_accessibility()){ 
					IDPtr var = v->cdefine_ivar_name();
					eb_.push(v->cdefine_ivar_accessibility());
					eb_.push(var);
					eb_.push(null);
					
					eb_.push(method_kind);
					eb_.push(null);
					eb_.push(null);
					eb_.push(var);
					eb_.splice(EXPR_IVAR, 1);
					eb_.splice(0, 1);
					eb_.splice(EXPR_RETURN, 1);
					eb_.splice(EXPR_FUN, 4);

					eb_.splice(EXPR_CDEFINE_MEMBER, 4);

					e->class_stmts()->push_back(eb_.pop());

					IDPtr var2 = Xid(set_)->cat(var);
					eb_.push(v->cdefine_ivar_accessibility());
					eb_.push(var2);
					eb_.push(null);

					eb_.push(method_kind);
					eb_.push(Xid(value));
					eb_.splice(EXPR_LVAR, 1);
					eb_.push(null);
					eb_.splice(0, 2);
					eb_.splice(0, 1);
					eb_.push(null);
					eb_.push(var);
					eb_.splice(EXPR_IVAR, 1);
					eb_.push(Xid(value));
					eb_.splice(EXPR_LVAR, 1);
					eb_.splice(EXPR_ASSIGN, 2);
					eb_.splice(EXPR_FUN, 4);

					eb_.splice(EXPR_CDEFINE_MEMBER, 4);

					e->class_stmts()->push_back(eb_.pop());
				}
			}
		}

		eb_.push(KIND_PUBLIC);
		eb_.push(Xid(auto_initialize));
		eb_.push(null);

		if(auto_initialize){
			eb_.push(method_kind);
			eb_.push(null);
			eb_.push(null);
			eb_.push(stmts);
			eb_.splice(EXPR_SCOPE, 1);
			eb_.splice(EXPR_FUN, 4);
		}
		else{
			eb_.splice(EXPR_NULL, 0);
		}

		eb_.splice(EXPR_CDEFINE_MEMBER, 4);

		e->class_stmts()->push_front(eb_.pop());
	}

	// 継承
	int_t mixins = 0;
	int_t mixin_base = stack_top;
	Xfor(v, e->class_mixins()){
		compile_expr(v, stack_top+mixins+1, stack_top+mixins);
		mixins++;
	}

	// 変数フレームを作成
	var_begin(VarFrame::CLASS);

	// 変数を定義
	{
		int_t number = 0;
		Xfor(v, e->class_stmts()){
			ExprPtr v1 = ep(v);
			if(v1->itag()==EXPR_CDEFINE_MEMBER){
				if(v1->cdefine_member_ns()){
					var_define(v1->cdefine_member_name(), nul<Expr>(), v1->cdefine_member_accessibility()->to_i(), false, true, false, number++);
				}
				else{
					var_define(v1->cdefine_member_name(), nul<Expr>(), v1->cdefine_member_accessibility()->to_i(), false, true, false);
				}
			}
		}
	}

	class_frames_.push();
	cf().entries.clear();
	cf().class_info_num = result_->class_info_table_.size();

	int_t ivar_num = 0;
	int_t instance_variable_identifier_offset = result_->identifier_table_.size();
	Xfor_cast(const ExprPtr& v, e->class_stmts()){
		if(v->itag()==EXPR_CDEFINE_IVAR){
			ClassFrame::Entry entry;
			entry.name = v->cdefine_ivar_name();
			cf().entries.push_back(entry);

			result_->identifier_table_.push_back(entry.name);

			ivar_num++;
		}
	}

	int_t class_info_num = result_->class_info_table_.size();

	ClassInfo info;
	info.pc = code_size();
	info.kind = e->class_kind();
	info.mixins = mixins;
	info.variable_size = vf().entries.size();
	info.instance_variable_size = ivar_num;
	info.instance_variable_identifier_offset = instance_variable_identifier_offset;
	info.name_number = register_identifier(e->class_name());
	
	info.variable_identifier_offset = result_->identifier_table_.size();

	//for(uint_t i=0; i<vf().entries.size(); ++i){
	//	result_->identifier_table_.push_back(intern(Xid(_)->op_cat(vf().entries[i].name)));
	//}

	put_inst(InstClassBegin(class_info_num, mixin_base));
	result_->class_info_table_.push_back(info);

	{
		int_t number = 0;
		Xfor(v, e->class_stmts()){
			ExprPtr v1 = ep(v);
			if(v1->itag()==EXPR_CDEFINE_MEMBER){				
				compile_expr(v1->cdefine_member_term(), stack_top+1, stack_top);

				int_t inumber = 0;
				if(v1->cdefine_member_ns()){
					compile_expr(v1->cdefine_member_ns(), stack_top+2, stack_top+1);
					LVarInfo info = var_find(v1->cdefine_member_name(), true, false, number++);
					inumber = info.pos;
				}
				else{
					put_inst(InstLoadValue(stack_top+1, LOAD_UNDEFINED));
					LVarInfo info = var_find(v1->cdefine_member_name(), true, false);
					inumber = info.pos;
				}

				put_inst(InstDefineClassMember(inumber, 
					register_identifier(v1->cdefine_member_name()), stack_top+1,
					v1->cdefine_member_accessibility()->to_i(), stack_top));
			}
		}
	}

	if(var_frames_.top().scope_chain){
		result_->class_info_table_[class_info_num].flags |= FunInfo::FLAG_SCOPE_CHAIN;
	}

	put_inst(InstClassEnd(result));
	class_frames_.pop();

	var_end();

	fun_frames_.pop();
}

int_t CodeBuilder::compile_fun(const ExprPtr& e, int_t stack_top, int_t result){
	int_t ordered = 0;
	int_t named = 0;

	Xfor_cast(const ExprPtr& v, e->fun_params()){
		if(!v->at(0) || (ep(v->at(0))->itag()!=EXPR_LVAR && ep(v->at(0))->itag()!=EXPR_IVAR)){
			error(Xt("XCE1004"));
			return int_t();
		}

		if(v->at(1)){
			named++;
		}
		else{
			if(named!=0){
				error(Xt("XCE1005"));
			}

			ordered++;
		}
	}

	int_t minv = ordered;
	int_t maxv = ordered + named;

	// ゲッター、またはセッター用の最適化を行う
	if(e->fun_kind()==KIND_METHOD){

		// ゲッタか？
		if(ordered==0 && named==0){
			ExprPtr body = e->fun_body();
			if(body->itag()==EXPR_SCOPE && body->scope_stmts() && body->scope_stmts()->size()==1){
				body = ep(body->scope_stmts()->front());
			}

			if(body->itag()==EXPR_RETURN){
				if(body->return_exprs() && body->return_exprs()->size()==1){
					body = ep(body->return_exprs()->front());
					if(body->itag()==EXPR_IVAR){
						put_inst(InstMakeInstanceVariableAccessor(result, 0, lookup_instance_variable(body->ivar_name()), class_info_num()));
						return 1;
					}
				}
			}
		}

		// セッタか？
		if(ordered==1 && named==0){
			ExprPtr body = e->fun_body();
			if(body->itag()==EXPR_SCOPE && body->scope_stmts() && body->scope_stmts()->size()==1){
				body = ep(body->scope_stmts()->front());
			}

			if(body->itag()==EXPR_ASSIGN){
				ExprPtr lhs = body->bin_lhs();
				ExprPtr rhs = body->bin_rhs();

				IDPtr key;
				ExprPtr arg = ep(e->fun_params()->at(0));
				if(arg){
					arg = ep(arg->at(0));
				}

				if(arg->itag()==EXPR_LVAR){
					key = arg->lvar_name();
				}
				else if(arg->itag()==EXPR_IVAR){
					key = arg->ivar_name();
				}

				if(key && lhs->itag()==EXPR_IVAR && rhs->itag()==EXPR_LVAR && raweq(rhs->lvar_name(), key)){
					put_inst(InstMakeInstanceVariableAccessor(result, 1, lookup_instance_variable(lhs->ivar_name()), class_info_num()));
					return 1;
				}
			}
		}
	}

	// 関数フレームを作成する
	fun_frames_.push();	
	ff().labels.clear();
	ff().loops.clear();
	ff().finallies.clear();
	ff().var_frame_count = var_frames_.size();
	ff().stack_count = 0;
	ff().max_stack_count = 0;
	ff().variable_count = 0;
	ff().max_variable_count = 0;
	ff().extendable_param = e->fun_extendable_param();

	// 変数フレームを作成して、引数を登録する
	var_begin(VarFrame::FUN);
	Xfor_cast(const ExprPtr& v1, e->fun_params()){
		if(const ExprPtr& v = ep(v1->at(0))){
			if(v->itag()==EXPR_LVAR){
				var_define(v->lvar_name());
			}
			else if(v->itag()==EXPR_IVAR){
				var_define(v->ivar_name());
			}
		}
	}

	// 可変長引数があるか？
	if(e->fun_extendable_param()){
		var_define(e->fun_extendable_param());
	}

	// 関数コアを作成
	FunInfo fun;
	fun.pc = code_size() + InstMakeFun::ISIZE;
	fun.kind = e->fun_kind();
	fun.min_param_count = minv;
	fun.max_param_count = maxv;
	fun.name_number = register_identifier(e->fun_name());
	fun.flags = e->fun_extendable_param() ? FunInfo::FLAG_EXTENDABLE_PARAM : 0;

	// 引数の名前を識別子テーブルに順番に乗せる
	fun.variable_size = vf().entries.size();
	fun.variable_identifier_offset = result_->identifier_table_.size();
	for(uint_t i=0; i<vf().entries.size(); ++i){
		vf().entries[i].initialized = true;
		result_->identifier_table_.push_back(vf().entries[i].name);
	}

	int_t fun_info_table_number = result_->xfun_info_table_.size();
	result_->xfun_info_table_.push_back(fun);

	int_t fun_end_label = reserve_label();
	set_jump(InstMakeFun::OFFSET_address, fun_end_label);
	put_inst(InstMakeFun(result, fun_info_table_number, 0));

	ff().variable_count += vf().entries.size();
	if(ff().max_variable_count<ff().variable_count){
		ff().max_variable_count = ff().variable_count;
	}

	// デフォルト値を持つ引数を処理する
	{
		int_t i = 0;
		Xfor_cast(const ExprPtr& v1, e->fun_params()){
			if(const ExprPtr& v = ptr_cast<Expr>(v1->at(1))){
				int_t label_true = reserve_label();
				int_t label_false = reserve_label();
				
				set_jump(InstIfUndefined::OFFSET_address_true, label_true);
				set_jump(InstIfUndefined::OFFSET_address_false, label_false);
				put_inst(InstIfUndefined(-(maxv-1-i)-1, 0, 0));
				set_label(label_true);

				compile_expr(v, stack_top+1, -(maxv-1-i)-1);
								
				set_label(label_false);
			}
			else{
				var_find(vf().entries[i].name, true);
			}	

			++i;
		}
	}

	// 引数にインスタンス変数がある場合に、特別な処理を入れる
	Xfor_cast(const ExprPtr& v1, e->fun_params()){
		const ExprPtr& v = ep(v1->at(0));
		if(v->itag()==EXPR_IVAR){
			eb_.push(v);
			eb_.push(v->ivar_name());
			eb_.splice(EXPR_LVAR, 1);
			eb_.splice(EXPR_ASSIGN, 2);
			compile_stmt(eb_.pop());
		}
	}

	if(e->fun_params()){
		add_stack_count(e->fun_params()->size());
		add_stack_count(-(int_t)e->fun_params()->size());
	}

	// 関数本体を処理する
	compile_stmt(e->fun_body());

	XTAL_ASSERT(ff().stack_count==0);
	
	break_off(ff().var_frame_count+1);

	put_inst(InstReturn(0));
	set_label(fun_end_label);

	process_labels();

	if(vf().scope_chain){
		result_->xfun_info_table_[fun_info_table_number].flags |= FunInfo::FLAG_SCOPE_CHAIN;
	}

	// 変数フレームをポップする
	var_end();

	result_->xfun_info_table_[fun_info_table_number].max_variable = ff().max_variable_count;

	// 関数フレームをポップする
	fun_frames_.downsize(1);

	return 1;
}

AnyPtr CodeBuilder::do_expr(const AnyPtr& p){

	if(!p){
		return null;
	}

	ExprPtr e = ep(p);

	switch(e->itag()){

		XTAL_DEFAULT;

		XTAL_CASE(EXPR_NULL){ return null; }
		XTAL_CASE(EXPR_TRUE){ return true; }
		XTAL_CASE(EXPR_FALSE){ return false; }
		XTAL_CASE(EXPR_NUMBER){ return e->number_value(); }
		XTAL_CASE(EXPR_STRING){
			if(e->string_kind()==KIND_STRING){
				return e->string_value();
			}
		}

		XTAL_CASE(EXPR_LVAR){
			LVarInfo info = var_find(e->lvar_name(), false, true);
			if(info.pos>=0){
				if((var_frame(info).fun_frames_size==fun_frames_.size() || !entry(info).assigned)){
					entry(info).referenced = true;
					return entry(info).value;
				}
			}
		}
	}

	return undefined;
}

void CodeBuilder::check_lvar_assign(const ExprPtr& e){
	if(e && e->itag()==EXPR_LVAR){
		LVarInfo info = var_find(e->lvar_name(), true, true);
		if(info.pos>=0){
			entry(info).assigned = true;
			entry(info).value = undefined;
		}
	}
}

// 代入演算子があるか調べる
void CodeBuilder::check_lvar_assign_stmt(const AnyPtr& p){
	if(ExprPtr e = ep(p)){
		switch(e->itag()){
		case EXPR_ASSIGN:
		case EXPR_ADD_ASSIGN:
		case EXPR_SUB_ASSIGN:
		case EXPR_CAT_ASSIGN:
		case EXPR_MUL_ASSIGN:
		case EXPR_DIV_ASSIGN:
		case EXPR_MOD_ASSIGN:
		case EXPR_OR_ASSIGN:
		case EXPR_AND_ASSIGN:
		case EXPR_XOR_ASSIGN:
		case EXPR_SHR_ASSIGN:
		case EXPR_SHL_ASSIGN:
		case EXPR_USHR_ASSIGN:
			check_lvar_assign(e->bin_lhs());
			break;

		case EXPR_INC:
		case EXPR_DEC:
			check_lvar_assign(e->una_term());
			break;

		case EXPR_MASSIGN:
			Xfor(v, e->massign_lhs_exprs()){
				check_lvar_assign(ep(v));
			}
			break;
		}

		Xfor(v, e){
			check_lvar_assign_stmt(v);
		}
	}
}

AnyPtr VMachine::eval(const xpeg::ExecutorPtr& executor, uint_t n){
	debug::CallerInfoPtr cp = caller(n+1);
	if(!cp || !cp->fun()){
		return undefined;
	}

	CodeBuilder cb;
	if(CodePtr code = cb.eval_compile(executor)){
		print_info();

		AnyPtr self = fun_frames_[n+1]->self();
		setup_call(1);
		set_arg_this(self);
		code->first_fun()->rawcall(to_smartptr(this));

		const inst_t* pc = ff().called_pc;
		ff().called_pc = 0;

		execute_inner(pc, n + 2);

		ff().processed = 0;
		print_info();
		return local_variable(result_base_+0);
	}
	else{
		XTAL_SET_EXCEPT(cpp_class<CompileError>()->call(Xt("XRE1010")->call(), cb.errors()->to_a()));
	}

	return undefined;
}

}

#endif

