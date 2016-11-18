/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
    treeHeight = 0;
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
    RC rc = pf.open(indexname, mode);
    if (rc < 0) {  //open failed
        return rc;
    }
    if (mode == 'r') { //read
        char buffer[PageFile::PAGE_SIZE];
        rc = pf.read(0, buffer);
        if (rc < 0) {
            return rc;
        }
        memcpy(&rootPid, buffer, sizeof(PageId));
        memcpy(&treeHeight, buffer + sizeof(PageId), sizeof(int));
//        int* intBuffPtr = (int*) buffer; // todo: 查这个用法
//        rootPid =  intBuffPtr[0];
//        treeHeight = intBuffPtr[1];
    } else { //write
        if (pf.endPid() == 0) { //create a new index file, write the initial info to first page
            char buffer[PageFile::PAGE_SIZE];
            memcpy(buffer, &rootPid, sizeof(PageId));
            memcpy(buffer + sizeof(PageId), &treeHeight, sizeof(int));
            rc  = pf.write(0, buffer);
            if (rc < 0) {
                return rc;
            }
        } else { // the index file already exists, read the information about the tree, same as mode 'r'
            char buffer[PageFile::PAGE_SIZE];
            rc = pf.read(0, buffer);
            if (rc < 0) {
                return rc;
            }
            memcpy(&rootPid, buffer, sizeof(PageId));
            memcpy(&treeHeight, buffer + sizeof(PageId), sizeof(int));
        }
    }
    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    //before close, write the updated treeHeight and rootPid to the disk
    RC rc;
    char buffer[PageFile::PAGE_SIZE];
    memcpy(buffer, &rootPid, sizeof(PageId));
    memcpy(buffer + sizeof(PageId), &treeHeight, sizeof(int));
    rc = pf.write(0, buffer);
    if (rc < 0) {
        return rc;
    }
    rc = pf.close();
    if (rc < 0) {
        return rc;
    }
    return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */

RC BTreeIndex::insert(int key, const RecordId& rid)
{
    // tree is not empty, call insertHelper
    if (treeHeight != 0) {
        PageId siblingPid;
        insertHelper(key, rid, 1, rootPid, siblingPid);
    }
    // empty tree, create root
    else {
        BTNonLeafNode *root = new BTNonLeafNode();
        BTLeafNode *left = new BTLeafNode();
        BTLeafNode *right = new BTLeafNode();
        rootPid = pf.endPid();// always 1? -- 对于新建的tree, 设置root的pid为1，左右叶节点的pid为2、3， 不断insert后，root会变
        right->insert(key, rid);//put the key & rid in the right child
        root->initializeRoot(rootPid + 1, key, rootPid + 2);
        left->setNextNodePtr(rootPid + 2);//remember to set the sibling pid
        root->write(rootPid, pf);
        left->write(rootPid + 1, pf); //2?
        right->write(rootPid + 2, pf);//3?
        treeHeight += 2; //remember to update the treeHeight
    }
    return 0;
}
//nonLeafNode 的 insertAndSplit() 应该删除midkey对应的entry
//siblingKey本身表示leaf node split后的 siblingKey，但也应该代表nonLeafNode split后的midKey, 所以统称为newKey
//siblingPid 表示overflow后新建的node的pid
int BTreeIndex:: insertHelper(int key, const RecordId& rid, int curtLevel, PageId curtPid, PageId &siblingPid) {
    if (curtLevel == treeHeight) { //leaf node
        BTLeafNode *leaf = new BTLeafNode();
        leaf->read(curtPid, pf);
        if (leaf->insert(key, rid) != 0) { //leaf node overflow
            BTLeafNode *sibling = new BTLeafNode();
            siblingPid = pf.endPid();
            int siblingKey;
            leaf->insertAndSplit(key, rid, *sibling, siblingKey);
            sibling->setNextNodePtr(leaf->getNextNodePtr());
            leaf->setNextNodePtr(siblingPid);
            leaf->write(curtPid, pf);
            sibling->write(siblingPid, pf);

            return siblingKey; // return the first sibling key to insert in the parent node
        }
        else { // leaf node not overflow
            leaf->write(curtPid, pf);
            return 0;
        }
    }
    else { //non-leaf node
        BTNonLeafNode *nonLeaf = new BTNonLeafNode();
        nonLeaf->read(curtPid, pf);
        PageId childPid;
        int uselessEid;
        nonLeaf->locateChildPtr(key, childPid, uselessEid);
        int newKey = insertHelper(key, rid, curtLevel + 1, childPid, siblingPid);
        if (newKey != 0) { //if has new thing to insert
            if (nonLeaf->insert(newKey, siblingPid) != 0) { //nonLeaf OR root overflow
                if (curtLevel == 1) { // root overflow
                    //create new root
                    BTNonLeafNode *newRoot = new BTNonLeafNode();
                    newRoot->initializeRoot(curtPid, newKey, siblingPid);
                    //update tree info
                    treeHeight++;
                    rootPid = pf.endPid();
                    newRoot->write(rootPid, pf);
                    //RC rc; // todo: use rc to collect possible error code
                    char buffer[PageFile::PAGE_SIZE];
                    //bzero(tmp_buffer, PageFile::PAGE_SIZE);//buffer是否需要初始化
                    pf.read(0, buffer);
                    memcpy(buffer, &rootPid, sizeof(PageId));
                    memcpy(buffer + sizeof(PageId), &treeHeight, sizeof(int));
                    pf.write(0, buffer);
                    return 0; // return 0?
                    }
                else { //nonLeaf overflow
                    BTNonLeafNode *nonLeafSibling = new BTNonLeafNode();
                    PageId nonLeafSiblingPid = pf.endPid();
                    int midKey;
                    nonLeaf->insertAndSplit(newKey, siblingPid, *nonLeafSibling, midKey);
                    nonLeaf->write(curtPid, pf);
                    nonLeafSibling->write(nonLeafSiblingPid, pf);
                    siblingPid = nonLeafSiblingPid;
                    return midKey; // return
                }
            }
            else { //nonLeaf OR root not overflow
                nonLeaf->write(curtPid, pf);
                return 0;
            }
        }
        else { // has nothing to insert, return 0;
            return 0;
        }
    }
}

