
#pragma once

#include "fwd.h"
#include "vmachine.h"

namespace xtal{

/** @addtogroup lib */
/*@{*/

/**
* @brief file_nameファイルをコンパイルする。
*
* @param file_name Xtalスクリプトが記述されたファイルの名前
* @return 実行できる関数オブジェクト
* この戻り値をobject_dumpすると、バイトコード形式で保存される。
*/
Any compile_file(const String& file_name);

/**
* @brief source文字列をコンパイルする。
*
* @param source Xtalスクリプトが記述された文字列
* @return 実行できる関数オブジェクト
* この戻り値をobject_dumpすると、バイトコード形式で保存される。
*/
Any compile(const String& source);

/**
* @brief file_nameファイルをコンパイルして実行する。
*
* @param file_name Xtalスクリプトが記述されたファイルの名前
* @return スクリプト内でexportされた値
*/
Any load(const String& file_name);

/**
* @brief file_nameファイルをコンパイルして実行し、コンパイル済みソースを保存する。
*
* @param file_name Xtalスクリプトが記述されたファイルの名前
* @return スクリプト内でexportされた値
*/
Any load_and_save(const String& file_name);

/**
* @brief ガーベジコレクションを実行する
*
* さほど時間はかからないが、完全にゴミを解放できないガーベジコレクト関数
*/
void gc();

/**
* @brief 循環オブジェクトも解放するフルガーベジコレクションを実行する
*
* 時間はかかるが、現在ゴミとなっているものはなるべく全て解放するガーベジコレクト関数
*/
void full_gc();

/**
* @brief ガーベジコレクションを有効化する
*
* enable_gcが呼ばれた回数と同じだけ呼び出すとガーベジコレクションが有効になる
*/
void enable_gc();

/**
* @brief ガーベジコレクションを無効化する
*
* 内部でこれが何回呼び出されたか記憶されており、呼び出した回数enable_gcを呼びないとガーベジコレクションは有効にならない
*/
void disable_gc();

/**
* @brief オブジェクトを直列化して保存する
*
* @param obj 直列化して保存したいオブジェクト
* @param out 直列化先のストリーム
*/
void object_dump(const Any& obj, const Stream& out);

/**
* @brief オブジェクトを直列化して読み込む
*
* @param in 直列化されたオブジェクトを読み込むストリーム
* @return 復元されたオブジェクト
*/
Any object_load(const Stream& in);

/**
* @brief メインVMachineオブジェクトを返す
*/
const VMachine& vmachine();

/**
* @brief Iteratorクラスを返す
*/
const Class& Iterator();

/**
* @brief Enumerableクラスを返す
*/
const Class& Enumerable();

/**
* @brief builtinクラスを返す
*/
const Class& builtin();

/**
* @brief libオブジェクトを返す
*/
const Any& lib();

/**
* @brief nopオブジェクトを返す
*/
const Any& nop();

Any get_text(const char* text);

Any format(const char* text);

Any source(const char* src, int_t size, const char* file);

void print(const VMachine& vm);

void println(const VMachine& vm);

void InitFormat();
void InitInt();
void InitFloat();

void iter_next(Any& target, Any& value, bool first);
void iter_next(Any& target, Any& value1, Any& value2, bool first);
void iter_next(Any& target, Any& value1, Any& value2, Any& value3, bool first);

struct IterBreaker{
	Any target;
	IterBreaker(const Any& tar=null):target(tar){}
	~IterBreaker();
	operator Any&(){ return target; }
	operator bool(){ return target; }
};

/*@}*/


}

