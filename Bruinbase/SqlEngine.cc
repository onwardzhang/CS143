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
#include "RecordFile.h"
#include <limits.h>

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE *sqlin;

int sqlparse(void);


RC SqlEngine::run(FILE *commandline) {
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
  // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

bool checkValueConds(const vector<SelCond> &valueConds, string value) {
  for (int j = 0; j < valueConds.size(); j++) {
    int diff = strcmp(value.c_str(), valueConds[j].value);
    switch (valueConds[j].comp) {
      case SelCond::EQ:
        if (diff != 0) return false;
        break;
      case SelCond::NE:
        if (diff == 0) return false;
        break;
      case SelCond::GT:
        if (diff <= 0) return false;
        break;
      case SelCond::LT:
        if (diff >= 0) return false;
        break;
      case SelCond::GE:
        if (diff < 0) return false;
        break;
      case SelCond::LE:
        if (diff > 0) return false;
        break;
    }
  }
  return true;
}

RC SqlEngine::select(int attr, const string &table, const vector<SelCond> &cond) {
  RecordFile rf;   // RecordFile containing the table
  RecordId rid;  // record cursor for table scanning

  RC rc;
  int key;
  string value;
  int count = 0;

  // open the table file, constant page read 1
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  //fprintf(stdout, "open table fine\n");
  vector<int> NEKeysV;
  vector<SelCond> valueConds;

  // compute range
  int lowerBound = INT_MIN;
  int upperBound = INT_MAX;

  for (unsigned i = 0; i < cond.size(); i++) {
    if (cond[i].attr == 2) {
      valueConds.push_back(cond[i]);
      continue;
    }
    //lowerBound upperBound can be reached
    switch (cond[i].comp) {
      case SelCond::EQ:
        if (atoi(cond[i].value) >= lowerBound && atoi(cond[i].value) <= upperBound) { //EQ value in the range
          upperBound = atoi(cond[i].value);
          lowerBound = upperBound;
        } else {
          return RC_NO_SUCH_RECORD;
        }
        break;
      case SelCond::NE:
        if (atoi(cond[i].value) >= lowerBound && atoi(cond[i].value) <= upperBound) {
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

  if (lowerBound > upperBound) { //safe check, may be redundant
    return RC_NO_SUCH_RECORD;
  }

//  fprintf(stdout, "lowerBound: %d\n", lowerBound);
//  fprintf(stdout, "upperBound: %d\n", upperBound);

//if need to use BTIndex: key has a range or select on key or count(*)
  bool useIndex = lowerBound > INT_MIN || upperBound < INT_MAX || attr == 4 || attr == 1;
  //todo: if attr == 4 , maybe we don't need to read every tuple to count, we can just use the keyCount of every leafNode to do this.
  //useIndex = false;
  if (useIndex) {
    //fprintf(stdout, "use index\n");
    BTreeIndex indexFile; // indexFile containing the B+ tree index
    // open the index file, constant page read 2
    if ((rc = indexFile.open(table + ".idx", 'r')) < 0) { //todo:如果不存在indexFile，该命令是否还是会新建一个空的?
      fprintf(stderr, "Error: indexFile %s does not exist\n", table.c_str());
      return rc;
    }
    //fprintf(stdout, "open indexFile successfully\n");
    // use index to search
    IndexCursor cursor;
    indexFile.locate(lowerBound, cursor);//locate the lowerBound (the first possible key)
//      fprintf(stdout, "after locating pid: %d\n",cursor.pid);
//      fprintf(stdout, "eid: %d\n",cursor.eid);
    //int nodeCount = 1;
    while (indexFile.readForward(cursor, key, rid) == 0) {//get key and rid
      if (key > upperBound) {
        //fprintf(stdout, "key > upperBound, break");
        break;
      }
      // check all NE condition for key
      int i = 0;
      for (; i < NEKeysV.size(); i++) {
        if (key == NEKeysV[i]) {
          fprintf(stdout, "key = NEKeysV, break");
          break;
        }
      }
      if (i != NEKeysV.size()) {
        fprintf(stdout, "i != NEKeysV.size(), break");
        continue;
      }

//if only select key or count(*) and no condition on value then there is no need to read tuples from table
      if ((attr == 1 || attr == 4) && valueConds.size() == 0) {
        //fprintf(stdout, "easy search\n");
        if (attr == 1) {
          //fprintf(stdout, "&&&&&&&&&&&&&&   key: %d   &&&&&&&&&&&&&&&&&&&&\n", key);
          fprintf(stdout, "%d\n", key);
        }
        count++;
        //fprintf(stdout, "*****************easy search have read %d tuples*******************\n",count);
      } else {
        // read the tuple
        if (rf.read(rid, key, value) < 0) {
          fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
          goto exit_select;
        }

        //check all value condition
        if (!checkValueConds(valueConds, value)) {
          continue;
        }
        //meet all conditions, then count++, print result
        count++;
        //fprintf(stdout, "\n*****************have read %d tuples*******************\n",count);
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
      //fprintf(stdout, "final count = %d\n", count);
    }
    rc = indexFile.close();
    if (rc < 0) {
      return rc;
    }

  } else { //not use index， use original scanning method
    //fprintf(stdout, "not use index!\n");
    int diff;
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
    // close the table file and return
    exit_select:
    rc = rf.close();
    if (rc < 0) {
      return rc;
    }
    return rc;
  }
}

RC SqlEngine::load(const string &table, const string &loadfile, bool index) {
  RC rc;
  RecordFile rf(table + ".tbl", 'w');

  ifstream fin;
  fin.open(loadfile.c_str());

  if (!fin.is_open()) {
    return RC_FILE_OPEN_FAILED;
  }

  if (index) {
    //fprintf(stdout, "begin loading\n");
    BTreeIndex indexFile;
    if (indexFile.open(table + ".idx", 'w') != 0) {
      return RC_FILE_OPEN_FAILED;
    }
    //int count = 0;
    for (string line; getline(fin, line);) {
      int key;
      string value;
      RecordId rid;
      parseLoadLine(line, key, value);
      rc = rf.append(key, value, rid);
      if (rc < 0) {
        return rc;
      }
      rc = indexFile.insert(key, rid);
      if (rc < 0) {
        return rc;
      }
      //count++;
      //fprintf(stdout, "\n*****************have inserted %d tuples*******************\n",count);
    }
    indexFile.close();
    rf.close();
  } else {
    for (string line; getline(fin, line);) {
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

RC SqlEngine::parseLoadLine(const string &line, int &key, string &value) {
  const char *s;
  char c;
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
