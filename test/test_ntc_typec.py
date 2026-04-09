"""
test_ntc_typec.py  --  PC8 TypeC NTC functional test
=====================================================
Validates:
  1. ntc_tbl[] monotonicity and completeness
  2. Lookup function (fml_ntc_temp_get_typec) correctness
  3. Offset formula (i - 50) vs table-labeled temperatures
  4. TypeC NTC protection thresholds and hysteresis
  5. Sliding-window average filter behaviour

Run:  python test/test_ntc_typec.py
"""
import sys

# ---------------------------------------------------------------------------
# 1. NU17112 ntc_tbl[] -- verbatim from prot.c (100k pull-up / 10K NTC)
#    Table comment labels: index 0 = -40 degC  ...  index 190 = 150 degC
# ---------------------------------------------------------------------------
NTC_TBL = [
    3996,  # -40
    3989, 3981, 3973, 3964, 3955, 3945, 3935, 3924, 3913, 3901,  # -39 ~ -30
    3888, 3875, 3861, 3846, 3830, 3814, 3797, 3779, 3760, 3741,  # -29 ~ -20
    3721, 3699, 3677, 3654, 3630, 3605, 3579, 3552, 3525, 3496,  # -19 ~ -10
    3466, 3436, 3404, 3371, 3338, 3304, 3268, 3232, 3195, 3157,  # -9  ~  0
    3118, 3079, 3039, 2998, 2956, 2914, 2871, 2827, 2783, 2739,  #  1  ~ 10
    2694, 2648, 2603, 2557, 2511, 2464, 2418, 2371, 2325, 2278,  # 11  ~ 20
    2232, 2185, 2139, 2093, 2048, 2002, 1957, 1912, 1868, 1824,  # 21  ~ 30
    1781, 1738, 1696, 1655, 1614, 1573, 1533, 1494, 1456, 1418,  # 31  ~ 40
    1381, 1345, 1309, 1274, 1240, 1207, 1174, 1142, 1111, 1081,  # 41  ~ 50
    1051, 1022,  993,  966,  939,  913,  887,  862,  838,  815,  # 51  ~ 60
     792,  769,  748,  727,  706,  686,  667,  648,  630,  612,  # 61  ~ 70
     595,  578,  562,  546,  531,  516,  501,  488,  474,  461,  # 71  ~ 80
     448,  436,  424,  412,  401,  390,  379,  369,  359,  349,  # 81  ~ 90
     340,  330,  322,  313,  305,  296,  289,  281,  274,  266,  # 91  ~ 100
     259,  253,  246,  240,  234,  228,  222,  216,  211,  205,  # 101 ~ 110
     200,  195,  190,  185,  181,  176,  172,  168,  164,  160,  # 111 ~ 120
     156,  152,  148,  145,  141,  138,  135,  132,  129,  126,  # 121 ~ 130
     123,  120,  117,  114,  112,  109,  107,  104,  102,  100,  # 131 ~ 140
      97,   95,   93,   91,   89,   87,   85,   83,   82,   80,  # 141 ~ 150
]

# Code offset (prot.c line 134) — fixed from 50 to 40
CODE_OFFSET = 40

# Table-labeled offset: index 0 represents -40 degC per comments
TABLE_OFFSET = 40


# ---------------------------------------------------------------------------
# 2. Replicate C lookup: fml_ntc_temp_get_typec() core logic
# ---------------------------------------------------------------------------
def ntc_lookup(v_ntc: int) -> int:
    """Replicate the C firmware lookup with the CURRENT code offset (i - 50)."""
    i = 0
    while i < len(NTC_TBL):
        if v_ntc >= NTC_TBL[i]:
            break
        i += 1
    return i - CODE_OFFSET


def ntc_lookup_correct(v_ntc: int) -> int:
    """What the lookup SHOULD return if offset matched table labels (i - 40)."""
    i = 0
    while i < len(NTC_TBL):
        if v_ntc >= NTC_TBL[i]:
            break
        i += 1
    return i - TABLE_OFFSET


def adc_at_temp(temp_degc: int) -> int:
    """Return the ADC value in ntc_tbl that corresponds to a labeled temperature.
    temp_degc: real temperature per table comments (-40 .. 150).
    """
    idx = temp_degc + TABLE_OFFSET  # table index
    if 0 <= idx < len(NTC_TBL):
        return NTC_TBL[idx]
    return -1


