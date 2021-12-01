#pragma once

/**
 * @file Header containing generalized tree data structure
 */

#include "stdio.h"
#include "stdlib.h"

#include "gutils.h"             /// Some handy utils

static const size_t MAX_MSG_LEN = 64;           /// Max log message length

static const size_t MAX_BUFFER_LEN = 1024;      /// Max restore buffer length

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
 * @brief service functions that must be provided for storing and restoring tree structure
 * @param data the data to read/write
 * @param level number of tabs to put before each line (just for aesthetics)
 * @param in/out filestreams to read/write
 */
bool  gTree_storeData  (GTREE_TYPE  data, size_t level, FILE *out);
bool  gTree_restoreData(GTREE_TYPE *data, FILE *in);                 
bool  gTree_printData  (GTREE_TYPE  data, FILE *out);

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
    gTree_status_BadData,
    gTree_status_BadRestoration,
    gTree_status_FileErr,
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
    "Error during data restoration",
    "Error during tree restoration",
    "Error in file IO",
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


/**
 * @brief Macro for easier and more secure node access in gObjPool
 */
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


/**
 * @brief adds sibling after the last existing one
 * @param tree pointer to structure
 * @param siblingId id of a node to add sibling to
 * @param id ptr to write new nodeId to
 * @param data data to write to new node
 * @return gTree status code
 */
gTree_status gTree_addSibling(gTree *tree, size_t siblingId, size_t *id, GTREE_TYPE data)
{
    GTREE_ASSERT_LOG(gPtrValid(tree), gTree_status_BadStructPtr, stderr);

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


/**
 * @brief adds child to node after the last existing one
 * @param tree pointer to structure
 * @param nodeId id of a node to add child to
 * @param id ptr to write new childId to
 * @param data data to write to new node
 * @return gTree status code
 */
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

    *id = childId;
    
    return gTree_status_OK;
}


/**
 * @brief deletes child of a node with the position pos
 * @param tree pointer to structure
 * @param parentId id of a node to delete child in
 * @param pos position of a child (starting with 0)
 * @param data data ptr to write to write poped data (could be NULL, then data discarded)
 * @return gTree status code
 */
gTree_status gTree_delChild(gTree *tree, size_t parentId, size_t pos, GTREE_TYPE *data)
{
    GTREE_ASSERT_LOG(gPtrValid(tree), gTree_status_BadStructPtr,  stderr);

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
    GTREE_ASSERT_LOG(gPtrValid(fout), gTree_status_BadDumpOutPtr, tree->logStream);
    
    fprintf(fout, "digraph dilist {\n\tnode [shape=record]\n\tsubgraph cluster {\n");
    
    for (size_t i = 0; i < tree->pool.capacity; ++i) {
        gTree_Node *node = &tree->pool.data[i].val;
        fprintf(fout, "\t\tnode%lu [label=\"Node %lu | | {data | ", i, i);
        if ((&tree->pool.data[i])->allocated) 
            gTree_printData(node->data, fout);
        fprintf(fout, "}\"]\n");
    }

    fprintf(fout, "\t}\n");
    
    for (size_t i = 0; i < tree->pool.capacity; ++i) {
        gTree_Node *node = &tree->pool.data[i].val;
        if ((&tree->pool.data[i])->allocated && (node->parent != -1)) {
            fprintf(fout, "\tnode%lu -> node%lu\n", node->parent, i);
            if (node->sibling != -1)
                fprintf(fout, "\tnode%lu -> node%lu [style=dotted]\n", i, node->sibling);
        }
    }

    fprintf(fout, "}\n");
    return gTree_status_OK;
}


/**
 * @brief stores subTree in a file in human-readable format
 * @param tree pointer to structure
 * @param nodeId id of a node to start storing from
 * @param level the number of tabulation to add before each output line
 * @param out filestream to write to
 * @return gTree status code
 */
gTree_status gTree_storeSubTree(const gTree *tree, size_t nodeId, size_t level, FILE *out) 
{
    GTREE_ASSERT_LOG(gPtrValid(tree), gTree_status_BadStructPtr,  stderr);
    GTREE_ASSERT_LOG(gPtrValid(out),  gTree_status_BadDumpOutPtr, tree->logStream);

    gTree_Node *node = GTREE_NODE_BY_ID(tree, nodeId);
    gTree_status status = gTree_status_OK;

    for (size_t i = 0; i < level; ++i)
        fprintf(out, "\t");
    fprintf(out, "{\n");
    for (size_t i = 0; i < level + 1; ++i)
        fprintf(out, "\t");
    fprintf(out, "[\n");
    gTree_storeData(node->data, level + 2, out);
    for (size_t i = 0; i < level + 1; ++i)
        fprintf(out, "\t");
    fprintf(out, "]\n");
 
    size_t childId = node->child;
    while (childId != -1) {
        status = gTree_storeSubTree(tree, childId, level + 1, out);
        GTREE_ASSERT_LOG(status == gTree_status_OK, status, tree->logStream);
        childId = GTREE_NODE_BY_ID(tree, childId)->sibling;
    }

    for (size_t i = 0; i < level; ++i)
        fprintf(out, "\t");
    fprintf(out, "}\n");

    return gTree_status_OK;
}


