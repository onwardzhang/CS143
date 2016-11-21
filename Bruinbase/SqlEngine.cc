/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"
//#include <unordered_set>
//#include <string>
#include <limits.h>

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  RC     rc;
  int    key;
  string value;
  int    count = 0;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  /* *************************************************************************** */

  //前提是cond里只有关于key的限制，对value呢？
  //how about create a vector<SelCond> to copy undone cond. or a vector records the index of undone cond.
//  vector<int> undoNEIndex; //存了cond[i].comp == NE
//  vector<int> undoValueIndex;//存了cond[i].attr == 2

  //todo: check undo condition can be wrapped,  and just use vector<SelCond> and the original check statements
  //unordered_set<int> NEKeys;
  vector<int> NEKeysV;
  //unordered_set<string> NEValues; //todo: <char*> or <string>?  <char*>貌似很危险。。。
  vector<string> NEValuesV;
  //unordered_set<string> EQValue; // todo: EQValue actually doesn't need a set,  just a string is enough
  vector<string> EQValueV;
  //if (cond.size() > 1) {//e.g. WHERE KEY > 100 AND KEY >= 102 AND KEY < 1000 AND KEY <= 998 AND KEY = 100
    // compute range
    int lowerBound = INT_MIN;
    int upperBound = INT_MAX;

    for (unsigned i = 0; i < cond.size(); i++) {
      if (cond[i].attr == 2) { // 如果condition是关于value的，跳过？
        //undoValueIndex.push_back(i);
        switch (cond[i].comp) {
          case SelCond::EQ: {
            string strEQ(cond[i].value);
            //todo: check!! cond[i].value 的type是char*
            //EQValue.insert(strEQ);
            EQValueV.push_back(strEQ);//用vector就可能有重复元素了
            //if (EQValue.size() > 1) { // cannot have two different EQ value
//            if (EQValueV.size() > 1) { // cannot have two different EQ value
//              return RC_NO_SUCH_RECORD;
//            }
            break;
          }
          case SelCond::NE: {
            string strNE(cond[i].value);//todo: check!!
            //NEValues.insert(strNE);
            NEValuesV.push_back(strNE);
            break;
          }
          default:
            return RC_NO_SUCH_RECORD;//todo: if other comparator for value, then return RC_NO_SUCH_RECORD
        }
        continue;
      }
      //todo: lowerBound upperBound是 >,< 还是>= <=好？， 应该是取等好,已改为取等！
      switch (cond[i].comp) {
        case SelCond::EQ:
          if (atoi(cond[i].value) > lowerBound && atoi(cond[i].value) < upperBound) { //EQ value 在当前范围内
            upperBound = atoi(cond[i].value);
            lowerBound = upperBound;
          } else {
            return RC_NO_SUCH_RECORD;
          }
          break;
        case SelCond::NE:
          if (atoi(cond[i].value) >= lowerBound && atoi(cond[i].value) <= upperBound) {
            //undoNEIndex.push_back(i);
           // NEKeys.insert(atoi(cond[i].value));
            NEKeysV.push_back(atoi(cond[i].value));
          }
          break;
        case SelCond::GT:
          lowerBound = lowerBound > (atoi(cond[i].value) + 1) ? lowerBound : (atoi(cond[i].value) + 1);
          break;
        case SelCond::GE:
          lowerBound = lowerBound > atoi(cond[i].value) ? lowerBound : atoi(cond[i].value);
          break;
        case SelCond::LT:
          upperBound = upperBound < (atoi(cond[i].value) - 1) ? upperBound : (atoi(cond[i].value) - 1);
          break;
        case SelCond::LE:
          upperBound = upperBound < atoi(cond[i].value) ? upperBound : atoi(cond[i].value);
          break;
      }
      if (lowerBound > upperBound) {
        return RC_NO_SUCH_RECORD;
      }
    }

    if (lowerBound > upperBound) { //应该可以不必冗余check
      return RC_NO_SUCH_RECORD;
    }

/*
  RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
 * If an index entry with
    * searchKey exists in the leaf node, set IndexCursor to its location
    * (i.e., IndexCursor.pid = PageId of the leaf node, and
    * IndexCursor.eid = the searchKey index entry number.) and return 0.
    * If not, set IndexCursor.pid = PageId of the leaf node and
    * IndexCursor.eid = the index entry immediately after the largest
    * index key that is smaller than searchKey, and return the error
    * code RC_NO_SUCH_RECORD.
           * Using the returned "IndexCursor", you will have to call readForward()
    * to retrieve the actual (key, rid) pair from the index.*/

/*    RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
    Read the (key, rid) pair at the location specified by the index cursor,
    * and move forward the cursor to the next entry.*/

//    vector<int> undoNEIndex; //存了cond[i].comp == NE
//    vector<int> undoValueIndex;//存了cond[i].attr == 2
//todo: deal with count!
    //todo : deal with attr == 4(count(*))


