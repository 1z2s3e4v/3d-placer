/* ANSI-C code produced by gperf version 3.0.3 */
/* Command-line: gperf -L ANSI-C -t -H group_hash_func -N lookup_group_name -C -I -k '*' group_lookup  */

#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))
/* The character set is not based on ISO-646.  */
#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

#line 1 "group_lookup"

#include "group_enum.h"
#line 4 "group_lookup"
struct libGroupMap { char *name; group_enum type; };
#include <string.h>

#define TOTAL_KEYWORDS 95
#define MIN_WORD_LENGTH 2
#define MAX_WORD_LENGTH 43
#define MIN_HASH_VALUE 4
#define MAX_HASH_VALUE 459
/* maximum key range = 456, duplicates = 0 */

#ifdef __GNUC__
__inline
#else
#ifdef __cplusplus
inline
#endif
#endif
static unsigned int
group_hash_func (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460,   0, 460,   0,  60,   0,
        5,   0, 112,  30,  45,   5, 460,  25,   0,  30,
        0,   0,  30,  45,   0,   0,   0,  20,  15,   0,
        5,   5, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460, 460, 460, 460, 460,
      460, 460, 460, 460, 460, 460
    };
  register int hval = len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[42]];
      /*FALLTHROUGH*/
      case 42:
        hval += asso_values[(unsigned char)str[41]];
      /*FALLTHROUGH*/
      case 41:
        hval += asso_values[(unsigned char)str[40]];
      /*FALLTHROUGH*/
      case 40:
        hval += asso_values[(unsigned char)str[39]];
      /*FALLTHROUGH*/
      case 39:
        hval += asso_values[(unsigned char)str[38]];
      /*FALLTHROUGH*/
      case 38:
        hval += asso_values[(unsigned char)str[37]];
      /*FALLTHROUGH*/
      case 37:
        hval += asso_values[(unsigned char)str[36]];
      /*FALLTHROUGH*/
      case 36:
        hval += asso_values[(unsigned char)str[35]];
      /*FALLTHROUGH*/
      case 35:
        hval += asso_values[(unsigned char)str[34]];
      /*FALLTHROUGH*/
      case 34:
        hval += asso_values[(unsigned char)str[33]];
      /*FALLTHROUGH*/
      case 33:
        hval += asso_values[(unsigned char)str[32]];
      /*FALLTHROUGH*/
      case 32:
        hval += asso_values[(unsigned char)str[31]];
      /*FALLTHROUGH*/
      case 31:
        hval += asso_values[(unsigned char)str[30]];
      /*FALLTHROUGH*/
      case 30:
        hval += asso_values[(unsigned char)str[29]];
      /*FALLTHROUGH*/
      case 29:
        hval += asso_values[(unsigned char)str[28]];
      /*FALLTHROUGH*/
      case 28:
        hval += asso_values[(unsigned char)str[27]];
      /*FALLTHROUGH*/
      case 27:
        hval += asso_values[(unsigned char)str[26]];
      /*FALLTHROUGH*/
      case 26:
        hval += asso_values[(unsigned char)str[25]];
      /*FALLTHROUGH*/
      case 25:
        hval += asso_values[(unsigned char)str[24]];
      /*FALLTHROUGH*/
      case 24:
        hval += asso_values[(unsigned char)str[23]];
      /*FALLTHROUGH*/
      case 23:
        hval += asso_values[(unsigned char)str[22]];
      /*FALLTHROUGH*/
      case 22:
        hval += asso_values[(unsigned char)str[21]];
      /*FALLTHROUGH*/
      case 21:
        hval += asso_values[(unsigned char)str[20]];
      /*FALLTHROUGH*/
      case 20:
        hval += asso_values[(unsigned char)str[19]];
      /*FALLTHROUGH*/
      case 19:
        hval += asso_values[(unsigned char)str[18]];
      /*FALLTHROUGH*/
      case 18:
        hval += asso_values[(unsigned char)str[17]];
      /*FALLTHROUGH*/
      case 17:
        hval += asso_values[(unsigned char)str[16]];
      /*FALLTHROUGH*/
      case 16:
        hval += asso_values[(unsigned char)str[15]];
      /*FALLTHROUGH*/
      case 15:
        hval += asso_values[(unsigned char)str[14]];
      /*FALLTHROUGH*/
      case 14:
        hval += asso_values[(unsigned char)str[13]];
      /*FALLTHROUGH*/
      case 13:
        hval += asso_values[(unsigned char)str[12]];
      /*FALLTHROUGH*/
      case 12:
        hval += asso_values[(unsigned char)str[11]];
      /*FALLTHROUGH*/
      case 11:
        hval += asso_values[(unsigned char)str[10]];
      /*FALLTHROUGH*/
      case 10:
        hval += asso_values[(unsigned char)str[9]];
      /*FALLTHROUGH*/
      case 9:
        hval += asso_values[(unsigned char)str[8]];
      /*FALLTHROUGH*/
      case 8:
        hval += asso_values[(unsigned char)str[7]];
      /*FALLTHROUGH*/
      case 7:
        hval += asso_values[(unsigned char)str[6]];
      /*FALLTHROUGH*/
      case 6:
        hval += asso_values[(unsigned char)str[5]];
      /*FALLTHROUGH*/
      case 5:
        hval += asso_values[(unsigned char)str[4]];
      /*FALLTHROUGH*/
      case 4:
        hval += asso_values[(unsigned char)str[3]];
      /*FALLTHROUGH*/
      case 3:
        hval += asso_values[(unsigned char)str[2]];
      /*FALLTHROUGH*/
      case 2:
        hval += asso_values[(unsigned char)str[1]];
      /*FALLTHROUGH*/
      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

