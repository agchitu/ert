#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <util.h>
#include <node_data.h>
#include <node_ctype.h>


/*
  Used to hold typed storage.
*/


struct node_data_struct {
  node_ctype        ctype;
  void             *data;
  int               buffer_size;  /* This is to facilitate deep copies of buffers. */
  
  /*
    For the copy constructor and delete
    operator the logic is very simple:
    
    if they are present they are used.
  */
  copyc_ftype  	 *copyc;  /* Copy constructor - can be NULL. */
  free_ftype     *del;    /* Destructor - can be NULL. */ 
};



/** 
   If the node has a copy constructor, the data is copied immediately
   - so the node contains a copy from object creation time.
*/

static node_data_type * node_data_alloc__(const void * data , node_ctype ctype , int buffer_size , copyc_ftype * copyc, free_ftype * del) {
  node_data_type * node = util_malloc(sizeof * node , __func__);
  node->ctype           = ctype;
  node->copyc           = copyc;
  node->del             = del;
  node->buffer_size     = buffer_size;  /* If buffer_size >0 copyc MUST be NULL */

  if (node->copyc != NULL)
    node->data = node->copyc( data );
  else
    node->data = (void *) data;
  
  return node;
}


node_data_type * node_data_alloc_ptr(const void * data  , copyc_ftype * copyc, free_ftype * del) {
  return node_data_alloc__( data , void_pointer , 0 , copyc , del);
}


node_data_type * node_data_alloc_buffer(const void * data, int buffer_size) {  /* The buffer is copied on insert. */
  void * data_copy = util_alloc_copy( data , buffer_size , __func__);
  return node_data_alloc__( data_copy , void_pointer , buffer_size , NULL , free);
}



/*
  When calling deep_copy the node MUST have a registered constructor,
  otherwise it will go belly up. HARD.
*/ 


static node_data_type * node_data_copyc(const node_data_type * src , bool deep_copy) {
  node_data_type * new;

  if (src->buffer_size > 0) {
    /* 
       The source node has internal storage - it has been allocated with _alloc_buffer() 
    */
    if (deep_copy) 
      new  = node_data_alloc__(util_alloc_copy( src->data , src->buffer_size , __func__)  /* A new copy is allocated prior to insert. */
			       , src->ctype , src->buffer_size , NULL , free);
    else
      new  = node_data_alloc__(src->data , src->ctype , src->buffer_size , NULL , NULL);  /* The copy does not have destructor. */
  } else {
    if (deep_copy) {
      if (src->copyc == NULL) 
	util_abort("%s: Tried allocate deep_copy of mnode with no constructor - aborting. \n",__func__);  
      new = node_data_alloc__(src->data , src->ctype , 0 , src->copyc , src->del);
    } else
      new = node_data_alloc__(src->data , src->ctype , 0 , NULL , NULL); /*shallow copy - we 'hide' constructor and destructor. */
  }

  return new;
}

node_data_type * node_data_alloc_copy(const node_data_type * node , bool deep_copy) {
  return node_data_copyc( node , deep_copy );
}


node_data_type * node_data_alloc_deep_copy(const node_data_type * node) {
  return node_data_copyc( node , true );
}


node_data_type * node_data_alloc_shallow_copy(const node_data_type * node) {
  return node_data_copyc( node , false );
}


/**
   This function does NOT call the destructor on the data. That means
   that calling scope is responsible for freeing the data; used by the
   vector_pop function.
*/
void node_data_free_container(node_data_type * node_data) {
  free(node_data);
}


void node_data_free(node_data_type * node_data) {
  if (node_data->del != NULL)
    node_data->del( (void *) node_data->data );
  
  node_data_free_container( node_data );
}


const void * node_data_get_const_ptr(const node_data_type * node_data) {
  return node_data->data;
}

void * node_data_get_ptr(const node_data_type * node_data) {
  return node_data->data;
}

/*****************************************************************/

char * node_data_get_string(const node_data_type * node_data) {
  if (node_data->ctype == void_pointer)
    return (char *) node_data->data;
  else {
    util_abort("%s: wrong type \n",__func__);
    return 0;
  }
}


node_data_type * node_data_alloc_string(const char * value) {
  void * data_copy = util_alloc_string_copy( value );
  return node_data_alloc__( data_copy , void_pointer , strlen(value + 1) , NULL , free);
}



int node_data_get_int(const node_data_type * node_data) {
  if (node_data->ctype == int_value)
    return *((int *) node_data->data);
  else {
    util_abort("%s: wrong type \n",__func__);
    return 0;
  }
}


int node_data_fetch_and_inc_int( node_data_type * node_data ) {
  if (node_data->ctype == int_value) {
    int * data = (int *) node_data->data;
    (*data) += 1;
    return *data;
  } else {
    util_abort("%s: wrong type \n",__func__);
    return 0;
  }
}


node_data_type * node_data_alloc_int(int value) {
  void * data_copy = util_alloc_copy( &value , sizeof value , __func__);
  return node_data_alloc__( data_copy , int_value , sizeof value , NULL , free);
}


double node_data_get_double(const node_data_type * node_data) {
  if (node_data->ctype == double_value)
    return *((double *) node_data->data);
  else {
    util_abort("%s: wrong type \n",__func__);
    return 0;
  }
}


node_data_type * node_data_alloc_double(double value) {
  void * data_copy = util_alloc_copy( &value , sizeof value , __func__);
  return node_data_alloc__( data_copy , double_value , sizeof value , NULL , free);
}


