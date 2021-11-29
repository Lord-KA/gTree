typedef int GTREE_TYPE;
#define GTREE_PRINTF_CODE "%d"

#include "gtree.h"

bool gTree_storeData(int data, size_t level, FILE *out) 
{
    for (size_t i = 0; i < level; ++i) 
        fprintf(out, "\t");
    fprintf(out, "%d\n", data);

    return 0;
}

bool gTree_restoreData(int *data, FILE *in)
{
    char buffer[MAX_BUFFER_LEN] = "";
    if (getline(buffer, MAX_BUFFER_LEN, in) == 1)
        return 1;
    fprintf(stderr, "buffer=#%s#\n", buffer);

    if (sscanf(buffer, "%d", data) != 1)
        return 1;

    if (getline(buffer, MAX_BUFFER_LEN, in) == 1)
        return 1;

    fprintf(stderr, "buffer=#%s#\n", buffer);
    return !consistsOnly(buffer, "]");
}


int test_1()
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

    fout = fopen("store.gtree", "w");
    gTree_storeSubTree(tree, tree->root, 0, fout);
    fclose(fout);

    gTree_dtor(tree);

    return 0;
}

int test_2()
{
    gTree tree;
    FILE *fin = fopen("store.gtree", "r");
    gTree_restoreTree(&tree, stderr, fin);
    fclose(fin);

    FILE *fout = fopen("dump.gv", "w");
    gTree_dumpPoolGraphViz(&tree, fout);
    fclose(fout);

    gTree_dtor(&tree);

    return 0;
}

int test_3()
{
    char buffer[MAX_BUFFER_LEN] = "1   \t {   ";
    char needle[MAX_BUFFER_LEN] = "{";
    printf("cosistsOnly(\"%s\", \"%s\") = %d\n", buffer, needle, consistsOnly(buffer, needle));
    return 0;
}

int main()
{
    test_2();
}
