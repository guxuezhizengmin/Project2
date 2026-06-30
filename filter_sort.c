#include "filter_sort.h"

typedef struct {
	CourseRecord *array;
	int capacity;
	int size;
	FilterCondition cond;
} FilterArg;

static void filter_cb(CourseRecord record, void *arg) {
	FilterArg *farg = (FilterArg*)arg;
	FilterCondition *c = &farg->cond;
	
	if (c->course_name[0] != '\0') {
		int match = c->fuzzy_course ?
		fuzzy_match(record.course_name, c->course_name) :
		strcmp(record.course_name, c->course_name) == 0;
		if (!match) return;
	}
	if (c->semester[0] != '\0' && strcmp(record.semester, c->semester) != 0) return;
	if (c->score_min != -1 && record.score < c->score_min) return;
	if (c->score_max != -1 && record.score > c->score_max) return;
	if (c->college[0] != '\0' && strcmp(record.college, c->college) != 0) return;
	
	if (farg->size >= farg->capacity) {
		int new_cap = farg->capacity == 0 ? 64 : farg->capacity * 2;
		CourseRecord *new_arr = (CourseRecord*)realloc(farg->array, new_cap * sizeof(CourseRecord));
		if (!new_arr) return;
		farg->array = new_arr;
		farg->capacity = new_cap;
	}
	farg->array[farg->size++] = record;
}

CourseRecord* filter_records(void *ds, DSInterface iface, FilterCondition cond, int *out_count) {
	*out_count = 0;
	if (iface.size(ds) == 0) return NULL;
	
	FilterArg farg = {NULL, 0, 0, cond};
	iface.traverse(ds, filter_cb, &farg);
	*out_count = farg.size;
	return farg.array;
}

static SortRule *g_rules = NULL;
static int g_rule_num = 0;

static int cmp_record(const void *a, const void *b) {
	CourseRecord *r1 = (CourseRecord*)a;
	CourseRecord *r2 = (CourseRecord*)b;
	
	for (int i = 0; i < g_rule_num; i++) {
		int cmp = 0;
		switch (g_rules[i].field) {
			case SORT_STUDENT_ID:  cmp = strcmp(r1->student_id, r2->student_id); break;
			case SORT_NAME:        cmp = strcmp(r1->name, r2->name); break;
			case SORT_SCORE:       cmp = r1->score - r2->score; break;
			case SORT_CREDIT:      cmp = (r1->credit > r2->credit) - (r1->credit < r2->credit); break;
			case SORT_ENROLL_DATE: cmp = compare_date(r1->enroll_date, r2->enroll_date); break;
		}
		if (cmp != 0) return g_rules[i].descending ? -cmp : cmp;
	}
	return 0;
}

void sort_records(CourseRecord *records, int count, SortRule *rules, int rule_count) {
	if (count <= 0 || rules == NULL || rule_count <= 0) return;
	g_rules = rules;
	g_rule_num = rule_count;
	qsort(records, count, sizeof(CourseRecord), cmp_record);
}

int export_records_to_csv(CourseRecord *records, int count, const char *filename) {
	if (records == NULL || count <= 0) return -1;
	FILE *fp = fopen(filename, "w");
	if (!fp) return -1;
	fprintf(fp, "学号,姓名,学院,课程编号,课程名称,学分,选课学期,选课日期,成绩\n");
	for (int i = 0; i < count; i++) {
		CourseRecord *r = &records[i];
		fprintf(fp, "%s,%s,%s,%s,%s,%.1f,%s,%s,%d\n",
				r->student_id, r->name, r->college,
				r->course_id, r->course_name, r->credit,
				r->semester, r->enroll_date, r->score);
	}
	fclose(fp);
	return 0;
}
