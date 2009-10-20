//
// C++ Implementation: ParseString
//
// Description:
//
//
// Author: Jally <jallyx@163.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "ParseString.h"
#include "../utils/output.h"
#define MAX_PINYINLEN 6

/**
 * 拼音单元数组.
 */
PinyinUnit pyunits[] = {
	{"iang", MINOR_TYPE},		///< 0x00 0
	{"iong", MINOR_TYPE},		///< 0x01 1
	{"uang", MINOR_TYPE},	///< 0x02 2
	{"ang", MAJIN_TYPE},		///< 0x03 3
	{"eng", MAJIN_TYPE},		///< 0x04 4
	{"ian", MINOR_TYPE},		///< 0x05 5
	{"iao", MINOR_TYPE},		///< 0x06 6
	{"ing", MINOR_TYPE},		///< 0x07 7
	{"ong", MINOR_TYPE},		///< 0x08 8
	{"uai", MINOR_TYPE},		///< 0x09 9
	{"uan", MINOR_TYPE},		///< 0x0a 10
	{"ai", MAJIN_TYPE},		///< 0x0b 11
	{"an", MAJIN_TYPE},		///< 0x0c 12
	{"ao", MAJIN_TYPE},		///< 0x0d 13
	{"ch", MAJOR_TYPE},		///< 0x0e 14
	{"ei", MAJIN_TYPE},		///< 0x0f 15
	{"en", MAJIN_TYPE},		///< 0x10 16
	{"er", ATOM_TYPE},		///< 0x11 17
	{"ia", MINOR_TYPE},		///< 0x12 18
	{"ie", MINOR_TYPE},		///< 0x13 19
	{"in", MINOR_TYPE},		///< 0x14 20
	{"iu", MINOR_TYPE},		///< 0x15 21
	{"ou", MAJIN_TYPE},		///< 0x16 22
	{"sh", MAJOR_TYPE},		///< 0x17 23
	{"ua", MINOR_TYPE},		///< 0x18 24
	{"ue", MINOR_TYPE},		///< 0x19 25
	{"ui", MINOR_TYPE},		///< 0x1a 26
	{"un", MINOR_TYPE},		///< 0x1b 27
	{"uo", MINOR_TYPE},		///< 0x1c 28
	{"ve", MINOR_TYPE},		///< 0x1d 29
	{"zh", MAJOR_TYPE},		///< 0x1e 30
	{"a", MAJIN_TYPE},		///< 0x1f 31
	{"b", MAJOR_TYPE},		///< 0x20 32
	{"c", MAJOR_TYPE},		///< 0x21 33
	{"d", MAJOR_TYPE},		///< 0x22 34
	{"e", MAJIN_TYPE},		///< 0x23 35
	{"f", MAJOR_TYPE},		///< 0x24 36
	{"g", MAJOR_TYPE},		///< 0x25 37
	{"h", MAJOR_TYPE},		///< 0x26 38
	{"i", MINOR_TYPE},		///< 0x27 39
	{"j", MAJOR_TYPE},		///< 0x28 40
	{"k", MAJOR_TYPE},		///< 0x29 41
	{"l", MAJOR_TYPE},		///< 0x2a 42
	{"m", MAJOR_TYPE},		///< 0x2b 43
	{"n", MAJOR_TYPE},		///< 0x2c 44
	{"o", MAJIN_TYPE},		///< 0x2d 45
	{"p", MAJOR_TYPE},		///< 0x2e 46
	{"q", MAJOR_TYPE},		///< 0x2f 47
	{"r", MAJOR_TYPE},		///< 0x30 48
	{"s", MAJOR_TYPE},		///< 0x31 49
	{"t", MAJOR_TYPE},		///< 0x32 50
	{"u", MINOR_TYPE},		///< 0x33 51
	{"v", MINOR_TYPE},		///< 0x34 52
	{"w", MAJOR_TYPE},		///< 0x35 53
	{"x", MAJOR_TYPE},		///< 0x36 54
	{"y", MAJOR_TYPE},		///< 0x37 55
	{"z", MAJOR_TYPE},		///< 0x38 56
	{NULL, ATOM_TYPE}
};

ParseString::ParseString()
{
}

