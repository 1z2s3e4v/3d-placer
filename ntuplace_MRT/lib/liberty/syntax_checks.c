/******************************************************************************
    Copyright (c) 1996-2000 Synopsys, Inc.    ALL RIGHTS RESERVED

  The contents of this file are subject to the restrictions and limitations
  set forth in the SYNOPSYS Open Source License Version 1.0  (the "License"); 
  you may not use this file except in compliance with such restrictions 
  and limitations. You may obtain instructions on how to receive a copy of 
  the License at

  http://www.synopsys.com/partners/tapin/tapinprogram.html. 

  Software distributed by Original Contributor under the License is 
  distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either 
  expressed or implied. See the License for the specific language governing 
  rights and limitations under the License.

******************************************************************************/

#include "syntax.h"
#include "si2dr_liberty.h"
#include "string.h"
#include <stdlib.h>
#include <math.h>

#include "mymalloc.h"

#include "attr_enum.h"
#include "group_enum.h"
struct libGroupMap { char *name; group_enum type; };
struct libAttrMap { char *name; attr_enum type; };

static int errcount;

static void check_group_correspondence( si2drGroupIdT g, libsynt_group_info *gi );
static void check_attr_correspondence( si2drAttrIdT attr, libsynt_attribute_info *ai );

const struct libGroupMap *
lookup_group_name (register const char *str, register unsigned int len);

const struct libAttrMap *
lookup_attr_name (register const char *str, register unsigned int len);
void create_floating_define_for_cell_area(si2drStringT string);


si2drMessageHandlerT MsgPrinter;

static char EB[8000]; /* just so I don't have to declare buffers all over the place,
                         to prepare output for output error messages */

static int isa_formula(char *str)
{
	/* a crude approximation at a formula-- the real best solution here would be to
	   have a general parser for arithmetic expressions; constant arithmetic expressions? */
	char *p = str;

	while ( *p )
	{
		if( (*p >= '0' && *p <= '9') || *p == ' ' || *p == '\t' || *p == '*' || *p == '/' || *p == '+' || *p == '-' || *p == 'e' || *p == 'E'
			|| *p == '.' )
			p++;
		else if( *p == 'V' && *(p+1) == 'D' && *(p+2) == 'D' )
			p+=3;
		else
			break;
	}
	if( *p )
		return 0;
	else
		return 1;
}

typedef struct var_list
{
		char *name;
		struct var_list *next;
		
} var_list;

static var_list *master_var_list = 0;

static void add_varlist(char *name)
{
	int found = 0;
	var_list *p;

	for(p=master_var_list; p; p=p->next)
	{
		if( !strcmp(name, p->name) )
		{
			found = 1;
			break;
		}
	}
	if( !found )
	{
		p = (var_list *)calloc(sizeof(var_list),1);
		p->name = (char*)my_malloc(strlen(name)+1);
		strcpy(p->name,name);
		p->next = master_var_list;
		master_var_list = p;
	}
}

static void destroy_varlist(void)
{
	var_list *p,*p1;

	for(p=master_var_list; p; p=p1)
	{
		p1 = p->next;
		free(p);
	}
	master_var_list = 0;
}

static void print_var_list(void)
{
	var_list *p;
	si2drErrorT err;
	
	(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_NO_ERROR, 
				  "---- var list:", 
				  &err);
	for(p=master_var_list; p; p=p->next)
	{
		(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_NO_ERROR, 
					  p->name, 
					  &err);
	}
	(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_NO_ERROR, 
				  "----", 
				  &err);
}


si2drErrorT syntax_check(si2drGroupIdT lib)
{
	extern libsynt_technology libsynt_techs[];
	extern int libsynt_tech_num;
	si2drAttrIdT attr;
	si2drGroupIdT group;
	si2drStringT  gtype;
	si2drErrorT  err;
	si2drValuesIdT vals;
	si2drValueTypeT vtype;
	si2drInt32T     intgr;
	si2drFloat64T   float64;
	si2drStringT    string;
	si2drBooleanT   bool;
	int found, first,i, dmln, tcln;
	char model[100], tech[100];
	si2drStringT tcfn;
	si2drStringT dmfn;
	si2drAttrTypeT atype;

	errcount = 0;
	
	/* one thing upon which all others hinge!

	   First: this top group must be a library.
	   
	   We'll quick scan for tech and model...
	   
	*/
	MsgPrinter = si2drPIGetMessageHandler(&err); /* the printer is in another file! */

	gtype = si2drGroupGetGroupType(lib,&err);
	if ( strcmp(gtype, "library") )
	{
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  "The top level group is not of type 'library'! This error is so foul, so heinous, so serious, \nso major, so fatal, that syntax/semantic checking is terminating!", 
					  &err);
		
		return SI2DR_SYNTAX_ERROR;
	}

	attr = si2drGroupFindAttrByName(lib, "technology", &err);

	if( !si2drObjectIsNull(attr,&err) )
	{
		atype = si2drAttrGetAttrType(attr,&err);
		tcln = si2drObjectGetLineNo(attr,&err);
		tcfn = si2drObjectGetFileName(attr,&err);
		
		if( atype != SI2DR_COMPLEX )
		{
			
			sprintf(EB,"The technology attribute specified at %s:%d is a simple attribute and should be a complex attribute!", si2drObjectGetFileName(attr,&err),si2drObjectGetLineNo(attr,&err));
			(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
						  EB, 
						  &err);
			errcount++;
			vtype = si2drSimpleAttrGetValueType(attr,&err);
			switch( vtype )
			{
			case SI2DR_STRING:
				strcpy( tech, si2drSimpleAttrGetStringValue(attr,&err));
				break;
				
			case SI2DR_FLOAT64:
			case SI2DR_INT32:
			case SI2DR_BOOLEAN:
				errcount++;
				sprintf(EB,"Not only is the technology attribute type wrong, so is the value type! It should be a string, but there's a value of some other type here!");
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				break;
			}
		}
		else
		{
			found = 0;
			first = 1;
			
			vals = si2drComplexAttrGetValues(attr,&err);
			do
			{
				si2drIterNextComplexValue(vals,&vtype, &intgr, &float64, &string, &bool, &err);
				switch( vtype )
				{
				case SI2DR_STRING:
					strcpy(tech,string);
					if( first )
						first = 0;
					else
					{
						sprintf(EB,"Found a string argument, but it was not the first and only argument of technology!");
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
					found = 1;
					break;
					
				case SI2DR_FLOAT64:
				case SI2DR_INT32:
				case SI2DR_BOOLEAN:
					sprintf(EB,"The technology attribute at %s:%d should only have one string value; More than one, or non-string attributes detected!", si2drObjectGetFileName(attr,&err),si2drObjectGetLineNo(attr,&err));
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					break;
				}
			}
			while( vtype != SI2DR_UNDEFINED_VALUETYPE );
			si2drIterQuit(vals, &err);
			
			if( !found )
			{
				sprintf(EB,"The technology attribute at %s:%d should only have one string value; No string value was  detected!", si2drObjectGetFileName(attr,&err),si2drObjectGetLineNo(attr,&err));
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
			else
			{
				sprintf(EB,"The technology %s was specified.", tech);
				(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_NO_ERROR, 
							  EB, 
							  &err);
			}
		}
	}
	else
	{
		sprintf(EB,"No technology attribute specified in the library; the technology 'cmos' is assumed.");
		(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_NO_ERROR, 
					  EB, 
					  &err);
		strcpy(tech,"cmos");
		tcln = -1;
	}

	attr = si2drGroupFindAttrByName(lib, "delay_model", &err);
	
	if( !si2drObjectIsNull(attr,&err) )
	{
		dmln = si2drObjectGetLineNo(attr,&err);
		dmfn = si2drObjectGetFileName(attr,&err);
		atype = si2drAttrGetAttrType(attr,&err);
		if( atype != SI2DR_SIMPLE )
		{
			sprintf(EB,"The delay_model attribute was not a simple attribute. Ignoring the specification. 'generic_cmos' will be assumed.");
			(*MsgPrinter)(SI2DR_SEVERITY_WARN, SI2DR_NO_ERROR, 
						  EB, 
						  &err);
			strcpy(model,"generic_cmos");
			errcount++;
		}
		else
		{
			vtype = si2drSimpleAttrGetValueType(attr,&err);
			switch( vtype )
			{
			case SI2DR_STRING:
				strcpy(model, si2drSimpleAttrGetStringValue(attr,&err));
				sprintf(EB,"delay_model specified was %s.", model);
				(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_NO_ERROR, 
							  EB, 
							  &err);
				break;
				
			case SI2DR_FLOAT64:
			case SI2DR_INT32:
			case SI2DR_BOOLEAN:
				sprintf(EB,"The delay_model attribute at %s:%d was not a simple attribute of string type. 'generic_cmos' will be assumed.", si2drObjectGetFileName(attr,&err),si2drObjectGetLineNo(attr,&err));
				(*MsgPrinter)(SI2DR_SEVERITY_WARN, SI2DR_NO_ERROR, 
							  EB, 
							  &err);
				errcount++;
				strcpy(model,"generic_cmos");
				break;
			}
		}
	}
	else
	{
		(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_NO_ERROR, 
					  "No delay_model attribute specified in the library; the delay_model 'generic_cmos' is assumed.", 
					  &err);
		strcpy(model,"generic_cmos");
		dmln = -1;
	}
	/* right at this moment, I need to come up with a better convention for naming the desc files.... */
	/* find the cmos/cmos model for now */
	found = 0;

	if( dmln > 0 && tcln > 0 && dmln < tcln )
	{
		sprintf(EB,"The delay_model should be declared after the technology, but is not. Check lines %s:%d and %s:%d.",
			   tcfn,tcln, dmfn,dmln);
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
	}
	
	if( !strcmp(tech,"cmos") )
		for(i=0;i<libsynt_tech_num ;i++)
		{
			if( !strcmp(libsynt_techs[i].name, model) )
			{
				found =1;
				break;
			}
		}
	if( !found )
		for(i=0;i<libsynt_tech_num ;i++)
		{
			if( !strcmp(libsynt_techs[i].name, tech) )
			{
				found =1;
				break;
			}
		}
	if( !found )
	{
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR,
					  "I cannot check the syntax of this particular technology/delay_model combination!", 
					  &err);

		(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_NO_ERROR, 
					  " Here is a list of technologies/delay_models I can handle:", 
					  &err);
		for(i=0;i<libsynt_tech_num ;i++)
		{
			sprintf(EB,"  %s", libsynt_techs[i].name);
			(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_NO_ERROR, 
						  EB, 
						  &err);
		}
		return SI2DR_SYNTAX_ERROR;
	}
	
	sprintf(EB,"Using the %s syntax tables...",libsynt_techs[i].name);
	(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_NO_ERROR, 
				  EB, 
				  &err);

	
	check_group_correspondence(lib, libsynt_techs[i].lib );
	
	if( errcount > 0 )
		return SI2DR_SYNTAX_ERROR;
	else
		return SI2DR_NO_ERROR;
}

static void enter_define_cell_area(si2drAttrIdT a)
{
	si2drErrorT err;
	si2drObjectIdT toid;
	si2drValuesIdT vals;
	si2drValueTypeT vtype;
	si2drInt32T     intgr;
	si2drFloat64T   float64;
	si2drStringT    string;
	si2drBooleanT   bool;
	char *p,*t;

	
	/* the object here is to add a define to the database;
	   but in such a way so it will not show up if we print the database! */

	/* first, get the first argument */

	/* should be a string with a list of comma/space sep'd numbers */
	if( si2drAttrGetAttrType(a,&err) != SI2DR_COMPLEX )
	{
		return; /* this will already be reported */
	}
		
	vals = si2drComplexAttrGetValues(a,&err);
	si2drIterNextComplexValue(vals,&vtype, &intgr, &float64, &string, &bool, &err);
	si2drIterQuit(vals,&err);
	
	if( vtype == SI2DR_UNDEFINED_VALUETYPE )
	{
		sprintf(EB,"%s:%d: The 'define_cell_area' attribute has no values.",
			   si2drObjectGetFileName(a,&err),
			   si2drObjectGetLineNo(a,&err));
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
		return;
	}
	
	if( vtype != SI2DR_STRING )
	{
		sprintf(EB,"%s:%d: The 'define_cell_area' attribute should be a string.",
			   si2drObjectGetFileName(a,&err),
			   si2drObjectGetLineNo(a,&err));
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
		return;
	}
	
	if( !string || !string[0] )
	{
		sprintf(EB,"%s:%d: The 'define_cell_area' attribute is an empty string.",
			   si2drObjectGetFileName(a,&err),
			   si2drObjectGetLineNo(a,&err));
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
		return;
	}
	create_floating_define_for_cell_area(string);
}


