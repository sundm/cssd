#pragma once

#define WASH_DEVICE    "0001"
#define STERILE_DEVICE "0002"
#define MALE "M"
#define FEMALE "F"

/**
 * Member functions defined in the class (regardless of whether it appears
 * in the header) automatically become inline, in that case, keyword `inline`
 * is completely redundant, the compiler does it anyway, but you can still
 * declare it just to describe intent.
 *
 * Free functions defined in header files (or member functions defined outside
 * the class, but in the header) must be marked inline, because otherwise,
 * every translation unit which includes the header will contain a definition
 * of the function, and the linker will complain about multiple definitions
 * (a violation of the One Definition Rule).
 *
 * Standard - 7.1.3/4
 * "An inline function with external linkage shall have the same address in all
 *  translation units. A static local variable in an extern inline function
 *  always refers to the same object. A string literal in the body of an extern
 *  inline function is the same object in different translation units"
 */

extern inline QString literal_device_state(const QString & isForbidden) {
	if (isForbidden == "0")
		return "空闲";
	if (isForbidden == "1")
		return "已禁用";
	if (isForbidden == "2")
		return "运行中";
	return "未知状态";
}

extern inline QString literal_program_type(const QString & typeValue) {
	if (typeValue == WASH_DEVICE)
		return "清洗预设程序";
	if (typeValue == STERILE_DEVICE)
		return "灭菌预设程序";
	return "未知程序";
}

extern inline QString literal_sterile_type(int type) {
	if (1 == type)
		return "高温灭菌器";
	if (2 == type)
		return "低温灭菌器";
	return "未知类型灭菌器";
}

extern inline QString literal_gender(const QString &value) {
	if (value == MALE) return "男";
	if (value == FEMALE) return "女";
	return QString();
}

extern inline QString literalSteType(int type) {
	switch (type) {
	case 1: return "有损坏";
	case 2: return "有缺失";
	case 3: return "缺失&损坏";
	default: return "-";
	}
}

extern inline QBrush brushForSteType(int type) {
	switch (type) {
	case 1: return QBrush(QColor(255, 215, 0));
	case 2: return QBrush(QColor(255, 106, 106));
	case 3: return QBrush(QColor(255, 106, 106));
	default: return QBrush();
	}
}