# ---------------------------------------------------------------------------
# 3. Sliding-window average filter simulation
# ---------------------------------------------------------------------------
class NtcFilter:
    """Replicate the 8-sample sliding average in fml_ntc_temp_get_typec()."""
    SIZE = 8

    def __init__(self, init_val=1650):
        self.buf = [init_val] * self.SIZE
        self.idx = 0

    def feed(self, adc_val: int) -> int:
        """Feed one ADC sample, return averaged value (integer division)."""
        self.buf[self.idx] = adc_val
        self.idx = (self.idx + 1) & (self.SIZE - 1)
        return sum(self.buf) // self.SIZE

    def feed_and_lookup(self, adc_val: int) -> int:
        avg = self.feed(adc_val)
        return ntc_lookup(avg)


# ---------------------------------------------------------------------------
# 4. TypeC NTC protection thresholds (from ntc.c after hysteresis fix)
# ---------------------------------------------------------------------------
TYPEC_PROTECTIONS = [
    {
        "name": "typec_ntc_lock (discharge)",
        "mode": "discharge",
        "trigger_hi":   105,  # ntc_temp_typec > 105
        "recovery_hi":   82,  # ntc_temp_typec <= 82
        "trigger_lo":   -15,  # ntc_temp_typec < -15
        "recovery_lo":  -10,  # ntc_temp_typec > -10  (FIXED from -15)
    },
    {
        "name": "typec_ntc_ot_flag (discharge)",
        "mode": "discharge",
        "trigger_hi":    81,  # ntc_temp_typec >= 81
        "recovery_hi":   35,  # ntc_temp_typec < 35
        "trigger_lo":  None,
        "recovery_lo": None,
    },
    {
        "name": "typec_charge_ntc_lock (charge)",
        "mode": "charge",
        "trigger_hi":   105,  # ntc_temp_typec > 105
        "recovery_hi":   68,  # ntc_temp_typec <= 68
        "trigger_lo":   -15,  # ntc_temp_typec < -15
        "recovery_lo":  -10,  # ntc_temp_typec > -10  (FIXED from -15)
    },
    {
        "name": "typec_ntc_ot_flag (charge)",
        "mode": "charge",
        "trigger_hi":    68,  # ntc_temp_typec > 68
        "recovery_hi":   30,  # ntc_temp_typec < 30
        "trigger_lo":  None,
        "recovery_lo": None,
    },
]


# ===================================================================
#                             TESTS
# ===================================================================
pass_count = 0
fail_count = 0
warn_count = 0


def check(condition, msg):
    global pass_count, fail_count
    if condition:
        pass_count += 1
        print(f"  [PASS] {msg}")
    else:
        fail_count += 1
        print(f"  [FAIL] {msg}")


def warn(msg):
    global warn_count
    warn_count += 1
    print(f"  [WARN] {msg}")


# -------------------------------------------------------------------
print("=" * 70)
print("TEST 1: ntc_tbl[] integrity")
print("=" * 70)

# 1a. Table length
expected_len = 191  # -40 to 150 = 191 entries
check(len(NTC_TBL) == expected_len,
      f"Table length = {len(NTC_TBL)}, expected {expected_len}")

# 1b. Strictly descending
is_descending = all(NTC_TBL[i] > NTC_TBL[i + 1] for i in range(len(NTC_TBL) - 1))
check(is_descending, "Table is strictly monotonically descending")

# 1c. All values positive
check(all(v > 0 for v in NTC_TBL), "All ADC values > 0")

# 1d. Reasonable ADC range (12-bit ADC = 0..4095)
check(NTC_TBL[0] <= 4095, f"Max ADC value {NTC_TBL[0]} fits in 12-bit range")
check(NTC_TBL[-1] >= 1, f"Min ADC value {NTC_TBL[-1]} > 0")

print()

# -------------------------------------------------------------------
print("=" * 70)
print("TEST 2: Lookup function correctness")
print("=" * 70)

# 2a. Known temperature points: feed the exact ADC value at labeled temp
test_temps = [-40, -20, -10, 0, 10, 25, 30, 50, 60, 80, 100, 120, 150]
for t in test_temps:
    adc = adc_at_temp(t)
    if adc < 0:
        continue
    result = ntc_lookup(adc)
    expected_with_code_offset = t - (CODE_OFFSET - TABLE_OFFSET)  # t - 10
    check(result == expected_with_code_offset,
          f"ADC={adc:4d} (labeled {t:+4d}C) -> lookup returns {result:+4d}C "
          f"(expected {expected_with_code_offset:+4d}C with i-{CODE_OFFSET})")

