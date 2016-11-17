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
        //todo: do we need to create a root node now?
    }
    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    //before close, write the updated treeHeight (and rootPid) to the disk
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
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    return 0;
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
    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{

    return 0;
}