static void attribute_specific_checks( si2drAttrIdT a )
{
	si2drErrorT err;
	si2drStringT name = si2drAttrGetName(a,&err);
	const struct libAttrMap *x = lookup_attr_name (name, strlen(name));
	attr_enum attrtype = x->type;

	switch( attrtype )
	{
	case LIBERTY_ATTRENUM_input_pins:
		break;
		
	case LIBERTY_ATTRENUM_fpga_timing_type:
		break;
		
	case LIBERTY_ATTRENUM_fpga_degenerate_output:
		break;
		
	case LIBERTY_ATTRENUM_xnf_schnm:
		break;
		
	case LIBERTY_ATTRENUM_xnf_device_name:
		break;
		
	case LIBERTY_ATTRENUM_fpga_min_degen_input_size:
		break;
		
	case LIBERTY_ATTRENUM_fpga_max_degen_input_size:
		break;
		
	case LIBERTY_ATTRENUM_fpga_lut_output:
		break;
		
	case LIBERTY_ATTRENUM_fpga_lut_insert_before_sequential:
		break;
		
	case LIBERTY_ATTRENUM_fpga_family:
		break;
		
	case LIBERTY_ATTRENUM_fpga_complex_degenerate:
		break;
		
	case LIBERTY_ATTRENUM_fpga_cell_type:
		break;
		
	case LIBERTY_ATTRENUM_fpga_bridge_inputs:
		break;
		
	case LIBERTY_ATTRENUM_fpga_prefer_undegenerated_gates:
		break;
		
	case LIBERTY_ATTRENUM_fpga_allow_duplicate_degenerated_gates:
		break;
		
	case LIBERTY_ATTRENUM_force_inverter_removal:
		break;
		
	case LIBERTY_ATTRENUM_fanout_resistance:
		break;
		
	case LIBERTY_ATTRENUM_fanout_capacitance:
		break;
		
	case LIBERTY_ATTRENUM_fanout_area:
		break;
		
	case LIBERTY_ATTRENUM_wire_load_from_area:
		break;
		
	case LIBERTY_ATTRENUM_fanout_length:
		break;
		
	case LIBERTY_ATTRENUM_slope:
		break;
		
	case LIBERTY_ATTRENUM_resistance:
		break;
		
	case LIBERTY_ATTRENUM_slower_factor:
		break;
		
	case LIBERTY_ATTRENUM_faster_factor:
		break;
		
	case LIBERTY_ATTRENUM_default_power_rail:
		break;
		
	case LIBERTY_ATTRENUM_vomin:
		break;
		
	case LIBERTY_ATTRENUM_vomax:
		break;
		
	case LIBERTY_ATTRENUM_vol:
		break;
		
	case LIBERTY_ATTRENUM_voh:
		break;
		
	case LIBERTY_ATTRENUM_power_rail:
		break;
		
	case LIBERTY_ATTRENUM_voltage:
		break;
		
	case LIBERTY_ATTRENUM_tree_type:
		break;
		
	case LIBERTY_ATTRENUM_temperature:
		break;

	case LIBERTY_ATTRENUM_process:
		break;

	case LIBERTY_ATTRENUM_short:
		break;

	case LIBERTY_ATTRENUM_cell_name:
		break;

	case LIBERTY_ATTRENUM_variable_3:
		break;

	case LIBERTY_ATTRENUM_vimin:
		break;

	case LIBERTY_ATTRENUM_vimax:
		break;

	case LIBERTY_ATTRENUM_vil:
		break;

	case LIBERTY_ATTRENUM_vih:
		break;

	case LIBERTY_ATTRENUM_variable_2:
		break;

	case LIBERTY_ATTRENUM_variable_1:
		break;

	case LIBERTY_ATTRENUM_downto:
		break;

	case LIBERTY_ATTRENUM_data_type:
		break;

	case LIBERTY_ATTRENUM_bit_width:
		break;

	case LIBERTY_ATTRENUM_bit_to:
		break;

	case LIBERTY_ATTRENUM_bit_from:
		break;

	case LIBERTY_ATTRENUM_base_type:
		break;

	case LIBERTY_ATTRENUM_table:
		break;

	case LIBERTY_ATTRENUM_force_11:
		break;

	case LIBERTY_ATTRENUM_force_10:
		break;

	case LIBERTY_ATTRENUM_force_01:
		break;

	case LIBERTY_ATTRENUM_force_00:
		break;

	case LIBERTY_ATTRENUM_tracks:
		break;

	case LIBERTY_ATTRENUM_total_track_area:
		break;

	case LIBERTY_ATTRENUM_word_width:
		break;

	case LIBERTY_ATTRENUM_type:
		break;

	case LIBERTY_ATTRENUM_row_address:
		break;

	case LIBERTY_ATTRENUM_column_address:
		break;

	case LIBERTY_ATTRENUM_address_width:
		break;

	case LIBERTY_ATTRENUM_value:
		break;

	case LIBERTY_ATTRENUM_enable_also:
		break;

	case LIBERTY_ATTRENUM_data_in:
		break;

	case LIBERTY_ATTRENUM_shifts:
		break;

	case LIBERTY_ATTRENUM_edges:
		break;

	case LIBERTY_ATTRENUM_multiplied_by:
		break;

	case LIBERTY_ATTRENUM_master_pin:
		break;

	case LIBERTY_ATTRENUM_invert:
		break;

	case LIBERTY_ATTRENUM_duty_cycle:
		break;

	case LIBERTY_ATTRENUM_divided_by:
		break;

	case LIBERTY_ATTRENUM_clock_pin:
		break;

	case LIBERTY_ATTRENUM_preset:
		break;

	case LIBERTY_ATTRENUM_next_state:
		break;

	case LIBERTY_ATTRENUM_clocked_on_also:
		break;

	case LIBERTY_ATTRENUM_clear_preset_var2:
		break;

	case LIBERTY_ATTRENUM_clear_preset_var1:
		break;

	case LIBERTY_ATTRENUM_clear:
		break;

	case LIBERTY_ATTRENUM_bus_type:
		break;

	case LIBERTY_ATTRENUM_tdisable:
		break;

	case LIBERTY_ATTRENUM_edge_type:
		break;

	case LIBERTY_ATTRENUM_rise_pin_resistance:
		break;

	case LIBERTY_ATTRENUM_rise_delay_intercept:
		break;

	case LIBERTY_ATTRENUM_mode:
		break;

	case LIBERTY_ATTRENUM_fall_pin_resistance:
		break;

	case LIBERTY_ATTRENUM_fall_delay_intercept:
		break;

	case LIBERTY_ATTRENUM_when_start:
		break;

	case LIBERTY_ATTRENUM_when_end:
		break;

	case LIBERTY_ATTRENUM_timing_type:
		break;

	case LIBERTY_ATTRENUM_timing_sense:
		break;

	case LIBERTY_ATTRENUM_steady_state_resistance_low_min:
		break;

	case LIBERTY_ATTRENUM_steady_state_resistance_low_max:
		break;

	case LIBERTY_ATTRENUM_steady_state_resistance_high_min:
		break;

	case LIBERTY_ATTRENUM_steady_state_resistance_high_max:
		break;

	case LIBERTY_ATTRENUM_steady_state_resistance_float_min:
		break;

	case LIBERTY_ATTRENUM_steady_state_resistance_float_max:
		break;

	case LIBERTY_ATTRENUM_slope_rise:
		break;

	case LIBERTY_ATTRENUM_slope_fall:
		break;

	case LIBERTY_ATTRENUM_setup_coefficient:
		break;

	case LIBERTY_ATTRENUM_sdf_edges:
		break;

	case LIBERTY_ATTRENUM_sdf_cond_start:
		break;

	case LIBERTY_ATTRENUM_sdf_cond_end:
		break;

	case LIBERTY_ATTRENUM_rise_resistance:
		break;

	case LIBERTY_ATTRENUM_related_output_pin:
		break;

	case LIBERTY_ATTRENUM_related_bus_equivalent:
		break;

	case LIBERTY_ATTRENUM_intrinsic_rise:
		break;

	case LIBERTY_ATTRENUM_intrinsic_fall:
		break;

	case LIBERTY_ATTRENUM_hold_coefficient:
		break;

	case LIBERTY_ATTRENUM_fall_resistance:
		break;

	case LIBERTY_ATTRENUM_edge_rate_sensitivity_r1:
		break;

	case LIBERTY_ATTRENUM_edge_rate_sensitivity_r0:
		break;

	case LIBERTY_ATTRENUM_edge_rate_sensitivity_f1:
		break;

	case LIBERTY_ATTRENUM_edge_rate_sensitivity_f0:
		break;

	case LIBERTY_ATTRENUM_default_timing:
		break;

	case LIBERTY_ATTRENUM_lsi_pad:
		break;

	case LIBERTY_ATTRENUM_fsim_map:
		break;

	case LIBERTY_ATTRENUM_constraint:
		break;

	case LIBERTY_ATTRENUM_sdf_cond:
		break;

	case LIBERTY_ATTRENUM_constraint_low:
		break;

	case LIBERTY_ATTRENUM_constraint_high:
		break;

	case LIBERTY_ATTRENUM_enable:
		break;

	case LIBERTY_ATTRENUM_clocked_on:
		break;

	case LIBERTY_ATTRENUM_address:
		break;

	case LIBERTY_ATTRENUM_when:
		break;

	case LIBERTY_ATTRENUM_switching_together_group:
		break;

	case LIBERTY_ATTRENUM_switching_interval:
		break;

	case LIBERTY_ATTRENUM_rising_together_group:
		break;

	case LIBERTY_ATTRENUM_related_rising_pin:
		break;

	case LIBERTY_ATTRENUM_related_outputs:
		break;

	case LIBERTY_ATTRENUM_related_inputs:
		break;

	case LIBERTY_ATTRENUM_related_input:
		break;

	case LIBERTY_ATTRENUM_related_falling_pin:
		break;

	case LIBERTY_ATTRENUM_falling_together_group:
		break;

	case LIBERTY_ATTRENUM_equal_or_opposite_output:
		break;

	case LIBERTY_ATTRENUM_values:
		break;

	case LIBERTY_ATTRENUM_intermediate_values:
		break;

	case LIBERTY_ATTRENUM_index_3:
		break;

	case LIBERTY_ATTRENUM_index_2:
		break;

	case LIBERTY_ATTRENUM_index_1:
		break;

	case LIBERTY_ATTRENUM_related_pin:
		break;

	case LIBERTY_ATTRENUM_related_bus_pins:
		break;

	case LIBERTY_ATTRENUM_members:
		break;

	case LIBERTY_ATTRENUM_input_map_shift:
		break;

	case LIBERTY_ATTRENUM_x_function:
		break;

	case LIBERTY_ATTRENUM_three_state:
		break;

	case LIBERTY_ATTRENUM_test_output_only:
		break;

	case LIBERTY_ATTRENUM_state_function:
		break;

	case LIBERTY_ATTRENUM_slew_control:
		break;

	case LIBERTY_ATTRENUM_signal_type:
		break;

	case LIBERTY_ATTRENUM_rise_time_before_threshold:
		break;

	case LIBERTY_ATTRENUM_rise_time_after_threshold:
		break;

	case LIBERTY_ATTRENUM_rise_current_slope_before_threshold:
		break;

	case LIBERTY_ATTRENUM_rise_current_slope_after_threshold:
		break;

	case LIBERTY_ATTRENUM_rise_capacitance:
		break;

	case LIBERTY_ATTRENUM_reference_capacitance:
		break;

	case LIBERTY_ATTRENUM_pulling_resistance:
		break;

	case LIBERTY_ATTRENUM_pulling_current:
		break;

	case LIBERTY_ATTRENUM_primary_output:
		break;

	case LIBERTY_ATTRENUM_prefer_tied:
		break;

	case LIBERTY_ATTRENUM_pin_func_type:
		break;

	case LIBERTY_ATTRENUM_output_voltage:
		break;

	case LIBERTY_ATTRENUM_output_signal_level:
		break;

	case LIBERTY_ATTRENUM_nextstate_type:
		break;

	case LIBERTY_ATTRENUM_multicell_pad_pin:
		break;

	case LIBERTY_ATTRENUM_min_transition:
		break;

	case LIBERTY_ATTRENUM_min_pulse_width_low:
		break;

	case LIBERTY_ATTRENUM_min_pulse_width_high:
		break;

	case LIBERTY_ATTRENUM_min_period:
		break;

	case LIBERTY_ATTRENUM_min_fanout:
		break;

	case LIBERTY_ATTRENUM_min_capacitance:
		break;

	case LIBERTY_ATTRENUM_max_transition:
		break;

	case LIBERTY_ATTRENUM_max_time_borrow:
		break;

	case LIBERTY_ATTRENUM_max_fanout:
		break;

	case LIBERTY_ATTRENUM_max_capacitance:
		break;

	case LIBERTY_ATTRENUM_is_pad:
		break;

	case LIBERTY_ATTRENUM_inverted_output:
		break;

	case LIBERTY_ATTRENUM_internal_node:
		break;

	case LIBERTY_ATTRENUM_input_voltage:
		break;

	case LIBERTY_ATTRENUM_input_signal_level:
		break;

	case LIBERTY_ATTRENUM_input_map:
		break;

	case LIBERTY_ATTRENUM_hysteresis:
		break;

	case LIBERTY_ATTRENUM_function:
		break;

	case LIBERTY_ATTRENUM_fault_model:
		break;

	case LIBERTY_ATTRENUM_fanout_load:
		break;

	case LIBERTY_ATTRENUM_fall_time_before_threshold:
		break;

	case LIBERTY_ATTRENUM_fall_time_after_threshold:
		break;

	case LIBERTY_ATTRENUM_fall_current_slope_before_threshold:
		break;

	case LIBERTY_ATTRENUM_fall_current_slope_after_threshold:
		break;

	case LIBERTY_ATTRENUM_fall_capacitance:
		break;

	case LIBERTY_ATTRENUM_edge_rate_rise:
		break;

	case LIBERTY_ATTRENUM_edge_rate_load_rise:
		break;

	case LIBERTY_ATTRENUM_edge_rate_load_fall:
		break;

	case LIBERTY_ATTRENUM_edge_rate_fall:
		break;

	case LIBERTY_ATTRENUM_edge_rate_breakpoint_r1:
		break;

	case LIBERTY_ATTRENUM_edge_rate_breakpoint_r0:
		break;

	case LIBERTY_ATTRENUM_edge_rate_breakpoint_f1:
		break;

	case LIBERTY_ATTRENUM_edge_rate_breakpoint_f0:
		break;

	case LIBERTY_ATTRENUM_driver_type:
		break;

	case LIBERTY_ATTRENUM_drive_current:
		break;

	case LIBERTY_ATTRENUM_direction:
		break;

	case LIBERTY_ATTRENUM_dcm_timing:
		break;

	case LIBERTY_ATTRENUM_connection_class:
		break;

	case LIBERTY_ATTRENUM_complementary_pin:
		break;

	case LIBERTY_ATTRENUM_clock_gate_test_pin:
		break;

	case LIBERTY_ATTRENUM_clock_gate_out_pin:
		break;

	case LIBERTY_ATTRENUM_clock_gate_obs_pin:
		break;

	case LIBERTY_ATTRENUM_clock_gate_enable_pin:
		break;

	case LIBERTY_ATTRENUM_clock_gate_clock_pin:
		break;

	case LIBERTY_ATTRENUM_clock:
		break;

	case LIBERTY_ATTRENUM_capacitance:
		break;

	case LIBERTY_ATTRENUM_rail_connection:
		break;

	case LIBERTY_ATTRENUM_pin_opposite:
		break;

	case LIBERTY_ATTRENUM_pin_equal:
		break;

	case LIBERTY_ATTRENUM_vhdl_name:
		break;

	case LIBERTY_ATTRENUM_use_for_size_only:
		break;

	case LIBERTY_ATTRENUM_single_bit_degenerate:
		break;

	case LIBERTY_ATTRENUM_set_node:
		break;

	case LIBERTY_ATTRENUM_scan_group:
		break;

	case LIBERTY_ATTRENUM_scaling_factors:
		break;

	case LIBERTY_ATTRENUM_preferred:
		break;

	case LIBERTY_ATTRENUM_pad_type:
		break;

	case LIBERTY_ATTRENUM_pad_cell:
		break;

	case LIBERTY_ATTRENUM_observe_node:
		break;

	case LIBERTY_ATTRENUM_mpm_name:
		break;

	case LIBERTY_ATTRENUM_mpm_libname:
		break;

	case LIBERTY_ATTRENUM_map_only:
		break;

	case LIBERTY_ATTRENUM_is_clock_gating_cell:
		break;

	case LIBERTY_ATTRENUM_interface_timing:
		break;

	case LIBERTY_ATTRENUM_handle_negative_constraint:
		break;

	case LIBERTY_ATTRENUM_geometry_print:
		break;

	case LIBERTY_ATTRENUM_dont_use:
		break;

	case LIBERTY_ATTRENUM_dont_touch:
		break;

	case LIBERTY_ATTRENUM_dont_fault:
		break;

	case LIBERTY_ATTRENUM_contention_condition:
		break;

	case LIBERTY_ATTRENUM_clock_gating_integrated_cell:
		break;

	case LIBERTY_ATTRENUM_cell_leakage_power:
		break;

	case LIBERTY_ATTRENUM_cell_footprint:
		break;

	case LIBERTY_ATTRENUM_auxiliary_pad_cell:
		break;

	case LIBERTY_ATTRENUM_area:
		break;

	case LIBERTY_ATTRENUM_technology:
		break;

	case LIBERTY_ATTRENUM_routing_layers:
		break;

	case LIBERTY_ATTRENUM_piece_define:
		break;

	case LIBERTY_ATTRENUM_library_features:
		break;

	case LIBERTY_ATTRENUM_define_cell_area:
		enter_define_cell_area(a);
		break;

	case LIBERTY_ATTRENUM_capacitive_load_unit:
		break;

	case LIBERTY_ATTRENUM_voltage_unit:
		break;

	case LIBERTY_ATTRENUM_timing_report:
		break;

	case LIBERTY_ATTRENUM_time_unit:
		break;

	case LIBERTY_ATTRENUM_slew_upper_threshold_pct_rise:
		break;

	case LIBERTY_ATTRENUM_slew_upper_threshold_pct_fall:
		break;

	case LIBERTY_ATTRENUM_slew_lower_threshold_pct_rise:
		break;

	case LIBERTY_ATTRENUM_slew_lower_threshold_pct_fall:
		break;

	case LIBERTY_ATTRENUM_slew_derate_from_library:
		break;

	case LIBERTY_ATTRENUM_simulation:
		break;

	case LIBERTY_ATTRENUM_revision:
		break;

	case LIBERTY_ATTRENUM_pulling_resistance_unit:
		break;

	case LIBERTY_ATTRENUM_preferred_output_pad_voltage:
		break;

	case LIBERTY_ATTRENUM_preferred_output_pad_slew_rate_control:
		break;

	case LIBERTY_ATTRENUM_preferred_input_pad_voltage:
		break;

	case LIBERTY_ATTRENUM_piece_type:
		break;

	case LIBERTY_ATTRENUM_output_threshold_pct_rise:
		break;

	case LIBERTY_ATTRENUM_output_threshold_pct_fall:
		break;

	case LIBERTY_ATTRENUM_nom_voltage:
		break;

	case LIBERTY_ATTRENUM_nom_temperature:
		break;

	case LIBERTY_ATTRENUM_nom_process:
		break;

	case LIBERTY_ATTRENUM_lsi_rounding_digit:
		break;

	case LIBERTY_ATTRENUM_lsi_rounding_cutoff:
		break;

	case LIBERTY_ATTRENUM_lsi_pad_rise:
		break;

	case LIBERTY_ATTRENUM_lsi_pad_fall:
		break;

	case LIBERTY_ATTRENUM_leakage_power_unit:
		break;

	case LIBERTY_ATTRENUM_key_version:
		break;

	case LIBERTY_ATTRENUM_key_seed:
		break;

	case LIBERTY_ATTRENUM_key_file:
		break;

	case LIBERTY_ATTRENUM_key_feature:
		break;

	case LIBERTY_ATTRENUM_key_bit:
		break;

	case LIBERTY_ATTRENUM_k_volt_wire_res:
		break;

	case LIBERTY_ATTRENUM_k_volt_wire_cap:
		break;

	case LIBERTY_ATTRENUM_k_volt_slope_rise:
		break;

	case LIBERTY_ATTRENUM_k_volt_slope_fall:
		break;

	case LIBERTY_ATTRENUM_k_volt_skew_rise:
		break;

	case LIBERTY_ATTRENUM_k_volt_skew_fall:
		break;

	case LIBERTY_ATTRENUM_k_volt_setup_rise:
		break;

	case LIBERTY_ATTRENUM_k_volt_setup_fall:
		break;

	case LIBERTY_ATTRENUM_k_volt_rise_transition:
		break;

	case LIBERTY_ATTRENUM_k_volt_rise_propagation:
		break;

	case LIBERTY_ATTRENUM_k_volt_rise_pin_resistance:
		break;

	case LIBERTY_ATTRENUM_k_volt_rise_delay_intercept:
		break;

	case LIBERTY_ATTRENUM_k_volt_removal_rise:
		break;

	case LIBERTY_ATTRENUM_k_volt_removal_fall:
		break;

	case LIBERTY_ATTRENUM_k_volt_recovery_rise:
		break;

	case LIBERTY_ATTRENUM_k_volt_recovery_fall:
		break;

	case LIBERTY_ATTRENUM_k_volt_pin_cap:
		break;

	case LIBERTY_ATTRENUM_k_volt_nochange_rise:
		break;

	case LIBERTY_ATTRENUM_k_volt_nochange_fall:
		break;

	case LIBERTY_ATTRENUM_k_volt_min_pulse_width_low:
		break;

	case LIBERTY_ATTRENUM_k_volt_min_pulse_width_high:
		break;

	case LIBERTY_ATTRENUM_k_volt_min_period:
		break;

	case LIBERTY_ATTRENUM_k_volt_intrinsic_rise:
		break;

	case LIBERTY_ATTRENUM_k_volt_intrinsic_fall:
		break;

	case LIBERTY_ATTRENUM_k_volt_internal_power:
		break;

	case LIBERTY_ATTRENUM_k_volt_hold_rise:
		break;

	case LIBERTY_ATTRENUM_k_volt_hold_fall:
		break;

	case LIBERTY_ATTRENUM_k_volt_fall_transition:
		break;

	case LIBERTY_ATTRENUM_k_volt_fall_propagation:
		break;

	case LIBERTY_ATTRENUM_k_volt_fall_pin_resistance:
		break;

	case LIBERTY_ATTRENUM_k_volt_fall_delay_intercept:
		break;

	case LIBERTY_ATTRENUM_k_volt_drive_rise:
		break;

	case LIBERTY_ATTRENUM_k_volt_drive_fall:
		break;

	case LIBERTY_ATTRENUM_k_volt_drive_current:
		break;

	case LIBERTY_ATTRENUM_k_volt_cell_rise:
		break;

	case LIBERTY_ATTRENUM_k_volt_cell_leakage_power:
		break;

	case LIBERTY_ATTRENUM_k_volt_cell_fall:
		break;

	case LIBERTY_ATTRENUM_k_volt_cell_degradation:
		break;

	case LIBERTY_ATTRENUM_k_temp_wire_res:
		break;

	case LIBERTY_ATTRENUM_k_temp_wire_cap:
		break;

	case LIBERTY_ATTRENUM_k_temp_slope_rise:
		break;

	case LIBERTY_ATTRENUM_k_temp_slope_fall:
		break;

	case LIBERTY_ATTRENUM_k_temp_skew_rise:
		break;

	case LIBERTY_ATTRENUM_k_temp_skew_fall:
		break;

	case LIBERTY_ATTRENUM_k_temp_setup_rise:
		break;

	case LIBERTY_ATTRENUM_k_temp_setup_fall:
		break;

	case LIBERTY_ATTRENUM_k_temp_rise_transition:
		break;

	case LIBERTY_ATTRENUM_k_temp_rise_propagation:
		break;

	case LIBERTY_ATTRENUM_k_temp_rise_pin_resistance:
		break;

	case LIBERTY_ATTRENUM_k_temp_rise_delay_intercept:
		break;

	case LIBERTY_ATTRENUM_k_temp_removal_rise:
		break;

	case LIBERTY_ATTRENUM_k_temp_removal_fall:
		break;

	case LIBERTY_ATTRENUM_k_temp_recovery_rise:
		break;

	case LIBERTY_ATTRENUM_k_temp_recovery_fall:
		break;

	case LIBERTY_ATTRENUM_k_temp_pin_cap:
		break;

	case LIBERTY_ATTRENUM_k_temp_nochange_rise:
		break;

	case LIBERTY_ATTRENUM_k_temp_nochange_fall:
		break;

	case LIBERTY_ATTRENUM_k_temp_min_pulse_width_low:
		break;

	case LIBERTY_ATTRENUM_k_temp_min_pulse_width_high:
		break;

	case LIBERTY_ATTRENUM_k_temp_min_period:
		break;

	case LIBERTY_ATTRENUM_k_temp_intrinsic_rise:
		break;

	case LIBERTY_ATTRENUM_k_temp_intrinsic_fall:
		break;

	case LIBERTY_ATTRENUM_k_temp_internal_power:
		break;

	case LIBERTY_ATTRENUM_k_temp_hold_rise:
		break;

	case LIBERTY_ATTRENUM_k_temp_hold_fall:
		break;

	case LIBERTY_ATTRENUM_k_temp_fall_transition:
		break;

	case LIBERTY_ATTRENUM_k_temp_fall_propagation:
		break;

	case LIBERTY_ATTRENUM_k_temp_fall_pin_resistance:
		break;

	case LIBERTY_ATTRENUM_k_temp_fall_delay_intercept:
		break;

	case LIBERTY_ATTRENUM_k_temp_drive_rise:
		break;

	case LIBERTY_ATTRENUM_k_temp_drive_fall:
		break;

	case LIBERTY_ATTRENUM_k_temp_drive_current:
		break;

	case LIBERTY_ATTRENUM_k_temp_cell_rise:
		break;

	case LIBERTY_ATTRENUM_k_temp_cell_leakage_power:
		break;

	case LIBERTY_ATTRENUM_k_temp_cell_fall:
		break;

	case LIBERTY_ATTRENUM_k_temp_cell_degradation:
		break;

	case LIBERTY_ATTRENUM_k_process_wire_res:
		break;

	case LIBERTY_ATTRENUM_k_process_wire_cap:
		break;

	case LIBERTY_ATTRENUM_k_process_slope_rise:
		break;

	case LIBERTY_ATTRENUM_k_process_slope_fall:
		break;

	case LIBERTY_ATTRENUM_k_process_skew_rise:
		break;

	case LIBERTY_ATTRENUM_k_process_skew_fall:
		break;

	case LIBERTY_ATTRENUM_k_process_setup_rise:
		break;

	case LIBERTY_ATTRENUM_k_process_setup_fall:
		break;

	case LIBERTY_ATTRENUM_k_process_rise_transition:
		break;

	case LIBERTY_ATTRENUM_k_process_rise_propagation:
		break;

	case LIBERTY_ATTRENUM_k_process_rise_pin_resistance:
		break;

	case LIBERTY_ATTRENUM_k_process_rise_delay_intercept:
		break;

	case LIBERTY_ATTRENUM_k_process_removal_rise:
		break;

	case LIBERTY_ATTRENUM_k_process_removal_fall:
		break;

	case LIBERTY_ATTRENUM_k_process_recovery_rise:
		break;

	case LIBERTY_ATTRENUM_k_process_recovery_fall:
		break;

	case LIBERTY_ATTRENUM_k_process_pin_rise_cap:
		break;

	case LIBERTY_ATTRENUM_k_process_pin_fall_cap:
		break;

	case LIBERTY_ATTRENUM_k_process_pin_cap:
		break;

	case LIBERTY_ATTRENUM_k_process_nochange_rise:
		break;

	case LIBERTY_ATTRENUM_k_process_nochange_fall:
		break;

	case LIBERTY_ATTRENUM_k_process_min_pulse_width_low:
		break;

	case LIBERTY_ATTRENUM_k_process_min_pulse_width_high:
		break;

	case LIBERTY_ATTRENUM_k_process_min_period:
		break;

	case LIBERTY_ATTRENUM_k_process_intrinsic_rise:
		break;

	case LIBERTY_ATTRENUM_k_process_intrinsic_fall:
		break;

	case LIBERTY_ATTRENUM_k_process_internal_power:
		break;

	case LIBERTY_ATTRENUM_k_process_hold_rise:
		break;

	case LIBERTY_ATTRENUM_k_process_hold_fall:
		break;

	case LIBERTY_ATTRENUM_k_process_fall_transition:
		break;

	case LIBERTY_ATTRENUM_k_process_fall_propagation:
		break;

	case LIBERTY_ATTRENUM_k_process_fall_pin_resistance:
		break;

	case LIBERTY_ATTRENUM_k_process_fall_delay_intercept:
		break;

	case LIBERTY_ATTRENUM_k_process_drive_rise:
		break;

	case LIBERTY_ATTRENUM_k_process_drive_fall:
		break;

	case LIBERTY_ATTRENUM_k_process_drive_current:
		break;

	case LIBERTY_ATTRENUM_k_process_cell_rise:
		break;

	case LIBERTY_ATTRENUM_k_process_cell_leakage_power:
		break;

	case LIBERTY_ATTRENUM_k_process_cell_fall:
		break;

	case LIBERTY_ATTRENUM_k_process_cell_degradation:
		break;

	case LIBERTY_ATTRENUM_input_threshold_pct_rise:
		break;

	case LIBERTY_ATTRENUM_input_threshold_pct_fall:
		break;

	case LIBERTY_ATTRENUM_in_place_swap_mode:
		break;

	case LIBERTY_ATTRENUM_em_temp_degradation_factor:
		break;

	case LIBERTY_ATTRENUM_delay_model:
		break;

	case LIBERTY_ATTRENUM_default_wire_load_selection:
		break;

	case LIBERTY_ATTRENUM_default_wire_load_resistance:
		break;

	case LIBERTY_ATTRENUM_default_wire_load_mode:
		break;

	case LIBERTY_ATTRENUM_default_wire_load_capacitance:
		break;

	case LIBERTY_ATTRENUM_default_wire_load_area:
		break;

	case LIBERTY_ATTRENUM_default_wire_load:
		break;

	case LIBERTY_ATTRENUM_default_slope_rise:
		break;

	case LIBERTY_ATTRENUM_default_slope_fall:
		break;

	case LIBERTY_ATTRENUM_default_setup_coefficient:
		break;

	case LIBERTY_ATTRENUM_default_rise_pin_resistance:
		break;

	case LIBERTY_ATTRENUM_default_rise_delay_intercept:
		break;

	case LIBERTY_ATTRENUM_default_reference_capacitance:
		break;

	case LIBERTY_ATTRENUM_default_rc_rise_coefficient:
		break;

	case LIBERTY_ATTRENUM_default_rc_fall_coefficient:
		break;

	case LIBERTY_ATTRENUM_default_output_pin_rise_res:
		break;

	case LIBERTY_ATTRENUM_default_output_pin_fall_res:
		break;

	case LIBERTY_ATTRENUM_default_output_pin_cap:
		break;

	case LIBERTY_ATTRENUM_default_operating_conditions:
		break;

	case LIBERTY_ATTRENUM_default_min_porosity:
		break;

	case LIBERTY_ATTRENUM_default_max_utilization:
		break;

	case LIBERTY_ATTRENUM_default_max_transition:
		break;

	case LIBERTY_ATTRENUM_default_max_fanout:
		break;

	case LIBERTY_ATTRENUM_default_max_capacitance:
		break;

	case LIBERTY_ATTRENUM_default_leakage_power_density:
		break;

	case LIBERTY_ATTRENUM_default_intrinsic_rise:
		break;

	case LIBERTY_ATTRENUM_default_intrinsic_fall:
		break;

	case LIBERTY_ATTRENUM_default_input_pin_cap:
		break;

	case LIBERTY_ATTRENUM_default_inout_pin_rise_res:
		break;

	case LIBERTY_ATTRENUM_default_inout_pin_fall_res:
		break;

	case LIBERTY_ATTRENUM_default_inout_pin_cap:
		break;

	case LIBERTY_ATTRENUM_default_hold_coefficient:
		break;

	case LIBERTY_ATTRENUM_default_fanout_load:
		break;

	case LIBERTY_ATTRENUM_default_fall_pin_resistance:
		break;

	case LIBERTY_ATTRENUM_default_fall_delay_intercept:
		break;

	case LIBERTY_ATTRENUM_default_edge_rate_breakpoint_r1:
		break;

	case LIBERTY_ATTRENUM_default_edge_rate_breakpoint_r0:
		break;

	case LIBERTY_ATTRENUM_default_edge_rate_breakpoint_f1:
		break;

	case LIBERTY_ATTRENUM_default_edge_rate_breakpoint_f0:
		break;

	case LIBERTY_ATTRENUM_default_connection_class:
		break;

	case LIBERTY_ATTRENUM_default_cell_leakage_power:
		break;

	case LIBERTY_ATTRENUM_date:
		break;

	case LIBERTY_ATTRENUM_current_unit:
		break;

	case LIBERTY_ATTRENUM_comment:
		break;

	case LIBERTY_ATTRENUM_bus_naming_style:
		break;

	}
	
	
}