/*  todo: 判断是否要使用index
 * if(min_key > -1 || max_key < INT_MAX || attr == 1 || attr == 4)
      use_index = true;*/
    bool useIndex = lowerBound > INT_MIN || upperBound < INT_MAX;
    if (useIndex) {
      BTreeIndex indexFile; // indexFile containing the B+ tree index
      // open the index file
      if ((rc = indexFile.open(table + ".idx", 'r')) < 0) { //todo:如果不存在indexFile，该命令是否还是会新建一个空的?
        fprintf(stderr, "Error: indexFile %s does not exist\n", table.c_str());
        return rc;
      }
      // use index to search
      IndexCursor cursor;
      if (lowerBound == upperBound) { //single tuple, key = lowerBound

        // check all NE condition for key
//        if (NEKeys.count(lowerBound) != 0) { // if the only possible key is in the NEKeys set, then it should be abandoned
//          return RC_NO_SUCH_RECORD;
//        }
        for (int i = 0; i < NEKeysV.size(); i++) {
          if (lowerBound == NEKeysV[i]) {
            return RC_NO_SUCH_RECORD;
          }
        }
/*      for (int i = 0; i < undoNEIndex.size(); i++) {
        if (lowerBound + 1 == atoi(cond[undoNEIndex[i]].value)) { //NE value = possible key
          return RC_NO_SUCH_RECORD;
        }
      }*/

        rc = indexFile.locate(lowerBound, cursor);//locate the key
        if (rc != 0) { //not find the key
          return RC_NO_SUCH_RECORD;
        } else {         // find the key and read the tuple
          //todo: maybe can wrap to a function to print a tuple
          indexFile.readForward(cursor, key, rid);//get rid

          //if select only key and no condition on value then there is no need to read the tuple, just return the key
          //if ((attr == 1 || attr == 4) && EQValue.size() == 0 && NEValues.size() == 0) {
          if ((attr == 1 || attr == 4) && EQValueV.size() == 0 && NEValuesV.size() == 0) {
            if (attr == 1) {
              fprintf(stdout, "%d\n", key);
            } else {
              fprintf(stdout, "%d\n", 1);//count = 1
            }
          } else {
            // read the tuple (key, value)
            if ((rc = rf.read(rid, key, value)) < 0) {
              fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
              goto exit_select;
            }
            //check all value condition
//            if (EQValue.size() != 0 && EQValue.count(value) == 0) {
//              return RC_NO_SUCH_RECORD;
//            }
//            if (NEValues.size() != 0 && NEValues.count(value) == 1) {
//              return RC_NO_SUCH_RECORD;
//            }
            //if (EQValueV.size() != 0) {
              for (int i = 0; i < EQValueV.size(); i++) {
                if (value != EQValueV[i]) { //todo: check != / == for string or char*
                  return RC_NO_SUCH_RECORD;
                }
              }
            //}
            for (int i = 0; i < NEValuesV.size(); i++) {
              if (value == NEValuesV[i]) {
                return RC_NO_SUCH_RECORD;
              }
            }
/*          for (int i = 0; i < undoValueIndex.size(); i++) {
            switch (cond[undoValueIndex[i]].comp) {
              case SelCond::EQ:
                if (strcmp(value.c_str(), cond[undoValueIndex[i]].value) != 0) {
                  return RC_NO_SUCH_RECORD;
                }
                break;
              case SelCond::NE:
                if (strcmp(value.c_str(), cond[undoValueIndex[i]].value) == 0) {
                  return RC_NO_SUCH_RECORD;
                }
                break;
              default:
                return RC_NO_SUCH_RECORD;
            }
          }*/
            //print result
            switch (attr) {
              case 1:
                fprintf(stdout, "%d\n", key);
                break;
              case 2:  // SELECT value
                fprintf(stdout, "%s\n", value.c_str());
                break;
              case 3:  // SELECT *
                fprintf(stdout, "%d '%s'\n", key, value.c_str());
                break;
              case 4:
                fprintf(stdout, "%d\n", 1);//count = 1
                break;
            }
            //todo: 记得输出所有结果后退出select
          }
        }
      } else { // a range of tuples
        indexFile.locate(lowerBound, cursor);//locate the lowerBound (the first possible key)
        //key = lowerBound;
        while (indexFile.readForward(cursor, key, rid) == 0) {//get key and rid
          // readForward return RC_END_OF_TREE if reach the end of the tree;

          if (key > upperBound) {
            break;
          }
          // check all NE condition for key
//          if (NEKeys.count(key) != 0) { // if the possible key is in the NEKeys set, then it should be abandoned
//            continue;
//          }
          int i = 0;
          for (; i < NEKeysV.size(); i++) {
            if (lowerBound == NEKeysV[i]) {
              break;
            }
          }
          if (i != NEKeysV.size()) {
            continue;
          }

          //if ((attr == 1 || attr == 4) && EQValue.size() == 0 &&
              //NEValues.size() == 0) { //if only select key then there is no need to read the tuple, just return the key
          if ((attr == 1 || attr == 4) && EQValueV.size() == 0 &&
              NEValuesV.size() == 0) { //if only select key then there is no need to read the tuple, just return the key
            if (attr == 1) {
              fprintf(stdout, "%d\n", key);
            }
            count++;
          } else {
            // read the tuple
            if ((rc = rf.read(rid, key, value)) < 0) {
              fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
              goto exit_select;
            }

            //check all value condition
//            if (EQValue.size() != 0 && EQValue.count(value) == 0) { //todo: if condition 只有value限制的话是否还需要用B+ tree
//              continue;
//            }
//            if (NEValues.size() != 0 && NEValues.count(value) == 1) {
//              continue;
//            }

            //if (EQValueV.size() != 0) {
            int j = 0;
            for (; j < EQValueV.size(); j++) {
              if (value != EQValueV[j]) { //todo: check != / == for string or char*
                break;
              }
            }
            if (j != EQValueV.size()) {
              continue;
            }
            //}
            int k = 0;
            for (; k < NEValuesV.size(); k++) {
              if (value == NEValuesV[k]) {
                break;
              }
            }
            if (k != NEValuesV.size()) {
              continue;
            }

            //print result
            switch (attr) {
              case 1:
                fprintf(stdout, "%d\n", key);
                break;
              case 2:  // SELECT value
                fprintf(stdout, "%s\n", value.c_str());
                break;
              case 3:  // SELECT *
                fprintf(stdout, "%d '%s'\n", key, value.c_str());
                break;
            }
          }
        }
        if (attr == 4) {
          fprintf(stdout, "%d\n", count);
        }
      }
      indexFile.close();
    } else { //not use index， use original scanning method
      int  diff;
      // scan the table file from the beginning
      rid.pid = rid.sid = 0;
      count = 0;
      while (rid < rf.endRid()) {
        // read the tuple
        if ((rc = rf.read(rid, key, value)) < 0) {
          fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
          goto exit_select;
        }

        // check the conditions on the tuple
        for (unsigned i = 0; i < cond.size(); i++) {
          // compute the difference between the tuple value and the condition value
          switch (cond[i].attr) {
            case 1:
              diff = key - atoi(cond[i].value);
              break;
            case 2:
              diff = strcmp(value.c_str(), cond[i].value);
              break;
          }

          // skip the tuple if any condition is not met
          switch (cond[i].comp) {
            case SelCond::EQ:
              if (diff != 0) goto next_tuple;
              break;
            case SelCond::NE:
              if (diff == 0) goto next_tuple;
              break;
            case SelCond::GT:
              if (diff <= 0) goto next_tuple;
              break;
            case SelCond::LT:
              if (diff >= 0) goto next_tuple;
              break;
            case SelCond::GE:
              if (diff < 0) goto next_tuple;
              break;
            case SelCond::LE:
              if (diff > 0) goto next_tuple;
              break;
          }
        }

        // the condition is met for the tuple.
        // increase matching tuple counter
        count++;

        // print the tuple
        switch (attr) {
          case 1:  // SELECT key
            fprintf(stdout, "%d\n", key);
            break;
          case 2:  // SELECT value
            fprintf(stdout, "%s\n", value.c_str());
            break;
          case 3:  // SELECT *
            fprintf(stdout, "%d '%s'\n", key, value.c_str());
            break;
        }

        // move to the next tuple
        next_tuple:
        ++rid;
      }

      // print matching tuple count if "select count(*)"
      if (attr == 4) {
        fprintf(stdout, "%d\n", count);
      }
    }
