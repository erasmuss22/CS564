#include "catalog.h"

//
// Destroys a relation. It performs the following steps:
//
// 	removes the catalog entry for the relation
// 	destroys the heap file containing the tuples in the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status RelCatalog::destroyRel(const string & relation)
{
  Status status;
  RelDesc rd;
  AttrDesc* ad;
  
  if (relation.empty() || 
      relation == string(RELCATNAME) || 
      relation == string(ATTRCATNAME))
    return BADCATPARM;
	
  // Get meta data of the relation and store in rd
  if ((status = relCat->getInfo(relation, rd)) != OK) return status;
  
  
  // Get all attributes of the relation
  if ((status = attrCat->getRelInfo(relation, rd.attrCnt, ad)) != OK){
	  delete[] ad;
	  return status;
  }
  
  
  // Delete all attributes of the relation. Since the relation is
  // getting destroyed, there is no need to update the attrCnt of
  // relcat for this relation
  for (int i = 0; i < rd.attrCnt; i++) {
        status = attrCat->removeInfo(relation, ad[i].attrName);
        if (status != OK) {
            delete[] ad;
            return status;
        }
  }
  
  
  // getRelInfo doesn't deallocate this, so do it here
  delete[] ad;
  
  
  // remove the relation
  if ((status = relCat->removeInfo(relation)) != OK) return status;
  
  
  // destroy the HeapFile
  status = destroyHeapFile(relation);
  return status;


}


//
// Drops a relation. It performs the following steps:
//
// 	removes the catalog entries for the relation
//
// Returns:
// 	OK on success
// 	error code otherwise
//

const Status AttrCatalog::dropRelation(const string & relation)
{
  Status status;
  AttrDesc *ad;
  RelDesc rd;
  int attrCnt, i;

  if (relation.empty()) return BADCATPARM;


  // Get meta data of the relation and store in rd
  if ((status = relCat->getInfo(relation, rd)) != OK) return status;


  // Get all attributes of the relation
  if ((status = attrCat->getRelInfo(relation, rd.attrCnt, ad)) != OK){
	  delete[] ad;
	  return status;
  }


  // Delete all attributes of the relation
  for (int i = 0; i < rd.attrCnt; i++) {
        status = attrCat->removeInfo(relation, ad[i].attrName);
        if (status != OK) {
            delete[] ad;
            return status;
        }
  }
  
  
  // getRelInfo doesn't deallocate this
  delete[] ad;
  
  
  // Update the meta data in relcat for this relation
  // Remove the record from the relCat table
  if ((status = relCat->removeInfo(relation)) != OK) return status;
			
			
  // Set the attribute count to 0 because we are deleting all attributes
  // from that relation
  rd.attrCnt = 0;
			
			
  // Add back to the table -- by deleting and then adding we
  // have updated the record
  if ((status = relCat->addInfo(rd)) != OK) return status;
  return OK;

}


