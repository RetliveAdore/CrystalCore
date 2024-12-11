/*
 * @Author: RetliveAdore lizaterop@gmail.com
 * @Date: 2024-12-08 21:50:19
 * @LastEditors: RetliveAdore lizaterop@gmail.com
 * @LastEditTime: 2024-12-10 23:29:20
 * @FilePath: \CrystalCore\src\rbt.c
 * @Description: 
 * 红黑树实现
 * Copyright (c) 2024 by lizaterop@gmail.com, All Rights Reserved. 
 */
#include <CrystalMemory.h>
#include <string.h>
#include "header.h"

extern void* CRAlloc(void* ptr, CRUINT64 size);

typedef struct treenode
{
	CRUINT64 key;
	CRLVOID data;
	CRBOOL red;
	struct treenode* parent;
	struct treenode* left;
	struct treenode* right;
}TREENODE, *PTREENODE;
typedef struct tree
{
	CRSTRUCTUREPUB pub;
	PTREENODE root;
}CRTREE, *PCRTREE;

CRAPI CRRBTREE CRTree(void)
{
	PCRTREE pInner = CRAlloc(NULL, sizeof(CRTREE));
	if (!pInner)
		return NULL;
	pInner->pub.total = 0;
	InitializeCriticalSection(&(pInner->pub.cs));
	pInner->root = NULL;
	return pInner;
}

CRAPI CRUINT64 CRTreeCount(CRRBTREE tree)
{
    PCRTREE pInner = tree;
    if (!pInner) return 0;
    return pInner->pub.total;
}

static PTREENODE _create_treenode_cr_(CRINT64 key, CRLVOID data, PTREENODE parent)
{
	PTREENODE pNode = CRAlloc(NULL, sizeof(TREENODE));
	if (pNode)
	{
		pNode->parent = parent;
		pNode->red = CRTRUE;
		pNode->key = key;
		pNode->data = data;
		pNode->left = NULL;
		pNode->right = NULL;
	}
	return pNode;
}
//当且仅当node是左节点时返回CRTRUE，其余所有情况返回CRFALSE
static inline CRBOOL _left_child_(PTREENODE node)
{
	if (node && node->parent && node == node->parent->left)
		return CRTRUE;
	return CRFALSE;
}
//当且仅当node存在且颜色为红色时返回CRTRUE
static inline CRBOOL _color_(PTREENODE node)
{
	return node == NULL ? CRFALSE : node->red;
}
//兄弟结点
static inline PTREENODE _sibling_(PTREENODE node)
{
	if (_left_child_(node))
		return node->parent->right;
	else if (node->parent)
		return node->parent->left;
	return NULL;
}
//左旋操作，顺便集成了根结点的判定
static void _left_rotate_(PTREENODE node, PCRTREE tree)
{
	PTREENODE top = node, right = node->right, child = right->left;
	if (!node->parent)
		tree->root = right;
	else if (_left_child_(node))
		node->parent->left = right;
	else node->parent->right = right;
	right->parent = top->parent;
	right->left = top;
	top->parent = right;
	top->right = child;
	if (child)
		child->parent = top;
}
//右旋操作，顺便集成了根结点的判定
static void _right_rotate_(PTREENODE node, PCRTREE tree)
{
	PTREENODE top = node, left = node->left, child = left->right;
	if (!node->parent)
		tree->root = left;
	else if (_left_child_(node))
		node->parent->left = left;
	else node->parent->right = left;
	left->parent = top->parent;
	left->right = top;
	top->parent = left;
	top->left = child;
	if (child)
		child->parent = top;
}
//获取前驱结点
static PTREENODE _prev_node_(PTREENODE node)
{
	/*
	* 此处不需要考虑向上寻找结点，因为在删除情况下不会用到这种情况
	* 在删除时，只要需要寻找前驱节点，就说明这个节点下方必然有两个节点
	*/
	node = node->left;
	while (node->right) node = node->right;
	return node;
}
//寻找对应结点，假如能够找到，返回此结点
static PTREENODE _look_up_(PTREENODE root, CRINT64 key, PTREENODE* parent)
{
	*parent = root;
Find:
	if (root->key == key)
		return root;
	else if (root->key < key && root->right)
	{
		root = root->right;
		*parent = root;
		goto Find;
	}
	else if (root->key > key && root->left)
	{
		root = root->left;
		*parent = root;
		goto Find;
	}
	return NULL;
}

