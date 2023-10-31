#include "pico/stdlib.h"
#include "hardware/pwm.h"

#define sw0 9
#define sw1 8
#define sw2 7
#define led1 22
#define led2 21
#define led3 20

bool isPressed(uint gpio);

int main() {
    // Configure switches and LEDs
    gpio_set_function(sw0, GPIO_FUNC_SIO);
    gpio_set_dir(sw0, false);
    gpio_pull_up(sw0);
    gpio_set_function(sw1, GPIO_FUNC_SIO);
    gpio_set_dir(sw1, false);
    gpio_pull_up(sw1);
    gpio_set_function(sw2, GPIO_FUNC_SIO);
    gpio_set_dir(sw2, false);
    gpio_pull_up(sw2);

    // Initialize variables for PWM control
    bool btn_held = false;
    uint16_t wrap = 999;
    uint duty = 0;
    uint pre_duty = 0;
    uint div = 125;
    uint slice_num1 = pwm_gpio_to_slice_num(led1);
    uint slice_num2 = pwm_gpio_to_slice_num(led2);
    uint slice_num3 = pwm_gpio_to_slice_num(led3);
    uint chan1 = pwm_gpio_to_channel(led1);
    uint chan2 = pwm_gpio_to_channel(led2);
    uint chan3 = pwm_gpio_to_channel(led3);

    // Disable PWM for all slices initially
    pwm_set_enabled(slice_num1, false);
    pwm_set_enabled(slice_num2, false);
    pwm_set_enabled(slice_num3, false);

    // Configure PWM settings
    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv_int(&cfg, div); // Set clock divider to achieve 1MHz
    pwm_config_set_wrap(&cfg, wrap);      // Set wrap value to achieve 1kHz
    pwm_init(slice_num1, &cfg, false);
    pwm_init(slice_num2, &cfg, false);
    pwm_init(slice_num3, &cfg, false);

    // Set the initial duty cycle for all LEDs
    pwm_set_chan_level(slice_num1, chan1, (wrap + 1) * duty / 100);
    pwm_set_chan_level(slice_num2, chan2, (wrap + 1) * duty / 100);
    pwm_set_chan_level(slice_num3, chan3, (wrap + 1) * duty / 100);

    // Configure LEDs as PWM outputs
    gpio_set_function(led1, GPIO_FUNC_PWM);
    gpio_set_function(led2, GPIO_FUNC_PWM);
    gpio_set_function(led3, GPIO_FUNC_PWM);

    // Enable PWM for all slices
    pwm_set_enabled(slice_num1, true);
    pwm_set_enabled(slice_num2, true);
    pwm_set_enabled(slice_num3, true);

    // Continuously check switch inputs
    while (1) {
        // Check sw1 for toggling the brightness
        if (!gpio_get(sw1) && !btn_held) {
            if (isPressed(sw1)) {
                // Toggle duty cycle between 0 and 50%
                if (duty) {
                    pre_duty = duty;
                    duty = 0;
                } else {
                    if (pre_duty) {
                        duty = pre_duty;
                    } else {
                        duty = 50;
                        pre_duty = duty;
                    }
                }
                // Update LED brightness
                pwm_set_chan_level(slice_num1, chan1, (wrap + 1) * duty / 100);
                pwm_set_chan_level(slice_num2, chan2, (wrap + 1) * duty / 100);
                pwm_set_chan_level(slice_num3, chan3, (wrap + 1) * duty / 100);
                btn_held = true;
            }
        } else if (gpio_get(sw1)) {
            btn_held = false;
        }

        // Check sw0 for increasing brightness
        if (!gpio_get(sw0)) {
            while (isPressed(sw0)) {
                if (duty < 100) {
                    duty += 1;
                    pre_duty = duty;
                    // Update LED brightness
                    pwm_set_chan_level(slice_num1, chan1, (wrap + 1) * duty / 100);
                    pwm_set_chan_level(slice_num2, chan2, (wrap + 1) * duty / 100);
                    pwm_set_chan_level(slice_num3, chan3, (wrap + 1) * duty / 100);
                    sleep_ms(100);
                }
            }
        }
        // Check sw2 for decreasing brightness
        if (!gpio_get(sw2)) {
            while (isPressed(sw2)) {
                if (duty > 0) {
                    duty -= 1;
                    pre_duty = duty;
                    // Update LED brightness
                    pwm_set_chan_level(slice_num1, chan1, (wrap + 1) * duty / 100);
                    pwm_set_chan_level(slice_num2, chan2, (wrap + 1) * duty / 100);
                    pwm_set_chan_level(slice_num3, chan3, (wrap + 1) * duty / 100);
                    sleep_ms(100);
                }
            }
        }
    }

    return 0;
}

// Function to debounce button presses
bool isPressed(uint gpio) {
    uint pressed = 0;
    uint released = 0;
    while (pressed < 3 && released < 3) {
        if (!gpio_get(gpio)) {
            pressed++;
            released = 0;
        } else {
            released++;
            pressed = 0;
        }
        sleep_ms(10);
    }
    return pressed ? true : false;
}
