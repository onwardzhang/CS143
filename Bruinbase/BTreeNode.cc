#include <cstdlib>
#include "BTreeNode.h"
#include "RecordFile.h"

using namespace std;

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile &pf) {
  return pf.read(pid, buffer);
}

/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile &pf) {
  return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount() {
  int keyCount = 0;
  memcpy(&keyCount, buffer, sizeof(int));
  return keyCount;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId &rid)
{
  int keyCount = getKeyCount();
  if (keyCount >= LEAF_MAX_KEY_COUNT) {
    return RC_NODE_FULL;
  }
  int eid;
  locate(key, eid);
  insertHelper(eid, key, rid);
  return 0;
}

void BTLeafNode::insertHelper(int eid, int key, const RecordId &rid) {
  int pos = PRESERVED_SPACE + eid * LEAF_ENTRY_SIZE;
  char *shift = buffer + pos;
  int keyCount = getKeyCount();
  size_t size = (size_t) ((keyCount - eid) * LEAF_ENTRY_SIZE); //maybe do not need +1, but no side effect
  char *tmp = (char *) malloc(size);
  memcpy(tmp, shift, size);
  memcpy(shift, &rid, sizeof(RecordId));
  shift += sizeof(RecordId);
  memcpy(shift, &key, sizeof(int));
  shift += sizeof(int);
  memcpy(shift, tmp, size);
  free(tmp);
  keyCount++;
  memcpy(buffer, &keyCount, sizeof(int));
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId &rid,
                              BTLeafNode &sibling, int &siblingKey) {
  if (sibling.getKeyCount() != 0) {
    return RC_INVALID_ATTRIBUTE;
  }

  int splitPos = PRESERVED_SPACE + LEAF_ENTRY_SIZE * LEAF_MAX_KEY_COUNT / 2;
  int keyCount = LEAF_MAX_KEY_COUNT / 2;
  sibling.changeBuffer(PRESERVED_SPACE, buffer + splitPos, (size_t) (LEAF_ENTRY_SIZE * LEAF_MAX_KEY_COUNT / 2));
  sibling.changeBuffer(0, &keyCount, sizeof(int));
  memset(buffer + splitPos, 0, (size_t) (LEAF_ENTRY_SIZE * LEAF_MAX_KEY_COUNT / 2));
  memcpy(buffer, &keyCount, sizeof(int));
  RecordId firstRid;
  sibling.readEntry(0, siblingKey, firstRid);
  if (key >= siblingKey) {
    sibling.insert(key, rid);
  } else {
    insert(key, rid);
  }
  return 0;
}

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the index entry
 * immediately after the largest index key that is smaller than searchKey,
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int &eid) //todo: check
{
  int keyCount = getKeyCount();
  int low = 0;//initial with 0
  int high = keyCount - 1;//initial with keyCount - 1
  while (low < high - 1) {
    int mid = low + (high - low) / 2;
    int midKey = entryIDToKey(mid);
    if (midKey == searchKey) {
      eid = mid;
      return 0;
    }
    if (midKey < searchKey) {
      low = mid;
    } else {
      high = mid;
    }
  }
  if (entryIDToKey(low) == searchKey) {
    eid = low;
    return 0;
  }
  if (entryIDToKey(high) == searchKey) {
    eid = high;
    return 0;
  }
  if (entryIDToKey(low) > searchKey) {
    eid = low;
    return RC_NO_SUCH_RECORD;
  }
  if (entryIDToKey(high) > searchKey) {
    eid = high;
    return RC_NO_SUCH_RECORD;
  }
  eid = high + 1;
  return RC_NO_SUCH_RECORD;
}

int BTLeafNode::entryIDToKey(int id) {
  int pos = PRESERVED_SPACE + id * LEAF_ENTRY_SIZE + sizeof(RecordId);
  int key;
  memcpy(&key, buffer + pos, sizeof(int));
  return key;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int &key, RecordId &rid) {
  if (getKeyCount() == 0) {
    return RC_INVALID_CURSOR;
  }
  if (eid >= getKeyCount() || eid < 0) {// ==
    return RC_INVALID_CURSOR;
  }
  int pos = PRESERVED_SPACE + eid * LEAF_ENTRY_SIZE;
  memcpy(&rid, buffer + pos, sizeof(RecordId));
  memcpy(&key, buffer + pos + sizeof(RecordId), sizeof(int));
  return 0;
}

