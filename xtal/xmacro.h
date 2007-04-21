
#pragma once

#include "lib.h"

/** @addtogroup xmacro */
/*@{*/

/**
* @brief foreachを簡単に記述するためのマクロ
*
* @code
* Xfor(value, array){
*   // use value
* }
* @endcode
*/
#define Xfor(var, target) \
	if(::xtal::Any var = 1)\
	if(::xtal::IterBreaker tar = target)\
	for(::xtal::iter_next(tar, var, true); tar; ::xtal::iter_next(tar, var, false))

/**
* @brief foreachを簡単に記述するためのマクロ
*
* @code
* Xfor(key, value, map.send("pairs")){
*   // use key and value
* }
* @endcode
*/
#define Xfor2(var1, var2, target) \
	if(::xtal::Any var1 = 1)\
	if(::xtal::Any var2 = 1)\
	if(::xtal::IterBreaker tar = target)\
	for(::xtal::iter_next(tar, var1, var2, true); tar; ::xtal::iter_next(tar, var1, var2, false))

/**
* @brief foreachを簡単に記述するためのマクロ
*
* @code
* Xfor(v1, v2, v3, hoge.send("each3")){
*   // use v1, v2 and v3
* }
* @endcode
*/
#define Xfor3(var1, var2, var3, target) \
	if(::xtal::Any var1 = 1)\
	if(::xtal::Any var2 = 1)\
	if(::xtal::Any var3 = 1)\
	if(::xtal::IterBreaker tar = target)\
	for(::xtal::iter_next(tar, var1, var2, var3, true); tar; ::xtal::iter_next(tar, var1, var2, var3, false))

/**
* @brief インターンされた文字列を簡単に記述するためのマクロ
*
* @code
* ID id = Xid(test);
* @endcode
*/
#define Xid(string) ::xtal::Key(::xtal::ID(#string, sizeof(#string)-1))

/**
* @brief get_textを簡単に記述するためのマクロ
*
* @code
* Any text = Xt("Text %d %s")(10, "test");
* @endcode
*/
#define Xt(text) ::xtal::get_text(text) 

/**
* @brief formatを簡単に記述するためのマクロ
*
* @code
* Any fmt = Xf("Text %d %s")(10, "test");
* @endcode
*/
#define Xf(text) ::xtal::format(text) 

/**
* @brief Xtalのソースを簡単に記述するためのマクロ
*
* @code
* Any src = Xsrc((
*   export [0, 2, 3, 4];
* ));
* @endcode
*/
#define Xsrc(text) ::xtal::source(#text+1, sizeof(#text)-3, __FILE__)

/*@}*/
