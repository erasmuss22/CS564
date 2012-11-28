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
// part 6
	Status status;
	RID rid;
	const char *filter;
	AttrDesc ad;
	
	HeapFileScan* hfs = new HeapFileScan(relation, status);
    if(status != OK) {
		delete hfs;
		return status;
	}
	
	if (attrValue != NULL){
		
		// Get and store the AttrDesc for the relName and attrName of the attr
		if((status = attrCat->getInfo(relation, attrName, ad)) != OK) { 
			return status;
		}
		
		// get the attribute type and store it as a filter
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
				
		    default:
			    filter = attrValue;
			    break;
	    }
		
	}
	

	if ((status = hfs->startScan(ad.attrOffset, ad.attrLen, type, filter, op)) != OK){
		delete hfs;
		return status;
	}
	
	while((status = hfs->scanNext(rid)) == OK){
		
		if((status = hfs->deleteRecord()) != OK) {
			delete hfs;
			return status;
		}
		
		
	}

	if (status != FILEEOF) { 
		delete hfs;
		return status;
	}
	delete hfs;
	return OK;



}


