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
  
  // check if any of the attribute names are too long, or if the attr is
  // a string, check if its length is too long
  for (int i = 0; i < attrCnt; i++) {
	    if (sizeof(attrList[i].attrName) > MAXNAME) return NAMETOOLONG;
		if (attrList[i].attrType == STRING){
			if (attrList[i].attrLen > MAXSTRINGLEN) return ATTRTOOLONG;
		}
  }
  
  
  // make sure that a relation with the same name doesn't already exist
  if ((status = relCat->getInfo(relation, rd)) == OK) return RELEXISTS;
  
  
  // fill in the RelDesc and add to relCat. Similar to dbcreate.C
  strcpy(rd.relName, relation.c_str());
  rd.attrCnt = 0; // every attribute added will update this
  if ((status = relCat->addInfo(rd)) != OK) return status;
  
  // fill the AttrDesc with attributes like in dbcreate.C
  for (int i = 0; i < attrCnt; i++) {
	  
	    // if the attribute is already in the relation, destroy the relation and
		// return the duplicate error
		if ((status = attrCat->getInfo(relation, attrList[i].attrName, ad)) == OK){ 
			relCat->destroyRel(relation);
			return DUPLATTR;
		}
		
		// add the attribute data to ad
        strcpy(ad.attrName, attrList[i].attrName);
        strcpy(ad.relName, attrList[i].relName);
        ad.attrLen = attrList[i].attrLen;
        ad.attrType = attrList[i].attrType;
		if (i == 0) {
           ad.attrOffset = i;
		} else {
		   ad.attrOffset += attrList[i-1].attrLen;
		}
		
		
          if ((status = attrCat->addInfo(ad)) != OK) return status;
		  
		  
		  // Must add the attribute to relCat	
		  // Start by getting the record to update
		  if ((status = relCat->getInfo(relation, rd)) != OK) return status;


		  // Remove the record from the relCat table
		  if ((status = relCat->removeInfo(relation)) != OK) return status;
							
					
		  // Increase the attribute count because we are adding 1 attribute
		  rd.attrCnt += 1;


		  // Add back to the table -- by deleting and then adding we
		  // have updated the record
		  if ((status = relCat->addInfo(rd)) != OK) return status;
    }
  
    // create a HeapFile instance to hold tuples of the relation
    status = createHeapFile(relation);
    return status;
    




}

