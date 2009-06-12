#include <stdlib.h>
#include <string.h>
#include <util.h>
#include <scalar_config.h>
#include <enkf_util.h>
#include <enkf_macros.h>
#include <trans_func.h>
#include <active_list.h>

#define SCALAR_CONFIG_TYPE_ID 877065

struct scalar_config_struct {
  int                __type_id;
  int 		     data_size;            
  int 		     internal_offset;
  double 	   * mean;
  double 	   * std;
  active_list_type * active_list;
  transform_ftype ** output_transform;
  char            ** output_transform_name;
  arg_pack_type   ** arg_pack;        
};




scalar_config_type * scalar_config_alloc_empty(int size) {
  scalar_config_type *scalar_config    = util_malloc(sizeof *scalar_config, __func__);
  scalar_config->__type_id             = SCALAR_CONFIG_TYPE_ID;
  scalar_config->data_size   	       = size;
  scalar_config->mean        	       = util_malloc(size * sizeof *scalar_config->mean        , __func__);
  scalar_config->std         	       = util_malloc(size * sizeof *scalar_config->std         ,  __func__);
  scalar_config->active_list           = active_list_alloc( ALL_ACTIVE );
  scalar_config->output_transform      = util_malloc(scalar_config->data_size * sizeof * scalar_config->output_transform      , __func__);
  scalar_config->output_transform_name = util_malloc(scalar_config->data_size * sizeof * scalar_config->output_transform_name , __func__);
  scalar_config->internal_offset       = 0;
  scalar_config->arg_pack              = util_malloc(scalar_config->data_size * sizeof * scalar_config->arg_pack , __func__);
  
  {
    int i;
    for (i=0; i < size; i++) {
      scalar_config->output_transform_name[i] = NULL;
      scalar_config->arg_pack[i]              = NULL;
      scalar_config->std[i]                   = 1.0;
      scalar_config->mean[i]                  = 0.0;
    }
  }
  return scalar_config;
}


const double * scalar_config_get_std(const scalar_config_type * config) {
  return config->std;
}


const double * scalar_config_get_mean(const scalar_config_type * config) {
  return config->mean;
}



void scalar_config_transform(const scalar_config_type * config , const double * input_data , double *output_data) {
  int index;
  for (index = 0; index < config->data_size; index++) {

    if (config->output_transform[index] == NULL)
      output_data[index] = input_data[index];
    else
      output_data[index] = config->output_transform[index](input_data[index] , config->arg_pack[index]);
  }
}

double scalar_config_transform_item(const scalar_config_type * config, double input_data, int item)
{
  if(item < 0 || config->data_size <= item)
  {
    util_abort("%s: scalar_config type has size %i, trying to access item %i (0 offset). Index out of bounds - aborting.\n",
               __func__,config->data_size,item);
  }

  if(config->output_transform[item] == NULL)
    return input_data;
  else
    return config->output_transform[item](input_data,config->arg_pack[item]);
}


void scalar_config_truncate(const scalar_config_type * config , double *data) {
  return;
}




void scalar_config_fscanf_line(scalar_config_type * config , int line_nr , FILE * stream) {
  config->output_transform[line_nr] = trans_func_lookup(stream , &config->output_transform_name[line_nr] , &config->arg_pack[line_nr]);
}





void scalar_config_free(scalar_config_type * scalar_config) {
  int i;
  free(scalar_config->mean);
  free(scalar_config->std);
  active_list_free(scalar_config->active_list);
  util_free_stringlist(scalar_config->output_transform_name , scalar_config->data_size);
  for (i=0; i < scalar_config->data_size; i++)
    if (scalar_config->arg_pack[i] != NULL) arg_pack_free(scalar_config->arg_pack[i]);

  free(scalar_config->arg_pack);
  free(scalar_config->output_transform);
  free(scalar_config);
}



/*****************************************************************/

SAFE_CAST(scalar_config , SCALAR_CONFIG_TYPE_ID)
GET_DATA_SIZE(scalar);
GET_ACTIVE_LIST(scalar);
VOID_FREE(scalar_config);