//吐槽：插入操作比删除操作简单多了
CRAPI CRCODE CRTreePut(CRRBTREE tree, CRLVOID data, CRINT64 key)
{
	PCRTREE pInner = tree;
	if (!pInner)
		return 1;
	EnterCriticalSection(&(pInner->pub.cs));
	if (!pInner->root)
	{
		pInner->root = _create_treenode_cr_(key, data, NULL);
		if (!pInner->root)
		{
			LeaveCriticalSection(&(pInner->pub.cs));
			return 2;
		}
		pInner->root->red = CRFALSE;
		goto Done;
	}
	PTREENODE node = NULL, parent = NULL, uncle = NULL, grandpa = NULL;
	if (node = _look_up_(pInner->root, key, &parent))
	{
		node->data = data;
		goto Done;
	}
	node = _create_treenode_cr_(key, data, parent);
	if (!node)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return 2;
	}
	if (key < parent->key)
		parent->left = node;
	else parent->right = node;
Fix:
	if (node == pInner->root)
	{
		node->red = CRFALSE;
		goto Done;
	}
	if (!_color_(node->parent))
	{
		goto Done;
	}
	parent = node->parent;
	uncle = _sibling_(parent);
	grandpa  = parent->parent;
	if (_color_(uncle))
	{
		parent->red = CRFALSE;
		uncle->red = CRFALSE;
		grandpa->red = CRTRUE;
		node = grandpa;
		goto Fix;
	}
	else
	{
		if (_left_child_(parent))
		{
			if (!_left_child_(node))
			{
				_left_rotate_(parent, pInner);
				parent = node;
				node = node->left;
			}
			parent->red = CRFALSE;
			grandpa->red = CRTRUE;
			_right_rotate_(grandpa, pInner);
		}
		else
		{
			if (_left_child_(node))
			{
				_right_rotate_(parent, pInner);
				parent = node;
				node = node->right;
			}
			parent->red = CRFALSE;
			grandpa->red = CRTRUE;
			_left_rotate_(grandpa, pInner);
		}
	}
Done:
	pInner->pub.total++;
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

//
//噩梦——删除红黑树节点

//declaration
static void _case_1_red_sibling_(PCRTREE tree, PTREENODE node);
static void _case_2_far_red_nephew_(PCRTREE tree, PTREENODE node);
static void _case_3_near_red_nephew(PCRTREE tree, PTREENODE node);
static void _case_4_red_parent_(PCRTREE tree, PTREENODE node);
static void _case_5_all_black(PCRTREE tree, PTREENODE node);

/*假如兄弟结点是红色，就转化成父节点是红色的情况，以统一处理
     p              S   |      p             S
    / \            / \  |     / \           / \
   D  red   ==>  red ...|   red  D   ==>  ... red
  ... /  \       / \    |   /  \ ...          / \
 .. ...  ...    D  ...  | ...  ... ..       ...  D
*/
void _case_1_red_sibling_(PCRTREE tree, PTREENODE node)
{
	node->parent->red = CRTRUE;
	if (_left_child_(node))
	{
		node->parent->right->red = CRFALSE;
		_left_rotate_(node->parent, tree);
	}
	else
	{
		node->parent->left->red = CRFALSE;
		_right_rotate_(node->parent, tree);
	}
}
/*兄弟节点是黑色且挂了一个远端红色侄子结点
     p              p
    / \            / \
   D   S          S   D
  ... / \        / \  ...
 .. ... red    red ...  ..
*/
void _case_2_far_red_nephew_(PCRTREE tree, PTREENODE node)
{
	PTREENODE sibling = _sibling_(node);
	PTREENODE farNephew;
	sibling->red = node->parent->red;
	node->parent->red = CRFALSE;
	if (_left_child_(node))
	{
		farNephew = sibling->right;
		_left_rotate_(node->parent, tree);
	}
	else
	{
		farNephew = sibling->left;
		_right_rotate_(node->parent, tree);
	}
	farNephew->red = CRFALSE;
}
/*兄弟节点设计黑色且只挂了一个近端红色侄子节点
	 p              p
    / \            / \
   D   S          S   D
  ... / \        / \  ...
 .. red  ...   ...  red ..
转化成为情况2来处理
*/
void _case_3_near_red_nephew(PCRTREE tree, PTREENODE node)
{
	PTREENODE sibling = _sibling_(node);
	PTREENODE nearNephew;
	if (_left_child_(node))
	{
		nearNephew = sibling->left;
		_right_rotate_(sibling, tree);
	}
	else
	{
		nearNephew = sibling->right;
		_left_rotate_(sibling, tree);
	}
	sibling->red = CRTRUE;
	nearNephew->red = CRFALSE;
	//转化之后就可以使用情况2来处理了
	_case_2_far_red_nephew_(tree, node);
}
/*父结点为红色而且有兄弟结点
* 将父节点变为黑色，兄弟节点变红就能平衡
    red          red
    / \          / \
   D   S        S   D
  / \ / \      / \ / \
 .........    .........
*/
void _case_4_red_parent_(PCRTREE tree, PTREENODE node)
{
	PTREENODE sibling = _sibling_(node);
	node->parent->red = CRFALSE;
	sibling->red = CRTRUE;
}
/*唯一一种需要用到迭代的情况，全黑
   Black        Black
    / \          / \
   D Black    Black D
  / \ / \      / \ / \
 .........    .........
*/
void _case_5_all_black(PCRTREE tree, PTREENODE node)
{
	PTREENODE sibling = _sibling_(node);
	sibling->red = CRTRUE;
}

