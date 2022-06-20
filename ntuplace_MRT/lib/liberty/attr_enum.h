

typedef enum
{
LIBERTY_ATTRENUM_input_pins					= 1,
LIBERTY_ATTRENUM_fpga_timing_type					= 2,
LIBERTY_ATTRENUM_fpga_degenerate_output					= 3,
LIBERTY_ATTRENUM_xnf_schnm					= 4,
LIBERTY_ATTRENUM_xnf_device_name					= 5,
LIBERTY_ATTRENUM_fpga_min_degen_input_size					= 6,
LIBERTY_ATTRENUM_fpga_max_degen_input_size					= 7,
LIBERTY_ATTRENUM_fpga_lut_output					= 8,
LIBERTY_ATTRENUM_fpga_lut_insert_before_sequential					= 9,
LIBERTY_ATTRENUM_fpga_family					= 10,
LIBERTY_ATTRENUM_fpga_complex_degenerate					= 11,
LIBERTY_ATTRENUM_fpga_cell_type					= 12,
LIBERTY_ATTRENUM_fpga_bridge_inputs					= 13,
LIBERTY_ATTRENUM_fpga_prefer_undegenerated_gates					= 14,
LIBERTY_ATTRENUM_fpga_allow_duplicate_degenerated_gates					= 15,
LIBERTY_ATTRENUM_force_inverter_removal					= 16,
LIBERTY_ATTRENUM_fanout_resistance					= 17,
LIBERTY_ATTRENUM_fanout_capacitance					= 18,
LIBERTY_ATTRENUM_fanout_area					= 19,
LIBERTY_ATTRENUM_wire_load_from_area					= 20,
LIBERTY_ATTRENUM_fanout_length					= 21,
LIBERTY_ATTRENUM_slope					= 22,
LIBERTY_ATTRENUM_resistance					= 23,
LIBERTY_ATTRENUM_slower_factor					= 24,
LIBERTY_ATTRENUM_faster_factor					= 25,
LIBERTY_ATTRENUM_default_power_rail					= 26,
LIBERTY_ATTRENUM_vomin					= 27,
LIBERTY_ATTRENUM_vomax					= 28,
LIBERTY_ATTRENUM_vol					= 29,
LIBERTY_ATTRENUM_voh					= 30,
LIBERTY_ATTRENUM_power_rail					= 31,
LIBERTY_ATTRENUM_voltage					= 32,
LIBERTY_ATTRENUM_tree_type					= 33,
LIBERTY_ATTRENUM_temperature					= 34,
LIBERTY_ATTRENUM_short					= 35,
LIBERTY_ATTRENUM_cell_name					= 36,
LIBERTY_ATTRENUM_variable_3					= 37,
LIBERTY_ATTRENUM_vimin					= 38,
LIBERTY_ATTRENUM_vimax					= 39,
LIBERTY_ATTRENUM_vil					= 40,
LIBERTY_ATTRENUM_vih					= 41,
LIBERTY_ATTRENUM_variable_2					= 42,
LIBERTY_ATTRENUM_variable_1					= 43,
LIBERTY_ATTRENUM_downto					= 44,
LIBERTY_ATTRENUM_data_type					= 45,
LIBERTY_ATTRENUM_bit_width					= 46,
LIBERTY_ATTRENUM_bit_to					= 47,
LIBERTY_ATTRENUM_bit_from					= 48,
LIBERTY_ATTRENUM_base_type					= 49,
LIBERTY_ATTRENUM_table					= 50,
LIBERTY_ATTRENUM_force_11					= 51,
LIBERTY_ATTRENUM_force_10					= 52,
LIBERTY_ATTRENUM_force_01					= 53,
LIBERTY_ATTRENUM_force_00					= 54,
LIBERTY_ATTRENUM_tracks					= 55,
LIBERTY_ATTRENUM_total_track_area					= 56,
LIBERTY_ATTRENUM_word_width					= 57,
LIBERTY_ATTRENUM_type					= 58,
LIBERTY_ATTRENUM_row_address					= 59,
LIBERTY_ATTRENUM_column_address					= 60,
LIBERTY_ATTRENUM_address_width					= 61,
LIBERTY_ATTRENUM_value					= 62,
LIBERTY_ATTRENUM_enable_also					= 63,
LIBERTY_ATTRENUM_data_in					= 64,
LIBERTY_ATTRENUM_shifts					= 65,
LIBERTY_ATTRENUM_edges					= 66,
LIBERTY_ATTRENUM_multiplied_by					= 67,
LIBERTY_ATTRENUM_master_pin					= 68,
LIBERTY_ATTRENUM_invert					= 69,
LIBERTY_ATTRENUM_duty_cycle					= 70,
LIBERTY_ATTRENUM_divided_by					= 71,
LIBERTY_ATTRENUM_clock_pin					= 72,
LIBERTY_ATTRENUM_preset					= 73,
LIBERTY_ATTRENUM_next_state					= 74,
LIBERTY_ATTRENUM_clocked_on_also					= 75,
LIBERTY_ATTRENUM_clear_preset_var2					= 76,
LIBERTY_ATTRENUM_clear_preset_var1					= 77,
LIBERTY_ATTRENUM_clear					= 78,
LIBERTY_ATTRENUM_bus_type					= 79,
LIBERTY_ATTRENUM_tdisable					= 80,
LIBERTY_ATTRENUM_edge_type					= 81,
LIBERTY_ATTRENUM_rise_pin_resistance					= 82,
LIBERTY_ATTRENUM_rise_delay_intercept					= 83,
LIBERTY_ATTRENUM_mode					= 84,
LIBERTY_ATTRENUM_fall_pin_resistance					= 85,
LIBERTY_ATTRENUM_fall_delay_intercept					= 86,
LIBERTY_ATTRENUM_when_start					= 87,
LIBERTY_ATTRENUM_when_end					= 88,
LIBERTY_ATTRENUM_timing_type					= 89,
LIBERTY_ATTRENUM_timing_sense					= 90,
LIBERTY_ATTRENUM_steady_state_resistance_low_min					= 91,
LIBERTY_ATTRENUM_steady_state_resistance_low_max					= 92,
LIBERTY_ATTRENUM_steady_state_resistance_low					= 93,
LIBERTY_ATTRENUM_steady_state_resistance_high_min					= 94,
LIBERTY_ATTRENUM_steady_state_resistance_high_max					= 95,
LIBERTY_ATTRENUM_steady_state_resistance_high					= 96,
LIBERTY_ATTRENUM_steady_state_resistance_float_min					= 97,
LIBERTY_ATTRENUM_steady_state_resistance_float_max					= 98,
LIBERTY_ATTRENUM_steady_state_resistance_below_low					= 99,
LIBERTY_ATTRENUM_steady_state_resistance_above_high					= 100,
LIBERTY_ATTRENUM_slope_rise					= 101,
LIBERTY_ATTRENUM_slope_fall					= 102,
LIBERTY_ATTRENUM_setup_coefficient					= 103,
LIBERTY_ATTRENUM_sdf_edges					= 104,
LIBERTY_ATTRENUM_sdf_cond_start					= 105,
LIBERTY_ATTRENUM_sdf_cond_end					= 106,
LIBERTY_ATTRENUM_rise_resistance					= 107,
LIBERTY_ATTRENUM_related_output_pin					= 108,
LIBERTY_ATTRENUM_related_bus_equivalent					= 109,
LIBERTY_ATTRENUM_intrinsic_rise					= 110,
LIBERTY_ATTRENUM_intrinsic_fall					= 111,
LIBERTY_ATTRENUM_hold_coefficient					= 112,
LIBERTY_ATTRENUM_fall_resistance					= 113,
LIBERTY_ATTRENUM_edge_rate_sensitivity_r1					= 114,
LIBERTY_ATTRENUM_edge_rate_sensitivity_r0					= 115,
LIBERTY_ATTRENUM_edge_rate_sensitivity_f1					= 116,
LIBERTY_ATTRENUM_edge_rate_sensitivity_f0					= 117,
LIBERTY_ATTRENUM_default_timing					= 118,
LIBERTY_ATTRENUM_lsi_pad					= 119,
LIBERTY_ATTRENUM_fsim_map					= 120,
LIBERTY_ATTRENUM_constraint					= 121,
LIBERTY_ATTRENUM_sdf_cond					= 122,
LIBERTY_ATTRENUM_constraint_low					= 123,
LIBERTY_ATTRENUM_constraint_high					= 124,
LIBERTY_ATTRENUM_enable					= 125,
LIBERTY_ATTRENUM_clocked_on					= 126,
LIBERTY_ATTRENUM_address					= 127,
LIBERTY_ATTRENUM_when					= 128,
LIBERTY_ATTRENUM_switching_together_group					= 129,
LIBERTY_ATTRENUM_switching_interval					= 130,
LIBERTY_ATTRENUM_rising_together_group					= 131,
LIBERTY_ATTRENUM_related_rising_pin					= 132,
LIBERTY_ATTRENUM_related_outputs					= 133,
LIBERTY_ATTRENUM_related_inputs					= 134,
LIBERTY_ATTRENUM_related_input					= 135,
LIBERTY_ATTRENUM_related_falling_pin					= 136,
LIBERTY_ATTRENUM_falling_together_group					= 137,
LIBERTY_ATTRENUM_equal_or_opposite_output					= 138,
LIBERTY_ATTRENUM_width_coefficient					= 139,
LIBERTY_ATTRENUM_height_coefficient					= 140,
LIBERTY_ATTRENUM_area_coefficient					= 141,
LIBERTY_ATTRENUM_variables					= 142,
LIBERTY_ATTRENUM_mapping					= 143,
LIBERTY_ATTRENUM_calc_mode					= 144,
LIBERTY_ATTRENUM_variable_7_range					= 145,
LIBERTY_ATTRENUM_variable_6_range					= 146,
LIBERTY_ATTRENUM_variable_5_range					= 147,
LIBERTY_ATTRENUM_variable_4_range					= 148,
LIBERTY_ATTRENUM_variable_3_range					= 149,
LIBERTY_ATTRENUM_variable_2_range					= 150,
LIBERTY_ATTRENUM_variable_1_range					= 151,
LIBERTY_ATTRENUM_values					= 152,
LIBERTY_ATTRENUM_orders					= 153,
LIBERTY_ATTRENUM_intermediate_values					= 154,
LIBERTY_ATTRENUM_index_3					= 155,
LIBERTY_ATTRENUM_index_2					= 156,
LIBERTY_ATTRENUM_index_1					= 157,
LIBERTY_ATTRENUM_coefs					= 158,
LIBERTY_ATTRENUM_process					= 159,
LIBERTY_ATTRENUM_poly_convert					= 160,
LIBERTY_ATTRENUM_related_pin					= 161,
LIBERTY_ATTRENUM_related_bus_pins					= 162,
LIBERTY_ATTRENUM_members					= 163,
LIBERTY_ATTRENUM_input_map_shift					= 164,
LIBERTY_ATTRENUM_x_function					= 165,
LIBERTY_ATTRENUM_three_state					= 166,
LIBERTY_ATTRENUM_test_output_only					= 167,
LIBERTY_ATTRENUM_state_function					= 168,
LIBERTY_ATTRENUM_slew_control					= 169,
LIBERTY_ATTRENUM_signal_type					= 170,
LIBERTY_ATTRENUM_rise_time_before_threshold					= 171,
LIBERTY_ATTRENUM_rise_time_after_threshold					= 172,
LIBERTY_ATTRENUM_rise_current_slope_before_threshold					= 173,
LIBERTY_ATTRENUM_rise_current_slope_after_threshold					= 174,
LIBERTY_ATTRENUM_rise_capacitance					= 175,
LIBERTY_ATTRENUM_reference_capacitance					= 176,
LIBERTY_ATTRENUM_pulling_resistance					= 177,
LIBERTY_ATTRENUM_pulling_current					= 178,
LIBERTY_ATTRENUM_primary_output					= 179,
LIBERTY_ATTRENUM_prefer_tied					= 180,
LIBERTY_ATTRENUM_pin_func_type					= 181,
LIBERTY_ATTRENUM_output_voltage					= 182,
LIBERTY_ATTRENUM_output_signal_level					= 183,
LIBERTY_ATTRENUM_nextstate_type					= 184,
LIBERTY_ATTRENUM_multicell_pad_pin					= 185,
LIBERTY_ATTRENUM_min_transition					= 186,
LIBERTY_ATTRENUM_min_pulse_width_low					= 187,
LIBERTY_ATTRENUM_min_pulse_width_high					= 188,
LIBERTY_ATTRENUM_min_period					= 189,
LIBERTY_ATTRENUM_min_fanout					= 190,
LIBERTY_ATTRENUM_min_capacitance					= 191,
LIBERTY_ATTRENUM_max_transition					= 192,
LIBERTY_ATTRENUM_max_time_borrow					= 193,
LIBERTY_ATTRENUM_max_fanout					= 194,
LIBERTY_ATTRENUM_max_capacitance					= 195,
LIBERTY_ATTRENUM_is_pad					= 196,
LIBERTY_ATTRENUM_inverted_output					= 197,
LIBERTY_ATTRENUM_internal_node					= 198,
LIBERTY_ATTRENUM_input_voltage					= 199,
LIBERTY_ATTRENUM_input_signal_level					= 200,
LIBERTY_ATTRENUM_input_map					= 201,
LIBERTY_ATTRENUM_hysteresis					= 202,
LIBERTY_ATTRENUM_has_builtin_pad					= 203,
LIBERTY_ATTRENUM_function					= 204,
LIBERTY_ATTRENUM_fault_model					= 205,
LIBERTY_ATTRENUM_fanout_load					= 206,
LIBERTY_ATTRENUM_fall_time_before_threshold					= 207,
LIBERTY_ATTRENUM_fall_time_after_threshold					= 208,
LIBERTY_ATTRENUM_fall_current_slope_before_threshold					= 209,
LIBERTY_ATTRENUM_fall_current_slope_after_threshold					= 210,
LIBERTY_ATTRENUM_fall_capacitance					= 211,
LIBERTY_ATTRENUM_edge_rate_rise					= 212,
LIBERTY_ATTRENUM_edge_rate_load_rise					= 213,
LIBERTY_ATTRENUM_edge_rate_load_fall					= 214,
LIBERTY_ATTRENUM_edge_rate_fall					= 215,
LIBERTY_ATTRENUM_edge_rate_breakpoint_r1					= 216,
LIBERTY_ATTRENUM_edge_rate_breakpoint_r0					= 217,
LIBERTY_ATTRENUM_edge_rate_breakpoint_f1					= 218,
LIBERTY_ATTRENUM_edge_rate_breakpoint_f0					= 219,
LIBERTY_ATTRENUM_driver_type					= 220,
LIBERTY_ATTRENUM_drive_current					= 221,
LIBERTY_ATTRENUM_direction					= 222,
LIBERTY_ATTRENUM_dcm_timing					= 223,
LIBERTY_ATTRENUM_connection_class					= 224,
LIBERTY_ATTRENUM_complementary_pin					= 225,
LIBERTY_ATTRENUM_clock_gate_test_pin					= 226,
LIBERTY_ATTRENUM_clock_gate_out_pin					= 227,
LIBERTY_ATTRENUM_clock_gate_obs_pin					= 228,
LIBERTY_ATTRENUM_clock_gate_enable_pin					= 229,
LIBERTY_ATTRENUM_clock_gate_clock_pin					= 230,
LIBERTY_ATTRENUM_clock					= 231,
LIBERTY_ATTRENUM_capacitance					= 232,
LIBERTY_ATTRENUM_rail_connection					= 233,
LIBERTY_ATTRENUM_pin_opposite					= 234,
LIBERTY_ATTRENUM_pin_equal					= 235,
LIBERTY_ATTRENUM_vhdl_name					= 236,
LIBERTY_ATTRENUM_use_for_size_only					= 237,
LIBERTY_ATTRENUM_single_bit_degenerate					= 238,
LIBERTY_ATTRENUM_set_node					= 239,
LIBERTY_ATTRENUM_scan_group					= 240,
LIBERTY_ATTRENUM_scaling_factors					= 241,
LIBERTY_ATTRENUM_preferred					= 242,
LIBERTY_ATTRENUM_pad_type					= 243,
LIBERTY_ATTRENUM_pad_cell					= 244,
LIBERTY_ATTRENUM_observe_node					= 245,
LIBERTY_ATTRENUM_mpm_name					= 246,
LIBERTY_ATTRENUM_mpm_libname					= 247,
LIBERTY_ATTRENUM_map_only					= 248,
LIBERTY_ATTRENUM_is_clock_gating_cell					= 249,
LIBERTY_ATTRENUM_interface_timing					= 250,
LIBERTY_ATTRENUM_handle_negative_constraint					= 251,
LIBERTY_ATTRENUM_geometry_print					= 252,
LIBERTY_ATTRENUM_dont_use					= 253,
LIBERTY_ATTRENUM_dont_touch					= 254,
LIBERTY_ATTRENUM_dont_fault					= 255,
LIBERTY_ATTRENUM_contention_condition					= 256,
LIBERTY_ATTRENUM_clock_gating_integrated_cell					= 257,
LIBERTY_ATTRENUM_cell_leakage_power					= 258,
LIBERTY_ATTRENUM_cell_footprint					= 259,
LIBERTY_ATTRENUM_auxiliary_pad_cell					= 260,
LIBERTY_ATTRENUM_area					= 261,
LIBERTY_ATTRENUM_technology					= 262,
LIBERTY_ATTRENUM_routing_layers					= 263,
LIBERTY_ATTRENUM_piece_define					= 264,
LIBERTY_ATTRENUM_library_features					= 265,
LIBERTY_ATTRENUM_define_cell_area					= 266,
LIBERTY_ATTRENUM_capacitive_load_unit					= 267,
LIBERTY_ATTRENUM_voltage_unit					= 268,
LIBERTY_ATTRENUM_timing_report					= 269,
LIBERTY_ATTRENUM_time_unit					= 270,
LIBERTY_ATTRENUM_slew_upper_threshold_pct_rise					= 271,
LIBERTY_ATTRENUM_slew_upper_threshold_pct_fall					= 272,
LIBERTY_ATTRENUM_slew_lower_threshold_pct_rise					= 273,
LIBERTY_ATTRENUM_slew_lower_threshold_pct_fall					= 274,
LIBERTY_ATTRENUM_slew_derate_from_library					= 275,
LIBERTY_ATTRENUM_simulation					= 276,
LIBERTY_ATTRENUM_revision					= 277,
LIBERTY_ATTRENUM_pulling_resistance_unit					= 278,
LIBERTY_ATTRENUM_preferred_output_pad_voltage					= 279,
LIBERTY_ATTRENUM_preferred_output_pad_slew_rate_control					= 280,
LIBERTY_ATTRENUM_preferred_input_pad_voltage					= 281,
LIBERTY_ATTRENUM_power_model					= 282,
LIBERTY_ATTRENUM_piece_type					= 283,
LIBERTY_ATTRENUM_output_threshold_pct_rise					= 284,
LIBERTY_ATTRENUM_output_threshold_pct_fall					= 285,
LIBERTY_ATTRENUM_nom_voltage					= 286,
LIBERTY_ATTRENUM_nom_temperature					= 287,
LIBERTY_ATTRENUM_nom_process					= 288,
LIBERTY_ATTRENUM_nom_calc_mode					= 289,
LIBERTY_ATTRENUM_lsi_rounding_digit					= 290,
LIBERTY_ATTRENUM_lsi_rounding_cutoff					= 291,
LIBERTY_ATTRENUM_lsi_pad_rise					= 292,
LIBERTY_ATTRENUM_lsi_pad_fall					= 293,
LIBERTY_ATTRENUM_leakage_power_unit					= 294,
LIBERTY_ATTRENUM_key_version					= 295,
LIBERTY_ATTRENUM_key_seed					= 296,
LIBERTY_ATTRENUM_key_file					= 297,
LIBERTY_ATTRENUM_key_feature					= 298,
LIBERTY_ATTRENUM_key_bit					= 299,
LIBERTY_ATTRENUM_k_volt_wire_res					= 300,
LIBERTY_ATTRENUM_k_volt_wire_cap					= 301,
LIBERTY_ATTRENUM_k_volt_slope_rise					= 302,
LIBERTY_ATTRENUM_k_volt_slope_fall					= 303,
LIBERTY_ATTRENUM_k_volt_skew_rise					= 304,
LIBERTY_ATTRENUM_k_volt_skew_fall					= 305,
LIBERTY_ATTRENUM_k_volt_setup_rise					= 306,
LIBERTY_ATTRENUM_k_volt_setup_fall					= 307,
LIBERTY_ATTRENUM_k_volt_rise_transition					= 308,
LIBERTY_ATTRENUM_k_volt_rise_propagation					= 309,
LIBERTY_ATTRENUM_k_volt_rise_pin_resistance					= 310,
LIBERTY_ATTRENUM_k_volt_rise_delay_intercept					= 311,
LIBERTY_ATTRENUM_k_volt_removal_rise					= 312,
LIBERTY_ATTRENUM_k_volt_removal_fall					= 313,
LIBERTY_ATTRENUM_k_volt_recovery_rise					= 314,
LIBERTY_ATTRENUM_k_volt_recovery_fall					= 315,
LIBERTY_ATTRENUM_k_volt_pin_cap					= 316,
LIBERTY_ATTRENUM_k_volt_nochange_rise					= 317,
LIBERTY_ATTRENUM_k_volt_nochange_fall					= 318,
LIBERTY_ATTRENUM_k_volt_min_pulse_width_low					= 319,
LIBERTY_ATTRENUM_k_volt_min_pulse_width_high					= 320,
LIBERTY_ATTRENUM_k_volt_min_period					= 321,
LIBERTY_ATTRENUM_k_volt_intrinsic_rise					= 322,
LIBERTY_ATTRENUM_k_volt_intrinsic_fall					= 323,
LIBERTY_ATTRENUM_k_volt_internal_power					= 324,
LIBERTY_ATTRENUM_k_volt_hold_rise					= 325,
LIBERTY_ATTRENUM_k_volt_hold_fall					= 326,
LIBERTY_ATTRENUM_k_volt_fall_transition					= 327,
LIBERTY_ATTRENUM_k_volt_fall_propagation					= 328,
LIBERTY_ATTRENUM_k_volt_fall_pin_resistance					= 329,
LIBERTY_ATTRENUM_k_volt_fall_delay_intercept					= 330,
LIBERTY_ATTRENUM_k_volt_drive_rise					= 331,
LIBERTY_ATTRENUM_k_volt_drive_fall					= 332,
LIBERTY_ATTRENUM_k_volt_drive_current					= 333,
LIBERTY_ATTRENUM_k_volt_cell_rise					= 334,
LIBERTY_ATTRENUM_k_volt_cell_leakage_power					= 335,
LIBERTY_ATTRENUM_k_volt_cell_fall					= 336,
LIBERTY_ATTRENUM_k_volt_cell_degradation					= 337,
LIBERTY_ATTRENUM_k_temp_wire_res					= 338,
LIBERTY_ATTRENUM_k_temp_wire_cap					= 339,
LIBERTY_ATTRENUM_k_temp_slope_rise					= 340,
LIBERTY_ATTRENUM_k_temp_slope_fall					= 341,
LIBERTY_ATTRENUM_k_temp_skew_rise					= 342,
LIBERTY_ATTRENUM_k_temp_skew_fall					= 343,
LIBERTY_ATTRENUM_k_temp_setup_rise					= 344,
LIBERTY_ATTRENUM_k_temp_setup_fall					= 345,
LIBERTY_ATTRENUM_k_temp_rise_transition					= 346,
LIBERTY_ATTRENUM_k_temp_rise_propagation					= 347,
LIBERTY_ATTRENUM_k_temp_rise_pin_resistance					= 348,
LIBERTY_ATTRENUM_k_temp_rise_delay_intercept					= 349,
LIBERTY_ATTRENUM_k_temp_removal_rise					= 350,
LIBERTY_ATTRENUM_k_temp_removal_fall					= 351,
LIBERTY_ATTRENUM_k_temp_recovery_rise					= 352,
LIBERTY_ATTRENUM_k_temp_recovery_fall					= 353,
LIBERTY_ATTRENUM_k_temp_pin_cap					= 354,
LIBERTY_ATTRENUM_k_temp_nochange_rise					= 355,
LIBERTY_ATTRENUM_k_temp_nochange_fall					= 356,
LIBERTY_ATTRENUM_k_temp_min_pulse_width_low					= 357,
LIBERTY_ATTRENUM_k_temp_min_pulse_width_high					= 358,
LIBERTY_ATTRENUM_k_temp_min_period					= 359,
LIBERTY_ATTRENUM_k_temp_intrinsic_rise					= 360,
LIBERTY_ATTRENUM_k_temp_intrinsic_fall					= 361,
LIBERTY_ATTRENUM_k_temp_internal_power					= 362,
LIBERTY_ATTRENUM_k_temp_hold_rise					= 363,
LIBERTY_ATTRENUM_k_temp_hold_fall					= 364,
LIBERTY_ATTRENUM_k_temp_fall_transition					= 365,
LIBERTY_ATTRENUM_k_temp_fall_propagation					= 366,
LIBERTY_ATTRENUM_k_temp_fall_pin_resistance					= 367,
LIBERTY_ATTRENUM_k_temp_fall_delay_intercept					= 368,
LIBERTY_ATTRENUM_k_temp_drive_rise					= 369,
LIBERTY_ATTRENUM_k_temp_drive_fall					= 370,
LIBERTY_ATTRENUM_k_temp_drive_current					= 371,
LIBERTY_ATTRENUM_k_temp_cell_rise					= 372,
LIBERTY_ATTRENUM_k_temp_cell_leakage_power					= 373,
LIBERTY_ATTRENUM_k_temp_cell_fall					= 374,
LIBERTY_ATTRENUM_k_temp_cell_degradation					= 375,
LIBERTY_ATTRENUM_k_process_wire_res					= 376,
LIBERTY_ATTRENUM_k_process_wire_cap					= 377,
LIBERTY_ATTRENUM_k_process_slope_rise					= 378,
LIBERTY_ATTRENUM_k_process_slope_fall					= 379,
LIBERTY_ATTRENUM_k_process_skew_rise					= 380,
LIBERTY_ATTRENUM_k_process_skew_fall					= 381,
LIBERTY_ATTRENUM_k_process_setup_rise					= 382,
LIBERTY_ATTRENUM_k_process_setup_fall					= 383,
LIBERTY_ATTRENUM_k_process_rise_transition					= 384,
LIBERTY_ATTRENUM_k_process_rise_propagation					= 385,
LIBERTY_ATTRENUM_k_process_rise_pin_resistance					= 386,
LIBERTY_ATTRENUM_k_process_rise_delay_intercept					= 387,
LIBERTY_ATTRENUM_k_process_removal_rise					= 388,
LIBERTY_ATTRENUM_k_process_removal_fall					= 389,
LIBERTY_ATTRENUM_k_process_recovery_rise					= 390,
LIBERTY_ATTRENUM_k_process_recovery_fall					= 391,
LIBERTY_ATTRENUM_k_process_pin_rise_cap					= 392,
LIBERTY_ATTRENUM_k_process_pin_fall_cap					= 393,
LIBERTY_ATTRENUM_k_process_pin_cap					= 394,
LIBERTY_ATTRENUM_k_process_nochange_rise					= 395,
LIBERTY_ATTRENUM_k_process_nochange_fall					= 396,
LIBERTY_ATTRENUM_k_process_min_pulse_width_low					= 397,
LIBERTY_ATTRENUM_k_process_min_pulse_width_high					= 398,
LIBERTY_ATTRENUM_k_process_min_period					= 399,
LIBERTY_ATTRENUM_k_process_intrinsic_rise					= 400,
LIBERTY_ATTRENUM_k_process_intrinsic_fall					= 401,
LIBERTY_ATTRENUM_k_process_internal_power					= 402,
LIBERTY_ATTRENUM_k_process_hold_rise					= 403,
LIBERTY_ATTRENUM_k_process_hold_fall					= 404,
LIBERTY_ATTRENUM_k_process_fall_transition					= 405,
LIBERTY_ATTRENUM_k_process_fall_propagation					= 406,
LIBERTY_ATTRENUM_k_process_fall_pin_resistance					= 407,
LIBERTY_ATTRENUM_k_process_fall_delay_intercept					= 408,
LIBERTY_ATTRENUM_k_process_drive_rise					= 409,
LIBERTY_ATTRENUM_k_process_drive_fall					= 410,
LIBERTY_ATTRENUM_k_process_drive_current					= 411,
LIBERTY_ATTRENUM_k_process_cell_rise					= 412,
LIBERTY_ATTRENUM_k_process_cell_leakage_power					= 413,
LIBERTY_ATTRENUM_k_process_cell_fall					= 414,
LIBERTY_ATTRENUM_k_process_cell_degradation					= 415,
LIBERTY_ATTRENUM_input_threshold_pct_rise					= 416,
LIBERTY_ATTRENUM_input_threshold_pct_fall					= 417,
LIBERTY_ATTRENUM_in_place_swap_mode					= 418,
LIBERTY_ATTRENUM_em_temp_degradation_factor					= 419,
LIBERTY_ATTRENUM_delay_model					= 420,
LIBERTY_ATTRENUM_default_wire_load_selection					= 421,
LIBERTY_ATTRENUM_default_wire_load_resistance					= 422,
LIBERTY_ATTRENUM_default_wire_load_mode					= 423,
LIBERTY_ATTRENUM_default_wire_load_capacitance					= 424,
LIBERTY_ATTRENUM_default_wire_load_area					= 425,
LIBERTY_ATTRENUM_default_wire_load					= 426,
LIBERTY_ATTRENUM_default_slope_rise					= 427,
LIBERTY_ATTRENUM_default_slope_fall					= 428,
LIBERTY_ATTRENUM_default_setup_coefficient					= 429,
LIBERTY_ATTRENUM_default_rise_pin_resistance					= 430,
LIBERTY_ATTRENUM_default_rise_delay_intercept					= 431,
LIBERTY_ATTRENUM_default_reference_capacitance					= 432,
LIBERTY_ATTRENUM_default_rc_rise_coefficient					= 433,
LIBERTY_ATTRENUM_default_rc_fall_coefficient					= 434,
LIBERTY_ATTRENUM_default_output_pin_rise_res					= 435,
LIBERTY_ATTRENUM_default_output_pin_fall_res					= 436,
LIBERTY_ATTRENUM_default_output_pin_cap					= 437,
LIBERTY_ATTRENUM_default_operating_conditions					= 438,
LIBERTY_ATTRENUM_default_min_porosity					= 439,
LIBERTY_ATTRENUM_default_max_utilization					= 440,
LIBERTY_ATTRENUM_default_max_transition					= 441,
LIBERTY_ATTRENUM_default_max_fanout					= 442,
LIBERTY_ATTRENUM_default_max_capacitance					= 443,
LIBERTY_ATTRENUM_default_leakage_power_density					= 444,
LIBERTY_ATTRENUM_default_intrinsic_rise					= 445,
LIBERTY_ATTRENUM_default_intrinsic_fall					= 446,
LIBERTY_ATTRENUM_default_input_pin_cap					= 447,
LIBERTY_ATTRENUM_default_inout_pin_rise_res					= 448,
LIBERTY_ATTRENUM_default_inout_pin_fall_res					= 449,
LIBERTY_ATTRENUM_default_inout_pin_cap					= 450,
LIBERTY_ATTRENUM_default_hold_coefficient					= 451,
LIBERTY_ATTRENUM_default_fanout_load					= 452,
LIBERTY_ATTRENUM_default_fall_pin_resistance					= 453,
LIBERTY_ATTRENUM_default_fall_delay_intercept					= 454,
LIBERTY_ATTRENUM_default_edge_rate_breakpoint_r1					= 455,
LIBERTY_ATTRENUM_default_edge_rate_breakpoint_r0					= 456,
LIBERTY_ATTRENUM_default_edge_rate_breakpoint_f1					= 457,
LIBERTY_ATTRENUM_default_edge_rate_breakpoint_f0					= 458,
LIBERTY_ATTRENUM_default_connection_class					= 459,
LIBERTY_ATTRENUM_default_cell_leakage_power					= 460,
LIBERTY_ATTRENUM_date					= 461,
LIBERTY_ATTRENUM_current_unit					= 462,
LIBERTY_ATTRENUM_comment					= 463,
LIBERTY_ATTRENUM_bus_naming_style					= 464,
} attr_enum;
