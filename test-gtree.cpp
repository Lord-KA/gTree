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

    size_t id = 0;

    gTree_addChild(tree, tree->root, &id, 1100);
    gTree_addChild(tree, tree->root, &id, 1200);
    gTree_addChild(tree, tree->root, &id, 1300);
    gTree_addChild(tree, tree->root, &id, 1400);

    gTree_addSibling(tree, 1, &id, 1500);

    gTree_addChild(tree, 5, &id, 2100);
    gTree_addChild(tree, 5, &id, 2200);
    gTree_addChild(tree, 5, &id, 2300);
    gTree_addChild(tree, 6,  &id, 3100);
    gTree_addChild(tree, 6,  &id, 3200);
    gTree_addChild(tree, 10, &id, 4100);

    gTree_delChild(tree, 0, 4, NULL);

    FILE *fout = fopen("dump.gv", "w");
    gTree_dumpPoolGraphViz(tree, fout);
    fclose(fout);

    gObjPool_dumpFree(&tree->pool, stderr);

    gTree_dtor(tree);
}