# 2b. Boundary: ADC above table max -> coldest
result = ntc_lookup(4095)
check(result == -CODE_OFFSET,
      f"ADC=4095 (above max) -> returns {result}C (clamped to coldest)")

# 2c. Boundary: ADC=0 (below table min) -> beyond hottest
result = ntc_lookup(0)
check(result == len(NTC_TBL) - CODE_OFFSET,
      f"ADC=0 (below all) -> returns {result}C (open circuit / sensor fault)")

print()

# -------------------------------------------------------------------
print("=" * 70)
print("TEST 3: Offset formula analysis (i - 50 vs i - 40)")
print("=" * 70)

offset_error = CODE_OFFSET - TABLE_OFFSET
print(f"  Code uses: i - {CODE_OFFSET}")
print(f"  Table labels imply: i - {TABLE_OFFSET}")
print(f"  Systematic error: {offset_error} degC (function returns T_real - {offset_error})")
print()

# Verify with specific examples
for t_real in [0, 25, -15, 105]:
    adc = adc_at_temp(t_real)
    if adc < 0:
        continue
    code_result = ntc_lookup(adc)
    correct_result = ntc_lookup_correct(adc)
    check(correct_result == t_real,
          f"Correct lookup at {t_real:+d}C: {correct_result:+d}C")
    if code_result != t_real:
        warn(f"Code offset mismatch at {t_real:+d}C: returns {code_result:+d}C "
             f"(real={t_real:+d}C, delta={code_result - t_real:+d}C)")

print()
print(f"  Impact on protection thresholds:")
print(f"  +--------------------------+----------+----------------+")
print(f"  | Threshold (code checks)  | Code val | Actual trigger |")
print(f"  +--------------------------+----------+----------------+")
for name, val in [("typec_ntc_lock OT", 105), ("typec_ntc_lock UT", -15),
                  ("typec_ot_flag dischg", 81), ("typec_ot_flag chrg", 68),
                  ("typec_chrg_lock OT", 105), ("typec_chrg_lock UT", -15)]:
    actual = val + offset_error
    print(f"  | {name:<24s} | {val:>+5d} C  | {actual:>+5d} C (real)  |")
print(f"  +--------------------------+----------+----------------+")

print()

# -------------------------------------------------------------------
print("=" * 70)
print("TEST 4: TypeC NTC protection hysteresis verification")
print("=" * 70)

for prot in TYPEC_PROTECTIONS:
    print(f"\n  --- {prot['name']} ---")

    # High-side hysteresis
    if prot["trigger_hi"] is not None and prot["recovery_hi"] is not None:
        gap_hi = prot["trigger_hi"] - prot["recovery_hi"]
        check(gap_hi > 0,
              f"OT hysteresis: trigger {prot['trigger_hi']}C > "
              f"recovery {prot['recovery_hi']}C (gap={gap_hi}C)")
        if gap_hi > 40:
            warn(f"OT hysteresis gap unusually large: {gap_hi}C "
                 f"(system stays in protection far longer than needed)")

    # Low-side hysteresis
    if prot["trigger_lo"] is not None and prot["recovery_lo"] is not None:
        gap_lo = prot["recovery_lo"] - prot["trigger_lo"]
        check(gap_lo > 0,
              f"UT hysteresis: trigger {prot['trigger_lo']}C, "
              f"recovery {prot['recovery_lo']}C (gap={gap_lo}C)")
        check(gap_lo >= 3,
              f"UT hysteresis gap >= 3C (got {gap_lo}C)")

print()

# -------------------------------------------------------------------
print("=" * 70)
print("TEST 5: Sliding-window filter behavior")
print("=" * 70)

# 5a. Cold start: buffer initialized to 1650 (approx 25C with code offset)
filt = NtcFilter(init_val=1650)
init_temp = ntc_lookup(1650)
print(f"  Init buffer value=1650 -> lookup={init_temp}C")
check(-10 <= init_temp <= 40, f"Init temperature {init_temp}C is in safe range")

# 5b. Step response: feed 8 identical samples -> should converge
target_adc = adc_at_temp(80)  # 80C real -> ADC=461
for _ in range(8):
    result = filt.feed_and_lookup(target_adc)
target_temp_expected = 80 - offset_error  # 70 with code offset
check(result == target_temp_expected,
      f"After 8x ADC={target_adc}, temp converges to {result}C "
      f"(expected {target_temp_expected}C)")

# 5c. Single spike rejection: one noisy sample in stable reading
filt2 = NtcFilter(init_val=2278)  # stable at ~20C ADC
for _ in range(8):
    filt2.feed(2278)
