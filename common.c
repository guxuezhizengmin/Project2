#include "common.h"

/* 日期比较：YYYY-MM-DD字典序等价于时间序 */
int compare_date(const char *date1, const char *date2) {
	return strcmp(date1, date2);
}

/* 模糊字符串匹配 */
int fuzzy_match(const char *str, const char *substr) {
	return strstr(str, substr) != NULL;
}

/* 生成唯一主键：学号+课程编号 */
void make_key(const char *student_id, const char *course_id, char *key_buf) {
	strcpy(key_buf, student_id);
	strcat(key_buf, course_id);
}

/* ==================== 安全输入工具集 ==================== */

/* 安全输入整数：限定范围，输入错误自动重输，自动清理缓冲区 */
int safe_input_int(int min, int max, const char *prompt) {
	int value;
	int scanf_ret;
	while (1) {
		printf("%s", prompt);
		scanf_ret = scanf("%d", &value);
		
		/* 强制清空输入缓冲区所有残留字符 */
		int c;
		while ((c = getchar()) != '\n' && c != EOF);
		
		if (scanf_ret != 1) {
			printf("[错误] 输入无效，请输入纯数字！\n");
			continue;
		}
		if (value < min || value > max) {
			printf("[错误] 超出范围，请输入 %d ~ %d 之间的数字！\n", min, max);
			continue;
		}
		return value;
	}
}

/* 安全输入字符串：限定最大长度，自动去换行，超长自动截断 */
void safe_input_str(char *buf, int max_len, const char *prompt) {
	printf("%s", prompt);
	if (fgets(buf, max_len, stdin) == NULL) {
		buf[0] = '\0';
		return;
	}
	
	/* 去掉末尾换行符 */
	size_t len = strlen(buf);
	if (len > 0 && buf[len - 1] == '\n') {
		buf[len - 1] = '\0';
	} else {
		/* 输入超长，清空缓冲区剩余内容 */
		int c;
		while ((c = getchar()) != '\n' && c != EOF);
		printf("[提示] 输入过长，已自动截断至 %d 字符\n", max_len - 1);
	}
}

/* 安全确认：只接受Y/y/N/n，返回1=确认 0=取消 */
int safe_confirm(const char *prompt) {
	char buf[16];
	while (1) {
		safe_input_str(buf, sizeof(buf), prompt);
		if (strcmp(buf, "Y") == 0 || strcmp(buf, "y") == 0) return 1;
		if (strcmp(buf, "N") == 0 || strcmp(buf, "n") == 0) return 0;
		printf("[错误] 输入无效，请输入 Y 或 N ！\n");
	}
}