static si2drGroupIdT get_containing_group(si2drGroupIdT group, si2drStringT type)
{
	si2drErrorT err;
	const struct libGroupMap *lgm;
	si2drGroupIdT gup = group;
	group_enum gt;
	lgm = lookup_group_name (type, strlen(type));
	if( lgm )
		gt = lgm->type;
	else
	{
		sprintf(EB,"BAD type (%s) handed to get_containing_group!", type);
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_INTERNAL_SYSTEM_ERROR, 
					  EB, 
					  &err);
		return si2drPIGetNullId(&err);
	}
	
	while( !si2drObjectIsNull((gup = si2drObjectGetOwner(gup,&err)),&err) )
	{
		if( si2drGroupGetID(gup,&err) == gt )
			return gup;
	}
	return si2drPIGetNullId(&err);
}

static char *mystrtod(char *str, char **end)
{
	char *x = str,*y;

	while( *x && (*x == ' ' || *x  == '	' || *x == ',' || *x == '\n' || *x == '\\') )
	{
		x++;
	}
	y = x;
	
	while( *x && (*x == '+' || *x == '-' || (*x >= '0'&& *x <= '9')
				  || *x == 'E' || *x == 'e' || *x == '.' ))
	{
		x++;
	}
	if( y == x )
	{
		*end = str;
		return str;
	}
	*end = x;
	return x;
}


static int count_floats_in_string(char *s)
{
	char *p,*t;
	int i= 0;
	double x;
	
	p = s;
	
	while( mystrtod(p,&t) != p )
	{
		i++;
		p = t;
	}
	return i;
}

static int count_strings_in_list(si2drAttrIdT attr)
{
	si2drErrorT err;
	si2drValuesIdT vals = si2drComplexAttrGetValues(attr,&err);
	int i = 0;
	si2drValueTypeT type;
	si2drInt32T intgr;
	si2drFloat64T fl;
	si2drStringT str;
	si2drBooleanT bool;
	
	while( 1 )
	{
		si2drIterNextComplexValue(vals,&type,&intgr,&fl,&str,&bool,&err);
		if( type != SI2DR_UNDEFINED_VALUETYPE )
		{
			i++;
		}
		else
			break;
	}
	si2drIterQuit(vals,&err);
	
	return i;
}


