#include "avl_tree.h"

static int max(int a, int b) { return a > b ? a : b; }
static int get_height(AVLNode *node) { return node ? node->height : 0; }
static void update_height(AVLNode *node) {
	if (node) node->height = max(get_height(node->left), get_height(node->right)) + 1;
}
static int get_balance(AVLNode *node) {
	return node ? get_height(node->left) - get_height(node->right) : 0;
}

static AVLNode* right_rotate(AVLNode *y) {
	AVLNode *x = y->left;
	AVLNode *T2 = x->right;
	x->right = y;
	y->left = T2;
	update_height(y);
	update_height(x);
	return x;
}

static AVLNode* left_rotate(AVLNode *x) {
	AVLNode *y = x->right;
	AVLNode *T2 = y->left;
	y->left = x;
	x->right = T2;
	update_height(x);
	update_height(y);
	return y;
}

static AVLNode* insert_node(AVLNode *node, CourseRecord record, int *success) {
	if (!node) {
		AVLNode *new_node = (AVLNode*)malloc(sizeof(AVLNode));
		if (!new_node) { *success = -1; return NULL; }
		new_node->data = record;
		new_node->left = new_node->right = NULL;
		new_node->height = 1;
		*success = 0;
		return new_node;
	}
	
	char key_new[21], key_node[21];
	make_key(record.student_id, record.course_id, key_new);
	make_key(node->data.student_id, node->data.course_id, key_node);
	int cmp = strcmp(key_new, key_node);
	
	if (cmp < 0) node->left = insert_node(node->left, record, success);
	else if (cmp > 0) node->right = insert_node(node->right, record, success);
	else { *success = -1; return node; }
	
	update_height(node);
	int balance = get_balance(node);
	
	if (balance > 1 && cmp < 0) return right_rotate(node);
	if (balance < -1 && cmp > 0) return left_rotate(node);
	if (balance > 1 && cmp > 0) {
		node->left = left_rotate(node->left);
		return right_rotate(node);
	}
	if (balance < -1 && cmp < 0) {
		node->right = right_rotate(node->right);
		return left_rotate(node);
	}
	return node;
}

static AVLNode* find_min_node(AVLNode *node) {
	while (node && node->left) node = node->left;
	return node;
}

static AVLNode* delete_node(AVLNode *node, const char *key, int *success) {
	if (!node) { *success = -1; return NULL; }
	
	char key_node[21];
	make_key(node->data.student_id, node->data.course_id, key_node);
	int cmp = strcmp(key, key_node);
	
	if (cmp < 0) node->left = delete_node(node->left, key, success);
	else if (cmp > 0) node->right = delete_node(node->right, key, success);
	else {
		*success = 0;
		if (!node->left || !node->right) {
			AVLNode *temp = node->left ? node->left : node->right;
			if (!temp) { temp = node; node = NULL; }
			else *node = *temp;
			free(temp);
		} else {
			AVLNode *temp = find_min_node(node->right);
			node->data = temp->data;
			char temp_key[21];
			make_key(temp->data.student_id, temp->data.course_id, temp_key);
			node->right = delete_node(node->right, temp_key, success);
		}
	}
	
	if (!node) return node;
	update_height(node);
	int balance = get_balance(node);
	
	if (balance > 1 && get_balance(node->left) >= 0) return right_rotate(node);
	if (balance > 1 && get_balance(node->left) < 0) {
		node->left = left_rotate(node->left);
		return right_rotate(node);
	}
	if (balance < -1 && get_balance(node->right) <= 0) return left_rotate(node);
	if (balance < -1 && get_balance(node->right) > 0) {
		node->right = right_rotate(node->right);
		return left_rotate(node);
	}
	return node;
}

static AVLNode* find_node(AVLNode *node, const char *key) {
	if (!node) return NULL;
	char key_node[21];
	make_key(node->data.student_id, node->data.course_id, key_node);
	int cmp = strcmp(key, key_node);
	if (cmp < 0) return find_node(node->left, key);
	else if (cmp > 0) return find_node(node->right, key);
	else return node;
}

static void inorder(AVLNode *node, void (*cb)(CourseRecord, void*), void *arg) {
	if (!node) return;
	inorder(node->left, cb, arg);
	cb(node->data, arg);
	inorder(node->right, cb, arg);
}

static void destroy_tree(AVLNode *node) {
	if (!node) return;
	destroy_tree(node->left);
	destroy_tree(node->right);
	free(node);
}

AVLTree* avl_tree_create() {
	AVLTree *tree = (AVLTree*)malloc(sizeof(AVLTree));
	if (!tree) return NULL;
	tree->root = NULL;
	tree->count = 0;
	return tree;
}

static int avl_insert(void *ds, CourseRecord record) {
	AVLTree *tree = (AVLTree*)ds;
	int success = -1;
	tree->root = insert_node(tree->root, record, &success);
	if (success == 0) tree->count++;
	return success;
}

static int avl_remove(void *ds, const char *sid, const char *cid) {
	AVLTree *tree = (AVLTree*)ds;
	char key[21];
	make_key(sid, cid, key);
	int success = -1;
	tree->root = delete_node(tree->root, key, &success);
	if (success == 0) tree->count--;
	return success;
}

static int avl_update(void *ds, const char *sid, const char *cid, int new_score) {
	char key[21];
	make_key(sid, cid, key);
	AVLNode *node = find_node(((AVLTree*)ds)->root, key);
	if (!node) return -1;
	node->data.score = new_score;
	return 0;
}

static CourseRecord* avl_find(void *ds, const char *sid, const char *cid) {
	char key[21];
	make_key(sid, cid, key);
	AVLNode *node = find_node(((AVLTree*)ds)->root, key);
	return node ? &(node->data) : NULL;
}

static void avl_traverse(void *ds, void (*cb)(CourseRecord, void*), void *arg) {
	inorder(((AVLTree*)ds)->root, cb, arg);
}

static int avl_size(void *ds) {
	return ((AVLTree*)ds)->count;
}

static void avl_destroy(void *ds) {
	AVLTree *tree = (AVLTree*)ds;
	destroy_tree(tree->root);
	free(tree);
}

DSInterface avl_tree_get_interface() {
	DSInterface iface;
	iface.insert   = avl_insert;
	iface.remove   = avl_remove;
	iface.update   = avl_update;
	iface.find     = avl_find;
	iface.traverse = avl_traverse;
	iface.size     = avl_size;
	iface.destroy  = avl_destroy;
	return iface;
}
