#pragma once

/**
 * @file Header containing generalized tree data structure
 */

#include "stdio.h"
#include "stdlib.h"

#include "gutils.h"

static const size_t MAX_MSG_LEN = 64;       /// Max log message length

static const char LOG_DELIM[] = "=============================";    /// Delim line for text logs

/** 
 * @brief basic tree node containing children id and the data
 */
struct gTree_Node         
{
    GTREE_TYPE data;                /// Stored user-provided data
    size_t child;                   /// Id of the first child
    size_t parent;                  /// Id of the previos node in tree
    size_t sibling;                 /// Id of the right sibling node
} typedef gTree_Node;

typedef gTree_Node GOBJPOOL_TYPE;           /// Type for utility Object Pool data structure

#include "gobjpool.h"           // including utility Object Pool data structure


/**
 * @brief main linked list structure
 */
struct gTree
{
    size_t root;                /// id of the root node
    gObjPool pool;              /// Object Pool for memory management
    FILE *logStream;            /// Log stream for centralized logging
} typedef gTree;


/**
 * @brief status codes for gTree
 */
enum gTree_status 
{
    gTree_status_OK,
    gTree_status_AllocErr,
    gTree_status_BadCapacity, 
    gTree_status_BadStructPtr,
    gTree_status_BadId,
    gTree_status_BadPos,
    gTree_status_BadNodePtr,
    gTree_status_BadDumpOutPtr,
    gTree_status_Cnt,
};


/**
 * @brief status codes explanations and error msgs for logs
 */
static const char gTree_statusMsg[gTree_status_Cnt][MAX_MSG_LEN] = {
    "OK",
    "Allocation error",
    "Bad capacity error",
    "Bad structure pointer provided",
    "Bad id provided",
    "Bad position requested",
    "Bad node pointer provided",
    "Bad FILE pointer provided to graphViz dump",
};


/**
 * @brief macro that checks if objPool status is OK and convers error code to compatible gTree_status otherwize
 */
#define CHECK_POOL_STATUS(status) ({        \
    if ((status) != gObjPool_status_OK)      \
        return (gTree_status)(status);        \
})


/**
 * @brief Local version of ASSERT_LOG macro 
 */
#ifndef NLOGS
#define GTREE_ASSERT_LOG(expr, errCode, logStream) ({                             \
    if (!(expr)) {                                                                 \
        fprintf((logStream),  "%s in %s!\n", gTree_statusMsg[(errCode)], __func__); \
        return (gTree_status)(errCode);                                              \
    }                                                                                 \
})
#else
#define GTREE_ASSERT_LOG(...) 
#endif

#define GTREE_NODE_BY_ID(tree, id) ({                         \
    gTree_Node *node;                                          \
    CHECK_POOL_STATUS(gObjPool_get(&tree->pool, id, &node));    \
    node;                                                        \
})

/**
 * @brief gTree constructor that initiates objPool and logStream and creates zero node
 * @param tree pointer to structure to construct on
 * @param newLogStream new log stream, could be `NULL`, then logs will be written to `stderr`
 * @return gTree status code
 */
gTree_status gTree_ctor(gTree *tree, FILE *newLogStream) 
{
    if (!gPtrValid(tree)) {                                          
        FILE *out;                                                   
        if (!gPtrValid(newLogStream))                                
            out = stderr;                                            
        else                                                         
            out = newLogStream;                                      
        fprintf(out, "ERROR: bad structure ptr provided to tree ctor!\n");
        return gTree_status_BadStructPtr;                         
    }

    tree->logStream = stderr;
    if (gPtrValid(newLogStream))
        tree->logStream = newLogStream;

    gObjPool_status status = gObjPool_ctor(&tree->pool, -1, newLogStream);
    CHECK_POOL_STATUS(status);

    status = gObjPool_alloc(&tree->pool, &tree->root);
    CHECK_POOL_STATUS(status);
    gTree_Node *node;
    status = gObjPool_get(&tree->pool, tree->root, &node);
    CHECK_POOL_STATUS(status);
    node->parent  = -1;
    node->child   = -1;
    node->sibling = -1;
    return gTree_status_OK;
}


/**
 * @brief gTree destructor 
 * @param tree pointer to structure to destruct
 * @return gTree status code
 */
gTree_status gTree_dtor(gTree *tree) 
{
    GTREE_ASSERT_LOG(gPtrValid(tree), gTree_status_BadStructPtr, stderr);
    gTree_Node *node = NULL;
    gObjPool_status status = gObjPool_status_OK;
    status = gObjPool_get(&tree->pool, tree->root, &node);
    if (node != NULL) {
        node->parent  = -1;
        node->child   = -1;
        node->sibling = -1;
    }
    status = gObjPool_dtor(&tree->pool);
    return gTree_status_OK;
}