static void check_lu_table_template_arraysize( si2drGroupIdT group)
{
	si2drErrorT err;
	si2drNamesIdT names;
	si2drStringT str;
	si2drAttrIdT attr;
	si2drAttrsIdT attrs;
	si2drGroupIdT rg,rg2;
	si2drGroupsIdT groups;

	names = si2drGroupGetNames(group, &err);
	str = si2drIterNextName(names,&err);
	si2drIterQuit(names,&err);
	if( str && str[0] )
	{
		char strx[8000];
		strcpy(strx,str);

		if(  !strcmp(strx,"scalar") )
		{
			si2drAttrIdT vs;
			/* should be just one value */
			vs = si2drGroupFindAttrByName(group, "values", &err);
			
			if( !si2drObjectIsNull(vs,&err) )
			{
				int numstrs = count_strings_in_list(vs);
				int numfloats;
				si2drValuesIdT vals;
				si2drStringT t1s,t2s;
				si2drValueTypeT type;
				si2drInt32T     intgr;
				si2drFloat64T   float64;
				si2drStringT    string;
				si2drBooleanT   bool;
				
				
				vals = si2drComplexAttrGetValues(vs,&err);
				si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
				si2drIterQuit(vals,&err);

				if( type == SI2DR_UNDEFINED_VALUETYPE )
				{
					sprintf(EB,"%s:%d, %s references template %s, so",
						   si2drObjectGetFileName(group, &err),
						   si2drObjectGetLineNo(group, &err),
						   si2drGroupGetGroupType(group,&err),
						   strx);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					sprintf(EB,"                  the values attribute should have one string specified\n                  with a number in it, but does not."
						   );
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				if( type != SI2DR_STRING )
				{
					sprintf(EB,"%s:%d, %s references template %s, so",
						   si2drObjectGetFileName(group, &err),
						   si2drObjectGetLineNo(group, &err),
						   si2drGroupGetGroupType(group,&err),
						   strx);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					sprintf(EB,"                  the values attribute should have one string specified\n                  with a number in it, but what is there is not a string."
						   );
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				else
				{
					numfloats = count_floats_in_string(string);
					if( numfloats != 1 || numstrs != 1 )
					{
						sprintf(EB,"%s:%d, %s references template %s, but",
							   si2drObjectGetFileName(group, &err),
							   si2drObjectGetLineNo(group, &err),
							   si2drGroupGetGroupType(group,&err),
							   strx);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						sprintf(EB,"                  the values attribute has more than one value or string specified."
							   );
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}
			}
			else
			{
				sprintf(EB,"%s:%d, %s references template %s, but",
					   si2drObjectGetFileName(group, &err),
					   si2drObjectGetLineNo(group, &err),
					   si2drGroupGetGroupType(group,&err),
					   strx);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  "                  the group does not have a values attribute.", 
							  &err);
				errcount++;
			}

			return;
		}
		
		/* find an lu_table_template with this name */
		rg = get_containing_group(group,"library");
		rg2 = si2drGroupFindGroupByName(rg,strx, "lu_table_template",&err);
		if( !si2drObjectIsNull(rg2,&err) )
		{
			si2drAttrIdT ind1,ind2, ind3;
			si2drAttrIdT vs;

			ind1 = si2drGroupFindAttrByName(group, "index_1", &err);
			ind2 = si2drGroupFindAttrByName(group, "index_2", &err);
			ind3 = si2drGroupFindAttrByName(group, "index_3", &err);
			
			if( si2drObjectIsNull(ind1,&err) )
				ind1 = si2drGroupFindAttrByName(rg2, "index_1", &err);

			if( si2drObjectIsNull(ind2,&err) )
				ind2 = si2drGroupFindAttrByName(rg2, "index_2", &err);
			
			if( si2drObjectIsNull(ind3,&err) )
				ind3 = si2drGroupFindAttrByName(rg2, "index_3", &err);
			
			vs = si2drGroupFindAttrByName(group, "values", &err);
			
			if ( !si2drObjectIsNull(ind1,&err) && !si2drObjectIsNull(ind2,&err) && !si2drObjectIsNull(ind3,&err)  )
			{
				if( !si2drObjectIsNull(vs,&err) )
				{
					int tpl1nums, tpl2nums, tpl3nums;
					int numstrs = count_strings_in_list(vs);
					si2drValuesIdT vals;
					si2drStringT t1s,t2s;
					si2drValueTypeT type;
					si2drInt32T     intgr;
					si2drFloat64T   float64;
					si2drStringT    string;
					si2drBooleanT   bool;
					
					
					vals = si2drComplexAttrGetValues(ind1,&err);
					si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
					si2drIterQuit(vals,&err);
					
					if( type == SI2DR_STRING )
					{
						tpl1nums = count_floats_in_string(string);
						
						vals = si2drComplexAttrGetValues(ind2,&err);
						si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
						si2drIterQuit(vals,&err);
						
						if( type == SI2DR_STRING )
						{
							tpl2nums = count_floats_in_string(string);

							
							vals = si2drComplexAttrGetValues(ind3,&err);
							si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
							si2drIterQuit(vals,&err);
						
							if( type == SI2DR_STRING )
							{
								
								tpl3nums = count_floats_in_string(string);

								if( numstrs == tpl1nums * tpl2nums)
								{
								/* now, count the nums in each entry, and make sure they equal tpl2nums */
								
									int i;
									vals = si2drComplexAttrGetValues(vs,&err);
									for(i=0;i<numstrs;i++)
									{
										si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
										if( type == SI2DR_STRING )
										{
											int numc = count_floats_in_string(string);
											if( numc != tpl3nums )
											{
												sprintf(EB,"%s:%d, %s references template %s,",
													   si2drObjectGetFileName(group, &err),
													   si2drObjectGetLineNo(group, &err),
													   si2drGroupGetGroupType(group,&err),
													   strx);
												(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
															  EB, 
															  &err);
												sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
													   tpl1nums*tpl2nums, tpl3nums
													);
												
												(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
															  EB, 
															  &err);
												sprintf(EB,"                  but in the group's values attribute (ending at line %d) value #%d has %d numbers.",
													   si2drObjectGetLineNo(vs, &err),
													   i+1, numc
													);
												(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
															  EB, 
															  &err);
												errcount++;
											}
										}
										else
										{
											sprintf(EB,"%s:%d, %s references template %s,",
												   si2drObjectGetFileName(group, &err),
												   si2drObjectGetLineNo(group, &err),
												   si2drGroupGetGroupType(group,&err),
												   strx);
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
												   tpl1nums*tpl2nums, tpl3nums
												);
											
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  but in the group's values attribute (ending at line %d) value #%d is not a string.",
												   si2drObjectGetLineNo(vs, &err),
												   i+1
												);
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											errcount++;
										}
									}
									si2drIterQuit(vals,&err);
								}
								else
								{
									sprintf(EB,"%s:%d, %s references template %s,",
										   si2drObjectGetFileName(group, &err),
										   si2drObjectGetLineNo(group, &err),
										   si2drGroupGetGroupType(group,&err),
										   strx);
									(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
												  EB, 
												  &err);
									sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
										   tpl1nums*tpl2nums, tpl3nums
										);
									
									(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
												  EB, 
												  &err);
									sprintf(EB,"                  but the group's values attribute (line %d) has %d strings.",
										   si2drObjectGetLineNo(vs, &err),
										   numstrs
										);
									(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
												  EB, 
												  &err);
									errcount++;
								}
							}
							else
							{
								
								sprintf(EB,"%s:%d, %s references template %s, but",
									   si2drObjectGetFileName(group, &err),
									   si2drObjectGetLineNo(group, &err),
									   si2drGroupGetGroupType(group,&err),
									   strx);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								sprintf(EB,"                  the template's index_3 attribute (line %d) does not have a string as a value.",
									   si2drObjectGetLineNo(ind2, &err)
									   );
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
						else
						{
							
							sprintf(EB,"%s:%d, %s references template %s, but",
								   si2drObjectGetFileName(group, &err),
								   si2drObjectGetLineNo(group, &err),
								   si2drGroupGetGroupType(group,&err),
								   strx);
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							sprintf(EB,"                  the template's index_2 attribute (line %d) does not have a string as a value.",
								   si2drObjectGetLineNo(ind2, &err)
								   );
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							errcount++;
						}
					}
					else
					{
						
						sprintf(EB,"%s:%d, %s references template %s, but",
							   si2drObjectGetFileName(group, &err),
							   si2drObjectGetLineNo(group, &err),
							   si2drGroupGetGroupType(group,&err),
							   strx);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						sprintf(EB,"                  the template's index_1 attribute (line %d) does not have a string as a value.",
							   si2drObjectGetLineNo(ind1, &err)
							);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}
				else
				{
					sprintf(EB,"%s:%d, %s references template %s, but",
						   si2drObjectGetFileName(group, &err),
						   si2drObjectGetLineNo(group, &err),
						   si2drGroupGetGroupType(group,&err),
						   strx);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					sprintf(EB,"                  the group does not have a values attribute."
						   );
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				
			}
			else if ( !si2drObjectIsNull(ind1,&err) && !si2drObjectIsNull(ind2,&err) && si2drObjectIsNull(ind3,&err)  )
			{
				if( !si2drObjectIsNull(vs,&err) )
				{
					int tpl1nums, tpl2nums;
					int numstrs = count_strings_in_list(vs);
					si2drValuesIdT vals;
					si2drStringT t1s,t2s;
					si2drValueTypeT type;
					si2drInt32T     intgr;
					si2drFloat64T   float64;
					si2drStringT    string;
					si2drBooleanT   bool;
					
					
					vals = si2drComplexAttrGetValues(ind1,&err);
					si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
					si2drIterQuit(vals,&err);
					
					if( type == SI2DR_STRING )
					{
						tpl1nums = count_floats_in_string(string);
						
						vals = si2drComplexAttrGetValues(ind2,&err);
						si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
						si2drIterQuit(vals,&err);
						
						if( type == SI2DR_STRING )
						{
							tpl2nums = count_floats_in_string(string);
							if( numstrs == tpl1nums )
							{
								/* now, count the nums in each entry, and make sure they equal tpl2nums */
								
								int i;
								vals = si2drComplexAttrGetValues(vs,&err);
								for(i=0;i<numstrs;i++)
								{
									si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
									if( type == SI2DR_STRING )
									{
										int numc = count_floats_in_string(string);
										if( numc != tpl2nums )
										{
											sprintf(EB,"%s:%d, %s references template %s,",
												   si2drObjectGetFileName(group, &err),
												   si2drObjectGetLineNo(group, &err),
												   si2drGroupGetGroupType(group,&err),
												   strx);
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
												   tpl1nums, tpl2nums
												);
											
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  but in the group's values attribute (ending at line %d) value #%d has %d numbers.",
												   si2drObjectGetLineNo(vs, &err),
												   i+1, numc
												);
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											errcount++;
										}
									}
									else
									{
										sprintf(EB,"%s:%d, %s references template %s,",
											   si2drObjectGetFileName(group, &err),
											   si2drObjectGetLineNo(group, &err),
											   si2drGroupGetGroupType(group,&err),
											   strx);
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
											   tpl1nums, tpl2nums
											);
										
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										sprintf(EB,"                  but in the group's values attribute (ending at line %d) value #%d is not a string.",
											   si2drObjectGetLineNo(vs, &err),
											   i+1
											);
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										errcount++;
									}
								}
								si2drIterQuit(vals,&err);
							}
							else
							{
								sprintf(EB,"%s:%d, %s references template %s,",
									   si2drObjectGetFileName(group, &err),
									   si2drObjectGetLineNo(group, &err),
									   si2drGroupGetGroupType(group,&err),
									   strx);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
									   tpl1nums, tpl2nums
									   );
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								
								sprintf(EB,"                  but the group's values attribute (line %d) has %d strings.",
									   si2drObjectGetLineNo(vs, &err),
									   numstrs
									);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
						else
						{
							
							sprintf(EB,"%s:%d, %s references template %s, but",
								   si2drObjectGetFileName(group, &err),
								   si2drObjectGetLineNo(group, &err),
								   si2drGroupGetGroupType(group,&err),
								   strx);
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							sprintf(EB,"                  the template's index_2 attribute (line %d) does not have a string as a value.",
								   si2drObjectGetLineNo(ind2, &err)
								);
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							errcount++;
						}
					}
					else
					{
						
						sprintf(EB,"%s:%d, %s references template %s, but",
							   si2drObjectGetFileName(group, &err),
							   si2drObjectGetLineNo(group, &err),
							   si2drGroupGetGroupType(group,&err),
							   strx);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						sprintf(EB,"                  the template's index_1 attribute (line %d) does not have a string as a value.",
							   si2drObjectGetLineNo(ind1, &err)
							);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}
				else
				{
					sprintf(EB,"%s:%d, %s references template %s, but",
						   si2drObjectGetFileName(group, &err),
						   si2drObjectGetLineNo(group, &err),
						   si2drGroupGetGroupType(group,&err),
						   strx);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					sprintf(EB,"                  the group does not have a values attribute."
						);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
			}
			else
			{
				if( !si2drObjectIsNull(ind1,&err) && si2drObjectIsNull(ind2,&err) )
				{
					if( !si2drObjectIsNull(vs,&err) )
					{
						
						int tpl1nums, tpl2nums;
						int numstrs = count_strings_in_list(vs);
						si2drValuesIdT vals;
						si2drStringT t1s,t2s;
						si2drValueTypeT type;
						si2drInt32T     intgr;
						si2drFloat64T   float64;
						si2drStringT    string;
						si2drBooleanT   bool;
						
						if( numstrs == 1 )
						{
							vals = si2drComplexAttrGetValues(ind1,&err);
							si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
							si2drIterQuit(vals,&err);
							
							if( type == SI2DR_STRING )
							{
								int i;
								tpl1nums = count_floats_in_string(string);
							
								/* now, count the nums in each entry, and make sure they equal tpl2nums */
							
								vals = si2drComplexAttrGetValues(vs,&err);
								for(i=0;i<numstrs;i++)
								{
									si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
									if( type == SI2DR_STRING )
									{
										int numc = count_floats_in_string(string);
										if( numc != tpl1nums )
										{
											sprintf(EB,"%s:%d, %s references template %s,",
												   si2drObjectGetFileName(group, &err),
												   si2drObjectGetLineNo(group, &err),
												   si2drGroupGetGroupType(group,&err),
												   strx);
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  which says there should be 1 string, with %d numbers in it in the values table,",
												   tpl1nums
												   );
											
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  but in the group's values attribute (ending at line %d) has %d numbers.",
												   si2drObjectGetLineNo(vs, &err),
												   numc
												);
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											errcount++;
										}
									}
									else
									{
										sprintf(EB,"%s:%d, %s references template %s,",
											   si2drObjectGetFileName(group, &err),
											   si2drObjectGetLineNo(group, &err),
											   si2drGroupGetGroupType(group,&err),
											   strx);
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										sprintf(EB,"                  which says there should be 1 string, with %d numbers in it in the values table,",
											   tpl1nums
											);
										
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										sprintf(EB,"                  but in the group's values attribute (ending at line %d) is not a string.",
											   si2drObjectGetLineNo(vs, &err)
											);
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										errcount++;
									}
								}
								si2drIterQuit(vals,&err);
							}
							else
							{
								
								sprintf(EB,"%s:%d, %s references template %s, but",
									   si2drObjectGetFileName(group, &err),
									   si2drObjectGetLineNo(group, &err),
									   si2drGroupGetGroupType(group,&err),
									   strx);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								sprintf(EB,"                  the template's index_1 attribute (line %d) does not have a string as a value.",
									   si2drObjectGetLineNo(ind1, &err)
									);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
						else
						{
							sprintf(EB,"%s:%d, %s references template %s, and only index_1 is defined, but",
								   si2drObjectGetFileName(group, &err),
								   si2drObjectGetLineNo(group, &err),
								   si2drGroupGetGroupType(group,&err),
								   strx);
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							sprintf(EB,"                  the group has more than one value item...."
								   );
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							errcount++;
						}
					}
					else
					{
						sprintf(EB,"%s:%d, %s references template %s, but",
							   si2drObjectGetFileName(group, &err),
							   si2drObjectGetLineNo(group, &err),
							   si2drGroupGetGroupType(group,&err),
							   strx);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						sprintf(EB,"                  the group does not have a values attribute."
							   );
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}

				else if( si2drObjectIsNull(ind1,&err) && !si2drObjectIsNull(ind2,&err) )
				{
					sprintf(EB,"%s:%d, %s references template %s, and index_2 was defined, but index_1 could not be found",
						   si2drObjectGetFileName(group, &err),
						   si2drObjectGetLineNo(group, &err),
						   si2drGroupGetGroupType(group,&err),
						   strx);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					sprintf(EB,"                 in the group or the template (line %d).",
						   si2drObjectGetLineNo(rg2, &err)
						);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				/* else ? */
			}
		}
		else
		{
			/* it could be a good 'ol polynomial */
			
			rg2 = si2drGroupFindGroupByName(rg,strx, "poly_template",&err);
			if( si2drObjectIsNull(rg2,&err) )
			{
				sprintf(EB,"%s:%d, %s's reference to template %s: the referenced group could not be found",
					   si2drObjectGetFileName(group, &err),
					   si2drObjectGetLineNo(group, &err),
					   si2drGroupGetGroupType(group,&err), strx);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
		}
	}
}

static void check_power_lut_template_arraysize( si2drGroupIdT group)
{
	si2drErrorT err;
	si2drNamesIdT names;
	si2drStringT str;
	si2drAttrIdT attr;
	si2drAttrsIdT attrs;
	si2drGroupIdT rg,rg2;
	si2drGroupsIdT groups;

	names = si2drGroupGetNames(group, &err);
	str = si2drIterNextName(names,&err);
	si2drIterQuit(names,&err);
	if( str && str[0] )
	{
		char strx[8000];
		strcpy(strx,str);

		if(  !strcmp(strx,"scalar") )
		{
			si2drAttrIdT vs;
			/* should be just one value */
			vs = si2drGroupFindAttrByName(group, "values", &err);
			
			if( !si2drObjectIsNull(vs,&err) )
			{
				int numstrs = count_strings_in_list(vs);
				int numfloats;
				si2drValuesIdT vals;
				si2drStringT t1s,t2s;
				si2drValueTypeT type;
				si2drInt32T     intgr;
				si2drFloat64T   float64;
				si2drStringT    string;
				si2drBooleanT   bool;
				
				
				vals = si2drComplexAttrGetValues(vs,&err);
				si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
				si2drIterQuit(vals,&err);

				if( type == SI2DR_UNDEFINED_VALUETYPE )
				{
					sprintf(EB,"%s:%d, %s references template %s, so",
						   si2drObjectGetFileName(group, &err),
						   si2drObjectGetLineNo(group, &err),
						   si2drGroupGetGroupType(group,&err),
						   strx);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					sprintf(EB,"                  the values attribute should have one string specified\n                  with a number in it, but does not."
						   );
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				if( type != SI2DR_STRING )
				{
					sprintf(EB,"%s:%d, %s references template %s, so",
						   si2drObjectGetFileName(group, &err),
						   si2drObjectGetLineNo(group, &err),
						   si2drGroupGetGroupType(group,&err),
						   strx);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					sprintf(EB,"                  the values attribute should have one string specified\n                  with a number in it, but what is there is not a string."
						   );
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				else
				{
					numfloats = count_floats_in_string(string);
					if( numfloats != 1 || numstrs != 1 )
					{
						sprintf(EB,"%s:%d, %s references template %s, but",
							   si2drObjectGetFileName(group, &err),
							   si2drObjectGetLineNo(group, &err),
							   si2drGroupGetGroupType(group,&err),
							   strx);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						sprintf(EB,"                  the values attribute has more than one value or string specified."
							   );
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}
			}
			else
			{
				sprintf(EB,"%s:%d, %s references template %s, but",
					   si2drObjectGetFileName(group, &err),
					   si2drObjectGetLineNo(group, &err),
					   si2drGroupGetGroupType(group,&err),
					   strx);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				sprintf(EB,"                  the group does not have a values attribute."
					   );
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}

			return;
		}
		
		/* find an lu_table_template with this name */
		rg = get_containing_group(group,"library");
		rg2 = si2drGroupFindGroupByName(rg,strx, "power_lut_template",&err);
		if( !si2drObjectIsNull(rg2,&err) )
		{
			si2drAttrIdT ind1,ind2, ind3;
			si2drAttrIdT vs;

			ind1 = si2drGroupFindAttrByName(group, "index_1", &err);
			ind2 = si2drGroupFindAttrByName(group, "index_2", &err);
			ind3 = si2drGroupFindAttrByName(group, "index_3", &err);
			
			if( si2drObjectIsNull(ind1,&err) )
				ind1 = si2drGroupFindAttrByName(rg2, "index_1", &err);

			if( si2drObjectIsNull(ind2,&err) )
				ind2 = si2drGroupFindAttrByName(rg2, "index_2", &err);
			
			if( si2drObjectIsNull(ind3,&err) )
				ind3 = si2drGroupFindAttrByName(rg2, "index_3", &err);
			
			vs = si2drGroupFindAttrByName(group, "values", &err);
			
			if ( !si2drObjectIsNull(ind1,&err) && !si2drObjectIsNull(ind2,&err) && !si2drObjectIsNull(ind3,&err)  )
			{
				if( !si2drObjectIsNull(vs,&err) )
				{
					int tpl1nums, tpl2nums, tpl3nums;
					int numstrs = count_strings_in_list(vs);
					si2drValuesIdT vals;
					si2drStringT t1s,t2s;
					si2drValueTypeT type;
					si2drInt32T     intgr;
					si2drFloat64T   float64;
					si2drStringT    string;
					si2drBooleanT   bool;
					
					
					vals = si2drComplexAttrGetValues(ind1,&err);
					si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
					si2drIterQuit(vals,&err);
					
					if( type == SI2DR_STRING )
					{
						tpl1nums = count_floats_in_string(string);
						
						vals = si2drComplexAttrGetValues(ind2,&err);
						si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
						si2drIterQuit(vals,&err);
						
						if( type == SI2DR_STRING )
						{
							tpl2nums = count_floats_in_string(string);

							
							vals = si2drComplexAttrGetValues(ind3,&err);
							si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
							si2drIterQuit(vals,&err);
						
							if( type == SI2DR_STRING )
							{
								
								tpl3nums = count_floats_in_string(string);

								if( numstrs == tpl1nums * tpl2nums)
								{
								/* now, count the nums in each entry, and make sure they equal tpl2nums */
								
									int i;
									vals = si2drComplexAttrGetValues(vs,&err);
									for(i=0;i<numstrs;i++)
									{
										si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
										if( type == SI2DR_STRING )
										{
											int numc = count_floats_in_string(string);
											if( numc != tpl3nums )
											{
												sprintf(EB,"%s:%d, %s references template %s,",
													   si2drObjectGetFileName(group, &err),
													   si2drObjectGetLineNo(group, &err),
													   si2drGroupGetGroupType(group,&err),
													   strx);
												(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
															  EB, 
															  &err);
												sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
													   tpl1nums*tpl2nums, tpl3nums
													   );
												
												(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
															  EB, 
															  &err);
												sprintf(EB,"                  but in the group's values attribute (ending at line %d) value #%d has %d numbers.",
													   si2drObjectGetLineNo(vs, &err),
													   i+1, numc
													   );
												(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
															  EB, 
															  &err);
												errcount++;
											}
										}
										else
										{
											sprintf(EB,"%s:%d, %s references template %s,",
												   si2drObjectGetFileName(group, &err),
												   si2drObjectGetLineNo(group, &err),
												   si2drGroupGetGroupType(group,&err),
												   strx);
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
												   tpl1nums*tpl2nums, tpl3nums
												   );
											
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  but in the group's values attribute (ending at line %d) value #%d is not a string.",
												   si2drObjectGetLineNo(vs, &err),
												   i+1
												   );
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											errcount++;
										}
									}
									si2drIterQuit(vals,&err);
								}
								else
								{
									sprintf(EB,"%s:%d, %s references template %s,",
										   si2drObjectGetFileName(group, &err),
										   si2drObjectGetLineNo(group, &err),
										   si2drGroupGetGroupType(group,&err),
										   strx);
									(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
												  EB, 
												  &err);
									sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
										   tpl1nums*tpl2nums, tpl3nums
										);
									
									(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
												  EB, 
												  &err);
									sprintf(EB,"                  but the group's values attribute (line %d) has %d strings.",
										   si2drObjectGetLineNo(vs, &err),
										   numstrs
										);
									(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
												  EB, 
												  &err);
									errcount++;
								}
							}
							else
							{
								
								sprintf(EB,"%s:%d, %s references template %s, but",
									   si2drObjectGetFileName(group, &err),
									   si2drObjectGetLineNo(group, &err),
									   si2drGroupGetGroupType(group,&err),
									   strx);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								sprintf(EB,"                  the template's index_3 attribute (line %d) does not have a string as a value.",
									   si2drObjectGetLineNo(ind2, &err)
									);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
						else
						{
							
							sprintf(EB,"%s:%d, %s references template %s, but",
								   si2drObjectGetFileName(group, &err),
								   si2drObjectGetLineNo(group, &err),
								   si2drGroupGetGroupType(group,&err),
								   strx);
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							sprintf(EB,"                  the template's index_2 attribute (line %d) does not have a string as a value.",
								   si2drObjectGetLineNo(ind2, &err)
								);
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							errcount++;
						}
					}
					else
					{
						
						sprintf(EB,"%s:%d, %s references template %s, but",
							   si2drObjectGetFileName(group, &err),
							   si2drObjectGetLineNo(group, &err),
							   si2drGroupGetGroupType(group,&err),
							   strx);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						sprintf(EB,"                  the template's index_1 attribute (line %d) does not have a string as a value.",
							   si2drObjectGetLineNo(ind1, &err)
							   );
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}
				else
				{
					sprintf(EB,"%s:%d, %s references template %s, but",
						   si2drObjectGetFileName(group, &err),
						   si2drObjectGetLineNo(group, &err),
						   si2drGroupGetGroupType(group,&err),
						   strx);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					sprintf(EB,"                  the group does not have a values attribute."
						   );
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				
			}
			else if ( !si2drObjectIsNull(ind1,&err) && !si2drObjectIsNull(ind2,&err) && si2drObjectIsNull(ind3,&err)  )
			{
				if( !si2drObjectIsNull(vs,&err) )
				{
					int tpl1nums, tpl2nums;
					int numstrs = count_strings_in_list(vs);
					si2drValuesIdT vals;
					si2drStringT t1s,t2s;
					si2drValueTypeT type;
					si2drInt32T     intgr;
					si2drFloat64T   float64;
					si2drStringT    string;
					si2drBooleanT   bool;
					
					
					vals = si2drComplexAttrGetValues(ind1,&err);
					si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
					si2drIterQuit(vals,&err);
					
					if( type == SI2DR_STRING )
					{
						tpl1nums = count_floats_in_string(string);
						
						vals = si2drComplexAttrGetValues(ind2,&err);
						si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
						si2drIterQuit(vals,&err);
						
						if( type == SI2DR_STRING )
						{
							tpl2nums = count_floats_in_string(string);
							if( numstrs == tpl1nums )
							{
								/* now, count the nums in each entry, and make sure they equal tpl2nums */
								
								int i;
								vals = si2drComplexAttrGetValues(vs,&err);
								for(i=0;i<numstrs;i++)
								{
									si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
									if( type == SI2DR_STRING )
									{
										int numc = count_floats_in_string(string);
										if( numc != tpl2nums )
										{
											sprintf(EB,"%s:%d, %s references template %s,",
												   si2drObjectGetFileName(group, &err),
												   si2drObjectGetLineNo(group, &err),
												   si2drGroupGetGroupType(group,&err),
												   strx);
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
												   tpl1nums, tpl2nums
												   );
											
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  but in the group's values attribute (ending at line %d) value #%d has %d numbers.",
												   si2drObjectGetLineNo(vs, &err),
												   i+1, numc
												   );
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											errcount++;
										}
									}
									else
									{
										sprintf(EB,"%s:%d, %s references template %s,",
											   si2drObjectGetFileName(group, &err),
											   si2drObjectGetLineNo(group, &err),
											   si2drGroupGetGroupType(group,&err),
											   strx);
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
											   tpl1nums, tpl2nums
											   );
										
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										sprintf(EB,"                  but in the group's values attribute (ending at line %d) value #%d is not a string.",
											   si2drObjectGetLineNo(vs, &err),
											   i+1
											   );
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										errcount++;
									}
								}
								si2drIterQuit(vals,&err);
							}
							else
							{
								sprintf(EB,"%s:%d, %s references template %s,",
									   si2drObjectGetFileName(group, &err),
									   si2drObjectGetLineNo(group, &err),
									   si2drGroupGetGroupType(group,&err),
									   strx);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								sprintf(EB,"                  which says there should be %d strings, each with %d numbers in it in the values table,",
									   tpl1nums, tpl2nums
									   );
								
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								sprintf(EB,"                  but the group's values attribute (line %d) has %d strings.",
									   si2drObjectGetLineNo(vs, &err),
									   numstrs
									   );
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
						else
						{
							
							sprintf(EB,"%s:%d, %s references template %s, but",
								   si2drObjectGetFileName(group, &err),
								   si2drObjectGetLineNo(group, &err),
								   si2drGroupGetGroupType(group,&err),
								   strx);
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							sprintf(EB,"                  the template's index_2 attribute (line %d) does not have a string as a value.",
								   si2drObjectGetLineNo(ind2, &err)
								   );
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							errcount++;
						}
					}
					else
					{
						
						sprintf(EB,"%s:%d, %s references template %s, but",
							   si2drObjectGetFileName(group, &err),
							   si2drObjectGetLineNo(group, &err),
							   si2drGroupGetGroupType(group,&err),
							   strx);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						sprintf(EB,"                  the template's index_1 attribute (line %d) does not have a string as a value.",
							   si2drObjectGetLineNo(ind1, &err)
							   );
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}
				else
				{
					sprintf(EB,"%s:%d, %s references template %s, but",
						   si2drObjectGetFileName(group, &err),
						   si2drObjectGetLineNo(group, &err),
						   si2drGroupGetGroupType(group,&err),
						   strx);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					sprintf(EB,"                  the group does not have a values attribute."
						   );
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
			}
			else
			{
				if( !si2drObjectIsNull(ind1,&err) && si2drObjectIsNull(ind2,&err) )
				{
					if( !si2drObjectIsNull(vs,&err) )
					{
						
						int tpl1nums, tpl2nums;
						int numstrs = count_strings_in_list(vs);
						si2drValuesIdT vals;
						si2drStringT t1s,t2s;
						si2drValueTypeT type;
						si2drInt32T     intgr;
						si2drFloat64T   float64;
						si2drStringT    string;
						si2drBooleanT   bool;
						
						if( numstrs == 1 )
						{
							vals = si2drComplexAttrGetValues(ind1,&err);
							si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
							si2drIterQuit(vals,&err);
							
							if( type == SI2DR_STRING )
							{
								int i;
								tpl1nums = count_floats_in_string(string);
							
								/* now, count the nums in each entry, and make sure they equal tpl2nums */
							
								vals = si2drComplexAttrGetValues(vs,&err);
								for(i=0;i<numstrs;i++)
								{
									si2drIterNextComplexValue(vals, &type, &intgr, &float64, &string,&bool,&err);
									if( type == SI2DR_STRING )
									{
										int numc = count_floats_in_string(string);
										if( numc != tpl1nums )
										{
											sprintf(EB,"%s:%d, %s references template %s,",
												   si2drObjectGetFileName(group, &err),
												   si2drObjectGetLineNo(group, &err),
												   si2drGroupGetGroupType(group,&err),
												   strx);
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  which says there should be 1 string, with %d numbers in it in the values table,",
												   tpl1nums
												   );
											
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											sprintf(EB,"                  but in the group's values attribute (ending at line %d) has %d numbers.",
												   si2drObjectGetLineNo(vs, &err),
												   numc
												   );
											(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
														  EB, 
														  &err);
											errcount++;
										}
									}
									else
									{
										sprintf(EB,"%s:%d, %s references template %s,",
											   si2drObjectGetFileName(group, &err),
											   si2drObjectGetLineNo(group, &err),
											   si2drGroupGetGroupType(group,&err),
											   strx);
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										sprintf(EB,"                  which says there should be 1 string, with %d numbers in it in the values table,",
											   tpl1nums
											   );
										
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										sprintf(EB,"                  but in the group's values attribute (ending at line %d) is not a string.",
											   si2drObjectGetLineNo(vs, &err)
											   );
										(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
													  EB, 
													  &err);
										errcount++;
									}
								}
								si2drIterQuit(vals,&err);
							}
							else
							{
								
								sprintf(EB,"%s:%d, %s references template %s, but",
									   si2drObjectGetFileName(group, &err),
									   si2drObjectGetLineNo(group, &err),
									   si2drGroupGetGroupType(group,&err),
									   strx);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								sprintf(EB,"                  the template's index_1 attribute (line %d) does not have a string as a value.",
									   si2drObjectGetLineNo(ind1, &err)
									   );
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
						else
						{
							sprintf(EB,"%s:%d, %s references template %s, and only index_1 is defined, but",
								   si2drObjectGetFileName(group, &err),
								   si2drObjectGetLineNo(group, &err),
								   si2drGroupGetGroupType(group,&err),
								   strx);
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							sprintf(EB,"                  the group has more than one value item...."
								   );
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							errcount++;
						}
					}
					else
					{
						sprintf(EB,"%s:%d, %s references template %s, but",
							   si2drObjectGetFileName(group, &err),
							   si2drObjectGetLineNo(group, &err),
							   si2drGroupGetGroupType(group,&err),
							   strx);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						sprintf(EB,"                  the group does not have a values attribute."
							   );
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}

				else if( si2drObjectIsNull(ind1,&err) && !si2drObjectIsNull(ind2,&err) )
				{
					sprintf(EB,"%s:%d, %s references template %s, and index_2 was defined, but index_1 could not be found",
						   si2drObjectGetFileName(group, &err),
						   si2drObjectGetLineNo(group, &err),
						   si2drGroupGetGroupType(group,&err),
						   strx);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					sprintf(EB,"                 in the group or the template (line %d).",
						   si2drObjectGetLineNo(rg2, &err)
						   );
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				/* else ? */
			}
		}
		else
		{
			/* it could be a good 'ol polynomial */
			
			rg2 = si2drGroupFindGroupByName(rg,strx, "poly_template",&err);
			if( si2drObjectIsNull(rg2,&err) )
			{
				sprintf(EB,"%s:%d, %s's reference to template %s: the referenced group could not be found",
					   si2drObjectGetFileName(group, &err),
					   si2drObjectGetLineNo(group, &err),
					   si2drGroupGetGroupType(group,&err), strx);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
		}
	}
}

static void check_cell_n_prop(si2drGroupIdT group)
{
	si2drErrorT err;
	si2drGroupsIdT gs = si2drGroupGetGroups(group,&err);
	si2drGroupIdT g2;
	group_enum gt;
	int gcell_rise =0, gcell_fall = 0, gprop_fall=0, gprop_rise= 0;
	int gcell_rise_ln, gcell_fall_ln, gprop_fall_ln, gprop_rise_ln;
	char *gcell_rise_fi, *gcell_fall_fi, *gprop_fall_fi, *gprop_rise_fi;
	int gtran_rise = 0, gtran_fall =0;
	int gtran_rise_ln, gtran_fall_ln;
	char *gtran_rise_fi, *gtran_fall_fi;
	
	while(!si2drObjectIsNull((g2=si2drIterNextGroup(gs,&err)),&err) )
	{
		gt = si2drGroupGetID(g2,&err);
		switch( gt )
		{
		case LIBERTY_GROUPENUM_cell_rise:
			gcell_rise++;
			gcell_rise_ln = si2drObjectGetLineNo(g2,&err);
			gcell_rise_fi = si2drObjectGetFileName(g2,&err);
			break;
			
		case LIBERTY_GROUPENUM_cell_fall:
			gcell_fall++;
			gcell_fall_ln = si2drObjectGetLineNo(g2,&err);
			gcell_fall_fi = si2drObjectGetFileName(g2,&err);
			break;
		case LIBERTY_GROUPENUM_rise_propagation:
			gprop_rise++;
			gprop_rise_ln = si2drObjectGetLineNo(g2,&err);
			gprop_rise_fi = si2drObjectGetFileName(g2,&err);
			break;
		case LIBERTY_GROUPENUM_fall_propagation:
			gprop_fall++;
			gprop_fall_ln = si2drObjectGetLineNo(g2,&err);
			gprop_fall_fi = si2drObjectGetFileName(g2,&err);
			break;
		case LIBERTY_GROUPENUM_rise_transition:
			gtran_rise++;
			gtran_rise_ln = si2drObjectGetLineNo(g2,&err);
			gtran_rise_fi = si2drObjectGetFileName(g2,&err);
			break;
		case LIBERTY_GROUPENUM_fall_transition:
			gtran_fall++;
			gtran_fall_ln = si2drObjectGetLineNo(g2,&err);
			gtran_fall_fi = si2drObjectGetFileName(g2,&err);
			break;
		}
	}
	si2drIterQuit(gs,&err);
	if( gcell_rise && gprop_fall )
	{
		sprintf(EB,"%s:%d and %s:%d: It is illegal to specify both cell_rise and fall_propagation in the same timing group.",
			   gcell_rise_fi, gcell_rise_ln, gprop_fall_fi, gprop_fall_ln);
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
	}
	if( gcell_fall && gprop_rise )
	{
		sprintf(EB,"%s:%d and %s:%d: It is illegal to specify both cell_fall and rise_propagation in the same timing group.",
			   gcell_fall_fi, gcell_fall_ln, gprop_rise_fi, gprop_rise_ln);
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
	}
	if( gcell_rise && !gtran_rise )
	{
		sprintf(EB,"%s:%d: It is illegal to specify cell_rise without a rise_transition in the same timing group.",
			   gcell_rise_fi,gcell_rise_ln);
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
	}
	if( gcell_fall && !gtran_fall )
	{
		sprintf(EB,"%s:%d: It is illegal to specify cell_fall without a fall_transition in the same timing group.",
			   gcell_fall_fi,gcell_fall_ln);
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
	}
	
}

static void check_clear_n_preset(si2drGroupIdT group)
{
	si2drErrorT err;
	si2drGroupsIdT gs;
	si2drGroupIdT g2;
	group_enum gt;

	si2drAttrIdT clear, preset, var1, var2;

	clear = si2drGroupFindAttrByName(group, "clear", &err);
	preset = si2drGroupFindAttrByName(group, "preset", &err);

	if( !si2drObjectIsNull(clear,&err) && !si2drObjectIsNull(preset,&err) )
	{
		var1 = si2drGroupFindAttrByName(group, "clear_preset_var1", &err);
		var2 = si2drGroupFindAttrByName(group, "clear_preset_var2", &err);

		if( si2drObjectIsNull(var1,&err) && si2drObjectIsNull(var2,&err) )
		{
			sprintf(EB,"%s:%d, Group %s, with both 'clear', and 'preset' attributes,\nrequires 'clear_preset_var1' and/or 'clear_preset_var2' attributes.",
				   si2drObjectGetFileName(group,&err),
				   si2drObjectGetLineNo(group,&err),
				   si2drGroupGetGroupType(group,&err));
			(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
						  EB, 
						  &err);
			errcount++;
		}
	}
}

static void check_members( si2drGroupIdT group)
{
	/* is members specified? Is it first? */
	si2drErrorT err;
	si2drAttrIdT mem = si2drGroupFindAttrByName(group,"members",&err);
	int faln, fgln, mln;

	if( !si2drObjectIsNull(mem,&err) )
	{
		/* a member attr is present */
		si2drAttrsIdT attrs;
		si2drAttrIdT  attr;
		si2drGroupsIdT groups;
		si2drGroupIdT g2;

		mln = si2drObjectGetLineNo(mem,&err);
	
		attrs = si2drGroupGetAttrs(group,&err);
		attr = si2drIterNextAttr(attrs,&err);
		si2drIterQuit(attrs,&err);

		if( !si2drObjectIsNull(attr,&err) )
		{
			faln = si2drObjectGetLineNo(attr,&err);
		}
		else
			faln = -1;
		
		groups = si2drGroupGetGroups(group,&err);
		g2 = si2drIterNextGroup(groups,&err);
		si2drIterQuit(groups,&err);
		
		if( !si2drObjectIsNull(g2,&err) )
		{
			fgln = si2drObjectGetLineNo(g2,&err);
		}
		else
			fgln = -1;
			
		if( !si2drObjectIsSame(mem,attr,&err) || (fgln > 0 && fgln < mln) )
		{
			si2drNamesIdT names;
			si2drStringT name, attrname;

			names = si2drGroupGetNames(group,&err);
			name = si2drIterNextName(names,&err);
			si2drIterQuit(names,&err);

			if( !name && !name[0] )
				name = "<Unnamed>";

			if( fgln > 0 && fgln < mln )
			{
				si2drNamesIdT gnames;
				si2drStringT gname;
				
				gnames = si2drGroupGetNames(g2,&err);
				gname = si2drIterNextName(gnames,&err);
				si2drIterQuit(gnames,&err);

				sprintf(EB,"%s:%d, The %s bundle needs to have its 'members' specified first.\nThe group %s (line %d) is found before 'members'.",
					   si2drObjectGetFileName(mem,&err),
					   si2drObjectGetLineNo(mem,&err),
					   name,
					   gname, fgln
					   );
				
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
			}
			if( !si2drObjectIsSame( mem,attr, &err) )
			{
				attrname = si2drAttrGetName(attr,&err);
				
				sprintf(EB,"%s:%d, The %s bundle needs to have its 'members' specified first.\nThe attr %s (line %d) is found before 'members'.",
					   si2drObjectGetFileName(mem,&err),
					   si2drObjectGetLineNo(mem,&err),
					   name,
					   attrname, faln
					   );
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
			}
			errcount++;
		}
	}
}

static void check_interface_timing(si2drGroupIdT group)
{
	si2drErrorT err;
	si2drAttrIdT ift;
	si2drGroupIdT rg,rg2;
	si2drGroupsIdT groups;
	si2drGroupIdT g2;
	group_enum gt;
	si2drNamesIdT names;
	si2drStringT name;
	int gln,iftn;
	si2drAttrIdT  attr;
	
	names = si2drGroupGetNames(group,&err);
	name = si2drIterNextName(names,&err);
	si2drIterQuit(names,&err);

	ift = si2drGroupFindAttrByName(group, "interface_timing", &err);
	if( !si2drObjectIsNull(ift,&err) )
	{
		si2drBooleanT iftv = si2drSimpleAttrGetBooleanValue(ift,&err);
		if( err == SI2DR_NO_ERROR && iftv == SI2DR_TRUE )
		{
			iftn = si2drObjectGetLineNo(ift,&err);
			
			/* check to make sure the stuff is as it should be... */
			groups = si2drGroupGetGroups(group,&err);
			
			while(!si2drObjectIsNull((g2=si2drIterNextGroup(groups,&err)),&err) )
			{
				gt = si2drGroupGetID(g2,&err);
				switch( gt )
				{
				case LIBERTY_GROUPENUM_ff:
					sprintf(EB,"%s:%d, The cell '%s' has the interface_timing attribute (line %d), but is not a black box.",
						   si2drObjectGetFileName(g2,&err),
						   si2drObjectGetLineNo(g2,&err),
						   name,
						   iftn);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					sprintf(EB,"                  -- The ff group should not be specified when interface_timing is true.");
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					break;
					
				case LIBERTY_GROUPENUM_pin:
					attr = si2drGroupFindAttrByName(g2, "function", &err);
					if( !si2drObjectIsNull(attr,&err) )
					{
						sprintf(EB,"%s:%d, The cell '%s' has the interface_timing attribute (line %d), but is not a black box.",
							   si2drObjectGetFileName(attr,&err),
							   si2drObjectGetLineNo(attr,&err),
							   name,
							   iftn);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						sprintf(EB,"                  -- pin groups should not have the function attribute specified when interface_timing is true.");
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
					break;
				}
			}
			si2drIterQuit(groups,&err);
		}
	}
}

si2drStringT get_first_group_name(si2drGroupIdT group)
{
	si2drErrorT err;
	si2drStringT str;
	si2drNamesIdT names;
	
	names = si2drGroupGetNames(group, &err);
	str = si2drIterNextName(names,&err);
	si2drIterQuit(names,&err);

	return str;
}

static void check_bus_pin_directions(si2drGroupIdT group) /* group is bus or bundle */
{
	si2drErrorT err;
	si2drAttrIdT bdir, pdir;
	group_enum gt;
	char buf[100];
	si2drStringT dirstr;
	
	bdir = si2drGroupFindAttrByName(group, "direction", &err);

	if( !si2drObjectIsNull(bdir,&err) )
	{
		si2drGroupIdT g2;
		si2drGroupsIdT gs = si2drGroupGetGroups(group,&err);

		dirstr = si2drSimpleAttrGetStringValue(bdir,&err);
		if( !dirstr || !dirstr[0] )
			return;

		strcpy(buf,dirstr);
		
		while ( !si2drObjectIsNull((g2 = si2drIterNextGroup(gs,&err)),&err) )
		{
			gt = si2drGroupGetID(g2,&err);
			
			if( gt == LIBERTY_GROUPENUM_pin )
			{
				pdir = si2drGroupFindAttrByName(g2, "direction", &err);

				if( !si2drObjectIsNull(pdir,&err) )
				{
					dirstr = si2drSimpleAttrGetStringValue(pdir,&err);

					if( !dirstr || !dirstr[0] )
						continue;

					if( strcmp(buf,dirstr) )
					{
						sprintf(EB,"%s:%d, The group '%s' has the direction attribute (line %d) '%s', but",
							   si2drObjectGetFileName(pdir,&err),
							   si2drObjectGetLineNo(pdir,&err),
							   get_first_group_name(group),
							   si2drObjectGetLineNo(bdir,&err),
							   buf);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						sprintf(EB,"                  the pin '%s' has the conflicting direction attribute '%s'.",
							   get_first_group_name(g2),
							   dirstr
							   );
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}
			}
		}
		si2drIterQuit(gs,&err);
	}
}

static void check_driver_types(si2drGroupIdT group) /* group should be a pin, perhaps even bus or bundle  */
{
	int pullup = 0, pulldown=0, bushold=0, opensource=0, opendrain=0, res =0, res1=0, res0=0;
	si2drAttrIdT drivertype;
	si2drAttrIdT direct;
	si2drErrorT err;
	si2drStringT str;
	

	drivertype = si2drGroupFindAttrByName(group,"driver_type",&err);
	
	if( si2drObjectIsNull(drivertype,&err) ) /* if nothing's there, nothing could be wrong! */
		return;

	if( si2drAttrGetAttrType(drivertype,&err) != SI2DR_SIMPLE )
		return;

	if( si2drSimpleAttrGetValueType(drivertype,&err) != SI2DR_STRING )
		return;

	str = si2drSimpleAttrGetStringValue(drivertype,&err);

	if( !str || !str[0] )
		return;
	
	/* we can do checks now on the drivertype combinations */

	if( strstr(str,"pull_up") )
		pullup = 1;
	if( strstr(str,"pull_down") )
		pulldown = 1;
	if( strstr(str,"open_source") )
		opensource = 1;
	if( strstr(str,"open_drain") )
		opendrain = 1;
	if( strstr(str,"bus_hold") )
		bushold = 1;

	if( pullup && pulldown )
	{
		sprintf(EB,"%s:%d, The driver_types 'pull_up' and 'pull_down' cannot be specified on the same pin",
			   si2drObjectGetFileName(drivertype,&err),
			   si2drObjectGetLineNo(drivertype,&err));
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
	}
	if( pullup && bushold )
	{
		sprintf(EB,"%s:%d, The driver_types 'pull_up' and 'bus_hold' cannot be specified on the same pin",
			   si2drObjectGetFileName(drivertype,&err),
			   si2drObjectGetLineNo(drivertype,&err));
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
	}
	if( pulldown && bushold )
	{
		sprintf(EB,"%s:%d, The driver_types 'pull_down' and 'bus_hold' cannot be specified on the same pin",
			   si2drObjectGetFileName(drivertype,&err),
			   si2drObjectGetLineNo(drivertype,&err));
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
	}
	if( opendrain && opensource )
	{
		sprintf(EB,"%s:%d, The driver_types 'open_source' and 'open_drain' cannot be specified on the same pin",
			   si2drObjectGetFileName(drivertype,&err),
			   si2drObjectGetLineNo(drivertype,&err));
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
	}
	if( opendrain && bushold )
	{
		sprintf(EB,"%s:%d, The driver_types 'open_drain' and 'bus_hold' cannot be specified on the same pin",
			   si2drObjectGetFileName(drivertype,&err),
			   si2drObjectGetLineNo(drivertype,&err));
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
	}
	if( opensource && bushold )
	{
		sprintf(EB,"%s:%d, The driver_types 'open_source' and 'bus_hold' cannot be specified on the same pin",
			   si2drObjectGetFileName(drivertype,&err),
			   si2drObjectGetLineNo(drivertype,&err));
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
	}

	if( opensource || opendrain )
	{
		direct = si2drGroupFindAttrByName(group,"direction",&err);
		
		if( si2drObjectIsNull(direct,&err) ) /* if nothing's there, nothing could be wrong! */
			return;
		
		if( si2drAttrGetAttrType(direct,&err) != SI2DR_SIMPLE )
			return;
		
		if( si2drSimpleAttrGetValueType(direct,&err) != SI2DR_STRING )
			return;
		
		str = si2drSimpleAttrGetStringValue(direct,&err);
		
		if( !str || !str[0] )
			return;
	
		if( !strcmp( str, "input" ) )
		{
			if( opensource )
			{
				sprintf(EB,"%s:%d, The 'open_source' driver_type (line %d) cannot be specified on an input pin",
					   si2drObjectGetFileName(direct,&err),
					   si2drObjectGetLineNo(direct,&err),
					   si2drObjectGetLineNo(drivertype,&err));
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
			else
			{
				sprintf(EB,"%s:%d, The 'open_drain' driver_type (line %d) cannot be specified on an input pin",
					   si2drObjectGetFileName(direct,&err),
					   si2drObjectGetLineNo(direct,&err),
					   si2drObjectGetLineNo(drivertype,&err));
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
		}
	}
}

static int get_bus_size(si2drGroupIdT group, char *busname)
{
	si2drErrorT err;
	si2drGroupIdT bus = si2drGroupFindGroupByName(group, busname, "bus", &err);
	
	if( !si2drObjectIsNull(bus,&err) )
	{
		si2drAttrIdT bustype = si2drGroupFindAttrByName(bus, "bus_type", &err);
		si2drGroupIdT typeb, libr;
		si2drAttrIdT bitfrom, bitto, bitwid;
		si2drInt32T wid = -1;
		if( !si2drObjectIsNull(bustype,&err) )
		{
			si2drStringT str;

			if( si2drAttrGetAttrType(bustype,&err) != SI2DR_SIMPLE )
			{
				return -1; /* this will already be reported */
			}
			
			if( si2drSimpleAttrGetValueType(bustype,&err) != SI2DR_STRING )
			{
				return -1;
			}
			
			str = si2drSimpleAttrGetStringValue(bustype,&err);
			
			if( !str || !str[0] )
			{
				return -1;
			}
			
			/* find the type in the cell or lib */
			typeb = si2drGroupFindGroupByName(group, str, "type", &err);
			if( si2drObjectIsNull(typeb,&err) )
			{
				libr = get_containing_group(group,"library");
				if( !si2drObjectIsNull(libr,&err) )
				{
					typeb = si2drGroupFindGroupByName(libr, str, "type", &err);
				}
			}
			
			if( si2drObjectIsNull(typeb,&err) )
			{
				return -1;
			}
			
			bitwid = si2drGroupFindAttrByName(typeb, "bit_width", &err);
			if( !si2drObjectIsNull(bitwid, &err) )
			{
				si2drValueTypeT vt;
				
				if( si2drAttrGetAttrType(bitwid,&err) != SI2DR_SIMPLE )
				{
					return -1; /* this will already be reported */
				}
				vt = si2drSimpleAttrGetValueType(bitwid,&err);
				
				if( vt == SI2DR_STRING )
				{
					return -1;
				}
				if( vt == SI2DR_INT32 )
				{
					wid = si2drSimpleAttrGetInt32Value(bitwid,&err);
				}
				else if (vt == SI2DR_FLOAT64 )
				{
					wid = si2drSimpleAttrGetFloat64Value(bitwid,&err);
				}
			}
			else
			{
				bitfrom = si2drGroupFindAttrByName(typeb, "bit_from", &err);
				bitto = si2drGroupFindAttrByName(typeb, "bit_to", &err);
				if( !si2drObjectIsNull(bitfrom,&err) && !si2drObjectIsNull(bitto,&err) )
				{
					si2drValueTypeT vt;
					int from,to,big,lit;
					
					if( si2drAttrGetAttrType(bitfrom,&err) != SI2DR_SIMPLE )
					{
						return -1; /* this will already be reported */
					}
					vt = si2drSimpleAttrGetValueType(bitfrom,&err);
					
					if( vt == SI2DR_STRING )
					{
						return -1;
					}
					if( vt == SI2DR_INT32 )
					{
						from = si2drSimpleAttrGetInt32Value(bitfrom,&err);
					}
					else if (vt == SI2DR_FLOAT64 )
					{
						from = si2drSimpleAttrGetFloat64Value(bitfrom,&err);
					}
					
					
					if( si2drAttrGetAttrType(bitto,&err) != SI2DR_SIMPLE )
					{
						return -1; /* this will already be reported */
					}
					vt = si2drSimpleAttrGetValueType(bitto,&err);
					
					if( vt == SI2DR_STRING )
					{
						return -1;
					}
					if( vt == SI2DR_INT32 )
					{
						to = si2drSimpleAttrGetInt32Value(bitto,&err);
					}
					else if (vt == SI2DR_FLOAT64 )
					{
						to = si2drSimpleAttrGetFloat64Value(bitto,&err);
					}
					big = (from > to ? from : to);
					lit = (from > to ? to : from);
					
					wid = big - lit +1;
				}
				else
					return -1;
			}
		}
		return wid;
	}
	return -1;
}

static int get_bun_size(si2drGroupIdT group, char *bun)
{
	si2drErrorT err;
	si2drGroupIdT bus = si2drGroupFindGroupByName(group, bun, "bundle", &err);
	if( !si2drObjectIsNull(bus, &err) )
	{
		si2drAttrIdT bunmem = si2drGroupFindAttrByName(bus, "members", &err);
		si2drInt32T wid;
		
		if( !si2drObjectIsNull(bunmem,&err) )
		{
			return count_strings_in_list(bunmem);
		}
	}
	return -1;
}


static void gen_var_list(char *formula)
{
	/* a very crude boolean algebra formula parser */
	char *p,*q;
	char varname[100];

	
	p = formula;
	q = varname;
	while( *p )
	{
		/* skip over any non-name chars */
		while( *p  && (*p =='(' || *p == ')' || *p == ' ' 
				|| *p =='\t' || *p =='&' || *p == '|' || *p == '*' 
				|| *p =='!' || *p == '^' || *p == '+' || *p == '\'' ))
			p++;

		while( *p  && (*p !='(' && *p != ')' && *p != ' ' && *p != '+' && *p != '\''
				&& *p !='\t' && *p !='&' && *p !='|' && *p !='*' && *p != '^' && *p != '!') )
			*q++ = *p++;

		*q = 0;
		if( varname[0] )
		{
			add_varlist(varname);
		}
		q = varname;
	}
}

static si2drGroupIdT find_bu_by_function( si2drGroupIdT cellequiv, char *name )
{
	si2drErrorT err;
	si2drGroupsIdT gs = si2drGroupGetGroups(cellequiv,&err);
	si2drGroupIdT g2;
	si2drAttrIdT attr;
	si2drValueTypeT vtype;
	si2drStringT    string = 0;
	si2drAttrTypeT atype;
	
	group_enum gt;
	
	while(!si2drObjectIsNull((g2=si2drIterNextGroup(gs,&err)),&err) )
	{
		gt = si2drGroupGetID(g2,&err);
		switch( gt )
		{
		case LIBERTY_GROUPENUM_bus:
		case LIBERTY_GROUPENUM_bundle:
			attr = si2drGroupFindAttrByName(g2, "function", &err);
			
			if( !si2drObjectIsNull(attr,&err) )
			{
				atype = si2drAttrGetAttrType(attr,&err);
				
				if( atype != SI2DR_COMPLEX )
				{
					
					vtype = si2drSimpleAttrGetValueType(attr,&err);
					switch( vtype )
					{
					case SI2DR_STRING:
						string = si2drSimpleAttrGetStringValue(attr,&err);
						if( !strcasecmp(name, string) )
						{
							si2drIterQuit(gs,&err);
							return g2;
						}
						break;
					}
				}
			}
			break;
		}
	}
	si2drIterQuit(gs,&err);
	return si2drPIGetNullId(&err);
}

static si2drGroupIdT find_pin_in_cellequiv( si2drGroupIdT cellequiv, char *name )
{
	si2drErrorT err;
	si2drGroupsIdT gs = si2drGroupGetGroups(cellequiv,&err);
	si2drGroupIdT g2;
	si2drAttrIdT attr;
	si2drValueTypeT vtype;
	si2drStringT    string = 0;
	si2drAttrTypeT atype;
	int found = 0;
	si2drNamesIdT names;
	si2drStringT str;
	
	group_enum gt;
	
	while(!si2drObjectIsNull((g2=si2drIterNextGroup(gs,&err)),&err) )
	{
		gt = si2drGroupGetID(g2,&err);
		switch( gt )
		{
		case LIBERTY_GROUPENUM_pin:
			names = si2drGroupGetNames(g2, &err);
			while( (str = si2drIterNextName(names,&err)) )
			{
				if( !strcmp(name, str) )
				{
					found++;
					break;
				}
			}
			si2drIterQuit(names,&err);

			if( found )
			{
				si2drIterQuit(gs,&err);
				return g2;
			}
			break;
		}
	}
	si2drIterQuit(gs,&err);
	return si2drPIGetNullId(&err);
}


static void check_next_state_formula( si2drGroupIdT cell, char *formula, int ffbankwidth, si2drAttrIdT attr )
{
	si2drErrorT err;
	var_list *v;
	si2drGroupIdT bus;
	
	destroy_varlist();
	gen_var_list(formula);

	for(v=master_var_list; v; v=v->next)
	{
		/* assume: every variable is either a bus or bundle;
		   a variable may match the function of a bus or bundle;
		   the width of the bus must match the width of the ff_bank */
		/* first, try to match the bus or bun */
		
		bus = si2drGroupFindGroupByName(cell, v->name, "bus", &err);
			
		if( si2drObjectIsNull(bus, &err) )
		{
			
			bus = si2drGroupFindGroupByName(cell, v->name, "bundle", &err);
			if( !si2drObjectIsNull(bus, &err) )
			{
				si2drAttrIdT bunmem = si2drGroupFindAttrByName(bus, "members", &err);
				si2drInt32T wid;
				
				wid = get_bun_size(cell, v->name);
				
				if( wid == -1 )
				{
					sprintf(EB,"%s:%d, The 'members' attribute was not found in the bundle. Cannot make bus width correspondence checks.",
						   si2drObjectGetFileName(bus,&err),
						   si2drObjectGetLineNo(bus,&err));
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					continue;
				}
				else
				{
					if( wid != ffbankwidth )
					{
						sprintf(EB,"%s:%d, This ff_bank, with width %d, does not match the referenced bundle %s (line %d), whose width is %d",
							   si2drObjectGetFileName(attr,&err),
							   si2drObjectGetLineNo(attr,&err),
							   ffbankwidth,
							   v->name,
							   si2drObjectGetLineNo(bus,&err),
							   wid
							   );
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
						continue;
					}
				}
				continue;
			}
		}
		
		if( !si2drObjectIsNull(bus, &err) )
		{
			si2drInt32T wid = get_bus_size(cell, v->name);
			
			if( wid != ffbankwidth )
			{
				sprintf(EB,"%s:%d, This ff_bank, with width %d, does not match the referenced bus %s (line %d), whose width is %d",
					   si2drObjectGetFileName(attr,&err),
					   si2drObjectGetLineNo(attr,&err),
					   ffbankwidth,
					   v->name,
					   si2drObjectGetLineNo(bus,&err),
					   wid
					);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
				continue;
			}
		}
		else
		{
			si2drNamesIdT names;
			group_enum gt;
			si2drStringT str;
			si2drInt32T wid;
			si2drGroupIdT pinn;

			bus = find_bu_by_function( cell, v->name );

			if( si2drObjectIsNull(bus,&err) )
			{
				pinn = find_pin_in_cellequiv(cell, v->name);
				
				if( si2drObjectIsNull(pinn,&err) )
				{
					sprintf(EB,"%s:%d, This ff_bank, with width %d, references the object %s, which I cannot find.",
						   si2drObjectGetFileName(attr,&err),
						   si2drObjectGetLineNo(attr,&err),
						   ffbankwidth,
						   v->name
						);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					continue;
				}
				else
					continue; /* not an error */
			}
			
			names = si2drGroupGetNames(bus, &err);
			str = si2drIterNextName(names,&err);
			si2drIterQuit(names,&err);
			if( str )
			{
				gt = si2drGroupGetID(bus,&err);
				switch( gt )
				{
				case LIBERTY_GROUPENUM_bus:
					wid = get_bus_size(cell, str);
					break;
					
				case LIBERTY_GROUPENUM_bundle:
					wid = get_bun_size(cell, str);
					break;
				}
				if( wid != ffbankwidth )
				{
					sprintf(EB,"%s:%d, This ff_bank, with width %d, does not match the referenced bus %s with function %s (line %d), whose width is %d",
						   si2drObjectGetFileName(attr,&err),
						   si2drObjectGetLineNo(attr,&err),
						   ffbankwidth,
						   str, v->name,
						   si2drObjectGetLineNo(bus,&err),
						   wid
						   );
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					continue;
				}
			}
		}
	}
	destroy_varlist();
}


static void check_ff_bank_widths(si2drGroupIdT group) /* called for ff_banks */
{
	si2drErrorT err;
	si2drStringT str, str1,str2,str3;
	si2drNamesIdT names;
	int ffbankwidth= 0;
	si2drAttrIdT attr;
	si2drGroupIdT cell;
	char busnam[1000];
	
	names = si2drGroupGetNames(group, &err);
	str = si2drIterNextName(names,&err);
	if( str )
	{
		str = si2drIterNextName(names,&err);
		if( str )
		{
			str = si2drIterNextName(names,&err);
			if( str )
			{
				ffbankwidth = atoi(str);
			}
		}
	}
	si2drIterQuit(names,&err);

	if( ffbankwidth == 0 )
	{
		sprintf(EB,"%s:%d, Could not retrieve ff_bank width from file, or it has value 0. Cannot make bus width correspondence checks.",
			   si2drObjectGetFileName(group,&err),
			   si2drObjectGetLineNo(group,&err));
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
		return;
	}

	attr = si2drGroupFindAttrByName(group,"next_state",&err);

	if ( !si2drObjectIsNull(attr,&err) )
	{
		if( si2drAttrGetAttrType(attr,&err) != SI2DR_SIMPLE )
		{
			return; /* this will already be reported */
		}
		
		if( si2drSimpleAttrGetValueType(attr,&err) != SI2DR_STRING )
		{
			sprintf(EB,"%s:%d, The 'next_state' attribute is not of string type. Cannot make bus width correspondence checks.",
				   si2drObjectGetFileName(attr,&err),
				   si2drObjectGetLineNo(attr,&err));
			(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
						  EB, 
						  &err);
			errcount++;
			return;
		}
		
		str = si2drSimpleAttrGetStringValue(attr,&err);
		
		if( !str || !str[0] )
		{
			sprintf(EB,"%s:%d, The 'next_state' attribute is an empty string. Cannot make bus width correspondence checks.",
				   si2drObjectGetFileName(attr,&err),
				   si2drObjectGetLineNo(attr,&err));
			(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
						  EB, 
						  &err);
			errcount++;
			return;
		}
	
		/* go find this referenced bus */

		strcpy(busnam,str);
		
		cell = get_containing_group(group,"test_cell");
		if( si2drObjectIsNull(cell,&err) )
			cell = get_containing_group(group,"model");
		if( si2drObjectIsNull(cell,&err) )
			cell = get_containing_group(group,"scaled_cell");
		if( si2drObjectIsNull(cell,&err) )
			cell = get_containing_group(group,"cell");
		

		if( !si2drObjectIsNull(cell, &err) )
		{
			check_next_state_formula( cell, busnam, ffbankwidth, attr );
		}
	}
	return;
}

static void check_bus_type( si2drGroupIdT bus )
{
	si2drErrorT err;
	si2drStringT str, str1,str2,str3;
	si2drNamesIdT names;
	int ffbankwidth= 0;
	si2drAttrIdT attr;
	si2drGroupIdT cell;
	char busnam[1000];
	si2drAttrIdT bustype = si2drGroupFindAttrByName(bus, "bus_type", &err);
	si2drGroupIdT typeb, libr;
	si2drAttrIdT bitfrom, bitto, bitwid;
	si2drInt32T wid;
	
	if( !si2drObjectIsNull(bustype,&err) )
	{
		if( si2drAttrGetAttrType(bustype,&err) != SI2DR_SIMPLE )
		{
			return; /* this will already be reported */
		}
		
		if( si2drSimpleAttrGetValueType(bustype,&err) != SI2DR_STRING )
		{
			return;
		}
		
		str = si2drSimpleAttrGetStringValue(bustype,&err);
		
		if( !str || !str[0] )
		{
			sprintf(EB,"%s:%d, The 'bus_type' attribute is an empty string.",
				   si2drObjectGetFileName(bustype,&err),
				   si2drObjectGetLineNo(bustype,&err));
			(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
						  EB, 
						  &err);
			errcount++;
			return;
		}
		
		/* find the type in the cell or lib */
		cell = get_containing_group(bus,"test_cell");
		if( si2drObjectIsNull(cell,&err) )
			cell = get_containing_group(bus,"model");
		if( si2drObjectIsNull(cell,&err) )
			cell = get_containing_group(bus,"scaled_cell");
		if( si2drObjectIsNull(cell,&err) )
			cell = get_containing_group(bus,"cell");
		
		typeb = si2drGroupFindGroupByName(cell, str, "type", &err);

		if( si2drObjectIsNull(typeb,&err) )
		{
			libr = get_containing_group(bus,"library");
			if( !si2drObjectIsNull(libr,&err) )
			{
				typeb = si2drGroupFindGroupByName(libr, str, "type", &err);
			}
		}
		
		if( si2drObjectIsNull(typeb,&err) )
		{
			sprintf(EB,"%s:%d, The 'bus_type' references the type '%s', which cannot be found in the containing cell or library.",
				   si2drObjectGetFileName(bustype,&err),
				   si2drObjectGetLineNo(bustype,&err), str);
			(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
						  EB, 
						  &err);
			errcount++;
			return;
		}
	}
	else
	{
		sprintf(EB,"%s:%d, The 'bus_type' attribute is missing in bus '%s'.",
			   si2drObjectGetFileName(bus,&err),
			   si2drObjectGetLineNo(bus,&err),
			   get_first_group_name(bus));
		(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
					  EB, 
					  &err);
		errcount++;
		return;
	}
	
}

static void find_all_refs_to_template(si2drGroupIdT group, char *name, int num )
{
	si2drErrorT err;
	si2drGroupsIdT gs = si2drGroupGetGroups(group,&err);
	si2drGroupIdT g2;
	group_enum gt;
	si2drNamesIdT names;
	si2drStringT str;
	
	while(!si2drObjectIsNull((g2=si2drIterNextGroup(gs,&err)),&err) )
	{
		if( !strcmp(si2drGroupGetGroupType(g2,&err), "lu_table_template") )
			continue;
		
		names = si2drGroupGetNames(g2, &err);
		while( (str = si2drIterNextName(names,&err)) )
		{
			if( !strcmp(name, str) )
			{
				if( !num )
				{
					sprintf(EB,"         Here is a list of references to this invalid template:");
					(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
				}

				num++;
				sprintf(EB,"           %s:%d, group '%s'",
					   si2drObjectGetFileName(g2,&err),
					   si2drObjectGetLineNo(g2,&err),
					   si2drGroupGetGroupType(g2,&err));
				(*MsgPrinter)(SI2DR_SEVERITY_NOTE, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				break;
			}
		}
		si2drIterQuit(names,&err);

		find_all_refs_to_template(g2,name,num); /* recursively descend */
		
	}
	si2drIterQuit(gs,&err);
}


static void check_index_x(si2drStringT indnam, si2drGroupIdT group)
{
	si2drErrorT err;
	si2drStringT str, str1,str2,str3;
	si2drNamesIdT names;
	si2drAttrIdT attr;
	si2drGroupIdT lib;
	char busnam[1000];
	si2drAttrIdT index_x = si2drGroupFindAttrByName(group, indnam, &err);
	si2drGroupIdT typeb, libr;
	si2drValuesIdT vals;
	si2drValueTypeT vtype;
	si2drInt32T     intgr;
	si2drFloat64T   float64;
	si2drStringT    string;
	si2drBooleanT   bool;
	char *p,*t;
	double x;
	if( !si2drObjectIsNull(index_x,&err) )
	{
		int inval;
		
		/* should be a string with a list of comma/space sep'd numbers */
		if( si2drAttrGetAttrType(index_x,&err) != SI2DR_COMPLEX )
		{
			return; /* this will already be reported */
		}
		
		vals = si2drComplexAttrGetValues(index_x,&err);
		si2drIterNextComplexValue(vals,&vtype, &intgr, &float64, &string, &bool, &err);
		si2drIterQuit(vals,&err);
		
		if( vtype == SI2DR_UNDEFINED_VALUETYPE )
		{
			sprintf(EB,"%s:%d, The '%s' attribute has no values.",
				   si2drObjectGetFileName(index_x,&err),
				   si2drObjectGetLineNo(index_x,&err),
				   indnam);
			(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
						  EB, 
						  &err);
			errcount++;
			return;
		}
		
		if( vtype != SI2DR_STRING )
		{
			sprintf(EB,"%s:%d, The '%s' attribute should be a string.",
				   si2drObjectGetFileName(index_x,&err),
				   si2drObjectGetLineNo(index_x,&err),
				   indnam);
			(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
						  EB, 
						  &err);
			errcount++;
			return;
		}
		
		if( !string || !string[0] )
		{
			sprintf(EB,"%s:%d, The '%s' attribute is an empty string.",
				   si2drObjectGetFileName(index_x,&err),
				   si2drObjectGetLineNo(index_x,&err),
				   indnam);
			(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
						  EB, 
						  &err);
			errcount++;
			return;
		}
	
		p = string;

		inval = 0;
		
		while( (x = strtod(p,&t)), t != p )
		{
			if( x < 0.0 )
			{
				sprintf(EB,"%s:%d, The '%s' attribute contains the negative value '%g'. The template '%s' is therefore invalid.",
					   si2drObjectGetFileName(index_x,&err),
					   si2drObjectGetLineNo(index_x,&err),
					   indnam,x,get_first_group_name(group));
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
				inval = 1;
			}

			if( t && *t == 0 )
				break;
			
			p = t+1;
		}

		if( inval )
		{
			char buf[1000];
			strcpy(buf,get_first_group_name(group)); /* do this because si2dr strings are volatile.  */
			
			lib = get_containing_group(group,"library");
			find_all_refs_to_template(lib, buf, 0);
		}
	}
}

static void check_lut_template( si2drGroupIdT group )
{
	
	check_index_x("index_1", group);
	check_index_x("index_2", group);
	
}

	
static void group_specific_checks( si2drGroupIdT group )
{
	si2drErrorT err;
	si2drStringT name = si2drGroupGetGroupType(group,&err);
	const struct libGroupMap *x = lookup_group_name (name, strlen(name));
	group_enum grouptype = x->type;
	int i, nc;
	si2drNamesIdT names;
	si2drStringT str;
	si2drAttrIdT attr;
	si2drAttrsIdT attrs;
	si2drGroupIdT rg,rg2;
	si2drGroupsIdT groups;
	
	switch( grouptype )
	{
	case LIBERTY_GROUPENUM_lut:
		break;

	case LIBERTY_GROUPENUM_wire_load_table:
		break;

	case LIBERTY_GROUPENUM_wire_load_selection:
		break;

	case LIBERTY_GROUPENUM_interconnect_delay:
		break;

	case LIBERTY_GROUPENUM_wire_load:
		break;

	case LIBERTY_GROUPENUM_timing_range:
		break;

	case LIBERTY_GROUPENUM_scaling_factors:
		break;

	case LIBERTY_GROUPENUM_scaled_cell:
		break;

	case LIBERTY_GROUPENUM_rise_transition_degradation:
		check_lu_table_template_arraysize(group);
		break;

	case LIBERTY_GROUPENUM_rise_net_delay:
		break;

	case LIBERTY_GROUPENUM_power_supply:
		break;

	case LIBERTY_GROUPENUM_power_lut_template:
		break;

	case LIBERTY_GROUPENUM_output_voltage:
		break;

	case LIBERTY_GROUPENUM_operating_conditions:
		break;

	case LIBERTY_GROUPENUM_model:
		break;

	case LIBERTY_GROUPENUM_lu_table_template:
		check_lut_template(group);
		break;

	case LIBERTY_GROUPENUM_input_voltage:
		break;

	case LIBERTY_GROUPENUM_fall_transition_degradation:
		check_lu_table_template_arraysize(group);
		break;

	case LIBERTY_GROUPENUM_fall_net_delay:
		break;

	case LIBERTY_GROUPENUM_em_lut_template:
		break;

	case LIBERTY_GROUPENUM_type:
		break;

	case LIBERTY_GROUPENUM_test_cell:
		break;

	case LIBERTY_GROUPENUM_statetable:
		break;

	case LIBERTY_GROUPENUM_state:
		break;

	case LIBERTY_GROUPENUM_seq_bank:
		check_clear_n_preset(group);
		break;

	case LIBERTY_GROUPENUM_seq:
		check_clear_n_preset(group);
		break;

	case LIBERTY_GROUPENUM_routing_track:
		break;

	case LIBERTY_GROUPENUM_mode_value:
		break;

	case LIBERTY_GROUPENUM_mode_definition:
		break;

	case LIBERTY_GROUPENUM_memory:
		break;

	case LIBERTY_GROUPENUM_leakage_power:
		break;

	case LIBERTY_GROUPENUM_latch_bank:
		check_clear_n_preset(group);
		break;

	case LIBERTY_GROUPENUM_latch:
		check_clear_n_preset(group);
		break;

	case LIBERTY_GROUPENUM_generated_clock:
		break;

	case LIBERTY_GROUPENUM_ff_bank:
		check_clear_n_preset(group);
		check_ff_bank_widths(group);
		break;

	case LIBERTY_GROUPENUM_ff:
		check_clear_n_preset(group);
		break;

	case LIBERTY_GROUPENUM_bus:
 		check_bus_pin_directions(group);
		check_bus_type( group );
		break;

	case LIBERTY_GROUPENUM_tlatch:
		break;

	case LIBERTY_GROUPENUM_rise_transition:
		check_lu_table_template_arraysize(group);
		break;

	case LIBERTY_GROUPENUM_rise_propagation:
		check_lu_table_template_arraysize(group);
		break;

	case LIBERTY_GROUPENUM_rise_constraint:
		break;

	case LIBERTY_GROUPENUM_retaining_rise:
		break;

	case LIBERTY_GROUPENUM_retaining_fall:
		break;

	case LIBERTY_GROUPENUM_fall_transition:
		check_lu_table_template_arraysize(group);
		break;

	case LIBERTY_GROUPENUM_fall_propagation:
		check_lu_table_template_arraysize(group);
		break;

	case LIBERTY_GROUPENUM_fall_constraint:
		break;

	case LIBERTY_GROUPENUM_cell_rise:
		check_lu_table_template_arraysize(group);
		break;

	case LIBERTY_GROUPENUM_cell_fall:
		check_lu_table_template_arraysize(group);
		break;

	case LIBERTY_GROUPENUM_cell_degradation:
		break;

	case LIBERTY_GROUPENUM_timing:
		check_cell_n_prop(group);
		break;

	case LIBERTY_GROUPENUM_pin:
		check_driver_types(group);
		break;

	case LIBERTY_GROUPENUM_minimum_period:
		break;

	case LIBERTY_GROUPENUM_min_pulse_width:
		break;

	case LIBERTY_GROUPENUM_memory_write:
		break;

	case LIBERTY_GROUPENUM_memory_read:
		break;

	case LIBERTY_GROUPENUM_rise_power:
		check_power_lut_template_arraysize(group);
		break;

	case LIBERTY_GROUPENUM_power:
		check_power_lut_template_arraysize(group);
		break;

	case LIBERTY_GROUPENUM_fall_power:
		check_power_lut_template_arraysize(group);
		break;

	case LIBERTY_GROUPENUM_internal_power:

		/* if there is a value here, then check it; otherwise the power/rise_power/fall_power 
		   groups will be there */
		attr = si2drGroupFindAttrByName(group, "values", &err);
		
		if( !si2drObjectIsNull(attr,&err) )
		{
			check_power_lut_template_arraysize(group);
		}
		break;

	case LIBERTY_GROUPENUM_em_max_toggle_rate:
		break;

	case LIBERTY_GROUPENUM_electromigration:
		break;

	case LIBERTY_GROUPENUM_bundle:
		check_members(group);
 		check_bus_pin_directions(group);
		break;

	case LIBERTY_GROUPENUM_cell:
		check_interface_timing(group);
		break;

	case LIBERTY_GROUPENUM_library:
		break;


	}
	
}


void check_group_correspondence( si2drGroupIdT g, libsynt_group_info *gi )
{
	int i, nc;
	si2drNamesIdT names;
	si2drErrorT err;
	si2drStringT str;
	si2drAttrIdT attr;
	si2drAttrsIdT attrs;
	si2drGroupIdT group;
	si2drGroupsIdT groups;
	
	int gl   = si2drObjectGetLineNo(g,&err);
	char *gf = si2drObjectGetFileName(g,&err);
	char gtype[100];
	
	/* check all group attributes, including names, etc. to constraints */

	if( gi && gi->ref )
		gi = gi->ref;
	
	strcpy( gtype, si2drGroupGetGroupType(g,&err) );
	
	/* names */
	nc = 0;
	names = si2drGroupGetNames(g, &err);
	while( (str = si2drIterNextName(names,&err)) )
	{
		nc++;
	}
	si2drIterQuit(names,&err);

	if( gi )
	{
		
		switch( gi->name_constraints )
		{
		case SYNTAX_GNAME_NONE:
			if( nc > 0 )
			{
				sprintf(EB,"%s:%d, The %s group in this context is not supposed to have any names, but %d name(s) is(are) there.",
					   gf, gl, gtype, nc);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
			break;
		case SYNTAX_GNAME_ONE:
			if( nc != 1 )
			{
				sprintf(EB,"%s:%d, The %s group in this context should have 1 name, but %d names are there.",
					   gf, gl, gtype, nc);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
			break;
		case SYNTAX_GNAME_TWO:
			if( nc != 2 )
			{
				sprintf(EB,"%s:%d, The %s group in this context should have 2 nams, but %d name(s) is(are) there.",
					   gf, gl, gtype, nc);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
			break;
		case SYNTAX_GNAME_THREE:
			if( nc != 3 )
			{
				sprintf(EB,"%s:%d, The %s group in this context should have 3 nams, but %d name(s) is(are) there.",
					   gf, gl, gtype, nc);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
			break;
		case SYNTAX_GNAME_NONE_OR_ONE:
			if( nc != 1 && nc != 0 )
			{
				sprintf(EB,"%s:%d, The %s group in this context should have 0 or 1 names, but %d names are there.",
					   gf, gl, gtype, nc);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
			break;
		case SYNTAX_GNAME_ONE_OR_MORE:
			if( nc == 0  )
			{
				sprintf(EB,"%s:%d, The %s group in this context should have 1 or more names, but no names are there.",
					   gf, gl, gtype);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
			break;
		}
	}
	
	/* attrs */
	attrs = si2drGroupGetAttrs(g, &err);
	while( !(si2drObjectIsNull((attr = si2drIterNextAttr(attrs,&err) ), &err)) )
	{
		si2drStringT anam = si2drAttrGetName(attr,&err);
		si2drAttrTypeT at = si2drAttrGetAttrType(attr,&err);
		si2drObjectIdT toid;
		int al = si2drObjectGetLineNo(attr,&err);
		char *af = si2drObjectGetFileName(attr,&err);
		
		toid.v1 = NULL;
		
		if( gi )
			liberty_hash_lookup(gi->attr_hash, anam, &toid);
		
		if( toid.v1 == NULL )
		{
			si2drDefineIdT def;
			int found;
			
			/* defines could explain the presence of a simple attr that
			   is not found in the group attrs */
			
			def = si2drPIFindDefineByName(anam, &err);
			if( err != SI2DR_NO_ERROR )
			{
				sprintf(EB,"%s:%d, The %s attribute is not allowed in the %s group in this context.",
					   af, al, anam, gtype);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
			else
			{
				/* check to make sure the define's constraints are being obeyed */
				si2drValueTypeT valtype;
				si2drStringT   allowed_group, name;
				si2drAttrTypeT t = si2drAttrGetAttrType(attr,&err);
				char buf[1000],*p,*q;
				si2drDefineGetInfo(def, &name, &allowed_group, &valtype, &err);
				strcpy(buf, allowed_group);
				q = buf;
				found = 0;
				strcat(buf,"|");
				if( strcmp(gtype,allowed_group) )
				{
					while( *q && (p = strchr(q,'|')) )
					{
						*p = 0;
						if( !strcmp(q,gtype) )
						{
							found = 1;
							break;
						}
						q = p+1;
					}
					if( !found )
					{
						sprintf(EB,"%s:%d, The %s attribute was defined to be used in the %s group, not the %s group.",
							   af, al, anam, allowed_group, gtype);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}
				if( t != SI2DR_SIMPLE )
				{
					sprintf(EB,"%s:%d, The %s attribute was defined and needs to used as a simple attribute.",
						   af, al, anam);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
#ifdef DONT_VERIFY_DEFINE_ATTRIBUTE_TYPE
				else
				{
					si2drValueTypeT  vt;
					vt = si2drSimpleAttrGetValueType(attr,&err);
					if( valtype == SI2DR_STRING && vt != SI2DR_STRING )
					{
						sprintf(EB,"%s:%d, The %s attribute should be of type String.",
							   af, al, anam);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
					if( valtype != SI2DR_STRING && vt == SI2DR_STRING )
					{
						sprintf(EB,"%s:%d, The %s attribute should not be of type String.",
							   af, al, anam);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}
#endif
			}
		}
		else
		{
			check_attr_correspondence(attr, (libsynt_attribute_info*)toid.v1);
		}
	}
	si2drIterQuit(attrs,&err);
	
	
	/* subgroups */
	groups = si2drGroupGetGroups(g, &err);
	while( !(si2drObjectIsNull((group = si2drIterNextGroup(groups,&err) ), &err)) )
	{
		si2drStringT gt = si2drGroupGetGroupType(group,&err);
		si2drObjectIdT toid;
		int al = si2drObjectGetLineNo(group,&err);
		char *af = si2drObjectGetFileName(group,&err);

		toid.v1 = NULL;
		
		if( gi )
			liberty_hash_lookup(gi->group_hash, gt, &toid);

		if( toid.v1 == NULL )
		{
			si2drDefineIdT def;
			/* check to see if a define_group is defined for this situation */
			def = si2drPIFindDefineByName(gt, &err);
			if( err != SI2DR_NO_ERROR )
			{
				sprintf(EB,"%s:%d, The %s group is not allowed in the %s group in this context.",
					   af, al, gt, gtype);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
			else
			{
				si2drStringT grname, grall;
				si2drValueTypeT grval;
				
				si2drDefineGetInfo( def,
									&grname,
									&grall,
									&grval,
									&err);

				if( strcmp(grall,gtype) && grval == SI2DR_UNDEFINED_VALUETYPE )
				{
					sprintf(EB,"%s:%d, The %s group is allowed in the %s group, but not allowed in the %s group in this case.",
						   af, al, gt, grall, gtype);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				else
					check_group_correspondence(group, (libsynt_group_info*)toid.v1);
					
			}
		}
		else
		{
			check_group_correspondence(group, (libsynt_group_info*)toid.v1);
		}
	}
	si2drIterQuit(groups,&err);
	if( gi )
		group_specific_checks( g );
}

static void check_attr_correspondence( si2drAttrIdT attr, libsynt_attribute_info *ai )
{
	si2drErrorT err;
	si2drAttrTypeT t = si2drAttrGetAttrType(attr,&err);
	int al = si2drObjectGetLineNo(attr,&err);
	char *af = si2drObjectGetFileName(attr,&err);
	si2drStringT aname = si2drAttrGetName(attr,&err);
	si2drValueTypeT  vt;
	si2drInt32T     intgr;
	si2drFloat64T   float64;
	si2drStringT    string;
	si2drBooleanT   bool;
	int found;
	int  i;
	
	if( t == SI2DR_SIMPLE )
	{
		if( ai->type == SYNTAX_ATTRTYPE_COMPLEX ||
			ai->type == SYNTAX_ATTRTYPE_COMPLEX_UNKNOWN ||
			ai->type == SYNTAX_ATTRTYPE_COMPLEX_LIST )
		{
			sprintf(EB,"%s:%d, Attribute %s should be complex, but is simple here.",
				   af, al, aname);
			(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
						  EB, 
						  &err);
			errcount++;
		}
		else
		{
			char *x;
			
			vt = si2drSimpleAttrGetValueType(attr,&err);

			switch( vt )
			{
			case SI2DR_INT32:
				intgr = si2drSimpleAttrGetInt32Value(attr,&err);
				
				switch( ai->type )
				{
				case SYNTAX_ATTRTYPE_STRING:
					sprintf(EB,"%s:%d, Attribute %s should be a string Attribute, but is integer instead.",
						   af, al, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					break;
					
				case SYNTAX_ATTRTYPE_INT:
					if( ai->u.intcon )
					{
						if( ai->u.intcon->greater_specd )
						{
							if( intgr < ai->u.intcon->greater_than )
							{
								sprintf(EB,"%s:%d, Attribute %s has value %d, but it should be greater than %d.",
									   af, al, aname, intgr, ai->u.intcon->greater_than);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
						if( ai->u.intcon->less_specd )
						{
							if( intgr > ai->u.intcon->less_than )
							{
								sprintf(EB,"%s:%d, Attribute %s has value %d, but it should be less than %d.",
									   af, al, aname, intgr, ai->u.intcon->less_than);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
					}
					break;
					
				case SYNTAX_ATTRTYPE_FLOAT:
					if( ai->u.floatcon )
					{
						if( ai->u.floatcon->greater_specd )
						{
							if( (double)intgr < ai->u.floatcon->greater_than )
							{
								sprintf(EB,"%s:%d, Attribute %s has value %d, but it should be greater than %g.",
									   af, al, aname, intgr, ai->u.floatcon->greater_than);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
						if( ai->u.floatcon->less_specd )
						{
							if( (double)intgr > ai->u.floatcon->less_than )
							{
								sprintf(EB,"%s:%d, Attribute %s has value %d, but it should be less than %g.",
									   af, al, aname, intgr, ai->u.floatcon->less_than);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
					}
					break;
					
				case SYNTAX_ATTRTYPE_VIRTUAL:
					break;
				case SYNTAX_ATTRTYPE_BOOLEAN:
					sprintf(EB,"%s:%d, Attribute %s should be a boolean Attribute, but is integer instead.",
						   af, al, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					break;
					
					
				case SYNTAX_ATTRTYPE_ENUM:
					sprintf(EB,"%s:%d, Attribute %s should be a string Attribute, but is integer instead.",
						   af, al, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					break;
				}
				break;
				
			case SI2DR_STRING:
				string = si2drSimpleAttrGetStringValue(attr,&err);
				
				switch( ai->type )
				{
				case SYNTAX_ATTRTYPE_STRING:
				case SYNTAX_ATTRTYPE_VIRTUAL:
					break;
					
				case SYNTAX_ATTRTYPE_ENUM:
					found = 0;
					for(i=0;i<ai->u.stringenum->size;i++)
					{
						if( !strcmp(ai->u.stringenum->array[i], string) )
						{
							found = 1;
							break;
						}
					}
					if( !found )
					{
						sprintf(EB,"%s:%d, Attribute %s has value %s, which does not match any of the following values:\n     ",
							   af, al, aname, string);
						
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						for(i=0;i<ai->u.stringenum->size;i++)
						{
							printf( "%s  ", ai->u.stringenum->array[i]);
						}
						sprintf(EB,"");
					}
					break;
					
				case SYNTAX_ATTRTYPE_BOOLEAN:
					if( strcasecmp(string,"false") && strcasecmp(string,"true") )
					{
						sprintf(EB,"%s:%d, Attribute %s should be a boolean Attribute, but is a string (%s) instead.",
							   af, al, aname, string);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
					}
					break;
					
				case SYNTAX_ATTRTYPE_INT:
				case SYNTAX_ATTRTYPE_FLOAT:
					strtod(string,&x);
					if( *x != 0 )
					{
						if( !isa_formula(string) )
						{
							sprintf(EB,"%s:%d, Attribute %s should be an integer or float Attribute, but is a string (%s) instead.",
								   af, al, aname, string);
							(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
										  EB, 
										  &err);
							errcount++;
						}
					}
					break;
				}
				break;
				
			case SI2DR_FLOAT64:
				float64 = si2drSimpleAttrGetFloat64Value(attr,&err);
				
				switch( ai->type )
				{
				case SYNTAX_ATTRTYPE_STRING:
					sprintf(EB,"%s:%d, Attribute %s should be a string Attribute, but is float instead.",
						   af, al, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					break;
					
				case SYNTAX_ATTRTYPE_BOOLEAN:
					sprintf(EB,"%s:%d, Attribute %s should be a boolean Attribute, but is float instead.",
						   af, al, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					break;

				case SYNTAX_ATTRTYPE_INT:
					if( ai->u.intcon )
					{
						if( ai->u.intcon->greater_specd )
						{
							if( float64 < ai->u.intcon->greater_than )
							{
								sprintf(EB,"%s:%d, Attribute %s has value %g, but it should be greater than %d.",
									   af, al, aname, float64, ai->u.intcon->greater_than);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
						if( ai->u.intcon->less_specd )
						{
							if( float64 > ai->u.intcon->less_than )
							{
								sprintf(EB,"%s:%d, Attribute %s has value %g, but it should be less than %d.",
									   af, al, aname, float64, ai->u.intcon->less_than);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
					}
					break;
					
				case SYNTAX_ATTRTYPE_FLOAT:
					if( ai->u.floatcon )
					{
						if( ai->u.floatcon->greater_specd )
						{
							if( (double)float64 < ai->u.floatcon->greater_than )
							{
								sprintf(EB,"%s:%d, Attribute %s has value %g, but it should be greater than %g.",
									   af, al, aname, float64, ai->u.floatcon->greater_than);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
						if( ai->u.floatcon->less_specd )
						{
							if( (double)float64 > ai->u.floatcon->less_than )
							{
								sprintf(EB,"%s:%d, Attribute %s has value %g, but it should be less than %g.",
									   af, al, aname, float64, ai->u.floatcon->less_than);
								(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
											  EB, 
											  &err);
								errcount++;
							}
						}
					}
					break;
					
				case SYNTAX_ATTRTYPE_VIRTUAL:
					
					break;
					
				case SYNTAX_ATTRTYPE_ENUM:
					sprintf(EB,"%s:%d, Attribute %s should be an enumerated string Attribute, but is float instead.",
						   af, al, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					break;
				}
				break;
				
			case SI2DR_BOOLEAN:
				bool = si2drSimpleAttrGetBooleanValue(attr,&err);
				
				switch( ai->type )
				{
				case SYNTAX_ATTRTYPE_STRING:
					sprintf(EB,"%s:%d, Attribute %s should be a string Attribute, but is boolean instead.",
						   af, al, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					break;
					
				case SYNTAX_ATTRTYPE_BOOLEAN:
				case SYNTAX_ATTRTYPE_VIRTUAL:
					break;
					
				case SYNTAX_ATTRTYPE_INT:
				case SYNTAX_ATTRTYPE_FLOAT:
					sprintf(EB,"%s:%d, Attribute %s should be an integer/float Attribute, but is boolean instead.",
						   af, al, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					break;
					
				case SYNTAX_ATTRTYPE_ENUM:
					sprintf(EB,"%s:%d, Attribute %s should be an enumerated string Attribute, but is boolean instead.",
						   af, al, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
					break;
				}
				break;
			}
		}
	}

	else if( t == SI2DR_COMPLEX )
	{
		if( ai->type != SYNTAX_ATTRTYPE_COMPLEX &&
			ai->type != SYNTAX_ATTRTYPE_COMPLEX_UNKNOWN &&
			ai->type != SYNTAX_ATTRTYPE_COMPLEX_LIST )
		{
			sprintf(EB,"%s:%d, Attribute %s should be simple, but is complex here.",
				   af, al, aname);
			(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
						  EB, 
						  &err);
			errcount++;
		}
		else
		{
			si2drValuesIdT vals = si2drComplexAttrGetValues(attr,&err);
			si2drValueTypeT type;
			si2drInt32T     intgr;
			si2drFloat64T   float64;
			si2drStringT    string;
			si2drBooleanT   bool;
			libsynt_argument *arg = ai->u.args;
			int argnum;
			
			argnum = 1;
			while( si2drIterNextComplexValue(vals,&type,&intgr,&float64,&string,&bool,&err), type != SI2DR_UNDEFINED_VALUETYPE )
			{
				if( !arg )
				{
					sprintf(EB,"%s:%d, %s has too many arguments.",
						   af, al, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				if( arg->type == SYNTAX_ATTRTYPE_COMPLEX_UNKNOWN || arg->type == SYNTAX_ATTRTYPE_COMPLEX_LIST )
				{
					arg = arg->next;
					break; /* that's all folks */
				}

				if( arg->type == SYNTAX_ATTRTYPE_STRING && type != SI2DR_STRING )
				{
					sprintf(EB,"%s:%d, Argument #%d of %s should be a string, but it is not.",
						   af, al, argnum, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				else if(arg->type == SYNTAX_ATTRTYPE_FLOAT && (type == SI2DR_STRING
															   || type == SI2DR_BOOLEAN ))
				{
					sprintf(EB,"%s:%d, Argument #%d of %s should be a float, but it is not.",
						   af, al, argnum, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				else if(arg->type == SYNTAX_ATTRTYPE_INT && (type == SI2DR_STRING
															   || type == SI2DR_BOOLEAN ))
				{
					sprintf(EB,"%s:%d, Argument #%d of %s should be an int, but it is not.",
						   af, al, argnum, aname);
					(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
								  EB, 
								  &err);
					errcount++;
				}
				else if(arg->type == SYNTAX_ATTRTYPE_BOOLEAN && (type != SI2DR_BOOLEAN))
				{
					if( type != SI2DR_STRING || (strcasecmp(string,"true") && strcasecmp(string,"false") ) )
					{
						sprintf(EB,"%s:%d, Argument #%d of %s should be a boolean (true or false), but it is not.",
							   af, al, argnum, aname);
						(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
									  EB, 
									  &err);
						errcount++;
					}
				}
				argnum++;
				if( arg )
					arg = arg->next;
				
			}
			si2drIterQuit(vals,&err);
			if( arg )
			{
				sprintf(EB,"%s:%d, %s has too few arguments.",
					   af, al, aname);
				(*MsgPrinter)(SI2DR_SEVERITY_ERR, SI2DR_SYNTAX_ERROR, 
							  EB, 
							  &err);
				errcount++;
			}
		}
	}
	attribute_specific_checks( attr );
}