/**
 * @brief checks if the haystack consists only needle and some spaces
 * @param haystack the null-terminating string to check
 * @param needle   the null-terminating string to search for in haystack
 * @return true if consists only needle and surrounding spaces, false otherwize
 */
bool consistsOnly(const char * const haystack, const char * const needle)
{
    /* WARINING: needle and haystack must be null-terminating */

    char *haystackIter = (char*)haystack;
    char *needleIter   = (char*)needle;
    while (isspace(*(haystackIter)))
        ++haystackIter;
    while (*needleIter != '\0' && *haystackIter != '\0') {
        if (*needleIter != *haystackIter)
            return false;
        ++needleIter;
        ++haystackIter;
    }
    if (*needleIter != '\0')
        return false;
    if (*haystackIter == '\0')
        return true;

    while (isspace(*(haystackIter))) 
        ++haystackIter;
    if (*haystackIter == '\0')
        return true;
    else
        return false;
}


/**
 * @brief restores subTree from a file in human-readable format
 * @param tree pointer to structure
 * @param nodeId id of a node to start restoring from
 * @param in filestream to read from
 * @return gTree status code
 */
gTree_status gTree_restoreSubTree(gTree *tree, size_t nodeId, FILE *in)
{
    /*
     * WARNING: Tree restoration (below) just somewhat supports 
     *          empty lines and doesn't support nor checks lines 
     *          that don't follow the format.
     */

    GTREE_ASSERT_LOG(gPtrValid(tree), gTree_status_BadStructPtr,  stderr);
    GTREE_ASSERT_LOG(gPtrValid(in),   gTree_status_FileErr,       tree->logStream);

    gTree_Node *node = GTREE_NODE_BY_ID(tree, nodeId);
    gTree_status status = gTree_status_OK;
    
    char buffer[MAX_BUFFER_LEN] = "";
    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "Restoring subTree from node %lu\n", nodeId);
    #endif

    size_t  curChildId = -1;
    size_t prevChildId = -1;
    GTREE_TYPE dummyData = {};
    int bracketCnt = 1;
    while ((bracketCnt > 0) && !feof(in)) {          
        GTREE_ASSERT_LOG(getline(buffer, MAX_BUFFER_LEN, in) == 0, gTree_status_FileErr, tree->logStream);
        if (consistsOnly(buffer, "{")) {
            ++bracketCnt;
            prevChildId = curChildId;
            status = gTree_addChild(tree, nodeId, &curChildId, dummyData);
            GTREE_ASSERT_LOG(status == gTree_status_OK, status, tree->logStream);
            gTree_Node *curChild = GTREE_NODE_BY_ID(tree, curChildId);
            curChild->parent = nodeId;
            curChild->child  = -1;

            status = gTree_restoreSubTree(tree, curChildId, in);
            --bracketCnt;
            GTREE_ASSERT_LOG(status == gTree_status_OK, status, tree->logStream);

            if (prevChildId != -1)
                GTREE_NODE_BY_ID(tree, prevChildId)->sibling = curChildId;
            else 
                GTREE_NODE_BY_ID(tree, nodeId)->child = curChildId;
        } else if (consistsOnly(buffer, "}")) {
            --bracketCnt;
        } else if (consistsOnly(buffer, "[")) {
            GTREE_ASSERT_LOG(gTree_restoreData(&node->data, in) == 0, gTree_status_BadData, tree->logStream);
        }
    }

    #ifdef EXTRA_VERBOSE
        fprintf(stderr, "End of restoring subTree from node %lu\n", nodeId);
    #endif

    GTREE_ASSERT_LOG(bracketCnt == 0, gTree_status_BadRestoration, tree->logStream);
    return gTree_status_OK;
}


/**
 * @brief constructs tree and restores it from a file in human-readable format
 * @param tree pointer to structure
 * @param newLogStream logStream to forward to constructor
 * @param in filestream to read from
 * @return gTree status code
 */
gTree_status gTree_restoreTree(gTree *tree, FILE *newLogStream, FILE *in)       
{
    /* WARNING: tree structure must be uninitialized */

    gTree_ctor(tree, newLogStream);
    char buffer[MAX_BUFFER_LEN] = "";
    GTREE_ASSERT_LOG(getline(buffer, MAX_BUFFER_LEN, in) == 0, gTree_status_FileErr, tree->logStream);
    gTree_status status = gTree_status_OK;

    if (consistsOnly(buffer, "{")) {
        status = gTree_restoreSubTree(tree, tree->root, in);
        GTREE_ASSERT_LOG(status == gTree_status_OK, status, tree->logStream);
    }
    
    return gTree_status_OK;
}
