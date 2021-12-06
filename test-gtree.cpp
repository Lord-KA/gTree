typedef int GTREE_TYPE;

#include "gtest/gtest.h"
#include "gtree.h"
#include <random>

std::mt19937 rnd(179);

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

bool gTree_printData(int data, FILE *out)
{
    fprintf(out, "%d", data);
    return 0;
}

int restore()
{
    gTree tree;
    FILE *fin = fopen("store.gtree", "r");
    EXPECT_FALSE(gTree_restoreTree(&tree, stderr, fin));
    fclose(fin);

    FILE *fout = fopen("dump.gv", "w");
    EXPECT_FALSE(gTree_dumpPoolGraphViz(&tree, fout));
    fclose(fout);

    EXPECT_FALSE(gTree_dtor(&tree));

    return 0;
}


TEST(Manual, fill_store_restore)
{
    gTree treeStruct;
    gTree *tree = &treeStruct;
    EXPECT_FALSE(gTree_ctor(tree, NULL));

    gTree_Node *node = NULL;
    EXPECT_FALSE(gObjPool_get(&tree->pool, tree->root, &node));
    node->data = 1000;

    size_t id = 0;

    EXPECT_FALSE(gTree_addChild(tree, tree->root, &id, 1100));
    EXPECT_FALSE(gTree_addChild(tree, tree->root, &id, 1200));
    EXPECT_FALSE(gTree_addChild(tree, tree->root, &id, 1300));
    EXPECT_FALSE(gTree_addChild(tree, tree->root, &id, 1400));

    EXPECT_FALSE(gTree_addSibling(tree, 1, &id, 1500));


    EXPECT_FALSE(gTree_addChild(tree, 5, &id, 2100));
    EXPECT_FALSE(gTree_addChild(tree, 5, &id, 2200));
    EXPECT_FALSE(gTree_addChild(tree, 5, &id, 2300));
    EXPECT_FALSE(gTree_addChild(tree, 6,  &id, 3100));
    EXPECT_FALSE(gTree_addChild(tree, 6,  &id, 3200));
    EXPECT_FALSE(gTree_addChild(tree, 10, &id, 4100));

    EXPECT_FALSE(gTree_delChild(tree, 0, 4, NULL));

    FILE *fout = fopen("dump.gv", "w");
    EXPECT_FALSE(gTree_dumpPoolGraphViz(tree, fout));
    fclose(fout);

    EXPECT_FALSE(gObjPool_dumpFree(&tree->pool, stderr));

    fout = fopen("store.gtree", "w");
    EXPECT_FALSE(gTree_storeSubTree(tree, tree->root, 0, fout));
    fclose(fout);

    EXPECT_FALSE(gTree_dtor(tree));

    restore();
}

TEST(Auto, massive_random_filling)
{
    gTree treeStruct;
    gTree *tree = &treeStruct;
    EXPECT_FALSE(gTree_ctor(tree, NULL));

    gTree_Node *node = NULL;
    EXPECT_FALSE(gObjPool_get(&tree->pool, tree->root, &node));
    node->data = 0xFAFAFAFA;

    size_t id = 0;
    for (size_t i = 1; i < 1000; ++i) {
        if (rnd() % 3 == 1 && i > 1) {
            EXPECT_FALSE(gTree_addSibling(tree, rnd() % (i - 1) + 1, &id, rnd()));
        } else {
            EXPECT_FALSE(gTree_addChild(tree, rnd() % i, &id, rnd()));
        }
    }  

    FILE *fout = fopen("dump.gv", "w");
    EXPECT_FALSE(gTree_dumpPoolGraphViz(tree, fout));
    fclose(fout);

    EXPECT_FALSE(gTree_dtor(tree));
}