#ifdef __GNUC__
__inline
#ifdef __GNUC_STDC_INLINE__
__attribute__ ((__gnu_inline__))
#endif
#endif
const struct libGroupMap *
lookup_group_name (register const char *str, register unsigned int len)
{
  static const struct libGroupMap wordlist[] =
    {
      {""}, {""}, {""}, {""},
#line 99 "group_lookup"
      {"cell",	LIBERTY_GROUPENUM_cell},
#line 34 "group_lookup"
      {"state",	LIBERTY_GROUPENUM_state},
      {""}, {""}, {""},
#line 32 "group_lookup"
      {"test_cell",	LIBERTY_GROUPENUM_test_cell},
      {""}, {""}, {""}, {""},
#line 78 "group_lookup"
      {"cell_rise",	LIBERTY_GROUPENUM_cell_rise},
      {""},
#line 13 "group_lookup"
      {"scaled_cell",	LIBERTY_GROUPENUM_scaled_cell},
      {""}, {""},
#line 10 "group_lookup"
      {"wire_load",	LIBERTY_GROUPENUM_wire_load},
      {""}, {""}, {""},
#line 6 "group_lookup"
      {"lut",	LIBERTY_GROUPENUM_lut},
      {""},
#line 54 "group_lookup"
      {"rise_constraint",	LIBERTY_GROUPENUM_rise_constraint},
#line 57 "group_lookup"
      {"retain_rise_slew",	LIBERTY_GROUPENUM_retain_rise_slew},
      {""}, {""},
#line 15 "group_lookup"
      {"rise_net_delay",	LIBERTY_GROUPENUM_rise_net_delay},
#line 52 "group_lookup"
      {"rise_transition",	LIBERTY_GROUPENUM_rise_transition},
      {""}, {""},
#line 9 "group_lookup"
      {"interconnect_delay",	LIBERTY_GROUPENUM_interconnect_delay},
#line 8 "group_lookup"
      {"wire_load_selection",	LIBERTY_GROUPENUM_wire_load_selection},
#line 88 "group_lookup"
      {"power",	LIBERTY_GROUPENUM_power},
      {""}, {""},
#line 82 "group_lookup"
      {"pin",	LIBERTY_GROUPENUM_pin},
#line 31 "group_lookup"
      {"type",	LIBERTY_GROUPENUM_type},
#line 23 "group_lookup"
      {"model",	LIBERTY_GROUPENUM_model},
      {""}, {""}, {""}, {""},
#line 87 "group_lookup"
      {"rise_power",	LIBERTY_GROUPENUM_rise_power},
#line 95 "group_lookup"
      {"domain",	LIBERTY_GROUPENUM_domain},
      {""},
#line 36 "group_lookup"
      {"seq",	LIBERTY_GROUPENUM_seq},
#line 90 "group_lookup"
      {"internal_power",	LIBERTY_GROUPENUM_internal_power},
#line 43 "group_lookup"
      {"latch",	LIBERTY_GROUPENUM_latch},
#line 48 "group_lookup"
      {"tlatch",	LIBERTY_GROUPENUM_tlatch},
      {""}, {""},
#line 50 "group_lookup"
      {"steady_state_current_low",	LIBERTY_GROUPENUM_steady_state_current_low},
      {""}, {""}, {""}, {""},
#line 55 "group_lookup"
      {"retaining_rise",	LIBERTY_GROUPENUM_retaining_rise},
      {""},
#line 80 "group_lookup"
      {"cell_degradation",	LIBERTY_GROUPENUM_cell_degradation},
      {""}, {""},
#line 49 "group_lookup"
      {"steady_state_current_tristate",	LIBERTY_GROUPENUM_steady_state_current_tristate},
      {""}, {""}, {""}, {""}, {""},
#line 33 "group_lookup"
      {"statetable",	LIBERTY_GROUPENUM_statetable},
#line 40 "group_lookup"
      {"memory",	LIBERTY_GROUPENUM_memory},
      {""}, {""}, {""},
#line 44 "group_lookup"
      {"generated_clock",	LIBERTY_GROUPENUM_generated_clock},
#line 81 "group_lookup"
      {"timing",	LIBERTY_GROUPENUM_timing},
#line 100 "group_lookup"
      {"library",	LIBERTY_GROUPENUM_library},
      {""}, {""},
#line 38 "group_lookup"
      {"mode_value",	LIBERTY_GROUPENUM_mode_value},
#line 86 "group_lookup"
      {"memory_read",	LIBERTY_GROUPENUM_memory_read},
#line 85 "group_lookup"
      {"memory_write",	LIBERTY_GROUPENUM_memory_write},
#line 47 "group_lookup"
      {"bus",	LIBERTY_GROUPENUM_bus},
      {""},
#line 7 "group_lookup"
      {"wire_load_table",	LIBERTY_GROUPENUM_wire_load_table},
#line 97 "group_lookup"
      {"electromigration",	LIBERTY_GROUPENUM_electromigration},
#line 14 "group_lookup"
      {"rise_transition_degradation",	LIBERTY_GROUPENUM_rise_transition_degradation},
      {""}, {""}, {""},
#line 98 "group_lookup"
      {"bundle",	LIBERTY_GROUPENUM_bundle},
      {""},
#line 37 "group_lookup"
      {"routing_track",	LIBERTY_GROUPENUM_routing_track},
      {""}, {""}, {""}, {""},
#line 41 "group_lookup"
      {"leakage_power",	LIBERTY_GROUPENUM_leakage_power},
      {""},
#line 21 "group_lookup"
      {"operating_conditions",	LIBERTY_GROUPENUM_operating_conditions},
      {""}, {""},
#line 22 "group_lookup"
      {"noise_lut_template",	LIBERTY_GROUPENUM_noise_lut_template},
      {""}, {""}, {""}, {""},
#line 30 "group_lookup"
      {"poly_template",	LIBERTY_GROUPENUM_poly_template},
      {""}, {""}, {""},
#line 11 "group_lookup"
      {"timing_range",	LIBERTY_GROUPENUM_timing_range},
#line 26 "group_lookup"
      {"input_voltage",	LIBERTY_GROUPENUM_input_voltage},
      {""},
#line 25 "group_lookup"
      {"iv_lut_template",	LIBERTY_GROUPENUM_iv_lut_template},
#line 53 "group_lookup"
      {"rise_propagation",	LIBERTY_GROUPENUM_rise_propagation},
      {""},
#line 71 "group_lookup"
      {"noise_immunity_low",	LIBERTY_GROUPENUM_noise_immunity_low},
      {""}, {""},
#line 79 "group_lookup"
      {"cell_fall",	LIBERTY_GROUPENUM_cell_fall},
      {""}, {""}, {""},
#line 29 "group_lookup"
      {"em_lut_template",	LIBERTY_GROUPENUM_em_lut_template},
      {""},
#line 17 "group_lookup"
      {"power_supply",	LIBERTY_GROUPENUM_power_supply},
#line 19 "group_lookup"
      {"power_lut_template",	LIBERTY_GROUPENUM_power_lut_template},
#line 20 "group_lookup"
      {"output_voltage",	LIBERTY_GROUPENUM_output_voltage},
      {""}, {""},
#line 77 "group_lookup"
      {"fall_constraint",	LIBERTY_GROUPENUM_fall_constraint},
#line 58 "group_lookup"
      {"retain_fall_slew",	LIBERTY_GROUPENUM_retain_fall_slew},
      {""}, {""},
#line 28 "group_lookup"
      {"fall_net_delay",	LIBERTY_GROUPENUM_fall_net_delay},
#line 75 "group_lookup"
      {"fall_transition",	LIBERTY_GROUPENUM_fall_transition},
#line 35 "group_lookup"
      {"seq_bank",	LIBERTY_GROUPENUM_seq_bank},
      {""},
#line 42 "group_lookup"
      {"latch_bank",	LIBERTY_GROUPENUM_latch_bank},
      {""}, {""},
#line 96 "group_lookup"
      {"em_max_toggle_rate",	LIBERTY_GROUPENUM_em_max_toggle_rate},
#line 18 "group_lookup"
      {"power_poly_template",	LIBERTY_GROUPENUM_power_poly_template},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 89 "group_lookup"
      {"fall_power",	LIBERTY_GROUPENUM_fall_power},
      {""}, {""},
#line 84 "group_lookup"
      {"min_pulse_width",	LIBERTY_GROUPENUM_min_pulse_width},
      {""},
#line 24 "group_lookup"
      {"lu_table_template",	LIBERTY_GROUPENUM_lu_table_template},
      {""}, {""}, {""}, {""},
#line 12 "group_lookup"
      {"scaling_factors",	LIBERTY_GROUPENUM_scaling_factors},
      {""}, {""}, {""},
#line 56 "group_lookup"
      {"retaining_fall",	LIBERTY_GROUPENUM_retaining_fall},
      {""}, {""}, {""},
#line 91 "group_lookup"
      {"hyperbolic_noise_low",	LIBERTY_GROUPENUM_hyperbolic_noise_low},
      {""}, {""}, {""},
#line 83 "group_lookup"
      {"minimum_period",	LIBERTY_GROUPENUM_minimum_period},
      {""}, {""}, {""}, {""}, {""},
#line 51 "group_lookup"
      {"steady_state_current_high",	LIBERTY_GROUPENUM_steady_state_current_high},
#line 59 "group_lookup"
      {"propagated_noise_width_low",	LIBERTY_GROUPENUM_propagated_noise_width_low},
#line 39 "group_lookup"
      {"mode_definition",	LIBERTY_GROUPENUM_mode_definition},
      {""},
#line 73 "group_lookup"
      {"noise_immunity_below_low",	LIBERTY_GROUPENUM_noise_immunity_below_low},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 27 "group_lookup"
      {"fall_transition_degradation",	LIBERTY_GROUPENUM_fall_transition_degradation},
      {""}, {""}, {""}, {""},
#line 16 "group_lookup"
      {"propagation_lut_template",	LIBERTY_GROUPENUM_propagation_lut_template},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""},
#line 76 "group_lookup"
      {"fall_propagation",	LIBERTY_GROUPENUM_fall_propagation},
      {""}, {""},
#line 46 "group_lookup"
      {"ff",	LIBERTY_GROUPENUM_ff},
      {""}, {""}, {""}, {""},
#line 63 "group_lookup"
      {"propagated_noise_peak_time_ratio_low",	LIBERTY_GROUPENUM_propagated_noise_peak_time_ratio_low},
      {""}, {""}, {""}, {""},
#line 93 "group_lookup"
      {"hyperbolic_noise_below_low",	LIBERTY_GROUPENUM_hyperbolic_noise_below_low},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 72 "group_lookup"
      {"noise_immunity_high",	LIBERTY_GROUPENUM_noise_immunity_high},
      {""}, {""},
#line 61 "group_lookup"
      {"propagated_noise_width_below_low",	LIBERTY_GROUPENUM_propagated_noise_width_below_low},
      {""}, {""}, {""}, {""},
#line 67 "group_lookup"
      {"propagated_noise_height_low",	LIBERTY_GROUPENUM_propagated_noise_height_low},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 92 "group_lookup"
      {"hyperbolic_noise_high",	LIBERTY_GROUPENUM_hyperbolic_noise_high},
#line 65 "group_lookup"
      {"propagated_noise_peak_time_ratio_below_low",	LIBERTY_GROUPENUM_propagated_noise_peak_time_ratio_below_low},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 60 "group_lookup"
      {"propagated_noise_width_high",	LIBERTY_GROUPENUM_propagated_noise_width_high},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 45 "group_lookup"
      {"ff_bank",	LIBERTY_GROUPENUM_ff_bank},
      {""},
#line 69 "group_lookup"
      {"propagated_noise_height_below_low",	LIBERTY_GROUPENUM_propagated_noise_height_below_low},
      {""}, {""}, {""}, {""}, {""}, {""},
#line 74 "group_lookup"
      {"noise_immunity_above_high",	LIBERTY_GROUPENUM_noise_immunity_above_high},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 64 "group_lookup"
      {"propagated_noise_peak_time_ratio_high",	LIBERTY_GROUPENUM_propagated_noise_peak_time_ratio_high},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""},
#line 94 "group_lookup"
      {"hyperbolic_noise_above_high",	LIBERTY_GROUPENUM_hyperbolic_noise_above_high},
#line 68 "group_lookup"
      {"propagated_noise_height_high",	LIBERTY_GROUPENUM_propagated_noise_height_high},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
#line 62 "group_lookup"
      {"propagated_noise_width_above_high",	LIBERTY_GROUPENUM_propagated_noise_width_above_high},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""},
#line 66 "group_lookup"
      {"propagated_noise_peak_time_ratio_above_high",	LIBERTY_GROUPENUM_propagated_noise_peak_time_ratio_above_high},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""}, {""},
      {""}, {""},
#line 70 "group_lookup"
      {"propagated_noise_height_above_high",	LIBERTY_GROUPENUM_propagated_noise_height_above_high}
    };

  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = group_hash_func (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register const char *s = wordlist[key].name;

          if (*str == *s && !strcmp (str + 1, s + 1))
            return &wordlist[key];
        }
    }
  return 0;
}
#line 101 "group_lookup"

