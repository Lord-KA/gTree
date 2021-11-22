typedef int GTREE_TYPE;
#define GTREE_PRINTF_CODE "%d"

#include "gtree.h"

int main()
{
    gTree treeStruct;
    gTree *tree = &treeStruct;
    gTree_ctor(tree, NULL);

    gTree_Node *node = NULL;
    gObjPool_get(&tree->pool, tree->root, &node);
    node->data = 1000;

    gTree_addChild(tree, tree->root, 1100);
    gTree_addChild(tree, tree->root, 1200);
    gTree_addChild(tree, tree->root, 1300);
    gTree_addChild(tree, tree->root, 1400);

    gTree_addSibling(tree, 1, 1500);

    gTree_addChild(tree, 1, 2100);
    gTree_addChild(tree, 1, 2200);
    gTree_addChild(tree, 1, 2300);

    FILE *fout = fopen("dump.gv", "w");
    gTree_dumpPoolGraphViz(tree, fout);
    fclose(fout);

    gObjPool_dumpFree(&tree->pool, stderr);

    gTree_dtor(tree);
}
