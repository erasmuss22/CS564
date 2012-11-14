#include "catalog.h"


const Status RelCatalog::createRel(const string & relation, 
				   const int attrCnt,
				   const attrInfo attrList[])
{
  Status status;
  RelDesc rd;
  AttrDesc ad;

  if (relation.empty() || attrCnt < 1)
    return BADCATPARM;

  if (relation.length() >= sizeof rd.relName)
    return NAMETOOLONG;

  
  // make sure that a relation with the same name doesn't already exist
  if ((status = this->getInfo(relation, rd)) == OK) return RELEXISTS;
  
  // fill in the RelDesc and add to relCat. Similar to dbcreate.C
  strcpy(rd.relName, relation.c_str());
  rd.attrCnt = attrCnt;
  if ((status = relCat->addInfo(rd)) != OK) return status;
  
  // fill the AttrDesc with attributes like in dbcreate.C
  for (int i = 0; i < attrCnt; i++) {
        strcpy(ad.attrName, attrList[i].attrName);
        strcpy(ad.relName, attrList[i].relName);
        ad.attrLen = attrList[i].attrLen;
        ad.attrType = attrList[i].attrType;
		if (i == 0) {
           ad.attrOffset = i;
		} else {
			ad.attrOffset += sizeof attrList[i-1];
		}
        if ((status = attrCat->addInfo(ad)) != OK) break;
    }
  
    // create a HeapFile instance to hold tuples of the relation
    status = createHeapFile(relation);
    return status;
    




}