void _fix_single_black_node_(PCRTREE tree, PTREENODE node)
{
	PTREENODE sibling;
Fix:
	if (node == tree->root)
	{
		node->red = CRFALSE;
		return;
	}
	//这种情况下兄弟节点必然存在，否则就不平衡了
	sibling = _sibling_(node);
	if (_color_(sibling))
	{
		_case_1_red_sibling_(tree, node);
		goto Fix;
	}
	else if (_left_child_(node))
	{
		if (_color_(sibling->right))
			_case_2_far_red_nephew_(tree, node);
		else if (_color_(sibling->left))
			_case_3_near_red_nephew(tree, node);
		else if (_color_(node->parent))
			_case_4_red_parent_(tree, node);
		else
		{
			_case_5_all_black(tree, node);
			node = node->parent;
			goto Fix;
		}
	}
	else
	{
		if (_color_(sibling->left))
			_case_2_far_red_nephew_(tree, node);
		else if (_color_(sibling->right))
			_case_3_near_red_nephew(tree, node);
		else if (_color_(node->parent))
			_case_4_red_parent_(tree, node);
		else
		{
			_case_5_all_black(tree, node);
			node = node->parent;
			goto Fix;
		}
	}
}

void _fix_parent_(PTREENODE node)
{
	if (_left_child_(node))
		node->parent->left = NULL;
	else if (node->parent)
		node->parent->right = NULL;
}

CRAPI CRCODE CRTreeGet(CRSTRUCTURE tree, CRLVOID* data, CRINT64 key)
{
	PCRTREE pInner = tree;
	if (!pInner)
		return 1;
	EnterCriticalSection(&(pInner->pub.cs));
	if (!pInner->root)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return 2;
	}
	PTREENODE parent;
	PTREENODE node = _look_up_(pInner->root, key, &parent);
	if (!node)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return 3;
	}
	if (data) *data = node->data;
Fix:
	if (node->left && node->right)
	{
		PTREENODE prev = _prev_node_(node);
		node->data = prev->data;
		node->key = prev->key;
		node = prev;
	}  //假如红黑树的结构是正确的，转换一次就够了
	if (node->red)
		goto Done;
	else if (node->left)
	{
		node->data = node->left->data;
		node->key = node->left->key;
		node = node->left;
	}
	else if (node->right)
	{
		node->data = node->right->data;
		node->key = node->right->key;
		node = node->right;
	}
	else
		_fix_single_black_node_(pInner, node);
Done:
	_fix_parent_(node);
	if (node == pInner->root)
		pInner->root = NULL;
	CRAlloc(node, 0);
	pInner->pub.total--;
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

CRAPI CRCODE CRTreeSeek(CRRBTREE tree, CRLVOID* data, CRINT64 key)
{
	PCRTREE pInner = tree;
	if (!pInner)
		return 1;
	EnterCriticalSection(&(pInner->pub.cs));
	if (!pInner->root)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return 2;
	}
	PTREENODE parent;
	PTREENODE node = _look_up_(pInner->root, key, &parent);
	if (!node)
	{
		LeaveCriticalSection(&(pInner->pub.cs));
		return 3;
	}
	if (data) *data = node->data;
	LeaveCriticalSection(&(pInner->pub.cs));
	return 0;
}

static void _clear_tree_node_(PTREENODE node)
{
	if (node->left)
		_clear_tree_node_(node->left);
	if (node->right)
		_clear_tree_node_(node->right);
	CRAlloc(node, 0);
}
CRAPI CRCODE CRFreeTree(CRRBTREE tree)
{
	PCRTREE pInner = tree;
	if (!pInner) return 1;
	if (pInner->root)
		_clear_tree_node_(pInner->root);
	CRAlloc(pInner, 0);
	return 0;
}

static void _for_tree_node_(PTREENODE node, IteratorCallback cal, CRLVOID user)
{
	if (node->left)
		_for_tree_node_(node->left, cal, user);
	cal(node->data, user, node->key);  //中序遍历当然是放在中间（这一点十分形象）
	if (node->right)
		_for_tree_node_(node->right, cal, user);
}
static void _for_tree_(PCRTREE tree, IteratorCallback cal, CRLVOID user)
{
	if (tree->root)
		_for_tree_node_(tree->root, cal, user);
}
CRAPI CRCODE CRTreeIterator(CRRBTREE tree, IteratorCallback cal, CRLVOID user)
{
	PCRTREE pInner = tree;
	if (!pInner)
		return 1;
	if (!cal)
		return 2;
	_for_tree_(pInner, cal, user);
	return 0;
}