ParseString::~ParseString()
{
}

/**
 * 分析拼音串.
 * @param string 原始拼音串，e.g.<yumen,yu'men>
 * @retval chidx 汉字索引数组
 * @retval len 汉字索引数组有效长度
 * @return 是否分析成功
 */
bool ParseString::ParsePinyinString(const char *string, CharsIndex **chidx, int *len)
{
	size_t size, count;
	PyUnitFlags type;
	const char *ptr;
	int8_t index;

	/* 预处理 */
	type = ATOM_TYPE;
	size = strlen(string);
	*chidx = new CharsIndex[size];
	for (count = 0; count < size; count++)
		(*chidx + count)->major = (*chidx + count)->minor = -1;
	*len = -1;

	/* 分析拼音串 */
	ptr = string;
	while (*ptr != '\0') {
		if ((index = SearchMatchUnit(ptr)) != -1) {
			AppendCharsIndex(*chidx, len, &type, index);
			ptr += strlen((pyunits + index)->unit);
		} else
			ptr++;
	}

	/* 结果处理 */
	(*len)++;

	return true;
}

/**
 * 根据汉字索引数组恢复拼音串.
 * @param chidx 汉字索引数组
 * @param len 汉字索引数组有效长度
 * @return 拼音串
 */
char *ParseString::RestorePinyinString(CharsIndex *chidx, int len)
{
	char *pinyin, *ptr;
	int size, count;

	/* 申请足够大的空间 */
	pinyin = (char *)g_malloc((MAX_PINYINLEN + 1) * len + 1);
	ptr = pinyin;
	size = 0;

	/* 将拼音串放入缓冲区 */
	count = 0;
	while (count < len) {
		if ((chidx + count)->minor != -1)
			sprintf(ptr, "%s%s\'", (pyunits + (chidx + count)->major)->unit,
					 (pyunits + (chidx + count)->minor)->unit);
		else
			sprintf(ptr, "%s\'", (pyunits + (chidx + count)->major)->unit);
		size += strlen(ptr);
		ptr = pinyin + size;
		count++;
	}
	*ptr = '\0';

	return pinyin;
}

/**
 * 获取字符串的拼音单元索引值.
 * @param string 字符串
 * @return 索引值
 */
int8_t ParseString::GetStringIndex(const char *string)
{
	return SearchMatchUnit(string);
}

/**
 * 获取拼音单元的总数.
 * @return 总数
 */
int8_t ParseString::GetPinyinUnitSum()
{
	return G_N_ELEMENTS(pyunits);
}

/**
 * 搜索当前指针串所匹配的拼音单元.
 * @param ptr 指针串，e.g.<yumen,u'men,'men>
 * @return 拼音单元在数组中的索引值
 */
int8_t ParseString::SearchMatchUnit(const char *ptr)
{
	PinyinUnit *pyu;

	pyu = pyunits;
	while (pyu->unit != NULL) {
		if (memcmp(pyu->unit, ptr, strlen(pyu->unit)) == 0)
			break;
		pyu++;
	}

	if (pyu->unit == NULL)
		return -1;
	return (pyu - pyunits);
}

/**
 * 追加一个索引值到汉字索引数组.
 * @param chidx 汉字索引数组
 * @param point 当前所操作汉字索引数组的位置
 * @param ttype 前一个索引的属性
 * @param index 索引值
 */
void ParseString::AppendCharsIndex(CharsIndex *chidx, int *point,
				 PyUnitFlags *type, int8_t index)
{
	PyUnitFlags ttype;

	ttype = (pyunits + index)->type;
	switch (*type) {
	case ATOM_TYPE:
	case MINOR_TYPE:
	case MAJIN_TYPE:
		if (ttype == ATOM_TYPE || ttype & MAJOR_TYPE) {
mark:			(*point)++;
			(chidx + *point)->major = index;
			*type = ttype;
		} else
			ptrace("discard pinyin unit \"%s\"\n", (pyunits + index)->unit);
		break;
	case MAJOR_TYPE:
		if (ttype & MINOR_TYPE) {
			(chidx + *point)->minor = index;
			*type = ATOM_TYPE;
		} else
			goto mark;
		break;
	default:
		break;
	}
}
