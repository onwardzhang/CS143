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
BTreeIndex::BTreeIndex() {
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
RC BTreeIndex::open(const string &indexname, char mode) {
  RC rc = pf.open(indexname, mode);
  if (rc < 0) {  //open failed
    return rc;
  }
  if (mode == 'r') { //read
    char buffer[PageFile::PAGE_SIZE];
    /////////////////todo
    memset(buffer, 0 , PageFile::PAGE_SIZE);
    rc = pf.read(0, buffer);
    if (rc < 0) {
      return rc;
    }
    memcpy(&rootPid, buffer, sizeof(PageId));
    memcpy(&treeHeight, buffer + sizeof(PageId), sizeof(int));
  } else { //write
    if (pf.endPid() == 0) { //create a new index file, write the initial info to first page
      char buffer[PageFile::PAGE_SIZE];
      /////////////////todo
      memset(buffer, 0 , PageFile::PAGE_SIZE);
      memcpy(buffer, &rootPid, sizeof(PageId));//-1
      memcpy(buffer + sizeof(PageId), &treeHeight, sizeof(int));//0
      rc = pf.write(0, buffer);
      if (rc < 0) {
        return rc;
      }
    } else { // the index file already exists, read the information about the tree, same as mode 'r'
      char buffer[PageFile::PAGE_SIZE];
      /////////////////todo
      memset(buffer, 0 , PageFile::PAGE_SIZE);
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
RC BTreeIndex::close() {
  //before close, write the updated treeHeight and rootPid to the disk
  RC rc;
  char buffer[PageFile::PAGE_SIZE];
  /////////////////todo
  memset(buffer, 0 , PageFile::PAGE_SIZE);
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

RC BTreeIndex::insert(int key, const RecordId &rid) {
  RC rc;
  // tree is not empty, call insertHelper
  if (treeHeight != 0) {
    PageId siblingPid;
    int newRootKey = insertHelper(key, rid, 1, rootPid, siblingPid);
    //if insertHelper return a nonZero value, should create a new root!
    if (newRootKey != 0) {
      //create new root
      BTNonLeafNode *newRoot = new BTNonLeafNode();
      newRoot->initializeRoot(rootPid, newRootKey, siblingPid);
      rootPid = pf.endPid();
      rc = newRoot->write(rootPid, pf);
      if (rc < 0) {
        return rc;
      }
      //update tree info
      treeHeight++;
      char buffer[PageFile::PAGE_SIZE];
      /////////////////todo
      memset(buffer, 0 , PageFile::PAGE_SIZE);
      memcpy(buffer, &rootPid, sizeof(PageId));
      memcpy(buffer + sizeof(PageId), &treeHeight, sizeof(int));
      rc = pf.write(0, buffer);
      if (rc < 0) {
        return rc;
      }
      return 0;
    }
  } else {// empty tree, create root
    BTLeafNode *root = new BTLeafNode();
    rootPid = pf.endPid();
    rc = root->insert(key, rid);
    if (rc < 0) {
      return rc;
    }
    rc = root->write(rootPid, pf);
    if (rc < 0) {
      return rc;
    }
    treeHeight++;
    char buffer[PageFile::PAGE_SIZE];
    /////////////////todo
    memset(buffer, 0 , PageFile::PAGE_SIZE);
    memcpy(buffer, &rootPid, sizeof(PageId));
    memcpy(buffer + sizeof(PageId), &treeHeight, sizeof(int));
    rc = pf.write(0, buffer);
    if (rc < 0) {
      return rc;
    }
  }
  return 0;
}

int BTreeIndex::insertHelper(int key, const RecordId &rid, int curtLevel, PageId curtPid, PageId &siblingPid) {
  RC rc;
  if (curtLevel == treeHeight) { //leaf node
    BTLeafNode *leaf = new BTLeafNode();
    rc = leaf->read(curtPid, pf);
    if (rc < 0) {
      return rc;
    }
    if (leaf->insert(key, rid) != 0) { //leaf node overflow
      BTLeafNode *sibling = new BTLeafNode();
      siblingPid = pf.endPid();
      int siblingKey;
      leaf->insertAndSplit(key, rid, *sibling, siblingKey);
      sibling->setNextNodePtr(leaf->getNextNodePtr());
      leaf->setNextNodePtr(siblingPid);
      rc = leaf->write(curtPid, pf);
      if (rc < 0) {
        return rc;
      }
      rc = sibling->write(siblingPid, pf);
      if (rc < 0) {
        return rc;
      }
      return siblingKey; // return the first sibling key to insert in the parent node
    } else { // leaf node not overflow
      rc = leaf->write(curtPid, pf);
      if (rc < 0) {
        return rc;
      }
      return 0;
    }
  } else { //non-leaf node
    BTNonLeafNode *nonLeaf = new BTNonLeafNode();
    rc = nonLeaf->read(curtPid, pf);
    if (rc < 0) {
      return rc;
    }
    PageId childPid;
    nonLeaf->locateChildPtr(key, childPid);
    //newKey stands for the siblingKey/midKey returned after leafNode/nonLeafNode split
    int newKey = insertHelper(key, rid, curtLevel + 1, childPid, siblingPid);
    if (newKey != 0) { //if has new thing to insert
      if (nonLeaf->insert(newKey, siblingPid) != 0) { //nonLeaf overflow
        BTNonLeafNode *nonLeafSibling = new BTNonLeafNode();
        PageId nonLeafSiblingPid = pf.endPid();
        int midKey;
        nonLeaf->insertAndSplit(newKey, siblingPid, *nonLeafSibling, midKey);
        rc = nonLeaf->write(curtPid, pf);
        if (rc < 0) {
          return rc;
        }
        rc = nonLeafSibling->write(nonLeafSiblingPid, pf);
        if (rc < 0) {
          return rc;
        }
        siblingPid = nonLeafSiblingPid;
        return midKey;
      } else { //nonLeaf not overflow
        rc = nonLeaf->write(curtPid, pf);
        if (rc < 0) {
          return rc;
        }
        return 0;
      }
    } else { // has nothing to insert, return 0;
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
RC BTreeIndex::locate(int searchKey, IndexCursor &cursor) {
  RC rc;
  if (treeHeight == 0) {
    cursor.eid = 0;
    cursor.pid = 1;
    return RC_NO_SUCH_RECORD;
  }
  int curtLevel = 1;
  BTNonLeafNode *tempNode = new BTNonLeafNode();
  PageId tempPid = rootPid;
  while (curtLevel++ < treeHeight) {
    rc = tempNode->read(tempPid, pf);
    if (rc < 0) {
      return rc;
    }
    tempNode->locateChildPtr(searchKey, tempPid);
  }
  cursor.pid = tempPid;
  BTLeafNode *targetNode = new BTLeafNode();
  rc = targetNode->read(tempPid, pf);
  if (rc < 0) {
    return rc;
  }
  rc = targetNode->locate(searchKey, cursor.eid);
  if (rc != 0) {
    return rc;
  }
  delete tempNode;
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
RC BTreeIndex::readForward(IndexCursor &cursor, int &key, RecordId &rid) {
  // first to judge if we have already read to the end, if pid = 0, means that the nextPtr of last page is 0, which is end.
  if (cursor.pid == 0) {
    return RC_END_OF_TREE;
  }

  RC rc;
  BTLeafNode *targetNode = new BTLeafNode();

  rc = targetNode->read(cursor.pid, pf);
  if (rc < 0) {
    return rc;
  }

  rc = targetNode->readEntry(cursor.eid, key, rid);
  if (rc < 0) {
    return rc;
  }

  if (cursor.eid == targetNode->getKeyCount() - 1) { // have read the last entry in the node, then read next page
    cursor.pid = targetNode->getNextNodePtr();
    cursor.eid = 0;
  } else {
    cursor.pid = cursor.pid;
    cursor.eid++;
  }
  delete targetNode;
  return 0;
}
