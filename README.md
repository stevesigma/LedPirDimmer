# LedPirDimmer
Arduino led dimmer with pir sensors
/**
 * when pir activated in_time ago:
 *   if current_phase is OFF
 *     raise PWM_YVAL
 *   else if current_phase is FADE_IN
 *     continue raise
 *     shift start
 *   else if current_phase is ON
 *     hold HIGH
 *   else if current_phase is FADE_OUT
 *     continue raise
 *     shift start
 * else if pir activated in_time + on_time ago:
 *   hold on HIGH
 * else if pir activated in_time + on_time + out_time ago:
 *   decrease PWM_YVAL
 * else
 *   hold LOW
 **/
