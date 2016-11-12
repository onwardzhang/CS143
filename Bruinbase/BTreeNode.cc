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
RC BTLeafNode::read(PageId pid, const PageFile& pf)
{
    return pf.read(pid, buffer);
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf)
{
    return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount()
{
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
RC BTLeafNode::insert(int key, const RecordId& rid)// todo:why use const RecordId&? reference. because too big?
{
    int keyCount = getKeyCount();
    if (keyCount >= LEAF_MAX_KEY_COUNT) {
        return RC_NODE_FULL;
    }
//    leafEntry newEntry;
//    newEntry.key = key;
//    newEntry.rid = rid;
    //int offset = PRESERVED_SPACE + LEAF_ENTRY_SIZE  * keyCount;
    int eid;
    locate(key, eid);
    insertHelper(eid, key, rid);
    return 0;
}

//int BTLeafNode:: searchPosition (int key) {
//    int keyCount = getKeyCount();
////    int low = PRESERVED_SPACE;
////    int high = PRESERVED_SPACE + keyCount * ENTRY_SIZE;
//    int low = 0;
//    int high = keyCount;
//    while (low < high - 1) {
//        int mid = low + (high - low) / 2;
//        int pos = PRESERVED_SPACE + mid * ENTRY_SIZE + sizeof(RecordId);
//        int midKey = posToKey(pos);
//        if (midKey == key) {
//            return pos;
//        }
//        if (midKey < key) {
//            low = mid;
//        } else {
//            high = mid;
//        }
//    }
//    int lowPos = PRESERVED_SPACE + low * ENTRY_SIZE + sizeof(RecordId);
//    int highPos = PRESERVED_SPACE + high * ENTRY_SIZE + sizeof(RecordId);
//    if (posToKey(lowPos) == key) {
//        return lowPos;
//    }
//    if (posToKey(highPos) == key) {
//        return highPos;
//    }
//    return lowPos;
//    // /not found,If not, set eid to the index entry
////    * immediately after the largest index key that is smaller than searchKey,
////    * and return the error code RC_NO_SUCH_RECORD.
//}

void BTLeafNode::insertHelper (int eid, int key, const RecordId& rid) {
    int pos = PRESERVED_SPACE + eid * LEAF_ENTRY_SIZE ;
    char * shift = buffer + pos;
    int keyCount = getKeyCount();
    size_t size = (size_t) ((keyCount - eid + 1) * LEAF_ENTRY_SIZE ); //todo: check
    char * tmp = (char*) malloc(size);//todo: check
    memcpy(tmp, shift, size);//todo: memmove vs memcpy, memcpy cannot overlap， but faster
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
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid, 
                              BTLeafNode& sibling, int& siblingKey)
{
    if(sibling.getKeyCount() != 0) {
        return RC_INVALID_ATTRIBUTE;//todo:return value?
    }

    int splitPos = PRESERVED_SPACE + LEAF_ENTRY_SIZE  * LEAF_MAX_KEY_COUNT / 2;//todo: 边界值，是否改取上下限，其实84为偶数所以当前情况不影响
    int keyCount = LEAF_MAX_KEY_COUNT / 2;
    // no way to copy value to sibling easily, add a new function
    //memcpy(sibling.buffer, buffer + splitPos, LEAF_ENTRY_SIZE  * LEAF_MAX_KEY_COUNT / 2);//buffer为private，应该不能这么访问
    sibling.changeBuffer(PRESERVED_SPACE, buffer + splitPos, (size_t) (LEAF_ENTRY_SIZE  * LEAF_MAX_KEY_COUNT / 2));
    sibling.changeBuffer(0, &keyCount, sizeof(int));
    memset(buffer + splitPos, 0, (size_t) (LEAF_ENTRY_SIZE  * LEAF_MAX_KEY_COUNT / 2));
    memcpy(buffer, &keyCount, sizeof(int));
    //insert
//    int eid;
//    locate(key, eid);
//    if(eid >= LEAF_MAX_KEY_COUNT / 2){
//        sibling.insert(key, rid);
//    } else {
//        insert(key, rid);
//    }
    RecordId firstRid;
    sibling.readEntry(0, siblingKey, firstRid);

    if(key >= siblingKey) {
        sibling.insert(key, rid);
    } else {
        insert(key, rid);
    }
    //todo: 是否需要在此处修改next sibling 的 pid，如何知道sibling的pid?
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
RC BTLeafNode::locate(int searchKey, int& eid) //todo: check
{
    int keyCount = getKeyCount();
//    int low = PRESERVED_SPACE;
//    int high = PRESERVED_SPACE + keyCount * LEAF_ENTRY_SIZE ;
    int low = 0;//todo: check, initial with 0 or 1
    int high = keyCount - 1;//todo: check, initial with keyCount or keyCount - 1
    while (low < high - 1) {
        int mid = low + (high - low) / 2;
        //int pos = PRESERVED_SPACE + mid * LEAF_ENTRY_SIZE  + sizeof(RecordId);
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
//    int lowPos = PRESERVED_SPACE + low * LEAF_ENTRY_SIZE  + sizeof(RecordId);
//    int highPos = PRESERVED_SPACE + high * LEAF_ENTRY_SIZE  + sizeof(RecordId);
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

int BTLeafNode::entryIDToKey (int id) {
    int pos = PRESERVED_SPACE + id * LEAF_ENTRY_SIZE  + sizeof(RecordId);
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
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid)
{
    if (eid >= getKeyCount() || eid < 0) {//todo: check, >= or >
        return RC_INVALID_CURSOR;
    }
    int pos = PRESERVED_SPACE + eid * LEAF_ENTRY_SIZE ;
    memcpy(&rid, buffer + pos, sizeof(RecordId));
    memcpy(&key, buffer + pos + sizeof(RecordId), sizeof(int));
    return 0;
}

/*
 * Return the pid of the next sibling node.
 * @return the PageId of the next sibling node 
 */
PageId BTLeafNode::getNextNodePtr() //the pid of the next sibling node is stored in the preserved space
{   PageId siblingPid;
    memcpy(&siblingPid, buffer + sizeof(int), sizeof(PageId));
    return siblingPid;
}

/*
 * Set the pid of the next sibling node.
 * @param pid[IN] the PageId of the next sibling node 
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid) // todo:how to get this pid
{
    memcpy(buffer + sizeof(int), &pid, sizeof(PageId));
    return 0;//todo: in what case we should return an error code?
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf)
{
    return pf.read(pid, buffer);
}
    
/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf)
{
    return pf.write(pid, buffer);
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount()
{
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
RC BTNonLeafNode::insert(int key, PageId pid)
{
    int keyCount = getKeyCount();
    if (keyCount >= NONLEAF_MAX_KEY_COUNT) {
        return RC_NODE_FULL;
    }
//    RecordId rid;
//    rid.pid = pid;
//    rid.sid = 0;
    int eid;
    PageId insertPosPid;
    locateChildPtr(key, insertPosPid, eid);
    //insertHelper(eid, key, rid);
    insertHelper(eid, key, pid);
    return 0;
}

/*
void BTNonLeafNode::insertHelper (int eid, int key, const RecordId& rid) {
    int pos = PRESERVED_SPACE + eid * ENTRY_SIZE;
    char * shift = buffer + pos;
    int keyCount = getKeyCount();
    size_t size = (size_t) ((keyCount - eid + 1) * ENTRY_SIZE); //todo: check
    char * tmp = (char*) malloc(size);//todo: check
    memcpy(tmp, shift, size);//todo: memmove vs memcpy, memcpy cannot overlap， but faster
    memcpy(shift, &rid, sizeof(RecordId));
    shift += sizeof(RecordId);
    memcpy(shift, &key, sizeof(int));
    shift += sizeof(int);
    memcpy(shift, tmp, size);
    free(tmp);
    keyCount++;
    memcpy(buffer, &keyCount, sizeof(int));
}
*/

void BTNonLeafNode::insertHelper (int eid, int key, const PageId& pid) {
    int pos = PRESERVED_SPACE + eid * NONLEAF_ENTRY_SIZE;
    char * shift = buffer + pos;
    int keyCount = getKeyCount();
    size_t size = (size_t) ((keyCount - eid + 1) * NONLEAF_ENTRY_SIZE); //todo: check
    char * tmp = (char*) malloc(size);//todo: check
    memcpy(tmp, shift, size);//todo: memmove vs memcpy, memcpy cannot overlap， but faster
    memcpy(shift, &pid, sizeof(PageId));
    shift += sizeof(PageId);
    memcpy(shift, &key, sizeof(int));
    shift += sizeof(int);
    memcpy(shift, tmp, size);
    free(tmp);
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
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{
    if(sibling.getKeyCount() != 0) {
        return RC_INVALID_ATTRIBUTE;//todo:return value?
    }

    int splitPos = PRESERVED_SPACE + NONLEAF_ENTRY_SIZE  * ((NONLEAF_MAX_KEY_COUNT + 1) / 2);//todo:取上限，127+1/2 =64,原node保留64个
    int keyCount = NONLEAF_MAX_KEY_COUNT / 2;//63

    sibling.changeBuffer(PRESERVED_SPACE, buffer + splitPos, (size_t) (NONLEAF_ENTRY_SIZE  * NONLEAF_MAX_KEY_COUNT / 2));
    sibling.changeBuffer(0, &keyCount, sizeof(int));
    memset(buffer + splitPos, 0, (size_t) (NONLEAF_ENTRY_SIZE  * NONLEAF_MAX_KEY_COUNT / 2));
    keyCount++;
    memcpy(buffer, &keyCount, sizeof(int));

    int oldLastKey;
    PageId  oldLastPid;
    int newFirstKey;
    PageId newFirstPid;
    readEntry(NONLEAF_MAX_KEY_COUNT / 2, oldLastKey, oldLastPid);
    sibling.readEntry(0, newFirstKey, newFirstPid);
    if (key > oldLastKey && key < newFirstKey) {
        memcpy(buffer + splitPos, &pid, sizeof(PageId));//todo: 是否应该删除中间点？ 目前没删
        memcpy(buffer + splitPos + sizeof(PageId), &key, sizeof(int));
        keyCount++;
        memcpy(buffer, &keyCount, sizeof(int));

        midKey = key;
        return 0;
    } else if (key > oldLastKey){
        sibling.insert(key, pid);
    } else {
        insert(key, pid);
    }
    midKey = oldLastKey;//todo: 是否应该删除中间点？ 目前没删
    return 0;
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid, int& eid)
{
    //(pid, key) pid为left pointer,指向小于key的child, 所以要想找到searchKey所在位置应该返回right pointer(>= key),即下一个key的pid
    int keyCount = getKeyCount();
    int low = 0;
    int high = keyCount - 1;//todo
    int resultKey;
    while (low < high - 1) { //find the first key which is bigger than searchKey
        int mid = low + (high - low) / 2;
        int midKey = entryIDToKey(mid);
        if (midKey == searchKey) {
            eid = mid + 1;
            readEntry(eid, resultKey, pid);//find the first key which is bigger than searchKey
            return 0;
        }
        if (midKey < searchKey) {
            low = mid;
        } else {
            high = mid;
        }
    }
    if (entryIDToKey(low) == searchKey) {
        eid = low + 1;
        readEntry(eid, resultKey, pid);
        return 0;
    }
    if (entryIDToKey(high) == searchKey) {
        eid = high + 1;
        if (eid > getKeyCount()) {//avoid eid exceeds keyCount, which will cause readEntry statement return error code
            memcpy(&pid, buffer + sizeof(int), sizeof(PageId)); // todo: assume the last right pointer is stored in the preserved space
            return 0; //todo: but how to maintain the last right pointer? what is the last right pointer??
            // todo: 初始化root时,1 key+2 pointers保证有last right pointer
        } else {
            readEntry(eid, resultKey, pid);
            return 0;
        }
    }
    if (entryIDToKey(low) > searchKey) {
        eid = low;
        readEntry(eid, resultKey, pid);
        return RC_NO_SUCH_RECORD;
    }
    if (entryIDToKey(high) > searchKey) {
        eid = high;
        readEntry(eid, resultKey, pid);
        return RC_NO_SUCH_RECORD;
    }
    eid = high + 1;
    if (eid > getKeyCount()) { //avoid eid exceeds keyCount, which will cause readEntry statement return error code
        memcpy(&pid, buffer + sizeof(int), sizeof(PageId));
        return RC_NO_SUCH_RECORD;
    } else {
        readEntry(eid, resultKey, pid);
        return RC_NO_SUCH_RECORD;
    }
}

int BTNonLeafNode::entryIDToKey (int id) {
    int pos = PRESERVED_SPACE + id * NONLEAF_ENTRY_SIZE + sizeof(PageId);
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
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{
//    RecordId rid1, rid2;
//    rid1.pid = pid1;
//    rid1.sid = 0;
//    rid2.pid = pid2;
//    rid2.sid = 0;
    char * pos = buffer + PRESERVED_SPACE;
    memcpy(pos, &pid1, sizeof(PageId));
    pos += sizeof(PageId);
    memcpy(pos, &key, sizeof(int));
    pos += sizeof(int);
    memcpy(pos, &pid2, sizeof(PageId));//todo:这个pid是不是应该放在preserved Space中呢？
    int keyCount = 1;
    memcpy(buffer, &keyCount, sizeof(int));
    return 0;//todo: when to return error?
}

RC BTNonLeafNode::readEntry(int eid, int& key, PageId& pid)
{
    if (eid >= getKeyCount() || eid < 0) { //todo
        return RC_INVALID_CURSOR;
    }
    int pos = PRESERVED_SPACE + eid * NONLEAF_ENTRY_SIZE;
    memcpy(&pid, buffer + pos, sizeof(PageId));
    memcpy(&key, buffer + pos + sizeof(PageId), sizeof(int));
    return 0;
}