/**
 * Run the standard B+Tree key search algorithm and identify the
 * leaf node where searchKey may exist. If an index entry with
 * searchKey exists in the leaf node, set IndexCursor to its location
 * (i.e., IndexCursor.pid = PageId of the leaf node, and
 * IndexCursor.eid = the searchKey index entry number.) and return 0.
 * If not, set IndexCursor.pid = PageId of the leaf node and
 * IndexCursor.eid = the index entry immediately after the largest
 * index key that is smaller than searchKey, and return the error
 * code RC_NO_SUCH_RECORD.
 * Using the returned "IndexCursor", you will have to call readForward()
 * to retrieve the actual (key, rid) pair from the index.
 * @param key[IN] the key to find
 * @param cursor[OUT] the cursor pointing to the index entry with
 *                    searchKey or immediately behind the largest key
 *                    smaller than searchKey.
 * @return 0 if searchKey is found. Othewise an error code
 */
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
    if (treeHeight == 0) { // <= 1 ? non-empty tree at least has treeHeight of 2.
        cursor.eid = 0;
        cursor.pid = 1;
        return RC_NO_SUCH_RECORD;
    }
    int curtLevel = 1;
    BTNonLeafNode *tempNode = new BTNonLeafNode();
    PageId tempPid = rootPid;
    int tempEid;
    while (curtLevel++ < treeHeight) {
        tempNode->read(tempPid, pf);
        tempNode->locateChildPtr(searchKey, tempPid, tempEid);
    }
    cursor.pid = tempPid;
    BTLeafNode *targetNode = new BTLeafNode();
    targetNode->read(tempPid, pf);
    RC rc;
    rc = targetNode->locate(searchKey, cursor.eid);
    if (rc != 0) {
        return rc;
    }
    delete tempNode;//todo: necessary? others?
    delete targetNode;
    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move forward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
    BTLeafNode *targetNode = new BTLeafNode();
    targetNode->read(cursor.pid, pf);
    targetNode->readEntry(cursor.eid, key, rid);
    if (cursor.eid < targetNode->getKeyCount()) {
        PageId nextPid = targetNode->getNextNodePtr();
        if (nextPid == 0) { //todo: check
            return RC_END_OF_TREE;
        } else {
            cursor.pid = nextPid;
            cursor.eid = 0;
        }
    } else {
        cursor.pid = cursor.pid;
        cursor.eid++;
    }
    delete targetNode; //todo: necessary?
    return 0;
}
