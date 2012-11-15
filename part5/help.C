#include <sys/types.h>
#include <functional>
#include <string.h>
#include <stdio.h>
using namespace std;

#include "error.h"
#include "utility.h"
#include "catalog.h"

// define if debug output wanted


//
// Retrieves and prints information from the catalogs about the for the
// user. If no relation is given (relation is NULL), then it lists all
// the relations in the database, along with the width in bytes of the
// relation, the number of attributes in the relation, and the number of
// attributes that are indexed.  If a relation is given, then it lists
// all of the attributes of the relation, as well as its type, length,
// and offset, whether it's indexed or not, and its index number.
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::help(const string & relation)
{
  Status status;
  RelDesc rd;
  AttrDesc *attrs;
  int attrCnt;

  if (relation.empty()) return UT_Print(RELCATNAME);
  
  // Get meta data of the relation
  if ((status = relCat->getInfo(relation, rd)) != OK) return status;
  // Get all attributes of the relation
  if ((status = attrCat->getRelInfo(relation, rd.attrCnt, attrs)) != OK){
	  delete[] attrs;
	  return status;
  }
  cout << "| " << "relName " << "attrName " << "offset " << "type " << "length " << "|" << endl;
  for (int i = 0; i < rd.attrCnt; i++){
	  cout << attrs[i].relName << " | " << attrs[i].attrName << " | " << attrs[i].attrOffset << " | " << attrs[i].attrType << " | " << attrs[i].attrLen << endl;
	  
  }
  delete[] attrs;


  return OK;
}