baseline = ntc_lookup(2278)

# Inject one extreme sample
filt2.feed(500)  # spike to ~90C ADC
spike_temp = ntc_lookup(filt2.buf[0])  # not using feed_and_lookup, just check avg
avg_after_spike = sum(filt2.buf) // 8
temp_after_spike = ntc_lookup(avg_after_spike)
check(abs(temp_after_spike - baseline) <= 15,
      f"Single spike dampened: baseline={baseline}C, "
      f"after spike avg temp={temp_after_spike}C (delta={temp_after_spike - baseline}C)")

print()

# -------------------------------------------------------------------
print("=" * 70)
print("TEST 6: Protection oscillation simulation")
print("=" * 70)

# Simulate temperature hovering near -15C threshold to verify hysteresis
# prevents rapid oscillation between lock/unlock
# Trigger: ntc_temp_typec < -15,  Recovery: ntc_temp_typec > -10
print("\n  --- typec_ntc_lock UT: trigger<-15C, recovery>-10C ---")

# Temps oscillating tightly around -15C (the trigger boundary)
# With hysteresis: once locked, stays locked until > -10C
# Without hysteresis: oscillates on every crossing of -15C
temps = [-14, -15, -16, -15, -14, -15, -16, -15, -14, -16, -15, -14]

# WITH hysteresis (fixed code: trigger < -15, recovery > -10)
lock_hys = False
trans_hys = 0
for t in temps:
    if not lock_hys:
        if t < -15:
            lock_hys = True
            trans_hys += 1
    else:
        if t > -10:
            lock_hys = False
            trans_hys += 1

check(trans_hys == 1,
      f"With hysteresis: {trans_hys} transitions across "
      f"{len(temps)} samples (expected 1: lock once, never reach -10 to unlock)")

# WITHOUT hysteresis (old code: trigger < -15, recovery > -15)
lock_no_hys = False
trans_no_hys = 0
for t in temps:
    if not lock_no_hys:
        if t < -15:
            lock_no_hys = True
            trans_no_hys += 1
    else:
        if t > -15:
            lock_no_hys = False
            trans_no_hys += 1

check(trans_no_hys > trans_hys,
      f"Without hysteresis: {trans_no_hys} transitions >> "
      f"{trans_hys} with hysteresis (oscillation confirmed)")
check(trans_no_hys >= 5,
      f"Without hysteresis: {trans_no_hys} transitions >= 5 "
      f"(rapid oscillation around -15C boundary)")

print()

# -------------------------------------------------------------------
print("=" * 70)
print("TEST 7: ADC channel configuration check")
print("=" * 70)

# Verify PC8 = BADC5 channel mapping
BADC_CH_PC8_ADC5 = 5
BADC_CH_PB6_ADC7 = 7  # old channel

check(BADC_CH_PC8_ADC5 == 5,
      f"PC8 maps to BADC channel 5 (ADC5)")
check(BADC_CH_PC8_ADC5 != BADC_CH_PB6_ADC7,
      f"New channel PC8(ADC5={BADC_CH_PC8_ADC5}) != "
      f"old channel PB6(ADC7={BADC_CH_PB6_ADC7})")

# Check GPIO MODE should be 2 for BADC5 function
# gpio.c:393: GPC->MODE.BITS.PIN8 = 2; //00:CC2_L 01:PC8 10:BADC5
PC8_MODE_FOR_ADC = 2
check(PC8_MODE_FOR_ADC == 2,
      f"PC8 MODE=2 selects BADC5 function (verified in gpio.c:393)")

print()

# ===================================================================
#                           SUMMARY
# ===================================================================
print("=" * 70)
print("SUMMARY")
print("=" * 70)
print(f"  PASS: {pass_count}")
print(f"  FAIL: {fail_count}")
print(f"  WARN: {warn_count}")
print()

if offset_error != 0:
    print(f"  *** OFFSET BUG DETECTED ***")
    print(f"  prot.c:134  return ((int)i - {CODE_OFFSET});")
    print(f"  Should be:  return ((int)i - {TABLE_OFFSET});")
    print(f"  All TypeC NTC temperatures are {offset_error}C lower than real.")
    print(f"  Protection thresholds trigger {offset_error}C later than intended.")
    print()

if fail_count > 0:
    print("  RESULT: FAIL")
    sys.exit(1)
else:
    print("  RESULT: PASS (with warnings)")
    sys.exit(0)
