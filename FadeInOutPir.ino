/**
 * PIR sensor
 */
 
#define  IN_TIME       15    // seconds
#define  ON_TIME      120    // seconds
#define  OUT_TIME      15    // seconds

#define  STEP_DELAY   100    // smoothnes milliseconds
#define  SLEEP_DELAY 2000    // sleep ms. when nothing to do in OFF state

// define PINs
#define  PIR_PIN        7    // analog/digital pin
#define  PWM_PIN        9    // LED connected to digital pin 9
#define  LED_PIN       13     // blink sometimes

#define hundreds() ((unsigned long)millis()/10L)
//#define seconds() ((unsigned long)millis()/1000L)

//#define DEBUG
//#define LED_ON_PIR
#define LED_ON_ACTION

float r_in;
float r_out;
unsigned long in_time_hs =  IN_TIME  * 100L;
unsigned long on_time_hs =  ON_TIME  * 100L;
unsigned long out_time_hs = OUT_TIME * 100L;
unsigned long time_in_begin;
unsigned long time_on_begin;
unsigned long time_out_begin;
unsigned long time_off_begin;

unsigned int pir_active = 0;
unsigned int delay_ms;
unsigned int pwm_x;
unsigned int pwm_y;
unsigned int last_pwm_y;


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

void setup() {
    // R = (pwmIntervals * log10(2))/(log10(255));
    r_in = ( ( (in_time_hs / (STEP_DELAY/10) ) * log10(2) ) / log10(255) );
    r_out = ( ( (out_time_hs / (STEP_DELAY/10) ) * log10(2) ) / log10(255) );

    pinMode(PIR_PIN, INPUT);
    pinMode(PWM_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    #ifdef DEBUG
        Serial.begin(9600);
        Serial.print("PIR sensor  ");
        Serial.print(F(__DATE__));
        Serial.print("  ");
        Serial.println(F(__TIME__));
    
        Serial.print("R_IN/R_OUT: ");
        Serial.print(r_in);
        Serial.print(" / ");
        Serial.println(r_out);
    #endif

    digitalWrite(LED_PIN, HIGH);    // note that it has begun
    analogWrite(PWM_PIN, 1);        // start from the begin
    
    time_in_begin = hundreds();                 // fade in begin
    time_on_begin = time_in_begin + in_time_hs;     // on begin
    time_out_begin = time_on_begin + on_time_hs;    // fade out begin
    time_off_begin = time_out_begin + out_time_hs;  // off begin
}

/** L O O P **/
void loop() {
    unsigned long now = hundreds();
    digitalWrite(LED_PIN, LOW);

    delay_ms = STEP_DELAY;
    pir_active = ( digitalRead(PIR_PIN) ? pir_active+1 : 0 );       // pir is off

    // pir state is LOW == activated
    if ( pir_active > 0 ) {

        #ifdef LED_ON_PIR
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));   // blink
        delay(50);
        #endif

        // pir was activated n times by STEP_DELAY:
        if ( pir_active == 3 ) {
            digitalWrite(LED_PIN, HIGH);

            // pir was activated during what phase?
            if (now < time_out_begin) {
                // fade in && on: extend ON phase
                time_out_begin = now + in_time_hs + on_time_hs;
                Serial.println("IN/ON-PIR");
            } else {
                if (now < time_off_begin ) {
                    // out: shift begin beyond: start_milliseconds = now() - time to the end
                    // time_in_begin = now - (in_time_hs + on_time_hs + out_time_hs) + (now - time_in_begin);
                    time_in_begin = now - (time_off_begin - now);
                    Serial.println("OUT-PIR");
                } else {
                    // off
                    time_in_begin = now;                 // fade in begin
                    Serial.println("OFF-PIR");
                }
                time_on_begin = time_in_begin + in_time_hs;     // on begin
                time_out_begin = time_on_begin + on_time_hs;    // fade out begin
            }
            time_off_begin = time_out_begin + out_time_hs;  // off begin
            
            #ifdef DEBUG
            Serial.print(now);
            Serial.print(" </^/>/_: ");
            Serial.print(time_in_begin);
            Serial.print("/");
            Serial.print(time_on_begin);
            Serial.print("/");
            Serial.print(time_out_begin);
            Serial.print("/");
            Serial.println(time_off_begin);
            #endif
        }
    }



    if (now < time_on_begin) {
        // fade in
        pwm_x = (now - time_in_begin) / (STEP_DELAY / 10);
        pwm_y = min(255, pow(2, (pwm_x / r_in ) ));

        #ifdef LED_ON_ACTION
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));   // blink
        #endif
        
        #ifdef DEBUG
        if ( last_pwm_y != pwm_y ) Serial.print("<");
        #endif
    } else if (now < time_out_begin) {
        // on
        pwm_x = (now - time_on_begin) / 100;
        pwm_y = 255;
        delay_ms = SLEEP_DELAY;

        #ifdef LED_ON_ACTION
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));   // blink
        #endif
        
        #ifdef DEBUG
        if ( last_pwm_y != pwm_y ) Serial.print("^");
        #endif
    } else if (now < time_off_begin ) {
        // out
//      pwm_x = ((out_time_hs - (now - time_in_begin - in_time_hs - on_time_hs)) / (STEP_DELAY/10) );
        pwm_x = ( time_off_begin - now ) / (STEP_DELAY/10) ;
        pwm_y = min(255, pow(2, ( pwm_x / r_out)));

        #ifdef LED_ON_ACTION
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));   // blink
        #endif
        
        #ifdef DEBUG
        if ( last_pwm_y != pwm_y ) Serial.print(">");
        #endif
    } else {
        // off
        digitalWrite(LED_PIN, LOW);
//      pwm_x = (now - time_off_begin) / (SLEEP_DELAY/10);
        pwm_x = (now - time_off_begin) / 100;
        pwm_y = 0;
        delay_ms = SLEEP_DELAY;

        #ifdef DEBUG
        if ( last_pwm_y != pwm_y ) Serial.print("_");
        #endif
    }
    
    analogWrite(PWM_PIN, pwm_y);
    
    #ifdef DEBUG
    if ( last_pwm_y != pwm_y ) {
        Serial.print(millis()/1000);
        Serial.print("[");
        Serial.print(pir_active);
        Serial.print(", ");
        Serial.print(pwm_x);
        Serial.print(", ");
        Serial.print(pwm_y);
        Serial.println("]");
    }
    #endif

    last_pwm_y = pwm_y;
    delay( pir_active ? STEP_DELAY : delay_ms );
}