//  } else { //single condition e.g. WHERE KEY = / <> 100 或 count(*) 或 value <> "sss", probably can be combined to the former situation
//
//  }
  /* *************************************************************************** */

  rc = 0;

  // close the table file and return
  exit_select:
  rf.close();
  return rc;
}

RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  /* your code here */
   /* For table storage, however, you must use the provided RecordFile class.
    The created Recordfile should be named as tablename + ".tbl".
    For example, when the user issues the command "LOAD movie FROM 'movieData.del'",
    you should create a RecordFile named movie.tbl (in the current working directory)
    and store all tuples in the file.
    If the file already exists, the LOAD command should append all records in the load file to the end of the table.*/
/*  Roughly, your implementation of the load function should open the input loadfile and the RecordFile,
  parse each line of the loadfile to read a tuple (possibly using SqlEngine::parseLoadLine())
  and insert the tuple to the RecordFile. */

    //append a new record at the end of the file. note that RecordFile does not have write() function.
    // append is the only way to write a record to a RecordFile.

    RecordFile rf (table + ".tbl",'w');

    ifstream fin;
    fin.open(loadfile.c_str());

    if (!fin.is_open()) {
        return RC_FILE_OPEN_FAILED;
    }

    if (index) {
        BTreeIndex indexFile;

        if (indexFile.open(table + ".idx", 'w') != 0) {
            return RC_FILE_OPEN_FAILED;
        }

        for (string line; getline(fin, line );) {
            int key;
            string value;
            RecordId rid;
            parseLoadLine(line, key, value);
            rf.append(key, value, rid);
            indexFile.insert(key, rid);
        }
        indexFile.close();
        rf.close();
    } else {
        for (string line; getline(fin, line );) {
            int key;
            string value;
            RecordId rid;
            parseLoadLine(line, key, value);
            rf.append(key, value, rid);
        }
        rf.close();
    }
    return 0;
}

RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;

    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');

    // if there is nothing left, set the value to empty string
    if (c == 0) {
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
