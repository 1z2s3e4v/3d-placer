

typedef enum
{
LIBERTY_GROUPENUM_lut					= 1,
LIBERTY_GROUPENUM_wire_load_table					= 2,
LIBERTY_GROUPENUM_wire_load_selection					= 3,
LIBERTY_GROUPENUM_interconnect_delay					= 4,
LIBERTY_GROUPENUM_wire_load					= 5,
LIBERTY_GROUPENUM_timing_range					= 6,
LIBERTY_GROUPENUM_scaling_factors					= 7,
LIBERTY_GROUPENUM_scaled_cell					= 8,
LIBERTY_GROUPENUM_rise_transition_degradation					= 9,
LIBERTY_GROUPENUM_rise_net_delay					= 10,
LIBERTY_GROUPENUM_propagation_lut_template					= 11,
LIBERTY_GROUPENUM_power_supply					= 12,
LIBERTY_GROUPENUM_power_poly_template					= 13,
LIBERTY_GROUPENUM_power_lut_template					= 14,
LIBERTY_GROUPENUM_output_voltage					= 15,
LIBERTY_GROUPENUM_operating_conditions					= 16,
LIBERTY_GROUPENUM_noise_lut_template					= 17,
LIBERTY_GROUPENUM_model					= 18,
LIBERTY_GROUPENUM_lu_table_template					= 19,
LIBERTY_GROUPENUM_iv_lut_template					= 20,
LIBERTY_GROUPENUM_input_voltage					= 21,
LIBERTY_GROUPENUM_fall_transition_degradation					= 22,
LIBERTY_GROUPENUM_fall_net_delay					= 23,
LIBERTY_GROUPENUM_em_lut_template					= 24,
LIBERTY_GROUPENUM_poly_template					= 25,
LIBERTY_GROUPENUM_type					= 26,
LIBERTY_GROUPENUM_test_cell					= 27,
LIBERTY_GROUPENUM_statetable					= 28,
LIBERTY_GROUPENUM_state					= 29,
LIBERTY_GROUPENUM_seq_bank					= 30,
LIBERTY_GROUPENUM_seq					= 31,
LIBERTY_GROUPENUM_routing_track					= 32,
LIBERTY_GROUPENUM_mode_value					= 33,
LIBERTY_GROUPENUM_mode_definition					= 34,
LIBERTY_GROUPENUM_memory					= 35,
LIBERTY_GROUPENUM_leakage_power					= 36,
LIBERTY_GROUPENUM_latch_bank					= 37,
LIBERTY_GROUPENUM_latch					= 38,
LIBERTY_GROUPENUM_generated_clock					= 39,
LIBERTY_GROUPENUM_ff_bank					= 40,
LIBERTY_GROUPENUM_ff					= 41,
LIBERTY_GROUPENUM_bus					= 42,
LIBERTY_GROUPENUM_tlatch					= 43,
LIBERTY_GROUPENUM_steady_state_current_tristate					= 44,
LIBERTY_GROUPENUM_steady_state_current_low					= 45,
LIBERTY_GROUPENUM_steady_state_current_high					= 46,
LIBERTY_GROUPENUM_rise_transition					= 47,
LIBERTY_GROUPENUM_rise_propagation					= 48,
LIBERTY_GROUPENUM_rise_constraint					= 49,
LIBERTY_GROUPENUM_retaining_rise					= 50,
LIBERTY_GROUPENUM_retaining_fall					= 51,
LIBERTY_GROUPENUM_retain_rise_slew					= 52,
LIBERTY_GROUPENUM_retain_fall_slew					= 53,
LIBERTY_GROUPENUM_propagated_noise_width_low					= 54,
LIBERTY_GROUPENUM_propagated_noise_width_high					= 55,
LIBERTY_GROUPENUM_propagated_noise_width_below_low					= 56,
LIBERTY_GROUPENUM_propagated_noise_width_above_high					= 57,
LIBERTY_GROUPENUM_propagated_noise_peak_time_ratio_low					= 58,
LIBERTY_GROUPENUM_propagated_noise_peak_time_ratio_high					= 59,
LIBERTY_GROUPENUM_propagated_noise_peak_time_ratio_below_low					= 60,
LIBERTY_GROUPENUM_propagated_noise_peak_time_ratio_above_high					= 61,
LIBERTY_GROUPENUM_propagated_noise_height_low					= 62,
LIBERTY_GROUPENUM_propagated_noise_height_high					= 63,
LIBERTY_GROUPENUM_propagated_noise_height_below_low					= 64,
LIBERTY_GROUPENUM_propagated_noise_height_above_high					= 65,
LIBERTY_GROUPENUM_noise_immunity_low					= 66,
LIBERTY_GROUPENUM_noise_immunity_high					= 67,
LIBERTY_GROUPENUM_noise_immunity_below_low					= 68,
LIBERTY_GROUPENUM_noise_immunity_above_high					= 69,
LIBERTY_GROUPENUM_fall_transition					= 70,
LIBERTY_GROUPENUM_fall_propagation					= 71,
LIBERTY_GROUPENUM_fall_constraint					= 72,
LIBERTY_GROUPENUM_cell_rise					= 73,
LIBERTY_GROUPENUM_cell_fall					= 74,
LIBERTY_GROUPENUM_cell_degradation					= 75,
LIBERTY_GROUPENUM_timing					= 76,
LIBERTY_GROUPENUM_pin					= 77,
LIBERTY_GROUPENUM_minimum_period					= 78,
LIBERTY_GROUPENUM_min_pulse_width					= 79,
LIBERTY_GROUPENUM_memory_write					= 80,
LIBERTY_GROUPENUM_memory_read					= 81,
LIBERTY_GROUPENUM_rise_power					= 82,
LIBERTY_GROUPENUM_power					= 83,
LIBERTY_GROUPENUM_fall_power					= 84,
LIBERTY_GROUPENUM_internal_power					= 85,
LIBERTY_GROUPENUM_hyperbolic_noise_low					= 86,
LIBERTY_GROUPENUM_hyperbolic_noise_high					= 87,
LIBERTY_GROUPENUM_hyperbolic_noise_below_low					= 88,
LIBERTY_GROUPENUM_hyperbolic_noise_above_high					= 89,
LIBERTY_GROUPENUM_domain					= 90,
LIBERTY_GROUPENUM_em_max_toggle_rate					= 91,
LIBERTY_GROUPENUM_electromigration					= 92,
LIBERTY_GROUPENUM_bundle					= 93,
LIBERTY_GROUPENUM_cell					= 94,
LIBERTY_GROUPENUM_library					= 95,
} group_enum;

