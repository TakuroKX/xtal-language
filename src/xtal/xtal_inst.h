/** \file src/xtal/xtal_inst.h
* \brief src/xtal/xtal_inst.h
*/

#ifndef XTAL_INST_H_INCLUDE_GUARD
#define XTAL_INST_H_INCLUDE_GUARD

#pragma once

namespace xtal{

struct Inst;

typedef u8 inst_t;
typedef i8 inst_i8_t;
typedef u8 inst_u8_t;

template<class T, int Kind>
struct inst_16_t{
	u8 values[2];
	
	inst_16_t(){}

	inst_16_t(int value){
		values[0] = (u8)(value>>8);
		values[1] = (u8)(value>>0);
	}
	
	operator T() const{
		return (T)((values[0]<<8) | values[1]);
	}
};

typedef inst_16_t<i16, 0> inst_i16_t;
typedef inst_16_t<u16, 0> inst_u16_t;
typedef inst_16_t<i16, 1> inst_address_t;


inline int_t inst_inspect(i8 value, Inst*, const CodePtr&){ return (int_t)value; }
inline int_t inst_inspect(u8 value, Inst*, const CodePtr&){ return (int_t)value; }
inline int_t inst_inspect(inst_i16_t value, Inst*, const CodePtr&){ return (int_t)value; }
inline int_t inst_inspect(inst_u16_t value, Inst*, const CodePtr&){ return (int_t)value; }

int_t inst_inspect(inst_address_t& value, Inst* inst, const CodePtr& code);

StringPtr make_inst_string(const char_t* InstName);

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1);

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2);

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2,
						const char_t* MemberName3, int_t MemberValue3);
StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2,
						const char_t* MemberName3, int_t MemberValue3,
						const char_t* MemberName4, int_t MemberValue4);

StringPtr make_inst_string(const char_t* InstName, 
						const char_t* MemberName1, int_t MemberValue1,
						const char_t* MemberName2, int_t MemberValue2,
						const char_t* MemberName3, int_t MemberValue3,
						const char_t* MemberName4, int_t MemberValue4,
						const char_t* MemberName5, int_t MemberValue5);

struct Inst{
	inst_t op;

	Inst(inst_t v = 0)
		:op(v){}

	operator const inst_t*() const{
		return &op;
	}

	template<class T>
	void checked_assign(T& ref, int v);
};

template<class T>
void Inst::checked_assign(T& ref, int v){
	ref = T(v);
	if(static_cast<int>(ref) != v){ op = 0xff; }
}

#define XTAL_DEF_INST_0(N, InstName) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst),\
			ISIZE = SIZE/sizeof(inst_t)\
		};\
		Inst##InstName(){\
			op = N;\
		}\
		StringPtr inspect(const CodePtr&){\
			return make_inst_string(\
					XTAL_STRING(#InstName));\
		}\
	}

#define XTAL_DEF_INST_1(N, InstName, MemberType1, MemberName1) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t)\
		};\
		MemberType1 MemberName1;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1){\
			op = N;\
			checked_assign(MemberName1, m1);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code));\
		}\
	}

#define XTAL_DEF_INST_2(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code));\
		}\
	}
	
#define XTAL_DEF_INST_3(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2, int m3){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code));\
		}\
	}
	
#define XTAL_DEF_INST_4(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2, int m3, int m4){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code),\
					XTAL_STRING(#MemberName4), inst_inspect(MemberName4, this, code));\
		}\
	}

