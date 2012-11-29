#include "catalog.h"
#include "query.h"


/*
 * Deletes records from a specified relation.
 *
 * Returns:
 * 	OK on success
 * 	an error code otherwise
 */

const Status QU_Delete(const string & relation, 
		       const string & attrName, 
		       const Operator op,
		       const Datatype type, 
		       const char *attrValue)
{
	Status status;
	RID rid;
	const char *filter;
	AttrDesc ad;
	
	
	// Open a HeapFileScan on the relation that needs the
	// record(s) removed
	
	HeapFileScan* hfs = new HeapFileScan(relation, status);
    if(status != OK) {
		delete hfs;
		return status;
	}
	
	// Check if the attrValue is NULL to determine if an unconditional
	// scan should be used
	if (attrValue != NULL){
		
		
		// Get and store the AttrDesc for the relName and attrName of the attr
		if((status = attrCat->getInfo(relation, attrName, ad)) != OK) { 
			return status;
		}
		
		// get the attribute type and store it as a filter after
		// converting to the correct type
		
		float tempf;
		int tempi;
		
		switch(ad.attrType){
			case FLOAT:
			
			    tempf = atof(attrValue);
			    filter = (char *)&tempf;
			    break;
				
		    case INTEGER:
			
			    tempi = atoi(attrValue);
			    filter = (char *)&tempi;
			    break;
				
		    case STRING:
			    filter = attrValue;
			    break;
	    }
		
	}
	
	// Start scanning to find records that match the filter and op. If the attrValue
	// is NULL, filter will be NULL and an unconditional scan is performed to delete
	// all records
	
	if ((status = hfs->startScan(ad.attrOffset, ad.attrLen, type, filter, op)) != OK){
		delete hfs;
		return status;
	}
	
	while((status = hfs->scanNext(rid)) == OK){
		
		// Find a match and delete it from the table
		if((status = hfs->deleteRecord()) != OK) {
			delete hfs;
			return status;
		}
		
	}
	
	
	// Final cleanup
	delete hfs;
	
	
	// If the EOF isn't reached, return that status
	if (status != FILEEOF) { 
		return status;
	}
	
	return OK;



}


