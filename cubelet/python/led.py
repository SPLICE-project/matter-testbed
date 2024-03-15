#!/usr/bin/env python3
import RPi.GPIO as GPIO
from socket import gethostname
from time import sleep

BUTTON_PIN = 23
LED_PIN    = 24


def get_cubelet_num():
    num = gethostname().split('cubelet')[1]
    return int(num)


def main():
    cubelet_num = get_cubelet_num()
    try:
        while True:
            button_pressed = GPIO.input(BUTTON_PIN)
            if button_pressed:
                for i in range(0, cubelet_num):
                    GPIO.output(LED_PIN, True)
                    sleep(0.2)
            else:
                GPIO.output(LED_PIN, False)
    except:
        GPIO.cleanup()


if __name__ == "__main__":
    GPIO.setup(BUTTON_PIN, GPIO.IN, pull_up_down=GPIO.PUD_UP)
    GPIO.setup(LED_PIN, GPIO.OUT)
    main()