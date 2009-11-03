//
// C++ Interface: ParseString
//
// Description:
// 将字符串转化为汉字索引数组
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef __SRC_ENGINE_PARSESTRING_H
#define __SRC_ENGINE_PARSESTRING_H

#include "mess.h"

/**
 * 拼音单元的属性值.
 */
typedef enum {
	ATOM_TYPE = 0,	///< 自成一体，不与任何单元结合
	MAJOR_TYPE = 1 << 0,	///< 在汉字拼音中只能处于第一个位置
	MINOR_TYPE = 1 << 1,	///< 在汉字拼音中只能处于第二个位置
	MAJIN_TYPE = MAJOR_TYPE | MINOR_TYPE,	///< 在汉字拼音中的位置不受限制
}PyUnitFlags;
/**
 * 拼音单元的信息结构.
 */
typedef struct {
	const char *unit;	///< 拼音单元串 *
	PyUnitFlags type;	///< 拼音单元属性
}PinyinUnit;

/**
 * 字符串分解类.
 */
class ParseString
{
public:
	ParseString();
	~ParseString();

	bool ParsePinyinString(const char *string, CharsIndex **chidx, int *len);
	char *RestorePinyinString(const CharsIndex *chidx, int len);
	int8_t GetStringIndex(const char *string);
	int8_t GetPinyinUnitSum();
private:
	int8_t SearchMatchUnit(const char *ptr);
	void AppendCharsIndex(CharsIndex *chidx, int *point,
				 PyUnitFlags *type, int8_t index);

	static PinyinUnit pyunits[];	///< 拼音单元表.
};

#endif