gTree_status gTree_addSibling(gTree *tree, size_t siblingId, size_t *id, GTREE_TYPE data)
{
    gTree_Node *sibling = NULL, *child = NULL;
    gObjPool_status status = gObjPool_status_OK;

    size_t childId = -1;
    status = gObjPool_alloc(&tree->pool, &childId);
    CHECK_POOL_STATUS(status);

    status = gObjPool_get(&tree->pool, siblingId, &sibling);
    CHECK_POOL_STATUS(status);

    status = gObjPool_get(&tree->pool, childId, &child);
    CHECK_POOL_STATUS(status);

    while (sibling->sibling != -1) {
        siblingId = sibling->sibling;
        status = gObjPool_get(&tree->pool, siblingId, &sibling);
        CHECK_POOL_STATUS(status);
    }
    sibling->sibling = childId;    
    child->parent  = sibling->parent;
    child->child   = -1;
    child->sibling = -1;
    child->data = data;
    
    return gTree_status_OK;
}


gTree_status gTree_addChild(gTree *tree, size_t nodeId, size_t *id, GTREE_TYPE data)
{
    GTREE_ASSERT_LOG(gPtrValid(tree), gTree_status_BadStructPtr, stderr);
    gTree_Node *node = NULL, *child = NULL, *sibling = NULL;
    gObjPool_status status = gObjPool_status_OK;
    size_t childId = -1;

    status = gObjPool_alloc(&tree->pool, &childId);
    CHECK_POOL_STATUS(status);

    status = gObjPool_get(&tree->pool, nodeId, &node);
    CHECK_POOL_STATUS(status);
    size_t siblingId = node->child;
 
    status = gObjPool_get(&tree->pool, childId, &child);
    CHECK_POOL_STATUS(status);

    if (node->child == -1) {
        node->child = childId;
    } else {
        status = gObjPool_get(&tree->pool, siblingId, &sibling);
        CHECK_POOL_STATUS(status);

        while (sibling->sibling != -1) {
            siblingId = sibling->sibling;
            status = gObjPool_get(&tree->pool, siblingId, &sibling);
            CHECK_POOL_STATUS(status);
        }
        sibling->sibling = childId;
    }
    child->parent  = nodeId;
    child->child   = -1;
    child->sibling = -1;
    child->data = data;
    
    return gTree_status_OK;
}

gTree_status gTree_delChild(gTree *tree, size_t parentId, size_t pos, GTREE_TYPE *data)
{
    size_t siblingId = GTREE_NODE_BY_ID(tree, parentId)->child;
    gTree_Node *node    = NULL;
    gTree_Node *sibling = NULL;
    size_t childId = -1;
    size_t nodeId  = -1;
    if (pos == 0) {
        nodeId = siblingId;
        node = GTREE_NODE_BY_ID(tree, nodeId);
        sibling = GTREE_NODE_BY_ID(tree, siblingId);
        childId = sibling->child;
        GTREE_NODE_BY_ID(tree, parentId)->child = childId;
    } else {
        for (size_t i = 0; i + 1 < pos; ++i) {
            siblingId = GTREE_NODE_BY_ID(tree, siblingId)->sibling;
        }
        sibling = GTREE_NODE_BY_ID(tree, siblingId);
        nodeId = sibling->sibling;
        node = GTREE_NODE_BY_ID(tree, nodeId);
        childId = node->child;
        sibling->sibling = childId;
    }
    
    assert(gPtrValid(node));

    if (childId != -1) {
        size_t subSiblingId = childId;
        gTree_Node *subSibling = NULL;
        while (subSiblingId != -1) {
            subSibling = GTREE_NODE_BY_ID(tree, subSiblingId);
            subSiblingId = subSibling->sibling;
            subSibling->parent = parentId;
        }

        subSibling->sibling = node->sibling;
    } else {
        sibling->sibling = node->sibling;
        node->sibling = -1;
    }

    if (data)
        *data = node->data;

    gObjPool_free(&tree->pool, nodeId);
    return gTree_status_OK;
}

/**
 * @brief dumps objPool of the tree to fout stream in GraphViz format
 * @param tree pointer to structure
 * @param fout stream to write dump to
 * @return gTree status code
 */
gTree_status gTree_dumpPoolGraphViz(const gTree *tree, FILE *fout)
{
    GTREE_ASSERT_LOG(gPtrValid(tree), gTree_status_BadStructPtr,  stderr);
    GTREE_ASSERT_LOG(gPtrValid(fout), gTree_status_BadDumpOutPtr, stderr);
    
    fprintf(fout, "digraph dilist {\n\tnode [shape=record]\n\tsubgraph cluster {\n");
    
    for (size_t i = 0; i < tree->pool.capacity; ++i) {
        gTree_Node *node = &tree->pool.data[i].val;
        fprintf(fout, "\t\tnode%lu [label=\"Node %lu | | {data | " GTREE_PRINTF_CODE "}\"]\n", i, i, node->data);
    }

    fprintf(fout, "\t}\n");
    
    for (size_t i = 0; i < tree->pool.capacity; ++i) {
        gTree_Node *node = &tree->pool.data[i].val;
        fprintf(stderr, "id = %lu | allocated = %d | data = %d\n", i, (&tree->pool.data[i])->allocated, node->data);
        if ((&tree->pool.data[i])->allocated && (node->parent != -1)) {
            fprintf(fout, "\tnode%lu -> node%lu\n", node->parent, i);
            if (node->sibling != -1)
                fprintf(fout, "\tnode%lu -> node%lu [style=dotted]\n", i, node->sibling);
        }
    }

    fprintf(fout, "}\n");
    return gTree_status_OK;
}