#define XTAL_DEF_INST_5(N, InstName, MemberType1, MemberName1, MemberType2, MemberName2, MemberType3, MemberName3, MemberType4, MemberName4, MemberType5, MemberName5) \
	struct Inst##InstName : public Inst{\
		enum{ \
			NUMBER = N,\
			SIZE = sizeof(Inst) + sizeof(MemberType1) + sizeof(MemberType2) + sizeof(MemberType3) + sizeof(MemberType4) + sizeof(MemberType5),\
			ISIZE = SIZE/sizeof(inst_t),\
			OFFSET_##MemberName1 = sizeof(inst_t),\
			OFFSET_##MemberName2 = OFFSET_##MemberName1 + sizeof(MemberType1),\
			OFFSET_##MemberName3 = OFFSET_##MemberName2 + sizeof(MemberType2),\
			OFFSET_##MemberName4 = OFFSET_##MemberName3 + sizeof(MemberType3),\
			OFFSET_##MemberName5 = OFFSET_##MemberName4 + sizeof(MemberType4)\
		};\
		MemberType1 MemberName1;\
		MemberType2 MemberName2;\
		MemberType3 MemberName3;\
		MemberType4 MemberName4;\
		MemberType5 MemberName5;\
		Inst##InstName(){\
			op = N;\
		}\
		Inst##InstName(int m1, int m2, int m3, int m4, int m5){\
			op = N;\
			checked_assign(MemberName1, m1);\
			checked_assign(MemberName2, m2);\
			checked_assign(MemberName3, m3);\
			checked_assign(MemberName4, m4);\
			checked_assign(MemberName5, m5);\
		}\
		StringPtr inspect(const CodePtr& code){\
			return make_inst_string(\
					XTAL_STRING(#InstName),\
					XTAL_STRING(#MemberName1), inst_inspect(MemberName1, this, code),\
					XTAL_STRING(#MemberName2), inst_inspect(MemberName2, this, code),\
					XTAL_STRING(#MemberName3), inst_inspect(MemberName3, this, code),\
					XTAL_STRING(#MemberName4), inst_inspect(MemberName4, this, code),\
					XTAL_STRING(#MemberName5), inst_inspect(MemberName5, this, code));\
		}\
	}

/**
* \internal
* \brief 何もしない。
*
* stack [] -> []
*/
XTAL_DEF_INST_0(0, Nop);

/**
* \internal
* \brief スタックにnull値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(1, PushNull);

/**
* \internal
* \brief スタックにundefined値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(2, PushUndefined);

/**
* \internal
* \brief スタックにtrue値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(3, PushTrue);

/**
* \internal
* \brief スタックにfalse値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(4, PushFalse);

/**
* \internal
* \brief スタックにtrue値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(5, PushTrueAndSkip);

/**
* \internal
* \brief スタックにint値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(6, PushInt1Byte,
        inst_i8_t, value
);

/**
* \internal
* \brief スタックにint値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(7, PushInt2Byte,
        inst_i16_t, value
);

/**
* \internal
* \brief スタックにfloat値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(8, PushFloat1Byte,
        inst_i8_t, value
);

/**
* \internal
* \brief スタックにfloat値をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(9, PushFloat2Byte,
        inst_i16_t, value
);

/**
* \internal
* \brief スタックに現在実行中の関数をプッシュする。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(10, PushCallee);

/**
* \internal
* \brief 現在のthisをスタックに積む。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(11, PushThis);

/**
* \internal
* \brief 現在の文脈をスタックに積む。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_0(12, PushCurrentContext);

/**
* \internal
* \brief スタックトップをポップする
*
* stack [value] -> []
*/
XTAL_DEF_INST_0(13, Pop);

/**
* \internal
* \brief スタックトップの値をプッシュする
*
* stack [value] -> [value, value2]
*/
XTAL_DEF_INST_0(14, Dup);

/**
* \internal
* \brief スタックトップの値を一つ前に入れる。
*
* stack [value1, value2] -> [value2, value1]
*/
XTAL_DEF_INST_0(15, Insert1);

/**
* \internal
* \brief スタックトップの値を二つ前に入れる。
*
* stack [value1, value2, value3] -> [value3, value1, value2]
*/
XTAL_DEF_INST_0(16, Insert2);

/**
* \internal
* \brief スタックトップの値を三つ前に入れる。
*
* stack [value1, value2, value3, value4] -> [value4, value1, value2, value3]
*/
XTAL_DEF_INST_0(17, Insert3);

/**
* \internal
* \brief 値の数を調整する
*
* stack [] -> []
*/
XTAL_DEF_INST_2(18, AdjustResult,
        inst_u8_t, result_count,
        inst_u8_t, need_result_count
);

XTAL_DEF_INST_1(19, LocalVariableInc,
		inst_u8_t, number
);

XTAL_DEF_INST_1(20, LocalVariableDec,
		inst_u8_t, number
);

XTAL_DEF_INST_1(21, LocalVariableInc2Byte,
		inst_u16_t, number
);

XTAL_DEF_INST_1(22, LocalVariableDec2Byte,
		inst_u16_t, number
);

/**
* \internal
* \brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(23, LocalVariable1Byte,
        inst_u8_t, number
);

/**
* \internal
* \brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(24, LocalVariable1ByteX2,
        inst_u8_t, number1,
        inst_u8_t, number2
);

/**
* \internal
* \brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_3(25, LocalVariable1ByteX3,
        inst_u8_t, number1,
        inst_u8_t, number2,
        inst_u8_t, number3
);

/**
* \internal
* \brief ローカル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(26, LocalVariable2Byte,
        inst_u16_t, number
);

/**
* \internal
* \brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(27, SetLocalVariable1Byte,
        inst_u8_t, number
);

/**
* \internal
* \brief ローカル変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(28, SetLocalVariable2Byte,
        inst_u16_t, number
);

/**
* \internal
* \brief インスタンス変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_2(29, InstanceVariable,
        inst_u8_t, number,
        inst_u16_t, info_number
);

/**
* \internal
* \brief インスタンス変数に値を設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_2(30, SetInstanceVariable,
        inst_u8_t, number,
        inst_u16_t, info_number
);

/**
* \internal
* \brief 関数呼び出しの後始末をする
*
* stack [] -> []
*/
XTAL_DEF_INST_0(31, CleanupCall);

/**
* \internal
* \brief 関数から抜ける
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_1(32, Return,
        inst_u8_t, results
);

/**
* \internal
* \brief fiberの実行を一時中断する
*
* stack [value1, value2, ...valueN] -> []
*/
XTAL_DEF_INST_1(33, Yield,
        inst_u8_t, results
);

/**
* \internal
* \brief 仮想マシンのループから脱出する。
*
* stack [] -> []
*/
XTAL_DEF_INST_0(34, Exit);

/**
* \internal
* \brief 値テーブルから値を取り出す
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(35, Value,
        inst_u16_t, value_number
);

XTAL_DEF_INST_0(36, CheckUnsupported);

enum{
	CALL_FLAG_TAIL_SHIFT = 0,
	CALL_FLAG_ARGS_SHIFT = 1,
	CALL_FLAG_NS_SHIFT = 2,
	CALL_FLAG_Q_SHIFT = 3,

	CALL_FLAG_NONE = 0,

	CALL_FLAG_TAIL = 1<<CALL_FLAG_TAIL_SHIFT,
	CALL_FLAG_ARGS = 1<<CALL_FLAG_ARGS_SHIFT,
	CALL_FLAG_NS = 1<<CALL_FLAG_NS_SHIFT,
	CALL_FLAG_Q = 1<<CALL_FLAG_Q_SHIFT
};

XTAL_DEF_INST_2(37, Property,
        inst_u16_t, identifier_number,
		inst_u8_t, need_result
);

XTAL_DEF_INST_1(38, SetProperty,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_4(39, Call,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
		inst_u8_t, flags
);

XTAL_DEF_INST_5(40, Send,
        inst_u8_t, ordered,
        inst_u8_t, named,
        inst_u8_t, need_result,
		inst_u8_t, flags,
        inst_u16_t, identifier_number
);

/**
* \internal
* \brief オブジェクトのメンバを取り出す。
*
* stack [object] -> [result]
*/
XTAL_DEF_INST_2(41, Member,
        inst_u16_t, identifier_number,
		inst_u8_t, flags
);

/**
* \internal
* \brief オブジェクトのメンバを定義する。
*
* stack [object, value] -> []
*/
XTAL_DEF_INST_2(42, DefineMember,
        inst_u16_t, identifier_number,
		inst_u8_t, flags
);

XTAL_DEF_INST_1(43, BlockBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(44, BlockEnd);

XTAL_DEF_INST_1(45, TryBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(46, TryEnd);

XTAL_DEF_INST_1(47, PushGoto,
        inst_address_t, address
);

XTAL_DEF_INST_0(48, PopGoto);

/**
* \internal
* \brief 無条件分岐
*
* stack [] -> []
*/
XTAL_DEF_INST_1(49, Goto,
        inst_address_t, address
);

/**
* \internal
* \brief 条件分岐
*
* stack [value] -> []
*/
XTAL_DEF_INST_2(50, If,
        inst_address_t, address_true,
		inst_address_t, address_false
);

XTAL_DEF_INST_0(51, IfEq);

XTAL_DEF_INST_0(52, IfLt);

XTAL_DEF_INST_0(53, IfRawEq);

XTAL_DEF_INST_0(54, IfIn);

XTAL_DEF_INST_0(55, IfIs);

XTAL_DEF_INST_2(56, IfArgIsUndefined,
	inst_u8_t, arg,
	inst_address_t, address
);

XTAL_DEF_INST_0(57, Pos);
XTAL_DEF_INST_0(58, Neg);
XTAL_DEF_INST_0(59, Com);
XTAL_DEF_INST_0(60, Not);

XTAL_DEF_INST_0(61, At);
XTAL_DEF_INST_0(62, SetAt);

XTAL_DEF_INST_0(63, Add);
XTAL_DEF_INST_0(64, Sub);
XTAL_DEF_INST_0(65, Cat);
XTAL_DEF_INST_0(66, Mul);
XTAL_DEF_INST_0(67, Div);
XTAL_DEF_INST_0(68, Mod);
XTAL_DEF_INST_0(69, And);
XTAL_DEF_INST_0(70, Or);
XTAL_DEF_INST_0(71, Xor);
XTAL_DEF_INST_0(72, Shl);
XTAL_DEF_INST_0(73, Shr);
XTAL_DEF_INST_0(74, Ushr);

XTAL_DEF_INST_0(75, Inc);
XTAL_DEF_INST_0(76, Dec);
XTAL_DEF_INST_0(77, AddAssign);
XTAL_DEF_INST_0(78, SubAssign);
XTAL_DEF_INST_0(79, CatAssign);
XTAL_DEF_INST_0(80, MulAssign);
XTAL_DEF_INST_0(81, DivAssign);
XTAL_DEF_INST_0(82, ModAssign);
XTAL_DEF_INST_0(83, AndAssign);
XTAL_DEF_INST_0(84, OrAssign);
XTAL_DEF_INST_0(85, XorAssign);
XTAL_DEF_INST_0(86, ShlAssign);
XTAL_DEF_INST_0(87, ShrAssign);
XTAL_DEF_INST_0(88, UshrAssign);

XTAL_DEF_INST_1(89, Range,
		inst_u8_t, kind
);

/**
* \internal
* \brief グローバル変数を取り出す。
*
* stack [] -> [value]
*/
XTAL_DEF_INST_1(90, GlobalVariable,
        inst_u16_t, identifier_number
);
/**
* \internal
* \brief グローバル変数に設定する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(91, SetGlobalVariable,
        inst_u16_t, identifier_number
);

/**
* \internal
* \brief グローバル変数を定義する。
*
* stack [value] -> []
*/
XTAL_DEF_INST_1(92, DefineGlobalVariable,
        inst_u16_t, identifier_number
);

XTAL_DEF_INST_2(93, Once,
        inst_address_t, address,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(94, SetOnce,
        inst_u16_t, value_number
);

XTAL_DEF_INST_1(95, ClassBegin,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(96, ClassEnd);

XTAL_DEF_INST_3(97, DefineClassMember,
        inst_u16_t, number,
		inst_u16_t, identifier_number,
		inst_u8_t, accessibility
);

XTAL_DEF_INST_0(98, MakeArray);

XTAL_DEF_INST_0(99, ArrayAppend);

XTAL_DEF_INST_0(100, MakeMap);

XTAL_DEF_INST_0(101, MapInsert);

XTAL_DEF_INST_0(102, MapSetDefault);

XTAL_DEF_INST_2(103, MakeFun,
        inst_u16_t, info_number,
		inst_address_t, address
);

XTAL_DEF_INST_3(104, MakeInstanceVariableAccessor,
        inst_u8_t, type,
		inst_u8_t, number,
        inst_u16_t, info_number
);

XTAL_DEF_INST_0(105, Throw);

XTAL_DEF_INST_0(106, ThrowUnsupportedError);

XTAL_DEF_INST_1(107, IfDebug,
		inst_address_t, address);

XTAL_DEF_INST_0(108, Assert);

XTAL_DEF_INST_0(109, BreakPoint);

XTAL_DEF_INST_0(110, MAX);

}

#endif // XTAL_INST_H_INCLUDE_GUARD