/*
 * Return the pid of the next sibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr() //the pid of the next sibling node is stored in the preserved space
{
  PageId siblingPid;
  memcpy(&siblingPid, buffer + sizeof(int), sizeof(PageId));
  return siblingPid;
}

/*
 * Set the pid of the next sibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid) {
  memcpy(buffer + sizeof(int), &pid, sizeof(PageId));
  return 0;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile &pf) {
  return pf.read(pid, buffer);
}

/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile &pf) {
  return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount() {
  int keyCount = 0;
  memcpy(&keyCount, buffer, sizeof(int));
  return keyCount;
}

/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid) {
  int keyCount = getKeyCount();
  if (keyCount >= NONLEAF_MAX_KEY_COUNT) {
    return RC_NODE_FULL;
  }
  int eid;
  locate(key, eid);
  insertHelper(eid, key, pid);
  return 0;
}

void BTNonLeafNode::insertHelper(int eid, int key, const PageId &pid) {
  int pos = PRESERVED_SPACE + eid * NONLEAF_ENTRY_SIZE;
  int keyCount = getKeyCount();
  if (eid >= keyCount) {//==
    memcpy(buffer + pos, &key, sizeof(int));
    memcpy(buffer + pos + sizeof(int), &pid, sizeof(PageId));
  } else {
    char *shift = buffer + pos;
    size_t size = (size_t) ((keyCount - eid) * NONLEAF_ENTRY_SIZE);
    char *tmp = (char *) malloc(size);
    memcpy(tmp, shift, size);
    memcpy(shift, &key, sizeof(int));
    shift += sizeof(int);
    memcpy(shift, &pid, sizeof(PageId));
    shift += sizeof(PageId);
    memcpy(shift, tmp, size);
    free(tmp);
  }
  keyCount++;
  memcpy(buffer, &keyCount, sizeof(int));
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode &sibling, int &midKey) {
  if (sibling.getKeyCount() != 0) {
    return RC_INVALID_ATTRIBUTE;
  }

  int splitPos = PRESERVED_SPACE + NONLEAF_ENTRY_SIZE * ((NONLEAF_MAX_KEY_COUNT + 1) / 2);//old node remains 64 entries
  int keyCount = NONLEAF_MAX_KEY_COUNT / 2;//63

  sibling.changeBuffer(PRESERVED_SPACE, buffer + splitPos, (size_t) (NONLEAF_ENTRY_SIZE * NONLEAF_MAX_KEY_COUNT / 2));
  sibling.changeBuffer(0, &keyCount, sizeof(int));
  memset(buffer + splitPos, 0, (size_t) (NONLEAF_ENTRY_SIZE * NONLEAF_MAX_KEY_COUNT / 2));
  keyCount++;//64
  memcpy(buffer, &keyCount, sizeof(int));

  int oldLastKey;
  PageId oldLastPid;
  int newFirstKey;
  PageId newFirstPid;
  readEntry(NONLEAF_MAX_KEY_COUNT / 2, oldLastKey, oldLastPid);
  sibling.readEntry(0, newFirstKey, newFirstPid);
  if (key > oldLastKey && key < newFirstKey) { // if the key to be inserted is the midKey, then return it rather than insert
    //return the key to be inserted as the midKey and insert the pid to insert to the preserved pid in sibling node
    midKey = key;
    sibling.changeBuffer(sizeof(int), &pid, sizeof(PageId));
    return 0;
  } else if (key > oldLastKey) {
    sibling.insert(key, pid);//sibling keyCount = 63 + 1; old keyCount = 64
  } else {
    insert(key, pid);//sibling keyCount = 63; old keyCount = 64 + 1
  }
  //insert the oldLastPid to the preserved pid in sibling node， >= oldLastKey <=> < newFirstKey
  sibling.changeBuffer(sizeof(int), &oldLastPid, sizeof(PageId));
  midKey = oldLastKey;
  //delete oldLastKey and its pid
  memset(buffer + splitPos - NONLEAF_ENTRY_SIZE, 0,
         (size_t) NONLEAF_ENTRY_SIZE);
  //reduce the keyCount of old node
  memcpy(&keyCount, buffer, sizeof(int));//64 /65
  keyCount--;
  memcpy(buffer, &keyCount, sizeof(int));
  return 0;
}

RC BTNonLeafNode::locate(int searchKey, int &eid) { //used for insert
  int keyCount = getKeyCount();
  int low = 0;//initial with 0
  int high = keyCount - 1;//initial with keyCount - 1
  while (low < high - 1) {
    int mid = low + (high - low) / 2;
    int midKey = entryIDToKey(mid);
    if (midKey == searchKey) {
      eid = mid; // assumption no duplicates! otherwise h = mid, continue loop
      return 0;
    }
    if (midKey < searchKey) {
      low = mid;
    } else {
      high = mid;
    }
  }
  if (entryIDToKey(low) == searchKey) {
    eid = low;
    return 0;
  }
  if (entryIDToKey(high) == searchKey) {
    eid = high;
    return 0;
  }
  if (entryIDToKey(low) > searchKey) {
    eid = low;
    return RC_NO_SUCH_RECORD;
  }
  if (entryIDToKey(high) > searchKey) {
    eid = high;
    return RC_NO_SUCH_RECORD;
  }
  eid = high + 1;
  return RC_NO_SUCH_RECORD;
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId &pid)
{
  int keyCount = getKeyCount();
  int low = 0;
  int high = keyCount - 1;
  int resultKey;
  int eid;
  while (low < high - 1) { //find the first key which is bigger than searchKey
    int mid = low + (high - low) / 2;
    int midKey = entryIDToKey(mid);
    if (midKey == searchKey) {
      eid = mid;// assumption no duplicates! otherwise h = mid, continue loop
      readEntry(eid, resultKey, pid);
      return 0;
    }
    if (midKey < searchKey) {
      low = mid;
    } else {
      high = mid;
    }
  }

  if (entryIDToKey(high) == searchKey) {
    eid = high;
    readEntry(eid, resultKey, pid);
    return 0;
  }
  if (entryIDToKey(low) == searchKey) {
    eid = low;
    readEntry(eid, resultKey, pid);
    return 0;
  }
  if (entryIDToKey(high) < searchKey) {
    eid = high;
    readEntry(eid, resultKey, pid);
    return RC_NO_SUCH_RECORD;
  }

  if (entryIDToKey(low) < searchKey) {
    eid = low;
    readEntry(eid, resultKey, pid);
    return RC_NO_SUCH_RECORD;
  }

  eid = low - 1;
  if (eid < 0) {
    memcpy(&pid, buffer + sizeof(int), sizeof(PageId));
    return RC_NO_SUCH_RECORD;
  } else {
    readEntry(eid, resultKey, pid);
    return RC_NO_SUCH_RECORD;
  }
}

int BTNonLeafNode::entryIDToKey(int id) {
  int pos = PRESERVED_SPACE + id * NONLEAF_ENTRY_SIZE;
  int key;
  memcpy(&key, buffer + pos, sizeof(int));
  return key;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2) {
  char *pos = buffer + sizeof(int);
  memcpy(pos, &pid1, sizeof(PageId));
  pos += sizeof(PageId);
  memcpy(pos, &key, sizeof(int));
  pos += sizeof(int);
  memcpy(pos, &pid2, sizeof(PageId));
  int keyCount = 1;
  memcpy(buffer, &keyCount, sizeof(int));
  return 0;
}

RC BTNonLeafNode::readEntry(int eid, int &key, PageId &pid) {
  if (eid >= getKeyCount() || eid < 0) { // ==
    return RC_INVALID_CURSOR;
  }
  int pos = PRESERVED_SPACE + eid * NONLEAF_ENTRY_SIZE;
  memcpy(&key, buffer + pos, sizeof(int));
  memcpy(&pid, buffer + pos + sizeof(int), sizeof(PageId));
  return 0;
}
